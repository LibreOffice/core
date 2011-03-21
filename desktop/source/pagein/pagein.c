/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "file_image.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* do_pagein */
static int do_pagein (const char * filename, size_t * size)
{
    int result;
    file_image image = FILE_IMAGE_INITIALIZER;

    if ((result = file_image_open (&image, filename)) != 0)
        return (result);

    if ((result = file_image_pagein (&image)) != 0)
    {
        fprintf (stderr, "file_image_pagein: %s\n", strerror(result));
        goto cleanup_and_leave;
    }

    if (size)
    {
        *size = image.m_size;
    }

cleanup_and_leave:
    file_image_close (&image);
    return (result);
}

extern int pagein_execute (int argc, char **argv);

/* main */
int pagein_execute (int argc, char **argv)
{
    int    i, v = 0;
    size_t nfiles = 0, nbytes = 0;

    if (argc < 2)
    {
        fprintf (
            stderr,
            "%s: Usage: pagein [-v[v]] [-L<path>] [@]<filename> ...\n",
            argv[0]);
        return (1);
    }

    for (i = 1; i < argc; i++)
    {
        FILE   * fp = 0;
        size_t   k  = 0;

        if (argv[i][0] == '-')
        {
            /* option */
            int j = 1;
            switch (argv[i][j])
            {
                case 'v':
                    /* verbosity level */
                    for (v += 1, j += 1; argv[i][j]; j++)
                        v += (argv[i][j] == 'v');
                    break;
                case 'L':
                    /* search path */
                    if (chdir (&(argv[i][2])) == -1)
                        fprintf (stderr, "chdir: %s\n", strerror(errno));
                    break;
                default:
                    /* ignored */
                    break;
            }

            /* next argv */
            continue;
        }

        if ((argv[i][0] == '@') && ((fp = fopen (argv[i], "r")) == 0))
        {
            char path[1024];
            if ((fp = fopen (&(argv[i][1]), "r")) == 0)
            {
                fprintf (stderr, "fopen: %s\n", strerror(errno));
                continue;
            }
            while (fgets (path, sizeof(path), fp) != 0)
            {
                path[strlen(path) - 1] = '\0', k = 0;
                if (do_pagein (path, &k) == 0)
                {
                    /* accumulate total size */
                    nbytes += k;
                }

                if (v >= 2)
                    fprintf (stderr, "pagein(\"%s\") = %d bytes\n", path, (int) k);
                nfiles += 1;
            }
            fclose (fp);
        }
        else
        {
            if (fp != 0)
                fclose (fp);

            if (do_pagein (argv[i], &k) == 0)
            {
                /* accumulate total size */
                nbytes += k;
            }

            if (v >= 2)
                fprintf (stderr, "pagein(\"%s\") = %d bytes\n", argv[i], (int) k);
            nfiles += 1;
        }
    }

    if (v >= 1)
        fprintf (stderr, "Total: %d files (%d bytes)\n", (int) nfiles, (int) nbytes);
    return (0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
