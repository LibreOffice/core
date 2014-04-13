/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "file_image.h"
#include "pagein.h"

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
        fprintf (stderr, "file_image_pagein %s: %s\n", filename, strerror(result));
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
                        fprintf (stderr, "chdir %s: %s\n", &(argv[i][2]), strerror(errno));
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
            char fullpath[4096];
            char *path = NULL;
            memset(fullpath, 0, sizeof(fullpath));
            strncpy (fullpath, argv[i] + 1, 3000);
            if (!(path = strrchr (fullpath, '/')))
                path = fullpath;
            else
                path++;

            if ((fp = fopen (&(argv[i][1]), "r")) == 0)
            {
                fprintf (stderr, "fopen %s: %s\n", &(argv[i][1]), strerror(errno));
                continue;
            }
            while (fgets (path, 1024, fp) != 0)
            {
                path[strlen(path) - 1] = '\0', k = 0;

                /* paths relative to the location of the pagein file */
                if (do_pagein (fullpath, &k) == 0)
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
