/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: getopt.c,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-12 15:29:06 $
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

#ifndef OS2 // YD use libc getopt

#include    <stdio.h>
#include    <string.h>

#define EPR                 fprintf(stderr,
#define ERR(str, chr)       if(opterr) { EPR "%s%c\n", str, chr); }

int opterr = 1;
int optind = 1;
int optopt;
char *optarg;

int
    cppgetopt(int argc, char *const argv[], const char *opts)
{
    static int sp = 1;
    register int c;
    register char *cp;

    if (sp == 1)
    {
        if (optind >= argc ||
            argv[optind][0] != '-' || argv[optind][1] == '\0')
            return -1;
        else
            if (strcmp(argv[optind], "--") == 0)
            {
                optind++;
                return -1;
            }
    }
    optopt = c = argv[optind][sp];
    if (c == ':' || (cp = strchr(opts, c)) == 0)
    {
        ERR(": illegal option -- ", c);
        if (argv[optind][++sp] == '\0')
        {
            optind++;
            sp = 1;
        }
        return '?';
    }
    if (*++cp == ':')
    {
        if (argv[optind][sp + 1] != '\0')
            optarg = &argv[optind++][sp + 1];
        else
            if (++optind >= argc)
            {
                ERR(": option requires an argument -- ", c);
                sp = 1;
                return '?';
            }
            else
                optarg = argv[optind++];
        sp = 1;
    }
    else
    {
        if (argv[optind][++sp] == '\0')
        {
            sp = 1;
            optind++;
        }
        optarg = 0;
    }
    return c;
}

#endif // OS2 // YD use libc getopt
