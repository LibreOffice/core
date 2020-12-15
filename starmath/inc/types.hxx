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

#ifndef INCLUDED_STARMATH_INC_TYPES_HXX
#define INCLUDED_STARMATH_INC_TYPES_HXX

#include <sal/types.h>
#define FONTNAME_MATH "OpenSymbol"

enum SmPrintSize
{
    PRINT_SIZE_NORMAL,
    PRINT_SIZE_SCALED,
    PRINT_SIZE_ZOOMED
};

// definitions for characters from the 'StarSymbol' font
// (some chars have more than one alias!)
//! Note: not listed here does not(!) mean "not used"
//!     (see %alpha ... %gamma for example)

sal_Unicode const MS_NONE = '\0';
sal_Unicode const MS_NULLCHAR = '\0';
sal_Unicode const MS_FACT = 0x0021;
sal_Unicode const MS_INFINITY = 0x221E;
sal_Unicode const MS_SLASH = 0x002F;

sal_Unicode const MS_NDIVIDES = 0x2224;
sal_Unicode const MS_DRARROW = 0x21D2;
sal_Unicode const MS_DLARROW = 0x21D0;
sal_Unicode const MS_DLRARROW = 0x21D4;
sal_Unicode const MS_OVERBRACE = 0x23DE;
sal_Unicode const MS_UNDERBRACE = 0x23DF;
sal_Unicode const MS_CIRC = 0x2218;
sal_Unicode const MS_ASSIGN = 0x003D;
sal_Unicode const MS_ERROR = 0x00BF;

sal_Unicode const MS_NEQ = 0x2260;
sal_Unicode const MS_PLUS = 0x002B;
sal_Unicode const MS_MINUS = 0x2212;
sal_Unicode const MS_MULTIPLY = 0x2217;
sal_Unicode const MS_TIMES = 0x00D7;
sal_Unicode const MS_CDOT = 0x22C5;
sal_Unicode const MS_DIV = 0x00F7;
sal_Unicode const MS_PLUSMINUS = 0x00B1;
sal_Unicode const MS_MINUSPLUS = 0x2213;
sal_Unicode const MS_OPLUS = 0x2295;
sal_Unicode const MS_OMINUS = 0x2296;
sal_Unicode const MS_OTIMES = 0x2297;
sal_Unicode const MS_ODIVIDE = 0x2298;
sal_Unicode const MS_ODOT = 0x2299;
sal_Unicode const MS_UNION = 0x222A;
sal_Unicode const MS_INTERSECT = 0x2229;

sal_Unicode const MS_LT = 0x003C;
sal_Unicode const MS_GT = 0x003E;
sal_Unicode const MS_LE = 0x2264;
sal_Unicode const MS_GE = 0x2265;
sal_Unicode const MS_LESLANT = 0x2A7D;
sal_Unicode const MS_GESLANT = 0x2A7E;
sal_Unicode const MS_LL = 0x226A;
sal_Unicode const MS_GG = 0x226B;
sal_Unicode const MS_SIM = 0x223C;
sal_Unicode const MS_SIMEQ = 0x2243;
sal_Unicode const MS_APPROX = 0x2248;
sal_Unicode const MS_DEF = 0x225D;
sal_Unicode const MS_EQUIV = 0x2261;
sal_Unicode const MS_PROP = 0x221D;
sal_Unicode const MS_PARTIAL = 0x2202;
sal_Unicode const MS_LAPLACE = 0x2112;
sal_Unicode const MS_FOURIER = 0x2131;

sal_Unicode const MS_SUBSET = 0x2282;
sal_Unicode const MS_SUPSET = 0x2283;
sal_Unicode const MS_SUBSETEQ = 0x2286;
sal_Unicode const MS_SUPSETEQ = 0x2287;
sal_Unicode const MS_NSUBSET = 0x2284;
sal_Unicode const MS_NSUPSET = 0x2285;
sal_Unicode const MS_NSUBSETEQ = 0x2288;
sal_Unicode const MS_NSUPSETEQ = 0x2289;
sal_Unicode const MS_IN = 0x2208;
sal_Unicode const MS_NOTIN = 0x2209;
sal_Unicode const MS_EXISTS = 0x2203;
sal_Unicode const MS_NOTEXISTS = 0x2204;
sal_Unicode const MS_BACKEPSILON = 0x220D;
sal_Unicode const MS_ALEPH = 0x2135;
sal_Unicode const MS_IM = 0x2111;
sal_Unicode const MS_RE = 0x211C;
sal_Unicode const MS_WP = 0x2118;

sal_Unicode const MS_LINE = 0x2223;
sal_Unicode const MS_VERTLINE = 0x007C;
sal_Unicode const MS_DLINE = 0x2225;
sal_Unicode const MS_DVERTLINE = 0x2016;
sal_Unicode const MS_ORTHO = 0x22A5;
sal_Unicode const MS_DOTSLOW = 0x2026;
sal_Unicode const MS_DOTSAXIS = 0x22EF;
sal_Unicode const MS_DOTSVERT = 0x22EE;
sal_Unicode const MS_DOTSUP = 0x22F0;
sal_Unicode const MS_DOTSDOWN = 0x22F1;
sal_Unicode const MS_TRANSR = 0x22B6;
sal_Unicode const MS_TRANSL = 0x22B7;
sal_Unicode const MS_BACKSLASH = 0x2216;
sal_Unicode const MS_NEG = 0x00AC;

sal_Unicode const MS_FORALL = 0x2200;
sal_Unicode const MS_NABLA = 0x2207;
sal_Unicode const MS_PROD = 0x220F;
sal_Unicode const MS_COPROD = 0x2210;
sal_Unicode const MS_SUM = 0x2211;
sal_Unicode const MS_SQRT = 0x221A;
sal_Unicode const MS_INT = 0x222B;
sal_Unicode const MS_IINT = 0x222C;
sal_Unicode const MS_IIINT = 0x222D;
sal_Unicode const MS_LINT = 0x222E;
sal_Unicode const MS_LLINT = 0x222F;
sal_Unicode const MS_LLLINT = 0x2230;

sal_Unicode const MS_GRAVE = 0x0060;
sal_Unicode const MS_COMBGRAVE = 0x0300;
sal_Unicode const MS_ACUTE = 0x00B4;
sal_Unicode const MS_COMBACUTE = 0x0301;
sal_Unicode const MS_HAT = 0x005E;
sal_Unicode const MS_COMBHAT = 0x0302;
sal_Unicode const MS_TILDE = 0x007E;
sal_Unicode const MS_COMBTILDE = 0x0303;
sal_Unicode const MS_BAR = 0x00AF;
sal_Unicode const MS_COMBBAR = 0x0304;
sal_Unicode const MS_COMBOVERLINE = 0x0305;
sal_Unicode const MS_BREVE = 0x02D8;
sal_Unicode const MS_COMBBREVE = 0x0306;
sal_Unicode const MS_CIRCLE = 0x02DA;
sal_Unicode const MS_COMBCIRCLE = 0x030A;
sal_Unicode const MS_CHECK = 0x02C7;
sal_Unicode const MS_COMBCHECK = 0x030C;
sal_Unicode const MS_HARPOON = 0x20D1;
sal_Unicode const MS_VEC = 0x20D7;
sal_Unicode const MS_DOT = 0x02D9;
sal_Unicode const MS_DDOT = 0x00A8;
sal_Unicode const MS_COMBDOT = 0x0307;
sal_Unicode const MS_COMBDDOT = 0x0308;
sal_Unicode const MS_DDDOT = 0x20DB;
sal_Unicode const MS_AND = 0x2227;
sal_Unicode const MS_OR = 0x2228;
sal_Unicode const MS_NI = 0x220B;
sal_Unicode const MS_EMPTYSET = 0x2205;

sal_Unicode const MS_LPARENT = 0x0028;
sal_Unicode const MS_RPARENT = 0x0029;
sal_Unicode const MS_LBRACKET = 0x005B;
sal_Unicode const MS_RBRACKET = 0x005D;
sal_Unicode const MS_LBRACE = 0x007B;
sal_Unicode const MS_RBRACE = 0x007D;
sal_Unicode const MS_LCEIL = 0x2308;
sal_Unicode const MS_RCEIL = 0x2309;
sal_Unicode const MS_LFLOOR = 0x230A;
sal_Unicode const MS_RFLOOR = 0x230B;
sal_Unicode const MS_LANGLE = 0x2329;
sal_Unicode const MS_RANGLE = 0x232A;
sal_Unicode const MS_LDBRACKET = 0x27E6;
sal_Unicode const MS_RDBRACKET = 0x27E7;
sal_Unicode const MS_LMATHANGLE = 0x27E8;
sal_Unicode const MS_RMATHANGLE = 0x27E9;

sal_Unicode const MS_PLACE = 0x2751;

sal_Unicode const MS_LAMBDABAR = 0x019B;
sal_Unicode const MS_HBAR = 0x210F;
sal_Unicode const MS_LEFTARROW = 0x2190;
sal_Unicode const MS_UPARROW = 0x2191;
sal_Unicode const MS_RIGHTARROW = 0x2192;
sal_Unicode const MS_DOWNARROW = 0x2193;

sal_Unicode const MS_SETN = 0x2115;
sal_Unicode const MS_SETZ = 0x2124;
sal_Unicode const MS_SETQ = 0x211A;
sal_Unicode const MS_SETR = 0x211D;
sal_Unicode const MS_SETC = 0x2102;

sal_Unicode const MS_PERCENT = 0x0025;

sal_Unicode const MS_PRECEDES = 0x227A;
sal_Unicode const MS_PRECEDESEQUAL = 0x227C;
sal_Unicode const MS_PRECEDESEQUIV = 0x227E;
sal_Unicode const MS_SUCCEEDS = 0x227B;
sal_Unicode const MS_SUCCEEDSEQUAL = 0x227D;
sal_Unicode const MS_SUCCEEDSEQUIV = 0x227F;
sal_Unicode const MS_NOTPRECEDES = 0x2280;
sal_Unicode const MS_NOTSUCCEEDS = 0x2281;

//mathml
sal_Unicode const MS_IT = 0x2061;
sal_Unicode const MS_AP = 0x2062;
sal_Unicode const MS_IC = 0x2063;
sal_Unicode const MS_IA = 0x2064;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
