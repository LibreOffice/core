/*************************************************************************
 *
 *  $RCSfile: include.c,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:48:45 $
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
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#if (defined(_WIN32) || defined(_MSDOS) || defined(__IBMC__)) && !(defined S390)
#   include <io.h>
#else
#   include <unistd.h>
#endif

#ifdef __hpux
#   define _HPUX_SOURCE
#endif
#ifdef _MSC_VER
#   define _POSIX_
#endif
#ifdef SCO
#define _IBCS2
#endif
#include <limits.h>

#include "cpp.h"

Includelist includelist[NINCLUDE];
Wraplist wraplist[NINCLUDE];

void
    doinclude(Tokenrow * trp, int depth, int import)
{
#ifdef OS2
    char fname[_MAX_PATH], iname[_MAX_PATH];
#else
    char fname[PATH_MAX], iname[PATH_MAX];
#endif
    Includelist *ip;
    int angled, len, fd, i;

    trp->tp += 1;
    if (trp->tp >= trp->lp)
        goto syntax;
    if (trp->tp->type != STRING && trp->tp->type != LT)
    {
        len = trp->tp - trp->bp;
        expandrow(trp, "<include>");
        trp->tp = trp->bp + len;
    }
    if (trp->tp->type == STRING)
    {
        len = trp->tp->len - 2;
        if (len > sizeof(fname) - 1)
            len = sizeof(fname) - 1;
        strncpy(fname, (char *) trp->tp->t + 1, len);
        angled = 0;
    }
    else
        if (trp->tp->type == LT)
        {
            len = 0;
            trp->tp++;
            while (trp->tp->type != GT)
            {
                if (trp->tp > trp->lp || len + trp->tp->len + 2 >= sizeof(fname))
                    goto syntax;
                strncpy(fname + len, (char *) trp->tp->t, trp->tp->len);
                len += trp->tp->len;
                trp->tp++;
            }
            angled = 1;
        }
        else
            goto syntax;
    trp->tp += 2;
    if (trp->tp < trp->lp || len == 0)
        goto syntax;
    fname[len] = '\0';
    if (fname[0] == '/')
    {
        fd = open(fname, O_RDONLY);
        strcpy(iname, fname);
    }
    else
        for (fd = -1, i = (depth < 0) ? (NINCLUDE - 1) : (depth - 1); i >= 0; i--)
        {
            ip = &includelist[i];
            if (ip->file == NULL || ip->deleted || (angled && ip->always == 0))
                continue;
            if (strlen(fname) + strlen(ip->file) + 2 > sizeof(iname))
                continue;
            strcpy(iname, ip->file);
            strcat(iname, "/");
            strcat(iname, fname);
            if ((fd = open(iname, O_RDONLY)) >= 0)
                break;
        }
    if (fd >= 0)
    {
        if (++incdepth > NINC )
            error(FATAL, "#%s too deeply nested", import ? "import" : "include");
        if (Xflag)
            genimport(fname, angled, iname, import);
         if (Iflag)
            error(INFO, "Open %s file [%s]", import ? "import" : "include", iname );

        for (i = NINCLUDE - 1; i >= 0; i--)
        {
            if ((wraplist[i].file != NULL) &&
                (strncmp(wraplist[i].file, iname, strlen(wraplist[i].file)) == 0))
                break;
        }

        setsource((char *) newstring((uchar *) iname, strlen(iname), 0), i, fd, NULL, (i >= 0) ? 1 : 0);

        if (!Pflag)
            genline();
    }
    else
    {
        trp->tp = trp->bp + 2;
        error(ERROR, "Could not find %s file %r", import ? "import" : "include", trp);
    }
    return;
syntax:
    error(ERROR, "Syntax error in #%s", import ? "import" : "include");
    return;
}

/*
 * Generate a line directive for cursource
 */
void
    genline(void)
{
    static Token ta = {UNCLASS};
    static Tokenrow tr = {&ta, &ta, &ta + 1, 1};
    uchar *p;

    ta.t = p = (uchar *) outptr;
    strcpy((char *) p, "#line ");
    p += sizeof("#line ") - 1;
    p = (uchar *) outnum((char *) p, cursource->line);
    *p++ = ' ';
    *p++ = '"';
    if (cursource->filename[0] != '/' && wd[0])
    {
        strcpy((char *) p, wd);
        p += strlen(wd);
        *p++ = '/';
    }
    strcpy((char *) p, cursource->filename);
    p += strlen((char *) p);
    *p++ = '"';
    *p++ = '\n';
    ta.len = (char *) p - outptr;
    outptr = (char *) p;
    tr.tp = tr.bp;
    puttokens(&tr);
}

/*
 * Generate a pragma import/include directive
 */
void
    genimport(char *fname, int angled, char *iname, int import)
{
    static Token ta = {UNCLASS};
    static Tokenrow tr = {&ta, &ta, &ta + 1, 1};
    uchar *p;

    ta.t = p = (uchar *) outptr;

    if (import)
        strcpy((char *) p, "#pragma import");
    else
        strcpy((char *) p, "#pragma include");

    p += strlen((char *) p);

    *p++ = '(';

    *p++ = angled ? '<' : '"';
    strcpy((char *) p, fname);
    p += strlen(fname);
    *p++ = angled ? '>' : '"';

    *p++ = ',';

    *p++ = '"';
    strcpy((char *) p, iname);
    p += strlen(iname);
    *p++ = '"';

    *p++ = ')';
    *p++ = '\n';

    ta.len = (char *) p - outptr;
    outptr = (char *) p;
    tr.tp = tr.bp;
    puttokens(&tr);
}

/*
 * Generate a extern C directive
 */
void
    genwrap(int end)
{
    static Token ta = {UNCLASS};
    static Tokenrow tr = {&ta, &ta, &ta + 1, 1};
    uchar *p;

    if (Cplusplus)
    {
        ta.t = p = (uchar *) outptr;

        if (! end)
            strcpy((char *) p, "extern \"C\" {");
        else
            strcpy((char *) p, "}");

        p += strlen((char *) p);

        *p++ = '\n';

        ta.len = (char *) p - outptr;
        outptr = (char *) p;
        tr.tp = tr.bp;
        puttokens(&tr);
    }
}

