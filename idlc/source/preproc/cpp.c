/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cpp.c,v $
 * $Revision: 1.9 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "cpp.h"

#define OUTS    16384
char outbuf[OUTS];
char *outptr = outbuf;
Source *cursource;
int nerrs;
struct token nltoken = {NL, 0, 0, 1, (uchar *) "\n"};
char *curtime;
int incdepth;
int ifdepth;
int ifsatisfied[NIF];
int skipping;

char rcsid[] = "$Version 1.2 $ $Revision: 1.9 $ $Date: 2008-04-11 10:37:48 $";

int realargc;
char* realargv[512];

void checkCommandFile(char* cmdfile)
{
    FILE    *commandfile;
    char    option[256];

    commandfile = fopen(cmdfile+1, "r");
    if( commandfile == NULL )
    {
           error(FATAL, "Can't open command file %s", cmdfile);
    } else
    {
        int i=0;
        int found = 0;
        char c;
        while ( fscanf(commandfile, "%c", &c) != EOF )
        {
            if (c=='\"') {
                if (found) {
                    found=0;
                } else {
                    found=1;
                    continue;
                }
            } else {
                if (c!=13 && c!=10) {
                    if (found || c!=' ') {
                        option[i++]=c;
                        continue;
                    }
                }
                if (i==0)
                    continue;
            }
            option[i]='\0';
            found=0;
            i=0;
            if (option[0]== '@' || option[1]== '@')
            {
                checkCommandFile(option);
            } else
            {
                realargv[realargc]= strdup(option);
                realargc++;
                option[0]='\0';
            }
        }
        if (option[0] != '\0') {
            option[i]='\0';
            realargv[realargc]= strdup(option);
            realargc++;
        }
        fclose(commandfile);
    }
}

void checkCommandArgs(int argc, char **argv)
{
    int i;

    for (i=0; i<argc; i++)
    {
        if (argv[i][0]== '@')
        {
            checkCommandFile(argv[i]);
        } else
        {
            realargv[i]= strdup(argv[i]);
            realargc++;
        }
    }
}

void cleanCommandArgs()
{
    int i;

    for (i=0; i<realargc; i++)
    {
        free(realargv[i]);
    }
}

#ifdef WNT
int __cdecl main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    Tokenrow tr;
    time_t t;
    char ebuf[BUFSIZ];

    checkCommandArgs(argc, argv);

    setbuf(stderr, ebuf);
    t = time(NULL);
    curtime = ctime(&t);
    maketokenrow(3, &tr);
    expandlex();
    setup(realargc, realargv);
    /* fixlex(); */
    if (!Pflag)
        genline();
    process(&tr);
    flushout();
    fflush(stderr);
    cleanCommandArgs();
    exit(nerrs);
    //return nerrs;
}

void
    process(Tokenrow * trp)
{
    int anymacros = 0;

    for (;;)
    {
        if (trp->tp >= trp->lp)
        {
            trp->tp = trp->lp = trp->bp;
            outptr = outbuf;
            anymacros |= gettokens(trp, 1);
            trp->tp = trp->bp;
        }
        if (trp->tp->type == END)
        {
            if (--incdepth >= 0)
            {
                if (cursource->ifdepth)
                    error(ERROR,
                          "Unterminated conditional in #include");
                unsetsource();
                cursource->line += cursource->lineinc;
                trp->tp = trp->lp;
                if (!Pflag)
                    genline();
                continue;
            }
            if (ifdepth)
                error(ERROR, "Unterminated #if/#ifdef/#ifndef");
            break;
        }
        if (trp->tp->type == SHARP)
        {
            trp->tp += 1;
            control(trp);
        }
        else
            if (!skipping && anymacros)
                expandrow(trp, NULL);
        if (skipping)
            setempty(trp);
        puttokens(trp);
        anymacros = 0;
        cursource->line += cursource->lineinc;
        if (cursource->lineinc > 1)
        {
            if (!Pflag)
                genline();
        }
    }
}

void
    control(Tokenrow * trp)
{
    Nlist *np;
    Token *tp;

    tp = trp->tp;
    if (tp->type != NAME)
    {
        if (tp->type == NUMBER)
            goto kline;
        if (tp->type != NL)
            error(ERROR, "Unidentifiable control line");
        return;                         /* else empty line */
    }
    if ((np = lookup(tp, 0)) == NULL || ((np->flag & ISKW) == 0 && !skipping))
    {
        error(WARNING, "Unknown preprocessor control %t", tp);
        return;
    }
    if (skipping)
    {
        switch (np->val)
        {
            case KENDIF:
                if (--ifdepth < skipping)
                    skipping = 0;
                --cursource->ifdepth;
                setempty(trp);
                return;

            case KIFDEF:
            case KIFNDEF:
            case KIF:
                if (++ifdepth >= NIF)
                    error(FATAL, "#if too deeply nested");
                ++cursource->ifdepth;
                return;

            case KELIF:
            case KELSE:
                if (ifdepth <= skipping)
                    break;
                return;

            default:
                return;
        }
    }
    switch (np->val)
    {
        case KDEFINE:
            dodefine(trp);
            break;

        case KUNDEF:
            tp += 1;
            if (tp->type != NAME || trp->lp - trp->bp != 4)
            {
                error(ERROR, "Syntax error in #undef");
                break;
            }
            if ((np = lookup(tp, 0)) != NULL)
            {
                np->flag &= ~ISDEFINED;

                if (Mflag)
                {
                    if (np->ap)
                        error(INFO, "Macro deletion of %s(%r)", np->name, np->ap);
                    else
                        error(INFO, "Macro deletion of %s", np->name);
                }
            }
            break;

        case KPRAGMA:
        case KIDENT:
            for (tp = trp->tp - 1; ((tp->type != NL) && (tp < trp->lp)); tp++)
                tp->type = UNCLASS;
            return;

        case KIFDEF:
        case KIFNDEF:
        case KIF:
            if (++ifdepth >= NIF)
                error(FATAL, "#if too deeply nested");
            ++cursource->ifdepth;
            ifsatisfied[ifdepth] = 0;
            if (eval(trp, np->val))
                ifsatisfied[ifdepth] = 1;
            else
                skipping = ifdepth;
            break;

        case KELIF:
            if (ifdepth == 0)
            {
                error(ERROR, "#elif with no #if");
                return;
            }
            if (ifsatisfied[ifdepth] == 2)
                error(ERROR, "#elif after #else");
            if (eval(trp, np->val))
            {
                if (ifsatisfied[ifdepth])
                    skipping = ifdepth;
                else
                {
                    skipping = 0;
                    ifsatisfied[ifdepth] = 1;
                }
            }
            else
                skipping = ifdepth;
            break;

        case KELSE:
            if (ifdepth == 0 || cursource->ifdepth == 0)
            {
                error(ERROR, "#else with no #if");
                return;
            }
            if (ifsatisfied[ifdepth] == 2)
                error(ERROR, "#else after #else");
            if (trp->lp - trp->bp != 3)
                error(ERROR, "Syntax error in #else");
            skipping = ifsatisfied[ifdepth] ? ifdepth : 0;
            ifsatisfied[ifdepth] = 2;
            break;

        case KENDIF:
            if (ifdepth == 0 || cursource->ifdepth == 0)
            {
                error(ERROR, "#endif with no #if");
                return;
            }
            --ifdepth;
            --cursource->ifdepth;
            if (trp->lp - trp->bp != 3)
                error(WARNING, "Syntax error in #endif");
            break;

        case KERROR:
            trp->tp = tp + 1;
            error(WARNING, "#error directive: %r", trp);
            break;

        case KLINE:
            trp->tp = tp + 1;
            expandrow(trp, "<line>");
            tp = trp->bp + 2;
    kline:
            if (tp + 1 >= trp->lp || tp->type != NUMBER || tp + 3 < trp->lp
                || (tp + 3 == trp->lp
                    && ((tp + 1)->type != STRING || *(tp + 1)->t == 'L')))
            {
                error(ERROR, "Syntax error in #line");
                return;
            }
            cursource->line = atol((char *) tp->t) - 1;
            if (cursource->line < 0 || cursource->line >= 32768)
                error(WARNING, "#line specifies number out of range");
            tp = tp + 1;
            if (tp + 1 < trp->lp)
                cursource->filename = (char *) newstring(tp->t + 1, tp->len - 2, 0);
            return;

        case KDEFINED:
            error(ERROR, "Bad syntax for control line");
            break;

        case KIMPORT:
            doinclude(trp, -1, 1);
            trp->lp = trp->bp;
            return;

        case KINCLUDE:
            doinclude(trp, -1, 0);
            trp->lp = trp->bp;
            return;

        case KINCLUDENEXT:
            doinclude(trp, cursource->pathdepth, 0);
            trp->lp = trp->bp;
            return;

        case KEVAL:
            eval(trp, np->val);
            break;

        default:
            error(ERROR, "Preprocessor control `%t' not yet implemented", tp);
            break;
    }
    setempty(trp);
    return;
}

void *
    domalloc(int size)
{
    void *p = malloc(size);

    if (p == NULL)
        error(FATAL, "Out of memory from malloc");
    return p;
}

void
    dofree(void *p)
{
    free(p);
}

void
    error(enum errtype type, char *string,...)
{
    va_list ap;
    char c, *cp, *ep;
    Token *tp;
    Tokenrow *trp;
    Source *s;
    int i;

    fprintf(stderr, "cpp: ");
    for (s = cursource; s; s = s->next)
        if (*s->filename)
            fprintf(stderr, "%s:%d ", s->filename, s->line);
    va_start(ap, string);
    for (ep = string; *ep; ep++)
    {
        if (*ep == '%')
        {
            switch (*++ep)
            {

                case 'c':
                       /* need a cast here since va_arg only
                          takes fully promoted types */
                   c = (char) va_arg(ap, int);
                    fprintf(stderr, "%c", c);
                    break;

                case 's':
                    cp = va_arg(ap, char *);
                    fprintf(stderr, "%s", cp);
                    break;

                case 'd':
                    i = va_arg(ap, int);
                    fprintf(stderr, "%d", i);
                    break;

                case 't':
                    tp = va_arg(ap, Token *);
                    fprintf(stderr, "%.*s", tp->len, tp->t);
                    break;

                case 'r':
                    trp = va_arg(ap, Tokenrow *);
                    for (tp = trp->tp; tp < trp->lp && tp->type != NL; tp++)
                    {
                        if (tp > trp->tp && tp->wslen)
                            fputc(' ', stderr);
                        fprintf(stderr, "%.*s", tp->len, tp->t);
                    }
                    break;

                default:
                    fputc(*ep, stderr);
                    break;
            }
        }
        else
            fputc(*ep, stderr);
    }
    va_end(ap);
    fputc('\n', stderr);
    if (type == FATAL)
        exit(-1);
    if (type != WARNING)
        nerrs += 1;
    fflush(stderr);
}
