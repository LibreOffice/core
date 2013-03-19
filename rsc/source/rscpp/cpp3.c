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

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _NO_PROTO
int AddInclude( char *pIncStr );  /* BP, 11.09.91, Forward-Deklaration */
#endif

#if (OSL_DEBUG_LEVEL > 1) && (HOST == SYS_VMS || HOST == SYS_UNIX)
#include <signal.h>
#endif

void InitCpp3()
{
}


int
openfile(char* filename)
/*
 * Open a file, add it to the linked list of open files.
 * This is called only from openfile() above.
 */
{
        register FILE           *fp;

        if ((fp = fopen(filename, "r")) == NULL) {
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

void addfile(FILE* fp, char* filename)
/*
 * Initialize tables for this open file.  This is called from openfile()
 * above (for #include files), and from the entry to cpp to open the main
 * input file.  It calls a common routine, getfile() to build the FILEINFO
 * structure which is used to read characters.  (getfile() is also called
 * to setup a macro replacement.)
 */
{
        register FILEINFO       *file;
        extern FILEINFO         *getfile( int, char * );
        file = getfile(NBUFF, filename);
        file->fp = fp;                  /* Better remember FILE *       */
        file->buffer[0] = EOS;          /* Initialize for first read    */
        line = 1;                       /* Working on line 1 now        */
        wrongline = TRUE;               /* Force out initial #line      */
}

void setincdirs()
/*
 * Append system-specific directories to the include directory list.
 * Called only when cpp is started.
 */
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

#if HOST == SYS_VMS
        extern char     *getenv();

        if (getenv("C$LIBRARY") != NULL)
            *incend++ = "C$LIBRARY:";
        *incend++ = "SYS$LIBRARY:";
#define MAXINCLUDE      (NINCLUDE - 2 - IS_INCLUDE)
#endif

#if HOST == SYS_RSX
        extern int      $$rsts;                 /* TRUE on RSTS/E       */
        extern int      $$pos;                  /* TRUE on PRO-350 P/OS */
        extern int      $$vms;                  /* TRUE on VMS compat.  */

        if ($$pos) {                            /* P/OS?                */
            *incend++ = "SY:[ZZDECUSC]";        /* C #includes          */
            *incend++ = "LB:[1,5]";             /* RSX library          */
        }
        else if ($$rsts) {                      /* RSTS/E?              */
            *incend++ = "SY:@";                 /* User-defined account */
            *incend++ = "C:";                   /* Decus-C library      */
            *incend++ = "LB:[1,1]";             /* RSX library          */
        }
        else if ($$vms) {                       /* VMS compatibility?   */
            *incend++ = "C:";
        }
        else {                                  /* Plain old RSX/IAS    */
            *incend++ = "LB:[1,1]";
        }
#define MAXINCLUDE      (NINCLUDE - 3 - IS_INCLUDE)
#endif

#if HOST == SYS_RT11
        extern int      $$rsts;                 /* RSTS/E emulation?    */

        if ($$rsts)
            *incend++ = "SY:@";                 /* User-defined account */
        *incend++ = "C:";                       /* Decus-C library disk */
        *incend++ = "SY:";                      /* System (boot) disk   */
#define MAXINCLUDE      (NINCLUDE - 3 - IS_INCLUDE)
#endif

#if HOST == SYS_UNKNOWN
/*
 * Kontext: GenMake
 * Unter DOS wird nun auch die Environment-Variable INCLUDE ausgewetet.
 * Es kommt erschwerend hinzu, dass alle Eintraege, die mit ';' getrennt
 * sind, mit in die Liste aufenommen werden muessen.
 * Dies wird mit der Funktion strtok() realisiert.
 * Vorsicht bei der Benutzung von malloc !!!
 * In savestring wird naemlich getmem() verwendet. Vermutlich kommen sich
 * die beiden Funktion in die Quere. Als ich malloc statt savestring
 * verwendete knallte es in strcpy() !
 */

#if !defined( WNT ) && ! defined UNX
        extern   char     *getenv( char *pStr ); /* BP */
#endif
                 char     *pIncGetEnv = NULL;    /* Pointer auf INCLUDE   */

        if ( ( pIncGetEnv = getenv("INCLUDE") ) != NULL )
            AddInclude( pIncGetEnv );

#define MAXINCLUDE      (NINCLUDE - 3 - IS_INCLUDE)
#endif


}

/* Kontext: Erweiterung des INCLUDE-Services
 * Bislang konnte der cpp keine Include-Angaben in der Kommandozeile
 * vertragen, bei denen die directries mit ';' getrennt wurden.
 * Dies ist auch verstaendlich, da dieses cpp fuer UNIX-Systeme
 * massgeschneidert wurde und in UNI die ';' als Zeichen zum Abschluss
 * von Kommandos gilt.
 */

int AddInclude( char* pIncStr )
{
    char     *pIncEnv    = NULL;    /* Kopie des INCLUDE     */
    char     *pIncPos;              /* wandert zum naechsten */

    pIncEnv = savestring( pIncStr );
    pIncPos = strtok( pIncEnv, ";" );

    while( pIncPos != NULL )
    {
        if (incend >= &incdir[MAXINCLUDE])
            cfatal("Too many include directories", NULLST);
        *incend++ = pIncPos;
        pIncPos   = strtok( NULL, ";" );
    }
    return( 1 );
}

int
dooptions(int argc, char** argv)
/*
 * dooptions is called to process command line arguments (-Detc).
 * It is called only at cpp startup.
 */
{
        register char           *ap;
        register DEFBUF         *dp;
        register int            c;
        int                     i, j;
        char                    *arg;
        SIZES                   *sizp;          /* For -S               */
        int                     size;           /* For -S               */
        int                     isdatum;        /* FALSE for -S*        */
        int                     endtest;        /* For -S               */

        for (i = j = 1; i < argc; i++) {
            arg = ap = argv[i];

            if (*ap++ != '-' || *ap == EOS)
            {
                    argv[j++] = argv[i];
            }
            else {
                c = *ap++;                      /* Option byte          */
                if (islower(c))                 /* Normalize case       */
                    c = toupper(c);
                switch (c) {                    /* Command character    */
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

                case 'N':                       /* No predefineds       */
                    nflag++;                    /* Repeat to undefine   */
                    break;                      /* __LINE__, etc.       */

                case 'S':
                    sizp = size_table;
                    if (0 != (isdatum = (*ap != '*'))) /* If it's just -S,     */
                        endtest = T_FPTR;       /* Stop here            */
                    else {                      /* But if it's -S*      */
                        ap++;                   /* Step over '*'        */
                        endtest = 0;            /* Stop at end marker   */
                    }
                    while (sizp->bits != endtest && *ap != EOS) {
                        if (!isdigit(*ap)) {    /* Skip to next digit   */
                            ap++;
                            continue;
                        }
                        size = 0;               /* Compile the value    */
                        while (isdigit(*ap)) {
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
                    debug = (isdigit(*ap)) ? atoi(ap) : 1;
#if (HOST == SYS_VMS || HOST == SYS_UNIX)
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
        if ( (bDumpDefs ? j > 4 : j > 3) ) {
#else
        if (j > 3) {
#endif
            cerror(
                "Too many file arguments.  Usage: cpp [input [output]]",
                NULLST);
        }
        return (j);                     /* Return new argc              */
}

int
readoptions(char* filename, char*** pfargv)
{
        FILE           *fp;
        int c;
        int bInQuotes = 0;
        char optbuff[1024], *poptbuff;
        int fargc=0, back;
        char *fargv[PARALIMIT], **pfa;

        pfa=*pfargv=malloc(sizeof(fargv));

        poptbuff=&optbuff[0];
        filename++;
        if ((fp = fopen(filename, "r")) == NULL) {
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
            c=fgetc(fp);
            if ( c != ' ' && c != CR && c != NL && c != HT && c != EOF)
            {
                *poptbuff++=(char)c;
                if( c == '"' )
                    bInQuotes = ~bInQuotes;
            }
            else
            {
                if( c != EOF && bInQuotes )
                    *poptbuff++=(char)c;
                else
                {
                    *poptbuff=EOS;
                    if (strlen(optbuff)>0)
                    {
                        pfa[fargc+1]=malloc(strlen(optbuff)+1);
                        strcpy(pfa[fargc+1],optbuff);
                        fargc++;
                        pfa[fargc+1]=0;
                        poptbuff=&optbuff[0];
                    }
                }
            }
        }
        while ( c != EOF );

        fclose(fp);
        back=dooptions(fargc+1,pfa);

        return (back);
}

#if HOST != SYS_UNIX
FILE_LOCAL void
zap_uc(char* ap)
/*
 * Dec operating systems mangle upper-lower case in command lines.
 * This routine forces the -D and -U arguments to uppercase.
 * It is called only on cpp startup by dooptions().
 */
{
        while (*ap != EOS) {
            /*
             * Don't use islower() here so it works with Multinational
             */
            if (*ap >= 'a' && *ap <= 'z')
                *ap = (char)toupper(*ap);
            ap++;
        }
}
#endif

void initdefines()
/*
 * Initialize the built-in #define's.  There are two flavors:
 *      #define decus   1               (static definitions)
 *      #define __FILE__ ??             (dynamic, evaluated by magic)
 * Called only on cpp startup.
 *
 * Note: the built-in static definitions are supressed by the -N option.
 * __LINE__, __FILE__, and __DATE__ are always present.
 */
{
        register char           **pp;
        register char           *tp;
        register DEFBUF         *dp;
        int                     i;
        time_t                  tvec;

#if !defined( WNT ) && !defined(G3)
        extern char             *ctime();
#endif

        /*
         * Predefine the built-in symbols.  Allow the
         * implementor to pre-define a symbol as "" to
         * eliminate it.
         */
        if (nflag == 0) {
            for (pp = preset; *pp != NULL; pp++) {
                if (*pp[0] != EOS) {
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
        if (nflag < 2) {
            for (pp = magic, i = DEF_NOARGS; *pp != NULL; pp++) {
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

#if HOST == SYS_VMS
/*
 * getredirection() is intended to aid in porting C programs
 * to VMS (Vax-11 C) which does not support '>' and '<'
 * I/O redirection.  With suitable modification, it may
 * useful for other portability problems as well.
 */

int
getredirection(argc, argv)
int             argc;
char            **argv;
/*
 * Process vms redirection arg's.  Exit if any error is seen.
 * If getredirection() processes an argument, it is erased
 * from the vector.  getredirection() returns a new argc value.
 *
 * Warning: do not try to simplify the code for vms.  The code
 * presupposes that getredirection() is called before any data is
 * read from stdin or written to stdout.
 *
 * Normal usage is as follows:
 *
 *      main(argc, argv)
 *      int             argc;
 *      char            *argv[];
 *      {
 *              argc = getredirection(argc, argv);
 *      }
 */
{
        register char           *ap;    /* Argument pointer     */
        int                     i;      /* argv[] index         */
        int                     j;      /* Output index         */
        int                     file;   /* File_descriptor      */
        extern int              errno;  /* Last vms i/o error   */

        for (j = i = 1; i < argc; i++) {   /* Do all arguments  */
            switch (*(ap = argv[i])) {
            case '<':                   /* <file                */
                if (freopen(++ap, "r", stdin) == NULL) {
                    perror(ap);         /* Can't find file      */
                    exit(errno);        /* Is a fatal error     */
                }
                break;

            case '>':                   /* >file or >>file      */
                if (*++ap == '>') {     /* >>file               */
                    /*
                     * If the file exists, and is writable by us,
                     * call freopen to append to the file (using the
                     * file's current attributes).  Otherwise, create
                     * a new file with "vanilla" attributes as if the
                     * argument was given as ">filename".
                     * access(name, 2) returns zero if we can write on
                     * the specified file.
                     */
                    if (access(++ap, 2) == 0) {
                        if (freopen(ap, "a", stdout) != NULL)
                            break;      /* Exit case statement  */
                        perror(ap);     /* Error, can't append  */
                        exit(errno);    /* After access test    */
                    }                   /* If file accessible   */
                }
                /*
                 * On vms, we want to create the file using "standard"
                 * record attributes.  creat(...) creates the file
                 * using the caller's default protection mask and
                 * "variable length, implied carriage return"
                 * attributes. dup2() associates the file with stdout.
                 */
                if ((file = creat(ap, 0, "rat=cr", "rfm=var")) == -1
                 || dup2(file, fileno(stdout)) == -1) {
                    perror(ap);         /* Can't create file    */
                    exit(errno);        /* is a fatal error     */
                }                       /* If '>' creation      */
                break;                  /* Exit case test       */

            default:
                argv[j++] = ap;         /* Not a redirector     */
                break;                  /* Exit case test       */
            }
        }                               /* For all arguments    */
        argv[j] = NULL;                 /* Terminate argv[]     */
        return (j);                     /* Return new argc      */
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
