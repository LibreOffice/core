/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagein.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:29:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

/* main */
int  main (int argc, char **argv)
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
