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
#include <ctype.h>
#include "cppdef.h"
#include "cpp.h"
#if HOST == SYS_VMS
/*
 * Include the rms stuff.  (We can't just include rms.h as it uses the
 * VaxC-specific library include syntax that Decus CPP doesn't support.
 * By including things by hand, we can CPP ourself.)
 */
#include <nam.h>
#include <fab.h>
#include <rab.h>
#include <rmsdef.h>
#endif

/*
 * Generate (by hand-inspection) a set of unique values for each control
 * operator.  Note that this is not guaranteed to work for non-Ascii
 * machines.  CPP won't compile if there are hash conflicts.
 */

#define L_assert        ('a' + ('s' << 1))
#define L_define        ('d' + ('f' << 1))
#define L_elif          ('e' + ('i' << 1))
#define L_else          ('e' + ('s' << 1))
#define L_endif         ('e' + ('d' << 1))
#define L_if            ('i' + (EOS << 1))
#define L_ifdef         ('i' + ('d' << 1))
#define L_ifndef        ('i' + ('n' << 1))
#define L_include       ('i' + ('c' << 1))
#define L_line          ('l' + ('n' << 1))
#define L_nogood        (EOS + (EOS << 1))      /* To catch #i          */
#define L_pragma        ('p' + ('a' << 1))
#define L_undef         ('u' + ('d' << 1))
#define L_error         ('e' + ('r' << 1))      /* BP 5.3.92, #error */
#if OSL_DEBUG_LEVEL > 1
#define L_debug         ('d' + ('b' << 1))      /* #debug               */
#define L_nodebug       ('n' + ('d' << 1))      /* #nodebug             */
#endif


void InitCpp2()
{

}


int
control(int counter)
/*
 * Process #control lines.  Simple commands are processed inline,
 * while complex commands have their own subroutines.
 *
 * The counter is used to force out a newline before #line, and
 * #pragma commands.  This prevents these commands from ending up at
 * the end of the previous line if cpp is invoked with the -C option.
 */
{
        register int            c;
        register char           *tp;
        register int            hash;
        char                    *ep;

        c = skipws();
        if (c == '\n' || c == EOF_CHAR)
            return (counter + 1);
        if (!isdigit(c))
            scanid(c);                  /* Get #word to token[]         */
        else {
            unget();                    /* Hack -- allow #123 as a      */
            strcpy(token, "line");      /* synonym for #line 123        */
        }
        hash = (token[1] == EOS) ? L_nogood : (token[0] + (token[2] << 1));
        switch (hash) {
        case L_assert:  tp = "assert";          break;
        case L_define:  tp = "define";          break;
        case L_elif:    tp = "elif";            break;
        case L_else:    tp = "else";            break;
        case L_endif:   tp = "endif";           break;
        case L_if:      tp = "if";              break;
        case L_ifdef:   tp = "ifdef";           break;
        case L_ifndef:  tp = "ifndef";          break;
        case L_include: tp = "include";         break;
        case L_line:    tp = "line";            break;
        case L_pragma:  tp = "pragma";          break;
        case L_undef:   tp = "undef";           break;
        case L_error:   tp = "error";           break;
#if OSL_DEBUG_LEVEL > 1
        case L_debug:   tp = "debug";           break;
        case L_nodebug: tp = "nodebug";         break;
#endif
        default:        hash = L_nogood;
        case L_nogood:  tp = "";                break;
        }
        if (!streq(tp, token))
            hash = L_nogood;
        /*
         * hash is set to a unique value corresponding to the
         * control keyword (or L_nogood if we think it's nonsense).
         */
        if (infile->fp == NULL)
            cwarn("Control line \"%s\" within macro expansion", token);
        if (!compiling) {                       /* Not compiling now    */
            switch (hash) {
            case L_if:                          /* These can't turn     */
            case L_ifdef:                       /*  compilation on, but */
            case L_ifndef:                      /*   we must nest #if's */
                if (++ifptr >= &ifstack[BLK_NEST])
                    goto if_nest_err;
                *ifptr = 0;                     /* !WAS_COMPILING       */
            case L_line:                        /* Many                 */
            /*
             * Are pragma's always processed?
             */
            case L_pragma:                      /*  options             */
            case L_include:                     /*   are uninteresting  */
            case L_define:                      /*    if we             */
            case L_undef:                       /*     aren't           */
            case L_assert:                      /*      compiling.      */
            case L_error:                       /* BP 5.3.92, #error */
dump_line:      skipnl();                       /* Ignore rest of line  */
                return (counter + 1);
            }
        }
        /*
         * Make sure that #line and #pragma are output on a fresh line.
         */
        if (counter > 0 && (hash == L_line || hash == L_pragma)) {
            PUTCHAR('\n');
            counter--;
        }
        switch (hash) {
        case L_line:
            /*
             * Parse the line to update the line number and "progname"
             * field and line number for the next input line.
             * Set wrongline to force it out later.
             */
            c = skipws();
            workp = work;                       /* Save name in work    */
            while (c != '\n' && c != EOF_CHAR) {
                save(c);
                c = get();
            }
            unget();
            save(EOS);
            /*
             * Split #line argument into <line-number> and <name>
             * We subtract 1 as we want the number of the next line.
             */
            line = atoi(work) - 1;              /* Reset line number    */
            for (tp = work; isdigit(*tp) || type[(int)*tp] == SPA; tp++)
                ;                               /* Skip over digits     */
            if (*tp != EOS) {                   /* Got a filename, so:  */
                if (*tp == '"' && (ep = strrchr(tp + 1, '"')) != NULL) {
                    tp++;                       /* Skip over left quote */
                    *ep = EOS;                  /* And ignore right one */
                }
                if (infile->progname != NULL)   /* Give up the old name */
                    free(infile->progname);     /* if it's allocated.   */
                infile->progname = savestring(tp);
            }
            wrongline = TRUE;                   /* Force output later   */
            break;

        case L_include:
            doinclude();
            break;

        case L_define:
            dodefine();
            break;

        case L_undef:
            doundef();
            break;

        case L_else:
            if (ifptr == &ifstack[0])
                goto nest_err;
            else if ((*ifptr & ELSE_SEEN) != 0)
                goto else_seen_err;
            *ifptr |= ELSE_SEEN;
            if ((*ifptr & WAS_COMPILING) != 0) {
                if (compiling || (*ifptr & TRUE_SEEN) != 0)
                    compiling = FALSE;
                else {
                    compiling = TRUE;
                }
            }
            break;

        case L_elif:
            if (ifptr == &ifstack[0])
                goto nest_err;
            else if ((*ifptr & ELSE_SEEN) != 0) {
else_seen_err:  cerror("#%s may not follow #else", token);
                goto dump_line;
            }
            if ((*ifptr & (WAS_COMPILING | TRUE_SEEN)) != WAS_COMPILING) {
                compiling = FALSE;              /* Done compiling stuff */
                goto dump_line;                 /* Skip this clause     */
            }
            doif(L_if);
            break;

        case L_if:
        case L_ifdef:
        case L_ifndef:
            if (++ifptr >= &ifstack[BLK_NEST])
if_nest_err:    cfatal("Too many nested #%s statements", token);
            *ifptr = WAS_COMPILING;
            doif(hash);
            break;

        case L_endif:
            if (ifptr == &ifstack[0]) {
nest_err:       cerror("#%s must be in an #if", token);
                goto dump_line;
            }
            if (!compiling && (*ifptr & WAS_COMPILING) != 0)
                wrongline = TRUE;
            compiling = ((*ifptr & WAS_COMPILING) != 0);
            --ifptr;
            break;

        case L_assert:
            if (eval() == 0)
                cerror("Preprocessor assertion failure", NULLST);
            break;

        case L_pragma:
            /*
             * #pragma is provided to pass "options" to later
             * passes of the compiler.  cpp doesn't have any yet.
             */
            fprintf( pCppOut, "#pragma ");
            while ((c = get()) != '\n' && c != EOF_CHAR)
                cput(c);
            unget();
            break;

#if OSL_DEBUG_LEVEL > 1
        case L_debug:
            if (debug == 0)
                dumpdef("debug set on");
            debug++;
            break;

        case L_nodebug:
            debug--;
            break;
#endif
        case L_error:                       /* BP 5.3.92, #error */
        {
            fprintf( pCppOut, "cpp: line %u, Error directive: ", line );
            while ((c = get()) != '\n' && c != EOF_CHAR)
                cput(c);
            fprintf( pCppOut, "\n" );
            exit( 1 );
            break;
        }
        default:
            /*
             * Undefined #control keyword.
             * Note: the correct behavior may be to warn and
             * pass the line to a subsequent compiler pass.
             * This would allow #asm or similar extensions.
             */
            cerror("Illegal # command \"%s\"", token);
            break;
        }
        if (hash != L_include) {
#if OLD_PREPROCESSOR
            /*
             * Ignore the rest of the #control line so you can write
             *          #if     foo
             *          #endif  foo
             */
            goto dump_line;                     /* Take common exit     */
#else
            if (skipws() != '\n') {
                cwarn("Unexpected text in #control line ignored", NULLST);
                skipnl();
            }
#endif
        }
        return (counter + 1);
}

FILE_LOCAL
void doif(int hash)
/*
 * Process an #if, #ifdef, or #ifndef.  The latter two are straightforward,
 * while #if needs a subroutine of its own to evaluate the expression.
 *
 * doif() is called only if compiling is TRUE.  If false, compilation
 * is always supressed, so we don't need to evaluate anything.  This
 * supresses unnecessary warnings.
 */
{
        register int            c;
        register int            found;

        if ((c = skipws()) == '\n' || c == EOF_CHAR) {
            unget();
            goto badif;
        }
        if (hash == L_if) {
            unget();
            found = (eval() != 0);      /* Evaluate expr, != 0 is  TRUE */
            hash = L_ifdef;             /* #if is now like #ifdef       */
        }
        else {
            if (type[c] != LET)         /* Next non-blank isn't letter  */
                goto badif;             /* ... is an error              */
            found = (lookid(c) != NULL); /* Look for it in symbol table */
        }
        if (found == (hash == L_ifdef)) {
            compiling = TRUE;
            *ifptr |= TRUE_SEEN;
        }
        else {
            compiling = FALSE;
        }
        return;

badif:  cerror("#if, #ifdef, or #ifndef without an argument", NULLST);
#if !OLD_PREPROCESSOR
        skipnl();                               /* Prevent an extra     */
        unget();                                /* Error message        */
#endif
        return;
}

FILE_LOCAL
void doinclude()
/*
 * Process the #include control line.
 * There are three variations:
 *      #include "file"         search somewhere relative to the
 *                              current source file, if not found,
 *                              treat as #include <file>.
 *      #include <file>         Search in an implementation-dependent
 *                              list of places.
 *      #include token          Expand the token, it must be one of
 *                              "file" or <file>, process as such.
 *
 * Note: the November 12 draft forbids '>' in the #include <file> format.
 * This restriction is unnecessary and not implemented.
 */
{
        register int            c;
        register int            delim;
#if HOST == SYS_VMS
        char                    def_filename[NAM$C_MAXRSS + 1];
#endif

        delim = macroid(skipws());
        if (delim != '<' && delim != '"')
            goto incerr;
        if (delim == '<')
            delim = '>';
        workp = work;
        instring = TRUE;                /* Accept all characters        */
#ifdef CONTROL_COMMENTS_NOT_ALLOWED
        while ((c = get()) != '\n' && c != EOF_CHAR)
            save(c);                    /* Put it away.                 */
        unget();                        /* Force nl after includee      */
        /*
         * The draft is unclear if the following should be done.
         */
        while (--workp >= work && *workp == ' ')
            ;                           /* Trim blanks from filename    */
        if (*workp != delim)
            goto incerr;
#else
        while ((c = get()) != delim && c != EOF_CHAR)
            save(c);
#endif
        *workp = EOS;                   /* Terminate filename           */
        instring = FALSE;
#if HOST == SYS_VMS
        /*
         * Assume the default .h filetype.
         */
        if (!vmsparse(work, ".H", def_filename)) {
            perror(work);               /* Oops.                        */
            goto incerr;
        }
        else if (openinclude(def_filename, (delim == '"')))
            return;
#else
        if (openinclude(work, (delim == '"')))
            return;
#endif
        /*
         * No sense continuing if #include file isn't there.
         */
        cfatal("Cannot open include file \"%s\"", work);

incerr: cerror("#include syntax error", NULLST);
        return;
}

FILE_LOCAL int
openinclude(char* filename, int searchlocal)
/*
 * Actually open an include file.  This routine is only called from
 * doinclude() above, but was written as a separate subroutine for
 * programmer convenience.  It searches the list of directories
 * and actually opens the file, linking it into the list of
 * active files.  Returns TRUE if the file was opened, FALSE
 * if openinclude() fails.  No error message is printed.
 */
{
        register char           **incptr;
#if HOST == SYS_VMS
#if NFWORK < (NAM$C_MAXRSS + 1)
    << error, NFWORK is not greater than NAM$C_MAXRSS >>
#endif
#endif
        char                    tmpname[NFWORK]; /* Filename work area   */

        if (searchlocal) {
            /*
             * Look in local directory first
             */
#if HOST == SYS_UNIX
            /*
             * Try to open filename relative to the directory of the current
             * source file (as opposed to the current directory). (ARF, SCK).
             */
            if (filename[0] != '/'
             && hasdirectory(infile->filename, tmpname))
                strcat(tmpname, filename);
            else {
                strcpy(tmpname, filename);
            }
#else
            if (!hasdirectory(filename, tmpname)
             && hasdirectory(infile->filename, tmpname))
                strcat(tmpname, filename);
            else {
                strcpy(tmpname, filename);
            }
#endif
            if (openfile(tmpname))
                return (TRUE);
        }
        /*
         * Look in any directories specified by -I command line
         * arguments, then in the builtin search list.
         */
        for (incptr = incdir; incptr < incend; incptr++) {
            if (strlen(*incptr) + strlen(filename) >= (NFWORK - 1))
                cfatal("Filename work buffer overflow", NULLST);
            else {
#if HOST == SYS_UNIX
                if (filename[0] == '/')
                    strcpy(tmpname, filename);
                else {
                    sprintf(tmpname, "%s/%s", *incptr, filename);
                }
#elif HOST == SYS_UNKNOWN
                if (filename[0] == '\\')
                    strcpy(tmpname, filename);
                else {
                    sprintf(tmpname, "%s\\%s", *incptr, filename);
                }
#else
                if (!hasdirectory(filename, tmpname))
                    sprintf(tmpname, "%s%s", *incptr, filename);
#endif
                if (openfile(tmpname))
                    return (TRUE);
            }
        }
        return (FALSE);
}

FILE_LOCAL int
hasdirectory(char* source, char* result)
/*
 * If a device or directory is found in the source filename string, the
 * node/device/directory part of the string is copied to result and
 * hasdirectory returns TRUE.  Else, nothing is copied and it returns FALSE.
 */
{
#if HOST == SYS_UNIX
        register char           *tp;

        if ((tp = strrchr(source, '/')) == NULL)
            return (FALSE);
        else {
            strncpy(result, source, tp - source + 1);
            result[tp - source + 1] = EOS;
            return (TRUE);
        }
#else
#if HOST == SYS_VMS
        if (vmsparse(source, NULLST, result)
         && result[0] != EOS)
            return (TRUE);
        else {
            return (FALSE);
        }
#else
        /*
         * Random DEC operating system (RSX, RT11, RSTS/E)
         */
        register char           *tp;

        if ((tp = strrchr(source, ']')) == NULL
         && (tp = strrchr(source, ':')) == NULL)
            return (FALSE);
        else {
            strncpy(result, source, tp - source + 1);
            result[tp - source + 1] = EOS;
            return (TRUE);
        }
#endif
#endif
}

#if HOST == SYS_VMS

/*
 * EXP_DEV is set if a device was specified, EXP_DIR if a directory
 * is specified.  (Both set indicate a file-logical, but EXP_DEV
 * would be set by itself if you are reading, say, SYS$INPUT:)
 */
#define DEVDIR (NAM$M_EXP_DEV | NAM$M_EXP_DIR)

FILE_LOCAL int
vmsparse(source, defstring, result)
char            *source;
char            *defstring;     /* non-NULL -> default string.          */
char            *result;        /* Size is at least NAM$C_MAXRSS + 1    */
/*
 * Parse the source string, applying the default (properly, using
 * the system parse routine), storing it in result.
 * TRUE if it parsed, FALSE on error.
 *
 * If defstring is NULL, there are no defaults and result gets
 * (just) the node::[directory] part of the string (possibly "")
 */
{
        struct FAB      fab = cc$rms_fab;       /* File access block    */
        struct NAM      nam = cc$rms_nam;       /* File name block      */
        char            fullname[NAM$C_MAXRSS + 1];
        register char   *rp;                    /* Result pointer       */

        fab.fab$l_nam = &nam;                   /* fab -> nam           */
        fab.fab$l_fna = source;                 /* Source filename      */
        fab.fab$b_fns = strlen(source);         /* Size of source       */
        fab.fab$l_dna = defstring;              /* Default string       */
        if (defstring != NULLST)
            fab.fab$b_dns = strlen(defstring);  /* Size of default      */
        nam.nam$l_esa = fullname;               /* Expanded filename    */
        nam.nam$b_ess = NAM$C_MAXRSS;           /* Expanded name size   */
        if (sys$parse(&fab) == RMS$_NORMAL) {   /* Parse away           */
            fullname[nam.nam$b_esl] = EOS;      /* Terminate string     */
            result[0] = EOS;                    /* Just in case         */
            rp = &result[0];
            /*
             * Remove stuff added implicitly, accepting node names and
             * dev:[directory] strings (but not process-permanent files).
             */
            if ((nam.nam$l_fnb & NAM$M_PPF) == 0) {
                if ((nam.nam$l_fnb & NAM$M_NODE) != 0) {
                    strncpy(result, nam.nam$l_node, nam.nam$b_node);
                    rp += nam.nam$b_node;
                    *rp = EOS;
                }
                if ((nam.nam$l_fnb & DEVDIR) == DEVDIR) {
                    strncpy(rp, nam.nam$l_dev, nam.nam$b_dev + nam.nam$b_dir);
                    rp += nam.nam$b_dev + nam.nam$b_dir;
                    *rp = EOS;
                }
            }
            if (defstring != NULLST) {
                strncpy(rp, nam.nam$l_name, nam.nam$b_name + nam.nam$b_type);
                rp += nam.nam$b_name + nam.nam$b_type;
                *rp = EOS;
                if ((nam.nam$l_fnb & NAM$M_EXP_VER) != 0) {
                    strncpy(rp, nam.nam$l_ver, nam.nam$b_ver);
                    rp[nam.nam$b_ver] = EOS;
                }
            }
            return (TRUE);
        }
        return (FALSE);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
