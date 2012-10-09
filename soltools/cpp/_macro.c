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

#ifdef _MSC_VER
#   define _POSIX_
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__IBMC__) || defined(__EMX__)
#   define PATH_MAX _MAX_PATH
#endif
#include <limits.h>

#include "cpp.h"

#define NCONCAT 16384

/*
 * do a macro definition.  tp points to the name being defined in the line
 */
void
    dodefine(Tokenrow * trp)
{
    Token *tp;
    Nlist *np;
    Source *s;
    Tokenrow *def, *args;
    static uchar location[(PATH_MAX + 8) * NINC], *cp;

    tp = trp->tp + 1;
    if (tp >= trp->lp || tp->type != NAME)
    {
        error(ERROR, "#defined token is not a name");
        return;
    }
    np = lookup(tp, 1);
    if (np->flag & ISUNCHANGE)
    {
        error(ERROR, "#defined token %t can't be redefined", tp);
        return;
    }
    /* collect arguments */
    tp += 1;
    args = NULL;
    if (tp < trp->lp && tp->type == LP && tp->wslen == 0)
    {
        tp += 1;
        args = new(Tokenrow);
        maketokenrow(2, args);
        if (tp->type != RP)
        {
            /* macro with args */
            size_t narg = 0;
            int err = 0;

            for (;;)
            {
                Token *atp;

                if (tp->type != NAME)
                {
                    err++;
                    break;
                }
                if (narg >= args->max)
                    growtokenrow(args);
                for (atp = args->bp; atp < args->lp; atp++)
                    if (atp->len == tp->len
                        && strncmp((char *) atp->t, (char *) tp->t, tp->len) == 0)
                        error(ERROR, "Duplicate macro argument");
                *args->lp++ = *tp;
                narg++;
                tp += 1;
                if (tp->type == RP)
                    break;
                if (tp->type != COMMA)
                {
                    err++;
                    break;
                }
                tp += 1;
            }
            if (err)
            {
                error(ERROR, "Syntax error in macro parameters");
                return;
            }
        }
        tp += 1;
    }
    trp->tp = tp;
    if (((trp->lp) - 1)->type == NL)
        trp->lp -= 1;
    def = normtokenrow(trp);
    if (np->flag & ISDEFINED)
    {
        if (comparetokens(def, np->vp)
            || (np->ap == NULL) != (args == NULL)
            || (np->ap && comparetokens(args, np->ap)))
        {
            if ( np->loc )
                error(ERROR,
                    "Macro redefinition of %t (already defined at %s)",
                    trp->bp + 2, np->loc);
            else
                error(ERROR,
                    "Macro redefinition of %t (already defined at %s)",
                    trp->bp + 2, "commandline" );
        }
    }
    if (args)
    {
        Tokenrow *tap;

        tap = normtokenrow(args);
        dofree(args->bp);
        args = tap;
    }
    np->ap = args;
    np->vp = def;
    np->flag |= ISDEFINED;

    /* build location string of macro definition */
    for (cp = location, s = cursource; s; s = s->next)
        if (*s->filename)
        {
            if (cp != location)
                *cp++ = ' ';
            sprintf((char *)cp, "%s:%d", s->filename, s->line);
            cp += strlen((char *)cp);
        }

    np->loc = newstring(location, strlen((char *)location), 0);

    if (Mflag)
    {
        if (np->ap)
            error(INFO, "Macro definition of %s(%r) [%r]", np->name, np->ap, np->vp);
        else
            error(INFO, "Macro definition of %s [%r]", np->name, np->vp);
    }
}

/*
 * Definition received via -D or -U
 */
void
    doadefine(Tokenrow * trp, int type)
{
    Nlist *np;
    static uchar onestr[2] = "1";
    static Token onetoken[1] = {{NUMBER, 0, 0, 1, onestr, 0}};
    static Tokenrow onetr = {onetoken, onetoken, onetoken + 1, 1};

    trp->tp = trp->bp;
    if (type == 'U')
    {
        if (trp->lp - trp->tp != 2 || trp->tp->type != NAME)
            goto syntax;
        if ((np = lookup(trp->tp, 0)) == NULL)
            return;
        np->flag &= ~ISDEFINED;
        return;
    }

    if (type == 'A')
    {
        if (trp->tp >= trp->lp || trp->tp->type != NAME)
            goto syntax;
        trp->tp->type = ARCHITECTURE;
        np = lookup(trp->tp, 1);
        np->flag |= ISARCHITECTURE;
        trp->tp += 1;
        if (trp->tp >= trp->lp || trp->tp->type == END)
        {
            np->vp = &onetr;
            return;
        }
        else
            error(FATAL, "Illegal -A argument %r", trp);
    }

    if (trp->tp >= trp->lp || trp->tp->type != NAME)
        goto syntax;
    np = lookup(trp->tp, 1);
    np->flag |= ISDEFINED;
    trp->tp += 1;
    if (trp->tp >= trp->lp || trp->tp->type == END)
    {
        np->vp = &onetr;
        return;
    }
    if (trp->tp->type != ASGN)
        goto syntax;
    trp->tp += 1;
    if ((trp->lp - 1)->type == END)
        trp->lp -= 1;
    np->vp = normtokenrow(trp);
    return;
syntax:
    error(FATAL, "Illegal -D or -U argument %r", trp);
}



/*
 * Do macro expansion in a row of tokens.
 * Flag is NULL if more input can be gathered.
 */
void
    expandrow(Tokenrow * trp, char *flag)
{
    Token * tp;
    Nlist * np;

    MacroValidatorList  validators;
    mvl_init(&validators);
    /* Sets all token-identifiers to 0 because tokens may not be initialised (never use C!) */
    tokenrow_zeroTokenIdentifiers(trp);

    if (flag)
        setsource(flag, -1, -1, "", 0);
    for (tp = trp->tp; tp < trp->lp;)
    {
        mvl_check(&validators, tp);

        if (tp->type != NAME
            || quicklook(tp->t[0], tp->len > 1 ? tp->t[1] : 0) == 0
            || (np = lookup(tp, 0)) == NULL
            || (np->flag & (ISDEFINED | ISMAC)) == 0
            || (np->flag & ISACTIVE) != 0)
        {
            tp++;
            continue;
        }
        trp->tp = tp;
        if (np->val == KDEFINED)
        {
            tp->type = DEFINED;
            if ((tp + 1) < trp->lp && (tp + 1)->type == NAME)
                (tp + 1)->type = NAME1;
            else
                if ((tp + 3) < trp->lp && (tp + 1)->type == LP
                    && (tp + 2)->type == NAME && (tp + 3)->type == RP)
                    (tp + 2)->type = NAME1;
                else
                    error(ERROR, "Incorrect syntax for `defined'");
            tp++;
            continue;
        }
        else
            if (np->val == KMACHINE)
            {
                if (((tp - 1) >= trp->bp) && ((tp - 1)->type == SHARP))
                {
                    tp->type = ARCHITECTURE;
                    if ((tp + 1) < trp->lp && (tp + 1)->type == NAME)
                        (tp + 1)->type = NAME2;
                    else
                        if ((tp + 3) < trp->lp && (tp + 1)->type == LP
                            && (tp + 2)->type == NAME && (tp + 3)->type == RP)
                            (tp + 2)->type = NAME2;
                        else
                            error(ERROR, "Incorrect syntax for `#machine'");
                }
                tp++;
                continue;
            }

        if (np->flag & ISMAC)
            builtin(trp, np->val);
        else
            expand(trp, np, &validators);
        tp = trp->tp;
    }   // end for
    if (flag)
        unsetsource();

    mvl_destruct(&validators);
}

/*
 * Expand the macro whose name is np, at token trp->tp, in the tokenrow.
 * Return trp->tp at the first token next to be expanded
 * (ordinarily the beginning of the expansion)
 * I.e.: the same position as before!
 * Only one expansion is performed, then we return to the expandrow()
 * loop and start at same position.
 */
void
    expand(Tokenrow * trp, Nlist * np, MacroValidatorList * pValidators)
{
    Tokenrow ntr;
    int ntokc, narg;
    Tokenrow *atr[NARG + 1];

    if (Mflag == 2)
    {
        if (np->ap)
            error(INFO, "Macro expansion of %t with %s(%r)", trp->tp, np->name, np->ap);
        else
            error(INFO, "Macro expansion of %t with %s", trp->tp, np->name);
    }

    copytokenrow(&ntr, np->vp);         /* copy macro value */
    if (np->ap == NULL)                 /* parameterless */
        ntokc = 1;
    else
    {
        int i;

        ntokc = gatherargs(trp, atr, &narg);
        if (narg < 0)
        {                               /* not actually a call (no '(') */
            trp->tp++;
            return;
        }
        if (narg != rowlen(np->ap))
        {
            error(ERROR, "Disagreement in number of macro arguments");
            trp->tp += ntokc;
            return;
        }

        /** If gatherargs passed a macro validating token, this token
            must become valid here.
            trp->tp+0 was checked in expandrow(), so we dont need to do it
            again here:
        */
        for (i = 1; i < ntokc; i++)
        {
            mvl_check(pValidators,trp->tp+i);
        }

        substargs(np, &ntr, atr);       /* put args into replacement */
        for (i = 0; i < narg; i++)
        {
            dofree(atr[i]->bp);
            dofree(atr[i]);
        }
    }

    doconcat(&ntr);                     /* execute ## operators */
    ntr.tp = ntr.bp;
    makespace(&ntr, trp->tp);

    tokenrow_zeroTokenIdentifiers(&ntr);
    insertrow(trp, ntokc, &ntr);

        /* add validator for just invalidated macro:
        */
    np->flag |= ISACTIVE;
    if (trp->tp != trp->lp)
    {   /* tp is a valid pointer: */
        mvl_add(pValidators,np,trp->tp);
    }
    else
    {   /* tp is == lp, therefore does not point to valid memory: */
        mvl_add(pValidators,np,0);
    }
        /* reset trp->tp to original position:
        */
    trp->tp -= ntr.lp - ntr.bp;         /* so the result will be tested for macros from the same position again */

    dofree(ntr.bp);

    return;
}

/*
 * Gather an arglist, starting in trp with tp pointing at the macro name.
 * Return total number of tokens passed, stash number of args found.
 * trp->tp is not changed relative to the tokenrow.
 */
int
    gatherargs(Tokenrow * trp, Tokenrow ** atr, int *narg)
{
    int parens = 1;
    int ntok = 0;
    Token *bp, *lp;
    Tokenrow ttr;
    int ntokp;
    int needspace;

    *narg = -1;                         /* means that there is no macro
                                         * call */
    /* look for the ( */
    for (;;)
    {
        trp->tp++;
        ntok++;
        if (trp->tp >= trp->lp)
        {
            gettokens(trp, 0);
            if ((trp->lp - 1)->type == END)
            {
                trp->lp -= 1;
                trp->tp -= ntok;
                return ntok;
            }
        }
        if (trp->tp->type == LP)
            break;
        if (trp->tp->type != NL)
            return ntok;
    }
    *narg = 0;
    ntok++;
    ntokp = ntok;
    trp->tp++;
    /* search for the terminating ), possibly extending the row */
    needspace = 0;
    while (parens > 0)
    {
        if (trp->tp >= trp->lp)
            gettokens(trp, 0);
        if (needspace)
        {
            needspace = 0;
            /* makespace(trp); [rh] */
        }
        if (trp->tp->type == END)
        {
            trp->lp -= 1;
            trp->tp -= ntok;
            error(ERROR, "EOF in macro arglist");
            return ntok;
        }
        if (trp->tp->type == NL)
        {
            trp->tp += 1;
            adjustrow(trp, -1);
            trp->tp -= 1;
            /* makespace(trp); [rh] */
            needspace = 1;
            continue;
        }
        if (trp->tp->type == LP)
            parens++;
        else
            if (trp->tp->type == RP)
                parens--;
        trp->tp++;
        ntok++;
    }
    trp->tp -= ntok;
    /* Now trp->tp won't move underneath us */
    lp = bp = trp->tp + ntokp;
    for (; parens >= 0; lp++)
    {
        if (lp->type == LP)
        {
            parens++;
            continue;
        }
        if (lp->type == RP)
            parens--;
        if (lp->type == DSHARP)
            lp->type = DSHARP1;         /* ## not special in arg */
        if ((lp->type == COMMA && parens == 0) ||
                ( parens < 0 && ((lp - 1)->type != LP)))
        {
            if (*narg >= NARG - 1)
                error(FATAL, "Sorry, too many macro arguments");
            ttr.bp = ttr.tp = bp;
            ttr.lp = lp;
            atr[(*narg)++] = normtokenrow(&ttr);
            bp = lp + 1;
        }
    }
    return ntok;
}

/*
 * substitute the argument list into the replacement string
 *  This would be simple except for ## and #
 */
void
    substargs(Nlist * np, Tokenrow * rtr, Tokenrow ** atr)
{
    Tokenrow tatr;
    Token *tp;
    int ntok, argno;

    for (rtr->tp = rtr->bp; rtr->tp < rtr->lp;)
    {
        if (rtr->tp->type == SHARP)
        {                               /* string operator */
            tp = rtr->tp;
            rtr->tp += 1;
            if ((argno = lookuparg(np, rtr->tp)) < 0)
            {
                error(ERROR, "# not followed by macro parameter");
                continue;
            }
            ntok = 1 + (int)(rtr->tp - tp);
            rtr->tp = tp;
            insertrow(rtr, ntok, stringify(atr[argno]));
            continue;
        }
        if (rtr->tp->type == NAME
            && (argno = lookuparg(np, rtr->tp)) >= 0)
        {
            if (((rtr->tp + 1) < rtr->lp && (rtr->tp + 1)->type == DSHARP)
                || (rtr->tp != rtr->bp  && (rtr->tp - 1)->type == DSHARP))
            {
                copytokenrow(&tatr, atr[argno]);
                makespace(&tatr, rtr->tp);
                insertrow(rtr, 1, &tatr);
                dofree(tatr.bp);
            }
            else
            {
                copytokenrow(&tatr, atr[argno]);
                makespace(&tatr, rtr->tp);
                expandrow(&tatr, "<macro>");
                insertrow(rtr, 1, &tatr);
                dofree(tatr.bp);
            }
            continue;
        }
        rtr->tp++;
    }
}

/*
 * Evaluate the ## operators in a tokenrow
 */
void
    doconcat(Tokenrow * trp)
{
    Token *ltp, *ntp;
    Tokenrow ntr;
    size_t len;

    for (trp->tp = trp->bp; trp->tp < trp->lp; trp->tp++)
    {
        if (trp->tp->type == DSHARP1)
            trp->tp->type = DSHARP;
        else
            if (trp->tp->type == DSHARP)
            {
                int  i;
                char tt[NCONCAT];

                ltp = trp->tp - 1;
                ntp = trp->tp + 1;

                if (ltp < trp->bp || ntp >= trp->lp)
                {
                    error(ERROR, "## occurs at border of replacement");
                    continue;
                }

                ntp = ltp;
                i   = 1;
                len = 0;

                do
                {
                    if (len + ntp->len + ntp->wslen > sizeof(tt))
                    {
                        error(ERROR, "## string concatination buffer overrun");
                        break;
                    }

                    if (ntp != trp->tp + 1)
                    {
                        strncpy((char *) tt + len, (char *) ntp->t - ntp->wslen,
                                ntp->len + ntp->wslen);
                        len += ntp->len + ntp->wslen;
                    }
                    else    // Leerzeichen um ## herum entfernen:
                    {
                        strncpy((char *) tt + len, (char *) ntp->t, ntp->len);
                        len += ntp->len;
                    }

                    ntp = trp->tp + i;
                    i++;
                }
                while (ntp < trp->lp);

                tt[len] = '\0';
                setsource("<##>", -1, -1, tt, 0);
                maketokenrow(3, &ntr);
                gettokens(&ntr, 1);
                unsetsource();
                if (ntr.bp->type == UNCLASS)
                    error(WARNING, "Bad token %r produced by ##", &ntr);
                while ((ntr.lp-1)->len == 0 && ntr.lp != ntr.bp)
                    ntr.lp--;

                doconcat(&ntr);
                trp->tp = ltp;
                makespace(&ntr, ltp);
                insertrow(trp, (int)(ntp - ltp), &ntr);
                dofree(ntr.bp);
                trp->tp--;
            }
    }
}

/*
 * tp is a potential parameter name of macro mac;
 * look it up in mac's arglist, and if found, return the
 * corresponding index in the argname array.  Return -1 if not found.
 */
int
    lookuparg(Nlist * mac, Token * tp)
{
    Token *ap;

    if (tp->type != NAME || mac->ap == NULL)
        return -1;
    for (ap = mac->ap->bp; ap < mac->ap->lp; ap++)
    {
        if (ap->len == tp->len && strncmp((char *) ap->t, (char *) tp->t, ap->len) == 0)
            return (int)(ap - mac->ap->bp);
    }
    return -1;
}

/*
 * Return a quoted version of the tokenrow (from # arg)
 */
#define STRLEN  512
Tokenrow *
    stringify(Tokenrow * vp)
{
    static Token t = {STRING, 0, 0, 0, NULL, 0};
    static Tokenrow tr = {&t, &t, &t + 1, 1};
    Token *tp;
    uchar s[STRLEN];
    uchar *sp = s, *cp;
    int i, instring;

    *sp++ = '"';
    for (tp = vp->bp; tp < vp->lp; tp++)
    {
        instring = tp->type == STRING || tp->type == CCON;
        if (sp + 2 * tp->len + tp->wslen  >= &s[STRLEN - 10])
        {
            error(ERROR, "Stringified macro arg is too long");
            break;
        }

        // Change by np 31.10.2001, #93725 - begin
        if ( tp->wslen > 0 )
            *sp++ = ' ';
        // change end.

        for (i = 0, cp = tp->t; (unsigned int)i < tp->len; i++)
        {
            if (instring && (*cp == '"' || *cp == '\\'))
                *sp++ = '\\';
            *sp++ = *cp++;
        }
    }
    *sp++ = '"';
    *sp = '\0';
    sp = s;
    t.len = strlen((char *) sp);
    t.t = newstring(sp, t.len, 0);
    return &tr;
}

/*
 * expand a builtin name
 */
void
    builtin(Tokenrow * trp, int biname)
{
    char *op;
    Token *tp;
    Source *s;

    tp = trp->tp;
    trp->tp++;
    /* need to find the real source */
    s = cursource;
    while (s && s->fd == -1)
        s = s->next;
    if (s == NULL)
        s = cursource;
    /* most are strings */
    tp->type = STRING;
    if (tp->wslen)
    {
        *outptr++ = ' ';
        tp->wslen = 1;
    }
    op = outptr;
    *op++ = '"';
    switch (biname)
    {

        case KLINENO:
            tp->type = NUMBER;
            op = outnum(op - 1, s->line);
            break;

        case KFILE:
            {
                char *src = s->filename;

                while ((*op++ = *src++) != 0)
                    if (src[-1] == '\\')
                        *op++ = '\\';
                op--;
                break;
            }

        case KDATE:
            strncpy(op, curtime + 4, 7);
            strncpy(op + 7, curtime + 20, 4);
            op += 11;
            break;

        case KTIME:
            strncpy(op, curtime + 11, 8);
            op += 8;
            break;

        default:
            error(ERROR, "cpp botch: unknown internal macro");
            return;
    }
    if (tp->type == STRING)
        *op++ = '"';
    tp->t = (uchar *) outptr;
    tp->len = op - outptr;
    outptr = op;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
