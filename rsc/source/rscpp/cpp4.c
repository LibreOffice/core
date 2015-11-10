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

#include <sal/types.h>
#include <stdio.h>
#include <ctype.h>
#include "cppdef.h"
#include "cpp.h"
/*
 * parm[], parmp, and parlist[] are used to store #define() argument
 * lists.  nargs contains the actual number of parameters stored.
 */
static char parm[NPARMWORK + 1];    /* define param work buffer     */
static char* parmp;                 /* Free space in parm           */
static char* parlist[LASTPARM];     /* -> start of each parameter   */
static int nargs;                   /* Parameters for this macro    */

void InitCpp4()
{
    int i;
    for( i = 0; i < NPARMWORK; i++ )
        parm[ i ] = 0;
    for( i = 0; i < LASTPARM; i++ )
        parlist[ i ] = NULL;

    nargs = 0;
}


/*
 * Called from control when a #define is scanned.  This module
 * parses formal parameters and the replacement string.  When
 * the formal parameter name is encountered in the replacement
 * string, it is replaced by a character in the range 128 to
 * 128+NPARAM (this allows up to 32 parameters within the
 * Dec Multinational range).
 *
 * There is some special case code to distinguish
 *      #define foo     bar
 * from #define foo()   bar
 *
 * Also, we make sure that
 *      #define foo     foo
 * expands to "foo" but doesn't put cpp into an infinite loop.
 *
 * A warning message is printed if you redefine a symbol to a
 * different text.  I.e,
 *      #define foo     123
 *      #define foo     123
 * is ok, but
 *      #define foo     123
 *      #define foo     +123
 * is not.
 *
 * The following subroutines are called from define():
 * checkparm    called when a token is scanned.  It checks through the
 *              array of formal parameters.  If a match is found, the
 *              token is replaced by a control byte which will be used
 *              to locate the parameter when the macro is expanded.
 * textput      puts a string in the macro work area (parm[]), updating
 *              parmp to point to the first free byte in parm[].
 *              textput() tests for work buffer overflow.
 * charput      puts a single character in the macro work area (parm[])
 *              in a manner analogous to textput().
 */
void dodefine()
{
    int c;
    DEFBUF* dp;            /* -> new definition    */
    int isredefine;        /* TRUE if redefined    */
    char* old = NULL;         /* Remember redefined   */

    if (type[(c = skipws())] != LET)
        goto bad_define;
    isredefine = FALSE;                     /* Set if redefining    */
    if ((dp = lookid(c)) == NULL)           /* If not known now     */
        dp = defendel(token, FALSE);        /* Save the name        */
    else                                    /* It's known:          */
    {
        isredefine = TRUE;                  /* Remember this fact   */
        old = dp->repl;                     /* Remember replacement */
        dp->repl = NULL;                    /* No replacement now   */
    }
    parlist[0] = parmp = parm;              /* Setup parm buffer    */
    if ((c = get()) == '(')                 /* With arguments?      */
    {
        nargs = 0;                          /* Init formals counter */
        do                                  /* Collect formal parms */
        {
            if (nargs >= LASTPARM)
                cfatal("Too many arguments for macro", NULLST);
            else if ((c = skipws()) == ')')
                break;                      /* Got them all         */
            else if (type[c] != LET)        /* Bad formal syntax    */
                goto bad_define;
            scanid(c);                      /* Get the formal param */
            parlist[nargs++] = parmp;       /* Save its start       */
            textput(token);                 /* Save text in parm[]  */
        }
        while ((c = skipws()) == ',');      /* Get another argument */
        if (c != ')')                       /* Must end at )        */
            goto bad_define;
        c = ' ';                            /* Will skip to body    */
    }
    else
    {
        /*
         * DEF_NOARGS is needed to distinguish between
         * "#define foo" and "#define foo()".
         */
        nargs = DEF_NOARGS;                 /* No () parameters     */
    }
    if (type[c] == SPA)                     /* At whitespace?       */
        c = skipws();                       /* Not any more.        */
    workp = work;                           /* Replacement put here */
    inmacro = TRUE;                         /* Keep \<newline> now  */
    while (c != EOF_CHAR && c != '\n')      /* Compile macro body   */
    {
        if (c == '#')                       /* Token concatenation? */
        {
            while (workp > work && type[(int)workp[-1]] == SPA)
                --workp;                    /* Erase leading spaces */
            save(TOK_SEP);                  /* Stuff a delimiter    */
            c = skipws();                   /* Eat whitespace       */
            if (type[c] == LET)             /* Another token here?  */
                ;                           /* Stuff it normally    */
            else if (type[c] == DIG)        /* Digit string after?  */
            {
                while (type[c] == DIG)      /* Stuff the digits     */
                {
                    save(c);
                    c = get();
                }
                save(TOK_SEP);              /* Delimit 2nd token    */
            }
            else
            {
                ciwarn("Strange character after # (%d.)", c);
            }
            continue;
        }
        switch (type[c])
        {
        case LET:
            checkparm(c, dp);               /* Might be a formal    */
            break;

        case DIG:                           /* Number in mac. body  */
        case DOT:                           /* Maybe a float number */
            scannumber(c, save);            /* Scan it off          */
            break;

        case QUO:                           /* String in mac. body  */
            stparmscan(c);
            break;

        case BSH:                           /* Backslash            */
            save('\\');
            if ((c = get()) == '\n')
                wrongline = TRUE;
            save(c);
            break;

        case SPA:                           /* Absorb whitespace    */
            /*
             * Note: the "end of comment" marker is passed on
             * to allow comments to separate tokens.
             */
            if (workp[-1] == ' ')           /* Absorb multiple      */
                break;                      /* spaces               */
            else if (c == '\t')
                c = ' ';                    /* Normalize tabs       */
            /* Fall through to store character                      */
        default:                            /* Other character      */
            save(c);
            break;
        }
        c = get();
    }
    inmacro = FALSE;                        /* Stop newline hack    */
    unget();                                /* For control check    */
    if (workp > work && workp[-1] == ' ')   /* Drop trailing blank  */
        workp--;
    *workp = EOS;                           /* Terminate work       */
    dp->repl = savestring(work);            /* Save the string      */
    dp->nargs = nargs;                      /* Save arg count       */
#if OSL_DEBUG_LEVEL > 1
    if (debug)
        dumpadef("macro definition", dp);
    else if (bDumpDefs)
        dumpadef(NULL, dp);
#endif
    if (isredefine)                         /* Error if redefined   */
    {
        if ((old != NULL && dp->repl != NULL && !streq(old, dp->repl)) ||
            (old == NULL && dp->repl != NULL) ||
            (old != NULL && dp->repl == NULL))
        {
#ifdef STRICT_UNDEF
            cerror("Redefining defined variable \"%s\"", dp->name);
#else
            cwarn("Redefining defined variable \"%s\"", dp->name);
#endif
        }
        if (old != NULL)                    /* We don't need the    */
            free(old);                      /* old definition now.  */
    }
    return;

  bad_define:
    cerror("#define syntax error", NULLST);
    inmacro = FALSE;                        /* Stop <newline> hack  */
}

/*
 * Replace this param if it's defined.  Note that the macro name is a
 * possible replacement token.  We stuff DEF_MAGIC in front of the token
 * which is treated as a LETTER by the token scanner and eaten by
 * the output routine.  This prevents the macro expander from
 * looping if someone writes "#define foo foo".
 */
void checkparm(int c, DEFBUF* dp)
{
    int i;
    char* cp;

    scanid(c);                              /* Get parm to token[]  */
    for (i = 0; i < nargs; i++)             /* For each argument    */
    {
        if (streq(parlist[i], token))       /* If it's known        */
        {
#ifdef SOLAR
            save(DEL);
#endif
            save(i + MAC_PARM);             /* Save a magic cookie  */
            return;                         /* And exit the search  */
        }
    }
    if (streq(dp->name, token))             /* Macro name in body?  */
        save(DEF_MAGIC);                    /* Save magic marker    */
    for (cp = token; *cp != EOS;)           /* And save             */
        save(*cp++);                        /* The token itself     */
}

/*
 * Normal string parameter scan.
 */
void stparmscan(int delim)
{
    char* wp;
    int i;

    wp = workp;                     /* Here's where it starts       */
    if (!scanstring(delim, save))
        return;                     /* Exit on scanstring error     */
    workp[-1] = EOS;                /* Erase trailing quote         */
    wp++;                           /* -> first string content byte */
    for (i = 0; i < nargs; i++)
    {
        if (streq(parlist[i], wp))
        {
#ifdef SOLAR
            *wp++ = DEL;
            *wp++ = MAC_PARM + PAR_MAC;     /* Stuff a magic marker */
            *wp++ = (char)(i + MAC_PARM);   /* Make a formal marker */
            *wp = wp[-4];                   /* Add on closing quote */
            workp = wp + 1;                 /* Reset string end     */
#else
            *wp++ = MAC_PARM + PAR_MAC;     /* Stuff a magic marker */
            *wp++ = (i + MAC_PARM);         /* Make a formal marker */
            *wp = wp[-3];                   /* Add on closing quote */
            workp = wp + 1;                 /* Reset string end     */
#endif
            return;
        }
    }
    workp[-1] = wp[-1];             /* Nope, reset end quote.       */
}

/*
 * Remove the symbol from the defined list.
 * Called from the #control processor.
 */
void doundef()
{
    int c;

    if (type[(c = skipws())] != LET)
        cerror("Illegal #undef argument", NULLST);
    else
    {
        scanid(c);                          /* Get name to token[]  */
        if (defendel(token, TRUE) == NULL)
        {
#ifdef STRICT_UNDEF
            cwarn("Symbol \"%s\" not defined in #undef", token);
#endif
        }
    }
}

/*
 * Put the string in the parm[] buffer.
 */
void textput(char* text)
{
    size_t size;

    size = strlen(text) + 1;
    if ((parmp + size) >= &parm[NPARMWORK])
        cfatal("Macro work area overflow", NULLST);
    else
    {
        strcpy(parmp, text);
        parmp += size;
    }
}

/*
 * Put the byte in the parm[] buffer.
 */
void charput(int c)
{
    if (parmp >= &parm[NPARMWORK])
        cfatal("Macro work area overflow", NULLST);
    else
    {
        *parmp++ = (char)c;
    }
}

/*
 *              M a c r o   E x p a n s i o n
 */

static DEFBUF* macro;         /* Catches start of infinite macro      */

/*
 * Expand a macro.  Called from the cpp mainline routine (via subroutine
 * macroid()) when a token is found in the symbol table.  It calls
 * expcollect() to parse actual parameters, checking for the correct number.
 * It then creates a "file" containing a single line containing the
 * macro with actual parameters inserted appropriately.  This is
 * "pushed back" onto the input stream.  (When the get() routine runs
 * off the end of the macro line, it will dismiss the macro itself.)
 */
void expand(DEFBUF* tokenp)
{
    int c;
    FILEINFO* file;

#if OSL_DEBUG_LEVEL > 1
    if (debug)
        dumpadef("expand entry", tokenp);
#endif
    /*
     * If no macro is pending, save the name of this macro
     * for an eventual error message.
     */
    if (recursion++ == 0)
        macro = tokenp;
    else if (recursion == RECURSION_LIMIT)
    {
        cerror("Recursive macro definition of \"%s\"", tokenp->name);
        fprintf(stderr, "(Defined by \"%s\")\n", macro->name);
        if (rec_recover)
        {
            do
            {
                c = get();
            }
            while (infile != NULL && infile->fp == NULL);
            unget();
            recursion = 0;
            return;
        }
    }
    /*
     * Here's a macro to expand.
     */
    nargs = 0;                              /* Formals counter      */
    parmp = parm;                           /* Setup parm buffer    */
    switch (tokenp->nargs)
    {
    case (-2):                              /* __LINE__             */
        sprintf(work, "%d", line);
        ungetstring(work);
        break;

    case (-3):                              /* __FILE__             */
        for (file = infile; file != NULL; file = file->parent)
        {
            if (file->fp != NULL)
            {
                sprintf(work, "\"%s\"", (file->progname != NULL)
                        ? file->progname : file->filename);
                ungetstring(work);
                break;
            }
        }
        break;

    default:
        /*
         * Nothing funny about this macro.
         */
        if (tokenp->nargs < 0)
            cfatal("Bug: Illegal __ macro \"%s\"", tokenp->name);
        while ((c = skipws()) == '\n')      /* Look for (, skipping */
            wrongline = TRUE;               /* spaces and newlines  */
        if (c != '(')
        {
            /*
             * If the programmer writes
             *      #define foo() ...
             *      ...
             *      foo [no ()]
             * just write foo to the output stream.
             */
            unget();
            cwarn("Macro \"%s\" needs arguments", tokenp->name);
            fputs(tokenp->name, pCppOut );
            return;
        }
        else if (expcollect())              /* Collect arguments    */
        {
            if (tokenp->nargs != nargs)     /* Should be an error?  */
            {
                cwarn("Wrong number of macro arguments for \"%s\"",
                      tokenp->name);
            }
#if OSL_DEBUG_LEVEL > 1
            if (debug)
                dumpparm("expand");
#endif
        }                           /* Collect arguments            */
    case DEF_NOARGS:                /* No parameters just stuffs    */
        expstuff(tokenp);           /* Do actual parameters         */
    }                               /* nargs switch                 */
}

/*
 * Collect the actual parameters for this macro.  TRUE if ok.
 */
FILE_LOCAL int expcollect()
{
    int c;
    int paren;                  /* For embedded ()'s    */
    for (;;)
    {
        paren = 0;                          /* Collect next arg.    */
        while ((c = skipws()) == '\n')      /* Skip over whitespace */
            wrongline = TRUE;               /* and newlines.        */
        if (c == ')')                       /* At end of all args?  */
        {
            /*
             * Note that there is a guard byte in parm[]
             * so we don't have to check for overflow here.
             */
            *parmp = EOS;                   /* Make sure terminated */
            break;                          /* Exit collection loop */
        }
        else if (nargs >= LASTPARM)
            cfatal("Too many arguments in macro expansion", NULLST);
        parlist[nargs++] = parmp;           /* At start of new arg  */
        for (;; c = cget())                 /* Collect arg's bytes  */
        {
            if (c == EOF_CHAR)
            {
                cerror("end of file within macro argument", NULLST);
                return FALSE;             /* Sorry.               */
            }
            else if (c == '\\')             /* Quote next character */
            {
                charput(c);                 /* Save the \ for later */
                charput(cget());            /* Save the next char.  */
                continue;                   /* And go get another   */
            }
            else if (type[c] == QUO)        /* Start of string?     */
            {
                scanstring(c, charput);     /* Scan it off          */
                continue;                   /* Go get next char     */
            }
            else if (c == '(')              /* Worry about balance  */
                paren++;                    /* To know about commas */
            else if (c == ')')              /* Other side too       */
            {
                if (paren == 0)             /* At the end?          */
                {
                    unget();                /* Look at it later     */
                    break;                  /* Exit arg getter.     */
                }
                paren--;                    /* More to come.        */
            }
            else if (c == ',' && paren == 0) /* Comma delimits args */
                break;
            else if (c == '\n')             /* Newline inside arg?  */
                wrongline = TRUE;           /* We'll need a #line   */
            charput(c);                     /* Store this one       */
        }                                   /* Collect an argument  */
        charput(EOS);                       /* Terminate argument   */
#if OSL_DEBUG_LEVEL > 1
        if (debug)
            fprintf( pCppOut, "parm[%d] = \"%s\"\n", nargs, parlist[nargs - 1]);
#endif
    }                                       /* Collect all args.    */
    return TRUE;                            /* Normal return        */
}

/*
 * Stuff the macro body, replacing formal parameters by actual parameters.
 */
FILE_LOCAL void expstuff(DEFBUF* tokenp)
{
    int c;                      /* Current character    */
    char* inp;                  /* -> repl string       */
    char* defp;                 /* -> macro output buff */
    size_t size;                   /* Actual parm. size    */
    char* defend;               /* -> output buff end   */
    int string_magic;           /* String formal hack   */
    FILEINFO* file;             /* Funny #include       */

    file = getfile(NBUFF, tokenp->name);
    inp = tokenp->repl;                     /* -> macro replacement */
    defp = file->buffer;                    /* -> output buffer     */
    defend = defp + (NBUFF - 1);            /* Note its end         */
    if (inp != NULL)
    {
        while ((c = (*inp++ & 0xFF)) != EOS)
        {
#ifdef SOLAR
            if (c == DEL)
            {
                c = (*inp++ & 0xFF);
#else
                if (c >= MAC_PARM && c <= (MAC_PARM + PAR_MAC))
                {
#endif
                    string_magic = (c == (MAC_PARM + PAR_MAC));
                    if (string_magic)
                        c = (*inp++ & 0xFF);
                    /*
                     * Replace formal parameter by actual parameter string.
                     */
                    if ((c -= MAC_PARM) < nargs)
                    {
                        size = strlen(parlist[c]);
                        if ((defp + size) >= defend)
                            goto nospace;
                        /*
                         * Erase the extra set of quotes.
                         */
                        if (string_magic && defp[-1] == parlist[c][0])
                        {
                            strcpy(defp-1, parlist[c]);
                            defp += (size - 2);
                        }
                        else
                        {
                            strcpy(defp, parlist[c]);
                            defp += size;
                        }
                    }
                }
                else if (defp >= defend)
                {
                  nospace:
                    cfatal("Out of space in macro \"%s\" arg expansion",
                           tokenp->name);
                }
                else
                {
                    *defp++ = (char)c;
                }
            }
        }
        *defp = EOS;
#if OSL_DEBUG_LEVEL > 1
        if (debug > 1)
            fprintf( pCppOut, "macroline: \"%s\"\n", file->buffer);
#endif
    }

#if OSL_DEBUG_LEVEL > 1

    /*
     * Dump parameter list.
     */
    void dumpparm(char* why)
    {
        int    i;

        fprintf( pCppOut, "dump of %d parameters (%" SAL_PRI_SIZET "u bytes total) %s\n",
                 nargs, parmp - parm, why);
        for (i = 0; i < nargs; i++)
        {
            fprintf( pCppOut, "parm[%d] (%d) = \"%s\"\n",
                     i + 1, (int)strlen(parlist[i]), parlist[i]);
        }
    }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
