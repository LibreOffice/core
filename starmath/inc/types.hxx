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
#define FALL_FONTNAME_MATH   "OpenSymbol"


enum SmPrintSize { PRINT_SIZE_NORMAL, PRINT_SIZE_SCALED, PRINT_SIZE_ZOOMED };


// definitions for characters from the 'StarSymbol' font
// (some chars have more than one alias!)
//! Note: not listed here does not(!) mean "not used"
//!     (see %alpha ... %gamma for example)

sal_Unicode const MS_FACT = 0x0021;
sal_Unicode const MS_INFINITY = 0x221E;
sal_Unicode const MS_SLASH = 0x002F;
sal_Unicode const MS_NONE = '\0';

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

// Intenational
sal_Unicode const MS_UC_CEDILLA        = 0x00C7;
sal_Unicode const MS_LC_CEDILLA        = 0x00E7;
sal_Unicode const MS_UC_CEDILLA_GOTHIC = 0xA762;
sal_Unicode const MS_LC_CEDILLA_GOTHIC = 0xA763;
sal_Unicode const MS_UN_TILDE          = 0x00D1;
sal_Unicode const MS_LN_TILDE          = 0x00E6;
sal_Unicode const MS_UAE               = 0x00C6;
sal_Unicode const MS_LAE               = 0x00E6;
sal_Unicode const MS_UOE               = 0x0152;
sal_Unicode const MS_LOE               = 0x0153;
sal_Unicode const MS_PILCROW           = 0x00B6;
sal_Unicode const MS_TM                = 0x2122;
sal_Unicode const MS_COPYRIGHT         = 0x00A9;
sal_Unicode const MS_COREGISTERED      = 0x00AE;
sal_Unicode const MS_MORDINALI         = 0x00BA;
sal_Unicode const MS_FORDINALI         = 0x00AA;
sal_Unicode const MS_NUMBER            = 0x0023;
sal_Unicode const MS_AMPERSAND         = 0x0026;
sal_Unicode const MS_DEGREE            = 0x00B0;
sal_Unicode const MS_AMSTRONG          = 0x00C5;
sal_Unicode const MS_DEATH             = 0xE429;

// Currency
sal_Unicode const MS_CURRENCY          = 0x00A4;
sal_Unicode const MS_COLON             = 0x20A1;
sal_Unicode const MS_CRUZEIRO          = 0x20A2;
sal_Unicode const MS_FRANC             = 0x20A3;
sal_Unicode const MS_LIRA              = 0x20A4;
sal_Unicode const MS_RUPEE             = 0x20A8;
sal_Unicode const MS_WON               = 0x20A9;
sal_Unicode const MS_DONG              = 0x20AB;
sal_Unicode const MS_YEN               = 0x00A5;
sal_Unicode const MS_EURO              = 0x20AC;
sal_Unicode const MS_CENT              = 0x00A2;
sal_Unicode const MS_DOLLAR            = 0x0024;
sal_Unicode const MS_PERCENT           = 0x0025;
sal_Unicode const MS_PERTHOUSAND       = 0xE080;
sal_Unicode const MS_PERTENTHOUSAND    = 0x2031;

// Greek
sal_Unicode const MS_UBEPSILON         = 0x220D;
sal_Unicode const MS_UALPHA            = 0x0391;
sal_Unicode const MS_UBETA             = 0x0392;
sal_Unicode const MS_UGAMMA            = 0x0393;
sal_Unicode const MS_UDELTA            = 0x0394;
sal_Unicode const MS_UEPSILON          = 0x0395;
sal_Unicode const MS_UZETA             = 0x0396;
sal_Unicode const MS_UETA              = 0x0397;
sal_Unicode const MS_UTHETA            = 0x0398;
sal_Unicode const MS_UIOTA             = 0x0399;
sal_Unicode const MS_UKAPPA            = 0x039A;
sal_Unicode const MS_ULAMBDA           = 0x039B;
sal_Unicode const MS_UMU               = 0x039C;
sal_Unicode const MS_UNU               = 0x039D;
sal_Unicode const MS_UXI               = 0x039E;
sal_Unicode const MS_UOMICRON          = 0x039F;
sal_Unicode const MS_UPI               = 0x03A0;
sal_Unicode const MS_URHO              = 0x03A1;
sal_Unicode const MS_USIGMA            = 0x03A3;
sal_Unicode const MS_UTAU              = 0x03A4;
sal_Unicode const MS_UUPSILON          = 0x03A5;
sal_Unicode const MS_UPHI              = 0x03A6;
sal_Unicode const MS_UCHI              = 0x03A7;
sal_Unicode const MS_UPSI              = 0x03A8;
sal_Unicode const MS_UOMEGA            = 0x03A9;
sal_Unicode const MS_LALPHA            = 0x03B1;
sal_Unicode const MS_LBETA             = 0x03B2;
sal_Unicode const MS_LGAMMA            = 0x03B3;
sal_Unicode const MS_LDELTA            = 0x03B4;
sal_Unicode const MS_LEPSILON          = 0x03B5;
sal_Unicode const MS_LZETA             = 0x03B6;
sal_Unicode const MS_LETA              = 0x03B7;
sal_Unicode const MS_LTHETA            = 0x03B8;
sal_Unicode const MS_LIOTA             = 0x03B9;
sal_Unicode const MS_LKAPPA            = 0x03BA;
sal_Unicode const MS_LLAMBDA           = 0x03BB;
sal_Unicode const MS_LMU               = 0x03BC;
sal_Unicode const MS_LNU               = 0x03BD;
sal_Unicode const MS_LXI               = 0x03BE;
sal_Unicode const MS_LOMICRON          = 0x03BF;
sal_Unicode const MS_LPI               = 0x03C0;
sal_Unicode const MS_LRHO              = 0x03C1;
sal_Unicode const MS_LSIGMA            = 0x03C3;
sal_Unicode const MS_OSIGMA            = 0x03C3;
sal_Unicode const MS_LTAU              = 0x03C4;
sal_Unicode const MS_LUPSILON          = 0x03C5;
sal_Unicode const MS_LPHI              = 0x03C6;
sal_Unicode const MS_LCHI              = 0x03C7;
sal_Unicode const MS_LPSI              = 0x03C8;
sal_Unicode const MS_LOMEGA            = 0x03C9;

// Coptic
// TODO implement this
// https://en.wikipedia.org/wiki/Coptic_alphabet

// Sets
sal_Unicode32 const MS_LSETA           = 0x0001D552;
sal_Unicode32 const MS_LSETB           = 0x0001D553;
sal_Unicode32 const MS_LSETC           = 0x0001D554;
sal_Unicode32 const MS_LSETD           = 0x0001D555;
sal_Unicode32 const MS_LSETE           = 0x0001D556;
sal_Unicode32 const MS_LSETF           = 0x0001D557;
sal_Unicode32 const MS_LSETG           = 0x0001D558;
sal_Unicode32 const MS_LSETH           = 0x0001D559;
sal_Unicode32 const MS_LSETI           = 0x0001D55A;
sal_Unicode32 const MS_LSETJ           = 0x0001D55B;
sal_Unicode32 const MS_LSETK           = 0x0001D55C;
sal_Unicode32 const MS_LSETL           = 0x0001D55D;
sal_Unicode32 const MS_LSETM           = 0x0001D55E;
sal_Unicode32 const MS_LSETN           = 0x0001D55F;
sal_Unicode32 const MS_LSETO           = 0x0001D560;
sal_Unicode32 const MS_LSETP           = 0x0001D561;
sal_Unicode32 const MS_LSETQ           = 0x0001D562;
sal_Unicode32 const MS_LSETR           = 0x0001D563;
sal_Unicode32 const MS_LSETS           = 0x0001D564;
sal_Unicode32 const MS_LSETT           = 0x0001D565;
sal_Unicode32 const MS_LSETU           = 0x0001D566;
sal_Unicode32 const MS_LSETV           = 0x0001D567;
sal_Unicode32 const MS_LSETW           = 0x0001D568;
sal_Unicode32 const MS_LSETX           = 0x0001D569;
sal_Unicode32 const MS_LSETY           = 0x0001D56A;
sal_Unicode32 const MS_LSETZ           = 0x0001D56B;
sal_Unicode32 const MS_USETA           = 0x0001D538;
sal_Unicode32 const MS_USETB           = 0x0001D539;
sal_Unicode32 const MS_USETC           = 0x00002102;
sal_Unicode32 const MS_USETD           = 0x0001D53B;
sal_Unicode32 const MS_USETE           = 0x0001D53C;
sal_Unicode32 const MS_USETF           = 0x0001D53D;
sal_Unicode32 const MS_USETG           = 0x0001D53E;
sal_Unicode32 const MS_USETH           = 0x0000210D;
sal_Unicode32 const MS_USETI           = 0x0001D540;
sal_Unicode32 const MS_USETJ           = 0x0001D541;
sal_Unicode32 const MS_USETK           = 0x0001D542;
sal_Unicode32 const MS_USETL           = 0x0001D543;
sal_Unicode32 const MS_USETM           = 0x0001D544;
sal_Unicode32 const MS_USETN           = 0x00002115;
sal_Unicode32 const MS_USETO           = 0x0001D546;
sal_Unicode32 const MS_USETP           = 0x00002119;
sal_Unicode32 const MS_USETQ           = 0x0000211A;
sal_Unicode32 const MS_USETR           = 0x0000211D;
sal_Unicode32 const MS_USETS           = 0x0001D54A;
sal_Unicode32 const MS_USETT           = 0x0001D54B;
sal_Unicode32 const MS_USETU           = 0x0001D54C;
sal_Unicode32 const MS_USETV           = 0x0001D54D;
sal_Unicode32 const MS_USETW           = 0x0001D54E;
sal_Unicode32 const MS_USETX           = 0x0001D54F;
sal_Unicode32 const MS_USETY           = 0x0001D550;
sal_Unicode32 const MS_USETZ           = 0x00002124;
sal_Unicode32 const MS_SET0            = 0x0001D7D8;
sal_Unicode32 const MS_SET1            = 0x0001D7D9;
sal_Unicode32 const MS_SET2            = 0x0001D7DA;
sal_Unicode32 const MS_SET3            = 0x0001D7DB;
sal_Unicode32 const MS_SET4            = 0x0001D7DC;
sal_Unicode32 const MS_SET5            = 0x0001D7DD;
sal_Unicode32 const MS_SET6            = 0x0001D7DE;
sal_Unicode32 const MS_SET7            = 0x0001D7DF;
sal_Unicode32 const MS_SET8            = 0x0001D7E0;
sal_Unicode32 const MS_SET9            = 0x0001D7E1;
sal_Unicode32 const MS_EMPTYSET        = 0x00002205;
sal_Unicode32 const MS_USET_GAMMA      = 0x0000213E;
sal_Unicode32 const MS_LSET_GAMMA      = 0x0000213D;
sal_Unicode32 const MS_USET_PI         = 0x0000213F;
sal_Unicode32 const MS_LSET_PI         = 0x0000213C;
sal_Unicode32 const MS_USET_SIGMA      = 0x00002140;

sal_Unicode32 const MS_UDIFF           = 0x00002145;
sal_Unicode32 const MS_LDIFF           = 0x00002146;
sal_Unicode32 const MS_EULER           = 0x00002147;
sal_Unicode32 const MS_ICOMPLEX        = 0x00002148;
sal_Unicode32 const MS_JCOMPLEX        = 0x00002149;

sal_Unicode const MS_PRECEDES = 0x227A;
sal_Unicode const MS_PRECEDESEQUAL = 0x227C;
sal_Unicode const MS_PRECEDESEQUIV = 0x227E;
sal_Unicode const MS_SUCCEEDS = 0x227B;
sal_Unicode const MS_SUCCEEDSEQUAL = 0x227D;
sal_Unicode const MS_SUCCEEDSEQUIV = 0x227F;
sal_Unicode const MS_NOTPRECEDES = 0x2280;
sal_Unicode const MS_NOTSUCCEEDS = 0x2281;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
