/*************************************************************************
 *
 *  $RCSfile: nlist.c,v $
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
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

extern int getopt(int, char *const *, const char *);
extern char *optarg;
extern int optind;
extern int Cplusplus;
Nlist *kwdefined;
char wd[128];

/*
    ER: Tabelle extra gross gemacht, da es anscheinend ein Problem mit der
    der Verkettung gibt, irgendwann irgendwo wird mal ein nlist->next
    ueberschrieben, was in eineme SIGSEGV resultiert.
    Den GDB mit watchpoint hab ich aber nach 2 Tagen abgebrochen..
    so loeppt´s jedenfalls erstmal..
 */
#define NLSIZE 15000

static Nlist *nlist[NLSIZE];

struct kwtab
{
    char *kw;
    int val;
    int flag;
}   kwtab[] =

{
    "if", KIF, ISKW,
    "ifdef", KIFDEF, ISKW,
    "ifndef", KIFNDEF, ISKW,
    "elif", KELIF, ISKW,
    "else", KELSE, ISKW,
    "endif", KENDIF, ISKW,
    "include", KINCLUDE, ISKW,
    "include_next", KINCLUDENEXT, ISKW,
    "import", KIMPORT, ISKW,
    "define", KDEFINE, ISKW,
    "undef", KUNDEF, ISKW,
    "line", KLINE, ISKW,
    "error", KERROR, ISKW,
    "pragma", KPRAGMA, ISKW,
    "ident", KIDENT, ISKW,
    "eval", KEVAL, ISKW,
    "defined", KDEFINED, ISDEFINED + ISUNCHANGE,
    "machine", KMACHINE, ISDEFINED + ISUNCHANGE,
    "__LINE__", KLINENO, ISMAC + ISUNCHANGE,
    "__FILE__", KFILE, ISMAC + ISUNCHANGE,
    "__DATE__", KDATE, ISMAC + ISUNCHANGE,
    "__TIME__", KTIME, ISMAC + ISUNCHANGE,
    "__STDC__", KSTDC, ISUNCHANGE,
    NULL
};

unsigned long namebit[077 + 1];
Nlist *np;

void
    setup_kwtab(void)
{
    struct kwtab *kp;
    Nlist *np;
    Token t;
    static Token deftoken[1] = {{NAME, 0, 0, 7, (uchar *) "defined"}};
    static Tokenrow deftr = {deftoken, deftoken, deftoken + 1, 1};

    for (kp = kwtab; kp->kw; kp++)
    {
        t.t = (uchar *) kp->kw;
        t.len = strlen(kp->kw);
        np = lookup(&t, 1);
        np->flag = kp->flag;
        np->val = kp->val;
        if (np->val == KDEFINED)
        {
            kwdefined = np;
            np->val = NAME;
            np->vp = &deftr;
            np->ap = 0;
        }
    }
}

Nlist *
    lookup(Token * tp, int install)
{
    unsigned int h;
    Nlist *np;
    uchar *cp, *cpe;

    h = 0;
    for (cp = tp->t, cpe = cp + tp->len; cp < cpe;)
        h += *cp++;
    h %= NLSIZE;
    np = nlist[h];
    while (np)
    {
        if (*tp->t == *np->name && tp->len == (unsigned int)np->len
            && strncmp((char *)tp->t, (char *)np->name, tp->len) == 0)
            return np;
        np = np->next;
    }
    if (install)
    {
        np = new(Nlist);
        np->vp = NULL;
        np->ap = NULL;
        np->flag = 0;
        np->val = 0;
        np->len = tp->len;
        np->name = newstring(tp->t, tp->len, 0);
        np->next = nlist[h];
        nlist[h] = np;
        quickset(tp->t[0], tp->len > 1 ? tp->t[1] : 0);
        return np;
    }
    return NULL;
}
