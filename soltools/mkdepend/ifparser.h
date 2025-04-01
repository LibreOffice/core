/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * $XConsortium: ifparser.h,v 1.1 92/08/22 13:05:39 rws Exp $
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
 * The normal C order of precedence is supported.
 *
 *
 * External Entry Points:
 *
 *     ParseIfExpression        parse a string for #if
 */

#include <stdio.h>

typedef int Bool;
 0
 1

typedef struct if_parser {
    struct {                /* functions */
      const char *(*handle_error)  (struct if_parser *, const char *, const char *);
      int         (*eval_variable) (struct if_parser *, const char *, size_t);
      int         (*eval_defined)  (struct if_parser *, const char *, size_t);
    } funcs;
} IfParser;

const char *ParseIfExpression (IfParser *, const char *, int *);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
