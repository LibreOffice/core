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
#include <ctype.h>
#if (defined(_WIN32) || defined(__IBMC__))
#include <io.h>
#else
#include <unistd.h>
#endif
#include "cpp.h"


static char wbuf[4 * OBS];
static char *wbp = wbuf;
static int EBCDIC_ExternTokenDetected = 0;
static int EBCDIC_StartTokenDetected = 0;

static unsigned char toLatin1[256] =
{
    0x00, 0x01, 0x02, 0x03, 0x9c, 0x09, 0x86, 0x7f, 0x97, 0x8d,
    0x8e, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
    0x9d, 0x0a, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8f, 0x1c, 0x1d,
    0x1e, 0x1f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x17, 0x1b,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x05, 0x06, 0x07, 0x90, 0x91,
    0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9a, 0x9b,
    0x14, 0x15, 0x9e, 0x1a, 0x20, 0xa0, 0xe2, 0xe4, 0xe0, 0xe1,
    0xe3, 0xe5, 0xe7, 0xf1, 0xa2, 0x2e, 0x3c, 0x28, 0x2b, 0x7c,
    0x26, 0xe9, 0xea, 0xeb, 0xe8, 0xed, 0xee, 0xef, 0xec, 0xdf,
    0x21, 0x24, 0x2a, 0x29, 0x3b, 0x5e, 0x2d, 0x2f, 0xc2, 0xc4,
    0xc0, 0xc1, 0xc3, 0xc5, 0xc7, 0xd1, 0xa6, 0x2c, 0x25, 0x5f,
    0x3e, 0x3f, 0xf8, 0xc9, 0xca, 0xcb, 0xc8, 0xcd, 0xce, 0xcf,
    0xcc, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22,
    0xd8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0xab, 0xbb, 0xf0, 0xfd, 0xfe, 0xb1, 0xb0, 0x6a, 0x6b, 0x6c,
    0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0xaa, 0xba, 0xe6, 0xb8,
    0xc6, 0xa4, 0xb5, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0xa1, 0xbf, 0xd0, 0x5b, 0xde, 0xae, 0xac, 0xa3,
    0xa5, 0xb7, 0xa9, 0xa7, 0xb6, 0xbc, 0xbd, 0xbe, 0xdd, 0xa8,
    0xaf, 0x5d, 0xb4, 0xd7, 0x7b, 0x41, 0x42, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49, 0xad, 0xf4, 0xf6, 0xf2, 0xf3, 0xf5,
    0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52,
    0xb9, 0xfb, 0xfc, 0xf9, 0xfa, 0xff, 0x5c, 0xf7, 0x53, 0x54,
    0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xb2, 0xd4, 0xd6, 0xd2,
    0xd3, 0xd5, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0xb3, 0xdb, 0xdc, 0xd9, 0xda, 0x9f
};

#define MASK    "\\x%x"

static int
    memcpy_EBCDIC( char * pwbuf, uchar const *p, int len )
{
    int currpos = 0;
    int processedchars = 0;

    if( len == 0 )
        return 0;

    if( len == 1 )
    {
        *pwbuf = *p;
        return 1;
    }

    /* copy spaces until " or ' */
    while( (p[ processedchars ] != '\"') && (p[ processedchars ] != '\'') )
        pwbuf[ currpos++ ] = p[ processedchars++ ];

    /* copy first " or ' */
    pwbuf[ currpos++ ] = p[ processedchars++ ];

    /* convert all characters until " or ' */
    while( processedchars < (len - 1) )
    {
        if( p[ processedchars ] == '\\' )
        {
            switch( p[ ++processedchars ] )
            {
                case 'n':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\n'] );
                    processedchars++;
                    break;

                case 't':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\t'] );
                    processedchars++;
                    break;

                case 'v':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\v'] );
                    processedchars++;
                    break;

                case 'b':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\b'] );
                    processedchars++;
                    break;

                case 'r':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\r'] );
                    processedchars++;
                    break;

                case 'f':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\f'] );
                    processedchars++;
                    break;

                case 'a':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\a'] );
                    processedchars++;
                    break;

                case '\\':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\\'] );
                    processedchars++;
                    break;

                case '?':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\?'] );
                    processedchars++;
                    break;

                case '\'':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\''] );
                    processedchars++;
                    break;

                case '"':
                    currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1['\"'] );
                    processedchars++;
                    break;

                /* octal coded character? -> copy */
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    {
                    int startpos = currpos;

                    pwbuf[ currpos++ ] = '\\';

                    while( p[ processedchars ] >= '0' && p[ processedchars ] <= '7' && (currpos < startpos + 4) )
                          pwbuf[ currpos++ ] = (unsigned char)p[ processedchars++ ];
                    break;
                    }

                /* hex coded character? -> copy */
                case 'x':
                case 'X':
                    {
                    int startpos = currpos;

                    pwbuf[ currpos++ ] = '\\';
                    pwbuf[ currpos++ ] = 'x';
                    processedchars++;

                    while( isxdigit( p[ processedchars ] ) && (currpos < startpos + 4) )
                          pwbuf[ currpos++ ] = (unsigned char)p[ processedchars++ ];
                    break;
                    }

            }
        }
        else
            currpos += sprintf( &pwbuf[ currpos ], MASK, toLatin1[p[ processedchars++ ]] );

    }

    /* copy last " or ' */
    pwbuf[ currpos++ ] = p[ processedchars ];

    return currpos;
}

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

            /* EBCDIC to ANSI conversion requested? */
            if( Aflag )
            {
                /* keyword __ToLatin1__ found? -> do conversion! */
                if( EBCDIC_StartTokenDetected )
                {
                    /* previous token was 'extern'? -> don't convert current token! */
                    if( EBCDIC_ExternTokenDetected )
                    {
                        EBCDIC_ExternTokenDetected = 0;
                        memcpy(wbp, p, len);
                    }
                    else
                    {
                        /* current token is keyword 'extern'? -> don't convert following token! */
                        if( (tp->wslen == 0) && (strncmp( (char*)p, "extern", len ) == 0) )
                        {
                            EBCDIC_ExternTokenDetected = 1;
                            memcpy(wbp, p, len);
                        }
                        else
                        {
                            /* token is string or char? -> process EBCDIC to ANSI conversion */
                            if ((tp->type == STRING) || (tp->type == CCON))
                                len = memcpy_EBCDIC(wbp,  p, len);
                            else
                                memcpy(wbp, p, len);
                        }
                    }
                }
                else
                    /* keyword __ToLatin1__ found? -> don't copy keyword and start conversion */
                    if( (tp->type == NAME) && (strncmp( (char*)p, "__ToLatin1__", len) == 0) )
                    {
                        EBCDIC_StartTokenDetected = 1;
                        len = 0;
                    }
                    else
                        memcpy(wbp, p, len);
            }
            else
                memcpy(wbp, p, len);

            wbp += len;
        }
        else
            *wbp++ = '\n';

        if (wbp >= &wbuf[OBS])
        {
            if ( write(1, wbuf, OBS) != -1 ) {
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
