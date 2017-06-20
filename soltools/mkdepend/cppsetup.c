/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* $XConsortium: cppsetup.c,v 1.13 94/04/17 20:10:32 gildea Exp $ */
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

#include <ctype.h>

#include "def.h"

#ifdef  CPP
/*
 * This file is strictly for the sake of cpy.y and yylex.c (if
 * you indeed have the source for cpp).
 */
#define IB 1
#define SB 2
#define NB 4
#define CB 8
#define QB 16
#define WB 32
#define SALT '#'
#if pdp11 | vax | ns16000 | mc68000 | ibm032
#define COFF 128
#else
#define COFF 0
#endif
/*
 * These variables used by cpy.y and yylex.c
 */
extern char *outp, *inp, *newp, *pend;
extern char *ptrtab;
extern char fastab[];
extern char slotab[];

/*
 * cppsetup
 */
struct filepointer  *currentfile;
struct inclist      *currentinc;

cppsetup(line, filep, inc)
    char   *line;
    struct filepointer *filep;
    struct inclist     *inc;
{
    char *p, savec;
    static boolean setupdone = FALSE;
    boolean value;

    if (!setupdone) {
        cpp_varsetup();
        setupdone = TRUE;
    }

    currentfile = filep;
    currentinc = inc;
    inp = newp = line;
    for (p=newp; *p; p++)
        ;

    /*
     * put a newline back on the end, and set up pend, etc.
     */
    *p++ = '\n';
    savec = *p;
    *p = '\0';
    pend = p;

    ptrtab = slotab+COFF;
    *--inp = SALT;
    outp=inp;
    value = yyparse();
    *p = savec;
    return value;
}

pperror(tag, x0,x1,x2,x3,x4)
    int tag,x0,x1,x2,x3,x4;
{
    warning("\"%s\", line %d: ", currentinc->i_file, currentfile->f_line);
    warning(x0,x1,x2,x3,x4);
}


yyerror(s)
    char   *s;
{
    fatalerr("Fatal error: %s\n", s);
}
#else /* not CPP */

#include "ifparser.h"

static const char *
my_if_errors (IfParser *ip, const char *cp, const char *expecting)
{
#ifdef DEBUG_MKDEPEND
    struct parse_data *pd = (struct parse_data *) ip->data;
    int lineno = pd->filep->f_line;
    char *filename = pd->inc->i_file;
    char prefix[300];
    int prefixlen;
    int i;

    sprintf (prefix, "\"%s\":%d", filename, lineno);
    prefixlen = strlen(prefix);
    fprintf (stderr, "%s:  %s", prefix, pd->line);
    i = cp - pd->line;
    if (i > 0 && pd->line[i-1] != '\n') {
    putc ('\n', stderr);
    }
    for (i += prefixlen + 3; i > 0; i--) {
    putc (' ', stderr);
    }
    fprintf (stderr, "^--- expecting %s\n", expecting);
#endif /* DEBUG_MKDEPEND */
    (void)ip;
    (void)cp;
    (void)expecting;
    return NULL;
}


#define MAXNAMELEN 256

static char *
lookup_variable (const char *var, size_t len)
{
    char tmpbuf[MAXNAMELEN + 1];

    if (len > MAXNAMELEN)
        return NULL;

    strncpy (tmpbuf, var, len);
    tmpbuf[len] = '\0';
    return isdefined(tmpbuf);
}


static int
my_eval_defined (IfParser *ip, const char *var, size_t len)
{
    (void)ip;
    if (lookup_variable (var, len))
        return 1;
    else
        return 0;
}

#define isvarfirstletter(ccc) (isalpha((unsigned char)(ccc)) || (ccc) == '_')

static int
my_eval_variable (IfParser *ip, const char *var, size_t len)
{
    char *s;

    (void)ip;

    s = lookup_variable (var, len);
    if (!s)
        return 0;
    do {
      var = s;
      if (!isvarfirstletter(*var))
        break;
      s = lookup_variable (var, strlen(var));
    } while (s);

    return atoi(var);
}


int cppsetup(char *line)
{
    IfParser ip;
    int val = 0;

    ip.funcs.handle_error = my_if_errors;
    ip.funcs.eval_defined = my_eval_defined;
    ip.funcs.eval_variable = my_eval_variable;

    (void) ParseIfExpression (&ip, line, &val);
    if (val)
      return IF;
    else
      return IFFALSE;
}
#endif /* CPP */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
