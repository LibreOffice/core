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
#define FONTNAME_MATH   "OpenSymbol"


enum SmPrintSize { PRINT_SIZE_NORMAL, PRINT_SIZE_SCALED, PRINT_SIZE_ZOOMED };


// definitions for characters from the 'StarSymbol' font
// (some chars have more than one alias!)
//! Note: not listed here does not(!) mean "not used"
//!     (see %alpha ... %gamma for example)
// Some characters will use unicode32.
// Some characters may be combined ones.
// Combining: http://unicode.org/faq/char_combmark.html








// Coptic
// TODO implement this
// https://en.wikipedia.org/wiki/Coptic_alphabet

sal_Unicode   const MS_NONE                = 0x0000;

/**
  * Mathtypes2
  */

// International
sal_Unicode32 const MS_UAE                 = 0x000000C6;
sal_Unicode32 const MS_UAO                 = 0x0000A734;
sal_Unicode32 const MS_UOE                 = 0x00000152;
sal_Unicode32 const MS_UOO                 = 0x0000A74E;
sal_Unicode32 const MS_UAA                 = 0x0000A732;
sal_Unicode32 const MS_UCCEDILLE           = 0x000000C7;
sal_Unicode32 const MS_UGCCEDILLE          = 0x0000A762;
sal_Unicode32 const MS_UTILDEN             = 0x000000D1;
sal_Unicode32 const MS_LAE                 = 0x000000E6;
sal_Unicode32 const MS_LOE                 = 0x00000153;
sal_Unicode32 const MS_LAO                 = 0x0000A735;
sal_Unicode32 const MS_LOO                 = 0x0000A74F;
sal_Unicode32 const MS_LAA                 = 0x0000A733;
sal_Unicode32 const MS_LQP                 = 0x00000239;
sal_Unicode32 const MS_LDB                 = 0x00000238;
sal_Unicode32 const MS_LCCEDILLE           = 0x000000E7;
sal_Unicode32 const MS_LGCCEDILLE          = 0x0000A763;
sal_Unicode32 const MS_LTILDEN             = 0x000000F1;

/**
  * Fine blackboard
  */

// Sets
sal_Unicode32 const MS_USETC               = 0x00002102;
sal_Unicode32 const MS_USETQ               = 0x0000211A;
sal_Unicode32 const MS_USETN               = 0x00002115;
sal_Unicode32 const MS_USETR               = 0x0000211D;
sal_Unicode32 const MS_USETZ               = 0x00002124;
sal_Unicode32 const MS_DS_LA               = 0x0001D552;
sal_Unicode32 const MS_DS_LB               = 0x0001D553;
sal_Unicode32 const MS_DS_LC               = 0x0001D554;
sal_Unicode32 const MS_DS_LD               = 0x0001D555;
sal_Unicode32 const MS_DS_LE               = 0x0001D556;
sal_Unicode32 const MS_DS_LF               = 0x0001D557;
sal_Unicode32 const MS_DS_LG               = 0x0001D558;
sal_Unicode32 const MS_DS_LH               = 0x0001D559;
sal_Unicode32 const MS_DS_LI               = 0x0001D55A;
sal_Unicode32 const MS_DS_LJ               = 0x0001D55B;
sal_Unicode32 const MS_DS_LK               = 0x0001D55C;
sal_Unicode32 const MS_DS_LL               = 0x0001D55D;
sal_Unicode32 const MS_DS_LM               = 0x0001D55E;
sal_Unicode32 const MS_DS_LN               = 0x0001D55F;
sal_Unicode32 const MS_DS_LO               = 0x0001D560;
sal_Unicode32 const MS_DS_LP               = 0x0001D561;
sal_Unicode32 const MS_DS_LQ               = 0x0001D562;
sal_Unicode32 const MS_DS_LR               = 0x0001D563;
sal_Unicode32 const MS_DS_LS               = 0x0001D564;
sal_Unicode32 const MS_DS_LT               = 0x0001D565;
sal_Unicode32 const MS_DS_LU               = 0x0001D566;
sal_Unicode32 const MS_DS_LV               = 0x0001D567;
sal_Unicode32 const MS_DS_LW               = 0x0001D568;
sal_Unicode32 const MS_DS_LX               = 0x0001D569;
sal_Unicode32 const MS_DS_LY               = 0x0001D56A;
sal_Unicode32 const MS_DS_LZ               = 0x0001D56B;
sal_Unicode32 const MS_DS_UA               = 0x0001D538;
sal_Unicode32 const MS_DS_UB               = 0x0001D539;
sal_Unicode32 const MS_DS_UC               = 0x00002102;
sal_Unicode32 const MS_DS_UD               = 0x0001D53B;
sal_Unicode32 const MS_DS_UE               = 0x0001D53C;
sal_Unicode32 const MS_DS_UF               = 0x0001D53D;
sal_Unicode32 const MS_DS_UG               = 0x0001D53E;
sal_Unicode32 const MS_DS_UH               = 0x0000210D;
sal_Unicode32 const MS_DS_UI               = 0x0001D540;
sal_Unicode32 const MS_DS_UJ               = 0x0001D541;
sal_Unicode32 const MS_DS_UK               = 0x0001D542;
sal_Unicode32 const MS_DS_UL               = 0x0001D543;
sal_Unicode32 const MS_DS_UM               = 0x0001D544;
sal_Unicode32 const MS_DS_UN               = 0x00002115;
sal_Unicode32 const MS_DS_UO               = 0x0001D546;
sal_Unicode32 const MS_DS_UP               = 0x00002119;
sal_Unicode32 const MS_DS_UQ               = 0x0000211A;
sal_Unicode32 const MS_DS_UR               = 0x0000211D;
sal_Unicode32 const MS_DS_US               = 0x0001D54A;
sal_Unicode32 const MS_DS_UT               = 0x0001D54B;
sal_Unicode32 const MS_DS_UU               = 0x0001D54C;
sal_Unicode32 const MS_DS_UV               = 0x0001D54D;
sal_Unicode32 const MS_DS_UW               = 0x0001D54E;
sal_Unicode32 const MS_DS_UX               = 0x0001D54F;
sal_Unicode32 const MS_DS_UY               = 0x0001D550;
sal_Unicode32 const MS_DS_UZ               = 0x00002124;
// Other sets
sal_Unicode32 const MS_EMPTYSET            = 0x00002205;
sal_Unicode32 const MS_DS_UGAMMA           = 0x0000213E;
sal_Unicode32 const MS_DS_LGAMMA           = 0x0000213D;
sal_Unicode32 const MS_DS_UPI              = 0x0000213F;
sal_Unicode32 const MS_DS_LPI              = 0x0000213C;
sal_Unicode32 const MS_DS_USIGMA           = 0x00002140;

/**
  * Currency
  */

// Currency
sal_Unicode32 const MS_CURRENCY            = 0x000000A4;
sal_Unicode32 const MS_COLON               = 0x000020A1;
sal_Unicode32 const MS_CRUZEIRO            = 0x000020A2;
sal_Unicode32 const MS_FRANC               = 0x000020A3;
sal_Unicode32 const MS_LIRA                = 0x000020A4;
sal_Unicode32 const MS_RUPEE               = 0x000020A8;
sal_Unicode32 const MS_WON                 = 0x000020A9;
sal_Unicode32 const MS_DONG                = 0x000020AB;
sal_Unicode32 const MS_YEN                 = 0x000000A5;
sal_Unicode32 const MS_EURO                = 0x000020AC;
sal_Unicode32 const MS_CENT                = 0x000000A2;
sal_Unicode32 const MS_DOLLAR              = 0x00000024;
sal_Unicode32 const MS_CDRAM               = 0x0000058F;

/**
  * Greek
  */

// Greek Capital
sal_Unicode32 const MS_UBEPSILON           = 0x0000220D;
sal_Unicode32 const MS_UALPHA              = 0x00000391;
sal_Unicode32 const MS_UBETA               = 0x00000392;
sal_Unicode32 const MS_UGAMMA              = 0x00000393;
sal_Unicode32 const MS_UDELTA              = 0x00000394;
sal_Unicode32 const MS_UEPSILON            = 0x00000395;
sal_Unicode32 const MS_UZETA               = 0x00000396;
sal_Unicode32 const MS_UETA                = 0x00000397;
sal_Unicode32 const MS_UTHETA              = 0x00000398;
sal_Unicode32 const MS_UIOTA               = 0x00000399;
sal_Unicode32 const MS_UKAPPA              = 0x0000039A;
sal_Unicode32 const MS_ULAMBDA             = 0x0000039B;
sal_Unicode32 const MS_UMU                 = 0x0000039C;
sal_Unicode32 const MS_UNU                 = 0x0000039D;
sal_Unicode32 const MS_UXI                 = 0x0000039E;
sal_Unicode32 const MS_UOMICRON            = 0x0000039F;
sal_Unicode32 const MS_UPI                 = 0x000003A0;
sal_Unicode32 const MS_URHO                = 0x000003A1;
sal_Unicode32 const MS_UTHETAS             = 0x000003F4;
sal_Unicode32 const MS_USIGMA              = 0x000003A3;
sal_Unicode32 const MS_UTAU                = 0x000003A4;
sal_Unicode32 const MS_UUPSILON            = 0x000003A5;
sal_Unicode32 const MS_UPHI                = 0x000003A6;
sal_Unicode32 const MS_UCHI                = 0x000003A7;
sal_Unicode32 const MS_UPSI                = 0x000003A8;
sal_Unicode32 const MS_UOMEGA              = 0x000003A9;

// Greek Lower case
sal_Unicode32 const MS_LALPHA              = 0x000003B1;
sal_Unicode32 const MS_LBETA               = 0x000003B2;
sal_Unicode32 const MS_LGAMMA              = 0x000003B3;
sal_Unicode32 const MS_LDELTA              = 0x000003B4;
sal_Unicode32 const MS_LEPSILON            = 0x000003B5;
sal_Unicode32 const MS_LZETA               = 0x000003B6;
sal_Unicode32 const MS_LETA                = 0x000003B7;
sal_Unicode32 const MS_LTHETA              = 0x000003B8;
sal_Unicode32 const MS_LIOTA               = 0x000003B9;
sal_Unicode32 const MS_LKAPPA              = 0x000003BA;
sal_Unicode32 const MS_LLAMBDA             = 0x000003BB;
sal_Unicode32 const MS_LMU                 = 0x000003BC;
sal_Unicode32 const MS_LNU                 = 0x000003BD;
sal_Unicode32 const MS_LXI                 = 0x000003BE;
sal_Unicode32 const MS_LOMICRON            = 0x000003BF;
sal_Unicode32 const MS_LPI                 = 0x000003C0;
sal_Unicode32 const MS_LRHO                = 0x000003C1;
sal_Unicode32 const MS_LSIGMA              = 0x000003C3;
sal_Unicode32 const MS_OSIGMA              = 0x000003C2;
sal_Unicode32 const MS_LTAU                = 0x000003C4;
sal_Unicode32 const MS_LUPSILON            = 0x000003C5;
sal_Unicode32 const MS_LPHI                = 0x000003C6;
sal_Unicode32 const MS_LCHI                = 0x000003C7;
sal_Unicode32 const MS_LPSI                = 0x000003C8;
sal_Unicode32 const MS_LOMEGA              = 0x000003C9;
sal_Unicode32 const MS_EPSILONLUNATE       = 0x000003F5;
sal_Unicode32 const MS_LTHETAS             = 0x000003D1;
sal_Unicode32 const MS_LKAPPAS             = 0x000003F0;
sal_Unicode32 const MS_LPHIS               = 0x000003D5;
sal_Unicode32 const MS_LRHOS               = 0x000003F1;
sal_Unicode32 const MS_LPIS                = 0x000003D6;

// Greek capital bold
sal_Unicode32 const MS_B_UALPHA            = 0x0001D6A8;
sal_Unicode32 const MS_B_UBETA             = 0x0001D6A9;
sal_Unicode32 const MS_B_UGAMMA            = 0x0001D6AA;
sal_Unicode32 const MS_B_UDELTA            = 0x0001D6AB;
sal_Unicode32 const MS_B_UEPSILON          = 0x0001D6AC;
sal_Unicode32 const MS_B_UZETA             = 0x0001D6AD;
sal_Unicode32 const MS_B_UETA              = 0x0001D6AE;
sal_Unicode32 const MS_B_UTHETA            = 0x0001D6AF;
sal_Unicode32 const MS_B_UIOTA             = 0x0001D6B0;
sal_Unicode32 const MS_B_UKAPPA            = 0x0001D6B1;
sal_Unicode32 const MS_B_ULAMBDA           = 0x0001D6B2;
sal_Unicode32 const MS_B_UMU               = 0x0001D6B3;
sal_Unicode32 const MS_B_UNU               = 0x0001D6B4;
sal_Unicode32 const MS_B_UXI               = 0x0001D6B5;
sal_Unicode32 const MS_B_UOMICRON          = 0x0001D6B6;
sal_Unicode32 const MS_B_UPI               = 0x0001D6B7;
sal_Unicode32 const MS_B_URHO              = 0x0001D6B8;
sal_Unicode32 const MS_B_UTHETAS           = 0x0001D6B9;
sal_Unicode32 const MS_B_USIGMA            = 0x0001D6BA;
sal_Unicode32 const MS_B_UTAU              = 0x0001D6BB;
sal_Unicode32 const MS_B_UUPSILON          = 0x0001D6BC;
sal_Unicode32 const MS_B_UPHI              = 0x0001D6BD;
sal_Unicode32 const MS_B_UCHI              = 0x0001D6BE;
sal_Unicode32 const MS_B_UPSI              = 0x0001D6BF;
sal_Unicode32 const MS_B_UOMEGA            = 0x0001D6C0;
sal_Unicode32 const MS_B_NABLA             = 0x0001D6C1;

// Greek Lower case bold
sal_Unicode32 const MS_B_LALPHA            = 0x0001D6C2;
sal_Unicode32 const MS_B_LBETA             = 0x0001D6C3;
sal_Unicode32 const MS_B_LGAMMA            = 0x0001D6C4;
sal_Unicode32 const MS_B_LDELTA            = 0x0001D6C5;
sal_Unicode32 const MS_B_LEPSILON          = 0x0001D6C6;
sal_Unicode32 const MS_B_LZETA             = 0x0001D6C7;
sal_Unicode32 const MS_B_LETA              = 0x0001D6C8;
sal_Unicode32 const MS_B_LTHETA            = 0x0001D6C9;
sal_Unicode32 const MS_B_LIOTA             = 0x0001D6CA;
sal_Unicode32 const MS_B_LKAPPA            = 0x0001D6CB;
sal_Unicode32 const MS_B_LLAMBDA           = 0x0001D6CC;
sal_Unicode32 const MS_B_LMU               = 0x0001D6CD;
sal_Unicode32 const MS_B_LNU               = 0x0001D6CE;
sal_Unicode32 const MS_B_LXI               = 0x0001D6CF;
sal_Unicode32 const MS_B_LOMICRON          = 0x0001D6D0;
sal_Unicode32 const MS_B_LPI               = 0x0001D6D1;
sal_Unicode32 const MS_B_LRHO              = 0x0001D6D2;
sal_Unicode32 const MS_B_OSIGMA            = 0x0001D6D3;
sal_Unicode32 const MS_B_LSIGMA            = 0x0001D6D4;
sal_Unicode32 const MS_B_LTAU              = 0x0001D6D5;
sal_Unicode32 const MS_B_LUPSILON          = 0x0001D6D6;
sal_Unicode32 const MS_B_LPHI              = 0x0001D6D7;
sal_Unicode32 const MS_B_LCHI              = 0x0001D6D8;
sal_Unicode32 const MS_B_LPSI              = 0x0001D6D9;
sal_Unicode32 const MS_B_LOMEGA            = 0x0001D6DA;
sal_Unicode32 const MS_B_PARTIAL           = 0x0001D6DB;
sal_Unicode32 const MS_B_EPSILONLUNATE     = 0x0001D6DC;
sal_Unicode32 const MS_B_LTHETAS           = 0x0001D6DD;
sal_Unicode32 const MS_B_LKAPPAS           = 0x0001D6DE;
sal_Unicode32 const MS_B_LPHIS             = 0x0001D6DF;
sal_Unicode32 const MS_B_LRHOS             = 0x0001D6E0;
sal_Unicode32 const MS_B_LPIS              = 0x0001D6E1;

// Greek capital italic
sal_Unicode32 const MS_I_UALPHA            = 0x0001D6E2;
sal_Unicode32 const MS_I_UBETA             = 0x0001D6E3;
sal_Unicode32 const MS_I_UGAMMA            = 0x0001D6E4;
sal_Unicode32 const MS_I_UDELTA            = 0x0001D6E5;
sal_Unicode32 const MS_I_UEPSILON          = 0x0001D6E6;
sal_Unicode32 const MS_I_UZETA             = 0x0001D6E7;
sal_Unicode32 const MS_I_UETA              = 0x0001D6E8;
sal_Unicode32 const MS_I_UTHETA            = 0x0001D6E9;
sal_Unicode32 const MS_I_UIOTA             = 0x0001D6EA;
sal_Unicode32 const MS_I_UKAPPA            = 0x0001D6EB;
sal_Unicode32 const MS_I_ULAMBDA           = 0x0001D6EC;
sal_Unicode32 const MS_I_UMU               = 0x0001D6ED;
sal_Unicode32 const MS_I_UNU               = 0x0001D6EE;
sal_Unicode32 const MS_I_UXI               = 0x0001D6EF;
sal_Unicode32 const MS_I_UOMICRON          = 0x0001D6F0;
sal_Unicode32 const MS_I_UPI               = 0x0001D6F1;
sal_Unicode32 const MS_I_URHO              = 0x0001D6F2;
sal_Unicode32 const MS_I_UTHETAS           = 0x0001D6F3;
sal_Unicode32 const MS_I_USIGMA            = 0x0001D6F4;
sal_Unicode32 const MS_I_UTAU              = 0x0001D6F5;
sal_Unicode32 const MS_I_UUPSILON          = 0x0001D6F6;
sal_Unicode32 const MS_I_UPHI              = 0x0001D6F7;
sal_Unicode32 const MS_I_UCHI              = 0x0001D6F8;
sal_Unicode32 const MS_I_UPSI              = 0x0001D6F9;
sal_Unicode32 const MS_I_UOMEGA            = 0x0001D6FA;
sal_Unicode32 const MS_I_NABLA             = 0x0001D6FB;

// Greek Lower case bold
sal_Unicode32 const MS_I_LALPHA            = 0x0001D6FC;
sal_Unicode32 const MS_I_LBETA             = 0x0001D6FD;
sal_Unicode32 const MS_I_LGAMMA            = 0x0001D6FE;
sal_Unicode32 const MS_I_LDELTA            = 0x0001D6FF;
sal_Unicode32 const MS_I_LEPSILON          = 0x0001D700;
sal_Unicode32 const MS_I_LZETA             = 0x0001D701;
sal_Unicode32 const MS_I_LETA              = 0x0001D702;
sal_Unicode32 const MS_I_LTHETA            = 0x0001D703;
sal_Unicode32 const MS_I_LIOTA             = 0x0001D704;
sal_Unicode32 const MS_I_LKAPPA            = 0x0001D705;
sal_Unicode32 const MS_I_LLAMBDA           = 0x0001D706;
sal_Unicode32 const MS_I_LMU               = 0x0001D707;
sal_Unicode32 const MS_I_LNU               = 0x0001D708;
sal_Unicode32 const MS_I_LXI               = 0x0001D709;
sal_Unicode32 const MS_I_LOMICRON          = 0x0001D70A;
sal_Unicode32 const MS_I_LPI               = 0x0001D70B;
sal_Unicode32 const MS_I_LRHO              = 0x0001D70C;
sal_Unicode32 const MS_I_OSIGMA            = 0x0001D70D;
sal_Unicode32 const MS_I_LSIGMA            = 0x0001D70E;
sal_Unicode32 const MS_I_LTAU              = 0x0001D70F;
sal_Unicode32 const MS_I_LUPSILON          = 0x0001D710;
sal_Unicode32 const MS_I_LPHI              = 0x0001D711;
sal_Unicode32 const MS_I_LCHI              = 0x0001D712;
sal_Unicode32 const MS_I_LPSI              = 0x0001D713;
sal_Unicode32 const MS_I_LOMEGA            = 0x0001D714;
sal_Unicode32 const MS_I_PARTIAL           = 0x0001D715;
sal_Unicode32 const MS_I_EPSILONLUNATE     = 0x0001D716;
sal_Unicode32 const MS_I_LTHETAS           = 0x0001D717;
sal_Unicode32 const MS_I_LKAPPAS           = 0x0001D718;
sal_Unicode32 const MS_I_LPHIS             = 0x0001D719;
sal_Unicode32 const MS_I_LRHOS             = 0x0001D71A;
sal_Unicode32 const MS_I_LPIS              = 0x0001D71B;

// Greek capital bold italic
sal_Unicode32 const MS_BI_UALPHA           = 0x0001D71C;
sal_Unicode32 const MS_BI_UBETA            = 0x0001D71D;
sal_Unicode32 const MS_BI_UGAMMA           = 0x0001D71E;
sal_Unicode32 const MS_BI_UDELTA           = 0x0001D71F;
sal_Unicode32 const MS_BI_UEPSILON         = 0x0001D720;
sal_Unicode32 const MS_BI_UZETA            = 0x0001D721;
sal_Unicode32 const MS_BI_UETA             = 0x0001D722;
sal_Unicode32 const MS_BI_UTHETA           = 0x0001D723;
sal_Unicode32 const MS_BI_UIOTA            = 0x0001D724;
sal_Unicode32 const MS_BI_UKAPPA           = 0x0001D725;
sal_Unicode32 const MS_BI_ULAMBDA          = 0x0001D726;
sal_Unicode32 const MS_BI_UMU              = 0x0001D727;
sal_Unicode32 const MS_BI_UNU              = 0x0001D728;
sal_Unicode32 const MS_BI_UXI              = 0x0001D729;
sal_Unicode32 const MS_BI_UOMICRON         = 0x0001D72A;
sal_Unicode32 const MS_BI_UPI              = 0x0001D72B;
sal_Unicode32 const MS_BI_URHO             = 0x0001D72C;
sal_Unicode32 const MS_BI_UTHETAS          = 0x0001D72D;
sal_Unicode32 const MS_BI_USIGMA           = 0x0001D72E;
sal_Unicode32 const MS_BI_UTAU             = 0x0001D72F;
sal_Unicode32 const MS_BI_UUPSILON         = 0x0001D730;
sal_Unicode32 const MS_BI_UPHI             = 0x0001D731;
sal_Unicode32 const MS_BI_UCHI             = 0x0001D732;
sal_Unicode32 const MS_BI_UPSI             = 0x0001D733;
sal_Unicode32 const MS_BI_UOMEGA           = 0x0001D734;
sal_Unicode32 const MS_BI_NABLA            = 0x0001D735;

// Greek Lower case bold
sal_Unicode32 const MS_BI_LALPHA           = 0x0001D736;
sal_Unicode32 const MS_BI_LBETA            = 0x0001D737;
sal_Unicode32 const MS_BI_LGAMMA           = 0x0001D738;
sal_Unicode32 const MS_BI_LDELTA           = 0x0001D739;
sal_Unicode32 const MS_BI_LEPSILON         = 0x0001D73A;
sal_Unicode32 const MS_BI_LZETA            = 0x0001D73B;
sal_Unicode32 const MS_BI_LETA             = 0x0001D73C;
sal_Unicode32 const MS_BI_LTHETA           = 0x0001D73D;
sal_Unicode32 const MS_BI_LIOTA            = 0x0001D73E;
sal_Unicode32 const MS_BI_LKAPPA           = 0x0001D73F;
sal_Unicode32 const MS_BI_LLAMBDA          = 0x0001D740;
sal_Unicode32 const MS_BI_LMU              = 0x0001D741;
sal_Unicode32 const MS_BI_LNU              = 0x0001D742;
sal_Unicode32 const MS_BI_LXI              = 0x0001D743;
sal_Unicode32 const MS_BI_LOMICRON         = 0x0001D744;
sal_Unicode32 const MS_BI_LPI              = 0x0001D745;
sal_Unicode32 const MS_BI_LRHO             = 0x0001D746;
sal_Unicode32 const MS_BI_OSIGMA           = 0x0001D747;
sal_Unicode32 const MS_BI_LSIGMA           = 0x0001D748;
sal_Unicode32 const MS_BI_LTAU             = 0x0001D749;
sal_Unicode32 const MS_BI_LUPSILON         = 0x0001D74A;
sal_Unicode32 const MS_BI_LPHI             = 0x0001D74B;
sal_Unicode32 const MS_BI_LCHI             = 0x0001D74C;
sal_Unicode32 const MS_BI_LPSI             = 0x0001D74D;
sal_Unicode32 const MS_BI_LOMEGA           = 0x0001D74E;
sal_Unicode32 const MS_BI_PARTIAL          = 0x0001D74F;
sal_Unicode32 const MS_BI_EPSILONLUNATE    = 0x0001D750;
sal_Unicode32 const MS_BI_LTHETAS          = 0x0001D751;
sal_Unicode32 const MS_BI_LKAPPAS          = 0x0001D752;
sal_Unicode32 const MS_BI_LPHIS            = 0x0001D753;
sal_Unicode32 const MS_BI_LRHOS            = 0x0001D754;
sal_Unicode32 const MS_BI_LPIS             = 0x0001D755;

/**
  * Latin
  */

// Latin capital letters
sal_Unicode32 const MS_L_UA                = 0x00000041;
sal_Unicode32 const MS_L_UB                = 0x00000042;
sal_Unicode32 const MS_L_UC                = 0x00000043;
sal_Unicode32 const MS_L_UD                = 0x00000044;
sal_Unicode32 const MS_L_UE                = 0x00000045;
sal_Unicode32 const MS_L_UF                = 0x00000046;
sal_Unicode32 const MS_L_UG                = 0x00000047;
sal_Unicode32 const MS_L_UH                = 0x00000048;
sal_Unicode32 const MS_L_UI                = 0x00000049;
sal_Unicode32 const MS_L_UJ                = 0x0000004A;
sal_Unicode32 const MS_L_UK                = 0x0000004B;
sal_Unicode32 const MS_L_UL                = 0x0000004C;
sal_Unicode32 const MS_L_UM                = 0x0000004D;
sal_Unicode32 const MS_L_UN                = 0x0000004E;
sal_Unicode32 const MS_L_UO                = 0x0000004F;
sal_Unicode32 const MS_L_UP                = 0x00000050;
sal_Unicode32 const MS_L_UQ                = 0x00000051;
sal_Unicode32 const MS_L_UR                = 0x00000052;
sal_Unicode32 const MS_L_US                = 0x00000053;
sal_Unicode32 const MS_L_UT                = 0x00000054;
sal_Unicode32 const MS_L_UU                = 0x00000055;
sal_Unicode32 const MS_L_UV                = 0x00000056;
sal_Unicode32 const MS_L_UW                = 0x00000057;
sal_Unicode32 const MS_L_UX                = 0x00000058;
sal_Unicode32 const MS_L_UY                = 0x00000059;
sal_Unicode32 const MS_L_UZ                = 0x0000005A;

// Latin lower case letters
sal_Unicode32 const MS_L_LA                = 0x00000061;
sal_Unicode32 const MS_L_LB                = 0x00000062;
sal_Unicode32 const MS_L_LC                = 0x00000063;
sal_Unicode32 const MS_L_LD                = 0x00000064;
sal_Unicode32 const MS_L_LE                = 0x00000065;
sal_Unicode32 const MS_L_LF                = 0x00000066;
sal_Unicode32 const MS_L_LG                = 0x00000067;
sal_Unicode32 const MS_L_LH                = 0x00000068;
sal_Unicode32 const MS_L_LI                = 0x00000069;
sal_Unicode32 const MS_L_LJ                = 0x0000006A;
sal_Unicode32 const MS_L_LK                = 0x0000006B;
sal_Unicode32 const MS_L_LL                = 0x0000006C;
sal_Unicode32 const MS_L_LM                = 0x0000006D;
sal_Unicode32 const MS_L_LN                = 0x0000006E;
sal_Unicode32 const MS_L_LO                = 0x0000006F;
sal_Unicode32 const MS_L_LP                = 0x00000070;
sal_Unicode32 const MS_L_LQ                = 0x00000071;
sal_Unicode32 const MS_L_LR                = 0x00000072;
sal_Unicode32 const MS_L_LS                = 0x00000073;
sal_Unicode32 const MS_L_LT                = 0x00000074;
sal_Unicode32 const MS_L_LU                = 0x00000075;
sal_Unicode32 const MS_L_LV                = 0x00000076;
sal_Unicode32 const MS_L_LW                = 0x00000077;
sal_Unicode32 const MS_L_LX                = 0x00000078;
sal_Unicode32 const MS_L_LY                = 0x00000079;
sal_Unicode32 const MS_L_LZ                = 0x0000007A;

// Latin capital letters bold
sal_Unicode32 const MS_B_UA                = 0x0001D400;
sal_Unicode32 const MS_B_UB                = 0x0001D401;
sal_Unicode32 const MS_B_UC                = 0x0001D402;
sal_Unicode32 const MS_B_UD                = 0x0001D403;
sal_Unicode32 const MS_B_UE                = 0x0001D404;
sal_Unicode32 const MS_B_UF                = 0x0001D405;
sal_Unicode32 const MS_B_UG                = 0x0001D406;
sal_Unicode32 const MS_B_UH                = 0x0001D407;
sal_Unicode32 const MS_B_UI                = 0x0001D408;
sal_Unicode32 const MS_B_UJ                = 0x0001D409;
sal_Unicode32 const MS_B_UK                = 0x0001D40A;
sal_Unicode32 const MS_B_UL                = 0x0001D40B;
sal_Unicode32 const MS_B_UM                = 0x0001D40C;
sal_Unicode32 const MS_B_UN                = 0x0001D40D;
sal_Unicode32 const MS_B_UO                = 0x0001D40E;
sal_Unicode32 const MS_B_UP                = 0x0001D40F;
sal_Unicode32 const MS_B_UQ                = 0x0001D410;
sal_Unicode32 const MS_B_UR                = 0x0001D411;
sal_Unicode32 const MS_B_US                = 0x0001D412;
sal_Unicode32 const MS_B_UT                = 0x0001D413;
sal_Unicode32 const MS_B_UU                = 0x0001D414;
sal_Unicode32 const MS_B_UV                = 0x0001D415;
sal_Unicode32 const MS_B_UW                = 0x0001D416;
sal_Unicode32 const MS_B_UX                = 0x0001D417;
sal_Unicode32 const MS_B_UY                = 0x0001D418;
sal_Unicode32 const MS_B_UZ                = 0x0001D419;

// Latin lower case letters bold
sal_Unicode32 const MS_B_LA                = 0x0001D41A;
sal_Unicode32 const MS_B_LB                = 0x0001D41B;
sal_Unicode32 const MS_B_LC                = 0x0001D41C;
sal_Unicode32 const MS_B_LD                = 0x0001D41D;
sal_Unicode32 const MS_B_LE                = 0x0001D41E;
sal_Unicode32 const MS_B_LF                = 0x0001D41F;
sal_Unicode32 const MS_B_LG                = 0x0001D420;
sal_Unicode32 const MS_B_LH                = 0x0001D421;
sal_Unicode32 const MS_B_LI                = 0x0001D422;
sal_Unicode32 const MS_B_LJ                = 0x0001D423;
sal_Unicode32 const MS_B_LK                = 0x0001D424;
sal_Unicode32 const MS_B_LL                = 0x0001D425;
sal_Unicode32 const MS_B_LM                = 0x0001D426;
sal_Unicode32 const MS_B_LN                = 0x0001D427;
sal_Unicode32 const MS_B_LO                = 0x0001D428;
sal_Unicode32 const MS_B_LP                = 0x0001D429;
sal_Unicode32 const MS_B_LQ                = 0x0001D42A;
sal_Unicode32 const MS_B_LR                = 0x0001D42B;
sal_Unicode32 const MS_B_LS                = 0x0001D42C;
sal_Unicode32 const MS_B_LT                = 0x0001D42D;
sal_Unicode32 const MS_B_LU                = 0x0001D42E;
sal_Unicode32 const MS_B_LV                = 0x0001D42F;
sal_Unicode32 const MS_B_LW                = 0x0001D430;
sal_Unicode32 const MS_B_LX                = 0x0001D431;
sal_Unicode32 const MS_B_LY                = 0x0001D432;
sal_Unicode32 const MS_B_LZ                = 0x0001D433;

// Latin capital letters italic
sal_Unicode32 const MS_I_UA                = 0x0001D434;
sal_Unicode32 const MS_I_UB                = 0x0001D435;
sal_Unicode32 const MS_I_UC                = 0x0001D436;
sal_Unicode32 const MS_I_UD                = 0x0001D437;
sal_Unicode32 const MS_I_UE                = 0x0001D438;
sal_Unicode32 const MS_I_UF                = 0x0001D439;
sal_Unicode32 const MS_I_UG                = 0x0001D43A;
sal_Unicode32 const MS_I_UH                = 0x0001D43B;
sal_Unicode32 const MS_I_UI                = 0x0001D43C;
sal_Unicode32 const MS_I_UJ                = 0x0001D43D;
sal_Unicode32 const MS_I_UK                = 0x0001D43E;
sal_Unicode32 const MS_I_UL                = 0x0001D43F;
sal_Unicode32 const MS_I_UM                = 0x0001D440;
sal_Unicode32 const MS_I_UN                = 0x0001D441;
sal_Unicode32 const MS_I_UO                = 0x0001D442;
sal_Unicode32 const MS_I_UP                = 0x0001D443;
sal_Unicode32 const MS_I_UQ                = 0x0001D444;
sal_Unicode32 const MS_I_UR                = 0x0001D445;
sal_Unicode32 const MS_I_US                = 0x0001D446;
sal_Unicode32 const MS_I_UT                = 0x0001D447;
sal_Unicode32 const MS_I_UU                = 0x0001D448;
sal_Unicode32 const MS_I_UV                = 0x0001D449;
sal_Unicode32 const MS_I_UW                = 0x0001D44A;
sal_Unicode32 const MS_I_UX                = 0x0001D44B;
sal_Unicode32 const MS_I_UY                = 0x0001D44C;
sal_Unicode32 const MS_I_UZ                = 0x0001D44D;

// Latin lower case letters italic
sal_Unicode32 const MS_I_LA                = 0x0001D44E;
sal_Unicode32 const MS_I_LB                = 0x0001D44F;
sal_Unicode32 const MS_I_LC                = 0x0001D450;
sal_Unicode32 const MS_I_LD                = 0x0001D451;
sal_Unicode32 const MS_I_LE                = 0x0001D452;
sal_Unicode32 const MS_I_LF                = 0x0001D453;
sal_Unicode32 const MS_I_LG                = 0x0001D454;
sal_Unicode32 const MS_I_LH                = 0x0001D455;
sal_Unicode32 const MS_I_LI                = 0x0001D456;
sal_Unicode32 const MS_I_LJ                = 0x0001D457;
sal_Unicode32 const MS_I_LK                = 0x0001D458;
sal_Unicode32 const MS_I_LL                = 0x0001D459;
sal_Unicode32 const MS_I_LM                = 0x0001D45A;
sal_Unicode32 const MS_I_LN                = 0x0001D45B;
sal_Unicode32 const MS_I_LO                = 0x0001D45C;
sal_Unicode32 const MS_I_LP                = 0x0001D45D;
sal_Unicode32 const MS_I_LQ                = 0x0001D45E;
sal_Unicode32 const MS_I_LR                = 0x0001D45F;
sal_Unicode32 const MS_I_LS                = 0x0001D460;
sal_Unicode32 const MS_I_LT                = 0x0001D461;
sal_Unicode32 const MS_I_LU                = 0x0001D462;
sal_Unicode32 const MS_I_LV                = 0x0001D463;
sal_Unicode32 const MS_I_LW                = 0x0001D464;
sal_Unicode32 const MS_I_LX                = 0x0001D465;
sal_Unicode32 const MS_I_LY                = 0x0001D466;
sal_Unicode32 const MS_I_LZ                = 0x0001D467;

// Latin capital letters bold italic
sal_Unicode32 const MS_BI_UA               = 0x0001D468;
sal_Unicode32 const MS_BI_UB               = 0x0001D469;
sal_Unicode32 const MS_BI_UC               = 0x0001D46A;
sal_Unicode32 const MS_BI_UD               = 0x0001D46B;
sal_Unicode32 const MS_BI_UE               = 0x0001D46C;
sal_Unicode32 const MS_BI_UF               = 0x0001D46D;
sal_Unicode32 const MS_BI_UG               = 0x0001D46E;
sal_Unicode32 const MS_BI_UH               = 0x0000210E;
sal_Unicode32 const MS_BI_UI               = 0x0001D470;
sal_Unicode32 const MS_BI_UJ               = 0x0001D471;
sal_Unicode32 const MS_BI_UK               = 0x0001D472;
sal_Unicode32 const MS_BI_UL               = 0x0001D473;
sal_Unicode32 const MS_BI_UM               = 0x0001D474;
sal_Unicode32 const MS_BI_UN               = 0x0001D475;
sal_Unicode32 const MS_BI_UO               = 0x0001D476;
sal_Unicode32 const MS_BI_UP               = 0x0001D477;
sal_Unicode32 const MS_BI_UQ               = 0x0001D478;
sal_Unicode32 const MS_BI_UR               = 0x0001D479;
sal_Unicode32 const MS_BI_US               = 0x0001D47A;
sal_Unicode32 const MS_BI_UT               = 0x0001D47B;
sal_Unicode32 const MS_BI_UU               = 0x0001D47C;
sal_Unicode32 const MS_BI_UV               = 0x0001D47D;
sal_Unicode32 const MS_BI_UW               = 0x0001D47E;
sal_Unicode32 const MS_BI_UX               = 0x0001D47F;
sal_Unicode32 const MS_BI_UY               = 0x0001D480;
sal_Unicode32 const MS_BI_UZ               = 0x0001D481;

// Latin lower case letters bold italic
sal_Unicode32 const MS_BI_LA               = 0x0001D482;
sal_Unicode32 const MS_BI_LB               = 0x0001D483;
sal_Unicode32 const MS_BI_LC               = 0x0001D484;
sal_Unicode32 const MS_BI_LD               = 0x0001D485;
sal_Unicode32 const MS_BI_LE               = 0x0001D486;
sal_Unicode32 const MS_BI_LF               = 0x0001D487;
sal_Unicode32 const MS_BI_LG               = 0x0001D488;
sal_Unicode32 const MS_BI_LH               = 0x0001D489;
sal_Unicode32 const MS_BI_LI               = 0x0001D48A;
sal_Unicode32 const MS_BI_LJ               = 0x0001D48B;
sal_Unicode32 const MS_BI_LK               = 0x0001D48C;
sal_Unicode32 const MS_BI_LL               = 0x0001D48D;
sal_Unicode32 const MS_BI_LM               = 0x0001D48E;
sal_Unicode32 const MS_BI_LN               = 0x0001D48F;
sal_Unicode32 const MS_BI_LO               = 0x0001D490;
sal_Unicode32 const MS_BI_LP               = 0x0001D491;
sal_Unicode32 const MS_BI_LQ               = 0x0001D492;
sal_Unicode32 const MS_BI_LR               = 0x0001D493;
sal_Unicode32 const MS_BI_LS               = 0x0001D494;
sal_Unicode32 const MS_BI_LT               = 0x0001D495;
sal_Unicode32 const MS_BI_LU               = 0x0001D496;
sal_Unicode32 const MS_BI_LV               = 0x0001D497;
sal_Unicode32 const MS_BI_LW               = 0x0001D498;
sal_Unicode32 const MS_BI_LX               = 0x0001D499;
sal_Unicode32 const MS_BI_LY               = 0x0001D49A;
sal_Unicode32 const MS_BI_LZ               = 0x0001D49B;

/**
  * Script
  */

// Latin capital script
sal_Unicode32 const MS_SC_UA               = 0x0001D49C;
sal_Unicode32 const MS_SC_UB               = 0x0000212C;
sal_Unicode32 const MS_SC_UC               = 0x0001D49E;
sal_Unicode32 const MS_SC_UD               = 0x0001D49F;
sal_Unicode32 const MS_SC_UE               = 0x00002130;
sal_Unicode32 const MS_SC_UF               = 0x00002131;
sal_Unicode32 const MS_SC_UG               = 0x0001D4A2;
sal_Unicode32 const MS_SC_UH               = 0x0000210B;
sal_Unicode32 const MS_SC_UI               = 0x00002110;
sal_Unicode32 const MS_SC_UJ               = 0x0001D4A5;
sal_Unicode32 const MS_SC_UK               = 0x0001D4A6;
sal_Unicode32 const MS_SC_UL               = 0x00002112;
sal_Unicode32 const MS_SC_UM               = 0x00002133;
sal_Unicode32 const MS_SC_UN               = 0x0001D4A9;
sal_Unicode32 const MS_SC_UO               = 0x0001D4AA;
sal_Unicode32 const MS_SC_UP               = 0x0001D4AB;
sal_Unicode32 const MS_SC_UQ               = 0x0001D4AC;
sal_Unicode32 const MS_SC_UR               = 0x0000211B;
sal_Unicode32 const MS_SC_US               = 0x0001D4AE;
sal_Unicode32 const MS_SC_UT               = 0x0001D4AF;
sal_Unicode32 const MS_SC_UU               = 0x0001D4B0;
sal_Unicode32 const MS_SC_UV               = 0x0001D4B1;
sal_Unicode32 const MS_SC_UW               = 0x0001D4B2;
sal_Unicode32 const MS_SC_UX               = 0x0001D4B3;
sal_Unicode32 const MS_SC_UY               = 0x0001D4B4;
sal_Unicode32 const MS_SC_UZ               = 0x0001D4B5;

// Latin lower case letters script
sal_Unicode32 const MS_SC_LA               = 0x0001D4B6;
sal_Unicode32 const MS_SC_LB               = 0x0001D4B7;
sal_Unicode32 const MS_SC_LC               = 0x0001D4B8;
sal_Unicode32 const MS_SC_LD               = 0x0001D4B9;
sal_Unicode32 const MS_SC_LE               = 0x0000212F;
sal_Unicode32 const MS_SC_LF               = 0x0001D4BB;
sal_Unicode32 const MS_SC_LG               = 0x0000210A;
sal_Unicode32 const MS_SC_LH               = 0x0001D4BD;
sal_Unicode32 const MS_SC_LI               = 0x0001D4BE;
sal_Unicode32 const MS_SC_LJ               = 0x0001D4BF;
sal_Unicode32 const MS_SC_LK               = 0x0001D4C0;
sal_Unicode32 const MS_SC_LL               = 0x0001D4C1;
sal_Unicode32 const MS_SC_LM               = 0x0001D4C2;
sal_Unicode32 const MS_SC_LN               = 0x0001D4C3;
sal_Unicode32 const MS_SC_LO               = 0x00002134;
sal_Unicode32 const MS_SC_LP               = 0x0001D4C5;
sal_Unicode32 const MS_SC_LQ               = 0x0001D4C6;
sal_Unicode32 const MS_SC_LR               = 0x0001D4C7;
sal_Unicode32 const MS_SC_LS               = 0x0001D4C8;
sal_Unicode32 const MS_SC_LT               = 0x0001D4C9;
sal_Unicode32 const MS_SC_LU               = 0x0001D4CA;
sal_Unicode32 const MS_SC_LV               = 0x0001D4CB;
sal_Unicode32 const MS_SC_LW               = 0x0001D4CC;
sal_Unicode32 const MS_SC_LX               = 0x0001D4CD;
sal_Unicode32 const MS_SC_LY               = 0x0001D4CE;
sal_Unicode32 const MS_SC_LZ               = 0x0001D4CF;

// Latin capital script bold
sal_Unicode32 const MS_BSC_UA              = 0x0001D4D0;
sal_Unicode32 const MS_BSC_UB              = 0x0001D4D1;
sal_Unicode32 const MS_BSC_UC              = 0x0001D4D2;
sal_Unicode32 const MS_BSC_UD              = 0x0001D4D3;
sal_Unicode32 const MS_BSC_UE              = 0x0001D4D4;
sal_Unicode32 const MS_BSC_UF              = 0x0001D4D5;
sal_Unicode32 const MS_BSC_UG              = 0x0001D4D6;
sal_Unicode32 const MS_BSC_UH              = 0x0001D4D7;
sal_Unicode32 const MS_BSC_UI              = 0x0001D4D8;
sal_Unicode32 const MS_BSC_UJ              = 0x0001D4D9;
sal_Unicode32 const MS_BSC_UK              = 0x0001D4DA;
sal_Unicode32 const MS_BSC_UL              = 0x0001D4DB;
sal_Unicode32 const MS_BSC_UM              = 0x0001D4DC;
sal_Unicode32 const MS_BSC_UN              = 0x0001D4DD;
sal_Unicode32 const MS_BSC_UO              = 0x0001D4DE;
sal_Unicode32 const MS_BSC_UP              = 0x0001D4DF;
sal_Unicode32 const MS_BSC_UQ              = 0x0001D4E0;
sal_Unicode32 const MS_BSC_UR              = 0x0001D4E1;
sal_Unicode32 const MS_BSC_US              = 0x0001D4E2;
sal_Unicode32 const MS_BSC_UT              = 0x0001D4E3;
sal_Unicode32 const MS_BSC_UU              = 0x0001D4E4;
sal_Unicode32 const MS_BSC_UV              = 0x0001D4E5;
sal_Unicode32 const MS_BSC_UW              = 0x0001D4E6;
sal_Unicode32 const MS_BSC_UX              = 0x0001D4E7;
sal_Unicode32 const MS_BSC_UY              = 0x0001D4E8;
sal_Unicode32 const MS_BSC_UZ              = 0x0001D4E9;

// Latin lower case script bold
sal_Unicode32 const MS_BSC_LA              = 0x0001D4EA;
sal_Unicode32 const MS_BSC_LB              = 0x0001D4EB;
sal_Unicode32 const MS_BSC_LC              = 0x0001D4EC;
sal_Unicode32 const MS_BSC_LD              = 0x0001D4ED;
sal_Unicode32 const MS_BSC_LE              = 0x0001D4EE;
sal_Unicode32 const MS_BSC_LF              = 0x0001D4EF;
sal_Unicode32 const MS_BSC_LG              = 0x0001D4F0;
sal_Unicode32 const MS_BSC_LH              = 0x0001D4F1;
sal_Unicode32 const MS_BSC_LI              = 0x0001D4F2;
sal_Unicode32 const MS_BSC_LJ              = 0x0001D4F3;
sal_Unicode32 const MS_BSC_LK              = 0x0001D4F4;
sal_Unicode32 const MS_BSC_LL              = 0x0001D4F5;
sal_Unicode32 const MS_BSC_LM              = 0x0001D4F6;
sal_Unicode32 const MS_BSC_LN              = 0x0001D4F7;
sal_Unicode32 const MS_BSC_LO              = 0x0001D4F8;
sal_Unicode32 const MS_BSC_LP              = 0x0001D4F9;
sal_Unicode32 const MS_BSC_LQ              = 0x0001D4FA;
sal_Unicode32 const MS_BSC_LR              = 0x0001D4FB;
sal_Unicode32 const MS_BSC_LS              = 0x0001D4FC;
sal_Unicode32 const MS_BSC_LT              = 0x0001D4FD;
sal_Unicode32 const MS_BSC_LU              = 0x0001D4FE;
sal_Unicode32 const MS_BSC_LV              = 0x0001D4FF;
sal_Unicode32 const MS_BSC_LW              = 0x0001D500;
sal_Unicode32 const MS_BSC_LX              = 0x0001D501;
sal_Unicode32 const MS_BSC_LY              = 0x0001D502;
sal_Unicode32 const MS_BSC_LZ              = 0x0001D503;

/**
  * Fraktur
  */

// Latin capital fraktur
sal_Unicode32 const MS_FR_UA               = 0x0001D504;
sal_Unicode32 const MS_FR_UB               = 0x0001D505;
sal_Unicode32 const MS_FR_UC               = 0x0000212D;
sal_Unicode32 const MS_FR_UD               = 0x0001D507;
sal_Unicode32 const MS_FR_UE               = 0x0001D508;
sal_Unicode32 const MS_FR_UF               = 0x0001D509;
sal_Unicode32 const MS_FR_UG               = 0x0001D50A;
sal_Unicode32 const MS_FR_UH               = 0x0000210C;
sal_Unicode32 const MS_FR_UI               = 0x00002111;
sal_Unicode32 const MS_FR_UJ               = 0x0001D50D;
sal_Unicode32 const MS_FR_UK               = 0x0001D50E;
sal_Unicode32 const MS_FR_UL               = 0x0001D50F;
sal_Unicode32 const MS_FR_UM               = 0x0001D510;
sal_Unicode32 const MS_FR_UN               = 0x0001D511;
sal_Unicode32 const MS_FR_UO               = 0x0001D512;
sal_Unicode32 const MS_FR_UP               = 0x0001D513;
sal_Unicode32 const MS_FR_UQ               = 0x0001D514;
sal_Unicode32 const MS_FR_UR               = 0x0000211C;
sal_Unicode32 const MS_FR_US               = 0x0001D516;
sal_Unicode32 const MS_FR_UT               = 0x0001D517;
sal_Unicode32 const MS_FR_UU               = 0x0001D518;
sal_Unicode32 const MS_FR_UV               = 0x0001D519;
sal_Unicode32 const MS_FR_UW               = 0x0001D51A;
sal_Unicode32 const MS_FR_UX               = 0x0001D51B;
sal_Unicode32 const MS_FR_UY               = 0x0001D51C;
sal_Unicode32 const MS_FR_UZ               = 0x00002128;

// Latin lower case fraktur
sal_Unicode32 const MS_FR_LA               = 0x0001D51E;
sal_Unicode32 const MS_FR_LB               = 0x0001D51F;
sal_Unicode32 const MS_FR_LC               = 0x0001D520;
sal_Unicode32 const MS_FR_LD               = 0x0001D521;
sal_Unicode32 const MS_FR_LE               = 0x0001D522;
sal_Unicode32 const MS_FR_LF               = 0x0001D523;
sal_Unicode32 const MS_FR_LG               = 0x0001D524;
sal_Unicode32 const MS_FR_LH               = 0x0001D525;
sal_Unicode32 const MS_FR_LI               = 0x0001D526;
sal_Unicode32 const MS_FR_LJ               = 0x0001D527;
sal_Unicode32 const MS_FR_LK               = 0x0001D528;
sal_Unicode32 const MS_FR_LL               = 0x0001D529;
sal_Unicode32 const MS_FR_LM               = 0x0001D52A;
sal_Unicode32 const MS_FR_LN               = 0x0001D52B;
sal_Unicode32 const MS_FR_LO               = 0x0001D52C;
sal_Unicode32 const MS_FR_LP               = 0x0001D52D;
sal_Unicode32 const MS_FR_LQ               = 0x0001D52E;
sal_Unicode32 const MS_FR_LR               = 0x0001D52F;
sal_Unicode32 const MS_FR_LS               = 0x0001D530;
sal_Unicode32 const MS_FR_LT               = 0x0001D531;
sal_Unicode32 const MS_FR_LU               = 0x0001D532;
sal_Unicode32 const MS_FR_LV               = 0x0001D533;
sal_Unicode32 const MS_FR_LW               = 0x0001D534;
sal_Unicode32 const MS_FR_LX               = 0x0001D535;
sal_Unicode32 const MS_FR_LY               = 0x0001D536;
sal_Unicode32 const MS_FR_LZ               = 0x0001D537;

// Latin capital fraktur bold
sal_Unicode32 const MS_BFR_UA              = 0x0001D56C;
sal_Unicode32 const MS_BFR_UB              = 0x0001D56D;
sal_Unicode32 const MS_BFR_UC              = 0x0001D56E;
sal_Unicode32 const MS_BFR_UD              = 0x0001D56F;
sal_Unicode32 const MS_BFR_UE              = 0x0001D570;
sal_Unicode32 const MS_BFR_UF              = 0x0001D571;
sal_Unicode32 const MS_BFR_UG              = 0x0001D572;
sal_Unicode32 const MS_BFR_UH              = 0x0001D573;
sal_Unicode32 const MS_BFR_UI              = 0x0001D574;
sal_Unicode32 const MS_BFR_UJ              = 0x0001D575;
sal_Unicode32 const MS_BFR_UK              = 0x0001D576;
sal_Unicode32 const MS_BFR_UL              = 0x0001D577;
sal_Unicode32 const MS_BFR_UM              = 0x0001D578;
sal_Unicode32 const MS_BFR_UN              = 0x0001D579;
sal_Unicode32 const MS_BFR_UO              = 0x0001D57A;
sal_Unicode32 const MS_BFR_UP              = 0x0001D57B;
sal_Unicode32 const MS_BFR_UQ              = 0x0001D57C;
sal_Unicode32 const MS_BFR_UR              = 0x0001D57D;
sal_Unicode32 const MS_BFR_US              = 0x0001D57E;
sal_Unicode32 const MS_BFR_UT              = 0x0001D57F;
sal_Unicode32 const MS_BFR_UU              = 0x0001D580;
sal_Unicode32 const MS_BFR_UV              = 0x0001D581;
sal_Unicode32 const MS_BFR_UW              = 0x0001D582;
sal_Unicode32 const MS_BFR_UX              = 0x0001D583;
sal_Unicode32 const MS_BFR_UY              = 0x0001D584;
sal_Unicode32 const MS_BFR_UZ              = 0x0001D585;

// Latin lower case fraktur bold
sal_Unicode32 const MS_BFR_LA              = 0x0001D586;
sal_Unicode32 const MS_BFR_LB              = 0x0001D587;
sal_Unicode32 const MS_BFR_LC              = 0x0001D588;
sal_Unicode32 const MS_BFR_LD              = 0x0001D589;
sal_Unicode32 const MS_BFR_LE              = 0x0001D58A;
sal_Unicode32 const MS_BFR_LF              = 0x0001D58B;
sal_Unicode32 const MS_BFR_LG              = 0x0001D58C;
sal_Unicode32 const MS_BFR_LH              = 0x0001D58D;
sal_Unicode32 const MS_BFR_LI              = 0x0001D58E;
sal_Unicode32 const MS_BFR_LJ              = 0x0001D58F;
sal_Unicode32 const MS_BFR_LK              = 0x0001D590;
sal_Unicode32 const MS_BFR_LL              = 0x0001D591;
sal_Unicode32 const MS_BFR_LM              = 0x0001D592;
sal_Unicode32 const MS_BFR_LN              = 0x0001D593;
sal_Unicode32 const MS_BFR_LO              = 0x0001D594;
sal_Unicode32 const MS_BFR_LP              = 0x0001D595;
sal_Unicode32 const MS_BFR_LQ              = 0x0001D596;
sal_Unicode32 const MS_BFR_LR              = 0x0001D597;
sal_Unicode32 const MS_BFR_LS              = 0x0001D598;
sal_Unicode32 const MS_BFR_LT              = 0x0001D599;
sal_Unicode32 const MS_BFR_LU              = 0x0001D59A;
sal_Unicode32 const MS_BFR_LV              = 0x0001D59B;
sal_Unicode32 const MS_BFR_LW              = 0x0001D59C;
sal_Unicode32 const MS_BFR_LX              = 0x0001D59D;
sal_Unicode32 const MS_BFR_LY              = 0x0001D59E;
sal_Unicode32 const MS_BFR_LZ              = 0x0001D59F;

/**
  * Units (SI and others)
  */

// Units
//Metric prefix
sal_Unicode   const MS_YOCTO               = 0x0079;
sal_Unicode   const MS_ZEPTO               = 0x007A;
sal_Unicode   const MS_ATTO                = 0x0061;
sal_Unicode   const MS_FEMTO               = 0x0066;
sal_Unicode   const MS_PICO                = 0x0070;
sal_Unicode   const MS_NANO                = 0x006E;
sal_Unicode   const MS_MICRO               = 0x00B5;
sal_Unicode   const MS_MILI                = 0x006D;
sal_Unicode   const MS_CENTI               = 0x0063;
sal_Unicode   const MS_DECI                = 0x0064;
sal_Unicode   const MS_DECA                = MS_NONE;
sal_Unicode   const MS_HECTO               = 0x0068;
sal_Unicode   const MS_KILO                = 0x006B;
sal_Unicode   const MS_MEGA                = 0x004D;
sal_Unicode   const MS_GIGA                = 0x0047;
sal_Unicode   const MS_TERRA               = 0x0054;
sal_Unicode   const MS_PETA                = 0x0050;
sal_Unicode   const MS_EXA                 = 0x0045;
sal_Unicode   const MS_ZETTA               = 0x005A;
sal_Unicode   const MS_YOTTA               = 0x0059;
//Units
sal_Unicode   const MS_LITRE               = 0x2113;
sal_Unicode   const MS_METER               = 0x006D;
sal_Unicode   const MS_AMSTRONG            = 0x212B;
sal_Unicode   const MS_SECOND              = 0x0073;
sal_Unicode   const MS_HERTZ               = MS_NONE;
sal_Unicode   const MS_NEWTON              = 0x004E;
sal_Unicode   const MS_JOULE               = 0x004A;
sal_Unicode   const MS_OUNCE               = 0x2125;
sal_Unicode   const MS_SIEMENS             = 0x2126;
sal_Unicode   const MS_DRAM                = 0x2128;
sal_Unicode   const MS_PASCAL              = MS_NONE;
sal_Unicode   const MS_GRAM                = 0x0067;

/**
  * Hebrew
  */

// Hebrew
// Punctuation
sal_Unicode   const MS_MAQAF               = 0x05BE;
sal_Unicode   const MS_PASEQ               = 0x05C0;
sal_Unicode   const MS_SOLFPASUQ           = 0x05C3;
sal_Unicode   const MS_NUNHAFUKHA          = 0x05C6;
// Letters
sal_Unicode   const MS_ALEPH               = 0x05D0;
sal_Unicode   const MS_BET                 = 0x05D1;
sal_Unicode   const MS_GIMEL               = 0x05D2;
sal_Unicode   const MS_DALET               = 0x05D3;
sal_Unicode   const MS_HE                  = 0x05D4;
sal_Unicode   const MS_VAV                 = 0x05D5;
sal_Unicode   const MS_ZAYIN               = 0x05D6;
sal_Unicode   const MS_HET                 = 0x05D7;
sal_Unicode   const MS_TET                 = 0x05D8;
sal_Unicode   const MS_YOD                 = 0x05D9;
sal_Unicode   const MS_FINALKAF            = 0x05DA;
sal_Unicode   const MS_KAF                 = 0x05DB;
sal_Unicode   const MS_LAMED               = 0x05DC;
sal_Unicode   const MS_FINALMEM            = 0x05DD;
sal_Unicode   const MS_MEM                 = 0x05DE;
sal_Unicode   const MS_FINALNUN            = 0x05DF;
sal_Unicode   const MS_NUN                 = 0x05E0;
sal_Unicode   const MS_SAMEKH              = 0x05E1;
sal_Unicode   const MS_AYIN                = 0x05E2;
sal_Unicode   const MS_FINELPE             = 0x05E3;
sal_Unicode   const MS_PE                  = 0x05E4;
sal_Unicode   const MS_FINALTSADI          = 0x05E5;
sal_Unicode   const MS_TSADI               = 0x05E6;
sal_Unicode   const MS_QOF                 = 0x05E7;
sal_Unicode   const MS_RESH                = 0x05E8;
sal_Unicode   const MS_SHIN                = 0x05E9;
sal_Unicode   const MS_TAV                 = 0x05EA;
// others
sal_Unicode   const MS_YDDOUBLEVAV         = 0x05F0;
sal_Unicode   const MS_YVAVYOD             = 0x05F1;
sal_Unicode   const MS_YDYOD               = 0x05F2;
sal_Unicode   const MS_GERESH              = 0x05F3;
sal_Unicode   const MS_GERSHAYIM           = 0x05F4;

/**
  * Phoenician
  */

// Phoenician
sal_Unicode32 const MS_ALF                 = 0x00010900;
sal_Unicode32 const MS_PBET                = 0x00010901;
sal_Unicode32 const MS_GAML                = 0x00010902;
sal_Unicode32 const MS_DELT                = 0x00010903;
sal_Unicode32 const MS_PHE                 = 0x00010904;
sal_Unicode32 const MS_WAU                 = 0x00010905;
sal_Unicode32 const MS_ZAI                 = 0x00010906;
sal_Unicode32 const MS_PHET                = 0x00010907;
sal_Unicode32 const MS_PTET                = 0x00010908;
sal_Unicode32 const MS_PYOD                = 0x00010909;
sal_Unicode32 const MS_PKAF                = 0x0001090A;
sal_Unicode32 const MS_LAMD                = 0x0001090B;
sal_Unicode32 const MS_PMEM                = 0x0001090C;
sal_Unicode32 const MS_PNUN                = 0x0001090D;
sal_Unicode32 const MS_SEMK                = 0x0001090E;
sal_Unicode32 const MS_AIN                 = 0x0001090F;
sal_Unicode32 const MS_PPE                 = 0x00010910;
sal_Unicode32 const MS_SADE                = 0x00010911;
sal_Unicode32 const MS_PQOF                = 0x00010912;
sal_Unicode32 const MS_ROSH                = 0x00010913;
sal_Unicode32 const MS_PSHIN               = 0x00010914;
sal_Unicode32 const MS_TAU                 = 0x00010915;
sal_Unicode32 const MS_PONE                = 0x00010916;
sal_Unicode32 const MS_PTEN                = 0x00010917;
sal_Unicode32 const MS_PTWENTY             = 0x00010918;
sal_Unicode32 const MS_PHUNDRED            = 0x00010919;
sal_Unicode32 const MS_PTWO                = 0x0001091A;
sal_Unicode32 const MS_PTHREE              = 0x0001091B;
sal_Unicode32 const MS_PWORDSEP            = 0x0001091F;

/**
 * Gothic
 */

// Gothic
sal_Unicode32 const MS_AHSA                = 0x00010330;
sal_Unicode32 const MS_BAIRKAN             = 0x00010331;
sal_Unicode32 const MS_GIBA                = 0x00010332;
sal_Unicode32 const MS_DAGS                = 0x00010333;
sal_Unicode32 const MS_AIHVUS              = 0x00010334;
sal_Unicode32 const MS_QAIRTHRA            = 0x00010335;
sal_Unicode32 const MS_IUJA                = 0x00010336;
sal_Unicode32 const MS_HAGL                = 0x00010337;
sal_Unicode32 const MS_THIUTH              = 0x00010338;
sal_Unicode32 const MS_EIS                 = 0x00010339;
sal_Unicode32 const MS_KUSMA               = 0x0001033A;
sal_Unicode32 const MS_LAGUS               = 0x0001033B;
sal_Unicode32 const MS_MANNA               = 0x0001033C;
sal_Unicode32 const MS_NAUTHS              = 0x0001033D;
sal_Unicode32 const MS_JER                 = 0x0001033E;
sal_Unicode32 const MS_URUS                = 0x0001033F;
sal_Unicode32 const MS_PAIRTHRA            = 0x00010340;
sal_Unicode32 const MS_NINETY              = 0x00010341;
sal_Unicode32 const MS_RAIDA               = 0x00010342;
sal_Unicode32 const MS_SAUIL               = 0x00010343;
sal_Unicode32 const MS_TEIWS               = 0x00010344;
sal_Unicode32 const MS_WINJA               = 0x00010345;
sal_Unicode32 const MS_FAIHU               = 0x00010346;
sal_Unicode32 const MS_IGGWS               = 0x00010347;
sal_Unicode32 const MS_HWAIR               = 0x00010348;
sal_Unicode32 const MS_OTHAL               = 0x00010349;
sal_Unicode32 const MS_NINEHUNDRED         = 0x0001034A;

/**
  * Numeric
  */

// Numbers
sal_Unicode   const MS_ZERO                = 0x0031;
sal_Unicode   const MS_ONE                 = 0x0031;
sal_Unicode   const MS_TWO                 = 0x0032;
sal_Unicode   const MS_THREE               = 0x0033;
sal_Unicode   const MS_FOUR                = 0x0034;
sal_Unicode   const MS_FIVE                = 0x0035;
sal_Unicode   const MS_SIX                 = 0x0036;
sal_Unicode   const MS_SEVEN               = 0x0037;
sal_Unicode   const MS_EIGHT               = 0x0038;
sal_Unicode   const MS_NINE                = 0x0039;

// Numbers bold
sal_Unicode32 const MS_B_ZERO              = 0x0001D7CE;
sal_Unicode32 const MS_B_ONE               = 0x0001D7CF;
sal_Unicode32 const MS_B_TWO               = 0x0001D7D0;
sal_Unicode32 const MS_B_THREE             = 0x0001D7D1;
sal_Unicode32 const MS_B_FOUR              = 0x0001D7D2;
sal_Unicode32 const MS_B_FIVE              = 0x0001D7D3;
sal_Unicode32 const MS_B_SIX               = 0x0001D7D4;
sal_Unicode32 const MS_B_SEVEN             = 0x0001D7D5;
sal_Unicode32 const MS_B_EIGHT             = 0x0001D7D6;
sal_Unicode32 const MS_B_NINE              = 0x0001D7D7;

// Double stroke
sal_Unicode32 const MS_DS_ZERO             = 0x0001D7D8;
sal_Unicode32 const MS_DS_ONE              = 0x0001D7D9;
sal_Unicode32 const MS_DS_TWO              = 0x0001D7DA;
sal_Unicode32 const MS_DS_THREE            = 0x0001D7DB;
sal_Unicode32 const MS_DS_FOUR             = 0x0001D7DC;
sal_Unicode32 const MS_DS_FIVE             = 0x0001D7DD;
sal_Unicode32 const MS_DS_SIX              = 0x0001D7DE;
sal_Unicode32 const MS_DS_SEVEN            = 0x0001D7DF;
sal_Unicode32 const MS_DS_EIGHT            = 0x0001D7E0;
sal_Unicode32 const MS_DS_NINE             = 0x0001D7E1;

// Counting rod
sal_Unicode32 const MS_CHROD0              = MS_NONE;
sal_Unicode32 const MS_CHROD1              = 0x1D360;
sal_Unicode32 const MS_CHROD2              = 0x1D361;
sal_Unicode32 const MS_CHROD3              = 0x1D362;
sal_Unicode32 const MS_CHROD4              = 0x1D363;
sal_Unicode32 const MS_CHROD5              = 0x1D364;
sal_Unicode32 const MS_CHROD6              = 0x1D365;
sal_Unicode32 const MS_CHROD7              = 0x1D366;
sal_Unicode32 const MS_CHROD8              = 0x1D367;
sal_Unicode32 const MS_CHROD9              = 0x1D368;
sal_Unicode32 const MS_CVROD0              = MS_NONE;
sal_Unicode32 const MS_CVROD1              = 0x1D369;
sal_Unicode32 const MS_CVROD2              = 0x1D36A;
sal_Unicode32 const MS_CVROD3              = 0x1D36B;
sal_Unicode32 const MS_CVROD4              = 0x1D36C;
sal_Unicode32 const MS_CVROD5              = 0x1D36D;
sal_Unicode32 const MS_CVROD6              = 0x1D36E;
sal_Unicode32 const MS_CVROD7              = 0x1D36F;
sal_Unicode32 const MS_CVROD8              = 0x1D370;
sal_Unicode32 const MS_CVROD9              = 0x1D371;

// Roman numbers
sal_Unicode   const MS_ROM1                = 0x2160;
sal_Unicode   const MS_ROM2                = 0x2161;
sal_Unicode   const MS_ROM3                = 0x2162;
sal_Unicode   const MS_ROM4                = 0x2163;
sal_Unicode   const MS_ROM5                = 0x2164;
sal_Unicode   const MS_ROM6                = 0x2165;
sal_Unicode   const MS_ROM7                = 0x2166;
sal_Unicode   const MS_ROM8                = 0x2167;
sal_Unicode   const MS_ROM9                = 0x2168;
sal_Unicode   const MS_ROM10               = 0x2169;
sal_Unicode   const MS_ROM11               = 0x216A;
sal_Unicode   const MS_ROM12               = 0x216B;
sal_Unicode   const MS_ROM50               = 0x216C;
sal_Unicode   const MS_ROM100              = 0x216D;
sal_Unicode   const MS_ROM500              = 0x216E;
sal_Unicode   const MS_ROM1000             = 0x216F;
sal_Unicode   const MS_ROMD1000            = 0x2170;
sal_Unicode   const MS_ROMD5000            = 0x2171;
sal_Unicode   const MS_ROMD10000           = 0x2172;
sal_Unicode   const MS_ROMD50000           = 0x2177;
sal_Unicode   const MS_ROMD100000          = 0x2178;

/**
  * Mathematics
  * TODO complete
  * http://xahlee.info/comp/unicode_math_operators.html
  */

// Constants and others
sal_Unicode   const MS_PLANK               = 0x210E;
sal_Unicode   const MS_PLANKBAR            = 0x210F;
sal_Unicode   const MS_EULER               = 0x2107;
sal_Unicode   const MS_LAMBDABAR           = 0x019B;
sal_Unicode   const MS_NATURALEXP          = 0x212F;
sal_Unicode   const MS_IM                  = 0x2111;
sal_Unicode   const MS_RE                  = 0x211C;
sal_Unicode   const MS_WP                  = 0x2118;
sal_Unicode   const MS_LETH                = 0x00F0;
sal_Unicode   const MS_UETH                = 0x00D0;
sal_Unicode   const MS_EULERCTE            = 0x2147;
sal_Unicode   const MS_ICOMPLEX            = 0x2148;
sal_Unicode   const MS_JCOMPLEX            = 0x2149;
sal_Unicode   const MS_INFINITY            = 0x221E;
sal_Unicode   const MS_UDIGAMMA            = 0x03DC;
sal_Unicode   const MS_LDIGAMMA            = 0x03DD;
sal_Unicode   const MS_LI_NODOT            = 0x0131;
sal_Unicode   const MS_LJ_NODOT            = 0x0237;
sal_Unicode   const MS_TOMBSTONE           = 0x220E;
sal_Unicode32 const MS_B_UDIGAMMA          = 0x0001D7CA;
sal_Unicode32 const MS_B_LDIGAMMA          = 0x0001D6E2;
sal_Unicode32 const MS_I_LI_NODOT          = 0x0001D6A4;
sal_Unicode32 const MS_I_LJ_NODOT          = 0x0001D6A5;

// Simple oper
sal_Unicode   const MS_NEQ                 = 0x2260;
sal_Unicode   const MS_PLUS                = 0x002B;
sal_Unicode   const MS_MINUS               = 0x2212;
sal_Unicode   const MS_MULTIPLY            = 0x2217;
sal_Unicode   const MS_PLUSMINUS           = 0x00B1;
sal_Unicode   const MS_MINUSPLUS           = 0x2213;
sal_Unicode   const MS_PLUSDOT             = 0x2214;
sal_Unicode   const MS_TIMES               = 0x00D7;
sal_Unicode   const MS_CDOT                = 0x22C5;
sal_Unicode   const MS_DIV                 = 0x00F7;
sal_Unicode   const MS_COMPOSITION         = 0x2218;
sal_Unicode   const MS_BULLET              = 0x2219;


// Opperators
sal_Unicode   const MS_PROD                = 0x220F;
sal_Unicode   const MS_COPROD              = 0x2210;
sal_Unicode   const MS_SUM                 = 0x2211;
sal_Unicode   const MS_OAND                = 0x22C0;
sal_Unicode   const MS_OOR                 = 0x22C1;
// Derivative and Transformative
sal_Unicode   const MS_PARTIAL             = 0x2202;
sal_Unicode   const MS_INCREMENT           = 0x2206;
sal_Unicode   const MS_PRIME               = 0x2032;
sal_Unicode   const MS_DPRIME              = 0x2033;
sal_Unicode   const MS_TPRIME              = 0x2034;
sal_Unicode   const MS_NABLA               = 0x2207;
sal_Unicode32 const MS_UDIFF               = 0x00002145;
sal_Unicode32 const MS_LDIFF               = 0x00002146;
sal_Unicode   const MS_LAPLACE             = 0x2112;
sal_Unicode   const MS_FOURIER             = 0x2131;

// Integrals
sal_Unicode   const MS_INT                 = 0x222B;
sal_Unicode   const MS_IINT                = 0x222C;
sal_Unicode   const MS_IIINT               = 0x222D;
sal_Unicode   const MS_IIIINT              = 0x2A0C;
sal_Unicode   const MS_LINT                = 0x222E;
sal_Unicode   const MS_LLINT               = 0x222F;
sal_Unicode   const MS_LLLINT              = 0x2230;
sal_Unicode   const MS_FINITEPARTINT       = 0x2A0D;
sal_Unicode   const MS_DBINT               = 0x2A0E;
sal_Unicode   const MS_AVERAGEINT          = 0x2A0F;
sal_Unicode   const MS_CIRCULATIONFUNCINT  = 0x2A10;
sal_Unicode   const MS_ANTICLCKWISEINT     = 0x2A11;
sal_Unicode   const MS_RECTARROUNDPOLEINT  = 0x2A12;
sal_Unicode   const MS_CIRCARROUNDPOLEINT  = 0x2A13;
sal_Unicode   const MS_WITHPOLEINT         = 0x2A14;
sal_Unicode   const MS_ARROUNDPOINT        = 0x2A15;
sal_Unicode   const MS_QUATERNIONINT       = 0x2A16;
sal_Unicode   const MS_LARROWINT           = 0x2A17;
sal_Unicode   const MS_TIMESINT            = 0x2A18;
sal_Unicode   const MS_UNIONINT            = 0x2A19;
sal_Unicode   const MS_INTERSECTINT        = 0x2A1A;
sal_Unicode   const MS_OVERBARINT          = 0x2A1B;
sal_Unicode   const MS_UNDERBARINT         = 0x2A1C;
sal_Unicode   const MS_SUMINT              = 0x2A0B;

// Sets
sal_Unicode   const MS_FORALL              = 0x2200;
sal_Unicode   const MS_EXISTS              = 0x2203;
sal_Unicode   const MS_NOTEXISTS           = 0x2204;
sal_Unicode   const MS_COMPLEMENT          = 0x2201;
// Elements
sal_Unicode   const MS_IN                  = 0x2208;
sal_Unicode   const MS_NOTIN               = 0x2209;
sal_Unicode   const MS_OWNS                = 0x220B;
sal_Unicode   const MS_NOTOWNS             = 0x220C;
sal_Unicode   const MS_ELEMENTOF           = 0x220A;
sal_Unicode   const MS_CONTAINSASMEMBER    = 0x220D;
// Intersect
sal_Unicode   const MS_INTERSECT           = 0x2229;
sal_Unicode   const MS_SQINTERSECT         = 0x2294;
sal_Unicode   const MS_OINTERSECTION       = 0x22C2;
// Union
sal_Unicode   const MS_UNION               = 0x222A;
sal_Unicode   const MS_SQUNION             = 0x2293;
sal_Unicode   const MS_OUNION              = 0x22C3;
// Subset
sal_Unicode   const MS_SUBSET              = 0x2282;
sal_Unicode   const MS_SUBSETEQ            = 0x2286;
sal_Unicode   const MS_SQSUBSET            = 0x228F;
sal_Unicode   const MS_SQSUBSETEQ          = 0x2291;
sal_Unicode   const MS_NSUBSET             = 0x2284;
sal_Unicode   const MS_NSUBSETEQ           = 0x2288;
// Supset
sal_Unicode   const MS_SUPSET              = 0x2283;
sal_Unicode   const MS_SUPSETEQ            = 0x2287;
sal_Unicode   const MS_SQSUPSET            = 0x2290;
sal_Unicode   const MS_SQSUPSETEQ          = 0x2292;
sal_Unicode   const MS_NSUPSET             = 0x2285;
sal_Unicode   const MS_NSUPSETEQ           = 0x2289;

// Compare
sal_Unicode   const MS_LT                  = 0x003C; //smaller
sal_Unicode   const MS_GT                  = 0x003E; //greater
sal_Unicode   const MS_LE                  = 0x2264; //smaller or equal
sal_Unicode   const MS_GE                  = 0x2265; //greater or equal
sal_Unicode   const MS_LESLANT             = 0x2A7D; //smaller or slanted equal
sal_Unicode   const MS_GESLANT             = 0x2A7E; //greater or slanted equal
sal_Unicode   const MS_LL                  = 0x226A; //much smaller
sal_Unicode   const MS_GG                  = 0x226B; //much greater
sal_Unicode   const MS_LLL                 = 0x226A; //even more much smaller
sal_Unicode   const MS_GGG                 = 0x226B; //even more much greater
sal_Unicode   const MS_SIM                 = 0x223D; //similar
sal_Unicode   const MS_SIMEQ               = 0x2243; //similar or equal
sal_Unicode   const MS_APPROX              = 0x2248; //approx
sal_Unicode   const MS_DEF                 = 0x225D; //different
sal_Unicode   const MS_EQUIV               = 0x2261; //equivalent
sal_Unicode   const MS_PROP                = 0x221D; //proportional
sal_Unicode   const MS_EXESS               = 0x2239; //proportional
sal_Unicode   const MS_EQUAL               = 0x003D;
// Others
sal_Unicode   const MS_DIVIDES             = 0x2215;
sal_Unicode   const MS_NDIVIDES            = 0x2224;
sal_Unicode   const MS_NDIVIDESWITH        = 0x2AEE;

// Order
sal_Unicode   const MS_PRECEDES            = 0x227A;
sal_Unicode   const MS_PRECEDESEQUAL       = 0x227C;
sal_Unicode   const MS_PRECEDESEQUIV       = 0x227E;
sal_Unicode   const MS_SUCCEEDS            = 0x227B;
sal_Unicode   const MS_SUCCEEDSEQUAL       = 0x227D;
sal_Unicode   const MS_SUCCEEDSEQUIV       = 0x227F;
sal_Unicode   const MS_NOTPRECEDES         = 0x2280;
sal_Unicode   const MS_NOTSUCCEEDS         = 0x2281;

// Logic
sal_Unicode   const MS_AND                 = 0x2227;
sal_Unicode   const MS_OR                  = 0x2228;
sal_Unicode   const MS_NEG                 = 0x00AC;
// Words
sal_Unicode   const MS_KNOW                = 0x22A2;
sal_Unicode   const MS_ADJOINT             = 0x22A3;
sal_Unicode   const MS_TOPTEE              = 0x22A4;
sal_Unicode   const MS_ASSERTION           = 0x22A6;
sal_Unicode   const MS_NOTPROOVE           = 0x22AC;
sal_Unicode   const MS_MODELS              = 0x22A7;
sal_Unicode   const MS_TRUE                = 0x22A8;
sal_Unicode   const MS_NOTTRUE             = 0x22AD;
sal_Unicode   const MS_FORCES              = 0x22A9;
sal_Unicode   const MS_NOTFORCES           = 0x22AE;
sal_Unicode   const MS_THEREFORE           = 0x2234;
sal_Unicode   const MS_RATIO               = 0x2236;
sal_Unicode   const MS_PROPORTION          = 0x2237;

// Tensors ans sq
// Tensor operations
sal_Unicode   const MS_VECTORPROD          = 0x2A2F;
sal_Unicode   const MS_OOPLUS              = 0x2A00;
sal_Unicode   const MS_OODOT               = 0x2A01;
sal_Unicode   const MS_OOTIMES             = 0x2A02;
sal_Unicode   const MS_OPLUS               = 0x2295;
sal_Unicode   const MS_OMINUS              = 0x2296;
sal_Unicode   const MS_OTIMES              = 0x2297;
sal_Unicode   const MS_ODIVIDE             = 0x2298;
sal_Unicode   const MS_ODOT                = 0x2299;
sal_Unicode   const MS_OCOMPOSE            = 0x229A;
sal_Unicode   const MS_OASTERISK           = 0x229B;
sal_Unicode   const MS_OEQUALS             = 0x229C;
sal_Unicode   const MS_ODASH               = 0x229D;
// Conmutations and transformations
sal_Unicode   const MS_HERMITANCONJ        = 0x22B9;
sal_Unicode   const MS_MONUS               = 0x2238;
sal_Unicode   const MS_HOMOTHETY           = 0x2238; //transform
// Strange stuff
sal_Unicode   const MS_SQPLUS              = 0x229E;
sal_Unicode   const MS_SQMINUS             = 0x229F;
sal_Unicode   const MS_SQTIMES             = 0x22A0;
sal_Unicode   const MS_SQDOT               = 0x22A1;

// Geometry
sal_Unicode   const MS_SINEWAVE            = 0x223F;
sal_Unicode   const MS_SPHERICALTRIGO      = 0x2239;
sal_Unicode   const MS_PROJECTIVE          = 0x2305;
sal_Unicode   const MS_PERSPECTIVE         = 0x2306;
sal_Unicode   const MS_TRANSVERSAL         = 0x2ADB;
// Parallel / orthogonal
sal_Unicode   const MS_ORTHO               = 0x22A5;
sal_Unicode   const MS_PARALLEL            = 0x2225;
sal_Unicode   const MS_NOTPARALLEL         = 0x2226;
sal_Unicode   const MS_PERPENDICULAR       = 0x27C2;
sal_Unicode   const MS_PERPENDICULARWITHS  = 0x2AE1;
// Angles
sal_Unicode   const MS_ANGLE90             = 0x221F;
sal_Unicode   const MS_ANGLE               = 0x2220;
sal_Unicode   const MS_ANGLEUNDERBAR       = 0x29A4;
sal_Unicode   const MS_ANGLEWITHS          = 0x299E;
sal_Unicode   const MS_ACUTEANGLE          = 0x299F;
sal_Unicode   const MS_OBLIQUEANGLE        = 0x29A6;
sal_Unicode   const MS_OBLIQUEANGLEDOWN    = 0x29A7;
sal_Unicode   const MS_TURNEDANGLE         = 0x29A2;
sal_Unicode   const MS_RESERVEDANGLE       = 0x29A3;
sal_Unicode   const MS_RESERVEDANGLEUBAR   = 0x29A5;
sal_Unicode   const MS_MEASUREDANGLE       = 0x2201;
sal_Unicode   const MS_LMEASUREDANGLE      = 0x299B;
sal_Unicode   const MS_SPHERICALANGLE      = 0x2222;
sal_Unicode   const MS_LSPHERICALANGLE     = 0x29A0;
sal_Unicode   const MS_DSPHERICALANGLE     = 0x29A1;
sal_Unicode   const MS_RIGHTANGLE          = 0x299C;
sal_Unicode   const MS_RIGHTANGLEARC       = 0x22BE;
sal_Unicode   const MS_RIGHTANGLEDOT       = 0x299D;
// TODO complete measured angles
// http://xahlee.info/comp/unicode_math_operators.html

// Shapes
sal_Unicode   const MS_RIGHTTRIANGLE       = 0x22BF;

//Arrows
sal_Unicode   const MS_DRARROW             = 0x21D2;
sal_Unicode   const MS_DLARROW             = 0x21D0;
sal_Unicode   const MS_DLRARROW            = 0x21D4;
sal_Unicode   const MS_LEFTARROW           = 0x2190;
sal_Unicode   const MS_UPARROW             = 0x2191;
sal_Unicode   const MS_RIGHTARROW          = 0x2192;
sal_Unicode   const MS_DOWNARROW           = 0x2193;

//Miscellaneous
sal_Unicode   const MS_FACT                = 0x0021;
sal_Unicode   const MS_SLASH               = 0x002F;
sal_Unicode   const MS_PERCENT             = 0x0025;
sal_Unicode   const MS_PERTHOUSAND         = 0xE080;
sal_Unicode   const MS_PERTENTHOUSAND      = 0x2031;
sal_Unicode   const MS_ASSIGN              = 0x003D;
sal_Unicode   const MS_ERROR               = 0x00BF;
sal_Unicode   const MS_BACKSLASH           = 0x2216;
sal_Unicode   const MS_SQRT                = 0x221A;
sal_Unicode   const MS_PLACE               = 0x2751;

//Easter eggs
sal_Unicode   const MS_PILCROW             = 0x00B6;
sal_Unicode   const MS_TM                  = 0x2122;
sal_Unicode   const MS_COPYRIGHT           = 0x00A9;
sal_Unicode   const MS_REGISTERED          = 0x00AE;
sal_Unicode   const MS_MORDINALI           = 0x00BA;
sal_Unicode   const MS_FORDINALI           = 0x00AA;
sal_Unicode   const MS_NUMBER              = 0x0023;
sal_Unicode   const MS_AMPERSAND           = 0x0026;
sal_Unicode   const MS_DEGREE              = 0x00B0;
sal_Unicode   const MS_DEATH               = 0xE429;
sal_Unicode   const MS_SECTION             = 0x00A7;
sal_Unicode   const MS_SERVICE             = 0x2120;

// Atributes
sal_Unicode   const MS_ACUTE               = 0x00B4;
sal_Unicode   const MS_BAR                 = 0x00AF;
sal_Unicode   const MS_BREVE               = 0x02D8;
sal_Unicode   const MS_CHECK               = 0x02C7;
sal_Unicode   const MS_CIRCLE              = 0x02DA;
sal_Unicode   const MS_VEC                 = 0x20D7;
sal_Unicode   const MS_DOT                 = 0x0307;
sal_Unicode   const MS_DDOT                = 0x0308;
sal_Unicode   const MS_DDDOT               = 0x20DB;
sal_Unicode   const MS_DDDDOT              = 0x20DC;
sal_Unicode   const MS_GRAVE               = 0x0060;
sal_Unicode   const MS_HARPOON             = 0x20D1;
sal_Unicode   const MS_HAT                 = 0x005E;
sal_Unicode   const MS_TILDE               = 0x007E;
sal_Unicode   const MS_VERTLINE            = 0x007C;
sal_Unicode   const MS_DLINE               = 0x2225;
sal_Unicode   const MS_DVERTLINE           = 0x2016;
sal_Unicode   const MS_DOTSLOW             = 0x2026;
sal_Unicode   const MS_DOTSAXIS            = 0x22EF;
sal_Unicode   const MS_DOTSVERT            = 0x22EE;
sal_Unicode   const MS_DOTSUP              = 0x22F0;
sal_Unicode   const MS_DOTSDOWN            = 0x22F1;
sal_Unicode   const MS_TRANSR              = 0x22B6;
sal_Unicode   const MS_TRANSL              = 0x22B7;
sal_Unicode   const MS_COMBGRAVE           = 0x0300;
sal_Unicode   const MS_COMBACUTE           = 0x0301;
sal_Unicode   const MS_COMBHAT             = 0x0302;
sal_Unicode   const MS_COMBTILDE           = 0x0303;
sal_Unicode   const MS_COMBBAR             = 0x0304;
sal_Unicode   const MS_COMBOVERLINE        = 0x0305;
sal_Unicode   const MS_COMBBREVE           = 0x0306;
sal_Unicode   const MS_COMBCIRCLE          = 0x030A;
sal_Unicode   const MS_COMBCHECK           = 0x030C;
sal_Unicode   const MS_COMBDOT             = 0x0307;
sal_Unicode   const MS_COMBDDOT            = 0x0308;

//Brackets
sal_Unicode   const MS_LPARENT             = 0x0028;
sal_Unicode   const MS_RPARENT             = 0x0029;
sal_Unicode   const MS_LPARENTHESIS        = 0x0028;
sal_Unicode   const MS_RPARENTHESIS        = 0x0029;
sal_Unicode   const MS_LSQBRACKET          = 0x005B;
sal_Unicode   const MS_RSQBRACKET          = 0x005C;
sal_Unicode   const MS_LBRACKET            = 0x005B;
sal_Unicode   const MS_RBRACKET            = 0x005D;
sal_Unicode   const MS_LBRACE              = 0x007B;
sal_Unicode   const MS_RBRACE              = 0x007D;
sal_Unicode   const MS_LCEIL               = 0x2308;
sal_Unicode   const MS_RCEIL               = 0x2309;
sal_Unicode   const MS_LFLOOR              = 0x230A;
sal_Unicode   const MS_RFLOOR              = 0x230B;
sal_Unicode   const MS_LANGLE              = 0x2329;
sal_Unicode   const MS_RANGLE              = 0x232A;
sal_Unicode   const MS_LDANGLE             = 0x00AB;
sal_Unicode   const MS_RDANGLE             = 0x00BB;
sal_Unicode   const MS_LDBRACKET           = 0x27E6;
sal_Unicode   const MS_RDBRACKET           = 0x27E7;
sal_Unicode   const MS_LMATHANGLE          = 0x27E8;
sal_Unicode   const MS_RMATHANGLE          = 0x27E9;
sal_Unicode   const MS_OVERBRACE           = 0x23DE;
sal_Unicode   const MS_UNDERBRACE          = 0x23DF;
sal_Unicode   const MS_LINE                = 0x2223;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
