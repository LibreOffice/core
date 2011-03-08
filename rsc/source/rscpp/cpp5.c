/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include    <stdio.h>
#include    <ctype.h>
#include    "cppdef.h"
#include    "cpp.h"

/*
 * Evaluate an #if expression.
 */

static char *opname[] = {       /* For debug and error messages */
"end of expression", "val", "id",
  "+",   "-",  "*",  "/",  "%",
  "<<", ">>",  "&",  "|",  "^",
  "==", "!=",  "<", "<=", ">=",  ">",
  "&&", "||",  "?",  ":",  ",",
  "unary +", "unary -", "~", "!",  "(",  ")", "(none)",
};

/*
 * opdope[] has the operator precedence:
 *     Bits
 *    7 Unused (so the value is always positive)
 *  6-2 Precedence (000x .. 017x)
 *  1-0 Binary op. flags:
 *      01  The binop flag should be set/cleared when this op is seen.
 *      10  The new value of the binop flag.
 * Note:  Expected, New binop
 * constant 0   1   Binop, end, or ) should follow constants
 * End of line  1   0   End may not be preceeded by an operator
 * binary   1   0   Binary op follows a value, value follows.
 * unary    0   0   Unary op doesn't follow a value, value follows
 *   (      0   0   Doesn't follow value, value or unop follows
 *   )      1   1   Follows value.  Op follows.
 */

static char opdope[OP_MAX] = {
  0001,                 /* End of expression        */
  0002,                 /* Digit            */
  0000,                 /* Letter (identifier)      */
  0141, 0141, 0151, 0151, 0151,     /* ADD, SUB, MUL, DIV, MOD  */
  0131, 0131, 0101, 0071, 0071,     /* ASL, ASR, AND,  OR, XOR  */
  0111, 0111, 0121, 0121, 0121, 0121,   /*  EQ,  NE,  LT,  LE,  GE,  GT */
  0061, 0051, 0041, 0041, 0031,     /* ANA, ORO, QUE, COL, CMA  */
/*
 * Unary op's follow
 */
  0160, 0160, 0160, 0160,       /* NEG, PLU, COM, NOT       */
  0170, 0013, 0023,         /* LPA, RPA, END        */
};
/*
 * OP_QUE and OP_RPA have alternate precedences:
 */
#define OP_RPA_PREC 0013
#define OP_QUE_PREC 0034

/*
 * S_ANDOR and S_QUEST signal "short-circuit" boolean evaluation, so that
 *  #if FOO != 0 && 10 / FOO ...
 * doesn't generate an error message.  They are stored in optab.skip.
 */
#define S_ANDOR     2
#define S_QUEST     1

typedef struct optab {
    char    op;         /* Operator         */
    char    prec;           /* Its precedence       */
    char    skip;           /* Short-circuit: TRUE to skip  */
} OPTAB;
static int  evalue;         /* Current value from evallex() */

#ifdef  nomacargs
FILE_LOCAL int
isbinary(op)
register int    op;
{
    return (op >= FIRST_BINOP && op <= LAST_BINOP);
}

FILE_LOCAL int
isunary(op)
register int    op;
{
    return (op >= FIRST_UNOP && op <= LAST_UNOP);
}
#else
#define isbinary(op)    (op >= FIRST_BINOP && op <= LAST_BINOP)
#define isunary(op) (op >= FIRST_UNOP  && op <= LAST_UNOP)
#endif

/*
 * The following definitions are used to specify basic variable sizes.
 */

#ifndef S_CHAR
#define S_CHAR      (sizeof (char))
#endif
#ifndef S_SINT
#define S_SINT      (sizeof (short int))
#endif
#ifndef S_INT
#define S_INT       (sizeof (int))
#endif
#ifndef S_LINT
#define S_LINT      (sizeof (long int))
#endif
#ifndef S_FLOAT
#define S_FLOAT     (sizeof (float))
#endif
#ifndef S_DOUBLE
#define S_DOUBLE    (sizeof (double))
#endif
#ifndef S_PCHAR
#define S_PCHAR     (sizeof (char *))
#endif
#ifndef S_PSINT
#define S_PSINT     (sizeof (short int *))
#endif
#ifndef S_PINT
#define S_PINT      (sizeof (int *))
#endif
#ifndef S_PLINT
#define S_PLINT     (sizeof (long int *))
#endif
#ifndef S_PFLOAT
#define S_PFLOAT    (sizeof (float *))
#endif
#ifndef S_PDOUBLE
#define S_PDOUBLE   (sizeof (double *))
#endif
#ifndef S_PFPTR
#define S_PFPTR     (sizeof (int (*)()))
#endif

typedef struct types {
    short   type;           /* This is the bit if       */
    char    *name;          /* this is the token word   */
} TYPES;

static TYPES basic_types[] = {
    { T_CHAR,   "char",     },
    { T_INT,    "int",      },
    { T_FLOAT,  "float",    },
    { T_DOUBLE, "double",   },
    { T_SHORT,  "short",    },
    { T_LONG,   "long",     },
    { T_SIGNED, "signed",   },
    { T_UNSIGNED,   "unsigned", },
    { 0,        NULL,       },  /* Signal end       */
};

/*
 * Test_table[] is used to test for illegal combinations.
 */
static short test_table[] = {
    T_FLOAT | T_DOUBLE | T_LONG | T_SHORT,
    T_FLOAT | T_DOUBLE | T_CHAR | T_INT,
    T_FLOAT | T_DOUBLE | T_SIGNED | T_UNSIGNED,
    T_LONG  | T_SHORT  | T_CHAR,
    0                       /* end marker   */
};

/*
 * The order of this table is important -- it is also referenced by
 * the command line processor to allow run-time overriding of the
 * built-in size values.  The order must not be changed:
 *  char, short, int, long, float, double (func pointer)
 */
SIZES size_table[] = {
    { T_CHAR,   S_CHAR,     S_PCHAR     },  /* char     */
    { T_SHORT,  S_SINT,     S_PSINT     },  /* short int    */
    { T_INT,    S_INT,      S_PINT      },  /* int      */
    { T_LONG,   S_LINT,     S_PLINT     },  /* long     */
    { T_FLOAT,  S_FLOAT,    S_PFLOAT    },  /* float    */
    { T_DOUBLE, S_DOUBLE,   S_PDOUBLE   },  /* double   */
    { T_FPTR,   0,      S_PFPTR     },  /* int (*())    */
    { 0,    0,      0       },  /* End of table */
};


void InitCpp5()
{

}



int
eval()
/*
 * Evaluate an expression.  Straight-forward operator precedence.
 * This is called from control() on encountering an #if statement.
 * It calls the following routines:
 * evallex  Lexical analyser -- returns the type and value of
 *      the next input token.
 * evaleval Evaluate the current operator, given the values on
 *      the value stack.  Returns a pointer to the (new)
 *      value stack.
 * For compatiblity with older cpp's, this return returns 1 (TRUE)
 * if a syntax error is detected.
 */
{
    register int    op;     /* Current operator     */
    register int    *valp;      /* -> value vector      */
    register OPTAB  *opp;       /* Operator stack       */
    int     prec;       /* Op precedence        */
    int     binop;      /* Set if binary op. needed */
    int     op1;        /* Operand from stack       */
    int     skip;       /* For short-circuit testing    */
    int     value[NEXP];    /* Value stack          */
    OPTAB       opstack[NEXP];  /* Operand stack        */
    extern int  *evaleval();    /* Does actual evaluation   */
    valp = value;
    opp = opstack;
    opp->op = OP_END;       /* Mark bottom of stack     */
    opp->prec = opdope[OP_END]; /* And its precedence       */
    opp->skip = 0;          /* Not skipping now     */
    binop = 0;
again:  ;
#ifdef  DEBUG_EVAL
    fprintf( pCppOut, "In #if at again: skip = %d, binop = %d, line is: %s",
        opp->skip, binop, infile->bptr);
#endif
    if ((op = evallex(opp->skip)) == OP_SUB && binop == 0)
        op = OP_NEG;            /* Unary minus      */
    else if (op == OP_ADD && binop == 0)
        op = OP_PLU;            /* Unary plus       */
    else if (op == OP_FAIL)
        return (1);             /* Error in evallex */
#ifdef  DEBUG_EVAL
    fprintf( pCppOut, "op = %s, opdope = %03o, binop = %d, skip = %d\n",
        opname[op], opdope[op], binop, opp->skip);
#endif
    if (op == DIG) {            /* Value?       */
        if (binop != 0) {
        cerror("misplaced constant in #if", NULLST);
        return (1);
        }
        else if (valp >= &value[NEXP-1]) {
        cerror("#if value stack overflow", NULLST);
        return (1);
        }
        else {
#ifdef  DEBUG_EVAL
        fprintf( pCppOut, "pushing %d onto value stack[%d]\n",
            evalue, valp - value);
#endif
        *valp++ = evalue;
        binop = 1;
        }
        goto again;
    }
    else if (op > OP_END) {
        cerror("Illegal #if line", NULLST);
        return (1);
    }
    prec = opdope[op];
    if (binop != (prec & 1)) {
        cerror("Operator %s in incorrect context", opname[op]);
        return (1);
    }
    binop = (prec & 2) >> 1;
    for (;;) {
#ifdef  DEBUG_EVAL
        fprintf( pCppOut, "op %s, prec %d., stacked op %s, prec %d, skip %d\n",
        opname[op], prec, opname[opp->op], opp->prec, opp->skip);
#endif
        if (prec > opp->prec) {
        if (op == OP_LPA)
            prec = OP_RPA_PREC;
        else if (op == OP_QUE)
            prec = OP_QUE_PREC;
        op1 = opp->skip;        /* Save skip for test   */
        /*
         * Push operator onto op. stack.
         */
        opp++;
        if (opp >= &opstack[NEXP]) {
            cerror("expression stack overflow at op \"%s\"",
            opname[op]);
            return (1);
        }
        opp->op = (char)op;
        opp->prec = (char)prec;
        skip = (valp[-1] != 0);     /* Short-circuit tester */
        /*
         * Do the short-circuit stuff here.  Short-circuiting
         * stops automagically when operators are evaluated.
         */
        if ((op == OP_ANA && !skip)
         || (op == OP_ORO && skip))
            opp->skip = S_ANDOR;    /* And/or skip starts   */
        else if (op == OP_QUE)      /* Start of ?: operator */
            opp->skip = (char)((op1 & S_ANDOR) | ((!skip) ? S_QUEST : 0));
        else if (op == OP_COL) {    /* : inverts S_QUEST    */
            opp->skip = (char)((op1 & S_ANDOR)
                  | (((op1 & S_QUEST) != 0) ? 0 : S_QUEST));
        }
        else {              /* Other ops leave  */
            opp->skip = (char)op1;      /*  skipping unchanged. */
        }
#ifdef  DEBUG_EVAL
        fprintf( pCppOut, "stacking %s, valp[-1] == %d at %s",
            opname[op], valp[-1], infile->bptr);
        dumpstack(opstack, opp, value, valp);
#endif
        goto again;
        }
        /*
         * Pop operator from op. stack and evaluate it.
         * End of stack and '(' are specials.
         */
        skip = opp->skip;           /* Remember skip value  */
        switch ((op1 = opp->op)) {      /* Look at stacked op   */
        case OP_END:            /* Stack end marker */
        if (op == OP_EOE)
            return (valp[-1]);      /* Finished ok.     */
        goto again;         /* Read another op. */

        case OP_LPA:            /* ( on stack       */
        if (op != OP_RPA) {     /* Matches ) on input   */
            cerror("unbalanced paren's, op is \"%s\"", opname[op]);
            return (1);
        }
        opp--;              /* Unstack it       */
        /* goto again;          -- Fall through     */

        case OP_QUE:
        goto again;         /* Evaluate true expr.  */

        case OP_COL:            /* : on stack.      */
        opp--;              /* Unstack :        */
        if (opp->op != OP_QUE) {    /* Matches ? on stack?  */
            cerror("Misplaced '?' or ':', previous operator is %s",
                   opname[(int)opp->op]);
            return (1);
        }
        /*
         * Evaluate op1.
         */
        default:                /* Others:      */
        opp--;              /* Unstack the operator */
#ifdef  DEBUG_EVAL
        fprintf( pCppOut, "Stack before evaluation of %s\n", opname[op1]);
        dumpstack(opstack, opp, value, valp);
#endif
        valp = evaleval(valp, op1, skip);
#ifdef  DEBUG_EVAL
        fprintf( pCppOut, "Stack after evaluation\n");
        dumpstack(opstack, opp, value, valp);
#endif
        }                   /* op1 switch end   */
    }                   /* Stack unwind loop    */
}

FILE_LOCAL int
evallex(int skip)
/*
 * Return next eval operator or value.  Called from eval().  It
 * calls a special-purpose routines for 'char' strings and
 * numeric values:
 * evalchar called to evaluate 'x'
 * evalnum  called to evaluate numbers.
 */
{
    register int    c, c1, t;

again:  do {                    /* Collect the token    */
        c = skipws();
        if ((c = macroid(c)) == EOF_CHAR || c == '\n') {
        unget();
        return (OP_EOE);        /* End of expression    */
        }
    } while ((t = type[c]) == LET && catenate());
    if (t == INV) {             /* Total nonsense   */
        if (!skip) {
        if (isascii(c) && isprint(c))
            cierror("illegal character '%c' in #if", c);
        else
            cierror("illegal character (%d decimal) in #if", c);
        }
        return (OP_FAIL);
    }
    else if (t == QUO) {            /* ' or "       */
        if (c == '\'') {            /* Character constant   */
        evalue = evalchar(skip);    /* Somewhat messy   */
#ifdef  DEBUG_EVAL
        fprintf( pCppOut, "evalchar returns %d.\n", evalue);
#endif
        return (DIG);           /* Return a value   */
        }
        cerror("Can't use a string in an #if", NULLST);
        return (OP_FAIL);
    }
    else if (t == LET) {            /* ID must be a macro   */
        if (streq(token, "defined")) {  /* Or defined name  */
        c1 = c = skipws();
        if (c == '(')           /* Allow defined(name)  */
            c = skipws();
        if (type[c] == LET) {
            evalue = (lookid(c) != NULL);
            if (c1 != '('       /* Need to balance  */
             || skipws() == ')')    /* Did we balance?  */
            return (DIG);       /* Parsed ok        */
        }
        cerror("Bad #if ... defined() syntax", NULLST);
        return (OP_FAIL);
        }
        else if (streq(token, "sizeof"))    /* New sizeof hackery   */
        return (dosizeof());        /* Gets own routine */
        /*
         * The Draft ANSI C Standard says that an undefined symbol
         * in an #if has the value zero.  We are a bit pickier,
         * warning except where the programmer was careful to write
         *      #if defined(foo) ? foo : 0
         */
#ifdef STRICT_UNDEF
        if (!skip)
        cwarn("undefined symbol \"%s\" in #if, 0 used", token);
#endif
        evalue = 0;
        return (DIG);
    }
    else if (t == DIG) {            /* Numbers are harder   */
        evalue = evalnum(c);
#ifdef  DEBUG_EVAL
        fprintf( pCppOut, "evalnum returns %d.\n", evalue);
#endif
    }
    else if (strchr("!=<>&|\\", c) != NULL) {
        /*
         * Process a possible multi-byte lexeme.
         */
        c1 = cget();            /* Peek at next char    */
        switch (c) {
        case '!':
        if (c1 == '=')
            return (OP_NE);
        break;

        case '=':
        if (c1 != '=') {        /* Can't say a=b in #if */
            unget();
            cerror("= not allowed in #if", NULLST);
            return (OP_FAIL);
        }
        return (OP_EQ);

        case '>':
        case '<':
        if (c1 == c)
            return ((c == '<') ? OP_ASL : OP_ASR);
        else if (c1 == '=')
            return ((c == '<') ? OP_LE  : OP_GE);
        break;

        case '|':
        case '&':
        if (c1 == c)
            return ((c == '|') ? OP_ORO : OP_ANA);
        break;

        case '\\':
        if (c1 == '\n')         /* Multi-line if    */
            goto again;
        cerror("Unexpected \\ in #if", NULLST);
        return (OP_FAIL);
        }
        unget();
    }
    return (t);
}

FILE_LOCAL int
dosizeof()
/*
 * Process the sizeof (basic type) operation in an #if string.
 * Sets evalue to the size and returns
 *  DIG     success
 *  OP_FAIL     bad parse or something.
 */
{
    register int    c;
    register TYPES  *tp;
    register SIZES  *sizp;
    register short  *testp;
    short       typecode;

    if ((c = skipws()) != '(')
        goto nogood;
    /*
     * Scan off the tokens.
     */
    typecode = 0;
    while (0 != (c = skipws())) {
        if ((c = macroid(c)) == EOF_CHAR || c == '\n')
        goto nogood;            /* End of line is a bug */
        else if (c == '(') {        /* thing (*)() func ptr */
        if (skipws() == '*'
         && skipws() == ')') {      /* We found (*)     */
            if (skipws() != '(')    /* Let () be optional   */
            unget();
            else if (skipws() != ')')
            goto nogood;
            typecode |= T_FPTR;     /* Function pointer */
        }
        else {              /* Junk is a bug    */
            goto nogood;
        }
        }
        else if (type[c] != LET)        /* Exit if not a type   */
        break;
        else if (!catenate()) {     /* Maybe combine tokens */
        /*
         * Look for this unexpandable token in basic_types.
         * The code accepts "int long" as well as "long int"
         * which is a minor bug as bugs go (and one shared with
         * a lot of C compilers).
         */
        for (tp = basic_types; tp->name != NULLST; tp++) {
            if (streq(token, tp->name))
            break;
        }
        if (tp->name == NULLST) {
            cerror("#if sizeof, unknown type \"%s\"", token);
            return (OP_FAIL);
        }
        typecode |= tp->type;       /* Or in the type bit   */
        }
    }
    /*
     * We are at the end of the type scan.  Chew off '*' if necessary.
     */
    if (c == '*') {
        typecode |= T_PTR;
        c = skipws();
    }
    if (c == ')') {             /* Last syntax check    */
        for (testp = test_table; *testp != 0; testp++) {
        if (!bittest(typecode & *testp)) {
            cerror("#if ... sizeof: illegal type combination", NULLST);
            return (OP_FAIL);
        }
        }
        /*
         * We assume that all function pointers are the same size:
         *      sizeof (int (*)()) == sizeof (float (*)())
         * We assume that signed and unsigned don't change the size:
         *      sizeof (signed int) == (sizeof unsigned int)
         */
        if ((typecode & T_FPTR) != 0)   /* Function pointer */
        typecode = T_FPTR | T_PTR;
        else {              /* Var or var * datum   */
        typecode &= ~(T_SIGNED | T_UNSIGNED);
        if ((typecode & (T_SHORT | T_LONG)) != 0)
            typecode &= ~T_INT;
        }
        if ((typecode & ~T_PTR) == 0) {
        cerror("#if sizeof() error, no type specified", NULLST);
        return (OP_FAIL);
        }
        /*
         * Exactly one bit (and possibly T_PTR) may be set.
         */
        for (sizp = size_table; sizp->bits != 0; sizp++) {
        if ((typecode & ~T_PTR) == sizp->bits) {
            evalue = ((typecode & T_PTR) != 0)
            ? sizp->psize : sizp->size;
            return (DIG);
        }
        }                   /* We shouldn't fail    */
        cierror("#if ... sizeof: bug, unknown type code 0x%x", typecode);
        return (OP_FAIL);
    }

nogood: unget();
    cerror("#if ... sizeof() syntax error", NULLST);
    return (OP_FAIL);
}

FILE_LOCAL int
bittest(int value)
/*
 * TRUE if value is zero or exactly one bit is set in value.
 */
{
#if (4096 & ~(-4096)) == 0
    return ((value & ~(-value)) == 0);
#else
    /*
     * Do it the hard way (for non 2's complement machines)
     */
    return (value == 0 || value ^ (value - 1) == (value * 2 - 1));
#endif
}

FILE_LOCAL int
evalnum(int c)
/*
 * Expand number for #if lexical analysis.  Note: evalnum recognizes
 * the unsigned suffix, but only returns a signed int value.
 */
{
    register int    value;
    register int    base;
    register int    c1;

    if (c != '0')
        base = 10;
    else if ((c = cget()) == 'x' || c == 'X') {
        base = 16;
        c = cget();
    }
    else base = 8;
    value = 0;
    for (;;) {
        c1 = c;
        if (isascii(c) && isupper(c1))
        c1 = tolower(c1);
#ifdef EBCDIC
        if (c1 <= 'f')
#else
        if (c1 >= 'a')
#endif
        c1 -= ('a' - 10);
        else c1 -= '0';
        if (c1 < 0 || c1 >= base)
        break;
        value *= base;
        value += c1;
        c = cget();
    }
    if (c == 'u' || c == 'U')   /* Unsigned nonsense        */
        c = cget();
    unget();
    return (value);
}

FILE_LOCAL int
evalchar(int skip)
/*
 * Get a character constant
 */
{
    register int    c;
    register int    value;
    register int    count;

    instring = TRUE;
    if ((c = cget()) == '\\') {
        switch ((c = cget())) {
        case 'a':               /* New in Standard  */
#if ('a' == '\a' || '\a' == ALERT)
        value = ALERT;          /* Use predefined value */
#else
        value = '\a';           /* Use compiler's value */
#endif
        break;

        case 'b':
        value = '\b';
        break;

        case 'f':
        value = '\f';
        break;

        case 'n':
        value = '\n';
        break;

        case 'r':
        value = '\r';
        break;

        case 't':
        value = '\t';
        break;

        case 'v':               /* New in Standard  */
#if ('v' == '\v' || '\v' == VT)
        value = VT;         /* Use predefined value */
#else
        value = '\v';           /* Use compiler's value */
#endif
        break;

        case 'x':               /* '\xFF'       */
        count = 3;
        value = 0;
        while ((((c = get()) >= '0' && c <= '9')
             || (c >= 'a' && c <= 'f')
             || (c >= 'A' && c <= 'F'))
            && (--count >= 0)) {
            value *= 16;
#ifdef EBCDIC
            value += (c <= '9') ? (c - '0') : ((c & 0xF) + 9);
#else
            value += (c >= '0') ? (c - '0') : ((c & 0xF) + 9);
#endif
        }
        unget();
        break;

        default:
        if (c >= '0' && c <= '7') {
            count = 3;
            value = 0;
            while (c >= '0' && c <= '7' && --count >= 0) {
            value *= 8;
            value += (c - '0');
            c = get();
            }
            unget();
        }
        else value = c;
        break;
        }
    }
    else if (c == '\'')
        value = 0;
    else value = c;
    /*
     * We warn on multi-byte constants and try to hack
     * (big|little)endian machines.
     */
#if BIG_ENDIAN
    count = 0;
#endif
    while ((c = get()) != '\'' && c != EOF_CHAR && c != '\n') {
        if (!skip)
        ciwarn("multi-byte constant '%c' isn't portable", c);
#if BIG_ENDIAN
        count += BITS_CHAR;
        value += (c << count);
#else
        value <<= BITS_CHAR;
        value += c;
#endif
    }
    instring = FALSE;
    return (value);
}

FILE_LOCAL int *
evaleval(int* valp, int op, int skip)
/*
 * Apply the argument operator to the data on the value stack.
 * One or two values are popped from the value stack and the result
 * is pushed onto the value stack.
 *
 * OP_COL is a special case.
 *
 * evaleval() returns the new pointer to the top of the value stack.
 */
{
    register int    v1, v2 = 0;

    if (isbinary(op))
        v2 = *--valp;
    v1 = *--valp;
#ifdef  DEBUG_EVAL
    fprintf( pCppOut, "%s op %s", (isbinary(op)) ? "binary" : "unary",
        opname[op]);
    if (isbinary(op))
        fprintf( pCppOut, ", v2 = %d.", v2);
    fprintf( pCppOut, ", v1 = %d.\n", v1);
#endif
    switch (op) {
    case OP_EOE:
         break;

    case OP_ADD:
        v1 += v2;
        break;

    case OP_SUB:
        v1 -= v2;
        break;

    case OP_MUL:
        v1 *= v2;
        break;

    case OP_DIV:
    case OP_MOD:
        if (v2 == 0) {
        if (!skip) {
            cwarn("%s by zero in #if, zero result assumed",
            (op == OP_DIV) ? "divide" : "mod");
        }
        v1 = 0;
        }
        else if (op == OP_DIV)
        v1 /= v2;
        else
        v1 %= v2;
        break;

    case OP_ASL:
        v1 <<= v2;
        break;

    case OP_ASR:
        v1 >>= v2;
        break;

    case OP_AND:
        v1 &= v2;
        break;

    case OP_OR:
        v1 |= v2;
        break;

    case OP_XOR:
        v1 ^= v2;
        break;

    case OP_EQ:
        v1 = (v1 == v2);
        break;

    case OP_NE:
        v1 = (v1 != v2);
        break;

    case OP_LT:
        v1 = (v1 < v2);
        break;

    case OP_LE:
        v1 = (v1 <= v2);
        break;

    case OP_GE:
        v1 = (v1 >= v2);
        break;

    case OP_GT:
        v1 = (v1 > v2);
        break;

    case OP_ANA:
        v1 = (v1 && v2);
        break;

    case OP_ORO:
        v1 = (v1 || v2);
        break;

    case OP_COL:
        /*
         * v1 has the "true" value, v2 the "false" value.
         * The top of the value stack has the test.
         */
        v1 = (*--valp) ? v1 : v2;
        break;

    case OP_NEG:
        v1 = (-v1);
        break;

    case OP_PLU:
        break;

    case OP_COM:
        v1 = ~v1;
        break;

    case OP_NOT:
        v1 = !v1;
        break;

    default:
        cierror("#if bug, operand = %d.", op);
        v1 = 0;
    }
    *valp++ = v1;
    return (valp);
}

#ifdef  DEBUG_EVAL
dumpstack(opstack, opp, value, valp)
OPTAB       opstack[NEXP];  /* Operand stack        */
register OPTAB  *opp;       /* Operator stack       */
int     value[NEXP];    /* Value stack          */
register int    *valp;      /* -> value vector      */
{
    fprintf( pCppOut, "index op prec skip name -- op stack at %s", infile->bptr);
    while (opp > opstack) {
        fprintf( pCppOut, " [%2d] %2d  %03o    %d %s\n", opp - opstack,
        opp->op, opp->prec, opp->skip, opname[opp->op]);
        opp--;
    }
    while (--valp >= value) {
        fprintf( pCppOut, "value[%d] = %d\n", (valp - value), *valp);
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
