//
// Password authentication utilities for Termux
// Copyright (C) 2018-2020 Leonid Plyushch <leonid.plyushch@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

/** Utility for setting new password **/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "termux-auth.h"

static void erase_ptr(void *ptr, unsigned int len) {
    volatile char *p = ptr;

    if (ptr == NULL) {
        return;
    }

    while (len--) {
        *p++ = 0x0;
    }
}

static char *read_password(const char *prompt) {
    char *password;

    password = strdup(getpass(prompt));

    if (!password) {
        fprintf(stderr, "Failed to read password input.\n");
        return NULL;
    }

    if (strlen(password) == 0) {
        fprintf(stderr, "Password cannot be empty.\n");
        return NULL;
    }

    return password;
}

static int main_remove_password(void) {
    int ret = EXIT_FAILURE;

    if (termux_remove_passwd()) {
        puts("Password login successfully disabled.");
        ret = EXIT_SUCCESS;
    } else {
        if (errno == EISDIR) {
            printf("Unexpectedly found directory where hashed password should be ("
                   AUTH_HASH_FILE_PATH
                   "), ignoring\n");
        }
        puts("Failed to disable password login.");
    }

    return ret;
}

static int main_set_password(void) {
    char *password;
    char *password_confirmation;
    int ret = EXIT_FAILURE;

    password = read_password("New password: ");
    if (!password) {
        return ret;
    }

    password_confirmation = read_password("Retype new password: ");
    if (!password_confirmation) {
        return ret;
    }

    if(strcmp(password, password_confirmation) != 0) {
        erase_ptr(password, strlen(password));
        erase_ptr(password_confirmation, strlen(password_confirmation));
        free(password);
        free(password_confirmation);

        puts("Sorry, passwords do not match.");

        return ret;
    }

    if (termux_change_passwd(password)) {
        puts("New password was successfully set.");
        ret = EXIT_SUCCESS;
    } else {
        puts("Failed to set new password.");
    }

    erase_ptr(password, strlen(password));
    erase_ptr(password_confirmation, strlen(password_confirmation));
    free(password);
    free(password_confirmation);

    return ret;
}

int main(int argc, char **argv) {
    switch (argc) {
    case 1:
        return main_set_password();
    case 2:
        if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "-l") == 0) {
            return main_remove_password();
        }
        // otherwise, fall through
    default:
        fprintf(stderr,
                "Run %s without args to set a password. Pass option -d to disable authentication credentials (lock account).\n",
                argv[0]);
        return EXIT_FAILURE;
    }
}
