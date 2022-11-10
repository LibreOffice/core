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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if (defined(_WIN32) || defined(__IBMC__))
#include <io.h>
#else
#include <unistd.h>
#endif
#include "cpp.h"


static char wbuf[4 * OBS];
static char *wbp = wbuf;

void
    maketokenrow(int size, Tokenrow * trp)
{
    trp->max = size;
    if (size > 0)
        trp->bp = (Token *) domalloc(size * sizeof(Token));
    else
        trp->bp = NULL;
    trp->tp = trp->bp;
    trp->lp = trp->bp;
}

Token *
    growtokenrow(Tokenrow * trp)
{
    size_t ncur = trp->tp - trp->bp;
    size_t nlast = trp->lp - trp->bp;

    trp->max = 3 * trp->max / 2 + 1;
    trp->bp = (Token *) realloc(trp->bp, trp->max * sizeof(Token));
    assert(trp->bp); // realloc failure is OOM -> no point to handle
    trp->lp = &trp->bp[nlast];
    trp->tp = &trp->bp[ncur];
    return trp->lp;
}

/*
 * Compare a row of tokens, ignoring the content of WS; return !=0 if different
 */
int
    comparetokens(Tokenrow * tr1, Tokenrow * tr2)
{
    Token *tp1, *tp2;

    tp1 = tr1->tp;
    tp2 = tr2->tp;
    if (tr1->lp - tp1 != tr2->lp - tp2)
        return 1;
    for (; tp1 < tr1->lp; tp1++, tp2++)
    {
        if (tp1->type != tp2->type
            || (tp1->wslen == 0) != (tp2->wslen == 0)
            || tp1->len != tp2->len
            || strncmp((char *) tp1->t, (char *) tp2->t, tp1->len) != 0)
            return 1;
    }
    return 0;
}

/*
 * replace ntok tokens starting at dtr->tp with the contents of str.
 * tp ends up pointing just beyond the replacement.
 * Canonical whitespace is assured on each side.
 */
void
    insertrow(Tokenrow * dtr, int ntok, Tokenrow const * str)
{
    int nrtok = (int)rowlen(str);

    dtr->tp += ntok;
    adjustrow(dtr, nrtok - ntok);
    dtr->tp -= ntok;
    movetokenrow(dtr, str);
    dtr->tp += nrtok;
}

/*
 * make sure there is WS before trp->tp, if tokens might merge in the output
 */
void
    makespace(Tokenrow * trp, Token const * ntp)
{
    uchar *tt;
    Token *tp = trp->tp;

    if (tp >= trp->lp)
        return;

    if (ntp->wslen)
    {
        tt = newstring(tp->t, tp->len, ntp->wslen);
        strncpy((char *)tt, (char *)ntp->t - ntp->wslen, ntp->wslen);
        tp->t = tt + ntp->wslen;
        tp->wslen = ntp->wslen;
    }
}

/*
 * Copy an entire tokenrow into another, at tp.
 * It is assumed that there is enough space.
 *  Not strictly conforming.
 */
void
    movetokenrow(Tokenrow * dtr, Tokenrow const * str)
{
    size_t nby;

    nby = (char *) str->lp - (char *) str->bp;
    if (nby)
        memmove(dtr->tp, str->bp, nby);
}

/*
 * Move the tokens in a row, starting at tr->tp, rightward by nt tokens;
 * nt may be negative (left move).
 * The row may need to be grown.
 * Non-strictly conforming because of the (char *), but easily fixed
 */
void
    adjustrow(Tokenrow * trp, int nt)
{
    size_t nby, size;

    if (nt == 0)
        return;
    size = (trp->lp - trp->bp) + nt;
    while (size > trp->max)
        growtokenrow(trp);
    nby = (char *) trp->lp - (char *) trp->tp;
    if (nby)
        memmove(trp->tp + nt, trp->tp, nby);
    trp->lp += nt;
}

/*
 * Copy a row of tokens into the destination holder, allocating
 * the space for the contents.  Return the destination.
 */
Tokenrow *
    copytokenrow(Tokenrow * dtr, Tokenrow const * str)
{
    int len = (int)rowlen(str);

    maketokenrow(len, dtr);
    movetokenrow(dtr, str);
    if (len != 0)
        dtr->lp += len;
    return dtr;
}

/*
 * Produce a copy of a row of tokens.  Start at trp->tp.
 * The value strings are copied as well.  The first token
 * has WS available.
 */
Tokenrow *
    normtokenrow(Tokenrow * trp)
{
    Token *tp;
    Tokenrow *ntrp = new(Tokenrow);
    int len;

    len = (int)(trp->lp - trp->tp);
    if (len <= 0)
        len = 1;
    maketokenrow(len, ntrp);
    for (tp = trp->tp; tp < trp->lp; tp++)
    {
        *ntrp->lp = *tp;
        if (tp->len)
        {
            ntrp->lp->t = newstring(tp->t, tp->len, 1);
            *ntrp->lp->t++ = ' ';
            if (tp->wslen)
                ntrp->lp->wslen = 1;
        }
        ntrp->lp++;
    }
    if (ntrp->lp > ntrp->bp)
        ntrp->bp->wslen = 0;
    return ntrp;
}

/*
 * Debugging
 */
void
    peektokens(Tokenrow * trp, char *str)
{
    Token *tp;

    tp = trp->tp;
    flushout();
    if (str)
        fprintf(stderr, "%s ", str);
    if (tp < trp->bp || tp > trp->lp)
        fprintf(stderr, "(tp offset %ld) ", (long int) (tp - trp->bp));
    for (tp = trp->bp; tp < trp->lp && tp < trp->bp + 32; tp++)
    {
        if (tp->type != NL)
        {
            int c = tp->t[tp->len];

            tp->t[tp->len] = 0;
            fprintf(stderr, "%s", tp->t);
            tp->t[tp->len] = (uchar) c;
        }
        fprintf(stderr, tp == trp->tp ? "{%x*} " : "{%x} ", tp->type);
    }
    fprintf(stderr, "\n");
    fflush(stderr);
}

void
    puttokens(Tokenrow * trp)
{
    Token *tp;
    int len;
    uchar *p;

    if (Vflag)
        peektokens(trp, "");
    tp = trp->bp;
    for (; tp < trp->lp; tp++)
    {
        if (tp->type != NL)
        {
            len = (int)(tp->len + tp->wslen);
            p = tp->t - tp->wslen;

            /* add parameter check to delete operator? */
            if( Dflag )
            {
                if( (tp->type == NAME) && (strncmp( (char*)p, "delete", len ) == 0) )
                {
                    Token* ntp = tp;
                    ntp++;

                    if( ntp->type == NAME )
                    {
                        uchar* np = ntp->t - ntp->wslen;
                        int nlen = (int)(ntp->len + ntp->wslen);

                        memcpy(wbp, "if(", 3 );
                        wbp += 4;
                        memcpy(wbp, np, nlen );
                        wbp += nlen;
                        memcpy(wbp, ")", 1 );
                        wbp++;

                        memcpy(wbp, p, len);
                    }
                }
            }

            memcpy(wbp, p, len);

            wbp += len;
        }
        else
            *wbp++ = '\n';

        if (wbp >= &wbuf[OBS])
        {
            if ( write(1, wbuf, OBS) != -1 )
            {
                if (wbp > &wbuf[OBS])
                    memmove(wbuf, wbuf + OBS, wbp - &wbuf[OBS]);
                wbp -= OBS;
            }
            else exit(1);
        }
    }
    trp->tp = tp;
    if (cursource->fd == 0)
        flushout();
}

void
    flushout(void)
{
    if (wbp > wbuf)
    {
        if ( write(1, wbuf, (int)(wbp - wbuf)) != -1)
            wbp = wbuf;
        else
            exit(1);
    }
}

/*
 * turn a row into just a newline
 */
void
    setempty(Tokenrow * trp)
{
    trp->tp = trp->bp;
    trp->lp = trp->bp + 1;
    *trp->bp = nltoken;
}

/*
 * generate a number
 */
char *
    outnum(char *p, int n)
{
    if (n >= 10)
        p = outnum(p, n / 10);
    *p++ = (char) (n % 10 + '0');
    return p;
}

/*
 * allocate and initialize a new string from s, of length l, at offset o
 * Null terminated.
 */
uchar *
    newstring(uchar const * s, size_t l, size_t o)
{
    uchar *ns = (uchar *) domalloc(l + o + 1);

    ns[l + o] = '\0';
    return (uchar *) strncpy((char *) ns + o, (char *) s, l) - o;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
