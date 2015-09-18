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

#ifndef INCLUDED_RSC_SOURCE_RSCPP_CPP_H
#define INCLUDED_RSC_SOURCE_RSCPP_CPP_H

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif

/* in cpp1.c: file-pointer auf stdout oder file */
extern FILE* pCppOut;                                   /* BP */
#define PUTCHAR( d )   fprintf( pCppOut, "%c", (d) )    /* BP */
#if OSL_DEBUG_LEVEL > 1
extern FILE* pDefOut;                                   /* ER */
#ifdef EVALDEFS
#define NEVALBUF        2048
#endif
#endif

/* limit for reading commandfiles */
#define PARALIMIT       100

#define EOS             '\0'            /* End of string                */
#define EOF_CHAR        0               /* Returned by get() on eof     */
#define NULLST          ((char *) NULL) /* Pointer to nowhere (linted)  */
#define DEF_NOARGS      (-1)            /* #define foo vs #define foo() */

/*
 * The following may need to change if the host system doesn't use ASCII.
 */
#define DEF_MAGIC       0x1D            /* Magic for #defines           */
#define TOK_SEP         0x1E            /* Token concatenation delim.   */
#define COM_SEP         0x1F            /* Magic comment separator      */

#define HT              0x09            /* horizontal tab               */
#define NL              0x0A            /* new line                     */
#define CR              0x0D            /* carriage return              */
#define DEL             0x7F



#ifdef  SOLAR
#define MAC_PARM        0x01            /* Macro formals start here     */
#else
/*
 * Note -- in Ascii, the following will map macro formals onto DEL + the
 * C1 control character region (decimal 128 .. (128 + PAR_MAC)) which will
 * be ok as long as PAR_MAC is less than 33).  Note that the last PAR_MAC
 * value is reserved for string substitution.
 */

#define MAC_PARM        DEL             /* Macro formals start here     */
#if PAR_MAC >= 33
        assertion fails -- PAR_MAC is not less than 33
#endif
#endif
#define LASTPARM        (PAR_MAC - 1)

/*
 * Character type codes.
 */

#define INV             0               /* Invalid, must be zero        */
#define OP_EOE          INV             /* End of expression            */
#define DIG             1               /* Digit                        */
#define LET             2               /* Identifier start             */
#define FIRST_BINOP     OP_ADD
#define OP_ADD          3
#define OP_SUB          4
#define OP_MUL          5
#define OP_DIV          6
#define OP_MOD          7
#define OP_ASL          8
#define OP_ASR          9
#define OP_AND          10              /* &, not &&                    */
#define OP_OR           11              /* |, not ||                    */
#define OP_XOR          12
#define OP_EQ           13
#define OP_NE           14
#define OP_LT           15
#define OP_LE           16
#define OP_GE           17
#define OP_GT           18
#define OP_ANA          19              /* &&                           */
#define OP_ORO          20              /* ||                           */
#define OP_QUE          21              /* ?                            */
#define OP_COL          22              /* :                            */
#define OP_CMA          23              /* , (relevant?)                */
#define LAST_BINOP      OP_CMA          /* Last binary operand          */
/*
 * The following are unary.
 */
#define OP_PLU          24              /* + (draft ANSI standard)      */
#define OP_NEG          25              /* -                            */
#define OP_COM          26              /* ~                            */
#define OP_NOT          27              /* !                            */
#define OP_LPA          28              /* (                            */
#define OP_RPA          29              /* )                            */
#define OP_END          30              /* End of expression marker     */
#define OP_MAX          (OP_END + 1)    /* Number of operators          */
#define OP_FAIL         (OP_END + 1)    /* For error returns            */

/*
 * The following are for lexical scanning only.
 */

#define QUO             65              /* Both flavors of quotation    */
#define DOT             66              /* . might start a number       */
#define SPA             67              /* Space and tab                */
#define BSH             68              /* Just a backslash             */
#define END             69              /* EOF                          */

/*
 * These bits are set in ifstack[]
 */
#define WAS_COMPILING   1               /* TRUE if compile set at entry */
#define ELSE_SEEN       2               /* TRUE when #else processed    */
#define TRUE_SEEN       4               /* TRUE when #if TRUE processed */

/*
 * Define bits for the basic types and their adjectives
 */

#define T_CHAR            1
#define T_INT             2
#define T_FLOAT           4
#define T_DOUBLE          8
#define T_SHORT          16
#define T_LONG           32
#define T_SIGNED         64
#define T_UNSIGNED      128
#define T_PTR           256             /* Pointer                      */
#define T_FPTR          512             /* Pointer to functions         */

/*
 * The DEFBUF structure stores information about #defined
 * macros.  Note that the defbuf->repl information is always
 * in malloc storage.
 */

typedef struct defbuf
{
    struct defbuf*  link;           /* Next define in chain */
    char*           repl;           /* -> replacement       */
    int             hash;           /* Symbol table hash    */
    int             nargs;          /* For define(args)     */
    char            name[];         /* #define name         */
} DEFBUF;

/*
 * The FILEINFO structure stores information about open files
 * and macros being expanded.
 */

typedef struct fileinfo
{
    char*           bptr;           /* Buffer pointer       */
    int             line;           /* for include or macro */
    FILE*           fp;             /* File if non-null     */
    struct fileinfo* parent;        /* Link to includer     */
    char*           filename;       /* File/macro name      */
    char*           progname;       /* From #line statement */
    unsigned int    unrecur;        /* For macro recursion  */
    char            buffer[1];      /* current input line   */
} FILEINFO;

/*
 * The SIZES structure is used to store the values for #if sizeof
 */

typedef struct sizes
{
    short       bits;                   /* If this bit is set,          */
    int         size;                   /* this is the datum size value */
    int         psize;                  /* this is the pointer size     */
} SIZES;

#define cput(c)         { if (c != TOK_SEP) PUTCHAR(c); }
#define streq(s1, s2)   (strcmp(s1, s2) == 0)

/*
 * Error codes.
 */
#define IO_NORMAL 0
#define IO_ERROR  1

/*
 * Externs
 */

extern int      line;                   /* Current line number          */
extern int      wrongline;              /* Force #line to cc pass 1     */
extern char     type[];                 /* Character classifier         */
extern char     token[IDMAX + 1];       /* Current input token          */
extern int      instring;               /* TRUE if scanning string      */
extern int      inmacro;                /* TRUE if scanning #define     */
extern int      errors;                 /* Error counter                */
extern int      recursion;              /* Macro depth counter          */
extern char     ifstack[BLK_NEST];      /* #if information              */
#define compiling ifstack[0]
extern char*    ifptr;                  /* -> current ifstack item      */
extern char*    incdir[NINCLUDE];       /* -i directories               */
extern char**   incend;                 /* -> active end of incdir      */
extern int      cflag;                  /* -C option (keep comments)    */
extern int      eflag;                  /* -E option (ignore errors)    */
extern int      nflag;                  /* -N option (no pre-defines)   */
extern int      rec_recover;            /* unwind recursive macros      */
extern char*    preset[];               /* Standard predefined symbols  */
extern char*    magic[];                /* Magic predefined symbols     */
extern FILEINFO* infile;                /* Current input file           */
extern char     work[NWORK + 1];        /* #define scratch              */
extern char*    workp;                  /* Free space in work           */
#if OSL_DEBUG_LEVEL > 1
extern int      debug;                  /* Debug level                  */
/* ER dump & evaluate #define's */
extern int      bDumpDefs;              /* TRUE if #define's dump req.  */
extern int      bIsInEval;              /* TRUE if #define dumping now  */
#ifdef EVALDEFS
extern char     EvalBuf[NEVALBUF + 1];  /* evaluation buffer            */
extern int      nEvalOff;               /* offset to free buffer pos    */
#endif
#endif
extern int      keepcomments;           /* Don't remove comments if set */
extern SIZES    size_table[];           /* For #if sizeof sizes         */

#ifdef NOMAIN                /* BP */
#ifndef _NO_PROTO
int rscpp_main( int argc, char **argv );
#endif
#define MAIN   rscpp_main     /* fuer die cpp.lib muss main() geandert werden */
#else
#ifdef WNT
#define MAIN   __cdecl main
#else
#define MAIN   main
#endif
#endif


void InitCpp1( void );
void InitCpp4( void );
void InitCpp6( void );

#define HELLO()   fprintf( stderr, "[Hello at %s, %d] ", __FILE__, __LINE__ )

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* cpp1.c */
void output( int c );
void sharp( void );
void cppmain( void );
#if OSL_DEBUG_LEVEL > 1
#ifdef EVALDEFS
int outputEval( int c );
#endif
#endif


/* cpp2.c */
int control( int counter );
void doinclude( void );
void dodefine( void );
void doif( int hash );
int openinclude( char*, int );
int hasdirectory( char*, char*, int );
int openfile( char* );

/* cpp3.c */
int openfiles( char* filename );
void addfile( FILE* fp, char* filename );
void setincdirs( void );
int AddInclude( char* pIncStr );
int getredirection( int argc, char** argv );
void zap_uc( char* ap );

void initdefines( void );
int dooptions( int argc, char* argv[] );
int readoptions( char* filename, char*** pfargv );

/* cpp4.c */
void dodefines( void );
void checkparm( int c, DEFBUF* dp );
int expcollect( void );
void expstuff( DEFBUF* dp );

void stparmscan( int delim );
#if OSL_DEBUG_LEVEL > 1
void dumpparm( char* why );
#endif

void doundef( void );
void textput( char* text );
void charput( int c );
void expand( DEFBUF* tokenp );

/* cpp5.c */
int eval( void );
int evallex( int );
int *evaleval( int*, int, int );
int evalchar( int );
int dosizeof( void );
int evalnum( int c );
int bittest( int );

/* cpp6.c */

void skipnl( void );
int skipws( void );
void scanid( int c );
int macroid( int c );
int catenate(void);
int scanstring( int c, void (*outfun)( int c ) );
void scannumber( int c, void (*outfun)( int c ) );
void save( int c );
char* savestring( char* text );
FILEINFO* getfile( size_t bufsize, char* name );
char *getmem( size_t size );
DEFBUF* lookid( int c );
DEFBUF* defendel( char* name, int delete );
void dunpdef( char* why );
void dumpadef( char* why, DEFBUF* dp );
int get( void );
int cget( void );
void unget( void );
void ungetstring( char* text );
void cerror( char* format, char* sarg );
void cwarn( char* format, char* sarg );
void cfatal( char* format, char* sarg );
void cierror( char* format, int n );
void ciwarn( char* format, int n );
#if OSL_DEBUG_LEVEL > 1
void dumpdef( char* why );
void dumpadef( char* why, DEFBUF *dp );
#endif

#endif // INCLUDED_RSC_SOURCE_RSCPP_CPP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
