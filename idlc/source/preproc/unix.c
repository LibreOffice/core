/*************************************************************************
 *
 *  $RCSfile: unix.c,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:48:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#if (defined(_WIN32) || defined(_MSDOS) || defined(__IBMC__)) && !(defined S390)
#include <io.h>
#else
#include <unistd.h>
#endif

#include "cpp.h"

extern int getopt(int, char *const *, const char *);
extern char *optarg, rcsid[];
extern int optind;

int Pflag = 0;                          /* print no line information */
int Iflag = 0;                          /* print includes */
int Mflag = 0;                          /* print macor expansion */
int Aflag = 0;                          /* translate character sets */
int Xflag = 0;                          /* print pragma for include/import */
int Vflag = 0;                          /* verbose flag */
int Cflag = 0;                          /* do not remove any comments */
int Cplusplus = 0;

void
    setup(int argc, char **argv)
{
    int c, fd, i, n;
    char *fp, *dp;
    Tokenrow tr;
    extern void setup_kwtab(void);

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
        if ((fd = open(fp, O_RDONLY)) <= 0)
            error(FATAL, "Can't open input file %s", fp);
    }

    if (optind + 1 < argc)
    {
        int fdo = creat(argv[optind + 1], 0666);

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

#if !defined(__IBMC__) && !defined(WNT)

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

