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
#ifdef UNX
#include <stdlib.h>
#endif
#include <ctype.h>
#include "cppdef.h"
#include "cpp.h"

#include "time.h" /* BP */

#include <string.h>

#if (OSL_DEBUG_LEVEL > 1) && (HOST == SYS_UNIX)
#include <signal.h>
#endif

/*
 * Open a file, add it to the linked list of open files.
 * This is called only from openfile() above.
 */
int openfile(char* filename)
{
    FILE* fp;

    if ((fp = fopen(filename, "r")) == NULL)
    {
#if OSL_DEBUG_LEVEL > 1
        if ( debug || !bDumpDefs )
            perror(filename);
#endif
        return (FALSE);
    }
#if OSL_DEBUG_LEVEL > 1
    if (debug)
        fprintf(stderr, "Reading from \"%s\"\n", filename);
#endif
    addfile(fp, filename);
    return (TRUE);
}

/*
 * Initialize tables for this open file.  This is called from openfile()
 * above (for #include files), and from the entry to cpp to open the main
 * input file.  It calls a common routine, getfile() to build the FILEINFO
 * structure which is used to read characters.  (getfile() is also called
 * to setup a macro replacement.)
 */
void addfile(FILE* fp, char* filename)
{
    FILEINFO* file;

    file = getfile(NBUFF, filename);
    file->fp = fp;                  /* Better remember FILE *       */
    file->buffer[0] = EOS;          /* Initialize for first read    */
    line = 1;                       /* Working on line 1 now        */
    wrongline = TRUE;               /* Force out initial #line      */
}

/*
 * Append system-specific directories to the include directory list.
 * Called only when cpp is started.
 */
void setincdirs()
{

#ifdef  CPP_INCLUDE
    *incend++ = CPP_INCLUDE;
#define IS_INCLUDE      1
#else
#define IS_INCLUDE      0
#endif

#if HOST == SYS_UNIX
    *incend++ = "/usr/include";
#define MAXINCLUDE      (NINCLUDE - 1 - IS_INCLUDE)
#endif


#if HOST == SYS_UNKNOWN
/*
 * Context: GenMake
 * Under DOS also the environment variable INCLUDE is used.
 * To make it difficult all entries separated by ';' have to be
 * included in the list and this is done with strtok().
 * Be careful using malloc() !!!
 * In savestring() as a matter of fact getmem() is used and probably these
 * two functions are getting in the way of each other.
 * When I used malloc() instead of savestring() an error occurred in strcpy().
 */

#if !defined(_WIN32) && !defined(UNX)
    extern char* getenv( char *pStr ); /* BP */
#endif
    char* pIncGetEnv = NULL;    /* Pointer to INCLUDE   */

    if ( ( pIncGetEnv = getenv("INCLUDE") ) != NULL )
        AddInclude( pIncGetEnv );

#define MAXINCLUDE      (NINCLUDE - 3 - IS_INCLUDE)
#endif

}


/* Context: Extension of the INCLUDE service
 * So far the cpp couldn't use include statements in the command line
 * where the directories are separated with ';'.
 * This totally understandable because this cpp is fitted to UNIX
 * systems and under UNIX ';' is used to terminate commandos.
 */

int AddInclude( char* pIncStr )
{
    char* pIncEnv    = NULL;    /* copy of INCLUDE     */
    char* pIncPos;              /* goes to the next */

    pIncEnv = savestring( pIncStr );
    pIncPos = strtok( pIncEnv, ";" );

    while( pIncPos != NULL )
    {
        if (incend >= &incdir[MAXINCLUDE])
            cfatal("Too many include directories", NULLST);
        *incend++ = pIncPos;
        pIncPos   = strtok( NULL, ";" );
    }
    /* coverity[leaked_storage] - we know this leaks, but it doesn't matter in this short lived utility */
    return 1;
}

/*
 * dooptions is called to process command line arguments (-Detc).
 * It is called only at cpp startup.
 */
int dooptions(int argc, char** argv)
{
    char* ap;
    DEFBUF* dp;
    int c;
    int i, j;
    char* arg;
    SIZES* sizp;        /* For -S               */
    int size;           /* For -S               */
    int isdatum;        /* FALSE for -S*        */
    int endtest;        /* For -S               */

    for (i = j = 1; i < argc; i++)
    {
        arg = ap = argv[i];

        if (*ap++ != '-' || *ap == EOS)
        {
            argv[j++] = argv[i];
        }
        else
        {
            c = *ap++;                      /* Option byte          */
            if (islower((unsigned char)c))                 /* Normalize case       */
                c = toupper(c);
            switch (c)                      /* Command character    */
            {
            case 'C':                       /* Keep comments        */
                cflag = TRUE;
                keepcomments = TRUE;
                break;

            case 'D':                       /* Define symbol        */
                /*
                 * If the option is just "-Dfoo", make it -Dfoo=1
                 */
                while (*ap != EOS && *ap != '=')
                    ap++;
                if (*ap == EOS)
                    ap = "1";
                else
                    *ap++ = EOS;
                /*
                 * Now, save the word and its definition.
                 */
                dp = defendel(argv[i] + 2, FALSE);
                dp->repl = savestring(ap);
                dp->nargs = DEF_NOARGS;
                break;

            case 'E':                       /* Ignore non-fatal     */
                eflag = TRUE;               /* errors.              */
                break;

            case 'I':                       /* Include directory    */
                AddInclude( ap );           /* BP, 11.09.91 */
                break;

            case 'N':                       /* No predefined        */
                nflag++;                    /* Repeat to undefine   */
                break;                      /* __LINE__, etc.       */

            case 'S':
                sizp = size_table;
                if (0 != (isdatum = (*ap != '*'))) /* If it's just -S,     */
                    endtest = T_FPTR;       /* Stop here            */
                else                        /* But if it's -S*      */
                {
                    ap++;                   /* Step over '*'        */
                    endtest = 0;            /* Stop at end marker   */
                }
                while (sizp->bits != endtest && *ap != EOS)
                {
                    if (!isdigit((unsigned char)*ap))      /* Skip to next digit   */
                    {
                        ap++;
                        continue;
                    }
                    size = 0;               /* Compile the value    */
                    while (isdigit((unsigned char)*ap))
                    {
                        size *= 10;
                        size += (*ap++ - '0');
                    }
                    if (isdatum)
                        sizp->size = size;  /* Datum size           */
                    else
                        sizp->psize = size; /* Pointer size         */
                    sizp++;
                }
                if (sizp->bits != endtest)
                    cwarn("-S, too few values specified in %s", argv[i]);
                else if (*ap != EOS)
                    cwarn("-S, too many values, \"%s\" unused", ap);
                break;

            case 'U':                       /* Undefine symbol      */
                if (defendel(ap, TRUE) == NULL)
                    cwarn("\"%s\" wasn't defined", ap);
                break;

#if OSL_DEBUG_LEVEL > 1
            case 'X':                       /* Debug                */
                debug = (isdigit((unsigned char)*ap)) ? atoi(ap) : 1;
#if (HOST == SYS_UNIX)
                signal(SIGINT, (void (*)(int)) abort); /* Trap "interrupt" */
#endif
                fprintf(stderr, "Debug set to %d\n", debug);
                break;
#endif

#if OSL_DEBUG_LEVEL > 1
            case 'P':                       /* #define's dump       */
                bDumpDefs = 1;
                fprintf(stderr, "Dump #define's is on\n");
                break;
#endif

            default:                        /* What is this one?    */
                cwarn("Unknown option \"%s\"", arg);
                fprintf(stderr, "The following options are valid:\n\
  -C\t\t\tWrite source file comments to output\n\
  -Dsymbol=value\tDefine a symbol with the given (optional) value\n\
  -Idirectory\t\tAdd a directory to the #include search list\n\
  -N\t\t\tDon't predefine target-specific names\n\
  -Stext\t\tSpecify sizes for #if sizeof\n\
  -Usymbol\t\tUndefine symbol\n");
#if OSL_DEBUG_LEVEL > 1
                fprintf(stderr, "  -Xvalue\t\tSet internal debug flag\n");
                fprintf(stderr, "  -P\t\t\tdump #define's\n");
#endif
                break;
            }                       /* Switch on all options        */
        }                           /* If it's a -option            */
    }                               /* For all arguments            */
#if OSL_DEBUG_LEVEL > 1
    if ( (bDumpDefs ? j > 4 : j > 3) )
#else
    if (j > 3)
#endif
    {
        cerror( "Too many file arguments.  Usage: cpp [input [output]]",
                NULLST);
    }
    return j;                     /* Return new argc              */
}

int readoptions(char* filename, char*** pfargv)
{
    FILE* fp;
    int c;
    int bInQuotes = 0;
    char optbuff[1024];
    char* poptbuff;
    int fargc=0;
    int back;
    char* fargv[PARALIMIT];
    char** pfa;

    pfa = *pfargv = malloc(sizeof(fargv));

    poptbuff = &optbuff[0];
    filename++;
    if ((fp = fopen(filename, "r")) == NULL)
    {
#if OSL_DEBUG_LEVEL > 1
        if ( debug || !bDumpDefs )
            perror(filename);
#endif
        return (FALSE);
    }
    do
    {
        /*
         *  #i27914# double ticks '"' now have a duplicate function:
         *  1. they define a string ( e.g. -DFOO="baz" )
         *  2. a string can contain spaces, so -DFOO="baz zum" defines one
         *  argument no two !
         */
        c = fgetc(fp);
        if ( c != ' ' && c != CR && c != NL && c != HT && c != EOF)
        {
            *poptbuff++ = (char)c;
            if( c == '"' )
                bInQuotes = ~bInQuotes;
        }
        else
        {
            if( c != EOF && bInQuotes )
                *poptbuff++ = (char)c;
            else
            {
                *poptbuff = EOS;
                if (optbuff[0] != '\0')
                {
                    pfa[fargc + 1] = strdup(optbuff);
                    fargc++;
                    pfa[fargc + 1] = NULL;
                    poptbuff = &optbuff[0];
                }
            }
        }
    }
    while ( c != EOF );

    fclose(fp);
    back=dooptions(fargc+1,pfa);

    return back;
}

/*
 * Initialize the built-in #define's.  There are two flavors:
 *      #define decus   1               (static definitions)
 *      #define __FILE__ ??             (dynamic, evaluated by magic)
 * Called only on cpp startup.
 *
 * Note: the built-in static definitions are suppressed by the -N option.
 * __LINE__, __FILE__, and __DATE__ are always present.
 */
void initdefines()
{
    char** pp;
    char* tp;
    DEFBUF* dp;
    int i;
    time_t tvec;

    /*
     * Predefine the built-in symbols.  Allow the
     * implementor to pre-define a symbol as "" to
     * eliminate it.
     */
    if (nflag == 0)
    {
        for (pp = preset; *pp != NULL; pp++)
        {
            if (*pp[0] != EOS)
            {
                dp = defendel(*pp, FALSE);
                dp->repl = savestring("1");
                dp->nargs = DEF_NOARGS;
            }
        }
    }
    /*
     * The magic pre-defines (__FILE__ and __LINE__ are
     * initialized with negative argument counts.  expand()
     * notices this and calls the appropriate routine.
     * DEF_NOARGS is one greater than the first "magic" definition.
     */
    if (nflag < 2)
    {
        for (pp = magic, i = DEF_NOARGS; *pp != NULL; pp++)
        {
            dp = defendel(*pp, FALSE);
            dp->nargs = --i;
        }
#if OK_DATE
        /*
         * Define __DATE__ as today's date.
         */
        dp = defendel("__DATE__", FALSE);
        dp->repl = tp = getmem(27);
        dp->nargs = DEF_NOARGS;
        time( &tvec);
        *tp++ = '"';
        strcpy(tp, ctime(&tvec));
        tp[24] = '"';                       /* Overwrite newline    */
#endif
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
