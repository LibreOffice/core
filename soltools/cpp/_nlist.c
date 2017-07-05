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
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

extern int Cplusplus;
Nlist *kwdefined;
char wd[128];

/*
    ER: Table was made extra large, because there seems to be a problem with the
    chaining. An nlist->next is sometimes overwritten somewhere, which
    results in a SIGSEGV. I canceled the GDB with watchpoint after 2 days, though..
    It works this way for now..
 */
#define NLSIZE 15000

static Nlist *nlist[NLSIZE];

struct kwtab
{
    char *kw;
    int val;
    int flag;
};

static struct kwtab kwtab[] =
{
        {"if", KIF, ISKW},
        {"ifdef", KIFDEF, ISKW},
        {"ifndef", KIFNDEF, ISKW},
        {"elif", KELIF, ISKW},
        {"else", KELSE, ISKW},
        {"endif", KENDIF, ISKW},
        {"include", KINCLUDE, ISKW},
        {"include_next", KINCLUDENEXT, ISKW},
        {"import", KIMPORT, ISKW},
        {"define", KDEFINE, ISKW},
        {"undef", KUNDEF, ISKW},
        {"line", KLINE, ISKW},
        {"error", KERROR, ISKW},
        {"pragma", KPRAGMA, ISKW},
        {"ident", KIDENT, ISKW},
        {"eval", KEVAL, ISKW},
        {"defined", KDEFINED, ISDEFINED + ISUNCHANGE},
        {"machine", KMACHINE, ISDEFINED + ISUNCHANGE},
        {"__LINE__", KLINENO, ISMAC + ISUNCHANGE},
        {"__FILE__", KFILE, ISMAC + ISUNCHANGE},
        {"__DATE__", KDATE, ISMAC + ISUNCHANGE},
        {"__TIME__", KTIME, ISMAC + ISUNCHANGE},
        {"__STDC__", KSTDC, ISUNCHANGE},
        {NULL, 0, 0}
};

unsigned long namebit[077 + 1];

void
    setup_kwtab(void)
{
    struct kwtab *kp;
    Nlist *np;
    Token t;
    static Token deftoken[1] = {{NAME, 0, 7, (uchar *) "defined", 0}};
    static Tokenrow deftr = {deftoken, deftoken, deftoken + 1, 1};

    for (kp = kwtab; kp->kw; kp++)
    {
        t.t = (uchar *) kp->kw;
        t.len = strlen(kp->kw);
        np = lookup(&t, 1);
        np->flag = (char) kp->flag;
        np->val = (char) kp->val;
        if (np->val == KDEFINED)
        {
            kwdefined = np;
            np->val = NAME;
            np->vp = &deftr;
            np->ap = NULL;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
