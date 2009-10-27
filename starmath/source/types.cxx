/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: types.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"


#include <types.hxx>


//!! no official unicode code point for these in non-private-area
//!! (even MathMl uses the code points listed on the right which are
//!! from the private-use-area as well)
//    MS_UNDERBRACE   = (xub_Unicode) 0xE081,         0xF613 ?
//    MS_OVERBRACE    = (xub_Unicode) 0xE082,         0xF612 ?

struct MathConversionEntry
{
    sal_Unicode     cMath;      // code point of the character from the
                                // StarSymbol/OpenSymbol private-use-area
    sal_Unicode     cUnicode;   // 'official' Unicode code point the above
                                // character should be matched to and vice versa.
                                // See http://www.w3.org/TR/2000/WD-MathML2-20000211/bycodes.html
                                // for most of them.
//    const sal_Unicode *pFontName;
    sal_Unicode     cMathType;  // 0 if the same value of cUnicode is to be used
};

static MathConversionEntry aMathConvTbl[] =
{
//    MS_UNDERBRACE,  0xF613,       // only Math-ML code-point available !
//    MS_OVERBRACE,   0xF612,       // only Math-ML code-point available !
    { MS_PLUS,        0x002B,     0 },
    { MS_LT,          0x003C,     0 },
    { MS_GT,          0x003E,     0 },
    { MS_LESLANT,     0x2264,     0 },
    { MS_GESLANT,     0x2265,     0 },
    { MS_DOTSLOW,     0x2026,     0 },
    { MS_RIGHTARROW,  0x2192,     0 },
    { MS_SQRT,        0x221A,     0 },
    { MS_HAT,         0x02C6,     0 },
    { MS_CHECK,       0x02C7,     0 },
    { MS_BREVE,       0x02D8,     0 },
    { MS_ACUTE,       0x00B4,     0 },
    { MS_GRAVE,       0x0060,     0 },
    { MS_TILDE,       0x02DC,     0 },
    { MS_BAR,         0x02C9,     0 },
    { MS_VEC,         0x2192,     0 },
    { MS_DDDOT,       0x22EF,     0 },
    { MS_LPARENT,     0x0028,     0 },
    { MS_RPARENT,     0x0029,     0 },
    { MS_PLACE,       0x25A1,     0 },
    { MS_LEFTARROW,   0x2190,     0 },
    { MS_UPARROW,     0x2191,     0 },
    { MS_DOWNARROW,   0x2193,     0 },
    { MS_PLACE,       0x2751,     0 },

    // predefined-user-defined symbols
    // (code-points on the left as defined in 'officecfg' for SO8,
    // non-private-use-area characters uncommented):

//    0x0041,         0x0391,     0,     // %ALPHA
//    0x0042,         0x0392,     0,     // %BETA
    { 0xE0AC,         0x0393,     0 },      // %GAMMA
    { 0xE0AD,         0x0394,     0 },      // %DELTA
//    0x0045,         0x0395,     0,     // %EPSILON
//    0x005A,         0x0396,     0,     // %ZETA
//    0x0048,         0x0397,     0,     // %ETA
    { 0xE0AE,         0x0398,     0 },      // %THETA
//    0x0049,         0x0399,     0,     // %IOTA
//    0x004B,         0x039A,     0,     // %KAPPA
    { 0xE0AF,         0x039B,     0 },      // %LAMBDA
//    0x004D,         0x039C,     0,     // %MU
//    0x004E,         0x039D,     0,     // %NU
    { 0xE0B0,         0x039E,     0 },      // %XI
//    0x004F,         0x039F,     0,     // %OMICRON
    { 0xE0B1,         0x03A0,     0 },      // %PI
//    0x0050,         0x03A1,     0,     // %RHO
    { 0xE0B2,         0x03A3,     0 },      // %SIGMA
//    0x0054,         0x03A4,     0,     // %TAU
    { 0xE0B3,         0x03A5,     0 },      // %UPSILON
    { 0xE0B4,         0x03A6,     0 },      // %PHI
//    0x0058,         0x03A7,     0,     // %CHI
    { 0xE0B5,         0x03A8,     0 },      // %PSI
    { 0xE0B6,         0x03A9,     0 },      // %OMEGA

    { 0xE0B7,         0x03B1,     0 },      // %alpha
    { 0xE0B8,         0x03B2,     0 },      // %beta
    { 0xE0B9,         0x03B3,     0 },      // %gamma
    { 0xE0BA,         0x03B4,     0 },      // %delta
    { 0xE0BB,         0x03B5,     0 },      // %epsilon
    { 0xE0BC,         0x03B6,     0 },      // %zeta
    { 0xE0BD,         0x03B7,     0 },      // %eta
    { 0xE0BE,         0x03B8,     0 },      // %theta
    { 0xE0BF,         0x03B9,     0 },      // %iota
    { 0xE0C0,         0x03BA,     0 },      // %kappa
    { 0xE0C1,         0x03BB,     0 },      // %lambda
    { 0xE0C2,         0x03BC,     0 },      // %mu
    { 0xE0C3,         0x03BD,     0 },      // %nu
    { 0xE0C4,         0x03BE,     0 },      // %xi
    { 0xE0C5,         0x03BF,     0 },      // %omicron
    { 0xE0C6,         0x03C0,     0 },      // %pi
    { 0xE0C7,         0x03C1,     0 },      // %rho
    { 0xE0C8,         0x03C3,     0 },      // %sigma
    { 0xE0C9,         0x03C4,     0 },      // %tau
    { 0xE0CA,         0x03C5,     0 },      // %upsilon
    { 0xE0CB,         0x03C6,     0 },      // %phi
    { 0xE0CC,         0x03C7,     0 },      // %chi
    { 0xE0CD,         0x03C8,     0 },      // %psi
    { 0xE0CE,         0x03C9,     0 },      // %omega

    { 0xE0CF,         0x025B,     0,/*x03B5*/}, // %varepsilon
    { 0xE0D0,         0x03D1,     0 },      // %vartheta
    { 0xE0D4,         0x03D5,     0 },      // %varphi
    { 0xE0D1,         0x03D6,     0 },      // %varpi
//    0x03F1,         0x03F1,     0/*x03C1*/, // %varrho
    { 0xE0D3,         0x03C2,     0 },      // %varsigma

//    0x2227,         0x2227,     0,     // %and
//    0x2228,         0x2228,     0,     // %or
//    0x2208,         0x2208,     0,     // %element
//    0x2209,         0x2209,     0,     // %noelement
//    0x226B,         0x226B,     0,     // %strictlygreaterthan
//    0x226A,         0x226A,     0,     // %strictlylessthan
//    0x2261,         0x2261,     0,     // %identical
//    0x221E,         0x221E,     0,     // %infinite
//    0x2260,         0x2260,     0,     // %notequal
    { 0xE080,         0x2030,     0 },      // %perthousand
//    0x2222,         0x2222,     0,      // %angle
    { 0xE08C,         0x2192,     0 }       // %tendto
};


static int nMathToUnicodeLen = sizeof(aMathConvTbl) / sizeof(aMathConvTbl[0]);

sal_Unicode ConvertMathPrivateUseAreaToUnicode( sal_Unicode cChar )
{
    sal_Unicode cRes = 0;
    if (IsInPrivateUseArea( cChar ))
    {
        for (int i = 0;  i < nMathToUnicodeLen && cRes == 0;  ++i)
        {
            if (aMathConvTbl[i].cMath == cChar)
                cRes = aMathConvTbl[i].cUnicode;
        }
    }
    return cRes;
}

#ifdef TL_NOT_YET_USED
sal_Unicode ConvertUnicodeToMathPrivateUseArea( sal_Unicode cChar )
{
    sal_Unicode cRes = 0;
    for (int i = 0;  i < nMathToUnicodeLen && cRes == 0;  ++i)
    {
        if (aMathConvTbl[i].cUnicode == cChar)
            cRes = aMathConvTbl[i].cMath;
    }
    return cRes;
}


sal_Unicode ConvertMathToMathType( sal_Unicode cChar )
{
    sal_Unicode cRes = 0;
    for (int i = 0;  i < nMathToUnicodeLen && cRes == 0;  ++i)
    {
        const MathConversionEntry &rEntry = aMathConvTbl[i];
        if (rEntry.cMath == cChar)
        {
            cRes = rEntry.cMathType;
            if (cRes == 0)
                cRes = rEntry.cUnicode;
        }
    }
    return cRes;
}

sal_Unicode ConvertMathTypeToMath( sal_Unicode cChar )
{
    sal_Unicode cRes = 0;
    for (int i = 0;  i < nMathToUnicodeLen && cRes == 0;  ++i)
    {
        const MathConversionEntry &rEntry = aMathConvTbl[i];
        sal_Unicode cTmp = rEntry.cMathType;
        if (cTmp == 0)
            cTmp = rEntry.cUnicode;
        if (cTmp == cChar)
            cRes = rEntry.cMath;
    }
    return cRes;
}
#endif //TL_NOT_YET_USED

sal_Unicode ConvertMathToMathML( sal_Unicode cChar )
{
    sal_Unicode cRes = 0;
    if (cChar == MS_OVERBRACE)
        cRes = 0xF612;      // Math-ML code-point (there is no official unicode-point)
    else if (cChar == MS_UNDERBRACE)
        cRes = 0xF613;      // Math-ML code-point (there is no official unicode-point)
    else
        cRes = ConvertMathPrivateUseAreaToUnicode( cChar );
    return cRes;
}

#ifdef TL_NOT_YET_USED
sal_Unicode ConvertMathMLToMath( sal_Unicode cChar )
{
    sal_Unicode cRes = 0;
    if (cChar == 0xF612)
        cRes = MS_OVERBRACE;      // Math-ML code-point (there is no official unicode-point)
    else if (cChar == 0xF613)
        cRes = MS_UNDERBRACE;      // Math-ML code-point (there is no official unicode-point)
    else
        cRes = ConvertUnicodeToMathPrivateUseArea( cChar );
    return cRes;
}


sal_Unicode GetTokenChar( sal_Unicode cChar, sal_Bool bConvertForExport )
{
    sal_Unicode cRes = cChar;
    if (bConvertForExport)
    {
        sal_Unicode cTmp = ConvertMathPrivateUseAreaToUnicode( cChar );
        if (cTmp != 0)
            cRes = cTmp;
    }
    return cRes;
}
#endif //TL_NOT_YET_USED


