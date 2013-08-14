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

#include <stdlib.h>
#include <string.h>

#define INS         32768   /* input buffer                             */
#define OBS         8092    /* outbut buffer                            */
#define NARG        32      /* Max number arguments to a macro          */
#define NINCLUDE    48      /* Max number of include directories (-I)   */
#define NIF         64      /* depth of nesting of #if                  */
#define NINC        32      /* depth of nesting of #include             */

#ifndef EOF
#define EOF (-1)
#endif

#ifndef NULL
#define NULL    0
#endif

typedef unsigned char uchar;

enum toktype
{
    END, UNCLASS, NAME, NUMBER, STRING, CCON, NL, WS, DSHARP,
    EQ, NEQ, LEQ, GEQ, LSH, RSH, LAND, LOR, PPLUS, MMINUS,
    ARROW, SBRA, SKET, LP, RP, DOT, AND, STAR, PLUS, MINUS,
    TILDE, NOT, SLASH, PCT, LT, GT, CIRC, OR, QUEST,
    COLON, ASGN, COMMA, SHARP, SEMIC, CBRA, CKET,
    ASPLUS, ASMINUS, ASSTAR, ASSLASH, ASPCT, ASCIRC, ASLSH,
    ASRSH, ASOR, ASAND, ELLIPS,
    DSHARP1, NAME1, NAME2, DEFINED, UMINUS, ARCHITECTURE, IDENT,
    COMMENT
};

enum kwtype
{
    KIF, KIFDEF, KIFNDEF, KELIF, KELSE, KENDIF, KINCLUDE, KINCLUDENEXT,
    KIMPORT, KDEFINE, KUNDEF, KLINE, KERROR, KPRAGMA, KIDENT, KDEFINED,
    KMACHINE, KLINENO, KFILE, KDATE, KTIME, KSTDC, KEVAL
};

#define ISDEFINED       0x01            /* has #defined value */
#define ISKW            0x02            /* is PP keyword */
#define ISUNCHANGE      0x04            /* can't be #defined in PP */
#define ISMAC           0x08            /* builtin macro, e.g. __LINE__ */
#define ISARCHITECTURE  0x10            /* architecture */
#define ISACTIVE        0x80            /* is macro currently expanded */

#define EOB     0xFE                    /* sentinel for end of input buffer */
#define EOFC    0xFD                    /* sentinel for end of input file */
#define XPWS    1                       /* token flag: white space to assure token sep. */
#define XTWS    2

typedef struct token
{
    unsigned char type;
    unsigned char flag;
    size_t wslen;
    size_t len;
    uchar *t;
    unsigned int identifier;            /* used from macro processor to identify where a macro becomes valid again. */
}   Token;

typedef struct tokenrow
{
    Token *tp;                          /* current one to scan */
    Token *bp;                          /* base (allocated value) */
    Token *lp;                          /* last+1 token used */
    size_t max;                         /* number allocated */
}   Tokenrow;

typedef struct source
{
    char *filename;                     /* name of file of the source */
    int line;                           /* current line number */
    int lineinc;                        /* adjustment for \\n lines */
    uchar *inb;                         /* input buffer */
    uchar *inp;                         /* input pointer */
    uchar *inl;                         /* end of input */
    int fd;                             /* input source */
    int ifdepth;                        /* conditional nesting in include */
    int pathdepth;
    int wrap;
    struct source *next;                /* stack for #include */
}   Source;

typedef struct nlist
{
    struct nlist *next;
    uchar *name;
    size_t len;
    Tokenrow *vp;                       /* value as macro */
    Tokenrow *ap;                       /* list of argument names, if any */
    char val;                           /* value as preprocessor name */
    char flag;                          /* is defined, is pp name */
    uchar *loc;                         /* location of definition */
}   Nlist;

typedef struct includelist
{
    char deleted;
    char always;
    char *file;
}   Includelist;

typedef struct wraplist
{
    char *file;
}   Wraplist;

#define new(t)  (t *)domalloc(sizeof(t))
#define quicklook(a,b)  (namebit[(a)&077] & (1U<<((b)&037)))
#define quickset(a,b)   namebit[(a)&077] |= (1U<<((b)&037))
extern unsigned long namebit[077 + 1];

enum errtype
{
    INFO, WARNING, ERROR, FATAL
};


typedef struct macroValidator
{
    Nlist *             pMacro;
    unsigned int        nTokenWhereMacroBecomesValid;
    struct macroValidator *
                        pNext;
} MacroValidator;
typedef struct mvl
{
    MacroValidator * pFirst;
    unsigned int     nextFreeIdentifier;
} MacroValidatorList;

void        mvl_init(
                MacroValidatorList *
                                out_pValidators);
void        mvl_destruct(
                MacroValidatorList *
                                out_pValidators);
/*  Adds MacroValidator to the list.
*/
void        mvl_add(
                MacroValidatorList *
                                inout_pValidators,
                Nlist *         in_pMacro,
                Token *         in_pTokenWhereMacroBecomesValid);

/*  Checks if one of the validators within the list points to
    the token in_pTokenToCheck. If so, the macro is set valid and
    the validator is removed.
*/
void        mvl_check(
                MacroValidatorList *
                                inout_pValidators,
                Token *         inout_pTokenToCheck);

void tokenrow_zeroTokenIdentifiers(Tokenrow* trp);

void expandlex(void);
void fixlex(void);
void setup(int, char **);
int gettokens(Tokenrow *, int);
int comparetokens(Tokenrow *, Tokenrow *);
Source *setsource(char *, int, int, char *, int);
void unsetsource(void);
void puttokens(Tokenrow *);
void process(Tokenrow *);
void *domalloc(size_t);
void dofree(void *);
void error(enum errtype, char *,...);
void flushout(void);
int fillbuf(Source *);
int trigraph(Source *);
int foldline(Source *);
Nlist *lookup(Token *, int);
void control(Tokenrow *);
void dodefine(Tokenrow *);
void doadefine(Tokenrow *, int);
void doinclude(Tokenrow *, int, int);
void doif(Tokenrow *, enum kwtype);
void expand(Tokenrow *, Nlist *, MacroValidatorList *);
void builtin(Tokenrow *, int);
int gatherargs(Tokenrow *, Tokenrow **, int *);
void substargs(Nlist *, Tokenrow *, Tokenrow **);
void expandrow(Tokenrow *, char *);
void maketokenrow(int, Tokenrow *);
Tokenrow *copytokenrow(Tokenrow *, Tokenrow *);
Token *growtokenrow(Tokenrow *);
Tokenrow *normtokenrow(Tokenrow *);
void adjustrow(Tokenrow *, int);
void movetokenrow(Tokenrow *, Tokenrow *);
void insertrow(Tokenrow *, int, Tokenrow *);
void peektokens(Tokenrow *, char *);
void doconcat(Tokenrow *);
Tokenrow *stringify(Tokenrow *);
int lookuparg(Nlist *, Token *);
long eval(Tokenrow *, int);
void genline(void);
void genimport(char *, int, char *, int);
void genwrap(int);
void setempty(Tokenrow *);
void makespace(Tokenrow *, Token *);
char *outnum(char *, int);
int digit(int);
uchar *newstring(uchar *, size_t, size_t);

#define rowlen(tokrow)  ((tokrow)->lp - (tokrow)->bp)

extern char *outptr;
extern Token nltoken;
extern Source *cursource;
extern char *curtime;
extern int incdepth;
extern int ifdepth;
extern int ifsatisfied[NIF];
extern int Mflag;
extern int Iflag;
extern int Pflag;
extern int Aflag;
extern int Lflag;
extern int Xflag;
extern int Vflag;
extern int Cflag;
extern int Dflag;
extern int Cplusplus;
extern int skipping;
extern Nlist *kwdefined;
extern Includelist includelist[NINCLUDE];
extern Wraplist wraplist[NINCLUDE];
extern char wd[];

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
