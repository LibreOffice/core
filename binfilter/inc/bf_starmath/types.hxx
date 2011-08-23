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
#ifndef TYPES_HXX
#define TYPES_HXX
namespace binfilter {


#define FONTNAME_MATH   "StarSymbol"
#define FONTNAME_MATH2  "OpenSymbol"

/////////////////////////////////////////////////////////////////

enum SmPrintSize { PRINT_SIZE_NORMAL, PRINT_SIZE_SCALED, PRINT_SIZE_ZOOMED };

/////////////////////////////////////////////////////////////////
// enum definitions for characters from the 'StarSymbol' font
// (some chars have more than one alias!)
//! Note: not listed here does not(!) mean "not used"
//!		(see %alpha ... %gamma for example)
//
enum MathSymbol
{
    MS_FACT         = (xub_Unicode) 0x0021,
    MS_INFINITY     = (xub_Unicode) 0x221E,
    MS_SLASH        = (xub_Unicode) 0x002F,

    MS_NDIVIDES     = (xub_Unicode) 0x2224,
    MS_DRARROW      = (xub_Unicode) 0x21D2,
    MS_DLARROW      = (xub_Unicode) 0x21D0,
    MS_DLRARROW     = (xub_Unicode) 0x21D4,
    MS_UNDERBRACE   = (xub_Unicode) 0xE081,
    MS_OVERBRACE    = (xub_Unicode) 0xE082,
    MS_CIRC         = (xub_Unicode) 0x00B0,
    MS_ASSIGN       = (xub_Unicode) 0x003D,
    MS_ERROR        = (xub_Unicode) 0x00BF,

    MS_NEQ          = (xub_Unicode) 0x2260,
    MS_PLUS         = (xub_Unicode) 0xE083,
    MS_MINUS        = (xub_Unicode) 0x2212,
    MS_MULTIPLY     = (xub_Unicode) 0x2217,
    MS_TIMES        = (xub_Unicode) 0x00D7,
    MS_CDOT         = (xub_Unicode) 0x22C5,
    MS_DIV          = (xub_Unicode) 0x00F7,
    MS_PLUSMINUS    = (xub_Unicode) 0x00B1,
    MS_MINUSPLUS    = (xub_Unicode) 0x2213,
    MS_OPLUS        = (xub_Unicode) 0x2295,
    MS_OMINUS       = (xub_Unicode) 0x2296,
    MS_OTIMES       = (xub_Unicode) 0x2297,
    MS_ODIVIDE      = (xub_Unicode) 0x2298,
    MS_ODOT         = (xub_Unicode) 0x2299,
    MS_UNION        = (xub_Unicode) 0x222A,
    MS_INTERSECT    = (xub_Unicode) 0x2229,

    MS_LT           = (xub_Unicode) 0xE084,
    MS_GT           = (xub_Unicode) 0xE085,
    MS_LE           = (xub_Unicode) 0x2264,
    MS_GE           = (xub_Unicode) 0x2265,
    MS_LESLANT      = (xub_Unicode) 0xE086,
    MS_GESLANT      = (xub_Unicode) 0xE087,
    MS_LL           = (xub_Unicode) 0x226A,
    MS_GG           = (xub_Unicode) 0x226B,
    MS_SIM          = (xub_Unicode) 0x007E,
    MS_SIMEQ        = (xub_Unicode) 0x2243,
    MS_APPROX       = (xub_Unicode) 0x2248,
    MS_DEF          = (xub_Unicode) 0x225D,
    MS_EQUIV        = (xub_Unicode) 0x2261,
    MS_PROP         = (xub_Unicode) 0x221D,
    MS_PARTIAL      = (xub_Unicode) 0x2202,
    MS_SUBSET       = (xub_Unicode) 0x2282,

    MS_SUPSET       = (xub_Unicode) 0x2283,
    MS_SUBSETEQ     = (xub_Unicode) 0x2286,
    MS_SUPSETEQ     = (xub_Unicode) 0x2287,
    MS_NSUBSET      = (xub_Unicode) 0x2284,
    MS_NSUPSET      = (xub_Unicode) 0x2285,
    MS_NSUBSETEQ    = (xub_Unicode) 0x2288,
    MS_NSUPSETEQ    = (xub_Unicode) 0x2289,
    MS_IN           = (xub_Unicode) 0x2208,
    MS_NOTIN        = (xub_Unicode) 0x2209,
    MS_EXISTS       = (xub_Unicode) 0x2203,
    MS_BACKEPSILON  = (xub_Unicode) 0x220D,
    MS_ALEPH        = (xub_Unicode) 0x2135,
    MS_IM           = (xub_Unicode) 0x2111,
    MS_RE           = (xub_Unicode) 0x211C,
    MS_WP           = (xub_Unicode) 0x2118,

    MS_LINE         = (xub_Unicode) 0x2223,
    MS_DLINE        = (xub_Unicode) 0x2225,
    MS_ORTHO        = (xub_Unicode) 0x22A5,
    MS_DOTSLOW      = (xub_Unicode) 0xE08B,
    MS_DOTSAXIS     = (xub_Unicode) 0x22EF,
    MS_DOTSVERT     = (xub_Unicode) 0x22EE,
    MS_DOTSUP       = (xub_Unicode) 0x22F0,
    MS_DOTSDOWN     = (xub_Unicode) 0x22F1,
    MS_TRANSR       = (xub_Unicode) 0x22B6,
    MS_TRANSL       = (xub_Unicode) 0x22B7,
    MS_RIGHTARROW   = (xub_Unicode) 0xE08C,
    MS_BACKSLASH    = (xub_Unicode) 0x2216,
    MS_NEG          = (xub_Unicode) 0x00AC,

    MS_INT          = (xub_Unicode) 0x222B,
    MS_IINT         = (xub_Unicode) 0x222C,
    MS_IIINT        = (xub_Unicode) 0x222D,
    MS_LINT         = (xub_Unicode) 0x222E,
    MS_LLINT        = (xub_Unicode) 0x222F,
    MS_LLLINT       = (xub_Unicode) 0x2230,
    MS_SQRT         = (xub_Unicode) 0xE08D,
    MS_SQRT2        = (xub_Unicode) 0xE08F,
    MS_COPROD       = (xub_Unicode) 0x2210,
    MS_PROD         = (xub_Unicode) 0x220F,
    MS_SUM          = (xub_Unicode) 0x2211,
    MS_NABLA        = (xub_Unicode) 0x2207,
    MS_FORALL       = (xub_Unicode) 0x2200,

    MS_HAT          = (xub_Unicode) 0xE091,
    MS_CHECK        = (xub_Unicode) 0xE092,
    MS_BREVE        = (xub_Unicode) 0xE093,
    MS_ACUTE        = (xub_Unicode) 0xE094,
    MS_GRAVE        = (xub_Unicode) 0xE095,
    MS_TILDE        = (xub_Unicode) 0xE096,
    MS_BAR          = (xub_Unicode) 0xE097,
    MS_VEC          = (xub_Unicode) 0xE098,
    MS_DOT          = (xub_Unicode) 0x02D9,
    MS_DDOT         = (xub_Unicode) 0x00A8,
    MS_DDDOT        = (xub_Unicode) 0xE09B,
    MS_CIRCLE       = (xub_Unicode) 0x02DA,
    MS_AND          = (xub_Unicode) 0x2227,
    MS_OR           = (xub_Unicode) 0x2228,
    MS_NI           = (xub_Unicode) 0x220B,
    MS_EMPTYSET     = (xub_Unicode) 0x2205,

    MS_LBRACE       = (xub_Unicode) 0x007B,
    MS_RBRACE       = (xub_Unicode) 0x007D,
    MS_LPARENT      = (xub_Unicode) 0xE09E,
    MS_RPARENT      = (xub_Unicode) 0xE09F,
    MS_LANGLE       = (xub_Unicode) 0x2329,
    MS_RANGLE       = (xub_Unicode) 0x232A,
    MS_LBRACKET     = (xub_Unicode) 0x005B,
    MS_RBRACKET     = (xub_Unicode) 0x005D,

    MS_LDBRACKET    = (xub_Unicode) 0x301A,
    MS_RDBRACKET    = (xub_Unicode) 0x301B,
    MS_PLACE        = (xub_Unicode) 0xE0AA,

    MS_LCEIL        = (xub_Unicode) 0x2308,
    MS_LFLOOR       = (xub_Unicode) 0x230A,
    MS_RCEIL        = (xub_Unicode) 0x2309,
    MS_RFLOOR       = (xub_Unicode) 0x230B,
    MS_SQRT2_X      = (xub_Unicode) 0xE0AB,

    MS_TOP          = (xub_Unicode) 0xE0D9,
    MS_HBAR         = (xub_Unicode) 0x210F,
    MS_LAMBDABAR    = (xub_Unicode) 0x019B,
    MS_LEFTARROW    = (xub_Unicode) 0xE0DB,
    MS_UPARROW      = (xub_Unicode) 0xE0DC,
    MS_DOWNARROW    = (xub_Unicode) 0xE0DD,
    MS_SETN         = (xub_Unicode) 0x2115,
    MS_SETZ         = (xub_Unicode) 0x2124,
    MS_SETQ         = (xub_Unicode) 0x211A,
    MS_SETR         = (xub_Unicode) 0x211D,
    MS_SETC         = (xub_Unicode) 0x2102
};


} //namespace binfilter
#endif

