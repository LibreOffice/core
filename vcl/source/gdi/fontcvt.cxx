/*************************************************************************
 *
 *  $RCSfile: fontcvt.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: th $ $Date: 2001-06-21 21:13:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_FONTCVT_HXX
#include <fontcvt.hxx>
#endif

#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif

//=======================================================================
// note: the character mappings that are only approximations
//       are marked (with an empty comment)

static const sal_Unicode aStarBatsTab[224] =
{
    // F020
        0x0020,    0x263a,    0x25cf,    0x274d,
        0x25a0,    0x25a1,/**/0x25a1,    0x2751,
        0x2752,/**/0x25c6,/**/0x25c6,/**/0x25c6,
        0x2756,/**/0x25c6,/**/0x274d,    0x27a2,
    // F030
    /**/0x2192,    0x2794,/**/0x2714,    0x2612,
        0x2611,    0x27b2,    0x261b,/**/0x270d,
        0x270e,    0x2713,    0x2714,/**/0x25cf,
    /**/0x274d,/**/0x25a0,    0x274f,    0x2750,
    // F040
    /**/0x2752,/**/0x25c6,/**/0x2756,    0x25c6,
        0x2722,    0x2723,    0x2724,    0x2725,
        0x2733,    0x2734,    0x2735,    0x2736,
        0x2737,    0x2738,    0x2739,    0x2717,
    // F050
        0x2718,    0x2719,    0x271a,    0x271b,
        0x271c,    0x272b,    0x272c,    0x272d,
        0x272e,    0x272f,    0x2730,         0,
             0,    0x278a,    0x278b,    0x278c,
    // F060
        0x278d,    0x278e,    0x278f,    0x2790,
        0x2791,    0x2792,    0x2793,         0,
        0x2780,    0x2781,    0x2782,    0x2783,
        0x2784,    0x2785,    0x2786,    0x2787,
    // F070
        0x2788,    0x2789,         0,         0,
        0x260e,         0,         0,         0,
             0,         0,         0,         0,
             0,    0x261e,    0x270d,         0,
    // F080
        0x20ac,         0,    0x201a,    0x0192,
        0x201e,    0x2026,    0x2020,    0x2021,
        0x02c6,    0x2030,    0x0160,    0x2039,
        0x0152,         0,    0x017d,         0,
    // F090
             0,    0x2018,    0x2019,    0x201c,
        0x201d,    0x2022,    0x2013,    0x2014,
        0x02dc,    0x2122,    0x0161,    0x203a,
        0x0153,         0,    0x017e,    0x0178,
    // F0A0
             0,         0,         0,    0x21e9,
        0x21e6,/**/0x0153,         0,    0x00a7,
             0,/**/0x00a9,         0,    0x00ab,
    /**/0x263a,    0x2639,/**/0x00ae,    0x21e8,
    // F0B0
    /**/0x21e9,/**/0x21e6,    0x21e7,    0x21e5,
        0x21e4,    0x2192,    0x2193,    0x2190,
        0x2191,/**/0x2730,         0,    0x00bb,
    /**/0x2192,/**/0x2193,/**/0x2190,/**/0x2191,
    // F0C0
             0,         0,         0,         0,
        0x25be,    0x25b4,    0x25bf,    0x25b5,
             0,         0,/**/0x0020,    0x2702,
        0x2708,    0x2721,    0x273f,    0x2744,
    // F0D0
        0x25d7,    0x2759,         0,         0,
        0x2762,    0x2663,    0x2665,    0x2660,
        0x2194,    0x2195,    0x2798,    0x279a,
        0x27b8,         0,    0x00b6,         0,
    // F0E0
        0x00a2,    0x00a4,    0x00a5,         0,
        0x20a1,    0x20a2,    0x20a3,    0x20a4,
        0x20a9,    0x20ab,    0x20a8,         0,
             0,         0,         0,         0,
    // F0F0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0
};

// -----------------------------------------------------------------------

static const sal_Unicode aStarMathTab[224] =
{
    // F020
    0x0020, 0x0021, 0x0022, 0x0023,
    0x2030, 0x0025, 0x0026, 0x221e,
    0x0028, 0x0029, 0x002a, 0x002b,
    0x002c, 0x002d, 0x002e, 0x002f,
    // F030
    0x2224, 0x21d2, 0x21d0, 0x21d4,
         0,      0, 0x00b0,      0,
         0,      0, 0x003a, 0x003b,
    0x003c, 0x003d, 0x003e, 0x00bf,
    // F040
    0x2260,/**/0x002b, 0x2212, 0x2217,
    0x00d7,    0x2219, 0x00f7, 0x00b1,
    0x2213,    0x2295, 0x2296, 0x2297,
    0x2298,    0x2299, 0x222a, 0x2229,
    // F050
    /**/0x003c,/**/0x003e, 0x2264, 0x2265,
    /**/0x2264,/**/0x2265, 0x226a, 0x226b,
    0x007e, 0x2243, 0x2248, 0x225d,
    0x2261, 0x221d,/**/0x2202, 0x2282,
    // F060
    0x2283, 0x2286, 0x2287, 0x2284,
    0x2285, 0x2288, 0x2289, 0x220b,
    0x2209,/**/0x2208, 0x2203, 0x220d,
    0x2135, 0x2111, 0x211c, 0x2118,
    // F070
    0x0192, 0x2221, 0x2222, 0x2223,
         0, 0x22a5,      0, 0x22ef,
    0x22ee, 0x22f0, 0x22f1, 0x22b6,
    0x22b7,      0, 0x2216,      0,
    // F080
    0x222b, 0x222c, 0x222d, 0x222e,
    0x222f, 0x2230, 0x221a,      0,
         0,      0, 0x2210, 0x220f,
    0x2211, 0x2207, 0x2200, 0x2225,
    // F090
         0,      0,      0,      0,
         0,      0,      0,      0,
         0,      0,      0,      0,
    0x2227, 0x2228, 0x220b, 0x2205,
    // F0A0
    0x007b, 0x007d,      0,      0,
    0x2329, 0x232a, 0x005b, 0x005d,
         0, 0x22be,      0, 0x2259,
    0x2254, 0x2255, 0x21b3, 0x2197,
    // F0B0
    0x2198, 0x2245,      0,      0,
         0,      0,      0, 0x22a4,
    0x2112, 0x2130, 0x2131,      0,
         0,      0,      0,      0,
    // F0C0
    0x2308, 0x230a, 0x2309, 0x230b,
         0,      0,      0,      0,
         0,      0,      0,      0,
         0,      0,      0,      0,
    // F0D0
         0,      0,      0,      0,
         0,      0,      0,      0,
         0,      0,      0,      0,
         0,      0,      0,      0,
    // F0E0
         0,      0,      0,      0,
         0,      0,      0,      0,
         0,      0,      0,      0,
         0,      0,      0, 0x2202,
    // F0F0
    0x2113,      0,      0,      0,
    0x210a,      0, 0x210f,      0,
         0,      0,      0, 0x2115,
    0x2124, 0x211a, 0x211d, 0x2102
};

// -----------------------------------------------------------------------

/*
static const sal_Unicode aWingDingsTab[224] =
{
//TODO:
};

// -----------------------------------------------------------------------

static const sal_Unicode aWingDings2Tab[224] =
{
//TODO:
};

// -----------------------------------------------------------------------

static const sal_Unicode aWingDings3Tab[224] =
{
//TODO:
};

// -----------------------------------------------------------------------

static const sal_Unicode aWebDingsTab[224] =
{
//TODO:
};

// -----------------------------------------------------------------------

static const sal_Unicode aMonoTypeSortsTab[224] =
{
//TODO:
};

// -----------------------------------------------------------------------

static const sal_Unicode aMonoTypeSorts2Tab[224] =
{
//TODO:
};
*/

//=======================================================================

static sal_Unicode ImplStarSymbolToStarBats( sal_Unicode c )
{
    switch ( c )
    {
        case 0x00A2: c = 0xF0E0; break;
        case 0x00A4: c = 0xF0E1; break;
        case 0x00A5: c = 0xF0E2; break;
        case 0x00A7: c = 0xF0A7; break;
        case 0x00AB: c = 0xF0AB; break;
        case 0x00B6: c = 0xF0DE; break;
        case 0x00BB: c = 0xF0BB; break;

        case 0x0152: c = 0xF08C; break;
        case 0x0153: c = 0xF09C; break;
        case 0x0160: c = 0xF08A; break;
        case 0x0161: c = 0xF09A; break;
        case 0x0178: c = 0xF09F; break;
        case 0x017D: c = 0xF08E; break;
        case 0x017E: c = 0xF09E; break;

        case 0x0192: c = 0xF083; break;
        case 0x02C6: c = 0xF088; break;
        case 0x02DC: c = 0xF098; break;
        case 0x2013: c = 0xF096; break;
        case 0x2014: c = 0xF097; break;
        case 0x2018: c = 0xF091; break;

        case 0x2019: c = 0xF092; break;
        case 0x201A: c = 0xF082; break;
        case 0x201C: c = 0xF093; break;
        case 0x201D: c = 0xF094; break;
        case 0x201E: c = 0xF084; break;
        case 0x2020: c = 0xF086; break;

        case 0x2021: c = 0xF087; break;
        case 0x2022: c = 0xF095; break;
        case 0x2026: c = 0xF085; break;
        case 0x2030: c = 0xF089; break;
        case 0x2039: c = 0xF08B; break;
        case 0x203A: c = 0xF09B; break;

        case 0x20A1: c = 0xF0E4; break;
        case 0x20A2: c = 0xF0E5; break;
        case 0x20A3: c = 0xF0E6; break;
        case 0x20A4: c = 0xF0E7; break;
        case 0x20A8: c = 0xF0EA; break;
        case 0x20A9: c = 0xF0E8; break;
        case 0x20AB: c = 0xF0E9; break;
        case 0x20AC: c = 0xF080; break;

        case 0x2122: c = 0xF099; break;
        case 0x2190: c = 0xF0B7; break;
        case 0x2191: c = 0xF0B8; break;
        case 0x2192: c = 0xF0B5; break;
        case 0x2193: c = 0xF0B6; break;
        case 0x2194: c = 0xF0D8; break;
        case 0x2195: c = 0xF0D9; break;

        case 0x21E4: c = 0xF0B4; break;
        case 0x21E5: c = 0xF0B3; break;
        case 0x21E6: c = 0xF0A4; break;
        case 0x21E7: c = 0xF0B2; break;
        case 0x21E8: c = 0xF0AF; break;
        case 0x21E9: c = 0xF0A3; break;

        case 0x25A0: c = 0xF024; break;
        case 0x25A1: c = 0xF025; break;
        case 0x25B4: c = 0xF0C5; break;
        case 0x25B5: c = 0xF0C7; break;
        case 0x25BE: c = 0xF0C4; break;
        case 0x25BF: c = 0xF0C6; break;
        case 0x25C6: c = 0xF043; break;
        case 0x25CF: c = 0xF022; break;
        case 0x25D7: c = 0xF0D0; break;

        case 0x260E: c = 0xF074; break;
        case 0x2611: c = 0xF034; break;
        case 0x2612: c = 0xF033; break;
        case 0x261B: c = 0xF036; break;
        case 0x261E: c = 0xF07D; break;
        case 0x2639: c = 0xF0AD; break;
        case 0x263A: c = 0xF021; break;

        case 0x2702: c = 0xF0CB; break;
        case 0x2708: c = 0xF0CC; break;
        case 0x270D: c = 0xF07E; break;
        case 0x270E: c = 0xF038; break;

        case 0x2713: c = 0xF039; break;
        case 0x2714: c = 0xF03A; break;
        case 0x2717: c = 0xF04F; break;
        case 0x2718: c = 0xF050; break;
        case 0x2719: c = 0xF051; break;
        case 0x271A: c = 0xF052; break;
        case 0x271B: c = 0xF053; break;
        case 0x271C: c = 0xF054; break;

        case 0x2721: c = 0xF0CD; break;
        case 0x2722: c = 0xF044; break;
        case 0x2723: c = 0xF045; break;
        case 0x2724: c = 0xF046; break;
        case 0x2725: c = 0xF047; break;
        case 0x272B: c = 0xF055; break;
        case 0x272C: c = 0xF056; break;
        case 0x272D: c = 0xF057; break;
        case 0x272E: c = 0xF058; break;
        case 0x272F: c = 0xF059; break;

        case 0x2730: c = 0xF05A; break;
        case 0x2733: c = 0xF048; break;
        case 0x2734: c = 0xF049; break;
        case 0x2735: c = 0xF04A; break;
        case 0x2736: c = 0xF04B; break;
        case 0x2737: c = 0xF04C; break;
        case 0x2738: c = 0xF04D; break;
        case 0x2739: c = 0xF04E; break;
        case 0x273F: c = 0xF0CE; break;

        case 0x2744: c = 0xF0CF; break;
        case 0x274D: c = 0xF023; break;
        case 0x274F: c = 0xF03E; break;
        case 0x2750: c = 0xF03F; break;
        case 0x2751: c = 0xF027; break;
        case 0x2752: c = 0xF028; break;
        case 0x2756: c = 0xF02C; break;
        case 0x2759: c = 0xF0D1; break;
        case 0x2762: c = 0xF0D4; break;

        case 0x2780: c = 0xF068; break;
        case 0x2781: c = 0xF069; break;
        case 0x2782: c = 0xF06A; break;
        case 0x2783: c = 0xF06B; break;
        case 0x2784: c = 0xF06C; break;
        case 0x2785: c = 0xF06D; break;
        case 0x2786: c = 0xF06E; break;
        case 0x2787: c = 0xF06F; break;
        case 0x2788: c = 0xF070; break;
        case 0x2789: c = 0xF071; break;
        case 0x278A: c = 0xF05D; break;
        case 0x278B: c = 0xF05E; break;
        case 0x278C: c = 0xF05F; break;
        case 0x278D: c = 0xF060; break;
        case 0x278E: c = 0xF061; break;
        case 0x278F: c = 0xF062; break;

        case 0x2790: c = 0xF063; break;
        case 0x2791: c = 0xF064; break;
        case 0x2792: c = 0xF065; break;
        case 0x2793: c = 0xF066; break;
        case 0x2794: c = 0xF031; break;
        case 0x2798: c = 0xF0DA; break;
        case 0x279A: c = 0xF0DB; break;

        case 0x27A2: c = 0xF02F; break;
        case 0x27B2: c = 0xF035; break;
        case 0x27B8: c = 0xF0DC; break;

        default: c = 0; break;
    }

    return c;
}

//=======================================================================

static ImplCvtChar aStarBatsCvt = {aStarBatsTab, "StarSymbol", NULL};
static ImplCvtChar aStarMathCvt = {aStarMathTab, "StarSymbol", NULL};
static ImplCvtChar aStarSymbolCvt = {NULL, "StarBats", ImplStarSymbolToStarBats};
/*
static ImplCvtChar aStarMathConversion = {aWingDingsTab, "StarSymbol", NULL};
static ImplCvtChar aStarMathConversion = {aWingDings2Tab, "StarSymbol", NULL};
static ImplCvtChar aStarMathConversion = {aWingDings3Tab, "StarSymbol", NULL};
static ImplCvtChar aStarMathConversion = {aWebDingsTab, "StarSymbol", NULL};
static ImplCvtChar aStarMathConversion = {aMonoTypeSortsTab, "StarSymbol", NULL};
static ImplCvtChar aStarMathConversion = {aMonoTypeSorts2Tab, "StarSymbol", NULL};
*/

//=======================================================================

sal_Unicode ImplRecodeChar( const ImplCvtChar* pConversion, sal_Unicode cChar )
{
    sal_Unicode cRetVal = cChar;
    if( pConversion->mpCvtFunc )
        cRetVal = pConversion->mpCvtFunc( cChar );
    else if( cChar>= 0xF020 && cChar<=0xF0FF )
        cRetVal = pConversion->mpCvtTab[ cChar - 0xF020 ];
    return cRetVal ? cRetVal : cChar;
}

// -----------------------------------------------------------------------

void ImplRecodeString( const ImplCvtChar* pConversion, String& rStr,
           xub_StrLen nIndex, xub_StrLen nLen )
{
    ULONG nLastIndex = (ULONG)nIndex + nLen;
    if ( nLastIndex > rStr.Len() )
        nLastIndex = rStr.Len();

    for(; nIndex < nLastIndex; ++nIndex )
    {
        sal_Unicode c = rStr.GetChar( nIndex );
        if( (c >= 0xF020) && (c <= 0xF0FF) )
            rStr.SetChar( nIndex, ImplRecodeChar( pConversion, c ) );
    }
}

// -----------------------------------------------------------------------

const ImplCvtChar* ImplGetRecodeData( const String& rOrgFontName,
                                      const String& rMapFontName )
{
    return NULL;
}

//=======================================================================

FontToSubsFontConverter CreateFontToSubsFontConverter(
    const String& rFontName, ULONG nFlags )
{
    const ImplCvtChar* pCvt = NULL;

    String aName = rFontName;
    ImplGetEnglishSearchFontName( aName );

    if ( nFlags & FONTTOSUBSFONT_IMPORT )
    {
        // FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS:
        if( aName.EqualsAscii( "starbats" ) )       pCvt = &aStarBatsCvt;
        else if( aName.EqualsAscii( "starmath" ) )  pCvt = &aStarMathCvt;
    }
    else
    {
        // FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS:
        if( aName.EqualsAscii( "starsymbol" ) )     pCvt = &aStarSymbolCvt;
    }

    return (FontToSubsFontConverter)pCvt;
}

// -----------------------------------------------------------------------

void DestroyFontToSubsFontConverter( FontToSubsFontConverter hConverter )
{
    // nothing to do for now, because we use static ImplCvtChars
}

// -----------------------------------------------------------------------

sal_Unicode ConvertFontToSubsFontChar(
    FontToSubsFontConverter hConverter, sal_Unicode cChar )
{
    if ( hConverter )
        return ImplRecodeChar( (ImplCvtChar*)hConverter, cChar );
    else
        return cChar;
}

// -----------------------------------------------------------------------

String GetFontToSubsFontName( FontToSubsFontConverter hConverter )
{
    if ( hConverter )
    {
        const char* pName = ((ImplCvtChar*)hConverter)->mpSubsFontName;
        return String::CreateFromAscii( pName );
    }
    else
        return String();
}
