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
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#if (defined(_WIN32) || defined(_MSDOS) || defined(__IBMC__))
#include <io.h>
#include <sys/stat.h>
#include <external/glibc/getopt.h>
#else
#include <unistd.h>
#endif

#include "cpp.h"

extern char rcsid[];

int Pflag = 0;                          /* print no line information */
int Iflag = 0;                          /* print includes */
int Mflag = 0;                          /* print macor expansion */
int Aflag = 0;                          /* translate character sets */
int Xflag = 0;                          /* print pragma for include/import */
int Vflag = 0;                          /* verbose flag */
int Cflag = 0;                          /* do not remove any comments */
int Cplusplus = 0;

extern void setup_kwtab(void);

void
    setup(int argc, char **argv)
{
    int c, fd, i, n;
    char *fp, *dp;
    Tokenrow tr;

    setup_kwtab();
    while ((c = getopt(argc, argv, "NOPV:I:D:U:F:A:X:u:l:+")) != -1)
        switch (c)
        {
            case 'N':
                for (i = 0; i < NINCLUDE; i++)
                    if (includelist[i].always == 1)
                        includelist[i].deleted = 1;
                break;

            case 'I':
                for (i = NINCLUDE - 2; i >= 0; i--)
                {
                    if (includelist[i].file == NULL)
                    {
                        includelist[i].always = 1;
                        includelist[i].file = optarg;
                        break;
                    }
                }
                if (i < 0)
                    error(FATAL, "Too many -I directives");
                break;

            case 'D':
            case 'U':
            case 'A':
                setsource("<cmdarg>", -1, -1, optarg, 0);
                maketokenrow(3, &tr);
                gettokens(&tr, 1);
                doadefine(&tr, c);
                dofree(tr.bp); tr.bp = 0;
                unsetsource();
                break;

            case 'P':                   /* Lineinfo */
                Pflag++;
                break;

            case 'V':
                for (n = 0; (c = optarg[n]) != '\0'; n++)
                    switch (c)
                    {
                        case 'i':
                            Iflag++;
                            break;

                        case 'm':
                            Mflag = 1;
                            break;

                        case 'x':
                            Mflag = 2;
                            break;

                        case 't':
                            Vflag++;
                            break;

                        case 'v':
                            fprintf(stderr, "%s %s\n", argv[0], rcsid);
                            break;

                        default:
                            error(WARNING, "Unknown verbose option %c", c);
                    }
                break;

            case 'X':
                for (n = 0; (c = optarg[n]) != '\0'; n++)
                    switch (c)
                    {
                        case 'a':
                            Aflag++;
                            break;

                        case 'i':
                            Xflag++;
                            break;

                        case 'c':
                            Cflag++;
                            break;

                        case 'w':
                            dp = &optarg[n + 1];
                            n += strlen(dp);
                            while (isspace(*dp)) dp++;

                            for (i = NINCLUDE - 1; i >= 0; i--)
                            {
                                if (wraplist[i].file == NULL)
                                {
                                    wraplist[i].file = dp;
                                    break;
                                }
                            }
                            if (i < 0)
                                error(WARNING, "Too many -Xw directives");
                            break;

                        default:
                            error(WARNING, "Unknown extension option %c", c);
                    }
                break;

            case '+':
                Cplusplus++;
                break;

            case 'u':                   /* -undef fuer GCC (dummy) */
            case 'l':                   /* -lang-c++ fuer GCC (dummy) */
                break;

            default:
                break;
        }
    dp = ".";
    fp = "<stdin>";
    fd = 0;
    if (optind < argc)
    {
        if ((fp = strrchr(argv[optind], '/')) != NULL)
        {
            int len = fp - argv[optind];

            dp = (char *) newstring((uchar *) argv[optind], len + 1, 0);
            dp[len] = '\0';
        }
        fp = (char *) newstring((uchar *) argv[optind], strlen(argv[optind]), 0);
        if ((fd = open(fp, O_RDONLY)) < 0)
            error(FATAL, "Can't open input file %s", fp);
    }

    if (optind + 1 < argc)
    {
#if defined(WNT) && (_MSC_VER >= 1400)
    int fdo = creat(argv[optind + 1], _S_IREAD | _S_IWRITE );
#else
    int fdo = creat(argv[optind + 1], 0666 );
#endif
        if (fdo < 0)
            error(FATAL, "Can't open output file %s", argv[optind + 1]);

        dup2(fdo, 1);
    }
    includelist[NINCLUDE - 1].always = 0;
    includelist[NINCLUDE - 1].file = dp;
    setsource(fp, -1, fd, NULL, 0);
}


/* memmove is defined here because some vendors don't provide it at
   all and others do a terrible job (like calling malloc) */

#if !defined(__IBMC__) && !defined(WNT) && !defined(__GLIBC__)

void *
    memmove(void *dp, const void *sp, size_t n)
{
    unsigned char *cdp, *csp;

    if (n <= 0)
        return 0;
    cdp = dp;
    csp = (unsigned char *) sp;
    if (cdp < csp)
    {
        do
        {
            *cdp++ = *csp++;
        } while (--n);
    }
    else
    {
        cdp += n;
        csp += n;
        do
        {
            *--cdp = *--csp;
        } while (--n);
    }
    return 0;
}

#endif
