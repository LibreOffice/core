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

#define NOMAIN

#include <stdio.h>
#include <ctype.h>
#include "cppdef.h"
#include "cpp.h"

FILE* pCppOut = NULL;
FILE* pCppIn  = NULL;

#if OSL_DEBUG_LEVEL > 1
FILE* pDefOut = NULL;       /* ER  evtl. #define's dump */
#endif

#ifdef B200
/* einzige Moeglichkeit unter BC Stack und Heap festzusetzen */
extern unsigned _stklen  = 24000;
extern unsigned _heaplen = 30000;
#endif



/*
 * Commonly used global variables:
 * line         is the current input line number.
 * wrongline    is set in many places when the actual output
 *              line is out of sync with the numbering, e.g,
 *              when expanding a macro with an embedded newline.
 *
 * token        holds the last identifier scanned (which might
 *              be a candidate for macro expansion).
 * errors       is the running cpp error counter.
 * infile       is the head of a linked list of input files (extended by
 *              #include and macros being expanded).  infile always points
 *              to the current file/macro.  infile->parent to the includer,
 *              etc.  infile->fd is NULL if this input stream is a macro.
 */
int line;                   /* Current line number          */
int wrongline;              /* Force #line to compiler      */
char token[IDMAX + 1];      /* Current input token          */
int errors;                 /* cpp error counter            */
FILEINFO* infile = NULL;    /* Current input file           */
#if OSL_DEBUG_LEVEL > 1
int debug;                  /* TRUE if debugging now        */
int bDumpDefs;              /* TRUE if #define's dump req.  */
#ifdef EVALDEFS
int bIsInEval;              /* TRUE if #define eval now     */
char EvalBuf[NEVALBUF + 1]; /* evaluation buffer            */
int nEvalOff = 0;           /* offset to free buffer pos    */
#endif
#endif
/*
 * This counter is incremented when a macro expansion is initiated.
 * If it exceeds a built-in value, the expansion stops -- this tests
 * for a runaway condition:
 *      #define X Y
 *      #define Y X
 *      X
 * This can be disabled by falsifying rec_recover.  (Nothing does this
 * currently: it is a hook for an eventual invocation flag.)
 */
int recursion;              /* Infinite recursion counter   */
int rec_recover = TRUE;     /* Unwind recursive macros      */

/*
 * instring is set TRUE when a string is scanned.  It modifies the
 * behavior of the "get next character" routine, causing all characters
 * to be passed to the caller (except <DEF_MAGIC>).  Note especially that
 * comments and \<newline> are not removed from the source.  (This
 * prevents cpp output lines from being arbitrarily long).
 *
 * inmacro is set by #define -- it absorbs comments and converts
 * form-feed and vertical-tab to space, but returns \<newline>
 * to the caller.  Strictly speaking, this is a bug as \<newline>
 * shouldn't delimit tokens, but we'll worry about that some other
 * time -- it is more important to prevent infinitly long output lines.
 *
 * instring and inmacro are parameters to the get() routine which
 * were made global for speed.
 */
int instring = FALSE;       /* TRUE if scanning string      */
int inmacro = FALSE;        /* TRUE if #defining a macro    */

/*
 * work[] and workp are used to store one piece of text in a temporary
 * buffer.  To initialize storage, set workp = work.  To store one
 * character, call save(c);  (This will fatally exit if there isn't
 * room.)  To terminate the string, call save(EOS).  Note that
 * the work buffer is used by several subroutines -- be sure your
 * data won't be overwritten.  The extra byte in the allocation is
 * needed for string formal replacement.
 */
char work[NWORK + 1];        /* Work buffer                  */
char* workp;                 /* Work buffer pointer          */

/*
 * keepcomments is set TRUE by the -C option.  If TRUE, comments
 * are written directly to the output stream.  This is needed if
 * the output from cpp is to be passed to lint (which uses commands
 * embedded in comments).  cflag contains the permanent state of the
 * -C flag.  keepcomments is always falsified when processing #control
 * commands and when compilation is suppressed by a false #if
 *
 * If eflag is set, CPP returns "success" even if non-fatal errors
 * were detected.
 *
 * If nflag is non-zero, no symbols are predefined except __LINE__.
 * __FILE__, and __DATE__.  If nflag > 1, absolutely no symbols
 * are predefined.
 */
int keepcomments = FALSE;   /* Write out comments flag      */
int cflag = FALSE;          /* -C option (keep comments)    */
int eflag = FALSE;          /* -E option (never fail)       */
int nflag = 0;              /* -N option (no predefines)    */

/*
 * ifstack[] holds information about nested #if's.  It is always
 * accessed via *ifptr.  The information is as follows:
 *      WAS_COMPILING   state of compiling flag at outer level.
 *      ELSE_SEEN       set TRUE when #else seen to prevent 2nd #else.
 *      TRUE_SEEN       set TRUE when #if or #elif succeeds
 * ifstack[0] holds the compiling flag.  It is TRUE if compilation
 * is currently enabled.  Note that this must be initialized TRUE.
 */
char ifstack[BLK_NEST] = { TRUE };   /* #if information      */
char* ifptr = ifstack;               /* -> current ifstack[] */

/*
 * incdir[] stores the -i directories (and the system-specific
 * #include <...> directories.
 */
char* incdir[NINCLUDE];              /* -i directories               */
char** incend = incdir;              /* -> free space in incdir[]    */

/*
 * This is the table used to predefine target machine and operating
 * system designators.  It may need hacking for specific circumstances.
 * Note: it is not clear that this is part of the Ansi Standard.
 * The -N option suppresses preset definitions.
 */
char* preset[] =
{                   /* names defined at cpp start   */
#ifdef  MACHINE
        MACHINE,
#endif
#ifdef  SYSTEM
        SYSTEM,
#endif
#ifdef  COMPILER
        COMPILER,
#endif
#if OSL_DEBUG_LEVEL > 1
        "decus_cpp",                    /* Ourselves!                   */
#endif
        NULL                            /* Must be last                 */
};

/*
 * The value of these predefined symbols must be recomputed whenever
 * they are evaluated.  The order must not be changed.
 */
char* magic[] =
{                    /* Note: order is important     */
        "__LINE__",
        "__FILE__",
        NULL                            /* Must be last                 */
};

static char* sharpfilename = NULL;

int nRunde = 0;

void InitCpp1()
{
    int i;
    /* in der LIB-Version muessen alle Variablen initialisiert werden */

    line = wrongline = errors = recursion = 0;
    for( i = 0; i < IDMAX; i++ )
        token[ i ] = 0;

    for( i = 0; i < NWORK; i++ )
        work[ i ] = 0;

    for( i = 0; i < NINCLUDE; i++ )
        incdir[ i ] = NULL;

    workp = NULL;
    for( i = 0; i < BLK_NEST; i++ )
        ifstack[ i ] = TRUE;
    ifptr = ifstack;

    pCppOut = stdout;
    pCppIn  = stdin;
#if OSL_DEBUG_LEVEL > 1
    debug = 0;
    bDumpDefs = 0;
    pDefOut = stdout;
#ifdef EVALDEFS
    bIsInEval = 0;
    for( i = 0; i < NEVALBUF; i++ )
        EvalBuf[ i ] = 0;
    nEvalOff = 0;
#endif
#endif
    rec_recover = TRUE;
    infile = NULL;
    instring = inmacro = keepcomments = cflag = eflag = FALSE;
    nflag = 0;
    incend = incdir;
    sharpfilename = NULL;
}

int MAIN(int argc, char** argv)
{
    int    i;
    char** useargv = NULL;
    char** pfargv = NULL;

    if( nRunde == 0 )
    {
        pCppIn = stdin;
        pCppOut = stdout;
    }

    nRunde++;
    InitCpp1();
    InitCpp4();
    InitCpp6();

    initdefines();                          /* O.S. specific def's  */
    if ( argv[argc-1][0] == '@' )
    {
        i = readoptions( argv[1], &pfargv );    /* Command file */
        useargv=pfargv;
    }
    else
    {
        i = dooptions(argc, argv);              /* Command line -flags  */
        useargv=argv;
    }
    switch (i)
    {
#if OSL_DEBUG_LEVEL > 1
    case 4:
        if ( bDumpDefs )
        {
            /*
             * Get defBase file, "-" means use stdout.
             */
            if (!streq(useargv[3], "-"))
            {
                pDefOut = fopen( useargv[3], "w" );
                if( pDefOut == NULL )
                {
                    perror(useargv[3]);
                    cerror("Can't open output file \"%s\"", useargv[3]);
                    exit(IO_ERROR);
                }
            }                           /* Continue by opening output    */
        }
#endif
    case 3:
        /*
         * Get output file, "-" means use stdout.
         */
        if (!streq(useargv[2], "-"))
        {
            pCppOut = fopen( useargv[2], "w" );
            if( pCppOut == NULL )
            {
                perror(useargv[2]);
                cerror("Can't open output file \"%s\"", useargv[2]);
                exit(IO_ERROR);
            }
        }                           /* Continue by opening input    */
    case 2:                         /* One file -> stdin            */
        /*
         * Open input file, "-" means use stdin.
         */
        if (!streq(useargv[1], "-"))
        {
            pCppIn = fopen( useargv[1], "r" );
            if( pCppIn == NULL)
            {
                perror(useargv[1]);
                cerror("Can't open input file \"%s\"", useargv[1]);
                exit(IO_ERROR);
            }
            strncpy(work, useargv[1], NWORK);  /* Remember input filename      */
            break;
        }                           /* Else, just get stdin         */
    case 0:                         /* No args?                     */
    case 1:                         /* No files, stdin -> stdout    */
        work[0] = EOS;              /* Unix can't find stdin name   */
        break;

    default:
        exit(IO_ERROR);             /* Can't happen                 */
    }

    setincdirs();                   /* Setup -I include directories */
    addfile( pCppIn, work);           /* "open" main input file       */
#if OSL_DEBUG_LEVEL > 1
    if (debug > 0 || bDumpDefs)
        dumpdef("preset #define symbols");
#endif
    if( pCppIn != stdin )
        rewind( pCppIn );

    cppmain();                      /* Process main file            */

    if ((i = (ifptr - &ifstack[0])) != 0)
    {
        cierror("Inside #ifdef block at end of input, depth = %d", i);
    }
#if OSL_DEBUG_LEVEL > 1
    if( pDefOut != stdout && pDefOut != stderr )
        fclose( pDefOut );
#endif
    if( pCppOut != stdout && pCppOut != stderr )
        fclose( pCppOut );

    if (errors > 0)
    {
        fprintf(stderr, (errors == 1)
                ? "%d error in preprocessor\n"
                : "%d errors in preprocessor\n", errors);
        if (!eflag)
            exit(IO_ERROR);
    }
    if( pfargv )
        free(pfargv);
    return IO_NORMAL;

}

/*
 * Main process for cpp -- copies tokens from the current input
 * stream (main file, include file, or a macro) to the output
 * file.
 */
void cppmain()
{
    int c;              /* Current character    */
    int counter;        /* newlines and spaces  */

    /*
     * Explicitly output a #line at the start of cpp output so
     * that lint (etc.) knows the name of the original source
     * file.  If we don't do this explicitly, we may get
     * the name of the first #include file instead.
     * We also seem to need a blank line following that first #line.
     */
#ifdef EVALDEFS
    if ( !bIsInEval )
#endif
    {
        sharp();
        PUTCHAR('\n');
    }
    /*
     * This loop is started "from the top" at the beginning of each line
     * wrongline is set TRUE in many places if it is necessary to write
     * a #line record.  (But we don't write them when expanding macros.)
     *
     * The counter variable has two different uses:  at
     * the start of a line, it counts the number of blank lines that
     * have been skipped over.  These are then either output via
     * #line records or by outputting explicit blank lines.
     * When expanding tokens within a line, the counter remembers
     * whether a blank/tab has been output.  These are dropped
     * at the end of the line, and replaced by a single blank
     * within lines.
     */
    for (;;)
    {
        counter = 0;                        /* Count empty lines    */
        for (;;)
        {                                   /* For each line, ...   */
            while (type[(c = get())] == SPA) /* Skip leading blanks */
                ;                           /* in this line.        */
            if (c == '\n')                  /* If line's all blank, */
                ++counter;                  /* Do nothing now       */
            else if (c == '#')
            {            /* Is 1st non-space '#' */
                keepcomments = FALSE;       /* Don't pass comments  */
                counter = control(counter); /* Yes, do a #command   */
                keepcomments = (cflag && compiling);
            }
            else if (c == EOF_CHAR)         /* At end of file?      */
            {
                break;
            }
            else if (!compiling)
            {                               /* #ifdef false?        */
                skipnl();                   /* Skip to newline      */
                counter++;                  /* Count it, too.       */
            }
            else
            {
                break;                      /* Actual token         */
            }
        }
        if (c == EOF_CHAR)                  /* Exit process at      */
            break;                          /* End of file          */
        /*
         * If the loop didn't terminate because of end of file, we
         * know there is a token to compile.  First, clean up after
         * absorbing newlines.  counter has the number we skipped.
         */
        if ((wrongline && infile->fp != NULL) || counter > 4)
            sharp();                        /* Output # line number */
        else
        {                                   /* If just a few, stuff */
            while (--counter >= 0)          /* them out ourselves   */
                PUTCHAR('\n');
        }
        /*
         * Process each token on this line.
         */
        unget();                            /* Reread the char.     */
        for (;;)
        {                                   /* For the whole line,  */
            do
            {                            /* Token concat. loop   */
                for (counter = 0; type[(c = get())] == SPA;)
                {
                    counter++;              /* Skip over blanks     */

                }
                if (c == EOF_CHAR || c == '\n')
                    goto end_line;          /* Exit line loop       */
                else if (counter > 0)       /* If we got any spaces */
                    PUTCHAR(' ');           /* Output one space     */
                c = macroid(c);             /* Grab the token       */
            }
            while (type[c] == LET && catenate());

            if (c == EOF_CHAR || c == '\n') /* From macro exp error */
                goto end_line;              /* Exit line loop       */

            switch (type[c])
            {
            case LET:
                fputs(token, pCppOut);       /* Quite ordinary token */
#ifdef EVALDEFS
                {
                    int len;
                    if ( bIsInEval
                         && nEvalOff + (len=strlen(token)) < NEVALBUF )
                    {
                        strcpy( &EvalBuf[nEvalOff], token );
                        nEvalOff += len;
                    }
                }
#endif
                break;


            case DIG:                       /* Output a number      */
            case DOT:                       /* Dot may begin floats */
#ifdef EVALDEFS
                if ( bIsInEval )
                    scannumber(c, outputEval);
                else
                    scannumber(c, output);
#else
                scannumber(c, output);
#endif
                break;

            case QUO:                       /* char or string const */
                scanstring(c, output);      /* Copy it to output    */
                break;

            default:                        /* Some other character */
                cput(c);                    /* Just output it       */
#ifdef EVALDEFS
                if ( bIsInEval && nEvalOff < NEVALBUF )
                    EvalBuf[nEvalOff++] = c;
#endif
                break;
            }                               /* Switch ends          */
        }                                   /* Line for loop        */
      end_line:
        if (c == '\n')
        {                                   /* Compiling at EOL?    */
            PUTCHAR('\n');                  /* Output newline, if   */
            if (infile->fp == NULL)         /* Expanding a macro,   */
                wrongline = TRUE;           /* Output # line later  */
        }
    }                                       /* Continue until EOF   */
#ifdef EVALDEFS
    if ( bIsInEval )
        EvalBuf[nEvalOff++] = '\0';
#endif
}

/*
 * Output one character to stdout -- output() is passed as an
 * argument to scanstring()
 */
void output(int c)
{
    if (c != TOK_SEP)
        PUTCHAR(c);
}

#ifdef EVALDEFS
/*
 * Output one character to stdout -- output() is passed as an
 * argument to scanstring()
 */
int outputEval(int c)
{
    if (c != TOK_SEP)
    {
        PUTCHAR(c);
        if ( bIsInEval && nEvalOff < NEVALBUF )
            EvalBuf[nEvalOff++] = c;
    }
}
#endif


/*
 * Output a line number line.
 */
void sharp()
{
    char* name;

    if (keepcomments)                       /* Make sure # comes on */
        PUTCHAR('\n');                      /* a fresh, new line.   */

    fprintf( pCppOut, "#%s %d", LINE_PREFIX, line);
    if (infile->fp != NULL)
    {
        name = (infile->progname != NULL) ? infile->progname : infile->filename;
        if (sharpfilename == NULL || (!streq(name, sharpfilename)))
        {
            if (sharpfilename != NULL)
                free(sharpfilename);
            sharpfilename = savestring(name);
            fprintf( pCppOut, " \"%s\"", name);
        }
    }
    PUTCHAR('\n');
    wrongline = FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
