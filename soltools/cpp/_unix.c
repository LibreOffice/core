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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#if (defined(_WIN32) || defined(__IBMC__))
#include <io.h>
#else
#include <unistd.h>
#endif

#include "cpp.h"

#if defined MACOSX || defined AIX || defined WNT
extern int stgetopt(int, char *const *, const char *);
extern char *optarg;
extern int optind;
#else
#include <getopt.h>
#endif

int Pflag = 0;                          /* print no line information */
int Iflag = 0;                          /* print includes */
int Mflag = 0;                          /* print macor expansion */
int Aflag = 0;                          /* translate character sets */
int Xflag = 0;                          /* print pragma for include/import */
int Vflag = 0;                          /* verbose flag */
int Cflag = 0;                          /* do not remove any comments */
int Dflag = 0;                          /* add parameter check to delete op */
int Cplusplus = 0;

extern void setup_kwtab(void);

void
    setup(int argc, char **argv)
{
    int c, fd, i, n;
    char *fp, *dp;
    Tokenrow tr;

    setup_kwtab();
#if defined MACOSX || defined(AIX) || defined WNT
    while ((c = stgetopt(argc, argv, "NOPV:I:D:U:F:A:X:u:l:+")) != -1)
#else
    while ((c = getopt(argc, argv, "NOPV:I:D:U:F:A:X:u:l:+")) != -1)
#endif
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
                            fprintf(stderr, "%s\n", argv[0]);
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

                        case 'd':
                            Dflag++;
                            break;

                        case 'w':
                            dp = &optarg[n + 1];
                            n += (int)strlen(dp);
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
            int len = (int)(fp - argv[optind]);

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
