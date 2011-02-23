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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"


#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <editeng/scripttypeitem.hxx>
#include "format.hxx"

/////////////////////////////////////////////////////////////////

// Latin default-fonts
static const sal_uInt16 aLatinDefFnts[FNT_END] =
{
    DEFAULTFONT_SERIF,  // FNT_VARIABLE
    DEFAULTFONT_SERIF,  // FNT_FUNCTION
    DEFAULTFONT_SERIF,  // FNT_NUMBER
    DEFAULTFONT_SERIF,  // FNT_TEXT
    DEFAULTFONT_SERIF,  // FNT_SERIF
    DEFAULTFONT_SANS,   // FNT_SANS
    DEFAULTFONT_FIXED   // FNT_FIXED
    //OpenSymbol,    // FNT_MATH
};

// CJK default-fonts
//! we use non-asian fonts for variables, functions and numbers since they
//! look better and even in asia only latin letters will be used for those.
//! At least that's what I was told...
static const sal_uInt16 aCJKDefFnts[FNT_END] =
{
    DEFAULTFONT_SERIF,          // FNT_VARIABLE
    DEFAULTFONT_SERIF,          // FNT_FUNCTION
    DEFAULTFONT_SERIF,          // FNT_NUMBER
    DEFAULTFONT_CJK_TEXT,       // FNT_TEXT
    DEFAULTFONT_CJK_TEXT,       // FNT_SERIF
    DEFAULTFONT_CJK_DISPLAY,    // FNT_SANS
    DEFAULTFONT_CJK_TEXT        // FNT_FIXED
    //OpenSymbol,    // FNT_MATH
};

// CTL default-fonts
static const sal_uInt16 aCTLDefFnts[FNT_END] =
{
    DEFAULTFONT_CTL_TEXT,    // FNT_VARIABLE
    DEFAULTFONT_CTL_TEXT,    // FNT_FUNCTION
    DEFAULTFONT_CTL_TEXT,    // FNT_NUMBER
    DEFAULTFONT_CTL_TEXT,    // FNT_TEXT
    DEFAULTFONT_CTL_TEXT,    // FNT_SERIF
    DEFAULTFONT_CTL_TEXT,    // FNT_SANS
    DEFAULTFONT_CTL_TEXT     // FNT_FIXED
    //OpenSymbol,    // FNT_MATH
};


String GetDefaultFontName( LanguageType nLang, sal_uInt16 nIdent )
{
    DBG_ASSERT( /*FNT_BEGIN <= nIdent  &&*/  nIdent <= FNT_END,
            "index out opd range" );

    if (FNT_MATH == nIdent)
        return String::CreateFromAscii( FNTNAME_MATH );
    else
    {
        const sal_uInt16 *pTable;
        switch ( SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ) )
        {
            case SCRIPTTYPE_LATIN :     pTable = aLatinDefFnts; break;
            case SCRIPTTYPE_ASIAN :     pTable = aCJKDefFnts; break;
            case SCRIPTTYPE_COMPLEX :   pTable = aCTLDefFnts; break;
            default :
                pTable = aLatinDefFnts;
                DBG_ERROR( "unknown script-type" );
        }

        return Application::GetDefaultDevice()->GetDefaultFont(
                        pTable[ nIdent ], nLang,
                        DEFAULTFONT_FLAGS_ONLYONE ).GetName();
    }
}

/////////////////////////////////////////////////////////////////

SmFormat::SmFormat()
:   aBaseSize(0, SmPtsTo100th_mm(12))
{
    nVersion    = SM_FMT_VERSION_NOW;

    eHorAlign       = AlignCenter;
    nGreekCharStyle = 0;
    bIsTextmode     = bScaleNormalBrackets = sal_False;

    vSize[SIZ_TEXT]     = 100;
    vSize[SIZ_INDEX]    = 60;
    vSize[SIZ_FUNCTION] =
    vSize[SIZ_OPERATOR] = 100;
    vSize[SIZ_LIMITS]   = 60;

    vDist[DIS_HORIZONTAL]           = 10;
    vDist[DIS_VERTICAL]             = 5;
    vDist[DIS_ROOT]                 = 0;
    vDist[DIS_SUPERSCRIPT]          =
    vDist[DIS_SUBSCRIPT]            = 20;
    vDist[DIS_NUMERATOR]            =
    vDist[DIS_DENOMINATOR]          = 0;
    vDist[DIS_FRACTION]             = 10;
    vDist[DIS_STROKEWIDTH]          = 5;
    vDist[DIS_UPPERLIMIT]           =
    vDist[DIS_LOWERLIMIT]           = 0;
    vDist[DIS_BRACKETSIZE]          =
    vDist[DIS_BRACKETSPACE]         = 5;
    vDist[DIS_MATRIXROW]            = 3;
    vDist[DIS_MATRIXCOL]            = 30;
    vDist[DIS_ORNAMENTSIZE]         =
    vDist[DIS_ORNAMENTSPACE]        = 0;
    vDist[DIS_OPERATORSIZE]         = 50;
    vDist[DIS_OPERATORSPACE]        = 20;
    vDist[DIS_LEFTSPACE]            =
    vDist[DIS_RIGHTSPACE]           = 100;
    vDist[DIS_TOPSPACE]             =
    vDist[DIS_BOTTOMSPACE]          =
    vDist[DIS_NORMALBRACKETSIZE]    = 0;

    vFont[FNT_VARIABLE] =
    vFont[FNT_FUNCTION] =
    vFont[FNT_NUMBER]   =
    vFont[FNT_TEXT]     =
    vFont[FNT_SERIF]    = SmFace(C2S(FNTNAME_TIMES), aBaseSize);
    vFont[FNT_SANS]     = SmFace(C2S(FNTNAME_HELV),  aBaseSize);
    vFont[FNT_FIXED]    = SmFace(C2S(FNTNAME_COUR),  aBaseSize);
    vFont[FNT_MATH]     = SmFace(C2S(FNTNAME_MATH),  aBaseSize);

    vFont[FNT_MATH].SetCharSet( RTL_TEXTENCODING_UNICODE );

    vFont[FNT_VARIABLE].SetItalic(ITALIC_NORMAL);
    vFont[FNT_FUNCTION].SetItalic(ITALIC_NONE);
    vFont[FNT_NUMBER]  .SetItalic(ITALIC_NONE);
    vFont[FNT_TEXT]    .SetItalic(ITALIC_NONE);
    vFont[FNT_SERIF]   .SetItalic(ITALIC_NONE);
    vFont[FNT_SANS]    .SetItalic(ITALIC_NONE);
    vFont[FNT_FIXED]   .SetItalic(ITALIC_NONE);

    for ( sal_uInt16 i = FNT_BEGIN;  i <= FNT_END;  i++ )
    {
        SmFace &rFace = vFont[i];
        rFace.SetTransparent( sal_True );
        rFace.SetAlign( ALIGN_BASELINE );
        rFace.SetColor( COL_AUTO );
        bDefaultFont[i] = sal_False;
    }
}


void SmFormat::SetFont(sal_uInt16 nIdent, const SmFace &rFont, sal_Bool bDefault )
{
    vFont[nIdent] = rFont;
    vFont[nIdent].SetTransparent( sal_True );
    vFont[nIdent].SetAlign( ALIGN_BASELINE );

    bDefaultFont[nIdent] = bDefault;
}

SmFormat & SmFormat::operator = (const SmFormat &rFormat)
{
    SetBaseSize(rFormat.GetBaseSize());
    SetVersion (rFormat.GetVersion());
    SetHorAlign(rFormat.GetHorAlign());
    SetTextmode(rFormat.IsTextmode());
    SetGreekCharStyle(rFormat.GetGreekCharStyle());
    SetScaleNormalBrackets(rFormat.IsScaleNormalBrackets());

    sal_uInt16  i;
    for (i = FNT_BEGIN;  i <= FNT_END;  i++)
    {
        SetFont(i, rFormat.GetFont(i));
        SetDefaultFont(i, rFormat.IsDefaultFont(i));
    }
    for (i = SIZ_BEGIN;  i <= SIZ_END;  i++)
        SetRelSize(i, rFormat.GetRelSize(i));
    for (i = DIS_BEGIN;  i <= DIS_END;  i++)
        SetDistance(i, rFormat.GetDistance(i));

    return *this;
}


sal_Bool SmFormat::operator == (const SmFormat &rFormat) const
{
    sal_Bool bRes = aBaseSize == rFormat.aBaseSize  &&
                eHorAlign == rFormat.eHorAlign  &&
                nGreekCharStyle == rFormat.nGreekCharStyle &&
                bIsTextmode == rFormat.bIsTextmode  &&
                bScaleNormalBrackets  == rFormat.bScaleNormalBrackets;

    sal_uInt16 i;
    for (i = 0;  i <= SIZ_END && bRes;  ++i)
    {
        if (vSize[i] != rFormat.vSize[i])
            bRes = sal_False;
    }
    for (i = 0;  i <= DIS_END && bRes;  ++i)
    {
        if (vDist[i] != rFormat.vDist[i])
            bRes = sal_False;
    }
    for (i = 0;  i <= FNT_END && bRes;  ++i)
    {
        if (vFont[i] != rFormat.vFont[i]  ||
            bDefaultFont[i] != rFormat.bDefaultFont[i])
            bRes = sal_False;
    }

    return bRes;
}


