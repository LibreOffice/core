/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* $XConsortium: def.h,v 1.25 94/04/17 20:10:33 gildea Exp $ */
/*

Copyright (c) 1993, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#ifndef NO_X11
#include <X11/Xosdefs.h>
#ifdef WIN32
#include <X11/Xw32defs.h>
#endif
#ifndef SUNOS4
#include <X11/Xfuncproto.h>
#endif /* SUNOS4 */
#endif /* NO_X11 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <ctype.h>
#ifndef X_NOT_POSIX
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef S_IFDIR
#define        S_IFDIR  0040000
#endif

#ifndef S_IFREG
#define        S_IFREG  0100000
#endif

#define MAXFILES    65536

#define MAXDIRS     64
#define SYMHASHSEED     131  /* 131 1313 13131 ... */
#define SYMHASHMEMBERS  64  /* must be 2^x to work right */
#define TRUE        1
#define FALSE       0

/* the following must match the directives table in main.c */
#define IF      0
#define IFDEF       1
#define IFNDEF      2
#define ELSE        3
#define ENDIF       4
#define DEFINE      5
#define UNDEF       6
#define INCLUDE     7
#define LINE        8
#define PRAGMA      9
#define ERROR           10
#define IDENT           11
#define SCCS            12
#define ELIF            13
#define EJECT           14
#define IFFALSE         15     /* pseudo value --- never matched */
#define ELIFFALSE       16     /* pseudo value --- never matched */
#define INCLUDEDOT      17     /* pseudo value --- never matched */
#define IFGUESSFALSE    18     /* pseudo value --- never matched */
#define ELIFGUESSFALSE  19     /* pseudo value --- never matched */

#ifdef DEBUG
extern int  _debugmask;
/*
 * debug levels are:
 *
 *     0    show ifn*(def)*,endif
 *     1    trace defined/!defined
 *     2    show #include
 *     3    show #include SYMBOL
 *     4-6  unused
 */
#define debug(level,arg) { if (_debugmask & (1 << level)) warning arg; }
#else
#define debug(level,arg) /**/
#endif /* DEBUG */

// VG: a C++ class for information about directories
#include "collectdircontent.hxx"

typedef unsigned char boolean;

struct pair {
    char        *p_name;
    char        *p_value;
    struct pair *p_next;
};

struct symhash {
    struct pair *s_pairs[SYMHASHMEMBERS];
};

struct  inclist {
    char        *i_incstring;  /* string from #include line */
    char        *i_file;       /* path name of the include file */
    struct inclist  **i_list;  /* list of files it itself includes */
    int         i_listlen;     /* length of i_list */
    boolean     i_notified;    /* whether we have revealed includes */
    boolean     i_marked;      /* whether it's in the makefile */
    boolean     i_searched;    /* whether we have read this */
    boolean     i_included_sym; /* whether #include SYMBOL was found */
                    /* Can't use i_list if TRUE */
};

struct filepointer {
    char    *f_p;
    char    *f_base;
    char    *f_end;
    long    f_len;
    long    f_line;
};

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#if defined(macII) && !defined(__STDC__)  /* stdlib.h fails to define these */
char *malloc(), *realloc();
#endif /* macII */
#else
char            *malloc();
char            *realloc();
#endif

char            *copy(char *);
char            *base_name(char *);
char            *get_line(struct filepointer *);
char            *isdefined(char *);
struct filepointer  *getfile(char *);
struct inclist *newinclude(char *newfile,
                           char *incstring);
struct inclist      *inc_path(char *, char *, boolean,
                                  struct IncludesCollection *);

void define( char *def, struct symhash **symbols );
void hash_define(char *name, char * val, struct symhash **symbols);
struct symhash *hash_copy( struct symhash *symbols );
void hash_free( struct symhash *symbols );
void freefile( struct filepointer * fp );
int find_includes(struct filepointer *filep, struct inclist *file,
    struct inclist *file_red, int recursion, boolean failOK,
    struct IncludesCollection* incCollection, struct symhash *symbols);
void included_by(struct inclist *ip,
    struct inclist * newfile);
int cppsetup(char *line,
    struct filepointer *filep, struct inclist *inc);
void add_include(struct filepointer *filep, struct inclist *file,
    struct inclist *file_red, char *include, boolean dot, boolean failOK,
    struct IncludesCollection* incCollection, struct symhash *symbols);
int match(char *str, char **list);
void recursive_pr_include(struct inclist *head, char *file,
    char *base);
void recursive_pr_dummy(struct inclist *head, char *file);
void inc_clean(void);

void fatalerr(char *, ...);
void warning(char *, ...);
void warning1(char *, ...);

void convert_slashes(char *);
char *append_slash(char *);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
