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
#include "precompiled_sc.hxx"




// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>


#include <editeng/adjitem.hxx>
#include <svx/algitem.hxx>
#include <editeng/brshitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/langitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/unolingu.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <vcl/pdfextoutdevdata.hxx>

#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif

#include "output.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "editutil.hxx"
#include "progress.hxx"
#include "scmod.hxx"
#include "fillinfo.hxx"

#include <math.h>

//! Autofilter-Breite mit column.cxx zusammenfassen
#define DROPDOWN_BITMAP_SIZE        18

#define DRAWTEXT_MAX    32767

const sal_uInt16 SC_SHRINKAGAIN_MAX = 7;

// STATIC DATA -----------------------------------------------------------


// -----------------------------------------------------------------------

class ScDrawStringsVars
{
    ScOutputData*       pOutput;                // Verbindung

    const ScPatternAttr* pPattern;              // Attribute
    const SfxItemSet*   pCondSet;               // aus bedingter Formatierung

    Font                aFont;                  // aus Attributen erzeugt
    FontMetric          aMetric;
    long                nAscentPixel;           // always pixels
    SvxCellOrientation  eAttrOrient;
    SvxCellHorJustify   eAttrHorJust;
    SvxCellVerJustify   eAttrVerJust;
    const SvxMarginItem* pMargin;
    sal_uInt16              nIndent;
    sal_Bool                bRotated;

    String              aString;                // Inhalte
    Size                aTextSize;
    long                nOriginalWidth;
    long                nMaxDigitWidth;
    long                nSignWidth;
    long                nDotWidth;
    long                nExpWidth;

    ScBaseCell*         pLastCell;
    sal_uLong               nValueFormat;
    sal_Bool                bLineBreak;
    sal_Bool                bRepeat;
    sal_Bool                bShrink;

    sal_Bool                bPixelToLogic;
    sal_Bool                bCellContrast;

    Color               aBackConfigColor;       // used for ScPatternAttr::GetFont calls
    Color               aTextConfigColor;

public:
                ScDrawStringsVars(ScOutputData* pData, sal_Bool bPTL);
                ~ScDrawStringsVars();

                //  SetPattern = ex-SetVars
                //  SetPatternSimple: ohne Font

    void        SetPattern( const ScPatternAttr* pNew, const SfxItemSet* pSet, ScBaseCell* pCell, sal_uInt8 nScript );
    void        SetPatternSimple( const ScPatternAttr* pNew, const SfxItemSet* pSet );

    sal_Bool        SetText( ScBaseCell* pCell );   // sal_True -> pOldPattern vergessen
    void        SetTextToWidthOrHash( ScBaseCell* pCell, long nWidth );
    void        SetAutoText( const String& rAutoText );

    const ScPatternAttr*    GetPattern() const      { return pPattern; }
    SvxCellOrientation      GetOrient() const       { return eAttrOrient; }
    SvxCellHorJustify       GetHorJust() const      { return eAttrHorJust; }
    SvxCellVerJustify       GetVerJust() const      { return eAttrVerJust; }
    const SvxMarginItem*    GetMargin() const       { return pMargin; }

    sal_uInt16  GetLeftTotal() const        { return pMargin->GetLeftMargin() + nIndent; }

    const String&           GetString() const       { return aString; }
    const Size&             GetTextSize() const     { return aTextSize; }
    long                    GetOriginalWidth() const { return nOriginalWidth; }

    sal_uLong   GetValueFormat() const                  { return nValueFormat; }
    sal_Bool    GetLineBreak() const                    { return bLineBreak; }
    sal_Bool    IsRepeat() const                        { return bRepeat; }
    sal_Bool    IsShrink() const                        { return bShrink; }

    long    GetAscent() const   { return nAscentPixel; }
    sal_Bool    IsRotated() const   { return bRotated; }

    void    SetShrinkScale( long nScale, sal_uInt8 nScript );

    sal_Bool    HasCondHeight() const   { return pCondSet && SFX_ITEM_SET ==
                                        pCondSet->GetItemState( ATTR_FONT_HEIGHT, sal_True ); }

    sal_Bool    HasEditCharacters() const;

private:
    void        SetHashText();
    long        GetMaxDigitWidth();     // in logic units
    long        GetSignWidth();
    long        GetDotWidth();
    long        GetExpWidth();
    void        TextChanged();
};

//==================================================================

ScDrawStringsVars::ScDrawStringsVars(ScOutputData* pData, sal_Bool bPTL) :
    pOutput     ( pData ),
    pPattern    ( NULL ),
    pCondSet    ( NULL ),
    eAttrOrient ( SVX_ORIENTATION_STANDARD ),
    eAttrHorJust( SVX_HOR_JUSTIFY_STANDARD ),
    eAttrVerJust( SVX_VER_JUSTIFY_BOTTOM ),
    pMargin     ( NULL ),
    nIndent     ( 0 ),
    bRotated    ( sal_False ),
    nOriginalWidth( 0 ),
    nMaxDigitWidth( 0 ),
    nSignWidth( 0 ),
    nDotWidth( 0 ),
    nExpWidth( 0 ),
    pLastCell   ( NULL ),
    nValueFormat( 0 ),
    bLineBreak  ( sal_False ),
    bRepeat     ( sal_False ),
    bShrink     ( sal_False ),
    bPixelToLogic( bPTL )
{
    ScModule* pScMod = SC_MOD();
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed sal_True)
    bCellContrast = pOutput->bUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    const svtools::ColorConfig& rColorConfig = pScMod->GetColorConfig();
    aBackConfigColor.SetColor( rColorConfig.GetColorValue(svtools::DOCCOLOR).nColor );
    aTextConfigColor.SetColor( rColorConfig.GetColorValue(svtools::FONTCOLOR).nColor );
}

ScDrawStringsVars::~ScDrawStringsVars()
{
}

void ScDrawStringsVars::SetShrinkScale( long nScale, sal_uInt8 nScript )
{
    // text remains valid, size is updated

    OutputDevice* pDev = pOutput->pDev;
    OutputDevice* pRefDevice = pOutput->pRefDevice;
    OutputDevice* pFmtDevice = pOutput->pFmtDevice;

    // call GetFont with a modified fraction, use only the height

    Fraction aFraction( nScale, 100 );
    if ( !bPixelToLogic )
        aFraction *= pOutput->aZoomY;
    Font aTmpFont;
    pPattern->GetFont( aTmpFont, SC_AUTOCOL_RAW, pFmtDevice, &aFraction, pCondSet, nScript );
    long nNewHeight = aTmpFont.GetHeight();
    if ( nNewHeight > 0 )
        aFont.SetHeight( nNewHeight );

    // set font and dependent variables as in SetPattern

    pDev->SetFont( aFont );
    if ( pFmtDevice != pDev )
        pFmtDevice->SetFont( aFont );

    aMetric = pFmtDevice->GetFontMetric();
    if ( pFmtDevice->GetOutDevType() == OUTDEV_PRINTER && aMetric.GetIntLeading() == 0 )
    {
        OutputDevice* pDefaultDev = Application::GetDefaultDevice();
        MapMode aOld = pDefaultDev->GetMapMode();
        pDefaultDev->SetMapMode( pFmtDevice->GetMapMode() );
        aMetric = pDefaultDev->GetFontMetric( aFont );
        pDefaultDev->SetMapMode( aOld );
    }

    nAscentPixel = aMetric.GetAscent();
    if ( bPixelToLogic )
        nAscentPixel = pRefDevice->LogicToPixel( Size( 0, nAscentPixel ) ).Height();

    SetAutoText( aString );     // same text again, to get text size
}

void ScDrawStringsVars::SetPattern( const ScPatternAttr* pNew, const SfxItemSet* pSet,
                                    ScBaseCell* pCell, sal_uInt8 nScript )
{
    nMaxDigitWidth = 0;
    nSignWidth     = 0;
    nDotWidth      = 0;
    nExpWidth      = 0;

    pPattern = pNew;
    pCondSet = pSet;

    //  pPattern auswerten

    OutputDevice* pDev = pOutput->pDev;
    OutputDevice* pRefDevice = pOutput->pRefDevice;
    OutputDevice* pFmtDevice = pOutput->pFmtDevice;

    //  Font

    ScAutoFontColorMode eColorMode;
    if ( pOutput->bUseStyleColor )
    {
        if ( pOutput->bForceAutoColor )
            eColorMode = bCellContrast ? SC_AUTOCOL_IGNOREALL : SC_AUTOCOL_IGNOREFONT;
        else
            eColorMode = bCellContrast ? SC_AUTOCOL_IGNOREBACK : SC_AUTOCOL_DISPLAY;
    }
    else
        eColorMode = SC_AUTOCOL_PRINT;

    if ( bPixelToLogic )
        pPattern->GetFont( aFont, eColorMode, pFmtDevice, NULL, pCondSet, nScript,
                            &aBackConfigColor, &aTextConfigColor );
    else
        pPattern->GetFont( aFont, eColorMode, pFmtDevice, &pOutput->aZoomY, pCondSet, nScript,
                            &aBackConfigColor, &aTextConfigColor );
    aFont.SetAlign(ALIGN_BASELINE);

    //  Orientierung

    eAttrOrient = pPattern->GetCellOrientation( pCondSet );

    //  alignment

    eAttrHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->GetItem( ATTR_HOR_JUSTIFY, pCondSet )).GetValue();

    eAttrVerJust = (SvxCellVerJustify)((const SvxVerJustifyItem&)pPattern->GetItem( ATTR_VER_JUSTIFY, pCondSet )).GetValue();
    if ( eAttrVerJust == SVX_VER_JUSTIFY_STANDARD )
        eAttrVerJust = SVX_VER_JUSTIFY_BOTTOM;

    //  line break

    bLineBreak = ((const SfxBoolItem&)pPattern->GetItem( ATTR_LINEBREAK, pCondSet )).GetValue();

    //  handle "repeat" alignment

    bRepeat = ( eAttrHorJust == SVX_HOR_JUSTIFY_REPEAT );
    if ( bRepeat )
    {
        // "repeat" disables rotation (before constructing the font)
        eAttrOrient = SVX_ORIENTATION_STANDARD;

        // #i31843# "repeat" with "line breaks" is treated as default alignment (but rotation is still disabled)
        if ( bLineBreak )
            eAttrHorJust = SVX_HOR_JUSTIFY_STANDARD;
    }

    short nRot;
    switch (eAttrOrient)
    {
        case SVX_ORIENTATION_STANDARD:
            nRot = 0;
            bRotated = (((const SfxInt32Item&)pPattern->GetItem( ATTR_ROTATE_VALUE, pCondSet )).GetValue() != 0) &&
                       !bRepeat;
            break;
        case SVX_ORIENTATION_STACKED:
            nRot = 0;
            bRotated = sal_False;
            break;
        case SVX_ORIENTATION_TOPBOTTOM:
            nRot = 2700;
            bRotated = sal_False;
            break;
        case SVX_ORIENTATION_BOTTOMTOP:
            nRot = 900;
            bRotated = sal_False;
            break;
        default:
            DBG_ERROR("Falscher SvxCellOrientation Wert");
            nRot = 0;
            bRotated = sal_False;
            break;
    }
    aFont.SetOrientation( nRot );

    //  Syntax-Modus

    if (pOutput->bSyntaxMode)
        pOutput->SetSyntaxColor( &aFont, pCell );

    pDev->SetFont( aFont );
    if ( pFmtDevice != pDev )
        pFmtDevice->SetFont( aFont );

    aMetric = pFmtDevice->GetFontMetric();

    //
    //  Wenn auf dem Drucker das Leading 0 ist, gibt es Probleme
    //  -> Metric vom Bildschirm nehmen (wie EditEngine!)
    //

    if ( pFmtDevice->GetOutDevType() == OUTDEV_PRINTER && aMetric.GetIntLeading() == 0 )
    {
        OutputDevice* pDefaultDev = Application::GetDefaultDevice();
        MapMode aOld = pDefaultDev->GetMapMode();
        pDefaultDev->SetMapMode( pFmtDevice->GetMapMode() );
        aMetric = pDefaultDev->GetFontMetric( aFont );
        pDefaultDev->SetMapMode( aOld );
    }

    nAscentPixel = aMetric.GetAscent();
    if ( bPixelToLogic )
        nAscentPixel = pRefDevice->LogicToPixel( Size( 0, nAscentPixel ) ).Height();

    Color aULineColor( ((const SvxUnderlineItem&)pPattern->GetItem( ATTR_FONT_UNDERLINE, pCondSet )).GetColor() );
    pDev->SetTextLineColor( aULineColor );

    Color aOLineColor( ((const SvxOverlineItem&)pPattern->GetItem( ATTR_FONT_OVERLINE, pCondSet )).GetColor() );
    pDev->SetOverlineColor( aOLineColor );

    //  Zahlenformat

//    sal_uLong nOld = nValueFormat;
    nValueFormat = pPattern->GetNumberFormat( pOutput->pDoc->GetFormatTable(), pCondSet );

/*  s.u.
    if (nValueFormat != nOld)
        pLastCell = NULL;           // immer neu formatieren
*/
    //  Raender

    pMargin = (const SvxMarginItem*)&pPattern->GetItem( ATTR_MARGIN, pCondSet );
    if ( eAttrHorJust == SVX_HOR_JUSTIFY_LEFT )
        nIndent = ((const SfxUInt16Item&)pPattern->GetItem( ATTR_INDENT, pCondSet )).GetValue();
    else
        nIndent = 0;

    //  "Shrink to fit"

    bShrink = static_cast<const SfxBoolItem&>(pPattern->GetItem( ATTR_SHRINKTOFIT, pCondSet )).GetValue();

    //  zumindest die Text-Groesse muss neu geholt werden
    //! unterscheiden, und den Text nicht neu vom Numberformatter holen?

    pLastCell = NULL;
}

void ScDrawStringsVars::SetPatternSimple( const ScPatternAttr* pNew, const SfxItemSet* pSet )
{
    nMaxDigitWidth = 0;
    nSignWidth     = 0;
    nDotWidth      = 0;
    nExpWidth      = 0;
    //  wird gerufen, wenn sich die Font-Variablen nicht aendern (!StringDiffer)

    pPattern = pNew;
    pCondSet = pSet;        //! noetig ???

    //  Zahlenformat

    sal_uLong nOld = nValueFormat;
//  nValueFormat = pPattern->GetNumberFormat( pFormatter );
    const SfxPoolItem* pFormItem;
    if ( !pCondSet || pCondSet->GetItemState(ATTR_VALUE_FORMAT,sal_True,&pFormItem) != SFX_ITEM_SET )
        pFormItem = &pPattern->GetItem(ATTR_VALUE_FORMAT);
    const SfxPoolItem* pLangItem;
    if ( !pCondSet || pCondSet->GetItemState(ATTR_LANGUAGE_FORMAT,sal_True,&pLangItem) != SFX_ITEM_SET )
        pLangItem = &pPattern->GetItem(ATTR_LANGUAGE_FORMAT);
    nValueFormat = pOutput->pDoc->GetFormatTable()->GetFormatForLanguageIfBuiltIn(
                    ((SfxUInt32Item*)pFormItem)->GetValue(),
                    ((SvxLanguageItem*)pLangItem)->GetLanguage() );

    if (nValueFormat != nOld)
        pLastCell = NULL;           // immer neu formatieren

    //  Raender

    pMargin = (const SvxMarginItem*)&pPattern->GetItem( ATTR_MARGIN, pCondSet );

    if ( eAttrHorJust == SVX_HOR_JUSTIFY_LEFT )
        nIndent = ((const SfxUInt16Item&)pPattern->GetItem( ATTR_INDENT, pCondSet )).GetValue();
    else
        nIndent = 0;

    //  "Shrink to fit"

    bShrink = static_cast<const SfxBoolItem&>(pPattern->GetItem( ATTR_SHRINKTOFIT, pCondSet )).GetValue();
}

inline sal_Bool SameValue( ScBaseCell* pCell, ScBaseCell* pOldCell )    // pCell ist != 0
{
    return pOldCell && pOldCell->GetCellType() == CELLTYPE_VALUE &&
            pCell->GetCellType() == CELLTYPE_VALUE &&
            ((ScValueCell*)pCell)->GetValue() == ((ScValueCell*)pOldCell)->GetValue();
}

sal_Bool ScDrawStringsVars::SetText( ScBaseCell* pCell )
{
    sal_Bool bChanged = sal_False;

    if (pCell)
    {
        if ( !SameValue( pCell, pLastCell ) )
        {
            pLastCell = pCell;          //  Zelle merken

            Color* pColor;
            sal_uLong nFormat = GetValueFormat();
            ScCellFormat::GetString( pCell,
                                     nFormat, aString, &pColor,
                                     *pOutput->pDoc->GetFormatTable(),
                                     pOutput->bShowNullValues,
                                     pOutput->bShowFormulas,
                                     ftCheck );

            if (aString.Len() > DRAWTEXT_MAX)
                aString.Erase(DRAWTEXT_MAX);

            if ( pColor && !pOutput->bSyntaxMode && !( pOutput->bUseStyleColor && pOutput->bForceAutoColor ) )
            {
                OutputDevice* pDev = pOutput->pDev;
                aFont.SetColor(*pColor);
                pDev->SetFont( aFont ); // nur fuer Ausgabe
                bChanged = sal_True;
                pLastCell = NULL;       // naechstes Mal wieder hierherkommen
            }

            TextChanged();
        }
        //  sonst String/Groesse behalten
    }
    else
    {
        aString.Erase();
        pLastCell = NULL;
        aTextSize = Size(0,0);
        nOriginalWidth = 0;
    }

    return bChanged;
}

void ScDrawStringsVars::SetHashText()
{
    SetAutoText( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("###")) );
}

void ScDrawStringsVars::SetTextToWidthOrHash( ScBaseCell* pCell, long nWidth )
{
    // #i113045# do the single-character width calculations in logic units
    if (bPixelToLogic)
        nWidth = pOutput->pRefDevice->PixelToLogic(Size(nWidth,0)).Width();

    if (!pCell)
        return;

    CellType eType = pCell->GetCellType();
    if (eType != CELLTYPE_VALUE && eType != CELLTYPE_FORMULA)
        // must be a value or formula cell.
        return;

    if (eType == CELLTYPE_FORMULA)
    {
        ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
        if (pFCell->GetErrCode() != 0)
        {
            SetHashText();      // If the error string doesn't fit, always use "###"
            return;
        }
        // If it's formula, the result must be a value.
        if (!pFCell->IsValue())
            return;
    }

    sal_uLong nFormat = GetValueFormat();
    if ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
    {
        // Not 'General' number format.  Set hash text and bail out.
        SetHashText();
        return;
    }

    double fVal = (eType == CELLTYPE_VALUE) ?
        static_cast<ScValueCell*>(pCell)->GetValue() : static_cast<ScFormulaCell*>(pCell)->GetValue();

    const SvNumberformat* pNumFormat = pOutput->pDoc->GetFormatTable()->GetEntry(nFormat);
    if (!pNumFormat)
        return;

    long nMaxDigit = GetMaxDigitWidth();
    sal_uInt16 nNumDigits = static_cast<sal_uInt16>(nWidth / nMaxDigit);

    if (!pNumFormat->GetOutputString(fVal, nNumDigits, aString))
        // Failed to get output string.  Bail out.
        return;

    sal_uInt8 nSignCount = 0, nDecimalCount = 0, nExpCount = 0;
    xub_StrLen nLen = aString.Len();
    sal_Unicode cDecSep = ScGlobal::GetpLocaleData()->getLocaleItem().decimalSeparator.getStr()[0];
    for (xub_StrLen i = 0; i < nLen; ++i)
    {
        sal_Unicode c = aString.GetChar(i);
        if (c == sal_Unicode('-'))
            ++nSignCount;
        else if (c == cDecSep)
            ++nDecimalCount;
        else if (c == sal_Unicode('E'))
            ++nExpCount;
    }

    // #i112250# A small value might be formatted as "0" when only counting the digits,
    // but fit into the column when considering the smaller width of the decimal separator.
    if (aString.EqualsAscii("0") && fVal != 0.0)
        nDecimalCount = 1;

    if (nDecimalCount)
        nWidth += (nMaxDigit - GetDotWidth()) * nDecimalCount;
    if (nSignCount)
        nWidth += (nMaxDigit - GetSignWidth()) * nSignCount;
    if (nExpCount)
        nWidth += (nMaxDigit - GetExpWidth()) * nExpCount;

    if (nDecimalCount || nSignCount || nExpCount)
    {
        // Re-calculate.
        nNumDigits = static_cast<sal_uInt16>(nWidth / nMaxDigit);
        if (!pNumFormat->GetOutputString(fVal, nNumDigits, aString))
            // Failed to get output string.  Bail out.
            return;
    }

    long nActualTextWidth = pOutput->pFmtDevice->GetTextWidth(aString);
    if (nActualTextWidth > nWidth)
    {
        // Even after the decimal adjustment the text doesn't fit.  Give up.
        SetHashText();
        return;
    }

    TextChanged();
    pLastCell = NULL;   // #i113022# equal cell and format in another column may give different string
}

void ScDrawStringsVars::SetAutoText( const String& rAutoText )
{
    aString = rAutoText;

    OutputDevice* pRefDevice = pOutput->pRefDevice;
    OutputDevice* pFmtDevice = pOutput->pFmtDevice;
    aTextSize.Width() = pFmtDevice->GetTextWidth( aString );
    aTextSize.Height() = pFmtDevice->GetTextHeight();

    if ( !pRefDevice->GetConnectMetaFile() || pRefDevice->GetOutDevType() == OUTDEV_PRINTER )
    {
        double fMul = pOutput->GetStretch();
        aTextSize.Width() = (long)(aTextSize.Width() / fMul + 0.5);
    }

    aTextSize.Height() = aMetric.GetAscent() + aMetric.GetDescent();
    if ( GetOrient() != SVX_ORIENTATION_STANDARD )
    {
        long nTemp = aTextSize.Height();
        aTextSize.Height() = aTextSize.Width();
        aTextSize.Width() = nTemp;
    }

    nOriginalWidth = aTextSize.Width();
    if ( bPixelToLogic )
        aTextSize = pRefDevice->LogicToPixel( aTextSize );

    pLastCell = NULL;       // derselbe Text kann in der naechsten Zelle wieder passen
}

long ScDrawStringsVars::GetMaxDigitWidth()
{
    if (nMaxDigitWidth > 0)
        return nMaxDigitWidth;

    sal_Char cZero = '0';
    for (sal_Char i = 0; i < 10; ++i)
    {
        sal_Char cDigit = cZero + i;
        long n = pOutput->pFmtDevice->GetTextWidth(String(cDigit));
        nMaxDigitWidth = ::std::max(nMaxDigitWidth, n);
    }
    return nMaxDigitWidth;
}

long ScDrawStringsVars::GetSignWidth()
{
    if (nSignWidth > 0)
        return nSignWidth;

    nSignWidth = pOutput->pFmtDevice->GetTextWidth(String('-'));
    return nSignWidth;
}

long ScDrawStringsVars::GetDotWidth()
{
    if (nDotWidth > 0)
        return nDotWidth;

    const ::rtl::OUString& sep = ScGlobal::GetpLocaleData()->getLocaleItem().decimalSeparator;
    nDotWidth = pOutput->pFmtDevice->GetTextWidth(sep);
    return nDotWidth;
}

long ScDrawStringsVars::GetExpWidth()
{
    if (nExpWidth > 0)
        return nExpWidth;

    nExpWidth = pOutput->pFmtDevice->GetTextWidth(String('E'));
    return nExpWidth;
}

void ScDrawStringsVars::TextChanged()
{
    OutputDevice* pRefDevice = pOutput->pRefDevice;
    OutputDevice* pFmtDevice = pOutput->pFmtDevice;
    aTextSize.Width() = pFmtDevice->GetTextWidth( aString );
    aTextSize.Height() = pFmtDevice->GetTextHeight();

    if ( !pRefDevice->GetConnectMetaFile() || pRefDevice->GetOutDevType() == OUTDEV_PRINTER )
    {
        double fMul = pOutput->GetStretch();
        aTextSize.Width() = (long)(aTextSize.Width() / fMul + 0.5);
    }

    aTextSize.Height() = aMetric.GetAscent() + aMetric.GetDescent();
    if ( GetOrient() != SVX_ORIENTATION_STANDARD )
    {
        long nTemp = aTextSize.Height();
        aTextSize.Height() = aTextSize.Width();
        aTextSize.Width() = nTemp;
    }

    nOriginalWidth = aTextSize.Width();
    if ( bPixelToLogic )
        aTextSize = pRefDevice->LogicToPixel( aTextSize );
}

sal_Bool ScDrawStringsVars::HasEditCharacters() const
{
    static const sal_Unicode pChars[] =
    {
        CHAR_NBSP, CHAR_SHY, CHAR_ZWSP, CHAR_LRM, CHAR_RLM, CHAR_NBHY, CHAR_ZWNBSP, 0
    };
    return aString.SearchChar( pChars ) != STRING_NOTFOUND;
}

//==================================================================

double ScOutputData::GetStretch()
{
    if ( pRefDevice->IsMapMode() )
    {
        //  #95920# If a non-trivial MapMode is set, its scale is now already
        //  taken into account in the OutputDevice's font handling
        //  (OutputDevice::ImplNewFont, see #95414#).
        //  The old handling below is only needed for pixel output.
        return 1.0;
    }

    // calculation in double is faster than Fraction multiplication
    // and doesn't overflow

    if ( pRefDevice == pFmtDevice )
    {
        MapMode aOld = pRefDevice->GetMapMode();
        return ((double)aOld.GetScaleY()) / ((double)aOld.GetScaleX()) * ((double)aZoomY) / ((double)aZoomX);
    }
    else
    {
        // when formatting for printer, device map mode has already been taken care of
        return ((double)aZoomY) / ((double)aZoomX);
    }
}

//==================================================================

//
//  output strings
//

void lcl_DoHyperlinkResult( OutputDevice* pDev, const Rectangle& rRect, ScBaseCell* pCell )
{
    vcl::PDFExtOutDevData* pPDFData = PTR_CAST( vcl::PDFExtOutDevData, pDev->GetExtOutDevData() );

    String aCellText;
    String aURL;
    if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
    {
        ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
        if ( pFCell->IsHyperLinkCell() )
            pFCell->GetURLResult( aURL, aCellText );
    }

    if ( aURL.Len() && pPDFData )
    {
        vcl::PDFExtOutDevBookmarkEntry aBookmark;
        aBookmark.nLinkId = pPDFData->CreateLink( rRect );
        aBookmark.aBookmark = aURL;
        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFData->GetBookmarks();
        rBookmarks.push_back( aBookmark );
    }
}

void ScOutputData::SetSyntaxColor( Font* pFont, ScBaseCell* pCell )
{
    if (pCell)
    {
        switch (pCell->GetCellType())
        {
            case CELLTYPE_VALUE:
                pFont->SetColor( *pValueColor );
                break;
            case CELLTYPE_STRING:
                pFont->SetColor( *pTextColor );
                break;
            case CELLTYPE_FORMULA:
                pFont->SetColor( *pFormulaColor );
                break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
}

void lcl_SetEditColor( EditEngine& rEngine, const Color& rColor )
{
    ESelection aSel( 0, 0, rEngine.GetParagraphCount(), 0 );
    SfxItemSet aSet( rEngine.GetEmptyItemSet() );
    aSet.Put( SvxColorItem( rColor, EE_CHAR_COLOR ) );
    rEngine.QuickSetAttribs( aSet, aSel );
    // function is called with update mode set to FALSE
}

void ScOutputData::SetEditSyntaxColor( EditEngine& rEngine, ScBaseCell* pCell )
{
    if (pCell)
    {
        Color aColor;
        switch (pCell->GetCellType())
        {
            case CELLTYPE_VALUE:
                aColor = *pValueColor;
                break;
            case CELLTYPE_STRING:
                aColor = *pTextColor;
                break;
            case CELLTYPE_FORMULA:
                aColor = *pFormulaColor;
                break;
            default:
            {
                // added to avoid warnings
            }
        }
        lcl_SetEditColor( rEngine, aColor );
    }
}

sal_Bool ScOutputData::GetMergeOrigin( SCCOL nX, SCROW nY, SCSIZE nArrY,
                                    SCCOL& rOverX, SCROW& rOverY,
                                    sal_Bool bVisRowChanged )
{
    sal_Bool bDoMerge = sal_False;
    sal_Bool bIsLeft = ( nX == nVisX1 );
    sal_Bool bIsTop  = ( nY == nVisY1 ) || bVisRowChanged;

    CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nX+1];
    if ( pInfo->bHOverlapped && pInfo->bVOverlapped )
        bDoMerge = bIsLeft && bIsTop;
    else if ( pInfo->bHOverlapped )
        bDoMerge = bIsLeft;
    else if ( pInfo->bVOverlapped )
        bDoMerge = bIsTop;

                                    // weiter solange versteckt
/*  if (!bDoMerge)
        return sal_False;
*/

    rOverX = nX;
    rOverY = nY;
    sal_Bool bHOver = pInfo->bHOverlapped;
    sal_Bool bVOver = pInfo->bVOverlapped;
    sal_Bool bHidden;

    while (bHOver)              // nY konstant
    {
        --rOverX;
        bHidden = pDoc->ColHidden(rOverX, nTab);
        if ( !bDoMerge && !bHidden )
            return sal_False;

        if (rOverX >= nX1 && !bHidden)
        {
//          rVirtPosX -= pRowInfo[0].pCellInfo[rOverX+1].nWidth;
            bHOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bHOverlapped;
            bVOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bVOverlapped;
        }
        else
        {
//          if (!bClipVirt)
//              rVirtPosX -= (long) (pDoc->GetColWidth( rOverX, nTab ) * nPPTX);
            sal_uInt16 nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                rOverX, rOverY, nTab, ATTR_MERGE_FLAG ))->GetValue();
            bHOver = ((nOverlap & SC_MF_HOR) != 0);
            bVOver = ((nOverlap & SC_MF_VER) != 0);
        }
    }

    while (bVOver)
    {
        --rOverY;
        bHidden = pDoc->RowHidden(rOverY, nTab);
        if ( !bDoMerge && !bHidden )
            return sal_False;

        if (nArrY>0)
            --nArrY;                        // lokale Kopie !

        if (rOverX >= nX1 && rOverY >= nY1 &&
            !pDoc->ColHidden(rOverX, nTab) &&
            !pDoc->RowHidden(rOverY, nTab) &&
            pRowInfo[nArrY].nRowNo == rOverY)
        {
//          rVirtPosY -= pRowInfo[nArrY].nHeight;
            bHOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bHOverlapped;
            bVOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bVOverlapped;
        }
        else
        {
//          if (!bClipVirt)
//              rVirtPosY -= (long) (pDoc->GetRowHeight( rOverY, nTab ) * nPPTY);
            sal_uInt16 nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                rOverX, rOverY, nTab, ATTR_MERGE_FLAG ))->GetValue();
            bHOver = ((nOverlap & SC_MF_HOR) != 0);
            bVOver = ((nOverlap & SC_MF_VER) != 0);
        }
    }

    return sal_True;
}

inline sal_Bool StringDiffer( const ScPatternAttr*& rpOldPattern, const ScPatternAttr*& rpNewPattern )
{
    DBG_ASSERT( rpNewPattern, "pNewPattern" );

    if ( rpNewPattern == rpOldPattern )
        return sal_False;
    else if ( !rpOldPattern )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT ) != &rpOldPattern->GetItem( ATTR_FONT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT ) != &rpOldPattern->GetItem( ATTR_CJK_FONT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT ) != &rpOldPattern->GetItem( ATTR_CTL_FONT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_HEIGHT ) != &rpOldPattern->GetItem( ATTR_FONT_HEIGHT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT_HEIGHT ) != &rpOldPattern->GetItem( ATTR_CJK_FONT_HEIGHT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT_HEIGHT ) != &rpOldPattern->GetItem( ATTR_CTL_FONT_HEIGHT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_WEIGHT ) != &rpOldPattern->GetItem( ATTR_FONT_WEIGHT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT_WEIGHT ) != &rpOldPattern->GetItem( ATTR_CJK_FONT_WEIGHT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT_WEIGHT ) != &rpOldPattern->GetItem( ATTR_CTL_FONT_WEIGHT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_POSTURE ) != &rpOldPattern->GetItem( ATTR_FONT_POSTURE ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT_POSTURE ) != &rpOldPattern->GetItem( ATTR_CJK_FONT_POSTURE ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT_POSTURE ) != &rpOldPattern->GetItem( ATTR_CTL_FONT_POSTURE ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_UNDERLINE ) != &rpOldPattern->GetItem( ATTR_FONT_UNDERLINE ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_OVERLINE ) != &rpOldPattern->GetItem( ATTR_FONT_OVERLINE ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_WORDLINE ) != &rpOldPattern->GetItem( ATTR_FONT_WORDLINE ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_CROSSEDOUT ) != &rpOldPattern->GetItem( ATTR_FONT_CROSSEDOUT ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_CONTOUR ) != &rpOldPattern->GetItem( ATTR_FONT_CONTOUR ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_SHADOWED ) != &rpOldPattern->GetItem( ATTR_FONT_SHADOWED ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_COLOR ) != &rpOldPattern->GetItem( ATTR_FONT_COLOR ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_HOR_JUSTIFY ) != &rpOldPattern->GetItem( ATTR_HOR_JUSTIFY ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_VER_JUSTIFY ) != &rpOldPattern->GetItem( ATTR_VER_JUSTIFY ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_STACKED ) != &rpOldPattern->GetItem( ATTR_STACKED ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_LINEBREAK ) != &rpOldPattern->GetItem( ATTR_LINEBREAK ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_MARGIN ) != &rpOldPattern->GetItem( ATTR_MARGIN ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_ROTATE_VALUE ) != &rpOldPattern->GetItem( ATTR_ROTATE_VALUE ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FORBIDDEN_RULES ) != &rpOldPattern->GetItem( ATTR_FORBIDDEN_RULES ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_EMPHASISMARK ) != &rpOldPattern->GetItem( ATTR_FONT_EMPHASISMARK ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_RELIEF ) != &rpOldPattern->GetItem( ATTR_FONT_RELIEF ) )
        return sal_True;
    else if ( &rpNewPattern->GetItem( ATTR_BACKGROUND ) != &rpOldPattern->GetItem( ATTR_BACKGROUND ) )
        return sal_True;    // needed with automatic text color
    else
    {
        rpOldPattern = rpNewPattern;
        return sal_False;
    }
}

inline void lcl_CreateInterpretProgress( sal_Bool& bProgress, ScDocument* pDoc,
        ScFormulaCell* pFCell )
{
    if ( !bProgress && pFCell->GetDirty() )
    {
        ScProgress::CreateInterpretProgress( pDoc, sal_True );
        bProgress = sal_True;
    }
}

inline sal_uInt8 GetScriptType( ScDocument* pDoc, ScBaseCell* pCell,
                            const ScPatternAttr* pPattern,
                            const SfxItemSet* pCondSet )
{
    return pDoc->GetCellScriptType( pCell, pPattern->GetNumberFormat( pDoc->GetFormatTable(), pCondSet ) );
}

inline sal_Bool IsAmbiguousScript( sal_uInt8 nScript )
{
    return ( nScript != SCRIPTTYPE_LATIN &&
             nScript != SCRIPTTYPE_ASIAN &&
             nScript != SCRIPTTYPE_COMPLEX );
}

sal_Bool ScOutputData::IsEmptyCellText( RowInfo* pThisRowInfo, SCCOL nX, SCROW nY )
{
    // pThisRowInfo may be NULL

    sal_Bool bEmpty;
    if ( pThisRowInfo && nX <= nX2 )
        bEmpty = pThisRowInfo->pCellInfo[nX+1].bEmptyCellText;
    else
        bEmpty = ( pDoc->GetCell( ScAddress( nX, nY, nTab ) ) == NULL );

    if ( !bEmpty && ( nX < nX1 || nX > nX2 || !pThisRowInfo ) )
    {
        //  for the range nX1..nX2 in RowInfo, cell protection attribute is already evaluated
        //  into bEmptyCellText in ScDocument::FillInfo / lcl_HidePrint (printfun)

        sal_Bool bIsPrint = ( eType == OUTTYPE_PRINTER );

        if ( bIsPrint || bTabProtected )
        {
            const ScProtectionAttr* pAttr = (const ScProtectionAttr*)
                    pDoc->GetEffItem( nX, nY, nTab, ATTR_PROTECTION );
            if ( bIsPrint && pAttr->GetHidePrint() )
                bEmpty = sal_True;
            else if ( bTabProtected )
            {
                if ( pAttr->GetHideCell() )
                    bEmpty = sal_True;
                else if ( bShowFormulas && pAttr->GetHideFormula() )
                {
                    ScBaseCell* pCell = pDoc->GetCell( ScAddress( nX, nY, nTab ) );
                    if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
                        bEmpty = sal_True;
                }
            }
        }
    }
    return bEmpty;
}

void ScOutputData::GetVisibleCell( SCCOL nCol, SCROW nRow, SCTAB nTabP, ScBaseCell*& rpCell )
{
    pDoc->GetCell( nCol, nRow, nTabP, rpCell );
    if ( rpCell && IsEmptyCellText( NULL, nCol, nRow ) )
        rpCell = NULL;
}

sal_Bool ScOutputData::IsAvailable( SCCOL nX, SCROW nY )
{
    //  apply the same logic here as in DrawStrings/DrawEdit:
    //  Stop at non-empty or merged or overlapped cell,
    //  where a note is empty as well as a cell that's hidden by protection settings

    const ScBaseCell* pCell = pDoc->GetCell( ScAddress( nX, nY, nTab ) );
    if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE && !IsEmptyCellText( NULL, nX, nY ) )
    {
        return sal_False;
    }

    const ScPatternAttr* pPattern = pDoc->GetPattern( nX, nY, nTab );
    if ( ((const ScMergeAttr&)pPattern->GetItem(ATTR_MERGE)).IsMerged() ||
         ((const ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).IsOverlapped() )
    {
        return sal_False;
    }

    return sal_True;
}

// nX, nArrY:       loop variables from DrawStrings / DrawEdit
// nPosX, nPosY:    corresponding positions for nX, nArrY
// nCellX, nCellY:  position of the cell that contains the text
// nNeeded:         Text width, including margin
// rPattern:        cell format at nCellX, nCellY
// nHorJustify:     horizontal alignment (visual) to determine which cells to use for long strings
// bCellIsValue:    if set, don't extend into empty cells
// bBreak:          if set, don't extend, and don't set clip marks (but rLeftClip/rRightClip is set)
// bOverwrite:      if set, also extend into non-empty cells (for rotated text)
// rParam           output: various area parameters.

void ScOutputData::GetOutputArea( SCCOL nX, SCSIZE nArrY, long nPosX, long nPosY,
                                  SCCOL nCellX, SCROW nCellY, long nNeeded,
                                  const ScPatternAttr& rPattern,
                                  sal_uInt16 nHorJustify, bool bCellIsValue,
                                  bool bBreak, bool bOverwrite,
                                  OutputAreaParam& rParam )
{
    //  rThisRowInfo may be for a different row than nCellY, is still used for clip marks
    RowInfo& rThisRowInfo = pRowInfo[nArrY];

    long nLayoutSign = bLayoutRTL ? -1 : 1;

    long nCellPosX = nPosX;         // find nCellX position, starting at nX/nPosX
    SCCOL nCompCol = nX;
    while ( nCellX > nCompCol )
    {
        //! extra member function for width?
        long nColWidth = ( nCompCol <= nX2 ) ?
                pRowInfo[0].pCellInfo[nCompCol+1].nWidth :
                (long) ( pDoc->GetColWidth( nCompCol, nTab ) * nPPTX );
        nCellPosX += nColWidth * nLayoutSign;
        ++nCompCol;
    }
    while ( nCellX < nCompCol )
    {
        --nCompCol;
        long nColWidth = ( nCompCol <= nX2 ) ?
                pRowInfo[0].pCellInfo[nCompCol+1].nWidth :
                (long) ( pDoc->GetColWidth( nCompCol, nTab ) * nPPTX );
        nCellPosX -= nColWidth * nLayoutSign;
    }

    long nCellPosY = nPosY;         // find nCellY position, starting at nArrY/nPosY
    SCSIZE nCompArr = nArrY;
    SCROW nCompRow = pRowInfo[nCompArr].nRowNo;
    while ( nCellY > nCompRow )
    {
        if ( nCompArr + 1 < nArrCount )
        {
            nCellPosY += pRowInfo[nCompArr].nHeight;
            ++nCompArr;
            nCompRow = pRowInfo[nCompArr].nRowNo;
        }
        else
        {
            sal_uInt16 nDocHeight = pDoc->GetRowHeight( nCompRow, nTab );
            if ( nDocHeight )
                nCellPosY += (long) ( nDocHeight * nPPTY );
            ++nCompRow;
        }
    }
    nCellPosY -= (long) pDoc->GetScaledRowHeight( nCellY, nCompRow-1, nTab, nPPTY );

    const ScMergeAttr* pMerge = (const ScMergeAttr*)&rPattern.GetItem( ATTR_MERGE );
    sal_Bool bMerged = pMerge->IsMerged();
    long nMergeCols = pMerge->GetColMerge();
    if ( nMergeCols == 0 )
        nMergeCols = 1;
    long nMergeRows = pMerge->GetRowMerge();
    if ( nMergeRows == 0 )
        nMergeRows = 1;

    long i;
    long nMergeSizeX = 0;
    for ( i=0; i<nMergeCols; i++ )
    {
        long nColWidth = ( nCellX+i <= nX2 ) ?
                pRowInfo[0].pCellInfo[nCellX+i+1].nWidth :
                (long) ( pDoc->GetColWidth( sal::static_int_cast<SCCOL>(nCellX+i), nTab ) * nPPTX );
        nMergeSizeX += nColWidth;
    }
    long nMergeSizeY = 0;
    short nDirect = 0;
    if ( rThisRowInfo.nRowNo == nCellY )
    {
        // take first row's height from row info
        nMergeSizeY += rThisRowInfo.nHeight;
        nDirect = 1;        // skip in loop
    }
    // following rows always from document
    nMergeSizeY += (long) pDoc->GetScaledRowHeight( nCellY+nDirect, nCellY+nMergeRows-1, nTab, nPPTY);

    --nMergeSizeX;      // leave out the grid horizontally, also for alignment (align between grid lines)

    rParam.mnColWidth = nMergeSizeX; // store the actual column width.

    //
    // construct the rectangles using logical left/right values (justify is called at the end)
    //

    //  rAlignRect is the single cell or merged area, used for alignment.

    rParam.maAlignRect.Left() = nCellPosX;
    rParam.maAlignRect.Right() = nCellPosX + ( nMergeSizeX - 1 ) * nLayoutSign;
    rParam.maAlignRect.Top() = nCellPosY;
    rParam.maAlignRect.Bottom() = nCellPosY + nMergeSizeY - 1;

    //  rClipRect is all cells that are used for output.
    //  For merged cells this is the same as rAlignRect, otherwise neighboring cells can also be used.

    rParam.maClipRect = rParam.maAlignRect;
    if ( nNeeded > nMergeSizeX )
    {
        SvxCellHorJustify eHorJust = (SvxCellHorJustify)nHorJustify;

        long nMissing = nNeeded - nMergeSizeX;
        long nLeftMissing = 0;
        long nRightMissing = 0;
        switch ( eHorJust )
        {
            case SVX_HOR_JUSTIFY_LEFT:
                nRightMissing = nMissing;
                break;
            case SVX_HOR_JUSTIFY_RIGHT:
                nLeftMissing = nMissing;
                break;
            case SVX_HOR_JUSTIFY_CENTER:
                nLeftMissing = nMissing / 2;
                nRightMissing = nMissing - nLeftMissing;
                break;
            default:
            {
                // added to avoid warnings
            }
        }

        // nLeftMissing, nRightMissing are logical, eHorJust values are visual
        if ( bLayoutRTL )
            ::std::swap( nLeftMissing, nRightMissing );

        SCCOL nRightX = nCellX;
        SCCOL nLeftX = nCellX;
        if ( !bMerged && !bCellIsValue && !bBreak )
        {
            //  look for empty cells into which the text can be extended

            while ( nRightMissing > 0 && nRightX < MAXCOL && ( bOverwrite || IsAvailable( nRightX+1, nCellY ) ) )
            {
                ++nRightX;
                long nAdd = (long) ( pDoc->GetColWidth( nRightX, nTab ) * nPPTX );
                nRightMissing -= nAdd;
                rParam.maClipRect.Right() += nAdd * nLayoutSign;

                if ( rThisRowInfo.nRowNo == nCellY && nRightX >= nX1 && nRightX <= nX2 )
                    rThisRowInfo.pCellInfo[nRightX].bHideGrid = sal_True;
            }

            while ( nLeftMissing > 0 && nLeftX > 0 && ( bOverwrite || IsAvailable( nLeftX-1, nCellY ) ) )
            {
                if ( rThisRowInfo.nRowNo == nCellY && nLeftX >= nX1 && nLeftX <= nX2 )
                    rThisRowInfo.pCellInfo[nLeftX].bHideGrid = sal_True;

                --nLeftX;
                long nAdd = (long) ( pDoc->GetColWidth( nLeftX, nTab ) * nPPTX );
                nLeftMissing -= nAdd;
                rParam.maClipRect.Left() -= nAdd * nLayoutSign;
            }
        }

        //  Set flag and reserve space for clipping mark triangle,
        //  even if rThisRowInfo isn't for nCellY (merged cells).
        if ( nRightMissing > 0 && bMarkClipped && nRightX >= nX1 && nRightX <= nX2 && !bBreak && !bCellIsValue )
        {
            rThisRowInfo.pCellInfo[nRightX+1].nClipMark |= SC_CLIPMARK_RIGHT;
            bAnyClipped = sal_True;
            long nMarkPixel = (long)( SC_CLIPMARK_SIZE * nPPTX );
            rParam.maClipRect.Right() -= nMarkPixel * nLayoutSign;
        }
        if ( nLeftMissing > 0 && bMarkClipped && nLeftX >= nX1 && nLeftX <= nX2 && !bBreak && !bCellIsValue )
        {
            rThisRowInfo.pCellInfo[nLeftX+1].nClipMark |= SC_CLIPMARK_LEFT;
            bAnyClipped = sal_True;
            long nMarkPixel = (long)( SC_CLIPMARK_SIZE * nPPTX );
            rParam.maClipRect.Left() += nMarkPixel * nLayoutSign;
        }

        rParam.mbLeftClip = ( nLeftMissing > 0 );
        rParam.mbRightClip = ( nRightMissing > 0 );
    }
    else
    {
        rParam.mbLeftClip = rParam.mbRightClip = sal_False;

        // leave space for AutoFilter on screen
        // (for automatic line break: only if not formatting for printer, as in ScColumn::GetNeededSize)

        if ( eType==OUTTYPE_WINDOW &&
             ( static_cast<const ScMergeFlagAttr&>(rPattern.GetItem(ATTR_MERGE_FLAG)).GetValue() & SC_MF_AUTO ) &&
             ( !bBreak || pRefDevice == pFmtDevice ) )
        {
            // filter drop-down width is now independent from row height
            const long nFilter = DROPDOWN_BITMAP_SIZE;
            sal_Bool bFit = ( nNeeded + nFilter <= nMergeSizeX );
            if ( bFit || bCellIsValue )
            {
                // content fits even in the remaining area without the filter button
                // -> align within that remaining area

                rParam.maAlignRect.Right() -= nFilter * nLayoutSign;
                rParam.maClipRect.Right() -= nFilter * nLayoutSign;

                // if a number doesn't fit, don't hide part of the number behind the button
                // -> set clip flags, so "###" replacement is used (but also within the smaller area)

                if ( !bFit )
                    rParam.mbLeftClip = rParam.mbRightClip = sal_True;
            }
        }
    }

    //  justify both rectangles for alignment calculation, use with DrawText etc.

    rParam.maAlignRect.Justify();
    rParam.maClipRect.Justify();

#if 0
    //! Test !!!
    pDev->Push();
    pDev->SetLineColor();
    pDev->SetFillColor( COL_LIGHTGREEN );
    pDev->DrawRect( pDev->PixelToLogic(rParam.maClipRect) );
    pDev->DrawRect( rParam.maClipRect );    // print preview
    pDev->Pop();
    //! Test !!!
#endif
}

void ScOutputData::DrawStrings( sal_Bool bPixelToLogic )
{
    DBG_ASSERT( pDev == pRefDevice ||
                pDev->GetMapMode().GetMapUnit() == pRefDevice->GetMapMode().GetMapUnit(),
                "DrawStrings: unterschiedliche MapUnits ?!?!" );

    vcl::PDFExtOutDevData* pPDFData = PTR_CAST( vcl::PDFExtOutDevData, pDev->GetExtOutDevData() );

    sal_Bool bWasIdleDisabled = pDoc->IsIdleDisabled();
    pDoc->DisableIdle( sal_True );
    Size aMinSize = pRefDevice->PixelToLogic(Size(0,100));      // erst darueber wird ausgegeben
//    sal_uInt32 nMinHeight = aMinSize.Height() / 200;                // 1/2 Pixel

    ScDrawStringsVars aVars( this, bPixelToLogic );

    sal_Bool bProgress = sal_False;

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
        nInitPosX += nMirrorW - 1;              // pixels
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    SCCOL nLastContentCol = MAXCOL;
    if ( nX2 < MAXCOL )
        nLastContentCol = sal::static_int_cast<SCCOL>(
            nLastContentCol - pDoc->GetEmptyLinesInBlock( nX2+1, nY1, nTab, MAXCOL, nY2, nTab, DIR_RIGHT ) );
    SCCOL nLoopStartX = nX1;
    if ( nX1 > 0 )
        --nLoopStartX;          // start before nX1 for rest of long text to the left

    // variables for GetOutputArea
    OutputAreaParam aAreaParam;
    sal_Bool bCellIsValue = sal_False;
    long nNeededWidth = 0;
    SvxCellHorJustify eOutHorJust = SVX_HOR_JUSTIFY_STANDARD;
    const ScPatternAttr* pPattern = NULL;
    const SfxItemSet* pCondSet = NULL;
    const ScPatternAttr* pOldPattern = NULL;
    const SfxItemSet* pOldCondSet = NULL;
    sal_uInt8 nOldScript = 0;

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->bChanged )
        {
            SCROW nY = pThisRowInfo->nRowNo;
//            long nCellHeight = (long) pThisRowInfo->nHeight;
            long nPosX = nInitPosX;
            if ( nLoopStartX < nX1 )
                nPosX -= pRowInfo[0].pCellInfo[nLoopStartX+1].nWidth * nLayoutSign;
            for (SCCOL nX=nLoopStartX; nX<=nX2; nX++)
            {
                sal_Bool bMergeEmpty = sal_False;
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                sal_Bool bEmpty = nX < nX1 || pInfo->bEmptyCellText;

                SCCOL nCellX = nX;                  // position where the cell really starts
                SCROW nCellY = nY;
                sal_Bool bDoCell = sal_False;
                sal_Bool bNeedEdit = sal_False;

                //
                //  Part of a merged cell?
                //

                sal_Bool bOverlapped = ( pInfo->bHOverlapped || pInfo->bVOverlapped );
                if ( bOverlapped )
                {
                    bEmpty = sal_True;

                    SCCOL nOverX;                   // start of the merged cells
                    SCROW nOverY;
                    sal_Bool bVisChanged = !pRowInfo[nArrY-1].bChanged;
                    if (GetMergeOrigin( nX,nY, nArrY, nOverX,nOverY, bVisChanged ))
                    {
                        nCellX = nOverX;
                        nCellY = nOverY;
                        bDoCell = sal_True;
                    }
                    else
                        bMergeEmpty = sal_True;
                }

                //
                //  Rest of a long text further to the left?
                //

                if ( bEmpty && !bMergeEmpty && nX < nX1 && !bOverlapped )
                {
                    SCCOL nTempX=nX1;
                    while (nTempX > 0 && IsEmptyCellText( pThisRowInfo, nTempX, nY ))
                        --nTempX;

                    if ( nTempX < nX1 &&
                         !IsEmptyCellText( pThisRowInfo, nTempX, nY ) &&
                         !pDoc->HasAttrib( nTempX,nY,nTab, nX1,nY,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                    {
                        nCellX = nTempX;
                        bDoCell = sal_True;
                    }
                }

                //
                //  Rest of a long text further to the right?
                //

                if ( bEmpty && !bMergeEmpty && nX == nX2 && !bOverlapped )
                {
                    //  don't have to look further than nLastContentCol

                    SCCOL nTempX=nX;
                    while (nTempX < nLastContentCol && IsEmptyCellText( pThisRowInfo, nTempX, nY ))
                        ++nTempX;

                    if ( nTempX > nX &&
                         !IsEmptyCellText( pThisRowInfo, nTempX, nY ) &&
                         !pDoc->HasAttrib( nTempX,nY,nTab, nX,nY,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                    {
                        nCellX = nTempX;
                        bDoCell = sal_True;
                    }
                }

                //
                //  normal visible cell
                //

                if (!bEmpty)
                    bDoCell = sal_True;

                //
                //  don't output the cell that's being edited
                //

                if ( bDoCell && bEditMode && nCellX == nEditCol && nCellY == nEditRow )
                    bDoCell = sal_False;

                //
                //  output the cell text
                //

                ScBaseCell* pCell = NULL;
                if (bDoCell)
                {
                    if ( nCellY == nY && nCellX == nX && nCellX >= nX1 && nCellX <= nX2 )
                        pCell = pThisRowInfo->pCellInfo[nCellX+1].pCell;
                    else
                        GetVisibleCell( nCellX, nCellY, nTab, pCell );      // get from document
                    if ( !pCell )
                        bDoCell = sal_False;
                    else if ( pCell->GetCellType() == CELLTYPE_EDIT )
                        bNeedEdit = sal_True;
                }
                if (bDoCell && !bNeedEdit)
                {
                    if ( nCellY == nY && nCellX >= nX1 && nCellX <= nX2 )
                    {
                        CellInfo& rCellInfo = pThisRowInfo->pCellInfo[nCellX+1];
                        pPattern = rCellInfo.pPatternAttr;
                        pCondSet = rCellInfo.pConditionSet;

                        if ( !pPattern )
                        {
                            // #i68085# pattern from cell info for hidden columns is null,
                            // test for null is quicker than using column flags
                            pPattern = pDoc->GetPattern( nCellX, nCellY, nTab );
                            pCondSet = pDoc->GetCondResult( nCellX, nCellY, nTab );
                        }
                    }
                    else        // get from document
                    {
                        pPattern = pDoc->GetPattern( nCellX, nCellY, nTab );
                        pCondSet = pDoc->GetCondResult( nCellX, nCellY, nTab );
                    }

                    sal_uInt8 nScript = GetScriptType( pDoc, pCell, pPattern, pCondSet );
                    if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();
                    if ( pPattern != pOldPattern || pCondSet != pOldCondSet ||
                         nScript != nOldScript || bSyntaxMode )
                    {
                        if ( StringDiffer(pOldPattern,pPattern) ||
                             pCondSet != pOldCondSet || nScript != nOldScript || bSyntaxMode )
                            aVars.SetPattern( pPattern, pCondSet, pCell, nScript );
                        else
                            aVars.SetPatternSimple( pPattern, pCondSet );
                        pOldPattern = pPattern;
                        pOldCondSet = pCondSet;
                        nOldScript = nScript;
                    }

                    //  use edit engine for rotated, stacked or mixed-script text
                    if ( aVars.GetOrient() == SVX_ORIENTATION_STACKED ||
                         aVars.IsRotated() || IsAmbiguousScript(nScript) )
                        bNeedEdit = sal_True;
                }
                if (bDoCell && !bNeedEdit)
                {
                    sal_Bool bFormulaCell = (pCell->GetCellType() == CELLTYPE_FORMULA );
                    if ( bFormulaCell )
                        lcl_CreateInterpretProgress( bProgress, pDoc, (ScFormulaCell*)pCell );
                    if ( aVars.SetText(pCell) )
                        pOldPattern = NULL;
                    bNeedEdit = aVars.HasEditCharacters() ||
                                    (bFormulaCell && ((ScFormulaCell*)pCell)->IsMultilineResult());
                }
                long nTotalMargin = 0;
                if (bDoCell && !bNeedEdit)
                {
                    CellType eCellType = pCell->GetCellType();
                    bCellIsValue = ( eCellType == CELLTYPE_VALUE );
                    if ( eCellType == CELLTYPE_FORMULA )
                    {
                        ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                        bCellIsValue = pFCell->IsRunning() || pFCell->IsValue();
                    }

                    eOutHorJust = ( aVars.GetHorJust() != SVX_HOR_JUSTIFY_STANDARD ) ?
                                  aVars.GetHorJust() :
                                  ( bCellIsValue ? SVX_HOR_JUSTIFY_RIGHT : SVX_HOR_JUSTIFY_LEFT );

                    if ( eOutHorJust == SVX_HOR_JUSTIFY_BLOCK || eOutHorJust == SVX_HOR_JUSTIFY_REPEAT )
                        eOutHorJust = SVX_HOR_JUSTIFY_LEFT;     // repeat is not yet implemented

                    sal_Bool bBreak = ( aVars.GetLineBreak() || aVars.GetHorJust() == SVX_HOR_JUSTIFY_BLOCK );

                    // #i111387# #o11817313# disable automatic line breaks only for "General" number format
                    if ( bBreak && bCellIsValue && ( aVars.GetValueFormat() % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 )
                        bBreak = sal_False;

                    sal_Bool bRepeat = aVars.IsRepeat() && !bBreak;
                    sal_Bool bShrink = aVars.IsShrink() && !bBreak && !bRepeat;

                    nTotalMargin =
                        static_cast<long>(aVars.GetLeftTotal() * nPPTX) +
                        static_cast<long>(aVars.GetMargin()->GetRightMargin() * nPPTX);

                    nNeededWidth = aVars.GetTextSize().Width() + nTotalMargin;

                    // GetOutputArea gives justfied rectangles
                    GetOutputArea( nX, nArrY, nPosX, nPosY, nCellX, nCellY, nNeededWidth,
                                   *pPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                                   bCellIsValue || bRepeat || bShrink, bBreak, sal_False,
                                   aAreaParam );

                    if ( bShrink )
                    {
                        if ( aVars.GetOrient() != SVX_ORIENTATION_STANDARD )
                        {
                            // Only horizontal scaling is handled here.
                            // DrawEdit is used to vertically scale 90 deg rotated text.
                            bNeedEdit = sal_True;
                        }
                        else if ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip )     // horizontal
                        {
                            long nAvailable = aAreaParam.maAlignRect.GetWidth() - nTotalMargin;
                            long nScaleSize = aVars.GetTextSize().Width();         // without margin

                            if ( nScaleSize > 0 )       // 0 if the text is empty (formulas, number formats)
                            {
                                long nScale = ( nAvailable * 100 ) / nScaleSize;

                                aVars.SetShrinkScale( nScale, nOldScript );
                                long nNewSize = aVars.GetTextSize().Width();

                                sal_uInt16 nShrinkAgain = 0;
                                while ( nNewSize > nAvailable && nShrinkAgain < SC_SHRINKAGAIN_MAX )
                                {
                                    // If the text is still too large, reduce the scale again by 10%, until it fits,
                                    // at most 7 times (it's less than 50% of the calculated scale then).

                                    nScale = ( nScale * 9 ) / 10;
                                    aVars.SetShrinkScale( nScale, nOldScript );
                                    nNewSize = aVars.GetTextSize().Width();
                                    ++nShrinkAgain;
                                }
                                // If even at half the size the font still isn't rendered smaller,
                                // fall back to normal clipping (showing ### for numbers).
                                if ( nNewSize <= nAvailable )
                                    aAreaParam.mbLeftClip = aAreaParam.mbRightClip = sal_False;

                                pOldPattern = NULL;
                            }
                        }
                    }

                    if ( bRepeat && !aAreaParam.mbLeftClip && !aAreaParam.mbRightClip )
                    {
                        long nAvailable = aAreaParam.maAlignRect.GetWidth() - nTotalMargin;
                        long nRepeatSize = aVars.GetTextSize().Width();         // without margin
                        // When formatting for the printer, the text sizes don't always add up.
                        // Round down (too few repetitions) rather than exceeding the cell size then:
                        if ( pFmtDevice != pRefDevice )
                            ++nRepeatSize;
                        if ( nRepeatSize > 0 )
                        {
                            long nRepeatCount = nAvailable / nRepeatSize;
                            if ( nRepeatCount > 1 )
                            {
                                String aCellStr = aVars.GetString();
                                String aRepeated = aCellStr;
                                for ( long nRepeat = 1; nRepeat < nRepeatCount; nRepeat++ )
                                    aRepeated.Append( aCellStr );
                                aVars.SetAutoText( aRepeated );
                            }
                        }
                    }

                    //  use edit engine if automatic line breaks are needed
                    if ( bBreak )
                    {
                        if ( aVars.GetOrient() == SVX_ORIENTATION_STANDARD )
                            bNeedEdit = ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip );
                        else
                        {
                            long nHeight = aVars.GetTextSize().Height() +
                                            (long)(aVars.GetMargin()->GetTopMargin()*nPPTY) +
                                            (long)(aVars.GetMargin()->GetBottomMargin()*nPPTY);
                            bNeedEdit = ( nHeight > aAreaParam.maClipRect.GetHeight() );
                        }
                    }
                }
                if (bNeedEdit)
                {
                    //  mark the cell in CellInfo to be drawn in DrawEdit:
                    //  Cells to the left are marked directly, cells to the
                    //  right are handled by the flag for nX2
                    SCCOL nMarkX = ( nCellX <= nX2 ) ? nCellX : nX2;
                    RowInfo* pMarkRowInfo = ( nCellY == nY ) ? pThisRowInfo : &pRowInfo[0];
                    pMarkRowInfo->pCellInfo[nMarkX+1].bEditEngine = sal_True;
                    bDoCell = sal_False;    // don't draw here
                }
                if ( bDoCell )
                {
                    if ( bCellIsValue && ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip ) )
                    {
                        // Adjust the decimals to fit the available column width.
                        aVars.SetTextToWidthOrHash(pCell, aAreaParam.mnColWidth - nTotalMargin);
                        nNeededWidth = aVars.GetTextSize().Width() +
                                    (long) ( aVars.GetLeftTotal() * nPPTX ) +
                                    (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                        if ( nNeededWidth <= aAreaParam.maClipRect.GetWidth() )
                            aAreaParam.mbLeftClip = aAreaParam.mbRightClip = sal_False;

                        //  If the "###" replacement doesn't fit into the cells, no clip marks
                        //  are shown, as the "###" already denotes too little space.
                        //  The rectangles from the first GetOutputArea call remain valid.
                    }

                    long nJustPosX = aAreaParam.maAlignRect.Left();     // "justified" - effect of alignment will be added
                    long nJustPosY = aAreaParam.maAlignRect.Top();
                    long nAvailWidth = aAreaParam.maAlignRect.GetWidth();
                    long nOutHeight = aAreaParam.maAlignRect.GetHeight();

                    sal_Bool bOutside = ( aAreaParam.maClipRect.Right() < nScrX || aAreaParam.maClipRect.Left() >= nScrX + nScrW );
                    if ( aAreaParam.maClipRect.Left() < nScrX )
                    {
                        aAreaParam.maClipRect.Left() = nScrX;
                        aAreaParam.mbLeftClip = sal_True;
                    }
                    if ( aAreaParam.maClipRect.Right() > nScrX + nScrW )
                    {
                        aAreaParam.maClipRect.Right() = nScrX + nScrW;          //! minus one?
                        aAreaParam.mbRightClip = sal_True;
                    }

                    sal_Bool bHClip = aAreaParam.mbLeftClip || aAreaParam.mbRightClip;
                    sal_Bool bVClip = sal_False;

                    if ( aAreaParam.maClipRect.Top() < nScrY )
                    {
                        aAreaParam.maClipRect.Top() = nScrY;
                        bVClip = sal_True;
                    }
                    if ( aAreaParam.maClipRect.Bottom() > nScrY + nScrH )
                    {
                        aAreaParam.maClipRect.Bottom() = nScrY + nScrH;         //! minus one?
                        bVClip = sal_True;
                    }

                    //
                    //      horizontalen Platz testen
                    //

                    sal_Bool bRightAdjusted = sal_False;        // to correct text width calculation later
                    sal_Bool bNeedEditEngine = sal_False;
                    if ( !bNeedEditEngine && !bOutside )
                    {
                        switch (eOutHorJust)
                        {
                            case SVX_HOR_JUSTIFY_LEFT:
                                nJustPosX += (long) ( aVars.GetLeftTotal() * nPPTX );
                                break;
                            case SVX_HOR_JUSTIFY_RIGHT:
                                nJustPosX += nAvailWidth - aVars.GetTextSize().Width() -
                                            (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                                bRightAdjusted = sal_True;
                                break;
                            case SVX_HOR_JUSTIFY_CENTER:
                                nJustPosX += ( nAvailWidth - aVars.GetTextSize().Width() +
                                            (long) ( aVars.GetLeftTotal() * nPPTX ) -
                                            (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX ) ) / 2;
                                break;
                            default:
                            {
                                // added to avoid warnings
                            }
                        }

                        long nTestClipHeight = aVars.GetTextSize().Height();
                        switch (aVars.GetVerJust())
                        {
                            case SVX_VER_JUSTIFY_TOP:
                                {
                                    long nTop = (long)( aVars.GetMargin()->GetTopMargin() * nPPTY );
                                    nJustPosY += nTop;
                                    nTestClipHeight += nTop;
                                }
                                break;
                            case SVX_VER_JUSTIFY_BOTTOM:
                                {
                                    long nBot = (long)( aVars.GetMargin()->GetBottomMargin() * nPPTY );
                                    nJustPosY += nOutHeight - aVars.GetTextSize().Height() - nBot;
                                    nTestClipHeight += nBot;
                                }
                                break;
                            case SVX_VER_JUSTIFY_CENTER:
                                {
                                    long nTop = (long)( aVars.GetMargin()->GetTopMargin() * nPPTY );
                                    long nBot = (long)( aVars.GetMargin()->GetBottomMargin() * nPPTY );
                                    nJustPosY += ( nOutHeight + nTop -
                                                    aVars.GetTextSize().Height() - nBot ) / 2;
                                    nTestClipHeight += Abs( nTop - nBot );
                                }
                                break;
                            default:
                            {
                                // added to avoid warnings
                            }
                        }

                        if ( nTestClipHeight > nOutHeight )
                        {
                            //  kein vertikales Clipping beim Drucken von Zellen mit
                            //  optimaler Hoehe, ausser bei Groesse in bedingter Formatierung
                            if ( eType != OUTTYPE_PRINTER ||
                                    ( pDoc->GetRowFlags( nCellY, nTab ) & CR_MANUALSIZE ) ||
                                    ( aVars.HasCondHeight() ) )
                                bVClip = sal_True;
                        }

                        if ( bHClip || bVClip )
                        {
                            //  nur die betroffene Dimension clippen,
                            //  damit bei nicht-proportionalem Resize nicht alle
                            //  rechtsbuendigen Zahlen abgeschnitten werden:

                            if (!bHClip)
                            {
                                aAreaParam.maClipRect.Left() = nScrX;
                                aAreaParam.maClipRect.Right() = nScrX+nScrW;
                            }
                            if (!bVClip)
                            {
                                aAreaParam.maClipRect.Top() = nScrY;
                                aAreaParam.maClipRect.Bottom() = nScrY+nScrH;
                            }

                            //  aClipRect is not used after SetClipRegion/IntersectClipRegion,
                            //  so it can be modified here
                            if (bPixelToLogic)
                                aAreaParam.maClipRect = pRefDevice->PixelToLogic( aAreaParam.maClipRect );

                            if (bMetaFile)
                            {
                                pDev->Push();
                                pDev->IntersectClipRegion( aAreaParam.maClipRect );
                            }
                            else
                                pDev->SetClipRegion( Region( aAreaParam.maClipRect ) );
                        }

                        Point aURLStart( nJustPosX, nJustPosY );    // copy before modifying for orientation

                        switch (aVars.GetOrient())
                        {
                            case SVX_ORIENTATION_STANDARD:
                                nJustPosY += aVars.GetAscent();
                                break;
                            case SVX_ORIENTATION_TOPBOTTOM:
                                nJustPosX += aVars.GetTextSize().Width() - aVars.GetAscent();
                                break;
                            case SVX_ORIENTATION_BOTTOMTOP:
                                nJustPosY += aVars.GetTextSize().Height();
                                nJustPosX += aVars.GetAscent();
                                break;
                            default:
                            {
                                // added to avoid warnings
                            }
                        }

                        // When clipping, the visible part is now completely defined by the alignment,
                        // there's no more special handling to show the right part of RTL text.

                        Point aDrawTextPos( nJustPosX, nJustPosY );
                        if ( bPixelToLogic )
                        {
                            //  undo text width adjustment in pixels
                            if (bRightAdjusted)
                                aDrawTextPos.X() += aVars.GetTextSize().Width();

                            aDrawTextPos = pRefDevice->PixelToLogic( aDrawTextPos );

                            //  redo text width adjustment in logic units
                            if (bRightAdjusted)
                                aDrawTextPos.X() -= aVars.GetOriginalWidth();
                        }

                        //  in Metafiles immer DrawTextArray, damit die Positionen mit
                        //  aufgezeichnet werden (fuer nicht-proportionales Resize):

                        String aString = aVars.GetString();
                        if (bMetaFile || pFmtDevice != pDev || aZoomX != aZoomY)
                        {
                            sal_Int32* pDX = new sal_Int32[aString.Len()];
                            pFmtDevice->GetTextArray( aString, pDX );

                            if ( !pRefDevice->GetConnectMetaFile() ||
                                    pRefDevice->GetOutDevType() == OUTDEV_PRINTER )
                            {
                                double fMul = GetStretch();
                                xub_StrLen nLen = aString.Len();
                                for (xub_StrLen i=0; i<nLen; i++)
                                    pDX[i] = (long)(pDX[i] / fMul + 0.5);
                            }

                            pDev->DrawTextArray( aDrawTextPos, aString, pDX );
                            delete[] pDX;
                        }
                        else
                            pDev->DrawText( aDrawTextPos, aString );

                        if ( bHClip || bVClip )
                        {
                            if (bMetaFile)
                                pDev->Pop();
                            else
                                pDev->SetClipRegion();
                        }

                        // PDF: whole-cell hyperlink from formula?
                        sal_Bool bHasURL = pPDFData && pCell && pCell->GetCellType() == CELLTYPE_FORMULA &&
                                        static_cast<ScFormulaCell*>(pCell)->IsHyperLinkCell();
                        if ( bHasURL )
                        {
                            Rectangle aURLRect( aURLStart, aVars.GetTextSize() );
                            lcl_DoHyperlinkResult( pDev, aURLRect, pCell );
                        }
                    }
                }
                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
            }
        }
        nPosY += pRowInfo[nArrY].nHeight;
    }
    if ( bProgress )
        ScProgress::DeleteInterpretProgress();
    pDoc->DisableIdle( bWasIdleDisabled );
}

//  -------------------------------------------------------------------------------

void lcl_ClearEdit( EditEngine& rEngine )       // Text und Attribute
{
    rEngine.SetUpdateMode( sal_False );

    rEngine.SetText(EMPTY_STRING);
    //  keine Para-Attribute uebrigbehalten...
    const SfxItemSet& rPara = rEngine.GetParaAttribs(0);
    if (rPara.Count())
        rEngine.SetParaAttribs( 0,
                    SfxItemSet( *rPara.GetPool(), rPara.GetRanges() ) );
}

sal_Bool lcl_SafeIsValue( ScBaseCell* pCell )
{
    if (!pCell)
        return sal_False;

    sal_Bool bRet = sal_False;
    switch ( pCell->GetCellType() )
    {
        case CELLTYPE_VALUE:
            bRet = sal_True;
            break;
        case CELLTYPE_FORMULA:
            {
                ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                if ( pFCell->IsRunning() || pFCell->IsValue() )
                    bRet = sal_True;
            }
            break;
        default:
        {
            // added to avoid warnings
        }
    }
    return bRet;
}

void lcl_ScaleFonts( EditEngine& rEngine, long nPercent )
{
    sal_Bool bUpdateMode = rEngine.GetUpdateMode();
    if ( bUpdateMode )
        rEngine.SetUpdateMode( sal_False );

    sal_uInt16 nParCount = rEngine.GetParagraphCount();
    for (sal_uInt16 nPar=0; nPar<nParCount; nPar++)
    {
        SvUShorts aPortions;
        rEngine.GetPortions( nPar, aPortions );

        sal_uInt16 nPCount = aPortions.Count();
        sal_uInt16 nStart = 0;
        for ( sal_uInt16 nPos=0; nPos<nPCount; nPos++ )
        {
            sal_uInt16 nEnd = aPortions.GetObject( nPos );
            ESelection aSel( nPar, nStart, nPar, nEnd );
            SfxItemSet aAttribs = rEngine.GetAttribs( aSel );

            long nWestern = static_cast<const SvxFontHeightItem&>(aAttribs.Get(EE_CHAR_FONTHEIGHT)).GetHeight();
            long nCJK = static_cast<const SvxFontHeightItem&>(aAttribs.Get(EE_CHAR_FONTHEIGHT_CJK)).GetHeight();
            long nCTL = static_cast<const SvxFontHeightItem&>(aAttribs.Get(EE_CHAR_FONTHEIGHT_CTL)).GetHeight();

            nWestern = ( nWestern * nPercent ) / 100;
            nCJK     = ( nCJK     * nPercent ) / 100;
            nCTL     = ( nCTL     * nPercent ) / 100;

            aAttribs.Put( SvxFontHeightItem( nWestern, 100, EE_CHAR_FONTHEIGHT ) );
            aAttribs.Put( SvxFontHeightItem( nCJK, 100, EE_CHAR_FONTHEIGHT_CJK ) );
            aAttribs.Put( SvxFontHeightItem( nCTL, 100, EE_CHAR_FONTHEIGHT_CTL ) );

            rEngine.QuickSetAttribs( aAttribs, aSel );      //! remove paragraph attributes from aAttribs?

            nStart = nEnd;
        }
    }

    if ( bUpdateMode )
        rEngine.SetUpdateMode( sal_True );
}

long lcl_GetEditSize( EditEngine& rEngine, sal_Bool bWidth, sal_Bool bSwap, long nAttrRotate )
{
    if ( bSwap )
        bWidth = !bWidth;

    if ( nAttrRotate )
    {
        long nRealWidth  = (long) rEngine.CalcTextWidth();
        long nRealHeight = rEngine.GetTextHeight();

        // assuming standard mode, otherwise width isn't used

        double nRealOrient = nAttrRotate * F_PI18000;   // 1/100th degrees
        double nAbsCos = fabs( cos( nRealOrient ) );
        double nAbsSin = fabs( sin( nRealOrient ) );
        if ( bWidth )
            return (long) ( nRealWidth * nAbsCos + nRealHeight * nAbsSin );
        else
            return (long) ( nRealHeight * nAbsCos + nRealWidth * nAbsSin );
    }
    else if ( bWidth )
        return (long) rEngine.CalcTextWidth();
    else
        return rEngine.GetTextHeight();
}


void ScOutputData::ShrinkEditEngine( EditEngine& rEngine, const Rectangle& rAlignRect,
            long nLeftM, long nTopM, long nRightM, long nBottomM,
            sal_Bool bWidth, sal_uInt16 nOrient, long nAttrRotate, sal_Bool bPixelToLogic,
            long& rEngineWidth, long& rEngineHeight, long& rNeededPixel, bool& rLeftClip, bool& rRightClip )
{
    if ( !bWidth )
    {
        // vertical

        long nScaleSize = bPixelToLogic ?
            pRefDevice->LogicToPixel(Size(0,rEngineHeight)).Height() : rEngineHeight;

        // Don't scale if it fits already.
        // Allowing to extend into the margin, to avoid scaling at optimal height.
        if ( nScaleSize <= rAlignRect.GetHeight() )
            return;

        sal_Bool bSwap = ( nOrient == SVX_ORIENTATION_TOPBOTTOM || nOrient == SVX_ORIENTATION_BOTTOMTOP );
        long nAvailable = rAlignRect.GetHeight() - nTopM - nBottomM;
        long nScale = ( nAvailable * 100 ) / nScaleSize;

        lcl_ScaleFonts( rEngine, nScale );
        rEngineHeight = lcl_GetEditSize( rEngine, sal_False, bSwap, nAttrRotate );
        long nNewSize = bPixelToLogic ?
            pRefDevice->LogicToPixel(Size(0,rEngineHeight)).Height() : rEngineHeight;

        sal_uInt16 nShrinkAgain = 0;
        while ( nNewSize > nAvailable && nShrinkAgain < SC_SHRINKAGAIN_MAX )
        {
            // further reduce, like in DrawStrings
            lcl_ScaleFonts( rEngine, 90 );     // reduce by 10%
            rEngineHeight = lcl_GetEditSize( rEngine, sal_False, bSwap, nAttrRotate );
            nNewSize = bPixelToLogic ?
                pRefDevice->LogicToPixel(Size(0,rEngineHeight)).Height() : rEngineHeight;
            ++nShrinkAgain;
        }

        // sizes for further processing (alignment etc):
        rEngineWidth = lcl_GetEditSize( rEngine, sal_True, bSwap, nAttrRotate );
        long nPixelWidth = bPixelToLogic ?
            pRefDevice->LogicToPixel(Size(rEngineWidth,0)).Width() : rEngineWidth;
        rNeededPixel = nPixelWidth + nLeftM + nRightM;
    }
    else if ( rLeftClip || rRightClip )
    {
        // horizontal

        long nAvailable = rAlignRect.GetWidth() - nLeftM - nRightM;
        long nScaleSize = rNeededPixel - nLeftM - nRightM;      // without margin

        if ( nScaleSize <= nAvailable )
            return;

        long nScale = ( nAvailable * 100 ) / nScaleSize;

        lcl_ScaleFonts( rEngine, nScale );
        rEngineWidth = lcl_GetEditSize( rEngine, sal_True, sal_False, nAttrRotate );
        long nNewSize = bPixelToLogic ?
            pRefDevice->LogicToPixel(Size(rEngineWidth,0)).Width() : rEngineWidth;

        sal_uInt16 nShrinkAgain = 0;
        while ( nNewSize > nAvailable && nShrinkAgain < SC_SHRINKAGAIN_MAX )
        {
            // further reduce, like in DrawStrings
            lcl_ScaleFonts( rEngine, 90 );     // reduce by 10%
            rEngineWidth = lcl_GetEditSize( rEngine, sal_True, sal_False, nAttrRotate );
            nNewSize = bPixelToLogic ?
                pRefDevice->LogicToPixel(Size(rEngineWidth,0)).Width() : rEngineWidth;
            ++nShrinkAgain;
        }
        if ( nNewSize <= nAvailable )
            rLeftClip = rRightClip = sal_False;

        // sizes for further processing (alignment etc):
        rNeededPixel = nNewSize + nLeftM + nRightM;
        rEngineHeight = lcl_GetEditSize( rEngine, sal_False, sal_False, nAttrRotate );
    }
}

void ScOutputData::DrawEdit(sal_Bool bPixelToLogic)
{
    vcl::PDFExtOutDevData* pPDFData = PTR_CAST( vcl::PDFExtOutDevData, pDev->GetExtOutDevData() );

    Size aMinSize = pRefDevice->PixelToLogic(Size(0,100));      // erst darueber wird ausgegeben
//    sal_uInt32 nMinHeight = aMinSize.Height() / 200;                // 1/2 Pixel

    ScModule* pScMod = SC_MOD();
    sal_Int32 nConfBackColor = pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed sal_True)
    sal_Bool bCellContrast = bUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    ScFieldEditEngine* pEngine = NULL;
    sal_Bool bHyphenatorSet = sal_False;
    const ScPatternAttr* pOldPattern = NULL;
    const SfxItemSet*    pOldCondSet = NULL;
    ScBaseCell* pCell = NULL;

    Size aRefOne = pRefDevice->PixelToLogic(Size(1,1));

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
#if 0
        Size aOnePixel = pDev->PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        nInitPosX += nMirrorW - nOneX;
#endif
        nInitPosX += nMirrorW - 1;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    //! store nLastContentCol as member!
    SCCOL nLastContentCol = MAXCOL;
    if ( nX2 < MAXCOL )
        nLastContentCol = sal::static_int_cast<SCCOL>(
            nLastContentCol - pDoc->GetEmptyLinesInBlock( nX2+1, nY1, nTab, MAXCOL, nY2, nTab, DIR_RIGHT ) );

    long nRowPosY = nScrY;
    for (SCSIZE nArrY=0; nArrY+1<nArrCount; nArrY++)            // 0 fuer Reste von zusammengefassten
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
//        long nCellHeight = (long) pThisRowInfo->nHeight;
        if (nArrY==1) nRowPosY = nScrY;                         // vorher wird einzeln berechnet

        if ( pThisRowInfo->bChanged || nArrY==0 )
        {
            long nPosX = 0;
            for (SCCOL nX=0; nX<=nX2; nX++)                 // wegen Ueberhaengen
            {
                if (nX==nX1) nPosX = nInitPosX;                 // positions before nX1 are calculated individually

                CellInfo*   pInfo = &pThisRowInfo->pCellInfo[nX+1];
                if (pInfo->bEditEngine)
                {
                    SCROW nY = pThisRowInfo->nRowNo;

                    SCCOL nCellX = nX;                  // position where the cell really starts
                    SCROW nCellY = nY;
                    sal_Bool bDoCell = sal_False;

                    long nPosY = nRowPosY;
                    if ( nArrY == 0 )
                    {
                        nPosY = nScrY;
                        nY = pRowInfo[1].nRowNo;
                        SCCOL nOverX;                   // start of the merged cells
                        SCROW nOverY;
                        if (GetMergeOrigin( nX,nY, 1, nOverX,nOverY, sal_True ))
                        {
                            nCellX = nOverX;
                            nCellY = nOverY;
                            bDoCell = sal_True;
                        }
                    }
                    else if ( nX == nX2 && !pThisRowInfo->pCellInfo[nX+1].pCell )
                    {
                        //  Rest of a long text further to the right?

                        SCCOL nTempX=nX;
                        while (nTempX < nLastContentCol && IsEmptyCellText( pThisRowInfo, nTempX, nY ))
                            ++nTempX;

                        if ( nTempX > nX &&
                             !IsEmptyCellText( pThisRowInfo, nTempX, nY ) &&
                             !pDoc->HasAttrib( nTempX,nY,nTab, nX,nY,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                        {
                            nCellX = nTempX;
                            bDoCell = sal_True;
                        }
                    }
                    else
                    {
                        bDoCell = sal_True;
                    }

                    if ( bDoCell && bEditMode && nCellX == nEditCol && nCellY == nEditRow )
                        bDoCell = sal_False;

                    const ScPatternAttr* pPattern = NULL;
                    const SfxItemSet* pCondSet = NULL;
                    if (bDoCell)
                    {
                        if ( nCellY == nY && nCellX >= nX1 && nCellX <= nX2 &&
                             !pDoc->ColHidden(nCellX, nTab) )
                        {
                            CellInfo& rCellInfo = pThisRowInfo->pCellInfo[nCellX+1];
                            pPattern = rCellInfo.pPatternAttr;
                            pCondSet = rCellInfo.pConditionSet;
                            pCell = rCellInfo.pCell;
                        }
                        else        // get from document
                        {
                            pPattern = pDoc->GetPattern( nCellX, nCellY, nTab );
                            pCondSet = pDoc->GetCondResult( nCellX, nCellY, nTab );
                            GetVisibleCell( nCellX, nCellY, nTab, pCell );
                        }
                        if ( !pCell )
                            bDoCell = sal_False;
                    }
                    if (bDoCell)
                    {
                        sal_Bool bHidden = sal_False;

                        //
                        //  Create EditEngine
                        //

                        if (!pEngine)
                        {
                            //  Ein RefDevice muss auf jeden Fall gesetzt werden,
                            //  sonst legt sich die EditEngine ein VirtualDevice an!
                            pEngine = new ScFieldEditEngine( pDoc->GetEnginePool() );
                            pEngine->SetUpdateMode( sal_False );
                            pEngine->SetRefDevice( pFmtDevice );    // always set
                            sal_uLong nCtrl = pEngine->GetControlWord();
                            if ( bShowSpellErrors )
                                nCtrl |= EE_CNTRL_ONLINESPELLING;
                            if ( eType == OUTTYPE_PRINTER )
                                nCtrl &= ~EE_CNTRL_MARKFIELDS;
                            pEngine->SetControlWord( nCtrl );
                            pEngine->SetForbiddenCharsTable( pDoc->GetForbiddenCharacters() );
                            pEngine->SetAsianCompressionMode( pDoc->GetAsianCompression() );
                            pEngine->SetKernAsianPunctuation( pDoc->GetAsianKerning() );
                            pEngine->EnableAutoColor( bUseStyleColor );
                            pEngine->SetDefaultHorizontalTextDirection(
                                (EEHorizontalTextDirection)pDoc->GetEditTextDirection( nTab ) );
                        }
                        else
                            lcl_ClearEdit( *pEngine );      // also calls SetUpdateMode(sal_False)


                        sal_Bool bCellIsValue = lcl_SafeIsValue(pCell);

                        SvxCellHorJustify eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                            pPattern->GetItem(ATTR_HOR_JUSTIFY, pCondSet)).GetValue();
                        sal_Bool bBreak = ( eHorJust == SVX_HOR_JUSTIFY_BLOCK ) ||
                                        ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK, pCondSet)).GetValue();
                        sal_Bool bRepeat = ( eHorJust == SVX_HOR_JUSTIFY_REPEAT && !bBreak );
                        sal_Bool bShrink = !bBreak && !bRepeat && static_cast<const SfxBoolItem&>
                                        (pPattern->GetItem( ATTR_SHRINKTOFIT, pCondSet )).GetValue();
                        SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );
                        long nAttrRotate = ((const SfxInt32Item&)pPattern->
                                            GetItem(ATTR_ROTATE_VALUE, pCondSet)).GetValue();
                        if ( eHorJust == SVX_HOR_JUSTIFY_REPEAT )
                        {
                            // ignore orientation/rotation if "repeat" is active
                            eOrient = SVX_ORIENTATION_STANDARD;
                            nAttrRotate = 0;

                            // #i31843# "repeat" with "line breaks" is treated as default alignment
                            // (but rotation is still disabled)
                            if ( bBreak )
                                eHorJust = SVX_HOR_JUSTIFY_STANDARD;
                        }
                        if ( eOrient==SVX_ORIENTATION_STANDARD && nAttrRotate )
                        {
                            //! Flag setzen, um die Zelle in DrawRotated wiederzufinden ?
                            //! (oder Flag schon bei DrawBackground, dann hier keine Abfrage)
                            bHidden = sal_True;     // gedreht wird getrennt ausgegeben
                        }

                        sal_Bool bAsianVertical = ( eOrient == SVX_ORIENTATION_STACKED &&
                                ((const SfxBoolItem&)pPattern->GetItem( ATTR_VERTICAL_ASIAN, pCondSet )).GetValue() );
                        if ( bAsianVertical )
                        {
                            // in asian mode, use EditEngine::SetVertical instead of EE_CNTRL_ONECHARPERLINE
                            eOrient = SVX_ORIENTATION_STANDARD;
                            // default alignment for asian vertical mode is top-right
                            if ( eHorJust == SVX_HOR_JUSTIFY_STANDARD )
                                eHorJust = SVX_HOR_JUSTIFY_RIGHT;
                        }



                        SvxCellHorJustify eOutHorJust =
                            ( eHorJust != SVX_HOR_JUSTIFY_STANDARD ) ? eHorJust :
                            ( bCellIsValue ? SVX_HOR_JUSTIFY_RIGHT : SVX_HOR_JUSTIFY_LEFT );

                        if ( eOutHorJust == SVX_HOR_JUSTIFY_BLOCK || eOutHorJust == SVX_HOR_JUSTIFY_REPEAT )
                            eOutHorJust = SVX_HOR_JUSTIFY_LEFT;     // repeat is not yet implemented


//!                     if ( !bHidden && eType == OUTTYPE_PRINTER &&
//!                         pDev->GetOutDevType() == OUTDEV_WINDOW &&
//!                         ((const SvxFontHeightItem&)pPattern->
//!                         GetItem(ATTR_FONT_HEIGHT)).GetHeight() <= nMinHeight )
//!                     {
//!                         Point aPos( nStartX, nStartY );
//!                         pDev->DrawPixel( aPos,
//!                                         ((const SvxColorItem&)pPattern->
//!                                         GetItem( ATTR_FONT_COLOR )).GetValue() );
//!                         bHidden = sal_True;
//!                     }

                        if (!bHidden)
                        {
                            //! mirror margin values for RTL?
                            //! move margin down to after final GetOutputArea call

                            const SvxMarginItem* pMargin = (const SvxMarginItem*)
                                                    &pPattern->GetItem(ATTR_MARGIN, pCondSet);
                            sal_uInt16 nIndent = 0;
                            if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
                                nIndent = ((const SfxUInt16Item&)pPattern->
                                                    GetItem(ATTR_INDENT, pCondSet)).GetValue();

                            long nLeftM = (long) ( (pMargin->GetLeftMargin() + nIndent) * nPPTX );
                            long nTopM  = (long) ( pMargin->GetTopMargin() * nPPTY );
                            long nRightM = (long) ( pMargin->GetRightMargin() * nPPTX );
                            long nBottomM = (long) ( pMargin->GetBottomMargin() * nPPTY );

                            SCCOL nXForPos = nX;
                            if ( nXForPos < nX1 )
                            {
                                nXForPos = nX1;
                                nPosX = nInitPosX;
                            }
                            SCSIZE nArrYForPos = nArrY;
                            if ( nArrYForPos < 1 )
                            {
                                nArrYForPos = 1;
                                nPosY = nScrY;
                            }

                            OutputAreaParam aAreaParam;

                            //
                            //  Initial page size - large for normal text, cell size for automatic line breaks
                            //

                            Size aPaperSize = Size( 1000000, 1000000 );
                            if ( bBreak || eOrient == SVX_ORIENTATION_STACKED || bAsianVertical )
                            {
                                //! also stacked, AsianVertical

                                //  call GetOutputArea with nNeeded=0, to get only the cell width

                                //! handle nArrY == 0
                                GetOutputArea( nXForPos, nArrYForPos, nPosX, nPosY, nCellX, nCellY, 0,
                                               *pPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                                               bCellIsValue, true, false, aAreaParam );

                                //! special ScEditUtil handling if formatting for printer

                                if ( eOrient == SVX_ORIENTATION_TOPBOTTOM || eOrient == SVX_ORIENTATION_BOTTOMTOP )
                                    aPaperSize.Width() = aAreaParam.maAlignRect.GetHeight() - nTopM - nBottomM;
                                else
                                    aPaperSize.Width() = aAreaParam.maAlignRect.GetWidth() - nLeftM - nRightM;

                                if (bAsianVertical && bBreak)
                                {
                                    //  add some extra height (default margin value) for safety
                                    //  as long as GetEditArea isn't used below
                                    long nExtraHeight = (long)( 20 * nPPTY );
                                    aPaperSize.Height() = aAreaParam.maAlignRect.GetHeight() - nTopM - nBottomM + nExtraHeight;
                                }
                            }
                            if (bPixelToLogic)
                            {
                                Size aLogicSize = pRefDevice->PixelToLogic(aPaperSize);
                                if ( bBreak && !bAsianVertical && pRefDevice != pFmtDevice )
                                {
                                    // #i85342# screen display and formatting for printer,
                                    // use same GetEditArea call as in ScViewData::SetEditEngine

                                    Fraction aFract(1,1);
                                    Rectangle aUtilRect = ScEditUtil( pDoc, nCellX, nCellY, nTab, Point(0,0), pFmtDevice,
                                        HMM_PER_TWIPS, HMM_PER_TWIPS, aFract, aFract ).GetEditArea( pPattern, sal_False );
                                    aLogicSize.Width() = aUtilRect.GetWidth();
                                }
                                pEngine->SetPaperSize(aLogicSize);
                            }
                            else
                                pEngine->SetPaperSize(aPaperSize);

                            //
                            //  Fill the EditEngine (cell attributes and text)
                            //

                            SvxCellVerJustify eVerJust = (SvxCellVerJustify)((const SvxVerJustifyItem&)
                                                pPattern->GetItem(ATTR_VER_JUSTIFY, pCondSet)).GetValue();

                            // default alignment for asian vertical mode is top-right
                            if ( bAsianVertical && eVerJust == SVX_VER_JUSTIFY_STANDARD )
                                eVerJust = SVX_VER_JUSTIFY_TOP;

                            // syntax highlighting mode is ignored here
                            // StringDiffer doesn't look at hyphenate, language items
                            if ( pPattern != pOldPattern || pCondSet != pOldCondSet )
                            {
                                SfxItemSet* pSet = new SfxItemSet( pEngine->GetEmptyItemSet() );
                                pPattern->FillEditItemSet( pSet, pCondSet );

                                pEngine->SetDefaults( pSet );
                                pOldPattern = pPattern;
                                pOldCondSet = pCondSet;

                                sal_uLong nControl = pEngine->GetControlWord();
                                if (eOrient==SVX_ORIENTATION_STACKED)
                                    nControl |= EE_CNTRL_ONECHARPERLINE;
                                else
                                    nControl &= ~EE_CNTRL_ONECHARPERLINE;
                                pEngine->SetControlWord( nControl );

                                if ( !bHyphenatorSet && ((const SfxBoolItem&)pSet->Get(EE_PARA_HYPHENATE)).GetValue() )
                                {
                                    //  set hyphenator the first time it is needed
                                    com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
                                    pEngine->SetHyphenator( xXHyphenator );
                                    bHyphenatorSet = sal_True;
                                }

                                Color aBackCol = ((const SvxBrushItem&)
                                    pPattern->GetItem( ATTR_BACKGROUND, pCondSet )).GetColor();
                                if ( bUseStyleColor && ( aBackCol.GetTransparency() > 0 || bCellContrast ) )
                                    aBackCol.SetColor( nConfBackColor );
                                pEngine->SetBackgroundColor( aBackCol );
                            }

                            //  horizontal alignment now may depend on cell content
                            //  (for values with number formats with mixed script types)
                            //  -> always set adjustment

                            SvxAdjust eSvxAdjust = SVX_ADJUST_LEFT;
                            if (eOrient==SVX_ORIENTATION_STACKED)
                                eSvxAdjust = SVX_ADJUST_CENTER;
                            else if (bBreak)
                            {
                                if (eOrient==SVX_ORIENTATION_STANDARD && !bAsianVertical)
                                    switch (eHorJust)
                                    {
                                        case SVX_HOR_JUSTIFY_STANDARD:
                                            eSvxAdjust = bCellIsValue ? SVX_ADJUST_RIGHT : SVX_ADJUST_LEFT;
                                            break;
                                        case SVX_HOR_JUSTIFY_LEFT:
                                        case SVX_HOR_JUSTIFY_REPEAT:            // nicht implementiert
                                            eSvxAdjust = SVX_ADJUST_LEFT;
                                            break;
                                        case SVX_HOR_JUSTIFY_RIGHT:
                                            eSvxAdjust = SVX_ADJUST_RIGHT;
                                            break;
                                        case SVX_HOR_JUSTIFY_CENTER:
                                            eSvxAdjust = SVX_ADJUST_CENTER;
                                            break;
                                        case SVX_HOR_JUSTIFY_BLOCK:
                                            eSvxAdjust = SVX_ADJUST_BLOCK;
                                            break;
                                    }
                                else
                                    switch (eVerJust)
                                    {
                                        case SVX_VER_JUSTIFY_TOP:
                                            eSvxAdjust = (eOrient==SVX_ORIENTATION_TOPBOTTOM || bAsianVertical) ?
                                                        SVX_ADJUST_LEFT : SVX_ADJUST_RIGHT;
                                            break;
                                        case SVX_VER_JUSTIFY_CENTER:
                                            eSvxAdjust = SVX_ADJUST_CENTER;
                                            break;
                                        case SVX_VER_JUSTIFY_BOTTOM:
                                        case SVX_HOR_JUSTIFY_STANDARD:
                                            eSvxAdjust = (eOrient==SVX_ORIENTATION_TOPBOTTOM || bAsianVertical) ?
                                                        SVX_ADJUST_RIGHT : SVX_ADJUST_LEFT;
                                            break;
                                    }
                            }
                            pEngine->SetDefaultItem( SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );

                            //  Read content from cell

                            sal_Bool bWrapFields = sal_False;
                            if (pCell)
                            {
                                if (pCell->GetCellType() == CELLTYPE_EDIT)
                                {
                                    const EditTextObject* pData;
                                    ((ScEditCell*)pCell)->GetData(pData);

                                    if (pData)
                                    {
                                        pEngine->SetText(*pData);

                                        if ( bBreak && !bAsianVertical && pData->HasField() )
                                        {
                                            //  Fields aren't wrapped, so clipping is enabled to prevent
                                            //  a field from being drawn beyond the cell size

                                            bWrapFields = sal_True;
                                        }
                                    }
                                    else
                                    {
                                        DBG_ERROR("pData == 0");
                                    }
                                }
                                else
                                {
                                    sal_uLong nFormat = pPattern->GetNumberFormat(
                                                                pDoc->GetFormatTable(), pCondSet );
                                    String aString;
                                    Color* pColor;
                                    ScCellFormat::GetString( pCell,
                                                             nFormat,aString, &pColor,
                                                             *pDoc->GetFormatTable(),
                                                             bShowNullValues,
                                                             bShowFormulas,
                                                             ftCheck );

                                    pEngine->SetText(aString);
                                    if ( pColor && !bSyntaxMode && !( bUseStyleColor && bForceAutoColor ) )
                                        lcl_SetEditColor( *pEngine, *pColor );
                                }

                                if ( bSyntaxMode )
                                    SetEditSyntaxColor( *pEngine, pCell );
                                else if ( bUseStyleColor && bForceAutoColor )
                                    lcl_SetEditColor( *pEngine, COL_AUTO );     //! or have a flag at EditEngine
                            }
                            else
                            {
                                DBG_ERROR("pCell == NULL");
                            }

                            pEngine->SetVertical( bAsianVertical );
                            pEngine->SetUpdateMode( sal_True );     // after SetText, before CalcTextWidth/GetTextHeight

                            //
                            //  Get final output area using the calculated width
                            //

                            long nEngineWidth;
                            if ( bBreak && eOrient != SVX_ORIENTATION_STACKED && !bAsianVertical )
                                nEngineWidth = 0;
                            else
                                nEngineWidth = (long) pEngine->CalcTextWidth();
                            long nEngineHeight = pEngine->GetTextHeight();

                            if (eOrient != SVX_ORIENTATION_STANDARD &&
                                eOrient != SVX_ORIENTATION_STACKED)
                            {
                                long nTemp = nEngineWidth;
                                nEngineWidth = nEngineHeight;
                                nEngineHeight = nTemp;
                            }

                            if (eOrient == SVX_ORIENTATION_STACKED)
                                nEngineWidth = nEngineWidth * 11 / 10;

                            long nNeededPixel = nEngineWidth;
                            if (bPixelToLogic)
                                nNeededPixel = pRefDevice->LogicToPixel(Size(nNeededPixel,0)).Width();
                            nNeededPixel += nLeftM + nRightM;

                            if ( ( !bBreak && eOrient != SVX_ORIENTATION_STACKED ) || bAsianVertical || bShrink )
                            {
                                // for break, the first GetOutputArea call is sufficient
                                GetOutputArea( nXForPos, nArrYForPos, nPosX, nPosY, nCellX, nCellY, nNeededPixel,
                                               *pPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                                               bCellIsValue || bRepeat || bShrink, false, false, aAreaParam );

                                if ( bShrink )
                                {
                                    sal_Bool bWidth = ( eOrient == SVX_ORIENTATION_STANDARD && !bAsianVertical );
                                    ShrinkEditEngine( *pEngine, aAreaParam.maAlignRect,
                                        nLeftM, nTopM, nRightM, nBottomM, bWidth,
                                        sal::static_int_cast<sal_uInt16>(eOrient), 0, bPixelToLogic,
                                        nEngineWidth, nEngineHeight, nNeededPixel,
                                        aAreaParam.mbLeftClip, aAreaParam.mbRightClip );
                                }

                                if ( bRepeat && !aAreaParam.mbLeftClip && !aAreaParam.mbRightClip && pEngine->GetParagraphCount() == 1 )
                                {
                                    // First check if twice the space for the formatted text is available
                                    // (otherwise just keep it unchanged).

                                    long nFormatted = nNeededPixel - nLeftM - nRightM;      // without margin
                                    long nAvailable = aAreaParam.maAlignRect.GetWidth() - nLeftM - nRightM;
                                    if ( nAvailable >= 2 * nFormatted )
                                    {
                                        // "repeat" is handled with unformatted text (for performance reasons)
                                        String aCellStr = pEngine->GetText();
                                        pEngine->SetText( aCellStr );

                                        long nRepeatSize = (long) pEngine->CalcTextWidth();
                                        if (bPixelToLogic)
                                            nRepeatSize = pRefDevice->LogicToPixel(Size(nRepeatSize,0)).Width();
                                        if ( pFmtDevice != pRefDevice )
                                            ++nRepeatSize;
                                        if ( nRepeatSize > 0 )
                                        {
                                            long nRepeatCount = nAvailable / nRepeatSize;
                                            if ( nRepeatCount > 1 )
                                            {
                                                String aRepeated = aCellStr;
                                                for ( long nRepeat = 1; nRepeat < nRepeatCount; nRepeat++ )
                                                    aRepeated.Append( aCellStr );
                                                pEngine->SetText( aRepeated );

                                                nEngineHeight = pEngine->GetTextHeight();
                                                nEngineWidth = (long) pEngine->CalcTextWidth();
                                                if (bPixelToLogic)
                                                    nNeededPixel = pRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
                                                else
                                                    nNeededPixel = nEngineWidth;
                                                nNeededPixel += nLeftM + nRightM;
                                            }
                                        }
                                    }
                                }

                                if ( bCellIsValue && ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip ) )
                                {
                                    pEngine->SetText( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("###")) );
                                    nEngineWidth = (long) pEngine->CalcTextWidth();
                                    if (bPixelToLogic)
                                        nNeededPixel = pRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
                                    else
                                        nNeededPixel = nEngineWidth;
                                    nNeededPixel += nLeftM + nRightM;

                                    //  No clip marks if "###" doesn't fit (same as in DrawStrings)
                                }

                                if ( eOutHorJust != SVX_HOR_JUSTIFY_LEFT && eOrient == SVX_ORIENTATION_STANDARD )
                                {
                                    aPaperSize.Width() = nNeededPixel + 1;
                                    if (bPixelToLogic)
                                        pEngine->SetPaperSize(pRefDevice->PixelToLogic(aPaperSize));
                                    else
                                        pEngine->SetPaperSize(aPaperSize);
                                }
                            }

                            long nStartX = aAreaParam.maAlignRect.Left();
                            long nStartY = aAreaParam.maAlignRect.Top();
                            long nCellWidth = aAreaParam.maAlignRect.GetWidth();
                            long nOutWidth = nCellWidth - 1 - nLeftM - nRightM;
                            long nOutHeight = aAreaParam.maAlignRect.GetHeight() - nTopM - nBottomM;

                            if ( bBreak || eOrient != SVX_ORIENTATION_STANDARD || bAsianVertical )
                            {
                                //  text with automatic breaks is aligned only within the
                                //  edit engine's paper size, the output of the whole area
                                //  is always left-aligned

                                nStartX += nLeftM;
                            }
                            else
                            {
                                if ( eOutHorJust == SVX_HOR_JUSTIFY_RIGHT )
                                    nStartX -= nNeededPixel - nCellWidth + nRightM + 1;
                                else if ( eOutHorJust == SVX_HOR_JUSTIFY_CENTER )
                                    nStartX -= ( nNeededPixel - nCellWidth + nRightM + 1 - nLeftM ) / 2;
                                else
                                    nStartX += nLeftM;
                            }

                            sal_Bool bOutside = ( aAreaParam.maClipRect.Right() < nScrX || aAreaParam.maClipRect.Left() >= nScrX + nScrW );
                            if ( aAreaParam.maClipRect.Left() < nScrX )
                            {
                                aAreaParam.maClipRect.Left() = nScrX;
                                aAreaParam.mbLeftClip = true;
                            }
                            if ( aAreaParam.maClipRect.Right() > nScrX + nScrW )
                            {
                                aAreaParam.maClipRect.Right() = nScrX + nScrW;          //! minus one?
                                aAreaParam.mbRightClip = true;
                            }

                            if ( !bHidden && !bOutside )
                            {
                                bool bClip = aAreaParam.mbLeftClip || aAreaParam.mbRightClip;
                                sal_Bool bSimClip = sal_False;

                                if ( bWrapFields )
                                {
                                    //  Fields in a cell with automatic breaks: clip to cell width
                                    bClip = sal_True;
                                }

                                if ( aAreaParam.maClipRect.Top() < nScrY )
                                {
                                    aAreaParam.maClipRect.Top() = nScrY;
                                    bClip = sal_True;
                                }
                                if ( aAreaParam.maClipRect.Bottom() > nScrY + nScrH )
                                {
                                    aAreaParam.maClipRect.Bottom() = nScrY + nScrH;     //! minus one?
                                    bClip = sal_True;
                                }

                                Size aCellSize;         // output area, excluding margins, in logical units
                                if (bPixelToLogic)
                                    aCellSize = pRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
                                else
                                    aCellSize = Size( nOutWidth, nOutHeight );

                                if ( nEngineHeight >= aCellSize.Height() + aRefOne.Height() )
                                {
                                    const ScMergeAttr* pMerge =
                                            (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                                    sal_Bool bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

                                    //  Don't clip for text height when printing rows with optimal height,
                                    //  except when font size is from conditional formatting.
                                    //! Allow clipping when vertically merged?
                                    if ( eType != OUTTYPE_PRINTER ||
                                        ( pDoc->GetRowFlags( nCellY, nTab ) & CR_MANUALSIZE ) ||
                                        ( pCondSet && SFX_ITEM_SET ==
                                            pCondSet->GetItemState(ATTR_FONT_HEIGHT, sal_True) ) )
                                        bClip = sal_True;
                                    else
                                        bSimClip = sal_True;

                                    //  Show clip marks if height is at least 5pt too small and
                                    //  there are several lines of text.
                                    //  Not for asian vertical text, because that would interfere
                                    //  with the default right position of the text.
                                    //  Only with automatic line breaks, to avoid having to find
                                    //  the cells with the horizontal end of the text again.
                                    if ( nEngineHeight - aCellSize.Height() > 100 &&
                                         ( bBreak || eOrient == SVX_ORIENTATION_STACKED ) &&
                                         !bAsianVertical && bMarkClipped &&
                                         ( pEngine->GetParagraphCount() > 1 || pEngine->GetLineCount(0) > 1 ) )
                                    {
                                        CellInfo* pClipMarkCell = NULL;
                                        if ( bMerged )
                                        {
                                            //  anywhere in the merged area...
                                            SCCOL nClipX = ( nX < nX1 ) ? nX1 : nX;
                                            pClipMarkCell = &pRowInfo[(nArrY != 0) ? nArrY : 1].pCellInfo[nClipX+1];
                                        }
                                        else
                                            pClipMarkCell = &pThisRowInfo->pCellInfo[nX+1];

                                        pClipMarkCell->nClipMark |= SC_CLIPMARK_RIGHT;      //! also allow left?
                                        bAnyClipped = sal_True;

                                        long nMarkPixel = (long)( SC_CLIPMARK_SIZE * nPPTX );
                                        if ( aAreaParam.maClipRect.Right() - nMarkPixel > aAreaParam.maClipRect.Left() )
                                            aAreaParam.maClipRect.Right() -= nMarkPixel;
                                    }
                                }

#if 0
                                long nClipStartY = nStartY;
                                if (nArrY==0 || bVisChanged)
                                {
                                    if ( nClipStartY < nRowPosY )
                                    {
                                        long nDif = nRowPosY - nClipStartY;
                                        bClip = sal_True;
                                        nClipStartY = nRowPosY;
                                        aClipSize.Height() -= nDif;
                                    }
                                }
#endif

                                Rectangle aLogicClip;
                                if (bClip || bSimClip)
                                {
                                    // Clip marks are already handled in GetOutputArea

                                    if (bPixelToLogic)
                                        aLogicClip = pRefDevice->PixelToLogic( aAreaParam.maClipRect );
                                    else
                                        aLogicClip = aAreaParam.maClipRect;

                                    if (bClip)  // bei bSimClip nur aClipRect initialisieren
                                    {
                                        if (bMetaFile)
                                        {
                                            pDev->Push();
                                            pDev->IntersectClipRegion( aLogicClip );
                                        }
                                        else
                                            pDev->SetClipRegion( Region( aLogicClip ) );
                                    }
                                }

                                Point aLogicStart;
                                if (bPixelToLogic)
                                    aLogicStart = pRefDevice->PixelToLogic( Point(nStartX,nStartY) );
                                else
                                    aLogicStart = Point(nStartX, nStartY);
                                if ( eOrient!=SVX_ORIENTATION_STANDARD || bAsianVertical || !bBreak )
                                {
                                    long nAvailWidth = aCellSize.Width();
                                    // space for AutoFilter is already handled in GetOutputArea

                                    //  horizontal alignment

                                    if (eOrient==SVX_ORIENTATION_STANDARD && !bAsianVertical)
                                    {
                                        if (eHorJust==SVX_HOR_JUSTIFY_RIGHT ||
                                            eHorJust==SVX_HOR_JUSTIFY_CENTER ||
                                            (eHorJust==SVX_HOR_JUSTIFY_STANDARD && bCellIsValue) )
                                        {
                                            pEngine->SetUpdateMode( sal_False );

                                            SvxAdjust eEditAdjust =
                                                (eHorJust==SVX_HOR_JUSTIFY_CENTER) ?
                                                    SVX_ADJUST_CENTER : SVX_ADJUST_RIGHT;
                                            pEngine->SetDefaultItem(
                                                SvxAdjustItem( eEditAdjust, EE_PARA_JUST ) );

                                            // #55142# reset adjustment for the next cell
                                            pOldPattern = NULL;

                                            pEngine->SetUpdateMode( sal_True );
                                        }
                                    }
                                    else
                                    {
                                        if (eHorJust==SVX_HOR_JUSTIFY_RIGHT)
                                            aLogicStart.X() += nAvailWidth - nEngineWidth;
                                        else if (eHorJust==SVX_HOR_JUSTIFY_CENTER)
                                            aLogicStart.X() += (nAvailWidth - nEngineWidth) / 2;
                                    }
                                }

                                if ( bAsianVertical )
                                {
                                    // paper size is subtracted below
                                    aLogicStart.X() += nEngineWidth;
                                }

                                if ( ( bAsianVertical || eOrient == SVX_ORIENTATION_TOPBOTTOM ||
                                        eOrient == SVX_ORIENTATION_BOTTOMTOP ) && bBreak )
                                {
                                    // vertical adjustment is within the EditEngine
                                    if (bPixelToLogic)
                                        aLogicStart.Y() += pRefDevice->PixelToLogic(Size(0,nTopM)).Height();
                                    else
                                        aLogicStart.Y() += nTopM;
                                }

                                if ( ( eOrient==SVX_ORIENTATION_STANDARD && !bAsianVertical ) ||
                                     eOrient==SVX_ORIENTATION_STACKED || !bBreak )
                                {
                                    if (eVerJust==SVX_VER_JUSTIFY_BOTTOM ||
                                        eVerJust==SVX_VER_JUSTIFY_STANDARD)
                                    {
                                        //! if pRefDevice != pFmtDevice, keep heights in logic units,
                                        //! only converting margin?

                                        if (bPixelToLogic)
                                            aLogicStart.Y() += pRefDevice->PixelToLogic( Size(0, nTopM +
                                                            pRefDevice->LogicToPixel(aCellSize).Height() -
                                                            pRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height()
                                                            )).Height();
                                        else
                                            aLogicStart.Y() += nTopM + aCellSize.Height() - nEngineHeight;
                                    }
                                    else if (eVerJust==SVX_VER_JUSTIFY_CENTER)
                                    {
                                        if (bPixelToLogic)
                                            aLogicStart.Y() += pRefDevice->PixelToLogic( Size(0, nTopM + (
                                                            pRefDevice->LogicToPixel(aCellSize).Height() -
                                                            pRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height() )
                                                            / 2)).Height();
                                        else
                                            aLogicStart.Y() += nTopM + (aCellSize.Height() - nEngineHeight) / 2;
                                    }
                                    else        // top
                                    {
                                        if (bPixelToLogic)
                                            aLogicStart.Y() += pRefDevice->PixelToLogic(Size(0,nTopM)).Height();
                                        else
                                            aLogicStart.Y() += nTopM;
                                    }
                                }

                                Point aURLStart = aLogicStart;      // copy before modifying for orientation

                                short nOriVal = 0;
                                if (eOrient==SVX_ORIENTATION_TOPBOTTOM)
                                {
                                    // nOriVal = -900;
                                    nOriVal = 2700;
                                    aLogicStart.X() += nEngineWidth;
                                }
                                else if (eOrient==SVX_ORIENTATION_BOTTOMTOP)
                                {
                                    nOriVal = 900;
                                    aLogicStart.Y() += bBreak ? pEngine->GetPaperSize().Width() :
                                                                nEngineHeight;
                                }
                                else if (eOrient==SVX_ORIENTATION_STACKED)
                                {
                                    Size aPaperLogic = pEngine->GetPaperSize();
                                    aPaperLogic.Width() = nEngineWidth;
                                    pEngine->SetPaperSize(aPaperLogic);
                                }

                                if ( pEngine->IsRightToLeft( 0 ) )
                                {
                                    //  For right-to-left, EditEngine always calculates its lines
                                    //  beginning from the right edge, but EditLine::nStartPosX is
                                    //  of sal_uInt16 type, so the PaperSize must be limited to USHRT_MAX.
                                    Size aLogicPaper = pEngine->GetPaperSize();
                                    if ( aLogicPaper.Width() > USHRT_MAX )
                                    {
                                        aLogicPaper.Width() = USHRT_MAX;
                                        pEngine->SetPaperSize(aLogicPaper);
                                    }
                                }

                                // bMoveClipped handling has been replaced by complete alignment
                                // handling (also extending to the left).

                                if ( bSimClip && !nOriVal && !bAsianVertical )
                                {
                                    //  kein hartes Clipping, aber nur die betroffenen
                                    //  Zeilen ausgeben

                                    Point aDocStart = aLogicClip.TopLeft();
                                    aDocStart -= aLogicStart;
                                    pEngine->Draw( pDev, aLogicClip, aDocStart, sal_False );
                                }
                                else
                                {
                                    if (bAsianVertical)
                                    {
                                        //  with SetVertical, the start position is top left of
                                        //  the whole output area, not the text itself
                                        aLogicStart.X() -= pEngine->GetPaperSize().Width();
                                    }
                                    pEngine->Draw( pDev, aLogicStart, nOriVal );
                                }

                                if (bClip)
                                {
                                    if (bMetaFile)
                                        pDev->Pop();
                                    else
                                        pDev->SetClipRegion();
                                }

                                // PDF: whole-cell hyperlink from formula?
                                sal_Bool bHasURL = pPDFData && pCell && pCell->GetCellType() == CELLTYPE_FORMULA &&
                                                static_cast<ScFormulaCell*>(pCell)->IsHyperLinkCell();
                                if ( bHasURL )
                                {
                                    long nURLWidth = (long) pEngine->CalcTextWidth();
                                    long nURLHeight = pEngine->GetTextHeight();
                                    if ( bBreak )
                                    {
                                        Size aPaper = pEngine->GetPaperSize();
                                        if ( bAsianVertical )
                                            nURLHeight = aPaper.Height();
                                        else
                                            nURLWidth = aPaper.Width();
                                    }
                                    if ( eOrient == SVX_ORIENTATION_TOPBOTTOM || eOrient == SVX_ORIENTATION_BOTTOMTOP )
                                        std::swap( nURLWidth, nURLHeight );
                                    else if ( bAsianVertical )
                                        aURLStart.X() -= nURLWidth;

                                    Rectangle aURLRect( aURLStart, Size( nURLWidth, nURLHeight ) );
                                    lcl_DoHyperlinkResult( pDev, aURLRect, pCell );
                                }
                            }
                        }
                    }
                }
                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
            }
        }
        nRowPosY += pRowInfo[nArrY].nHeight;
    }

    delete pEngine;

    if (bAnyRotated)
        DrawRotated(bPixelToLogic);     //! von aussen rufen ?
}

//  -------------------------------------------------------------------------------

void ScOutputData::DrawRotated(sal_Bool bPixelToLogic)
{
    //! nRotMax speichern
    SCCOL nRotMax = nX2;
    for (SCSIZE nRotY=0; nRotY<nArrCount; nRotY++)
        if (pRowInfo[nRotY].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nRotY].nRotMaxCol > nRotMax)
            nRotMax = pRowInfo[nRotY].nRotMaxCol;


    ScModule* pScMod = SC_MOD();
    sal_Int32 nConfBackColor = pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed sal_True)
    sal_Bool bCellContrast = bUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    ScFieldEditEngine* pEngine = NULL;
    sal_Bool bHyphenatorSet = sal_False;
    const ScPatternAttr* pPattern;
    const SfxItemSet*    pCondSet;
    const ScPatternAttr* pOldPattern = NULL;
    const SfxItemSet*    pOldCondSet = NULL;
    ScBaseCell* pCell = NULL;

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
#if 0
        Size aOnePixel = pDev->PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        nInitPosX += nMirrorW - nOneX;
#endif
        nInitPosX += nMirrorW - 1;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    long nRowPosY = nScrY;
    for (SCSIZE nArrY=0; nArrY+1<nArrCount; nArrY++)            // 0 fuer Reste von zusammengefassten
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nCellHeight = (long) pThisRowInfo->nHeight;
        if (nArrY==1) nRowPosY = nScrY;                         // vorher wird einzeln berechnet

        if ( ( pThisRowInfo->bChanged || nArrY==0 ) && pThisRowInfo->nRotMaxCol != SC_ROTMAX_NONE )
        {
            long nPosX = 0;
            for (SCCOL nX=0; nX<=nRotMax; nX++)
            {
                if (nX==nX1) nPosX = nInitPosX;                 // positions before nX1 are calculated individually

                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                if ( pInfo->nRotateDir != SC_ROTDIR_NONE )
                {
                    SCROW nY = pThisRowInfo->nRowNo;

                    sal_Bool bHidden = sal_False;
                    if (bEditMode)
                        if ( nX == nEditCol && nY == nEditRow )
                            bHidden = sal_True;

                    if (!bHidden)
                    {
                        if (!pEngine)
                        {
                            //  Ein RefDevice muss auf jeden Fall gesetzt werden,
                            //  sonst legt sich die EditEngine ein VirtualDevice an!
                            pEngine = new ScFieldEditEngine( pDoc->GetEnginePool() );
                            pEngine->SetUpdateMode( sal_False );
                            pEngine->SetRefDevice( pFmtDevice );    // always set
                            sal_uLong nCtrl = pEngine->GetControlWord();
                            if ( bShowSpellErrors )
                                nCtrl |= EE_CNTRL_ONLINESPELLING;
                            if ( eType == OUTTYPE_PRINTER )
                                nCtrl &= ~EE_CNTRL_MARKFIELDS;
                            pEngine->SetControlWord( nCtrl );
                            pEngine->SetForbiddenCharsTable( pDoc->GetForbiddenCharacters() );
                            pEngine->SetAsianCompressionMode( pDoc->GetAsianCompression() );
                            pEngine->SetKernAsianPunctuation( pDoc->GetAsianKerning() );
                            pEngine->EnableAutoColor( bUseStyleColor );
                            pEngine->SetDefaultHorizontalTextDirection(
                                (EEHorizontalTextDirection)pDoc->GetEditTextDirection( nTab ) );
                        }
                        else
                            lcl_ClearEdit( *pEngine );      // also calls SetUpdateMode(sal_False)

                        long nPosY = nRowPosY;
                        sal_Bool bVisChanged = sal_False;

                        //! Rest von zusammengefasster Zelle weiter oben funktioniert nicht!

                        sal_Bool bFromDoc = sal_False;
                        pPattern = pInfo->pPatternAttr;
                        pCondSet = pInfo->pConditionSet;
                        if (!pPattern)
                        {
                            pPattern = pDoc->GetPattern( nX, nY, nTab );
                            bFromDoc = sal_True;
                        }
                        pCell = pInfo->pCell;
                        if (bFromDoc)
                            pCondSet = pDoc->GetCondResult( nX, nY, nTab );

                        if (!pCell && nX>nX2)
                            GetVisibleCell( nX, nY, nTab, pCell );

                        if ( !pCell || IsEmptyCellText( pThisRowInfo, nX, nY ) )
                            bHidden = sal_True;     // nRotateDir is also set without a cell

                        long nCellWidth = (long) pRowInfo[0].pCellInfo[nX+1].nWidth;

                        SvxCellHorJustify eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                            pPattern->GetItem(ATTR_HOR_JUSTIFY, pCondSet)).GetValue();
                        sal_Bool bBreak = ( eHorJust == SVX_HOR_JUSTIFY_BLOCK ) ||
                                    ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK, pCondSet)).GetValue();
                        sal_Bool bRepeat = ( eHorJust == SVX_HOR_JUSTIFY_REPEAT && !bBreak );
                        sal_Bool bShrink = !bBreak && !bRepeat && static_cast<const SfxBoolItem&>
                                        (pPattern->GetItem( ATTR_SHRINKTOFIT, pCondSet )).GetValue();
                        SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );

                        const ScMergeAttr* pMerge =
                                (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                        sal_Bool bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

                        long nStartX = nPosX;
                        long nStartY = nPosY;
                        if (nX<nX1)
                        {
                            if ((bBreak || eOrient!=SVX_ORIENTATION_STANDARD) && !bMerged)
                                bHidden = sal_True;
                            else
                            {
                                nStartX = nInitPosX;
                                SCCOL nCol = nX1;
                                while (nCol > nX)
                                {
                                    --nCol;
                                    nStartX -= nLayoutSign * (long) pRowInfo[0].pCellInfo[nCol+1].nWidth;
                                }
                            }
                        }
                        long nCellStartX = nStartX;

                        //  Ersatzdarstellung fuer zu kleinen Text weggelassen

                        if (!bHidden)
                        {
                            long nOutWidth = nCellWidth - 1;
                            long nOutHeight;
                            if (pInfo)
                                nOutHeight = nCellHeight;
                            else
                                nOutHeight = (long) ( pDoc->GetRowHeight(nY,nTab) * nPPTY );

                            if ( bMerged )                              // Zusammengefasst
                            {
                                SCCOL nCountX = pMerge->GetColMerge();
                                for (SCCOL i=1; i<nCountX; i++)
                                    nOutWidth += (long) ( pDoc->GetColWidth(nX+i,nTab) * nPPTX );
                                SCROW nCountY = pMerge->GetRowMerge();
                                nOutHeight += (long) pDoc->GetScaledRowHeight( nY+1, nY+nCountY-1, nTab, nPPTY);
                            }

                            SvxCellVerJustify eVerJust = (SvxCellVerJustify)((const SvxVerJustifyItem&)
                                                pPattern->GetItem(ATTR_VER_JUSTIFY, pCondSet)).GetValue();

                            // Syntax-Modus wird hier ignoriert...

                            // StringDiffer doesn't look at hyphenate, language items
                            if ( pPattern != pOldPattern || pCondSet != pOldCondSet )
                            {
                                SfxItemSet* pSet = new SfxItemSet( pEngine->GetEmptyItemSet() );
                                pPattern->FillEditItemSet( pSet, pCondSet );

                                                                    // Ausrichtung fuer EditEngine
                                SvxAdjust eSvxAdjust = SVX_ADJUST_LEFT;
                                if (eOrient==SVX_ORIENTATION_STACKED)
                                    eSvxAdjust = SVX_ADJUST_CENTER;
                                // Adjustment fuer bBreak ist hier weggelassen
                                pSet->Put( SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );

                                pEngine->SetDefaults( pSet );
                                pOldPattern = pPattern;
                                pOldCondSet = pCondSet;

                                sal_uLong nControl = pEngine->GetControlWord();
                                if (eOrient==SVX_ORIENTATION_STACKED)
                                    nControl |= EE_CNTRL_ONECHARPERLINE;
                                else
                                    nControl &= ~EE_CNTRL_ONECHARPERLINE;
                                pEngine->SetControlWord( nControl );

                                if ( !bHyphenatorSet && ((const SfxBoolItem&)pSet->Get(EE_PARA_HYPHENATE)).GetValue() )
                                {
                                    //  set hyphenator the first time it is needed
                                    com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
                                    pEngine->SetHyphenator( xXHyphenator );
                                    bHyphenatorSet = sal_True;
                                }

                                Color aBackCol = ((const SvxBrushItem&)
                                    pPattern->GetItem( ATTR_BACKGROUND, pCondSet )).GetColor();
                                if ( bUseStyleColor && ( aBackCol.GetTransparency() > 0 || bCellContrast ) )
                                    aBackCol.SetColor( nConfBackColor );
                                pEngine->SetBackgroundColor( aBackCol );
                            }

                            //  Raender

                            //!     Position und Papersize auf EditUtil umstellen !!!

                            const SvxMarginItem* pMargin = (const SvxMarginItem*)
                                                    &pPattern->GetItem(ATTR_MARGIN, pCondSet);
                            sal_uInt16 nIndent = 0;
                            if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
                                nIndent = ((const SfxUInt16Item&)pPattern->
                                                    GetItem(ATTR_INDENT, pCondSet)).GetValue();

                            long nTotalHeight = nOutHeight; // ohne Rand abzuziehen
                            if ( bPixelToLogic )
                                nTotalHeight = pRefDevice->PixelToLogic(Size(0,nTotalHeight)).Height();

                            long nLeftM = (long) ( (pMargin->GetLeftMargin() + nIndent) * nPPTX );
                            long nTopM  = (long) ( pMargin->GetTopMargin() * nPPTY );
                            long nRightM  = (long) ( pMargin->GetRightMargin() * nPPTX );
                            long nBottomM = (long) ( pMargin->GetBottomMargin() * nPPTY );
                            nStartX += nLeftM;
                            nStartY += nTopM;
                            nOutWidth -= nLeftM + nRightM;
                            nOutHeight -= nTopM + nBottomM;

                            //  Rotation schon hier, um bei Umbruch auch PaperSize anzupassen
                            long nAttrRotate = 0;
                            double nSin = 0.0;
                            double nCos = 1.0;
                            SvxRotateMode eRotMode = SVX_ROTATE_MODE_STANDARD;
                            if ( eOrient == SVX_ORIENTATION_STANDARD )
                            {
                                nAttrRotate = ((const SfxInt32Item&)pPattern->
                                                    GetItem(ATTR_ROTATE_VALUE, pCondSet)).GetValue();
                                if ( nAttrRotate )
                                {
                                    eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
                                                pPattern->GetItem(ATTR_ROTATE_MODE, pCondSet)).GetValue();

                                    if ( nAttrRotate == 18000 )
                                        eRotMode = SVX_ROTATE_MODE_STANDARD;    // keinen Ueberlauf

                                    if ( bLayoutRTL )
                                        nAttrRotate = -nAttrRotate;

                                    double nRealOrient = nAttrRotate * F_PI18000;   // 1/100 Grad
                                    nCos = cos( nRealOrient );
                                    nSin = sin( nRealOrient );
                                }
                            }

                            Size aPaperSize = Size( 1000000, 1000000 );
                            if (eOrient==SVX_ORIENTATION_STACKED)
                                aPaperSize.Width() = nOutWidth;             // zum Zentrieren
                            else if (bBreak)
                            {
                                if (nAttrRotate)
                                {
                                    //! richtige PaperSize fuer Umbruch haengt von der Zeilenzahl
                                    //! ab, solange die Zeilen nicht einzeln versetzt ausgegeben
                                    //! werden koennen -> darum unbegrenzt, also kein Umbruch.
                                    //! Mit versetzten Zeilen waere das folgende richtig:
                                    aPaperSize.Width() = (long)(nOutHeight / fabs(nSin));
                                }
                                else if (eOrient == SVX_ORIENTATION_STANDARD)
                                    aPaperSize.Width() = nOutWidth;
                                else
                                    aPaperSize.Width() = nOutHeight - 1;
                            }
                            if (bPixelToLogic)
                                pEngine->SetPaperSize(pRefDevice->PixelToLogic(aPaperSize));
                            else
                                pEngine->SetPaperSize(aPaperSize);  // Scale ist immer 1

                            //  Daten aus Zelle lesen

                            if (pCell)
                            {
                                if (pCell->GetCellType() == CELLTYPE_EDIT)
                                {
                                    const EditTextObject* pData;
                                    ((ScEditCell*)pCell)->GetData(pData);

                                    if (pData)
                                        pEngine->SetText(*pData);
                                    else
                                    {
                                        DBG_ERROR("pData == 0");
                                    }
                                }
                                else
                                {
                                    sal_uLong nFormat = pPattern->GetNumberFormat(
                                                                pDoc->GetFormatTable(), pCondSet );
                                    String aString;
                                    Color* pColor;
                                    ScCellFormat::GetString( pCell,
                                                             nFormat,aString, &pColor,
                                                             *pDoc->GetFormatTable(),
                                                             bShowNullValues,
                                                             bShowFormulas,
                                                             ftCheck );

                                    pEngine->SetText(aString);
                                    if ( pColor && !bSyntaxMode && !( bUseStyleColor && bForceAutoColor ) )
                                        lcl_SetEditColor( *pEngine, *pColor );
                                }

                                if ( bSyntaxMode )
                                    SetEditSyntaxColor( *pEngine, pCell );
                                else if ( bUseStyleColor && bForceAutoColor )
                                    lcl_SetEditColor( *pEngine, COL_AUTO );     //! or have a flag at EditEngine
                            }
                            else
                            {
                                DBG_ERROR("pCell == NULL");
                            }

                            pEngine->SetUpdateMode( sal_True );     // after SetText, before CalcTextWidth/GetTextHeight

                            long nEngineWidth  = (long) pEngine->CalcTextWidth();
                            long nEngineHeight = pEngine->GetTextHeight();

                            if (nAttrRotate && bBreak)
                            {
                                double nAbsCos = fabs( nCos );
                                double nAbsSin = fabs( nSin );

                                // #47740# adjust witdh of papersize for height of text
                                int nSteps = 5;
                                while (nSteps > 0)
                                {
                                    // everything is in pixels
                                    long nEnginePixel = pRefDevice->LogicToPixel(
                                                            Size(0,nEngineHeight)).Height();
                                    long nEffHeight = nOutHeight - (long)(nEnginePixel * nAbsCos) + 2;
                                    long nNewWidth = (long)(nEffHeight / nAbsSin) + 2;
                                    sal_Bool bFits = ( nNewWidth >= aPaperSize.Width() );
                                    if ( bFits )
                                        nSteps = 0;
                                    else
                                    {
                                        if ( nNewWidth < 4 )
                                        {
                                            // can't fit -> fall back to using half height
                                            nEffHeight = nOutHeight / 2;
                                            nNewWidth = (long)(nEffHeight / nAbsSin) + 2;
                                            nSteps = 0;
                                        }
                                        else
                                            --nSteps;

                                        // set paper width and get new text height
                                        aPaperSize.Width() = nNewWidth;
                                        if (bPixelToLogic)
                                            pEngine->SetPaperSize(pRefDevice->PixelToLogic(aPaperSize));
                                        else
                                            pEngine->SetPaperSize(aPaperSize);  // Scale ist immer 1
                                        //pEngine->QuickFormatDoc( sal_True );
                                        nEngineWidth  = (long) pEngine->CalcTextWidth();
                                        nEngineHeight = pEngine->GetTextHeight();
                                    }
                                }
                            }

                            long nRealWidth  = nEngineWidth;
                            long nRealHeight = nEngineHeight;

                            //  wenn gedreht, Groesse anpassen
                            if (nAttrRotate)
                            {
                                double nAbsCos = fabs( nCos );
                                double nAbsSin = fabs( nSin );

                                if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                                    nEngineWidth = (long) ( nRealWidth * nAbsCos +
                                                            nRealHeight * nAbsSin );
                                else
                                    nEngineWidth = (long) ( nRealHeight / nAbsSin );
                                //! begrenzen !!!

                                nEngineHeight = (long) ( nRealHeight * nAbsCos +
                                                         nRealWidth * nAbsSin );
                            }

                            if (!nAttrRotate)           //  hier nur gedrehter Text
                                bHidden = sal_True;         //! vorher abfragen !!!

                            //! weglassen, was nicht hereinragt

                            if (!bHidden)
                            {
                                sal_Bool bClip = sal_False;
                                Size aClipSize = Size( nScrX+nScrW-nStartX, nScrY+nScrH-nStartY );

                                //  weiterschreiben

                                Size aCellSize;
                                if (bPixelToLogic)
                                    aCellSize = pRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
                                else
                                    aCellSize = Size( nOutWidth, nOutHeight );  // Scale ist 1

                                long nGridWidth = nEngineWidth;
                                sal_Bool bNegative = sal_False;
                                if ( eRotMode != SVX_ROTATE_MODE_STANDARD )
                                {
                                    nGridWidth = aCellSize.Width() +
                                            Abs((long) ( aCellSize.Height() * nCos / nSin ));
                                    bNegative = ( pInfo->nRotateDir == SC_ROTDIR_LEFT );
                                    if ( bLayoutRTL )
                                        bNegative = !bNegative;
                                }

                                // use GetOutputArea to hide the grid
                                // (clip region is done manually below)
                                OutputAreaParam aAreaParam;

                                SCCOL nCellX = nX;
                                SCROW nCellY = nY;
                                SvxCellHorJustify eOutHorJust = eHorJust;
                                if ( eRotMode != SVX_ROTATE_MODE_STANDARD )
                                    eOutHorJust = bNegative ? SVX_HOR_JUSTIFY_RIGHT : SVX_HOR_JUSTIFY_LEFT;
                                long nNeededWidth = nGridWidth;     // in pixel for GetOutputArea
                                if ( bPixelToLogic )
                                    nNeededWidth =  pRefDevice->LogicToPixel(Size(nNeededWidth,0)).Width();

                                GetOutputArea( nX, nArrY, nCellStartX, nPosY, nCellX, nCellY, nNeededWidth,
                                                *pPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                                                sal_False, sal_False, sal_True, aAreaParam );

                                if ( bShrink )
                                {
                                    long nPixelWidth = bPixelToLogic ?
                                        pRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width() : nEngineWidth;
                                    long nNeededPixel = nPixelWidth + nLeftM + nRightM;

                                    aAreaParam.mbLeftClip = aAreaParam.mbRightClip = sal_True;

                                    // always do height
                                    ShrinkEditEngine( *pEngine, aAreaParam.maAlignRect, nLeftM, nTopM, nRightM, nBottomM,
                                        sal_False, sal::static_int_cast<sal_uInt16>(eOrient), nAttrRotate, bPixelToLogic,
                                        nEngineWidth, nEngineHeight, nNeededPixel, aAreaParam.mbLeftClip, aAreaParam.mbRightClip );

                                    if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                                    {
                                        // do width only if rotating within the cell (standard mode)
                                        ShrinkEditEngine( *pEngine, aAreaParam.maAlignRect, nLeftM, nTopM, nRightM, nBottomM,
                                            sal_True, sal::static_int_cast<sal_uInt16>(eOrient), nAttrRotate, bPixelToLogic,
                                            nEngineWidth, nEngineHeight, nNeededPixel, aAreaParam.mbLeftClip, aAreaParam.mbRightClip );
                                    }

                                    // nEngineWidth/nEngineHeight is updated in ShrinkEditEngine
                                    // (but width is only valid for standard mode)
                                    nRealWidth  = (long) pEngine->CalcTextWidth();
                                    nRealHeight = pEngine->GetTextHeight();

                                    if ( eRotMode != SVX_ROTATE_MODE_STANDARD )
                                        nEngineWidth = (long) ( nRealHeight / fabs( nSin ) );
                                }

                                // sal_Bool bVClip = ( nEngineHeight > aCellSize.Height() );

                                long nClipStartX = nStartX;
                                if (nX<nX1)
                                {
                                    //! Clipping unnoetig, wenn links am Fenster

                                    bClip = sal_True;                   // nur Rest ausgeben!
                                    if (nStartX<nScrX)
                                    {
                                        long nDif = nScrX - nStartX;
                                        nClipStartX = nScrX;
                                        aClipSize.Width() -= nDif;
                                    }
                                }

                                long nClipStartY = nStartY;
                                if (nArrY==0 || bVisChanged)
                                {
                                    if ( nClipStartY < nRowPosY )
                                    {
                                        long nDif = nRowPosY - nClipStartY;
                                        bClip = sal_True;
                                        nClipStartY = nRowPosY;
                                        aClipSize.Height() -= nDif;
                                    }
                                }

                                bClip = sal_True;       // always clip at the window/page border

                                //Rectangle aClipRect;
                                if (bClip)
                                {
                                    if ( nAttrRotate /* && eRotMode != SVX_ROTATE_MODE_STANDARD */ )
                                    {
                                        //  gedrehten, ausgerichteten Text nur an den
                                        //  Seitengrenzen clippen
                                        nClipStartX = nScrX;
                                        aClipSize.Width() = nScrW;
                                    }

                                    if (bPixelToLogic)
                                        aAreaParam.maClipRect = pRefDevice->PixelToLogic( Rectangle(
                                                        Point(nClipStartX,nClipStartY), aClipSize ) );
                                    else
                                        aAreaParam.maClipRect = Rectangle(Point(nClipStartX, nClipStartY),
                                                                aClipSize );    // Scale = 1

                                    if (bMetaFile)
                                    {
                                        pDev->Push();
                                        pDev->IntersectClipRegion( aAreaParam.maClipRect );
                                    }
                                    else
                                        pDev->SetClipRegion( Region( aAreaParam.maClipRect ) );
                                }

                                Point aLogicStart;
                                if (bPixelToLogic)
                                    aLogicStart = pRefDevice->PixelToLogic( Point(nStartX,nStartY) );
                                else
                                    aLogicStart = Point(nStartX, nStartY);
                                if ( eOrient!=SVX_ORIENTATION_STANDARD || !bBreak )
                                {
                                    long nAvailWidth = aCellSize.Width();
                                    if (eType==OUTTYPE_WINDOW &&
                                            eOrient!=SVX_ORIENTATION_STACKED &&
                                            pInfo && pInfo->bAutoFilter)
                                    {
                                        // filter drop-down width is now independent from row height
                                        if (bPixelToLogic)
                                            nAvailWidth -= pRefDevice->PixelToLogic(Size(0,DROPDOWN_BITMAP_SIZE)).Height();
                                        else
                                            nAvailWidth -= DROPDOWN_BITMAP_SIZE;
                                        long nComp = nEngineWidth;
                                        if (nAvailWidth<nComp) nAvailWidth=nComp;
                                    }

                                    //  horizontale Ausrichtung

                                    if (eOrient==SVX_ORIENTATION_STANDARD && !nAttrRotate)
                                    {
                                        if (eHorJust==SVX_HOR_JUSTIFY_RIGHT ||
                                            eHorJust==SVX_HOR_JUSTIFY_CENTER)
                                        {
                                            pEngine->SetUpdateMode( sal_False );

                                            SvxAdjust eSvxAdjust =
                                                (eHorJust==SVX_HOR_JUSTIFY_RIGHT) ?
                                                    SVX_ADJUST_RIGHT : SVX_ADJUST_CENTER;
                                            pEngine->SetDefaultItem(
                                                SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );

                                            aPaperSize.Width() = nOutWidth;
                                            if (bPixelToLogic)
                                                pEngine->SetPaperSize(pRefDevice->PixelToLogic(aPaperSize));
                                            else
                                                pEngine->SetPaperSize(aPaperSize);

                                            pEngine->SetUpdateMode( sal_True );
                                        }
                                    }
                                    else
                                    {
                                        //  bei gedrehtem Text ist Standard zentriert
                                        if (eHorJust==SVX_HOR_JUSTIFY_RIGHT)
                                            aLogicStart.X() += nAvailWidth - nEngineWidth;
                                        else if (eHorJust==SVX_HOR_JUSTIFY_CENTER ||
                                                 eHorJust==SVX_HOR_JUSTIFY_STANDARD)
                                            aLogicStart.X() += (nAvailWidth - nEngineWidth) / 2;
                                    }
                                }

                                if ( bLayoutRTL )
                                {
                                    if (bPixelToLogic)
                                        aLogicStart.X() -= pRefDevice->PixelToLogic(
                                                        Size( nCellWidth, 0 ) ).Width();
                                    else
                                        aLogicStart.X() -= nCellWidth;
                                }

                                if ( eOrient==SVX_ORIENTATION_STANDARD ||
                                     eOrient==SVX_ORIENTATION_STACKED || !bBreak )
                                {
                                    if (eVerJust==SVX_VER_JUSTIFY_BOTTOM ||
                                        eVerJust==SVX_VER_JUSTIFY_STANDARD)
                                    {
                                        if (bPixelToLogic)
                                            aLogicStart.Y() += pRefDevice->PixelToLogic( Size(0,
                                                            pRefDevice->LogicToPixel(aCellSize).Height() -
                                                            pRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height()
                                                            )).Height();
                                        else
                                            aLogicStart.Y() += aCellSize.Height() - nEngineHeight;
                                    }

                                    else if (eVerJust==SVX_VER_JUSTIFY_CENTER)
                                    {
                                        if (bPixelToLogic)
                                            aLogicStart.Y() += pRefDevice->PixelToLogic( Size(0,(
                                                            pRefDevice->LogicToPixel(aCellSize).Height() -
                                                            pRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height())
                                                            / 2)).Height();
                                        else
                                            aLogicStart.Y() += (aCellSize.Height() - nEngineHeight) / 2;
                                    }
                                }

                                // TOPBOTTON and BOTTOMTOP are handled in DrawStrings/DrawEdit
                                DBG_ASSERT( eOrient == SVX_ORIENTATION_STANDARD && nAttrRotate,
                                            "DrawRotated: no rotation" );

                                long nOriVal = 0;
                                if ( nAttrRotate )
                                {
                                    // Attribut ist 1/100, Font 1/10 Grad
                                    nOriVal = nAttrRotate / 10;

                                    double nAddX = 0.0;
                                    double nAddY = 0.0;
                                    if ( nCos > 0.0 && eRotMode != SVX_ROTATE_MODE_STANDARD )
                                    {
                                        //! begrenzen !!!
                                        double nH = nRealHeight * nCos;
                                        nAddX += nH * ( nCos / fabs(nSin) );
                                    }
                                    if ( nCos < 0.0 && eRotMode == SVX_ROTATE_MODE_STANDARD )
                                        nAddX -= nRealWidth * nCos;
                                    if ( nSin < 0.0 )
                                        nAddX -= nRealHeight * nSin;
                                    if ( nSin > 0.0 )
                                        nAddY += nRealWidth * nSin;
                                    if ( nCos < 0.0 )
                                        nAddY -= nRealHeight * nCos;

                                    if ( eRotMode != SVX_ROTATE_MODE_STANDARD )
                                    {
                                        //! begrenzen !!!
                                        double nSkew = nTotalHeight * nCos / fabs(nSin);
                                        if ( eRotMode == SVX_ROTATE_MODE_CENTER )
                                            nAddX -= nSkew * 0.5;
                                        if ( ( eRotMode == SVX_ROTATE_MODE_TOP && nSin > 0.0 ) ||
                                             ( eRotMode == SVX_ROTATE_MODE_BOTTOM && nSin < 0.0 ) )
                                            nAddX -= nSkew;

                                        long nUp = 0;
                                        if ( eVerJust == SVX_VER_JUSTIFY_CENTER )
                                            nUp = ( aCellSize.Height() - nEngineHeight ) / 2;
                                        else if ( eVerJust == SVX_VER_JUSTIFY_TOP )
                                        {
                                            if ( nSin > 0.0 )
                                                nUp = aCellSize.Height() - nEngineHeight;
                                        }
                                        else    // BOTTOM / STANDARD
                                        {
                                            if ( nSin < 0.0 )
                                                nUp = aCellSize.Height() - nEngineHeight;
                                        }
                                        if ( nUp )
                                            nAddX += ( nUp * nCos / fabs(nSin) );
                                    }

                                    aLogicStart.X() += (long) nAddX;
                                    aLogicStart.Y() += (long) nAddY;
                                }

                                //  bSimClip is not used here (because nOriVal is set)

                                if ( pEngine->IsRightToLeft( 0 ) )
                                {
                                    //  For right-to-left, EditEngine always calculates its lines
                                    //  beginning from the right edge, but EditLine::nStartPosX is
                                    //  of sal_uInt16 type, so the PaperSize must be limited to USHRT_MAX.
                                    Size aLogicPaper = pEngine->GetPaperSize();
                                    if ( aLogicPaper.Width() > USHRT_MAX )
                                    {
                                        aLogicPaper.Width() = USHRT_MAX;
                                        pEngine->SetPaperSize(aLogicPaper);
                                    }
                                }

                                pEngine->Draw( pDev, aLogicStart, (short)nOriVal );

                                if (bClip)
                                {
                                    if (bMetaFile)
                                        pDev->Pop();
                                    else
                                        pDev->SetClipRegion();
                                }
                            }
                        }
                    }
                }
                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
            }
        }
        nRowPosY += pRowInfo[nArrY].nHeight;
    }

    delete pEngine;
}



