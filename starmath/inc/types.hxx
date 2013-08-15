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

#ifndef TYPES_HXX
#define TYPES_HXX

#include <sal/types.h>
#define FONTNAME_MATH   "OpenSymbol"

/////////////////////////////////////////////////////////////////

enum SmPrintSize { PRINT_SIZE_NORMAL, PRINT_SIZE_SCALED, PRINT_SIZE_ZOOMED };

/////////////////////////////////////////////////////////////////

inline sal_Bool IsInPrivateUseArea( sal_Unicode cChar ) { return 0xE000 <= cChar  &&  cChar <= 0xF8FF; }

// Greek char range as seen in svx/.../charmap.cxx RID_SUBSETSTR_BASIC_GREEK
inline sal_Bool IsGreekChar( sal_Unicode cChar ) { return 0x0370 <= cChar  &&  cChar <= 0x03FF; }

sal_Unicode ConvertMathToMathML( sal_Unicode cChar );

/////////////////////////////////////////////////////////////////
// enum definitions for characters from the 'StarSymbol' font
// (some chars have more than one alias!)
//! Note: not listed here does not(!) mean "not used"
//!     (see %alpha ... %gamma for example)
//
enum MathSymbol
{
    MS_FACT         = (sal_Unicode) 0x0021,
    MS_INFINITY     = (sal_Unicode) 0x221E,
    MS_SLASH        = (sal_Unicode) 0x002F,

    MS_NDIVIDES     = (sal_Unicode) 0x2224,
    MS_DRARROW      = (sal_Unicode) 0x21D2,
    MS_DLARROW      = (sal_Unicode) 0x21D0,
    MS_DLRARROW     = (sal_Unicode) 0x21D4,
    MS_OVERBRACE    = (sal_Unicode) 0x23DE,
    MS_UNDERBRACE   = (sal_Unicode) 0x23DF,
    MS_CIRC         = (sal_Unicode) 0x2218,
    MS_ASSIGN       = (sal_Unicode) 0x003D,
    MS_ERROR        = (sal_Unicode) 0x00BF,

    MS_NEQ          = (sal_Unicode) 0x2260,
    MS_PLUS         = (sal_Unicode) 0x002B,
    MS_MINUS        = (sal_Unicode) 0x2212,
    MS_MULTIPLY     = (sal_Unicode) 0x2217,
    MS_TIMES        = (sal_Unicode) 0x00D7,
    MS_CDOT         = (sal_Unicode) 0x22C5,
    MS_DIV          = (sal_Unicode) 0x00F7,
    MS_PLUSMINUS    = (sal_Unicode) 0x00B1,
    MS_MINUSPLUS    = (sal_Unicode) 0x2213,
    MS_OPLUS        = (sal_Unicode) 0x2295,
    MS_OMINUS       = (sal_Unicode) 0x2296,
    MS_OTIMES       = (sal_Unicode) 0x2297,
    MS_ODIVIDE      = (sal_Unicode) 0x2298,
    MS_ODOT         = (sal_Unicode) 0x2299,
    MS_UNION        = (sal_Unicode) 0x222A,
    MS_INTERSECT    = (sal_Unicode) 0x2229,

    MS_LT           = (sal_Unicode) 0x003C,
    MS_GT           = (sal_Unicode) 0x003E,
    MS_LE           = (sal_Unicode) 0x2264,
    MS_GE           = (sal_Unicode) 0x2265,
    MS_LESLANT      = (sal_Unicode) 0x2A7D,
    MS_GESLANT      = (sal_Unicode) 0x2A7E,
    MS_LL           = (sal_Unicode) 0x226A,
    MS_GG           = (sal_Unicode) 0x226B,
    MS_SIM          = (sal_Unicode) 0x223C,
    MS_SIMEQ        = (sal_Unicode) 0x2243,
    MS_APPROX       = (sal_Unicode) 0x2248,
    MS_DEF          = (sal_Unicode) 0x225D,
    MS_EQUIV        = (sal_Unicode) 0x2261,
    MS_PROP         = (sal_Unicode) 0x221D,
    MS_PARTIAL      = (sal_Unicode) 0x2202,

    MS_SUBSET       = (sal_Unicode) 0x2282,
    MS_SUPSET       = (sal_Unicode) 0x2283,
    MS_SUBSETEQ     = (sal_Unicode) 0x2286,
    MS_SUPSETEQ     = (sal_Unicode) 0x2287,
    MS_NSUBSET      = (sal_Unicode) 0x2284,
    MS_NSUPSET      = (sal_Unicode) 0x2285,
    MS_NSUBSETEQ    = (sal_Unicode) 0x2288,
    MS_NSUPSETEQ    = (sal_Unicode) 0x2289,
    MS_IN           = (sal_Unicode) 0x2208,
    MS_NOTIN        = (sal_Unicode) 0x2209,
    MS_EXISTS       = (sal_Unicode) 0x2203,
    MS_NOTEXISTS    = (sal_Unicode) 0x2204,
    MS_BACKEPSILON  = (sal_Unicode) 0x220D,
    MS_ALEPH        = (sal_Unicode) 0x2135,
    MS_IM           = (sal_Unicode) 0x2111,
    MS_RE           = (sal_Unicode) 0x211C,
    MS_WP           = (sal_Unicode) 0x2118,

    MS_LINE         = (sal_Unicode) 0x2223,
    MS_VERTLINE     = (sal_Unicode) 0x007C,
    MS_DLINE        = (sal_Unicode) 0x2225,
    MS_DVERTLINE    = (sal_Unicode) 0x2016,
    MS_ORTHO        = (sal_Unicode) 0x22A5,
    MS_DOTSLOW      = (sal_Unicode) 0x2026,
    MS_DOTSAXIS     = (sal_Unicode) 0x22EF,
    MS_DOTSVERT     = (sal_Unicode) 0x22EE,
    MS_DOTSUP       = (sal_Unicode) 0x22F0,
    MS_DOTSDOWN     = (sal_Unicode) 0x22F1,
    MS_TRANSR       = (sal_Unicode) 0x22B6,
    MS_TRANSL       = (sal_Unicode) 0x22B7,
    MS_BACKSLASH    = (sal_Unicode) 0x2216,
    MS_NEG          = (sal_Unicode) 0x00AC,

    MS_FORALL       = (sal_Unicode) 0x2200,
    MS_NABLA        = (sal_Unicode) 0x2207,
    MS_PROD         = (sal_Unicode) 0x220F,
    MS_COPROD       = (sal_Unicode) 0x2210,
    MS_SUM          = (sal_Unicode) 0x2211,
    MS_SQRT         = (sal_Unicode) 0x221A,
    MS_INT          = (sal_Unicode) 0x222B,
    MS_IINT         = (sal_Unicode) 0x222C,
    MS_IIINT        = (sal_Unicode) 0x222D,
    MS_LINT         = (sal_Unicode) 0x222E,
    MS_LLINT        = (sal_Unicode) 0x222F,
    MS_LLLINT       = (sal_Unicode) 0x2230,

    MS_GRAVE        = (sal_Unicode) 0x0060,
    MS_COMBGRAVE    = (sal_Unicode) 0x0300,
    MS_ACUTE        = (sal_Unicode) 0x00B4,
    MS_COMBACUTE    = (sal_Unicode) 0x0301,
    MS_HAT          = (sal_Unicode) 0x005E,
    MS_COMBHAT      = (sal_Unicode) 0x0302,
    MS_TILDE        = (sal_Unicode) 0x007E,
    MS_COMBTILDE    = (sal_Unicode) 0x0303,
    MS_BAR          = (sal_Unicode) 0x00AF,
    MS_COMBBAR      = (sal_Unicode) 0x0304,
    MS_BREVE        = (sal_Unicode) 0x02D8,
    MS_COMBBREVE    = (sal_Unicode) 0x0306,
    MS_CIRCLE       = (sal_Unicode) 0x02DA,
    MS_COMBCIRCLE   = (sal_Unicode) 0x030A,
    MS_CHECK        = (sal_Unicode) 0x02C7,
    MS_COMBCHECK    = (sal_Unicode) 0x030C,
    MS_VEC          = (sal_Unicode) 0x20D7,
    MS_DOT          = (sal_Unicode) 0x02D9,
    MS_DDOT         = (sal_Unicode) 0x00A8,
    MS_DDDOT        = (sal_Unicode) 0x20DB,
    MS_AND          = (sal_Unicode) 0x2227,
    MS_OR           = (sal_Unicode) 0x2228,
    MS_NI           = (sal_Unicode) 0x220B,
    MS_EMPTYSET     = (sal_Unicode) 0x2205,

    MS_LPARENT      = (sal_Unicode) 0x0028,
    MS_RPARENT      = (sal_Unicode) 0x0029,
    MS_LBRACKET     = (sal_Unicode) 0x005B,
    MS_RBRACKET     = (sal_Unicode) 0x005D,
    MS_LBRACE       = (sal_Unicode) 0x007B,
    MS_RBRACE       = (sal_Unicode) 0x007D,
    MS_LCEIL        = (sal_Unicode) 0x2308,
    MS_RCEIL        = (sal_Unicode) 0x2309,
    MS_LFLOOR       = (sal_Unicode) 0x230A,
    MS_RFLOOR       = (sal_Unicode) 0x230B,
    MS_LANGLE       = (sal_Unicode) 0x2329,
    MS_RANGLE       = (sal_Unicode) 0x232A,
    MS_LDBRACKET    = (sal_Unicode) 0x27E6,
    MS_RDBRACKET    = (sal_Unicode) 0x27E7,
    MS_LMATHANGLE   = (sal_Unicode) 0x27E8,
    MS_RMATHANGLE   = (sal_Unicode) 0x27E9,

    MS_PLACE        = (sal_Unicode) 0x2751,

    MS_LAMBDABAR    = (sal_Unicode) 0x019B,
    MS_HBAR         = (sal_Unicode) 0x210F,
    MS_LEFTARROW    = (sal_Unicode) 0x2190,
    MS_UPARROW      = (sal_Unicode) 0x2191,
    MS_RIGHTARROW   = (sal_Unicode) 0x2192,
    MS_DOWNARROW    = (sal_Unicode) 0x2193,
    MS_SETN         = (sal_Unicode) 0x2115,
    MS_SETZ         = (sal_Unicode) 0x2124,
    MS_SETQ         = (sal_Unicode) 0x211A,
    MS_SETR         = (sal_Unicode) 0x211D,
    MS_SETC         = (sal_Unicode) 0x2102,

    MS_PERCENT      = (sal_Unicode) 0x0025,

    MS_PRECEDES     = (sal_Unicode) 0x227A,
    MS_PRECEDESEQUAL= (sal_Unicode) 0x227C,
    MS_PRECEDESEQUIV= (sal_Unicode) 0x227E,
    MS_SUCCEEDS     = (sal_Unicode) 0x227B,
    MS_SUCCEEDSEQUAL= (sal_Unicode) 0x227D,
    MS_SUCCEEDSEQUIV= (sal_Unicode) 0x227F,
    MS_NOTPRECEDES  = (sal_Unicode) 0x2280,
    MS_NOTSUCCEEDS  = (sal_Unicode) 0x2281,
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
