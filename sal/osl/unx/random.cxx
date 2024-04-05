/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oslrandom.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

bool osl_get_system_random_data(char* buffer, size_t desired_len)
{
    int fd;

    assert(buffer);

    static int (*lok_open_urandom)()
        = reinterpret_cast<int (*)()>(dlsym(RTLD_DEFAULT, "lok_open_urandom"));
    if (!lok_open_urandom || (fd = lok_open_urandom()) < 0)
        fd = open("/dev/urandom", O_RDONLY);

    if (fd != -1)
    {
        while (desired_len)
        {
            ssize_t nb_read;
            if ((nb_read = read(fd, buffer, desired_len)) == -1)
            {
                if (errno != EINTR)
                {
                    close(fd);
                    return false;
                }
            }
            else
            {
                buffer += nb_read;
                desired_len -= nb_read;
            }
        }
        close(fd);
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
