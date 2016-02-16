/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * $XConsortium: ifparser.c,v 1.8 95/06/03 00:01:41 gildea Exp $
 *
 * Copyright 1992 Network Computing Devices, Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices may not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Network Computing Devices makes
 * no representations about the suitability of this software for any purpose.
 * It is provided ``as is'' without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Fulton
 *          Network Computing Devices, Inc.
 *
 * Simple if statement processor
 *
 * This module can be used to evaluate string representations of C language
 * if constructs.  It accepts the following grammar:
 *
 *     EXPRESSION   :=  VALUE
 *           |  VALUE  BINOP    EXPRESSION
 *
 *     VALUE        :=  '('  EXPRESSION  ')'
 *           |  '!'  VALUE
 *           |  '-'  VALUE
 *           |  'defined'  '('  variable  ')'
 *           |  'defined'  variable
 *           |  # variable '(' variable-list ')'
 *           |  variable
 *           |  number
 *
 *     BINOP        :=  '*' |  '/'  |  '%'
 *           |  '+' |  '-'
 *           |  '<<'    |  '>>'
 *           |  '<' |  '>'  |  '<='  |  '>='
 *           |  '=='    |  '!='
 *           |  '&' |  '|'
 *           |  '&&'    |  '||'
 *
 * The normal C order of precidence is supported.
 *
 *
 * External Entry Points:
 *
 *     ParseIfExpression        parse a string for #if
 */

#include "ifparser.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************************
           Internal Macros and Utilities for Parser
 ****************************************************************************/

#define DO(val) if (!(val)) return NULL
#define CALLFUNC(ggg,fff) (*((ggg)->funcs.fff))
#define SKIPSPACE(ccc) while (isspace(*ccc)) ccc++
#define isvarfirstletter(ccc) (isalpha(ccc) || (ccc) == '_')


static const char *
parse_variable (IfParser *g, const char *cp, const char **varp)
{
    SKIPSPACE (cp);

    if (!isvarfirstletter (*cp))
        return CALLFUNC(g, handle_error) (g, cp, "variable name");

    *varp = cp;
    /* EMPTY */
    for (cp++; isalnum(*cp) || *cp == '_'; cp++) ;
    return cp;
}


static const char *
parse_number (IfParser *g, const char *cp, int *valp)
{
    SKIPSPACE (cp);

    if (!isdigit(*cp))
        return CALLFUNC(g, handle_error) (g, cp, "number");

#ifdef _WIN32
    {
        char *cp2;
        *valp = strtol(cp, &cp2, 0);
    }
#else
    *valp = atoi (cp);
    /* EMPTY */
    for (cp++; isdigit(*cp); cp++) ;
#endif
    return cp;
}


static const char *
parse_value (IfParser *g, const char *cp, int *valp)
{
    const char *var;

    *valp = 0;

    SKIPSPACE (cp);
    if (!*cp)
        return cp;

    switch (*cp) {
      case '(':
        DO (cp = ParseIfExpression (g, cp + 1, valp));
        SKIPSPACE (cp);
        if (*cp != ')')
            return CALLFUNC(g, handle_error) (g, cp, ")");

        return cp + 1;          /* skip the right paren */

      case '!':
        DO (cp = parse_value (g, cp + 1, valp));
        *valp = !(*valp);
        return cp;

      case '-':
        DO (cp = parse_value (g, cp + 1, valp));
        *valp = -(*valp);
        return cp;

      case '#':
        DO (cp = parse_variable (g, cp + 1, &var));
        SKIPSPACE (cp);
        if (*cp != '(')
            return CALLFUNC(g, handle_error) (g, cp, "(");
        do {
            DO (cp = parse_variable (g, cp + 1, &var));
            SKIPSPACE (cp);
        } while (*cp && *cp != ')');
        if (*cp != ')')
            return CALLFUNC(g, handle_error) (g, cp, ")");
        *valp = 1; /* XXX */
        return cp + 1;

      case 'd':
        if (strncmp (cp, "defined", 7) == 0 && !isalnum(cp[7])) {
            int paren = 0;
            size_t len;

            cp += 7;
            SKIPSPACE (cp);
            if (*cp == '(') {
              paren = 1;
              cp++;
            }
            DO (cp = parse_variable (g, cp, &var));
            len = (size_t)(cp - var);
            SKIPSPACE (cp);
            if (paren && *cp != ')')
                return CALLFUNC(g, handle_error) (g, cp, ")");
            *valp = (*(g->funcs.eval_defined)) (g, var, len);
            return cp + paren;      /* skip the right paren */
        }
    /* fall out */
    }

    if (isdigit(*cp)) {
      DO (cp = parse_number (g, cp, valp));
    } else if (!isvarfirstletter(*cp))
      return CALLFUNC(g, handle_error) (g, cp, "variable or number");
    else {
      DO (cp = parse_variable (g, cp, &var));
      *valp = (*(g->funcs.eval_variable)) (g, var, (size_t)(cp - var));
    }

    return cp;
}



static const char *
parse_product (IfParser *g, const char *cp, int *valp)
{
    int rightval;

    DO (cp = parse_value (g, cp, valp));
    SKIPSPACE (cp);

    switch (*cp) {
      case '*':
        DO (cp = parse_product (g, cp + 1, &rightval));
        *valp = (*valp * rightval);
        break;

      case '/':
        DO (cp = parse_product (g, cp + 1, &rightval));

        /* Do nothing in the divide-by-zero case. */
        if (rightval) {
            *valp = (*valp / rightval);
        }
        break;

      case '%':
        DO (cp = parse_product (g, cp + 1, &rightval));
        *valp = (*valp % rightval);
        break;
    }
    return cp;
}


static const char *
parse_sum (IfParser *g, const char *cp, int *valp)
{
    int rightval;

    DO (cp = parse_product (g, cp, valp));
    SKIPSPACE (cp);

    switch (*cp) {
      case '+':
        DO (cp = parse_sum (g, cp + 1, &rightval));
        *valp = (*valp + rightval);
        break;

      case '-':
        DO (cp = parse_sum (g, cp + 1, &rightval));
        *valp = (*valp - rightval);
        break;
    }
    return cp;
}


static const char *
parse_shift (IfParser *g, const char *cp, int *valp)
{
    int rightval;

    DO (cp = parse_sum (g, cp, valp));
    SKIPSPACE (cp);

    switch (*cp) {
      case '<':
        if (cp[1] == '<') {
            DO (cp = parse_shift (g, cp + 2, &rightval));
            *valp = (*valp << rightval);
        }
        break;

      case '>':
        if (cp[1] == '>') {
            DO (cp = parse_shift (g, cp + 2, &rightval));
            *valp = (*valp >> rightval);
        }
        break;
    }
    return cp;
}


static const char *
parse_inequality (IfParser *g, const char *cp, int *valp)
{
    int rightval;

    DO (cp = parse_shift (g, cp, valp));
    SKIPSPACE (cp);

    switch (*cp) {
      case '<':
        if (cp[1] == '=') {
            DO (cp = parse_inequality (g, cp + 2, &rightval));
            *valp = (*valp <= rightval);
        } else {
            DO (cp = parse_inequality (g, cp + 1, &rightval));
            *valp = (*valp < rightval);
        }
        break;

      case '>':
        if (cp[1] == '=') {
            DO (cp = parse_inequality (g, cp + 2, &rightval));
            *valp = (*valp >= rightval);
        } else {
            DO (cp = parse_inequality (g, cp + 1, &rightval));
            *valp = (*valp > rightval);
        }
        break;
    }
    return cp;
}


static const char *
parse_equality (IfParser *g, const char *cp, int *valp)
{
    int rightval;

    DO (cp = parse_inequality (g, cp, valp));
    SKIPSPACE (cp);

    switch (*cp) {
      case '=':
        if (cp[1] == '=')
            cp++;
        DO (cp = parse_equality (g, cp + 1, &rightval));
        *valp = (*valp == rightval);
        break;

      case '!':
        if (cp[1] != '=')
            break;
        DO (cp = parse_equality (g, cp + 2, &rightval));
        *valp = (*valp != rightval);
        break;
    }
    return cp;
}


static const char *
parse_band (IfParser *g, const char *cp, int *valp)
{
    int rightval;

    DO (cp = parse_equality (g, cp, valp));
    SKIPSPACE (cp);

    switch (*cp) {
      case '&':
        if (cp[1] != '&') {
            DO (cp = parse_band (g, cp + 1, &rightval));
            *valp = (*valp & rightval);
        }
        break;
    }
    return cp;
}


static const char *
parse_bor (IfParser *g, const char *cp, int *valp)
{
    int rightval;

    DO (cp = parse_band (g, cp, valp));
    SKIPSPACE (cp);

    switch (*cp) {
      case '|':
        if (cp[1] != '|') {
            DO (cp = parse_bor (g, cp + 1, &rightval));
            *valp = (*valp | rightval);
        }
        break;
    }
    return cp;
}


static const char *
parse_land (IfParser *g, const char *cp, int *valp)
{
    int rightval;

    DO (cp = parse_bor (g, cp, valp));
    SKIPSPACE (cp);

    switch (*cp) {
      case '&':
        if (cp[1] != '&')
            return CALLFUNC(g, handle_error) (g, cp, "&&");
        DO (cp = parse_land (g, cp + 2, &rightval));
        *valp = (*valp && rightval);
        break;
    }
    return cp;
}


static const char *
parse_lor (IfParser *g, const char *cp, int *valp)
{
    int rightval;

    DO (cp = parse_land (g, cp, valp));
    SKIPSPACE (cp);

    switch (*cp) {
      case '|':
        if (cp[1] != '|')
            return CALLFUNC(g, handle_error) (g, cp, "||");
        DO (cp = parse_lor (g, cp + 2, &rightval));
        *valp = (*valp || rightval);
        break;
    }
    return cp;
}


/****************************************************************************
                 External Entry Points
 ****************************************************************************/

const char *
ParseIfExpression (IfParser *g, const char *cp, int *valp)
{
    return parse_lor (g, cp, valp);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
