/*************************************************************************
 *
 *  $RCSfile: tokens.c,v $
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
#include <ctype.h>
#if (defined(_WIN32) || defined(_MSDOS) || defined(__IBMC__)) && !(defined S390)
#include <io.h>
#else
#include <unistd.h>
#endif
#include "cpp.h"


static char wbuf[4 * OBS];
static char *wbp = wbuf;
static int EBCDIC_ExternTokenDetected = 0;
static int EBCDIC_StartTokenDetected = 0;

unsigned char toLatin1[256] =
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

int
    memcpy_EBCDIC( char * pwbuf, uchar *p, int len )
{
    int newlen = 0;
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
    int ncur = trp->tp - trp->bp;
    int nlast = trp->lp - trp->bp;

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
    insertrow(Tokenrow * dtr, int ntok, Tokenrow * str)
{
    int nrtok = rowlen(str);

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
    makespace(Tokenrow * trp, Token * ntp)
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
        tp->flag |= XPWS;
    }
}

/*
 * Copy an entire tokenrow into another, at tp.
 * It is assumed that there is enough space.
 *  Not strictly conforming.
 */
void
    movetokenrow(Tokenrow * dtr, Tokenrow * str)
{
    int nby;

    /* nby = sizeof(Token) * (str->lp - str->bp); */
    nby = (char *) str->lp - (char *) str->bp;
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
    int nby, size;

    if (nt == 0)
        return;
    size = (trp->lp - trp->bp) + nt;
    while (size > trp->max)
        growtokenrow(trp);
    /* nby = sizeof(Token) * (trp->lp - trp->tp); */
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
    copytokenrow(Tokenrow * dtr, Tokenrow * str)
{
    int len = rowlen(str);

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

    len = trp->lp - trp->tp;
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
        fprintf(stderr, "(tp offset %d) ", tp - trp->bp);
    for (tp = trp->bp; tp < trp->lp && tp < trp->bp + 32; tp++)
    {
        if (tp->type != NL)
        {
            int c = tp->t[tp->len];

            tp->t[tp->len] = 0;
            fprintf(stderr, "%s", tp->t);
            tp->t[tp->len] = c;
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
            len = tp->len + tp->wslen;
            p = tp->t - tp->wslen;

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
            write(1, wbuf, OBS);
            if (wbp > &wbuf[OBS])
                memcpy(wbuf, wbuf + OBS, wbp - &wbuf[OBS]);
            wbp -= OBS;
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
        write(1, wbuf, wbp - wbuf);
        wbp = wbuf;
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
    *p++ = n % 10 + '0';
    return p;
}

/*
 * allocate and initialize a new string from s, of length l, at offset o
 * Null terminated.
 */
uchar *
    newstring(uchar * s, int l, int o)
{
    uchar *ns = (uchar *) domalloc(l + o + 1);

    ns[l + o] = '\0';
    return (uchar *) strncpy((char *) ns + o, (char *) s, l) - o;
}
