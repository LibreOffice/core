/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cpp.h,v $
 * $Revision: 1.3 $
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

#define INS         32768   /* input buffer                             */
#define OBS         8092    /* outbut buffer                            */
#define NARG        32      /* Max number arguments to a macro          */
#define NINCLUDE    32      /* Max number of include directories (-I)   */
#define NIF         64      /* depth of nesting of #if                  */
#define NINC        32      /* depth of nesting of #include             */

#ifndef EOF
#define EOF (-1)
#endif

#ifndef NULL
#define NULL    0
#endif

#ifndef __alpha
typedef unsigned char uchar;

#endif

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
    unsigned int wslen;
    unsigned int len;
    uchar *t;
}   Token;

typedef struct tokenrow
{
    Token *tp;                          /* current one to scan */
    Token *bp;                          /* base (allocated value) */
    Token *lp;                          /* last+1 token used */
    int max;                            /* number allocated */
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
    int len;
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
#define quicklook(a,b)  (namebit[(a)&077] & (1<<((b)&037)))
#define quickset(a,b)   namebit[(a)&077] |= (1<<((b)&037))
extern unsigned long namebit[077 + 1];

enum errtype
{
    INFO, WARNING, ERROR, FATAL
};

void expandlex(void);
void fixlex(void);
void setup(int, char **);
int gettokens(Tokenrow *, int);
int comparetokens(Tokenrow *, Tokenrow *);
Source *setsource(char *, int, int, char *, int);
void unsetsource(void);
void puttokens(Tokenrow *);
void process(Tokenrow *);
void *domalloc(int);
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
void expand(Tokenrow *, Nlist *);
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
uchar *newstring(uchar *, int, int);

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
extern int Cplusplus;
extern int skipping;
extern Nlist *kwdefined;
extern Includelist includelist[NINCLUDE];
extern Wraplist wraplist[NINCLUDE];
extern char wd[];

