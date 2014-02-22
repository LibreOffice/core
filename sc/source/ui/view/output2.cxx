/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/adjustitem.hxx>
#include <svx/algitem.hxx>
#include <editeng/brushitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/fontitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/settings.hxx>

#include "output.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "editutil.hxx"
#include "progress.hxx"
#include "scmod.hxx"
#include "fillinfo.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "markdata.hxx"
#include "stlsheet.hxx"
#include "spellcheckcontext.hxx"

#include <com/sun/star/i18n/DirectionProperty.hpp>
#include <comphelper/string.hxx>

#include <boost/ptr_container/ptr_vector.hpp>

#include <math.h>

using namespace com::sun::star;


#define DROPDOWN_BITMAP_SIZE        18

#define DRAWTEXT_MAX    32767

const sal_uInt16 SC_SHRINKAGAIN_MAX = 7;






class ScDrawStringsVars
{
    ScOutputData*       pOutput;                

    const ScPatternAttr* pPattern;              
    const SfxItemSet*   pCondSet;               

    Font                aFont;                  
    FontMetric          aMetric;
    long                nAscentPixel;           
    SvxCellOrientation  eAttrOrient;
    SvxCellHorJustify   eAttrHorJust;
    SvxCellVerJustify   eAttrVerJust;
    SvxCellJustifyMethod eAttrHorJustMethod;
    SvxCellJustifyMethod eAttrVerJustMethod;
    const SvxMarginItem* pMargin;
    sal_uInt16          nIndent;
    bool                bRotated;

    OUString            aString;                
    Size                aTextSize;
    long                nOriginalWidth;
    long                nMaxDigitWidth;
    long                nSignWidth;
    long                nDotWidth;
    long                nExpWidth;

    ScRefCellValue      maLastCell;
    sal_uLong           nValueFormat;
    bool                bLineBreak;
    bool                bRepeat;
    bool                bShrink;

    bool                bPixelToLogic;
    bool                bCellContrast;

    Color               aBackConfigColor;       
    Color               aTextConfigColor;
    sal_Int32           nPos;
    sal_Unicode         nChar;

public:
                ScDrawStringsVars(ScOutputData* pData, bool bPTL);
                ~ScDrawStringsVars();

                
                

    void SetPattern(
        const ScPatternAttr* pNew, const SfxItemSet* pSet, const ScRefCellValue& rCell,
        sal_uInt8 nScript );

    void        SetPatternSimple( const ScPatternAttr* pNew, const SfxItemSet* pSet );

    bool SetText( ScRefCellValue& rCell );   
    void        SetHashText();
    void SetTextToWidthOrHash( ScRefCellValue& rCell, long nWidth );
    void        SetAutoText( const OUString& rAutoText );

    const ScPatternAttr*    GetPattern() const       { return pPattern; }
    SvxCellOrientation      GetOrient() const        { return eAttrOrient; }
    SvxCellHorJustify       GetHorJust() const       { return eAttrHorJust; }
    SvxCellVerJustify       GetVerJust() const       { return eAttrVerJust; }
    SvxCellJustifyMethod    GetHorJustMethod() const { return eAttrHorJustMethod; }
    SvxCellJustifyMethod    GetVerJustMethod() const { return eAttrVerJustMethod; }
    const SvxMarginItem*    GetMargin() const        { return pMargin; }

    sal_uInt16              GetLeftTotal() const     { return pMargin->GetLeftMargin() + nIndent; }
    sal_uInt16              GetRightTotal() const    { return pMargin->GetRightMargin() + nIndent; }

    const OUString&         GetString() const        { return aString; }
    const Size&             GetTextSize() const      { return aTextSize; }
    long                    GetOriginalWidth() const { return nOriginalWidth; }

    sal_uLong GetResultValueFormat() const;

    sal_uLong   GetValueFormat() const                  { return nValueFormat; }
    bool    GetLineBreak() const                    { return bLineBreak; }
    bool    IsRepeat() const                        { return bRepeat; }
    bool    IsShrink() const                        { return bShrink; }
    void        RepeatToFill( long colWidth );

    long    GetAscent() const   { return nAscentPixel; }
    bool    IsRotated() const   { return bRotated; }

    void    SetShrinkScale( long nScale, sal_uInt8 nScript );

    bool    HasCondHeight() const   { return pCondSet && SFX_ITEM_SET ==
                                        pCondSet->GetItemState( ATTR_FONT_HEIGHT, true ); }

    bool    HasEditCharacters() const;

private:
    long        GetMaxDigitWidth();     
    long        GetSignWidth();
    long        GetDotWidth();
    long        GetExpWidth();
    void        TextChanged();
};



ScDrawStringsVars::ScDrawStringsVars(ScOutputData* pData, bool bPTL) :
    pOutput     ( pData ),
    pPattern    ( NULL ),
    pCondSet    ( NULL ),
    nAscentPixel(0),
    eAttrOrient ( SVX_ORIENTATION_STANDARD ),
    eAttrHorJust( SVX_HOR_JUSTIFY_STANDARD ),
    eAttrVerJust( SVX_VER_JUSTIFY_BOTTOM ),
    eAttrHorJustMethod( SVX_JUSTIFY_METHOD_AUTO ),
    eAttrVerJustMethod( SVX_JUSTIFY_METHOD_AUTO ),
    pMargin     ( NULL ),
    nIndent     ( 0 ),
    bRotated    ( false ),
    nOriginalWidth( 0 ),
    nMaxDigitWidth( 0 ),
    nSignWidth( 0 ),
    nDotWidth( 0 ),
    nExpWidth( 0 ),
    nValueFormat( 0 ),
    bLineBreak  ( false ),
    bRepeat     ( false ),
    bShrink     ( false ),
    bPixelToLogic( bPTL ),
    nPos( -1 ),
    nChar( 0x0 )
{
    ScModule* pScMod = SC_MOD();
    bCellContrast = pOutput->mbUseStyleColor &&
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
    

    OutputDevice* pDev = pOutput->mpDev;
    OutputDevice* pRefDevice = pOutput->mpRefDevice;
    OutputDevice* pFmtDevice = pOutput->pFmtDevice;

    

    Fraction aFraction( nScale, 100 );
    if ( !bPixelToLogic )
        aFraction *= pOutput->aZoomY;
    Font aTmpFont;
    pPattern->GetFont( aTmpFont, SC_AUTOCOL_RAW, pFmtDevice, &aFraction, pCondSet, nScript );
    long nNewHeight = aTmpFont.GetHeight();
    if ( nNewHeight > 0 )
        aFont.SetHeight( nNewHeight );

    

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

    SetAutoText( aString );     
}

namespace {

template<typename _ItemType, typename _EnumType>
_EnumType lcl_GetValue(const ScPatternAttr& rPattern, sal_uInt16 nWhich, const SfxItemSet* pCondSet)
{
    const _ItemType& rItem = static_cast<const _ItemType&>(rPattern.GetItem(nWhich, pCondSet));
    return static_cast<_EnumType>(rItem.GetValue());
}

bool lcl_GetBoolValue(const ScPatternAttr& rPattern, sal_uInt16 nWhich, const SfxItemSet* pCondSet)
{
    return lcl_GetValue<SfxBoolItem, bool>(rPattern, nWhich, pCondSet);
}

}

void ScDrawStringsVars::SetPattern(
    const ScPatternAttr* pNew, const SfxItemSet* pSet, const ScRefCellValue& rCell,
    sal_uInt8 nScript )
{
    nMaxDigitWidth = 0;
    nSignWidth     = 0;
    nDotWidth      = 0;
    nExpWidth      = 0;

    pPattern = pNew;
    pCondSet = pSet;

    

    OutputDevice* pDev = pOutput->mpDev;
    OutputDevice* pRefDevice = pOutput->mpRefDevice;
    OutputDevice* pFmtDevice = pOutput->pFmtDevice;

    

    ScAutoFontColorMode eColorMode;
    if ( pOutput->mbUseStyleColor )
    {
        if ( pOutput->mbForceAutoColor )
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

    

    eAttrOrient = pPattern->GetCellOrientation( pCondSet );

    

    eAttrHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->GetItem( ATTR_HOR_JUSTIFY, pCondSet )).GetValue();

    eAttrVerJust = (SvxCellVerJustify)((const SvxVerJustifyItem&)pPattern->GetItem( ATTR_VER_JUSTIFY, pCondSet )).GetValue();
    if ( eAttrVerJust == SVX_VER_JUSTIFY_STANDARD )
        eAttrVerJust = SVX_VER_JUSTIFY_BOTTOM;

    

    eAttrHorJustMethod = lcl_GetValue<SvxJustifyMethodItem, SvxCellJustifyMethod>(*pPattern, ATTR_HOR_JUSTIFY_METHOD, pCondSet);
    eAttrVerJustMethod = lcl_GetValue<SvxJustifyMethodItem, SvxCellJustifyMethod>(*pPattern, ATTR_VER_JUSTIFY_METHOD, pCondSet);

    

    bLineBreak = ((const SfxBoolItem&)pPattern->GetItem( ATTR_LINEBREAK, pCondSet )).GetValue();

    

    bRepeat = ( eAttrHorJust == SVX_HOR_JUSTIFY_REPEAT );
    if ( bRepeat )
    {
        
        eAttrOrient = SVX_ORIENTATION_STANDARD;

        
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
            bRotated = false;
            break;
        case SVX_ORIENTATION_TOPBOTTOM:
            nRot = 2700;
            bRotated = false;
            break;
        case SVX_ORIENTATION_BOTTOMTOP:
            nRot = 900;
            bRotated = false;
            break;
        default:
            OSL_FAIL("Falscher SvxCellOrientation Wert");
            nRot = 0;
            bRotated = false;
            break;
    }
    aFont.SetOrientation( nRot );

    

    if (pOutput->mbSyntaxMode)
        pOutput->SetSyntaxColor(&aFont, rCell);

    pDev->SetFont( aFont );
    if ( pFmtDevice != pDev )
        pFmtDevice->SetFont( aFont );

    aMetric = pFmtDevice->GetFontMetric();

    //
    
    
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

    

    nValueFormat = pPattern->GetNumberFormat( pOutput->mpDoc->GetFormatTable(), pCondSet );

    

    pMargin = (const SvxMarginItem*)&pPattern->GetItem( ATTR_MARGIN, pCondSet );
    if ( eAttrHorJust == SVX_HOR_JUSTIFY_LEFT || eAttrHorJust == SVX_HOR_JUSTIFY_RIGHT )
        nIndent = ((const SfxUInt16Item&)pPattern->GetItem( ATTR_INDENT, pCondSet )).GetValue();
    else
        nIndent = 0;

    

    bShrink = static_cast<const SfxBoolItem&>(pPattern->GetItem( ATTR_SHRINKTOFIT, pCondSet )).GetValue();

    
    

    maLastCell.clear();
}

void ScDrawStringsVars::SetPatternSimple( const ScPatternAttr* pNew, const SfxItemSet* pSet )
{
    nMaxDigitWidth = 0;
    nSignWidth     = 0;
    nDotWidth      = 0;
    nExpWidth      = 0;
    

    pPattern = pNew;
    pCondSet = pSet;        

    

    sal_uLong nOld = nValueFormat;
    const SfxPoolItem* pFormItem;
    if ( !pCondSet || pCondSet->GetItemState(ATTR_VALUE_FORMAT,true,&pFormItem) != SFX_ITEM_SET )
        pFormItem = &pPattern->GetItem(ATTR_VALUE_FORMAT);
    const SfxPoolItem* pLangItem;
    if ( !pCondSet || pCondSet->GetItemState(ATTR_LANGUAGE_FORMAT,true,&pLangItem) != SFX_ITEM_SET )
        pLangItem = &pPattern->GetItem(ATTR_LANGUAGE_FORMAT);
    nValueFormat = pOutput->mpDoc->GetFormatTable()->GetFormatForLanguageIfBuiltIn(
                    ((SfxUInt32Item*)pFormItem)->GetValue(),
                    ((SvxLanguageItem*)pLangItem)->GetLanguage() );

    if (nValueFormat != nOld)
        maLastCell.clear();           

    

    pMargin = (const SvxMarginItem*)&pPattern->GetItem( ATTR_MARGIN, pCondSet );

    if ( eAttrHorJust == SVX_HOR_JUSTIFY_LEFT )
        nIndent = ((const SfxUInt16Item&)pPattern->GetItem( ATTR_INDENT, pCondSet )).GetValue();
    else
        nIndent = 0;

    

    bShrink = static_cast<const SfxBoolItem&>(pPattern->GetItem( ATTR_SHRINKTOFIT, pCondSet )).GetValue();
}

inline bool SameValue( const ScRefCellValue& rCell, const ScRefCellValue& rOldCell )
{
    return rOldCell.meType == CELLTYPE_VALUE && rCell.meType == CELLTYPE_VALUE &&
        rCell.mfValue == rOldCell.mfValue;
}

bool ScDrawStringsVars::SetText( ScRefCellValue& rCell )
{
    bool bChanged = false;

    if (!rCell.isEmpty())
    {
        if (!SameValue(rCell, maLastCell))
        {
            maLastCell = rCell;          

            Color* pColor;
            sal_uLong nFormat = GetValueFormat();
            ScCellFormat::GetString( rCell,
                                     nFormat, aString, &pColor,
                                     *pOutput->mpDoc->GetFormatTable(),
                                     pOutput->mpDoc,
                                     pOutput->mbShowNullValues,
                                     pOutput->mbShowFormulas,
                                     ftCheck, true );
            if ( nFormat )
            {
                nPos = aString.indexOf( 0x1B );
                if ( nPos != -1 )
                {
                    nChar = aString[ nPos + 1 ];
                    
                    aString = aString.replaceAt( nPos, 2, "" );
                }
            }
            else
            {
                nPos = -1;
                nChar = 0x0;
            }
            if (aString.getLength() > DRAWTEXT_MAX)
                aString = aString.copy(0, DRAWTEXT_MAX);

            if ( pColor && !pOutput->mbSyntaxMode && !( pOutput->mbUseStyleColor && pOutput->mbForceAutoColor ) )
            {
                OutputDevice* pDev = pOutput->mpDev;
                aFont.SetColor(*pColor);
                pDev->SetFont( aFont ); 
                bChanged = true;
                maLastCell.clear();       
            }

            TextChanged();
        }
        
    }
    else
    {
        aString = OUString();
        maLastCell.clear();
        aTextSize = Size(0,0);
        nOriginalWidth = 0;
    }

    return bChanged;
}

void ScDrawStringsVars::SetHashText()
{
    SetAutoText(OUString("###"));
}

void ScDrawStringsVars::RepeatToFill( long colWidth )
{
    if ( nPos == -1 || nPos > aString.getLength() )
        return;

    long charWidth = pOutput->pFmtDevice->GetTextWidth(OUString(nChar));
    if ( charWidth < 1) return;
    if (bPixelToLogic)
        colWidth = pOutput->mpRefDevice->PixelToLogic(Size(colWidth,0)).Width();
    
    long aSpaceToFill = ( colWidth - aTextSize.Width() );

    if ( aSpaceToFill <= charWidth )
        return;

    long nCharsToInsert = aSpaceToFill / charWidth;
    OUStringBuffer aFill;
    comphelper::string::padToLength(aFill, nCharsToInsert, nChar);
    aString = aString.replaceAt( nPos, 0, aFill.makeStringAndClear() );
    TextChanged();
}

void ScDrawStringsVars::SetTextToWidthOrHash( ScRefCellValue& rCell, long nWidth )
{
    
    if (bPixelToLogic)
        nWidth = pOutput->mpRefDevice->PixelToLogic(Size(nWidth,0)).Width();

    CellType eType = rCell.meType;
    if (eType != CELLTYPE_VALUE && eType != CELLTYPE_FORMULA)
        
        return;

    if (eType == CELLTYPE_FORMULA)
    {
        ScFormulaCell* pFCell = rCell.mpFormula;
        if (pFCell->GetErrCode() != 0 || pOutput->mbShowFormulas)
        {
            SetHashText();      
            return;
        }
        
        if (!pFCell->IsValue())
            return;
    }

    sal_uLong nFormat = GetResultValueFormat();
    if ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
    {
        
        SetHashText();
        return;
    }

    double fVal = rCell.getValue();

    const SvNumberformat* pNumFormat = pOutput->mpDoc->GetFormatTable()->GetEntry(nFormat);
    if (!pNumFormat)
        return;

    long nMaxDigit = GetMaxDigitWidth();
    sal_uInt16 nNumDigits = static_cast<sal_uInt16>(nWidth / nMaxDigit);
    {
        OUString sTempOut(aString);
        if (!pNumFormat->GetOutputString(fVal, nNumDigits, sTempOut))
        {
            aString = sTempOut;
            
            return;
        }
        aString = sTempOut;
    }
    sal_uInt8 nSignCount = 0, nDecimalCount = 0, nExpCount = 0;
    sal_Int32 nLen = aString.getLength();
    sal_Unicode cDecSep = ScGlobal::GetpLocaleData()->getLocaleItem().decimalSeparator[0];
    for( sal_Int32 i = 0; i < nLen; ++i )
    {
        sal_Unicode c = aString[i];
        if (c == '-')
            ++nSignCount;
        else if (c == cDecSep)
            ++nDecimalCount;
        else if (c == 'E')
            ++nExpCount;
    }

    
    
    if (aString == "0" && fVal != 0.0)
        nDecimalCount = 1;

    if (nDecimalCount)
        nWidth += (nMaxDigit - GetDotWidth()) * nDecimalCount;
    if (nSignCount)
        nWidth += (nMaxDigit - GetSignWidth()) * nSignCount;
    if (nExpCount)
        nWidth += (nMaxDigit - GetExpWidth()) * nExpCount;

    if (nDecimalCount || nSignCount || nExpCount)
    {
        
        nNumDigits = static_cast<sal_uInt16>(nWidth / nMaxDigit);
        OUString sTempOut(aString);
        if (!pNumFormat->GetOutputString(fVal, nNumDigits, sTempOut))
        {
            aString = sTempOut;
            
            return;
        }
        aString = sTempOut;
    }

    long nActualTextWidth = pOutput->pFmtDevice->GetTextWidth(aString);
    if (nActualTextWidth > nWidth)
    {
        
        SetHashText();
        return;
    }

    TextChanged();
    maLastCell.clear();   
}

void ScDrawStringsVars::SetAutoText( const OUString& rAutoText )
{
    aString = rAutoText;

    OutputDevice* pRefDevice = pOutput->mpRefDevice;
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

    maLastCell.clear();       
}

long ScDrawStringsVars::GetMaxDigitWidth()
{
    if (nMaxDigitWidth > 0)
        return nMaxDigitWidth;

    sal_Char cZero = '0';
    for (sal_Char i = 0; i < 10; ++i)
    {
        sal_Char cDigit = cZero + i;
        long n = pOutput->pFmtDevice->GetTextWidth(OUString(cDigit));
        nMaxDigitWidth = ::std::max(nMaxDigitWidth, n);
    }
    return nMaxDigitWidth;
}

long ScDrawStringsVars::GetSignWidth()
{
    if (nSignWidth > 0)
        return nSignWidth;

    nSignWidth = pOutput->pFmtDevice->GetTextWidth(OUString('-'));
    return nSignWidth;
}

long ScDrawStringsVars::GetDotWidth()
{
    if (nDotWidth > 0)
        return nDotWidth;

    const OUString& sep = ScGlobal::GetpLocaleData()->getLocaleItem().decimalSeparator;
    nDotWidth = pOutput->pFmtDevice->GetTextWidth(sep);
    return nDotWidth;
}

long ScDrawStringsVars::GetExpWidth()
{
    if (nExpWidth > 0)
        return nExpWidth;

    nExpWidth = pOutput->pFmtDevice->GetTextWidth(OUString('E'));
    return nExpWidth;
}

void ScDrawStringsVars::TextChanged()
{
    OutputDevice* pRefDevice = pOutput->mpRefDevice;
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

bool ScDrawStringsVars::HasEditCharacters() const
{
    for (sal_Int32 nIdx = 0; nIdx < aString.getLength(); ++nIdx)
    {
        switch(aString[nIdx])
        {
            case CHAR_NBSP:
            case CHAR_SHY:
            case CHAR_ZWSP:
            case CHAR_LRM:
            case CHAR_RLM:
            case CHAR_NBHY:
            case CHAR_ZWNBSP:
                return true;
            default:
                break;
        }
    }

    return false;
}

sal_uLong ScDrawStringsVars::GetResultValueFormat() const
{
    
    

    return nValueFormat;
}



double ScOutputData::GetStretch()
{
    if ( mpRefDevice->IsMapMode() )
    {
        
        
        
        
        return 1.0;
    }

    
    

    if ( mpRefDevice == pFmtDevice )
    {
        MapMode aOld = mpRefDevice->GetMapMode();
        return ((double)aOld.GetScaleY()) / ((double)aOld.GetScaleX()) * ((double)aZoomY) / ((double)aZoomX);
    }
    else
    {
        
        return ((double)aZoomY) / ((double)aZoomX);
    }
}



//

//

static void lcl_DoHyperlinkResult( OutputDevice* pDev, const Rectangle& rRect, ScRefCellValue& rCell )
{
    vcl::PDFExtOutDevData* pPDFData = PTR_CAST( vcl::PDFExtOutDevData, pDev->GetExtOutDevData() );

    OUString aCellText;
    OUString aURL;
    if (rCell.meType == CELLTYPE_FORMULA)
    {
        ScFormulaCell* pFCell = rCell.mpFormula;
        if ( pFCell->IsHyperLinkCell() )
            pFCell->GetURLResult( aURL, aCellText );
    }

    if ( !aURL.isEmpty() && pPDFData )
    {
        vcl::PDFExtOutDevBookmarkEntry aBookmark;
        aBookmark.nLinkId = pPDFData->CreateLink( rRect );
        aBookmark.aBookmark = aURL;
        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFData->GetBookmarks();
        rBookmarks.push_back( aBookmark );
    }
}

void ScOutputData::SetSyntaxColor( Font* pFont, const ScRefCellValue& rCell )
{
    switch (rCell.meType)
    {
        case CELLTYPE_VALUE:
            pFont->SetColor(*pValueColor);
        break;
        case CELLTYPE_STRING:
            pFont->SetColor(*pTextColor);
        break;
        case CELLTYPE_FORMULA:
            pFont->SetColor(*pFormulaColor);
        break;
        default:
        {
            
        }
    }
}

static void lcl_SetEditColor( EditEngine& rEngine, const Color& rColor )
{
    ESelection aSel( 0, 0, rEngine.GetParagraphCount(), 0 );
    SfxItemSet aSet( rEngine.GetEmptyItemSet() );
    aSet.Put( SvxColorItem( rColor, EE_CHAR_COLOR ) );
    rEngine.QuickSetAttribs( aSet, aSel );
    
}

void ScOutputData::SetEditSyntaxColor( EditEngine& rEngine, ScRefCellValue& rCell )
{
    Color aColor;
    switch (rCell.meType)
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
            
        }
    }
    lcl_SetEditColor( rEngine, aColor );
}

bool ScOutputData::GetMergeOrigin( SCCOL nX, SCROW nY, SCSIZE nArrY,
                                    SCCOL& rOverX, SCROW& rOverY,
                                    bool bVisRowChanged )
{
    bool bDoMerge = false;
    bool bIsLeft = ( nX == nVisX1 );
    bool bIsTop  = ( nY == nVisY1 ) || bVisRowChanged;

    CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nX+1];
    if ( pInfo->bHOverlapped && pInfo->bVOverlapped )
        bDoMerge = bIsLeft && bIsTop;
    else if ( pInfo->bHOverlapped )
        bDoMerge = bIsLeft;
    else if ( pInfo->bVOverlapped )
        bDoMerge = bIsTop;

    rOverX = nX;
    rOverY = nY;
    bool bHOver = pInfo->bHOverlapped;
    bool bVOver = pInfo->bVOverlapped;
    bool bHidden;

    while (bHOver)              
    {
        --rOverX;
        bHidden = mpDoc->ColHidden(rOverX, nTab);
        if ( !bDoMerge && !bHidden )
            return false;

        if (rOverX >= nX1 && !bHidden)
        {
            bHOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bHOverlapped;
            bVOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bVOverlapped;
        }
        else
        {
            sal_uInt16 nOverlap = ((ScMergeFlagAttr*)mpDoc->GetAttr(
                                rOverX, rOverY, nTab, ATTR_MERGE_FLAG ))->GetValue();
            bHOver = ((nOverlap & SC_MF_HOR) != 0);
            bVOver = ((nOverlap & SC_MF_VER) != 0);
        }
    }

    while (bVOver)
    {
        --rOverY;
        bHidden = mpDoc->RowHidden(rOverY, nTab);
        if ( !bDoMerge && !bHidden )
            return false;

        if (nArrY>0)
            --nArrY;                        

        if (rOverX >= nX1 && rOverY >= nY1 &&
            !mpDoc->ColHidden(rOverX, nTab) &&
            !mpDoc->RowHidden(rOverY, nTab) &&
            pRowInfo[nArrY].nRowNo == rOverY)
        {
            bHOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bHOverlapped;
            bVOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bVOverlapped;
        }
        else
        {
            sal_uInt16 nOverlap = ((ScMergeFlagAttr*)mpDoc->GetAttr(
                                rOverX, rOverY, nTab, ATTR_MERGE_FLAG ))->GetValue();
            bHOver = ((nOverlap & SC_MF_HOR) != 0);
            bVOver = ((nOverlap & SC_MF_VER) != 0);
        }
    }

    return true;
}

inline bool StringDiffer( const ScPatternAttr*& rpOldPattern, const ScPatternAttr*& rpNewPattern )
{
    OSL_ENSURE( rpNewPattern, "pNewPattern" );

    if ( rpNewPattern == rpOldPattern )
        return false;
    else if ( !rpOldPattern )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT ) != &rpOldPattern->GetItem( ATTR_FONT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT ) != &rpOldPattern->GetItem( ATTR_CJK_FONT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT ) != &rpOldPattern->GetItem( ATTR_CTL_FONT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_HEIGHT ) != &rpOldPattern->GetItem( ATTR_FONT_HEIGHT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT_HEIGHT ) != &rpOldPattern->GetItem( ATTR_CJK_FONT_HEIGHT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT_HEIGHT ) != &rpOldPattern->GetItem( ATTR_CTL_FONT_HEIGHT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_WEIGHT ) != &rpOldPattern->GetItem( ATTR_FONT_WEIGHT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT_WEIGHT ) != &rpOldPattern->GetItem( ATTR_CJK_FONT_WEIGHT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT_WEIGHT ) != &rpOldPattern->GetItem( ATTR_CTL_FONT_WEIGHT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_POSTURE ) != &rpOldPattern->GetItem( ATTR_FONT_POSTURE ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT_POSTURE ) != &rpOldPattern->GetItem( ATTR_CJK_FONT_POSTURE ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT_POSTURE ) != &rpOldPattern->GetItem( ATTR_CTL_FONT_POSTURE ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_UNDERLINE ) != &rpOldPattern->GetItem( ATTR_FONT_UNDERLINE ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_OVERLINE ) != &rpOldPattern->GetItem( ATTR_FONT_OVERLINE ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_WORDLINE ) != &rpOldPattern->GetItem( ATTR_FONT_WORDLINE ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_CROSSEDOUT ) != &rpOldPattern->GetItem( ATTR_FONT_CROSSEDOUT ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_CONTOUR ) != &rpOldPattern->GetItem( ATTR_FONT_CONTOUR ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_SHADOWED ) != &rpOldPattern->GetItem( ATTR_FONT_SHADOWED ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_COLOR ) != &rpOldPattern->GetItem( ATTR_FONT_COLOR ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_HOR_JUSTIFY ) != &rpOldPattern->GetItem( ATTR_HOR_JUSTIFY ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_HOR_JUSTIFY_METHOD ) != &rpOldPattern->GetItem( ATTR_HOR_JUSTIFY_METHOD ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_VER_JUSTIFY ) != &rpOldPattern->GetItem( ATTR_VER_JUSTIFY ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_VER_JUSTIFY_METHOD ) != &rpOldPattern->GetItem( ATTR_VER_JUSTIFY_METHOD ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_STACKED ) != &rpOldPattern->GetItem( ATTR_STACKED ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_LINEBREAK ) != &rpOldPattern->GetItem( ATTR_LINEBREAK ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_MARGIN ) != &rpOldPattern->GetItem( ATTR_MARGIN ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_ROTATE_VALUE ) != &rpOldPattern->GetItem( ATTR_ROTATE_VALUE ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FORBIDDEN_RULES ) != &rpOldPattern->GetItem( ATTR_FORBIDDEN_RULES ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_EMPHASISMARK ) != &rpOldPattern->GetItem( ATTR_FONT_EMPHASISMARK ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_RELIEF ) != &rpOldPattern->GetItem( ATTR_FONT_RELIEF ) )
        return true;
    else if ( &rpNewPattern->GetItem( ATTR_BACKGROUND ) != &rpOldPattern->GetItem( ATTR_BACKGROUND ) )
        return true;    
    else
    {
        rpOldPattern = rpNewPattern;
        return false;
    }
}

static inline void lcl_CreateInterpretProgress( bool& bProgress, ScDocument* pDoc,
        ScFormulaCell* pFCell )
{
    if ( !bProgress && pFCell->GetDirty() )
    {
        ScProgress::CreateInterpretProgress( pDoc, true );
        bProgress = true;
    }
}

inline bool IsAmbiguousScript( sal_uInt8 nScript )
{
    return ( nScript != SCRIPTTYPE_LATIN &&
             nScript != SCRIPTTYPE_ASIAN &&
             nScript != SCRIPTTYPE_COMPLEX );
}

bool ScOutputData::IsEmptyCellText( RowInfo* pThisRowInfo, SCCOL nX, SCROW nY )
{
    

    bool bEmpty;
    if ( pThisRowInfo && nX <= nX2 )
        bEmpty = pThisRowInfo->pCellInfo[nX+1].bEmptyCellText;
    else
    {
        ScRefCellValue aCell;
        aCell.assign(*mpDoc, ScAddress(nX, nY, nTab));
        bEmpty = aCell.isEmpty();
    }

    if ( !bEmpty && ( nX < nX1 || nX > nX2 || !pThisRowInfo ) )
    {
        
        

        bool bIsPrint = ( eType == OUTTYPE_PRINTER );

        if ( bIsPrint || bTabProtected )
        {
            const ScProtectionAttr* pAttr = (const ScProtectionAttr*)
                    mpDoc->GetEffItem( nX, nY, nTab, ATTR_PROTECTION );
            if ( bIsPrint && pAttr->GetHidePrint() )
                bEmpty = true;
            else if ( bTabProtected )
            {
                if ( pAttr->GetHideCell() )
                    bEmpty = true;
                else if ( mbShowFormulas && pAttr->GetHideFormula() )
                {
                    if (mpDoc->GetCellType(ScAddress(nX, nY, nTab)) == CELLTYPE_FORMULA)
                        bEmpty = true;
                }
            }
        }
    }
    return bEmpty;
}

void ScOutputData::GetVisibleCell( SCCOL nCol, SCROW nRow, SCTAB nTabP, ScRefCellValue& rCell )
{
    rCell.assign(*mpDoc, ScAddress(nCol, nRow, nTabP));
    if (!rCell.isEmpty() && IsEmptyCellText(NULL, nCol, nRow))
        rCell.clear();
}

bool ScOutputData::IsAvailable( SCCOL nX, SCROW nY )
{
    
    
    

    ScRefCellValue aCell;
    aCell.assign(*mpDoc, ScAddress(nX, nY, nTab));
    if (!aCell.isEmpty() && !IsEmptyCellText(NULL, nX, nY))
        return false;

    const ScPatternAttr* pPattern = mpDoc->GetPattern( nX, nY, nTab );
    if ( ((const ScMergeAttr&)pPattern->GetItem(ATTR_MERGE)).IsMerged() ||
         ((const ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).IsOverlapped() )
    {
        return false;
    }

    return true;
}












void ScOutputData::GetOutputArea( SCCOL nX, SCSIZE nArrY, long nPosX, long nPosY,
                                  SCCOL nCellX, SCROW nCellY, long nNeeded,
                                  const ScPatternAttr& rPattern,
                                  sal_uInt16 nHorJustify, bool bCellIsValue,
                                  bool bBreak, bool bOverwrite,
                                  OutputAreaParam& rParam )
{
    
    RowInfo& rThisRowInfo = pRowInfo[nArrY];

    long nLayoutSign = bLayoutRTL ? -1 : 1;

    long nCellPosX = nPosX;         
    SCCOL nCompCol = nX;
    while ( nCellX > nCompCol )
    {
        
        long nColWidth = ( nCompCol <= nX2 ) ?
                pRowInfo[0].pCellInfo[nCompCol+1].nWidth :
                (long) ( mpDoc->GetColWidth( nCompCol, nTab ) * mnPPTX );
        nCellPosX += nColWidth * nLayoutSign;
        ++nCompCol;
    }
    while ( nCellX < nCompCol )
    {
        --nCompCol;
        long nColWidth = ( nCompCol <= nX2 ) ?
                pRowInfo[0].pCellInfo[nCompCol+1].nWidth :
                (long) ( mpDoc->GetColWidth( nCompCol, nTab ) * mnPPTX );
        nCellPosX -= nColWidth * nLayoutSign;
    }

    long nCellPosY = nPosY;         
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
            sal_uInt16 nDocHeight = mpDoc->GetRowHeight( nCompRow, nTab );
            if ( nDocHeight )
                nCellPosY += (long) ( nDocHeight * mnPPTY );
            ++nCompRow;
        }
    }
    nCellPosY -= (long) mpDoc->GetScaledRowHeight( nCellY, nCompRow-1, nTab, mnPPTY );

    const ScMergeAttr* pMerge = (const ScMergeAttr*)&rPattern.GetItem( ATTR_MERGE );
    bool bMerged = pMerge->IsMerged();
    long nMergeCols = pMerge->GetColMerge();
    if ( nMergeCols == 0 )
        nMergeCols = 1;
    long nMergeRows = pMerge->GetRowMerge();
    if ( nMergeRows == 0 )
        nMergeRows = 1;

    long nMergeSizeX = 0;
    for ( long i=0; i<nMergeCols; i++ )
    {
        long nColWidth = ( nCellX+i <= nX2 ) ?
                pRowInfo[0].pCellInfo[nCellX+i+1].nWidth :
                (long) ( mpDoc->GetColWidth( sal::static_int_cast<SCCOL>(nCellX+i), nTab ) * mnPPTX );
        nMergeSizeX += nColWidth;
    }
    long nMergeSizeY = 0;
    short nDirect = 0;
    if ( rThisRowInfo.nRowNo == nCellY )
    {
        
        nMergeSizeY += rThisRowInfo.nHeight;
        nDirect = 1;        
    }
    
    nMergeSizeY += (long) mpDoc->GetScaledRowHeight( nCellY+nDirect, nCellY+nMergeRows-1, nTab, mnPPTY);

    --nMergeSizeX;      

    rParam.mnColWidth = nMergeSizeX; 

    //
    
    //

    

    rParam.maAlignRect.Left() = nCellPosX;
    rParam.maAlignRect.Right() = nCellPosX + ( nMergeSizeX - 1 ) * nLayoutSign;
    rParam.maAlignRect.Top() = nCellPosY;
    rParam.maAlignRect.Bottom() = nCellPosY + nMergeSizeY - 1;

    
    

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
                
            }
        }

        
        if ( bLayoutRTL )
            ::std::swap( nLeftMissing, nRightMissing );

        SCCOL nRightX = nCellX;
        SCCOL nLeftX = nCellX;
        if ( !bMerged && !bCellIsValue && !bBreak )
        {
            

            while ( nRightMissing > 0 && nRightX < MAXCOL && ( bOverwrite || IsAvailable( nRightX+1, nCellY ) ) )
            {
                ++nRightX;
                long nAdd = (long) ( mpDoc->GetColWidth( nRightX, nTab ) * mnPPTX );
                nRightMissing -= nAdd;
                rParam.maClipRect.Right() += nAdd * nLayoutSign;

                if ( rThisRowInfo.nRowNo == nCellY && nRightX >= nX1 && nRightX <= nX2 )
                    rThisRowInfo.pCellInfo[nRightX].bHideGrid = true;
            }

            while ( nLeftMissing > 0 && nLeftX > 0 && ( bOverwrite || IsAvailable( nLeftX-1, nCellY ) ) )
            {
                if ( rThisRowInfo.nRowNo == nCellY && nLeftX >= nX1 && nLeftX <= nX2 )
                    rThisRowInfo.pCellInfo[nLeftX].bHideGrid = true;

                --nLeftX;
                long nAdd = (long) ( mpDoc->GetColWidth( nLeftX, nTab ) * mnPPTX );
                nLeftMissing -= nAdd;
                rParam.maClipRect.Left() -= nAdd * nLayoutSign;
            }
        }

        
        
        if ( nRightMissing > 0 && bMarkClipped && nRightX >= nX1 && nRightX <= nX2 && !bBreak && !bCellIsValue )
        {
            rThisRowInfo.pCellInfo[nRightX+1].nClipMark |= SC_CLIPMARK_RIGHT;
            bAnyClipped = true;
            long nMarkPixel = (long)( SC_CLIPMARK_SIZE * mnPPTX );
            rParam.maClipRect.Right() -= nMarkPixel * nLayoutSign;
        }
        if ( nLeftMissing > 0 && bMarkClipped && nLeftX >= nX1 && nLeftX <= nX2 && !bBreak && !bCellIsValue )
        {
            rThisRowInfo.pCellInfo[nLeftX+1].nClipMark |= SC_CLIPMARK_LEFT;
            bAnyClipped = true;
            long nMarkPixel = (long)( SC_CLIPMARK_SIZE * mnPPTX );
            rParam.maClipRect.Left() += nMarkPixel * nLayoutSign;
        }

        rParam.mbLeftClip = ( nLeftMissing > 0 );
        rParam.mbRightClip = ( nRightMissing > 0 );
    }
    else
    {
        rParam.mbLeftClip = rParam.mbRightClip = false;

        
        

        if ( eType==OUTTYPE_WINDOW &&
             ( static_cast<const ScMergeFlagAttr&>(rPattern.GetItem(ATTR_MERGE_FLAG)).GetValue() & (SC_MF_AUTO|SC_MF_BUTTON|SC_MF_BUTTON_POPUP) ) &&
             ( !bBreak || mpRefDevice == pFmtDevice ) )
        {
            
            const long nFilter = DROPDOWN_BITMAP_SIZE;
            bool bFit = ( nNeeded + nFilter <= nMergeSizeX );
            if ( bFit || bCellIsValue )
            {
                
                

                rParam.maAlignRect.Right() -= nFilter * nLayoutSign;
                rParam.maClipRect.Right() -= nFilter * nLayoutSign;

                
                

                if ( !bFit )
                    rParam.mbLeftClip = rParam.mbRightClip = true;
            }
        }
    }

    

    rParam.maAlignRect.Justify();
    rParam.maClipRect.Justify();
}

namespace {

bool beginsWithRTLCharacter(const OUString& rStr)
{
    if (rStr.isEmpty())
        return false;

    switch (ScGlobal::pCharClass->getCharacterDirection(rStr, 0))
    {
        case i18n::DirectionProperty_RIGHT_TO_LEFT:
        case i18n::DirectionProperty_RIGHT_TO_LEFT_ARABIC:
        case i18n::DirectionProperty_RIGHT_TO_LEFT_EMBEDDING:
        case i18n::DirectionProperty_RIGHT_TO_LEFT_OVERRIDE:
            return true;
        default:
            ;
    }

    return false;
}

}

/** Get left, right or centered alignment from RTL context.

    Does not return standard, block or repeat, for these the contextual left or
    right alignment is returned.
 */
static SvxCellHorJustify getAlignmentFromContext( SvxCellHorJustify eInHorJust,
        bool bCellIsValue, const OUString& rText,
        const ScPatternAttr& rPattern, const SfxItemSet* pCondSet,
        const ScDocument* pDoc, SCTAB nTab )
{
    SvxCellHorJustify eHorJustContext = eInHorJust;
    bool bUseWritingDirection = false;
    if (eInHorJust == SVX_HOR_JUSTIFY_STANDARD)
    {
        
        
        if (beginsWithRTLCharacter( rText))
            eHorJustContext = bCellIsValue ? SVX_HOR_JUSTIFY_LEFT : SVX_HOR_JUSTIFY_RIGHT;
        else if (bCellIsValue)
            eHorJustContext = SVX_HOR_JUSTIFY_RIGHT;
        else
            bUseWritingDirection = true;
    }

    if (bUseWritingDirection ||
            eInHorJust == SVX_HOR_JUSTIFY_BLOCK || eInHorJust == SVX_HOR_JUSTIFY_REPEAT)
    {
        sal_uInt16 nDirection = lcl_GetValue<SvxFrameDirectionItem, sal_uInt16>( rPattern, ATTR_WRITINGDIR, pCondSet);
        if (nDirection == FRMDIR_HORI_LEFT_TOP || nDirection == FRMDIR_VERT_TOP_LEFT)
            eHorJustContext = SVX_HOR_JUSTIFY_LEFT;
        else if (nDirection == FRMDIR_ENVIRONMENT)
        {
            SAL_WARN_IF( !pDoc, "sc.ui", "getAlignmentFromContext - pDoc==NULL");
            eHorJustContext = (pDoc && pDoc->IsLayoutRTL(nTab)) ? SVX_HOR_JUSTIFY_RIGHT : SVX_HOR_JUSTIFY_LEFT;
        }
        else
            eHorJustContext = SVX_HOR_JUSTIFY_RIGHT;
    }
    return eHorJustContext;
}

void ScOutputData::DrawStrings( bool bPixelToLogic )
{
    OSL_ENSURE( mpDev == mpRefDevice ||
                mpDev->GetMapMode().GetMapUnit() == mpRefDevice->GetMapMode().GetMapUnit(),
                "DrawStrings: unterschiedliche MapUnits ?!?!" );

    vcl::PDFExtOutDevData* pPDFData = PTR_CAST( vcl::PDFExtOutDevData, mpDev->GetExtOutDevData() );

    bool bWasIdleEnabled = mpDoc->IsIdleEnabled();
    mpDoc->EnableIdle(false);

    ScDrawStringsVars aVars( this, bPixelToLogic );

    bool bProgress = false;

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
        nInitPosX += nMirrorW - 1;              
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    SCCOL nLastContentCol = MAXCOL;
    if ( nX2 < MAXCOL )
        nLastContentCol = sal::static_int_cast<SCCOL>(
            nLastContentCol - mpDoc->GetEmptyLinesInBlock( nX2+1, nY1, nTab, MAXCOL, nY2, nTab, DIR_RIGHT ) );
    SCCOL nLoopStartX = nX1;
    if ( nX1 > 0 )
        --nLoopStartX;          

    
    OutputAreaParam aAreaParam;
    bool bCellIsValue = false;
    long nNeededWidth = 0;
    const ScPatternAttr* pPattern = NULL;
    const SfxItemSet* pCondSet = NULL;
    const ScPatternAttr* pOldPattern = NULL;
    const SfxItemSet* pOldCondSet = NULL;
    sal_uInt8 nOldScript = 0;

    
    
    ::boost::ptr_vector<ScPatternAttr> aAltPatterns;

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->bChanged )
        {
            SCROW nY = pThisRowInfo->nRowNo;
            long nPosX = nInitPosX;
            if ( nLoopStartX < nX1 )
                nPosX -= pRowInfo[0].pCellInfo[nLoopStartX+1].nWidth * nLayoutSign;
            for (SCCOL nX=nLoopStartX; nX<=nX2; nX++)
            {
                bool bMergeEmpty = false;
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                bool bEmpty = nX < nX1 || pInfo->bEmptyCellText;

                SCCOL nCellX = nX;                  
                SCROW nCellY = nY;
                bool bDoCell = false;
                bool bNeedEdit = false;

                //
                
                //

                bool bOverlapped = (pInfo->bHOverlapped || pInfo->bVOverlapped);
                if ( bOverlapped )
                {
                    bEmpty = true;

                    SCCOL nOverX;                   
                    SCROW nOverY;
                    bool bVisChanged = !pRowInfo[nArrY-1].bChanged;
                    if (GetMergeOrigin( nX,nY, nArrY, nOverX,nOverY, bVisChanged ))
                    {
                        nCellX = nOverX;
                        nCellY = nOverY;
                        bDoCell = true;
                    }
                    else
                        bMergeEmpty = true;
                }

                //
                
                //

                if ( bEmpty && !bMergeEmpty && nX < nX1 && !bOverlapped )
                {
                    SCCOL nTempX=nX1;
                    while (nTempX > 0 && IsEmptyCellText( pThisRowInfo, nTempX, nY ))
                        --nTempX;

                    if ( nTempX < nX1 &&
                         !IsEmptyCellText( pThisRowInfo, nTempX, nY ) &&
                         !mpDoc->HasAttrib( nTempX,nY,nTab, nX1,nY,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                    {
                        nCellX = nTempX;
                        bDoCell = true;
                    }
                }

                //
                
                //

                if ( bEmpty && !bMergeEmpty && nX == nX2 && !bOverlapped )
                {
                    

                    SCCOL nTempX=nX;
                    while (nTempX < nLastContentCol && IsEmptyCellText( pThisRowInfo, nTempX, nY ))
                        ++nTempX;

                    if ( nTempX > nX &&
                         !IsEmptyCellText( pThisRowInfo, nTempX, nY ) &&
                         !mpDoc->HasAttrib( nTempX,nY,nTab, nX,nY,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                    {
                        nCellX = nTempX;
                        bDoCell = true;
                    }
                }

                //
                
                //

                if (!bEmpty)
                    bDoCell = true;

                //
                
                //

                if ( bDoCell && bEditMode && nCellX == nEditCol && nCellY == nEditRow )
                    bDoCell = false;

                
                if ( bDoCell )
                {
                    if(pInfo->pDataBar && !pInfo->pDataBar->mbShowValue)
                        bDoCell = false;
                    if(pInfo->pIconSet && !pInfo->pIconSet->mbShowValue)
                        bDoCell = false;
                }

                //
                
                //

                ScRefCellValue aCell;
                if (bDoCell)
                {
                    if ( nCellY == nY && nCellX == nX && nCellX >= nX1 && nCellX <= nX2 )
                        aCell = pThisRowInfo->pCellInfo[nCellX+1].maCell;
                    else
                        GetVisibleCell( nCellX, nCellY, nTab, aCell );      
                    if (aCell.isEmpty())
                        bDoCell = false;
                    else if (aCell.meType == CELLTYPE_EDIT)
                        bNeedEdit = true;
                }

                
                if (bDoCell && !bNeedEdit && aCell.meType == CELLTYPE_STRING)
                {
                    if (mpSpellCheckCxt && mpSpellCheckCxt->isMisspelled(nCellX, nCellY))
                        bNeedEdit = true;
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
                            
                            
                            pPattern = mpDoc->GetPattern( nCellX, nCellY, nTab );
                            pCondSet = mpDoc->GetCondResult( nCellX, nCellY, nTab );
                        }
                    }
                    else        
                    {
                        pPattern = mpDoc->GetPattern( nCellX, nCellY, nTab );
                        pCondSet = mpDoc->GetCondResult( nCellX, nCellY, nTab );
                    }
                    if ( mpDoc->GetPreviewFont() || mpDoc->GetPreviewCellStyle() )
                    {
                        aAltPatterns.push_back(new ScPatternAttr(*pPattern));
                        ScPatternAttr* pAltPattern = &aAltPatterns.back();
                        if (  ScStyleSheet* pPreviewStyle = mpDoc->GetPreviewCellStyle( nCellX, nCellY, nTab ) )
                        {
                            pAltPattern->SetStyleSheet(pPreviewStyle);
                        }
                        else if ( SfxItemSet* pFontSet = mpDoc->GetPreviewFont( nCellX, nCellY, nTab ) )
                        {
                            const SfxPoolItem* pItem;
                            if ( pFontSet->GetItemState( ATTR_FONT, true, &pItem ) == SFX_ITEM_SET )
                                pAltPattern->GetItemSet().Put( (const SvxFontItem&)*pItem );
                            if ( pFontSet->GetItemState( ATTR_CJK_FONT, true, &pItem ) == SFX_ITEM_SET )
                                pAltPattern->GetItemSet().Put( (const SvxFontItem&)*pItem );
                            if ( pFontSet->GetItemState( ATTR_CTL_FONT, true, &pItem ) == SFX_ITEM_SET )
                                pAltPattern->GetItemSet().Put( (const SvxFontItem&)*pItem );
                        }
                        pPattern = pAltPattern;
                    }

                    if (aCell.hasNumeric() &&
                        static_cast<const SfxBoolItem&>(
                            pPattern->GetItem(ATTR_LINEBREAK, pCondSet)).GetValue())
                    {
                        
                        aAltPatterns.push_back(new ScPatternAttr(*pPattern));
                        ScPatternAttr* pAltPattern = &aAltPatterns.back();
                        SfxBoolItem aLineBreak(ATTR_LINEBREAK, false);
                        pAltPattern->GetItemSet().Put(aLineBreak);
                        pPattern = pAltPattern;
                    }

                    sal_uInt8 nScript = mpDoc->GetCellScriptType(
                        ScAddress(nCellX, nCellY, nTab),
                        pPattern->GetNumberFormat(mpDoc->GetFormatTable(), pCondSet));

                    if (nScript == 0)
                        nScript = ScGlobal::GetDefaultScriptType();

                    if ( pPattern != pOldPattern || pCondSet != pOldCondSet ||
                         nScript != nOldScript || mbSyntaxMode )
                    {
                        if ( StringDiffer(pOldPattern,pPattern) ||
                             pCondSet != pOldCondSet || nScript != nOldScript || mbSyntaxMode )
                        {
                            aVars.SetPattern(pPattern, pCondSet, aCell, nScript);
                        }
                        else
                            aVars.SetPatternSimple( pPattern, pCondSet );
                        pOldPattern = pPattern;
                        pOldCondSet = pCondSet;
                        nOldScript = nScript;
                    }

                    
                    if ( aVars.GetOrient() == SVX_ORIENTATION_STACKED ||
                         aVars.IsRotated() || IsAmbiguousScript(nScript) )
                        bNeedEdit = true;
                }
                if (bDoCell && !bNeedEdit)
                {
                    bool bFormulaCell = (aCell.meType == CELLTYPE_FORMULA);
                    if ( bFormulaCell )
                        lcl_CreateInterpretProgress(bProgress, mpDoc, aCell.mpFormula);
                    if ( aVars.SetText(aCell) )
                        pOldPattern = NULL;
                    bNeedEdit = aVars.HasEditCharacters() || (bFormulaCell && aCell.mpFormula->IsMultilineResult());
                }
                long nTotalMargin = 0;
                SvxCellHorJustify eOutHorJust = SVX_HOR_JUSTIFY_STANDARD;
                if (bDoCell && !bNeedEdit)
                {
                    CellType eCellType = aCell.meType;
                    bCellIsValue = ( eCellType == CELLTYPE_VALUE );
                    if ( eCellType == CELLTYPE_FORMULA )
                    {
                        ScFormulaCell* pFCell = aCell.mpFormula;
                        bCellIsValue = pFCell->IsRunning() || pFCell->IsValue();
                    }

                    eOutHorJust = getAlignmentFromContext( aVars.GetHorJust(), bCellIsValue, aVars.GetString(),
                            *pPattern, pCondSet, mpDoc, nTab);

                    bool bBreak = ( aVars.GetLineBreak() || aVars.GetHorJust() == SVX_HOR_JUSTIFY_BLOCK );
                    
                    if (bBreak && bCellIsValue && (aVars.GetResultValueFormat() % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
                        bBreak = false;

                    bool bRepeat = aVars.IsRepeat() && !bBreak;
                    bool bShrink = aVars.IsShrink() && !bBreak && !bRepeat;

                    nTotalMargin =
                        static_cast<long>(aVars.GetLeftTotal() * mnPPTX) +
                        static_cast<long>(aVars.GetMargin()->GetRightMargin() * mnPPTX);

                    nNeededWidth = aVars.GetTextSize().Width() + nTotalMargin;

                    
                    GetOutputArea( nX, nArrY, nPosX, nPosY, nCellX, nCellY, nNeededWidth,
                                   *pPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                                   bCellIsValue || bRepeat || bShrink, bBreak, false,
                                   aAreaParam );

                    aVars.RepeatToFill( aAreaParam.mnColWidth - nTotalMargin );
                    if ( bShrink )
                    {
                        if ( aVars.GetOrient() != SVX_ORIENTATION_STANDARD )
                        {
                            
                            
                            bNeedEdit = true;
                        }
                        else if ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip )     
                        {
                            long nAvailable = aAreaParam.maAlignRect.GetWidth() - nTotalMargin;
                            long nScaleSize = aVars.GetTextSize().Width();         

                            if ( nScaleSize > 0 )       
                            {
                                long nScale = ( nAvailable * 100 ) / nScaleSize;

                                aVars.SetShrinkScale( nScale, nOldScript );
                                long nNewSize = aVars.GetTextSize().Width();

                                sal_uInt16 nShrinkAgain = 0;
                                while ( nNewSize > nAvailable && nShrinkAgain < SC_SHRINKAGAIN_MAX )
                                {
                                    
                                    

                                    nScale = ( nScale * 9 ) / 10;
                                    aVars.SetShrinkScale( nScale, nOldScript );
                                    nNewSize = aVars.GetTextSize().Width();
                                    ++nShrinkAgain;
                                }
                                
                                
                                if ( nNewSize <= nAvailable )
                                    aAreaParam.mbLeftClip = aAreaParam.mbRightClip = false;

                                pOldPattern = NULL;
                            }
                        }
                    }

                    if ( bRepeat && !aAreaParam.mbLeftClip && !aAreaParam.mbRightClip )
                    {
                        long nAvailable = aAreaParam.maAlignRect.GetWidth() - nTotalMargin;
                        long nRepeatSize = aVars.GetTextSize().Width();         
                        
                        
                        if ( pFmtDevice != mpRefDevice )
                            ++nRepeatSize;
                        if ( nRepeatSize > 0 )
                        {
                            long nRepeatCount = nAvailable / nRepeatSize;
                            if ( nRepeatCount > 1 )
                            {
                                OUString aCellStr = aVars.GetString();
                                OUString aRepeated = aCellStr;
                                for ( long nRepeat = 1; nRepeat < nRepeatCount; nRepeat++ )
                                    aRepeated += aCellStr;
                                aVars.SetAutoText( aRepeated );
                            }
                        }
                    }

                    
                    if ( bBreak )
                    {
                        if ( aVars.GetOrient() == SVX_ORIENTATION_STANDARD )
                            bNeedEdit = ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip );
                        else
                        {
                            long nHeight = aVars.GetTextSize().Height() +
                                            (long)(aVars.GetMargin()->GetTopMargin()*mnPPTY) +
                                            (long)(aVars.GetMargin()->GetBottomMargin()*mnPPTY);
                            bNeedEdit = ( nHeight > aAreaParam.maClipRect.GetHeight() );
                        }
                    }
                    if (!bNeedEdit)
                    {
                        bNeedEdit =
                            aVars.GetHorJust() == SVX_HOR_JUSTIFY_BLOCK &&
                            aVars.GetHorJustMethod() == SVX_JUSTIFY_METHOD_DISTRIBUTE;
                    }
                }
                if (bNeedEdit)
                {
                    
                    
                    
                    SCCOL nMarkX = ( nCellX <= nX2 ) ? nCellX : nX2;
                    RowInfo* pMarkRowInfo = ( nCellY == nY ) ? pThisRowInfo : &pRowInfo[0];
                    pMarkRowInfo->pCellInfo[nMarkX+1].bEditEngine = true;
                    bDoCell = false;    
                }
                if ( bDoCell )
                {
                    if ( bCellIsValue && ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip ) )
                    {
                        if (mbShowFormulas)
                            aVars.SetHashText();
                        else
                            
                            aVars.SetTextToWidthOrHash(aCell, aAreaParam.mnColWidth - nTotalMargin);

                        nNeededWidth = aVars.GetTextSize().Width() +
                                    (long) ( aVars.GetLeftTotal() * mnPPTX ) +
                                    (long) ( aVars.GetMargin()->GetRightMargin() * mnPPTX );
                        if ( nNeededWidth <= aAreaParam.maClipRect.GetWidth() )
                            aAreaParam.mbLeftClip = aAreaParam.mbRightClip = false;

                        
                        
                        
                    }

                    long nJustPosX = aAreaParam.maAlignRect.Left();     
                    long nJustPosY = aAreaParam.maAlignRect.Top();
                    long nAvailWidth = aAreaParam.maAlignRect.GetWidth();
                    long nOutHeight = aAreaParam.maAlignRect.GetHeight();

                    bool bOutside = ( aAreaParam.maClipRect.Right() < nScrX || aAreaParam.maClipRect.Left() >= nScrX + nScrW );
                    if ( aAreaParam.maClipRect.Left() < nScrX )
                    {
                        aAreaParam.maClipRect.Left() = nScrX;
                        aAreaParam.mbLeftClip = true;
                    }
                    if ( aAreaParam.maClipRect.Right() > nScrX + nScrW )
                    {
                        aAreaParam.maClipRect.Right() = nScrX + nScrW;          
                        aAreaParam.mbRightClip = true;
                    }

                    bool bHClip = aAreaParam.mbLeftClip || aAreaParam.mbRightClip;
                    bool bVClip = false;

                    if ( aAreaParam.maClipRect.Top() < nScrY )
                    {
                        aAreaParam.maClipRect.Top() = nScrY;
                        bVClip = true;
                    }
                    if ( aAreaParam.maClipRect.Bottom() > nScrY + nScrH )
                    {
                        aAreaParam.maClipRect.Bottom() = nScrY + nScrH;         
                        bVClip = true;
                    }

                    //
                    
                    //

                    bool bRightAdjusted = false;        
                    bool bNeedEditEngine = false;
                    if ( !bNeedEditEngine && !bOutside )
                    {
                        switch (eOutHorJust)
                        {
                            case SVX_HOR_JUSTIFY_LEFT:
                                nJustPosX += (long) ( aVars.GetLeftTotal() * mnPPTX );
                                break;
                            case SVX_HOR_JUSTIFY_RIGHT:
                                nJustPosX += nAvailWidth - aVars.GetTextSize().Width() -
                                            (long) ( aVars.GetRightTotal() * mnPPTX );
                                bRightAdjusted = true;
                                break;
                            case SVX_HOR_JUSTIFY_CENTER:
                                nJustPosX += ( nAvailWidth - aVars.GetTextSize().Width() +
                                            (long) ( aVars.GetLeftTotal() * mnPPTX ) -
                                            (long) ( aVars.GetMargin()->GetRightMargin() * mnPPTX ) ) / 2;
                                break;
                            default:
                            {
                                
                            }
                        }

                        long nTestClipHeight = aVars.GetTextSize().Height();
                        switch (aVars.GetVerJust())
                        {
                            case SVX_VER_JUSTIFY_TOP:
                            case SVX_VER_JUSTIFY_BLOCK:
                                {
                                    long nTop = (long)( aVars.GetMargin()->GetTopMargin() * mnPPTY );
                                    nJustPosY += nTop;
                                    nTestClipHeight += nTop;
                                }
                                break;
                            case SVX_VER_JUSTIFY_BOTTOM:
                                {
                                    long nBot = (long)( aVars.GetMargin()->GetBottomMargin() * mnPPTY );
                                    nJustPosY += nOutHeight - aVars.GetTextSize().Height() - nBot;
                                    nTestClipHeight += nBot;
                                }
                                break;
                            case SVX_VER_JUSTIFY_CENTER:
                                {
                                    long nTop = (long)( aVars.GetMargin()->GetTopMargin() * mnPPTY );
                                    long nBot = (long)( aVars.GetMargin()->GetBottomMargin() * mnPPTY );
                                    nJustPosY += ( nOutHeight + nTop -
                                                    aVars.GetTextSize().Height() - nBot ) / 2;
                                    nTestClipHeight += std::abs( nTop - nBot );
                                }
                                break;
                            default:
                            {
                                
                            }
                        }

                        if ( nTestClipHeight > nOutHeight )
                        {
                            
                            
                            if ( eType != OUTTYPE_PRINTER ||
                                    ( mpDoc->GetRowFlags( nCellY, nTab ) & CR_MANUALSIZE ) ||
                                    ( aVars.HasCondHeight() ) )
                                bVClip = true;
                        }

                        if ( bHClip || bVClip )
                        {
                            
                            
                            

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

                            
                            
                            if (bPixelToLogic)
                                aAreaParam.maClipRect = mpRefDevice->PixelToLogic( aAreaParam.maClipRect );

                            if (bMetaFile)
                            {
                                mpDev->Push();
                                mpDev->IntersectClipRegion( aAreaParam.maClipRect );
                            }
                            else
                                mpDev->SetClipRegion( Region( aAreaParam.maClipRect ) );
                        }

                        Point aURLStart( nJustPosX, nJustPosY );    

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
                                
                            }
                        }

                        
                        

                        Point aDrawTextPos( nJustPosX, nJustPosY );
                        if ( bPixelToLogic )
                        {
                            
                            if (bRightAdjusted)
                                aDrawTextPos.X() += aVars.GetTextSize().Width();

                            aDrawTextPos = mpRefDevice->PixelToLogic( aDrawTextPos );

                            
                            if (bRightAdjusted)
                                aDrawTextPos.X() -= aVars.GetOriginalWidth();
                        }

                        
                        

                        OUString aString = aVars.GetString();
                        if (bMetaFile || pFmtDevice != mpDev || aZoomX != aZoomY)
                        {
                            sal_Int32* pDX = new sal_Int32[aString.getLength()];
                            pFmtDevice->GetTextArray( aString, pDX );

                            if ( !mpRefDevice->GetConnectMetaFile() ||
                                    mpRefDevice->GetOutDevType() == OUTDEV_PRINTER )
                            {
                                double fMul = GetStretch();
                                sal_Int32 nLen = aString.getLength();
                                for( sal_Int32 i = 0; i<nLen; i++ )
                                    pDX[i] = (long)(pDX[i] / fMul + 0.5);
                            }

                            mpDev->DrawTextArray( aDrawTextPos, aString, pDX );
                            delete[] pDX;
                        }
                        else
                            mpDev->DrawText( aDrawTextPos, aString );

                        if ( bHClip || bVClip )
                        {
                            if (bMetaFile)
                                mpDev->Pop();
                            else
                                mpDev->SetClipRegion();
                        }

                        
                        bool bHasURL = pPDFData && aCell.meType == CELLTYPE_FORMULA && aCell.mpFormula->IsHyperLinkCell();
                        if ( bHasURL )
                        {
                            Rectangle aURLRect( aURLStart, aVars.GetTextSize() );
                            lcl_DoHyperlinkResult(mpDev, aURLRect, aCell);
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
    mpDoc->EnableIdle(bWasIdleEnabled);
}



ScFieldEditEngine* ScOutputData::CreateOutputEditEngine()
{
    ScFieldEditEngine* pEngine = new ScFieldEditEngine(mpDoc, mpDoc->GetEnginePool());
    pEngine->SetUpdateMode( false );
    
    pEngine->SetRefDevice( pFmtDevice );
    sal_uLong nCtrl = pEngine->GetControlWord();
    if ( bShowSpellErrors )
        nCtrl |= EE_CNTRL_ONLINESPELLING;
    if ( eType == OUTTYPE_PRINTER )
        nCtrl &= ~EE_CNTRL_MARKFIELDS;
    if ( eType == OUTTYPE_WINDOW && mpRefDevice == pFmtDevice )
        nCtrl &= ~EE_CNTRL_FORMAT100;       
    pEngine->SetControlWord( nCtrl );
    mpDoc->ApplyAsianEditSettings( *pEngine );
    pEngine->EnableAutoColor( mbUseStyleColor );
    pEngine->SetDefaultHorizontalTextDirection( (EEHorizontalTextDirection)mpDoc->GetEditTextDirection( nTab ) );
    return pEngine;
}

static void lcl_ClearEdit( EditEngine& rEngine )       
{
    rEngine.SetUpdateMode( false );

    rEngine.SetText(EMPTY_OUSTRING);
    
    const SfxItemSet& rPara = rEngine.GetParaAttribs(0);
    if (rPara.Count())
        rEngine.SetParaAttribs( 0,
                    SfxItemSet( *rPara.GetPool(), rPara.GetRanges() ) );
}

static bool lcl_SafeIsValue( ScRefCellValue& rCell )
{
    switch (rCell.meType)
    {
        case CELLTYPE_VALUE:
            return true;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFCell = rCell.mpFormula;
            if (pFCell->IsRunning() || pFCell->IsValue())
                return true;
        }
        break;
        default:
        {
            
        }
    }
    return false;
}

static void lcl_ScaleFonts( EditEngine& rEngine, long nPercent )
{
    bool bUpdateMode = rEngine.GetUpdateMode();
    if ( bUpdateMode )
        rEngine.SetUpdateMode( false );

    sal_Int32 nParCount = rEngine.GetParagraphCount();
    for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
    {
        std::vector<sal_Int32> aPortions;
        rEngine.GetPortions( nPar, aPortions );

        sal_Int32 nStart = 0;
        for ( std::vector<sal_Int32>::const_iterator it(aPortions.begin()); it != aPortions.end(); ++it )
        {
            sal_Int32 nEnd = *it;
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

            rEngine.QuickSetAttribs( aAttribs, aSel );      

            nStart = nEnd;
        }
    }

    if ( bUpdateMode )
        rEngine.SetUpdateMode( true );
}

static long lcl_GetEditSize( EditEngine& rEngine, bool bWidth, bool bSwap, long nAttrRotate )
{
    if ( bSwap )
        bWidth = !bWidth;

    if ( nAttrRotate )
    {
        long nRealWidth  = (long) rEngine.CalcTextWidth();
        long nRealHeight = rEngine.GetTextHeight();

        

        double nRealOrient = nAttrRotate * F_PI18000;   
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
            bool bWidth, sal_uInt16 nOrient, long nAttrRotate, bool bPixelToLogic,
            long& rEngineWidth, long& rEngineHeight, long& rNeededPixel, bool& rLeftClip, bool& rRightClip )
{
    if ( !bWidth )
    {
        

        long nScaleSize = bPixelToLogic ?
            mpRefDevice->LogicToPixel(Size(0,rEngineHeight)).Height() : rEngineHeight;

        
        
        if ( nScaleSize <= rAlignRect.GetHeight() )
            return;

        bool bSwap = ( nOrient == SVX_ORIENTATION_TOPBOTTOM || nOrient == SVX_ORIENTATION_BOTTOMTOP );
        long nAvailable = rAlignRect.GetHeight() - nTopM - nBottomM;
        long nScale = ( nAvailable * 100 ) / nScaleSize;

        lcl_ScaleFonts( rEngine, nScale );
        rEngineHeight = lcl_GetEditSize( rEngine, false, bSwap, nAttrRotate );
        long nNewSize = bPixelToLogic ?
            mpRefDevice->LogicToPixel(Size(0,rEngineHeight)).Height() : rEngineHeight;

        sal_uInt16 nShrinkAgain = 0;
        while ( nNewSize > nAvailable && nShrinkAgain < SC_SHRINKAGAIN_MAX )
        {
            
            lcl_ScaleFonts( rEngine, 90 );     
            rEngineHeight = lcl_GetEditSize( rEngine, false, bSwap, nAttrRotate );
            nNewSize = bPixelToLogic ?
                mpRefDevice->LogicToPixel(Size(0,rEngineHeight)).Height() : rEngineHeight;
            ++nShrinkAgain;
        }

        
        rEngineWidth = lcl_GetEditSize( rEngine, true, bSwap, nAttrRotate );
        long nPixelWidth = bPixelToLogic ?
            mpRefDevice->LogicToPixel(Size(rEngineWidth,0)).Width() : rEngineWidth;
        rNeededPixel = nPixelWidth + nLeftM + nRightM;
    }
    else if ( rLeftClip || rRightClip )
    {
        

        long nAvailable = rAlignRect.GetWidth() - nLeftM - nRightM;
        long nScaleSize = rNeededPixel - nLeftM - nRightM;      

        if ( nScaleSize <= nAvailable )
            return;

        long nScale = ( nAvailable * 100 ) / nScaleSize;

        lcl_ScaleFonts( rEngine, nScale );
        rEngineWidth = lcl_GetEditSize( rEngine, true, false, nAttrRotate );
        long nNewSize = bPixelToLogic ?
            mpRefDevice->LogicToPixel(Size(rEngineWidth,0)).Width() : rEngineWidth;

        sal_uInt16 nShrinkAgain = 0;
        while ( nNewSize > nAvailable && nShrinkAgain < SC_SHRINKAGAIN_MAX )
        {
            
            lcl_ScaleFonts( rEngine, 90 );     
            rEngineWidth = lcl_GetEditSize( rEngine, true, false, nAttrRotate );
            nNewSize = bPixelToLogic ?
                mpRefDevice->LogicToPixel(Size(rEngineWidth,0)).Width() : rEngineWidth;
            ++nShrinkAgain;
        }
        if ( nNewSize <= nAvailable )
            rLeftClip = rRightClip = false;

        
        rNeededPixel = nNewSize + nLeftM + nRightM;
        rEngineHeight = lcl_GetEditSize( rEngine, false, false, nAttrRotate );
    }
}

ScOutputData::DrawEditParam::DrawEditParam(const ScPatternAttr* pPattern, const SfxItemSet* pCondSet, bool bCellIsValue) :
    meHorJustAttr( lcl_GetValue<SvxHorJustifyItem, SvxCellHorJustify>(*pPattern, ATTR_HOR_JUSTIFY, pCondSet) ),
    meHorJustContext( meHorJustAttr ),
    meHorJustResult( meHorJustAttr ),
    meVerJust( lcl_GetValue<SvxVerJustifyItem, SvxCellVerJustify>(*pPattern, ATTR_VER_JUSTIFY, pCondSet) ),
    meHorJustMethod( lcl_GetValue<SvxJustifyMethodItem, SvxCellJustifyMethod>(*pPattern, ATTR_HOR_JUSTIFY_METHOD, pCondSet) ),
    meVerJustMethod( lcl_GetValue<SvxJustifyMethodItem, SvxCellJustifyMethod>(*pPattern, ATTR_VER_JUSTIFY_METHOD, pCondSet) ),
    meOrient( pPattern->GetCellOrientation(pCondSet) ),
    mnArrY(0),
    mnX(0), mnY(0), mnCellX(0), mnCellY(0), mnTab(0),
    mnPosX(0), mnPosY(0), mnInitPosX(0),
    mbBreak( (meHorJustAttr == SVX_HOR_JUSTIFY_BLOCK) || lcl_GetBoolValue(*pPattern, ATTR_LINEBREAK, pCondSet) ),
    mbCellIsValue(bCellIsValue),
    mbAsianVertical(false),
    mbPixelToLogic(false),
    mbHyphenatorSet(false),
    mpEngine(NULL),
    mpPattern(pPattern),
    mpCondSet(pCondSet),
    mpPreviewFontSet(NULL),
    mpOldPattern(NULL),
    mpOldCondSet(NULL),
    mpOldPreviewFontSet(NULL),
    mpThisRowInfo(NULL),
    mpMisspellRanges(NULL)
{}

bool ScOutputData::DrawEditParam::readCellContent(
    ScDocument* pDoc, bool bShowNullValues, bool bShowFormulas, bool bSyntaxMode, bool bUseStyleColor, bool bForceAutoColor, bool& rWrapFields)
{
    if (maCell.meType == CELLTYPE_EDIT)
    {
        const EditTextObject* pData = maCell.mpEditText;
        if (pData)
        {
            mpEngine->SetText(*pData);

            if ( mbBreak && !mbAsianVertical && pData->HasField() )
            {
                
                

                rWrapFields = true;
            }
        }
        else
        {
            OSL_FAIL("pData == 0");
            return false;
        }
    }
    else
    {
        sal_uLong nFormat = mpPattern->GetNumberFormat(
                                    pDoc->GetFormatTable(), mpCondSet );
        OUString aString;
        Color* pColor;
        ScCellFormat::GetString( maCell,
                                 nFormat,aString, &pColor,
                                 *pDoc->GetFormatTable(),
                                 pDoc,
                                 bShowNullValues,
                                 bShowFormulas,
                                 ftCheck );

        mpEngine->SetText(aString);
        if ( pColor && !bSyntaxMode && !( bUseStyleColor && bForceAutoColor ) )
            lcl_SetEditColor( *mpEngine, *pColor );
    }

    if (mpMisspellRanges)
        mpEngine->SetAllMisspellRanges(*mpMisspellRanges);

    return true;
}

void ScOutputData::DrawEditParam::setPatternToEngine(bool bUseStyleColor)
{
    
    

    if (mpPattern == mpOldPattern && mpCondSet == mpOldCondSet && mpPreviewFontSet == mpOldPreviewFontSet )
        return;

    sal_Int32 nConfBackColor = SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    bool bCellContrast = bUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    SfxItemSet* pSet = new SfxItemSet( mpEngine->GetEmptyItemSet() );
    mpPattern->FillEditItemSet( pSet, mpCondSet );
    if ( mpPreviewFontSet )
    {
        const SfxPoolItem* pItem;
        if ( mpPreviewFontSet->GetItemState( ATTR_FONT, true, &pItem ) == SFX_ITEM_SET )
        {
            SvxFontItem aFontItem(EE_CHAR_FONTINFO);
            aFontItem = (const SvxFontItem&)*pItem;
            pSet->Put( aFontItem );
        }
        if ( mpPreviewFontSet->GetItemState( ATTR_CJK_FONT, true, &pItem ) == SFX_ITEM_SET )
        {
            SvxFontItem aCjkFontItem(EE_CHAR_FONTINFO_CJK);
            aCjkFontItem = (const SvxFontItem&)*pItem;
            pSet->Put( aCjkFontItem );
        }
        if ( mpPreviewFontSet->GetItemState( ATTR_CTL_FONT, true, &pItem ) == SFX_ITEM_SET )
        {
            SvxFontItem aCtlFontItem(EE_CHAR_FONTINFO_CTL);
            aCtlFontItem = (const SvxFontItem&)*pItem;
            pSet->Put( aCtlFontItem );
        }
    }
    mpEngine->SetDefaults( pSet );
    mpOldPattern = mpPattern;
    mpOldCondSet = mpCondSet;
    mpOldPreviewFontSet = mpPreviewFontSet;

    sal_uLong nControl = mpEngine->GetControlWord();
    if (meOrient == SVX_ORIENTATION_STACKED)
        nControl |= EE_CNTRL_ONECHARPERLINE;
    else
        nControl &= ~EE_CNTRL_ONECHARPERLINE;
    mpEngine->SetControlWord( nControl );

    if ( !mbHyphenatorSet && ((const SfxBoolItem&)pSet->Get(EE_PARA_HYPHENATE)).GetValue() )
    {
        
        com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
        mpEngine->SetHyphenator( xXHyphenator );
        mbHyphenatorSet = true;
    }

    Color aBackCol = ((const SvxBrushItem&)mpPattern->GetItem( ATTR_BACKGROUND, mpCondSet )).GetColor();
    if ( bUseStyleColor && ( aBackCol.GetTransparency() > 0 || bCellContrast ) )
        aBackCol.SetColor( nConfBackColor );
    mpEngine->SetBackgroundColor( aBackCol );
}

void ScOutputData::DrawEditParam::calcMargins(long& rTopM, long& rLeftM, long& rBottomM, long& rRightM, double nPPTX, double nPPTY) const
{
    const SvxMarginItem& rMargin =
        static_cast<const SvxMarginItem&>(mpPattern->GetItem(ATTR_MARGIN, mpCondSet));

    sal_uInt16 nIndent = 0;
    if (meHorJustAttr == SVX_HOR_JUSTIFY_LEFT || meHorJustAttr == SVX_HOR_JUSTIFY_RIGHT)
        nIndent = lcl_GetValue<SfxUInt16Item, sal_uInt16>(*mpPattern, ATTR_INDENT, mpCondSet);

    rLeftM   = static_cast<long>(((rMargin.GetLeftMargin() + nIndent) * nPPTX));
    rTopM    = static_cast<long>((rMargin.GetTopMargin() * nPPTY));
    rRightM  = static_cast<long>((rMargin.GetRightMargin() * nPPTX));
    rBottomM = static_cast<long>((rMargin.GetBottomMargin() * nPPTY));
    if(meHorJustAttr == SVX_HOR_JUSTIFY_RIGHT)
    {
        rLeftM   = static_cast<long>((rMargin.GetLeftMargin()  * nPPTX));
        rRightM  = static_cast<long>(((rMargin.GetRightMargin() + nIndent) * nPPTX));
    }
}

void ScOutputData::DrawEditParam::calcPaperSize(
    Size& rPaperSize, const Rectangle& rAlignRect, double nPPTX, double nPPTY) const
{
    long nTopM, nLeftM, nBottomM, nRightM;
    calcMargins(nTopM, nLeftM, nBottomM, nRightM, nPPTX, nPPTY);

    if (isVerticallyOriented())
    {
        rPaperSize.Width() = rAlignRect.GetHeight() - nTopM - nBottomM;
        rPaperSize.Height() = rAlignRect.GetWidth() - nLeftM - nRightM;
    }
    else
    {
        rPaperSize.Width() = rAlignRect.GetWidth() - nLeftM - nRightM;
        rPaperSize.Height() = rAlignRect.GetHeight() - nTopM - nBottomM;
    }

    if (mbAsianVertical)
    {
        rPaperSize.Height() = rAlignRect.GetHeight() - nTopM - nBottomM;
        
        
        
        rPaperSize.Height() -= 5;
    }
}

void ScOutputData::DrawEditParam::getEngineSize(ScFieldEditEngine* pEngine, long& rWidth, long& rHeight) const
{
    long nEngineWidth = 0;
    if (!mbBreak || meOrient == SVX_ORIENTATION_STACKED || mbAsianVertical)
        nEngineWidth = static_cast<long>(pEngine->CalcTextWidth());

    long nEngineHeight = pEngine->GetTextHeight();

    if (isVerticallyOriented())
    {
        long nTemp = nEngineWidth;
        nEngineWidth = nEngineHeight;
        nEngineHeight = nTemp;
    }

    if (meOrient == SVX_ORIENTATION_STACKED)
        nEngineWidth = nEngineWidth * 11 / 10;

    rWidth = nEngineWidth;
    rHeight = nEngineHeight;
}

bool ScOutputData::DrawEditParam::hasLineBreak() const
{
    return (mbBreak || (meOrient == SVX_ORIENTATION_STACKED) || mbAsianVertical);
}

bool ScOutputData::DrawEditParam::isHyperlinkCell() const
{
    if (maCell.meType != CELLTYPE_FORMULA)
        return false;

    return maCell.mpFormula->IsHyperLinkCell();
}

bool ScOutputData::DrawEditParam::isVerticallyOriented() const
{
    return (meOrient == SVX_ORIENTATION_TOPBOTTOM || meOrient == SVX_ORIENTATION_BOTTOMTOP);
}

void ScOutputData::DrawEditParam::calcStartPosForVertical(
    Point& rLogicStart, long nCellWidth, long nEngineWidth, long nTopM, OutputDevice* pRefDevice)
{
    OSL_ENSURE(isVerticallyOriented(), "Use this only for vertically oriented cell!");

    if (mbPixelToLogic)
        rLogicStart = pRefDevice->PixelToLogic(rLogicStart);

    if (mbBreak)
    {
        
        if (mbPixelToLogic)
            rLogicStart.Y() += pRefDevice->PixelToLogic(Size(0,nTopM)).Height();
        else
            rLogicStart.Y() += nTopM;

        switch (meHorJustResult)
        {
            case SVX_HOR_JUSTIFY_CENTER:
                rLogicStart.X() += (nCellWidth - nEngineWidth) / 2;
            break;
            case SVX_HOR_JUSTIFY_RIGHT:
                rLogicStart.X() += nCellWidth - nEngineWidth;
            break;
            default:
                ; 
        }
    }
}

void ScOutputData::DrawEditParam::setAlignmentToEngine()
{
    if (isVerticallyOriented() || mbAsianVertical)
    {
        SvxAdjust eSvxAdjust = SVX_ADJUST_LEFT;
        switch (meVerJust)
        {
            case SVX_VER_JUSTIFY_TOP:
                eSvxAdjust = (meOrient == SVX_ORIENTATION_TOPBOTTOM || mbAsianVertical) ?
                            SVX_ADJUST_LEFT : SVX_ADJUST_RIGHT;
                break;
            case SVX_VER_JUSTIFY_CENTER:
                eSvxAdjust = SVX_ADJUST_CENTER;
                break;
            case SVX_VER_JUSTIFY_BOTTOM:
            case SVX_VER_JUSTIFY_STANDARD:
                eSvxAdjust = (meOrient == SVX_ORIENTATION_TOPBOTTOM || mbAsianVertical) ?
                            SVX_ADJUST_RIGHT : SVX_ADJUST_LEFT;
                break;
            case SVX_VER_JUSTIFY_BLOCK:
                eSvxAdjust = SVX_ADJUST_BLOCK;
                break;
        }

        mpEngine->SetDefaultItem( SvxAdjustItem(eSvxAdjust, EE_PARA_JUST) );
        mpEngine->SetDefaultItem( SvxJustifyMethodItem(meVerJustMethod, EE_PARA_JUST_METHOD) );

        if (meHorJustResult == SVX_HOR_JUSTIFY_BLOCK)
            mpEngine->SetDefaultItem( SvxVerJustifyItem(SVX_VER_JUSTIFY_BLOCK, EE_PARA_VER_JUST) );
    }
    else
    {
        
        
        

        SvxAdjust eSvxAdjust = SVX_ADJUST_LEFT;
        if (meOrient == SVX_ORIENTATION_STACKED)
            eSvxAdjust = SVX_ADJUST_CENTER;
        else if (mbBreak)
        {
            if (meOrient == SVX_ORIENTATION_STANDARD)
                switch (meHorJustResult)
                {
                    case SVX_HOR_JUSTIFY_REPEAT:            
                    case SVX_HOR_JUSTIFY_STANDARD:
                        assert(!"meHorJustResult does not match getAlignmentFromContext()");
                        
                    case SVX_HOR_JUSTIFY_LEFT:
                        eSvxAdjust = SVX_ADJUST_LEFT;
                        break;
                    case SVX_HOR_JUSTIFY_CENTER:
                        eSvxAdjust = SVX_ADJUST_CENTER;
                        break;
                    case SVX_HOR_JUSTIFY_RIGHT:
                        eSvxAdjust = SVX_ADJUST_RIGHT;
                        break;
                    case SVX_HOR_JUSTIFY_BLOCK:
                        eSvxAdjust = SVX_ADJUST_BLOCK;
                        break;
                }
            else
                switch (meVerJust)
                {
                    case SVX_VER_JUSTIFY_TOP:
                        eSvxAdjust = SVX_ADJUST_RIGHT;
                        break;
                    case SVX_VER_JUSTIFY_CENTER:
                        eSvxAdjust = SVX_ADJUST_CENTER;
                        break;
                    case SVX_VER_JUSTIFY_BOTTOM:
                    case SVX_VER_JUSTIFY_STANDARD:
                        eSvxAdjust = SVX_ADJUST_LEFT;
                        break;
                    case SVX_VER_JUSTIFY_BLOCK:
                        eSvxAdjust = SVX_ADJUST_BLOCK;
                        break;
                }
        }

        mpEngine->SetDefaultItem( SvxAdjustItem(eSvxAdjust, EE_PARA_JUST) );

        if (mbAsianVertical)
        {
            mpEngine->SetDefaultItem( SvxJustifyMethodItem(meVerJustMethod, EE_PARA_JUST_METHOD) );
            if (meHorJustResult == SVX_HOR_JUSTIFY_BLOCK)
                mpEngine->SetDefaultItem( SvxVerJustifyItem(SVX_VER_JUSTIFY_BLOCK, EE_PARA_VER_JUST) );
        }
        else
        {
            mpEngine->SetDefaultItem( SvxJustifyMethodItem(meHorJustMethod, EE_PARA_JUST_METHOD) );
            if (meVerJust == SVX_VER_JUSTIFY_BLOCK)
                mpEngine->SetDefaultItem( SvxVerJustifyItem(SVX_VER_JUSTIFY_BLOCK, EE_PARA_VER_JUST) );
        }
    }

    mpEngine->SetVertical(mbAsianVertical);
    if (maCell.meType == CELLTYPE_EDIT)
    {
        
        
        
        const EditTextObject* pData = maCell.mpEditText;
        if (pData)
            const_cast<EditTextObject*>(pData)->SetVertical(mbAsianVertical);
    }
}

bool ScOutputData::DrawEditParam::adjustHorAlignment(ScFieldEditEngine* pEngine)
{
    if (meHorJustResult == SVX_HOR_JUSTIFY_RIGHT || meHorJustResult == SVX_HOR_JUSTIFY_CENTER)
    {
        SvxAdjust eEditAdjust = (meHorJustResult == SVX_HOR_JUSTIFY_CENTER) ?
            SVX_ADJUST_CENTER : SVX_ADJUST_RIGHT;

        pEngine->SetUpdateMode(false);
        pEngine->SetDefaultItem( SvxAdjustItem(eEditAdjust, EE_PARA_JUST) );
        pEngine->SetUpdateMode(true);
        return true;
    }
    return false;
}

void ScOutputData::DrawEditParam::adjustForRTL()
{
    if (!mpEngine->IsRightToLeft(0))
        
        return;

    
    
    
    Size aLogicPaper = mpEngine->GetPaperSize();
    if ( aLogicPaper.Width() > USHRT_MAX )
    {
        aLogicPaper.Width() = USHRT_MAX;
        mpEngine->SetPaperSize(aLogicPaper);
    }
}

void ScOutputData::DrawEditParam::adjustForHyperlinkInPDF(Point aURLStart, OutputDevice* pDev)
{
    
    vcl::PDFExtOutDevData* pPDFData = PTR_CAST( vcl::PDFExtOutDevData, pDev->GetExtOutDevData() );
    bool bHasURL = pPDFData && isHyperlinkCell();
    if (!bHasURL)
        return;

    long nURLWidth = (long) mpEngine->CalcTextWidth();
    long nURLHeight = mpEngine->GetTextHeight();
    if (mbBreak)
    {
        Size aPaper = mpEngine->GetPaperSize();
        if ( mbAsianVertical )
            nURLHeight = aPaper.Height();
        else
            nURLWidth = aPaper.Width();
    }
    if (isVerticallyOriented())
        std::swap( nURLWidth, nURLHeight );
    else if (mbAsianVertical)
        aURLStart.X() -= nURLWidth;

    Rectangle aURLRect( aURLStart, Size( nURLWidth, nURLHeight ) );
    lcl_DoHyperlinkResult(pDev, aURLRect, maCell);
}

void ScOutputData::DrawEditStandard(DrawEditParam& rParam)
{
    OSL_ASSERT(rParam.meOrient == SVX_ORIENTATION_STANDARD);
    OSL_ASSERT(!rParam.mbAsianVertical);

    Size aRefOne = mpRefDevice->PixelToLogic(Size(1,1));

    bool bHidden = false;
    bool bRepeat = (rParam.meHorJustAttr == SVX_HOR_JUSTIFY_REPEAT && !rParam.mbBreak);
    bool bShrink = !rParam.mbBreak && !bRepeat && lcl_GetBoolValue(*rParam.mpPattern, ATTR_SHRINKTOFIT, rParam.mpCondSet);
    long nAttrRotate = lcl_GetValue<SfxInt32Item, long>(*rParam.mpPattern, ATTR_ROTATE_VALUE, rParam.mpCondSet);

    if ( rParam.meHorJustAttr == SVX_HOR_JUSTIFY_REPEAT )
    {
        
        rParam.meOrient = SVX_ORIENTATION_STANDARD;
        nAttrRotate = 0;

        
        
        
        
        if ( rParam.mbBreak )
            rParam.meHorJustResult = rParam.meHorJustContext;
    }

    if (nAttrRotate)
    {
        
        
        bHidden = true;     
    }

    if (bHidden)
        return;

    SvxCellHorJustify eOutHorJust = rParam.meHorJustContext;

    
    
    long nTopM, nLeftM, nBottomM, nRightM;
    rParam.calcMargins(nTopM, nLeftM, nBottomM, nRightM, mnPPTX, mnPPTY);

    SCCOL nXForPos = rParam.mnX;
    if ( nXForPos < nX1 )
    {
        nXForPos = nX1;
        rParam.mnPosX = rParam.mnInitPosX;
    }
    SCSIZE nArrYForPos = rParam.mnArrY;
    if ( nArrYForPos < 1 )
    {
        nArrYForPos = 1;
        rParam.mnPosY = nScrY;
    }

    OutputAreaParam aAreaParam;

    //
    
    //

    Size aPaperSize = Size( 1000000, 1000000 );
    if (rParam.mbBreak)
    {
        

        
        GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, 0,
                       *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                       rParam.mbCellIsValue, true, false, aAreaParam );

        
        rParam.calcPaperSize(aPaperSize, aAreaParam.maAlignRect, mnPPTX, mnPPTY);
    }
    if (rParam.mbPixelToLogic)
    {
        Size aLogicSize = mpRefDevice->PixelToLogic(aPaperSize);
        if ( rParam.mbBreak && !rParam.mbAsianVertical && mpRefDevice != pFmtDevice )
        {
            
            

            Fraction aFract(1,1);
            Rectangle aUtilRect = ScEditUtil( mpDoc, rParam.mnCellX, rParam.mnCellY, nTab, Point(0,0), pFmtDevice,
                HMM_PER_TWIPS, HMM_PER_TWIPS, aFract, aFract ).GetEditArea( rParam.mpPattern, false );
            aLogicSize.Width() = aUtilRect.GetWidth();
        }
        rParam.mpEngine->SetPaperSize(aLogicSize);
    }
    else
        rParam.mpEngine->SetPaperSize(aPaperSize);

    //
    
    //

    
    if ( rParam.mbAsianVertical && rParam.meVerJust == SVX_VER_JUSTIFY_STANDARD )
        rParam.meVerJust = SVX_VER_JUSTIFY_TOP;

    rParam.setPatternToEngine(mbUseStyleColor);
    rParam.setAlignmentToEngine();

    

    bool bWrapFields = false;
    if (!rParam.readCellContent(mpDoc, mbShowNullValues, mbShowFormulas, mbSyntaxMode, mbUseStyleColor, mbForceAutoColor, bWrapFields))
        
        return;

    if ( mbSyntaxMode )
        SetEditSyntaxColor(*rParam.mpEngine, rParam.maCell);
    else if ( mbUseStyleColor && mbForceAutoColor )
        lcl_SetEditColor( *rParam.mpEngine, COL_AUTO );     

    rParam.mpEngine->SetUpdateMode( true );     

    //
    
    //

    long nEngineWidth, nEngineHeight;
    rParam.getEngineSize(rParam.mpEngine, nEngineWidth, nEngineHeight);

    long nNeededPixel = nEngineWidth;
    if (rParam.mbPixelToLogic)
        nNeededPixel = mpRefDevice->LogicToPixel(Size(nNeededPixel,0)).Width();
    nNeededPixel += nLeftM + nRightM;

    if (!rParam.mbBreak || bShrink)
    {
        
        GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, nNeededPixel,
                       *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                       rParam.mbCellIsValue || bRepeat || bShrink, false, false, aAreaParam );

        if ( bShrink )
        {
            ShrinkEditEngine( *rParam.mpEngine, aAreaParam.maAlignRect,
                nLeftM, nTopM, nRightM, nBottomM, true,
                sal::static_int_cast<sal_uInt16>(rParam.meOrient), 0, rParam.mbPixelToLogic,
                nEngineWidth, nEngineHeight, nNeededPixel,
                aAreaParam.mbLeftClip, aAreaParam.mbRightClip );
        }
        if ( bRepeat && !aAreaParam.mbLeftClip && !aAreaParam.mbRightClip && rParam.mpEngine->GetParagraphCount() == 1 )
        {
            
            

            long nFormatted = nNeededPixel - nLeftM - nRightM;      
            long nAvailable = aAreaParam.maAlignRect.GetWidth() - nLeftM - nRightM;
            if ( nAvailable >= 2 * nFormatted )
            {
                
                OUString aCellStr = rParam.mpEngine->GetText();
                rParam.mpEngine->SetText( aCellStr );

                long nRepeatSize = (long) rParam.mpEngine->CalcTextWidth();
                if (rParam.mbPixelToLogic)
                    nRepeatSize = mpRefDevice->LogicToPixel(Size(nRepeatSize,0)).Width();
                if ( pFmtDevice != mpRefDevice )
                    ++nRepeatSize;
                if ( nRepeatSize > 0 )
                {
                    long nRepeatCount = nAvailable / nRepeatSize;
                    if ( nRepeatCount > 1 )
                    {
                        OUString aRepeated = aCellStr;
                        for ( long nRepeat = 1; nRepeat < nRepeatCount; nRepeat++ )
                            aRepeated += aCellStr;
                        rParam.mpEngine->SetText( aRepeated );

                        nEngineHeight = rParam.mpEngine->GetTextHeight();
                        nEngineWidth = (long) rParam.mpEngine->CalcTextWidth();
                        if (rParam.mbPixelToLogic)
                            nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
                        else
                            nNeededPixel = nEngineWidth;
                        nNeededPixel += nLeftM + nRightM;
                    }
                }
            }
        }

        if ( rParam.mbCellIsValue && ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip ) )
        {
            rParam.mpEngine->SetText(OUString("###"));
            nEngineWidth = (long) rParam.mpEngine->CalcTextWidth();
            if (rParam.mbPixelToLogic)
                nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
            else
                nNeededPixel = nEngineWidth;
            nNeededPixel += nLeftM + nRightM;

            
        }

        if (eOutHorJust != SVX_HOR_JUSTIFY_LEFT)
        {
            aPaperSize.Width() = nNeededPixel + 1;
            if (rParam.mbPixelToLogic)
                rParam.mpEngine->SetPaperSize(mpRefDevice->PixelToLogic(aPaperSize));
            else
                rParam.mpEngine->SetPaperSize(aPaperSize);
        }
    }

    long nStartX = aAreaParam.maAlignRect.Left();
    long nStartY = aAreaParam.maAlignRect.Top();
    long nCellWidth = aAreaParam.maAlignRect.GetWidth();
    long nOutWidth = nCellWidth - 1 - nLeftM - nRightM;
    long nOutHeight = aAreaParam.maAlignRect.GetHeight() - nTopM - nBottomM;

    if (rParam.mbBreak)
    {
        
        
        

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

    bool bOutside = (aAreaParam.maClipRect.Right() < nScrX || aAreaParam.maClipRect.Left() >= nScrX + nScrW);
    if (bOutside)
        return;

    if ( aAreaParam.maClipRect.Left() < nScrX )
    {
        aAreaParam.maClipRect.Left() = nScrX;
        aAreaParam.mbLeftClip = true;
    }
    if ( aAreaParam.maClipRect.Right() > nScrX + nScrW )
    {
        aAreaParam.maClipRect.Right() = nScrX + nScrW;          
        aAreaParam.mbRightClip = true;
    }

    bool bClip = aAreaParam.mbLeftClip || aAreaParam.mbRightClip;
    bool bSimClip = false;

    if ( bWrapFields )
    {
        
        bClip = true;
    }

    if ( aAreaParam.maClipRect.Top() < nScrY )
    {
        aAreaParam.maClipRect.Top() = nScrY;
        bClip = true;
    }
    if ( aAreaParam.maClipRect.Bottom() > nScrY + nScrH )
    {
        aAreaParam.maClipRect.Bottom() = nScrY + nScrH;     
        bClip = true;
    }

    Size aCellSize;         
    if (rParam.mbPixelToLogic)
        aCellSize = mpRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
    else
        aCellSize = Size( nOutWidth, nOutHeight );

    if ( nEngineHeight >= aCellSize.Height() + aRefOne.Height() )
    {
        const ScMergeAttr* pMerge =
                (ScMergeAttr*)&rParam.mpPattern->GetItem(ATTR_MERGE);
        bool bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

        
        
        
        if ( eType != OUTTYPE_PRINTER ||
            ( mpDoc->GetRowFlags( rParam.mnCellY, nTab ) & CR_MANUALSIZE ) ||
            ( rParam.mpCondSet && SFX_ITEM_SET ==
                rParam.mpCondSet->GetItemState(ATTR_FONT_HEIGHT, true) ) )
            bClip = true;
        else
            bSimClip = true;

        
        
        
        
        
        
        if ( nEngineHeight - aCellSize.Height() > 100 &&
             rParam.mbBreak && bMarkClipped &&
             ( rParam.mpEngine->GetParagraphCount() > 1 || rParam.mpEngine->GetLineCount(0) > 1 ) )
        {
            CellInfo* pClipMarkCell = NULL;
            if ( bMerged )
            {
                
                SCCOL nClipX = ( rParam.mnX < nX1 ) ? nX1 : rParam.mnX;
                pClipMarkCell = &pRowInfo[(rParam.mnArrY != 0) ? rParam.mnArrY : 1].pCellInfo[nClipX+1];
            }
            else
                pClipMarkCell = &rParam.mpThisRowInfo->pCellInfo[rParam.mnX+1];

            pClipMarkCell->nClipMark |= SC_CLIPMARK_RIGHT;      
            bAnyClipped = true;

            long nMarkPixel = (long)( SC_CLIPMARK_SIZE * mnPPTX );
            if ( aAreaParam.maClipRect.Right() - nMarkPixel > aAreaParam.maClipRect.Left() )
                aAreaParam.maClipRect.Right() -= nMarkPixel;
        }
    }

    Rectangle aLogicClip;
    if (bClip || bSimClip)
    {
        

        if (rParam.mbPixelToLogic)
            aLogicClip = mpRefDevice->PixelToLogic( aAreaParam.maClipRect );
        else
            aLogicClip = aAreaParam.maClipRect;

        if (bClip)  
        {
            if (bMetaFile)
            {
                mpDev->Push();
                mpDev->IntersectClipRegion( aLogicClip );
            }
            else
                mpDev->SetClipRegion( Region( aLogicClip ) );
        }
    }

    Point aLogicStart;
    if (rParam.mbPixelToLogic)
        aLogicStart = mpRefDevice->PixelToLogic( Point(nStartX,nStartY) );
    else
        aLogicStart = Point(nStartX, nStartY);

    if (!rParam.mbBreak)
    {
        
        if (rParam.adjustHorAlignment(rParam.mpEngine))
            
            rParam.mpOldPattern = NULL;
    }

    if (rParam.meVerJust==SVX_VER_JUSTIFY_BOTTOM ||
        rParam.meVerJust==SVX_VER_JUSTIFY_STANDARD)
    {
        
        

        if (rParam.mbPixelToLogic)
            aLogicStart.Y() += mpRefDevice->PixelToLogic( Size(0, nTopM +
                            mpRefDevice->LogicToPixel(aCellSize).Height() -
                            mpRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height()
                            )).Height();
        else
            aLogicStart.Y() += nTopM + aCellSize.Height() - nEngineHeight;
    }
    else if (rParam.meVerJust==SVX_VER_JUSTIFY_CENTER)
    {
        if (rParam.mbPixelToLogic)
            aLogicStart.Y() += mpRefDevice->PixelToLogic( Size(0, nTopM + (
                            mpRefDevice->LogicToPixel(aCellSize).Height() -
                            mpRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height() )
                            / 2)).Height();
        else
            aLogicStart.Y() += nTopM + (aCellSize.Height() - nEngineHeight) / 2;
    }
    else        
    {
        if (rParam.mbPixelToLogic)
            aLogicStart.Y() += mpRefDevice->PixelToLogic(Size(0,nTopM)).Height();
        else
            aLogicStart.Y() += nTopM;
    }

    Point aURLStart = aLogicStart;      

    rParam.adjustForRTL();

    
    

    if (bSimClip)
    {
        
        

        Point aDocStart = aLogicClip.TopLeft();
        aDocStart -= aLogicStart;
        rParam.mpEngine->Draw( mpDev, aLogicClip, aDocStart, false );
    }
    else
    {
        rParam.mpEngine->Draw(mpDev, aLogicStart, 0);
    }

    if (bClip)
    {
        if (bMetaFile)
            mpDev->Pop();
        else
            mpDev->SetClipRegion();
    }

    rParam.adjustForHyperlinkInPDF(aURLStart, mpDev);
}

void ScOutputData::ShowClipMarks( DrawEditParam& rParam, long nEngineHeight, const Size& aCellSize,
                                  bool bMerged, OutputAreaParam& aAreaParam)
{
    
    
    
    
    
    
    if ( nEngineHeight - aCellSize.Height() > 100 &&
            rParam.mbBreak && bMarkClipped &&
            ( rParam.mpEngine->GetParagraphCount() > 1 || rParam.mpEngine->GetLineCount(0) > 1 ) )
    {
        CellInfo* pClipMarkCell = NULL;
        if ( bMerged )
        {
            
            SCCOL nClipX = ( rParam.mnX < nX1 ) ? nX1 : rParam.mnX;
            pClipMarkCell = &pRowInfo[(rParam.mnArrY != 0) ? rParam.mnArrY : 1].pCellInfo[nClipX+1];
        }
        else
            pClipMarkCell = &rParam.mpThisRowInfo->pCellInfo[rParam.mnX+1];

        pClipMarkCell->nClipMark |= SC_CLIPMARK_RIGHT;      
        bAnyClipped = true;

        const long nMarkPixel = static_cast<long>( SC_CLIPMARK_SIZE * mnPPTX );
        if ( aAreaParam.maClipRect.Right() - nMarkPixel > aAreaParam.maClipRect.Left() )
            aAreaParam.maClipRect.Right() -= nMarkPixel;
    }
}

bool ScOutputData::Clip( DrawEditParam& rParam, const Size& aCellSize,
                         OutputAreaParam& aAreaParam, long nEngineHeight,
                         bool bWrapFields)
{
    if ( aAreaParam.maClipRect.Left() < nScrX )
    {
        aAreaParam.maClipRect.Left() = nScrX;
        aAreaParam.mbLeftClip = true;
    }
    if ( aAreaParam.maClipRect.Right() > nScrX + nScrW )
    {
        aAreaParam.maClipRect.Right() = nScrX + nScrW;          
        aAreaParam.mbRightClip = true;
    }

    bool bClip = aAreaParam.mbLeftClip || aAreaParam.mbRightClip;
    bool bSimClip = false;

    if ( bWrapFields )
    {
        
        bClip = true;
    }

    if ( aAreaParam.maClipRect.Top() < nScrY )
    {
        aAreaParam.maClipRect.Top() = nScrY;
        bClip = true;
    }
    if ( aAreaParam.maClipRect.Bottom() > nScrY + nScrH )
    {
        aAreaParam.maClipRect.Bottom() = nScrY + nScrH;     
        bClip = true;
    }

    const Size& aRefOne = mpRefDevice->PixelToLogic(Size(1,1));
    if ( nEngineHeight >= aCellSize.Height() + aRefOne.Height() )
    {
        const ScMergeAttr* pMerge =
                (ScMergeAttr*)&rParam.mpPattern->GetItem(ATTR_MERGE);
        const bool bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

        
        
        
        if ( eType != OUTTYPE_PRINTER ||
            ( mpDoc->GetRowFlags( rParam.mnCellY, nTab ) & CR_MANUALSIZE ) ||
            ( rParam.mpCondSet && SFX_ITEM_SET ==
                rParam.mpCondSet->GetItemState(ATTR_FONT_HEIGHT, true) ) )
            bClip = true;
        else
            bSimClip = true;

        ShowClipMarks( rParam, nEngineHeight, aCellSize, bMerged, aAreaParam);
    }

    Rectangle aLogicClip;
    if (bClip || bSimClip)
    {
        

        if (rParam.mbPixelToLogic)
            aLogicClip = mpRefDevice->PixelToLogic( aAreaParam.maClipRect );
        else
            aLogicClip = aAreaParam.maClipRect;

        if (bClip)  
        {
            if (bMetaFile)
            {
                mpDev->Push();
                mpDev->IntersectClipRegion( aLogicClip );
            }
            else
                mpDev->SetClipRegion( Region( aLogicClip ) );
        }
    }

    return bClip;
}

void ScOutputData::DrawEditBottomTop(DrawEditParam& rParam)
{
    OSL_ASSERT(rParam.meHorJustAttr != SVX_HOR_JUSTIFY_REPEAT);

    const bool bRepeat = (rParam.meHorJustAttr == SVX_HOR_JUSTIFY_REPEAT && !rParam.mbBreak);
    const bool bShrink = !rParam.mbBreak && !bRepeat && lcl_GetBoolValue(*rParam.mpPattern, ATTR_SHRINKTOFIT, rParam.mpCondSet);

    SvxCellHorJustify eOutHorJust = rParam.meHorJustContext;

    
    
    long nTopM, nLeftM, nBottomM, nRightM;
    rParam.calcMargins(nTopM, nLeftM, nBottomM, nRightM, mnPPTX, mnPPTY);

    SCCOL nXForPos = rParam.mnX;
    if ( nXForPos < nX1 )
    {
        nXForPos = nX1;
        rParam.mnPosX = rParam.mnInitPosX;
    }
    SCSIZE nArrYForPos = rParam.mnArrY;
    if ( nArrYForPos < 1 )
    {
        nArrYForPos = 1;
        rParam.mnPosY = nScrY;
    }

    OutputAreaParam aAreaParam;

    //
    
    //

    Size aPaperSize = Size( 1000000, 1000000 );
    if (rParam.mbBreak)
    {
        

        
        GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, 0,
                       *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                       rParam.mbCellIsValue, true, false, aAreaParam );

        
        rParam.calcPaperSize(aPaperSize, aAreaParam.maAlignRect, mnPPTX, mnPPTY);
    }
    if (rParam.mbPixelToLogic)
    {
        Size aLogicSize = mpRefDevice->PixelToLogic(aPaperSize);
        rParam.mpEngine->SetPaperSize(aLogicSize);
    }
    else
        rParam.mpEngine->SetPaperSize(aPaperSize);

    //
    
    //

    rParam.setPatternToEngine(mbUseStyleColor);
    rParam.setAlignmentToEngine();

    

    bool bWrapFields = false;
    if (!rParam.readCellContent(mpDoc, mbShowNullValues, mbShowFormulas, mbSyntaxMode, mbUseStyleColor, mbForceAutoColor, bWrapFields))
        
        return;

    if ( mbSyntaxMode )
        SetEditSyntaxColor( *rParam.mpEngine, rParam.maCell );
    else if ( mbUseStyleColor && mbForceAutoColor )
        lcl_SetEditColor( *rParam.mpEngine, COL_AUTO );     

    rParam.mpEngine->SetUpdateMode( true );     

    //
    
    //

    long nEngineWidth, nEngineHeight;
    rParam.getEngineSize(rParam.mpEngine, nEngineWidth, nEngineHeight);

    long nNeededPixel = nEngineWidth;
    if (rParam.mbPixelToLogic)
        nNeededPixel = mpRefDevice->LogicToPixel(Size(nNeededPixel,0)).Width();
    nNeededPixel += nLeftM + nRightM;

    if (!rParam.mbBreak || bShrink)
    {
        
        GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, nNeededPixel,
                       *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                       rParam.mbCellIsValue || bRepeat || bShrink, false, false, aAreaParam );

        if ( bShrink )
        {
            ShrinkEditEngine( *rParam.mpEngine, aAreaParam.maAlignRect,
                nLeftM, nTopM, nRightM, nBottomM, false,
                sal::static_int_cast<sal_uInt16>(rParam.meOrient), 0, rParam.mbPixelToLogic,
                nEngineWidth, nEngineHeight, nNeededPixel,
                aAreaParam.mbLeftClip, aAreaParam.mbRightClip );
        }
        if ( bRepeat && !aAreaParam.mbLeftClip && !aAreaParam.mbRightClip && rParam.mpEngine->GetParagraphCount() == 1 )
        {
            
            

            const long nFormatted = nNeededPixel - nLeftM - nRightM;      
            const long nAvailable = aAreaParam.maAlignRect.GetWidth() - nLeftM - nRightM;
            if ( nAvailable >= 2 * nFormatted )
            {
                
                OUString aCellStr = rParam.mpEngine->GetText();
                rParam.mpEngine->SetText( aCellStr );

                long nRepeatSize = static_cast<long>( rParam.mpEngine->CalcTextWidth() );
                if (rParam.mbPixelToLogic)
                    nRepeatSize = mpRefDevice->LogicToPixel(Size(nRepeatSize,0)).Width();
                if ( pFmtDevice != mpRefDevice )
                    ++nRepeatSize;
                if ( nRepeatSize > 0 )
                {
                    const long nRepeatCount = nAvailable / nRepeatSize;
                    if ( nRepeatCount > 1 )
                    {
                        OUString aRepeated = aCellStr;
                        for ( long nRepeat = 1; nRepeat < nRepeatCount; nRepeat++ )
                            aRepeated += aCellStr;
                        rParam.mpEngine->SetText( aRepeated );

                        nEngineHeight = rParam.mpEngine->GetTextHeight();
                        nEngineWidth = static_cast<long>( rParam.mpEngine->CalcTextWidth() );
                        if (rParam.mbPixelToLogic)
                            nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
                        else
                            nNeededPixel = nEngineWidth;
                        nNeededPixel += nLeftM + nRightM;
                    }
                }
            }
        }

        if ( rParam.mbCellIsValue && ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip ) )
        {
            rParam.mpEngine->SetText(OUString("###"));
            nEngineWidth = (long) rParam.mpEngine->CalcTextWidth();
            if (rParam.mbPixelToLogic)
                nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
            else
                nNeededPixel = nEngineWidth;
            nNeededPixel += nLeftM + nRightM;

            
        }
    }

    long nStartX = aAreaParam.maAlignRect.Left();
    const long nStartY = aAreaParam.maAlignRect.Top();
    const long nCellWidth = aAreaParam.maAlignRect.GetWidth();
    const long nOutWidth = nCellWidth - 1 - nLeftM - nRightM;
    const long nOutHeight = aAreaParam.maAlignRect.GetHeight() - nTopM - nBottomM;

    if (rParam.mbBreak)
    {
        
        
        

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

    const bool bOutside = (aAreaParam.maClipRect.Right() < nScrX || aAreaParam.maClipRect.Left() >= nScrX + nScrW);
    if (bOutside)
        return;

    
    const Size& aCellSize = rParam.mbPixelToLogic
        ? mpRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) )
        : Size( nOutWidth, nOutHeight );

    const bool bClip = Clip( rParam, aCellSize, aAreaParam, nEngineHeight, bWrapFields );

    Point aLogicStart(nStartX, nStartY);
    rParam.calcStartPosForVertical(aLogicStart, aCellSize.Width(), nEngineWidth, nTopM, mpRefDevice);

    Point aURLStart = aLogicStart;      

    if (rParam.meHorJustResult == SVX_HOR_JUSTIFY_BLOCK || rParam.mbBreak)
    {
        Size aPSize = rParam.mpEngine->GetPaperSize();
        aPSize.Width() = aCellSize.Height();
        rParam.mpEngine->SetPaperSize(aPSize);
        aLogicStart.Y() +=
            rParam.mbBreak ? aPSize.Width() : nEngineHeight;
    }
    else
    {
        
        
        

        
        Size aPSize = rParam.mpEngine->GetPaperSize();
        aPSize.Width() = rParam.mpEngine->CalcTextWidth();
        rParam.mpEngine->SetPaperSize(aPSize);

        long nGap = 0;
        long nTopOffset = 0;
        if (rParam.mbPixelToLogic)
        {
            nGap = mpRefDevice->LogicToPixel(aCellSize).Height() - mpRefDevice->LogicToPixel(aPSize).Width();
            nGap = mpRefDevice->PixelToLogic(Size(0, nGap)).Height();
            nTopOffset = mpRefDevice->PixelToLogic(Size(0,nTopM)).Height();
        }
        else
        {
            nGap = aCellSize.Height() - aPSize.Width();
            nTopOffset = nTopM;
        }

        
        aLogicStart.Y() += aCellSize.Height();
        aLogicStart.Y() += nTopOffset;

        switch (rParam.meVerJust)
        {
            case SVX_VER_JUSTIFY_STANDARD:
            case SVX_VER_JUSTIFY_BOTTOM:
                
            break;
            case SVX_VER_JUSTIFY_CENTER:
                
                aLogicStart.Y() -= nGap / 2;
            break;
            case SVX_VER_JUSTIFY_BLOCK:
            case SVX_VER_JUSTIFY_TOP:
                
                aLogicStart.Y() -= nGap;
            default:
                ;
        }
    }

    rParam.adjustForRTL();
    rParam.mpEngine->Draw(mpDev, aLogicStart, 900);

    if (bClip)
    {
        if (bMetaFile)
            mpDev->Pop();
        else
            mpDev->SetClipRegion();
    }

    rParam.adjustForHyperlinkInPDF(aURLStart, mpDev);
}

void ScOutputData::DrawEditTopBottom(DrawEditParam& rParam)
{
    OSL_ASSERT(rParam.meHorJustAttr != SVX_HOR_JUSTIFY_REPEAT);

    const bool bRepeat = (rParam.meHorJustAttr == SVX_HOR_JUSTIFY_REPEAT && !rParam.mbBreak);
    const bool bShrink = !rParam.mbBreak && !bRepeat && lcl_GetBoolValue(*rParam.mpPattern, ATTR_SHRINKTOFIT, rParam.mpCondSet);

    SvxCellHorJustify eOutHorJust = rParam.meHorJustContext;

    
    
    long nTopM, nLeftM, nBottomM, nRightM;
    rParam.calcMargins(nTopM, nLeftM, nBottomM, nRightM, mnPPTX, mnPPTY);

    SCCOL nXForPos = rParam.mnX;
    if ( nXForPos < nX1 )
    {
        nXForPos = nX1;
        rParam.mnPosX = rParam.mnInitPosX;
    }
    SCSIZE nArrYForPos = rParam.mnArrY;
    if ( nArrYForPos < 1 )
    {
        nArrYForPos = 1;
        rParam.mnPosY = nScrY;
    }

    OutputAreaParam aAreaParam;

    //
    
    //

    Size aPaperSize = Size( 1000000, 1000000 );
    if (rParam.hasLineBreak())
    {
        

        
        GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, 0,
                       *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                       rParam.mbCellIsValue, true, false, aAreaParam );

        
        rParam.calcPaperSize(aPaperSize, aAreaParam.maAlignRect, mnPPTX, mnPPTY);
    }
    if (rParam.mbPixelToLogic)
    {
        Size aLogicSize = mpRefDevice->PixelToLogic(aPaperSize);
        rParam.mpEngine->SetPaperSize(aLogicSize);
    }
    else
        rParam.mpEngine->SetPaperSize(aPaperSize);

    //
    
    //

    rParam.setPatternToEngine(mbUseStyleColor);
    rParam.setAlignmentToEngine();

    

    bool bWrapFields = false;
    if (!rParam.readCellContent(mpDoc, mbShowNullValues, mbShowFormulas, mbSyntaxMode, mbUseStyleColor, mbForceAutoColor, bWrapFields))
        
        return;

    if ( mbSyntaxMode )
        SetEditSyntaxColor( *rParam.mpEngine, rParam.maCell );
    else if ( mbUseStyleColor && mbForceAutoColor )
        lcl_SetEditColor( *rParam.mpEngine, COL_AUTO );     

    rParam.mpEngine->SetUpdateMode( true );     

    //
    
    //

    long nEngineWidth, nEngineHeight;
    rParam.getEngineSize(rParam.mpEngine, nEngineWidth, nEngineHeight);

    long nNeededPixel = nEngineWidth;
    if (rParam.mbPixelToLogic)
        nNeededPixel = mpRefDevice->LogicToPixel(Size(nNeededPixel,0)).Width();
    nNeededPixel += nLeftM + nRightM;

    if (!rParam.mbBreak || bShrink)
    {
        
        GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, nNeededPixel,
                       *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                       rParam.mbCellIsValue || bRepeat || bShrink, false, false, aAreaParam );

        if ( bShrink )
        {
            ShrinkEditEngine( *rParam.mpEngine, aAreaParam.maAlignRect,
                nLeftM, nTopM, nRightM, nBottomM, false,
                sal::static_int_cast<sal_uInt16>(rParam.meOrient), 0, rParam.mbPixelToLogic,
                nEngineWidth, nEngineHeight, nNeededPixel,
                aAreaParam.mbLeftClip, aAreaParam.mbRightClip );
        }
        if ( bRepeat && !aAreaParam.mbLeftClip && !aAreaParam.mbRightClip && rParam.mpEngine->GetParagraphCount() == 1 )
        {
            
            

            const long nFormatted = nNeededPixel - nLeftM - nRightM;      
            const long nAvailable = aAreaParam.maAlignRect.GetWidth() - nLeftM - nRightM;
            if ( nAvailable >= 2 * nFormatted )
            {
                
                OUString aCellStr = rParam.mpEngine->GetText();
                rParam.mpEngine->SetText( aCellStr );

                long nRepeatSize = static_cast<long>( rParam.mpEngine->CalcTextWidth() );
                if (rParam.mbPixelToLogic)
                    nRepeatSize = mpRefDevice->LogicToPixel(Size(nRepeatSize,0)).Width();
                if ( pFmtDevice != mpRefDevice )
                    ++nRepeatSize;
                if ( nRepeatSize > 0 )
                {
                    const long nRepeatCount = nAvailable / nRepeatSize;
                    if ( nRepeatCount > 1 )
                    {
                        OUString aRepeated = aCellStr;
                        for ( long nRepeat = 1; nRepeat < nRepeatCount; nRepeat++ )
                            aRepeated += aCellStr;
                        rParam.mpEngine->SetText( aRepeated );

                        nEngineHeight = rParam.mpEngine->GetTextHeight();
                        nEngineWidth = static_cast<long>( rParam.mpEngine->CalcTextWidth() );
                        if (rParam.mbPixelToLogic)
                            nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
                        else
                            nNeededPixel = nEngineWidth;
                        nNeededPixel += nLeftM + nRightM;
                    }
                }
            }
        }

        if ( rParam.mbCellIsValue && ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip ) )
        {
            rParam.mpEngine->SetText(OUString("###"));
            nEngineWidth = static_cast<long>( rParam.mpEngine->CalcTextWidth() );
            if (rParam.mbPixelToLogic)
                nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
            else
                nNeededPixel = nEngineWidth;
            nNeededPixel += nLeftM + nRightM;

            
        }
    }

    long nStartX = aAreaParam.maAlignRect.Left();
    const long nStartY = aAreaParam.maAlignRect.Top();
    const long nCellWidth = aAreaParam.maAlignRect.GetWidth();
    const long nOutWidth = nCellWidth - 1 - nLeftM - nRightM;
    const long nOutHeight = aAreaParam.maAlignRect.GetHeight() - nTopM - nBottomM;

    if (rParam.mbBreak)
    {
        
        
        

        nStartX += nLeftM;
        if (rParam.meHorJustResult == SVX_HOR_JUSTIFY_BLOCK)
            nStartX += aPaperSize.Height();
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

    const bool bOutside = (aAreaParam.maClipRect.Right() < nScrX || aAreaParam.maClipRect.Left() >= nScrX + nScrW);
    if (bOutside)
        return;

    
    const Size& aCellSize = rParam.mbPixelToLogic
        ? mpRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) )
        : Size( nOutWidth, nOutHeight );

    const bool bClip = Clip( rParam, aCellSize, aAreaParam, nEngineHeight, bWrapFields );

    Point aLogicStart(nStartX, nStartY);
    rParam.calcStartPosForVertical(aLogicStart, aCellSize.Width(), nEngineWidth, nTopM, mpRefDevice);

    Point aURLStart = aLogicStart;      

    if (rParam.meHorJustResult != SVX_HOR_JUSTIFY_BLOCK)
    {
        aLogicStart.X() += nEngineWidth;
        if (!rParam.mbBreak)
        {
            
            Size aPSize = rParam.mpEngine->GetPaperSize();
            aPSize.Width() = rParam.mpEngine->CalcTextWidth();
            rParam.mpEngine->SetPaperSize(aPSize);

            long nGap = 0;
            long nTopOffset = 0; 
            if (rParam.mbPixelToLogic)
            {
                nGap = mpRefDevice->LogicToPixel(aPSize).Width() - mpRefDevice->LogicToPixel(aCellSize).Height();
                nGap = mpRefDevice->PixelToLogic(Size(0, nGap)).Height();
                nTopOffset = mpRefDevice->PixelToLogic(Size(0,nTopM)).Height();
            }
            else
            {
                nGap = aPSize.Width() - aCellSize.Height();
                nTopOffset = nTopM;
            }
            aLogicStart.Y() += nTopOffset;

            switch (rParam.meVerJust)
            {
                case SVX_VER_JUSTIFY_STANDARD:
                case SVX_VER_JUSTIFY_BOTTOM:
                    
                    aLogicStart.Y() -= nGap;
                break;
                case SVX_VER_JUSTIFY_CENTER:
                    
                    aLogicStart.Y() -= nGap / 2;
                break;
                case SVX_VER_JUSTIFY_BLOCK:
                case SVX_VER_JUSTIFY_TOP:
                    
                default:
                    ;
            }
        }
    }

    rParam.adjustForRTL();

    
    

    rParam.mpEngine->Draw(mpDev, aLogicStart, 2700);

    if (bClip)
    {
        if (bMetaFile)
            mpDev->Pop();
        else
            mpDev->SetClipRegion();
    }

    rParam.adjustForHyperlinkInPDF(aURLStart, mpDev);
}

void ScOutputData::DrawEditStacked(DrawEditParam& rParam)
{
    OSL_ASSERT(rParam.meHorJustAttr != SVX_HOR_JUSTIFY_REPEAT);
    Size aRefOne = mpRefDevice->PixelToLogic(Size(1,1));

    bool bRepeat = (rParam.meHorJustAttr == SVX_HOR_JUSTIFY_REPEAT && !rParam.mbBreak);
    bool bShrink = !rParam.mbBreak && !bRepeat && lcl_GetBoolValue(*rParam.mpPattern, ATTR_SHRINKTOFIT, rParam.mpCondSet);

    rParam.mbAsianVertical =
        lcl_GetBoolValue(*rParam.mpPattern, ATTR_VERTICAL_ASIAN, rParam.mpCondSet);

    if ( rParam.mbAsianVertical )
    {
        
        rParam.meOrient = SVX_ORIENTATION_STANDARD;
        DrawEditAsianVertical(rParam);
        return;
    }

    SvxCellHorJustify eOutHorJust = rParam.meHorJustContext;

    
    
    long nTopM, nLeftM, nBottomM, nRightM;
    rParam.calcMargins(nTopM, nLeftM, nBottomM, nRightM, mnPPTX, mnPPTY);

    SCCOL nXForPos = rParam.mnX;
    if ( nXForPos < nX1 )
    {
        nXForPos = nX1;
        rParam.mnPosX = rParam.mnInitPosX;
    }
    SCSIZE nArrYForPos = rParam.mnArrY;
    if ( nArrYForPos < 1 )
    {
        nArrYForPos = 1;
        rParam.mnPosY = nScrY;
    }

    OutputAreaParam aAreaParam;

    //
    
    //

    Size aPaperSize = Size( 1000000, 1000000 );
    

    
    GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, 0,
                   *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                   rParam.mbCellIsValue, true, false, aAreaParam );

    
    rParam.calcPaperSize(aPaperSize, aAreaParam.maAlignRect, mnPPTX, mnPPTY);

    if (rParam.mbPixelToLogic)
    {
        Size aLogicSize = mpRefDevice->PixelToLogic(aPaperSize);
        if ( rParam.mbBreak && mpRefDevice != pFmtDevice )
        {
            
            

            Fraction aFract(1,1);
            Rectangle aUtilRect = ScEditUtil( mpDoc, rParam.mnCellX, rParam.mnCellY, nTab, Point(0,0), pFmtDevice,
                HMM_PER_TWIPS, HMM_PER_TWIPS, aFract, aFract ).GetEditArea( rParam.mpPattern, false );
            aLogicSize.Width() = aUtilRect.GetWidth();
        }
        rParam.mpEngine->SetPaperSize(aLogicSize);
    }
    else
        rParam.mpEngine->SetPaperSize(aPaperSize);

    //
    
    //

    rParam.setPatternToEngine(mbUseStyleColor);
    rParam.setAlignmentToEngine();

    

    bool bWrapFields = false;
    if (!rParam.readCellContent(mpDoc, mbShowNullValues, mbShowFormulas, mbSyntaxMode, mbUseStyleColor, mbForceAutoColor, bWrapFields))
        
        return;

    if ( mbSyntaxMode )
        SetEditSyntaxColor( *rParam.mpEngine, rParam.maCell );
    else if ( mbUseStyleColor && mbForceAutoColor )
        lcl_SetEditColor( *rParam.mpEngine, COL_AUTO );     

    rParam.mpEngine->SetUpdateMode( true );     

    //
    
    //

    long nEngineWidth, nEngineHeight;
    rParam.getEngineSize(rParam.mpEngine, nEngineWidth, nEngineHeight);

    long nNeededPixel = nEngineWidth;
    if (rParam.mbPixelToLogic)
        nNeededPixel = mpRefDevice->LogicToPixel(Size(nNeededPixel,0)).Width();
    nNeededPixel += nLeftM + nRightM;

    if (bShrink)
    {
        
        GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, nNeededPixel,
                       *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                       true, false, false, aAreaParam );

        ShrinkEditEngine( *rParam.mpEngine, aAreaParam.maAlignRect,
            nLeftM, nTopM, nRightM, nBottomM, true,
            sal::static_int_cast<sal_uInt16>(rParam.meOrient), 0, rParam.mbPixelToLogic,
            nEngineWidth, nEngineHeight, nNeededPixel,
            aAreaParam.mbLeftClip, aAreaParam.mbRightClip );

        if ( bRepeat && !aAreaParam.mbLeftClip && !aAreaParam.mbRightClip && rParam.mpEngine->GetParagraphCount() == 1 )
        {
            
            

            long nFormatted = nNeededPixel - nLeftM - nRightM;      
            long nAvailable = aAreaParam.maAlignRect.GetWidth() - nLeftM - nRightM;
            if ( nAvailable >= 2 * nFormatted )
            {
                
                OUString aCellStr = rParam.mpEngine->GetText();
                rParam.mpEngine->SetText( aCellStr );

                long nRepeatSize = (long) rParam.mpEngine->CalcTextWidth();
                if (rParam.mbPixelToLogic)
                    nRepeatSize = mpRefDevice->LogicToPixel(Size(nRepeatSize,0)).Width();
                if ( pFmtDevice != mpRefDevice )
                    ++nRepeatSize;
                if ( nRepeatSize > 0 )
                {
                    long nRepeatCount = nAvailable / nRepeatSize;
                    if ( nRepeatCount > 1 )
                    {
                        OUString aRepeated = aCellStr;
                        for ( long nRepeat = 1; nRepeat < nRepeatCount; nRepeat++ )
                            aRepeated += aCellStr;
                        rParam.mpEngine->SetText( aRepeated );

                        nEngineHeight = rParam.mpEngine->GetTextHeight();
                        nEngineWidth = (long) rParam.mpEngine->CalcTextWidth();
                        if (rParam.mbPixelToLogic)
                            nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
                        else
                            nNeededPixel = nEngineWidth;
                        nNeededPixel += nLeftM + nRightM;
                    }
                }
            }
        }

        if ( rParam.mbCellIsValue && ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip ) )
        {
            rParam.mpEngine->SetText(OUString("###"));
            nEngineWidth = (long) rParam.mpEngine->CalcTextWidth();
            if (rParam.mbPixelToLogic)
                nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
            else
                nNeededPixel = nEngineWidth;
            nNeededPixel += nLeftM + nRightM;

            
        }

        if ( eOutHorJust != SVX_HOR_JUSTIFY_LEFT )
        {
            aPaperSize.Width() = nNeededPixel + 1;
            if (rParam.mbPixelToLogic)
                rParam.mpEngine->SetPaperSize(mpRefDevice->PixelToLogic(aPaperSize));
            else
                rParam.mpEngine->SetPaperSize(aPaperSize);
        }
    }

    long nStartX = aAreaParam.maAlignRect.Left();
    long nStartY = aAreaParam.maAlignRect.Top();
    long nCellWidth = aAreaParam.maAlignRect.GetWidth();
    long nOutWidth = nCellWidth - 1 - nLeftM - nRightM;
    long nOutHeight = aAreaParam.maAlignRect.GetHeight() - nTopM - nBottomM;

    if (rParam.mbBreak)
    {
        
        
        

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

    bool bOutside = (aAreaParam.maClipRect.Right() < nScrX || aAreaParam.maClipRect.Left() >= nScrX + nScrW);
    if (bOutside)
        return;

    if ( aAreaParam.maClipRect.Left() < nScrX )
    {
        aAreaParam.maClipRect.Left() = nScrX;
        aAreaParam.mbLeftClip = true;
    }
    if ( aAreaParam.maClipRect.Right() > nScrX + nScrW )
    {
        aAreaParam.maClipRect.Right() = nScrX + nScrW;          
        aAreaParam.mbRightClip = true;
    }

    bool bClip = aAreaParam.mbLeftClip || aAreaParam.mbRightClip;
    bool bSimClip = false;

    if ( bWrapFields )
    {
        
        bClip = true;
    }

    if ( aAreaParam.maClipRect.Top() < nScrY )
    {
        aAreaParam.maClipRect.Top() = nScrY;
        bClip = true;
    }
    if ( aAreaParam.maClipRect.Bottom() > nScrY + nScrH )
    {
        aAreaParam.maClipRect.Bottom() = nScrY + nScrH;     
        bClip = true;
    }

    Size aCellSize;         
    if (rParam.mbPixelToLogic)
        aCellSize = mpRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
    else
        aCellSize = Size( nOutWidth, nOutHeight );

    if ( nEngineHeight >= aCellSize.Height() + aRefOne.Height() )
    {
        const ScMergeAttr* pMerge =
                (ScMergeAttr*)&rParam.mpPattern->GetItem(ATTR_MERGE);
        bool bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

        
        
        
        if ( eType != OUTTYPE_PRINTER ||
            ( mpDoc->GetRowFlags( rParam.mnCellY, nTab ) & CR_MANUALSIZE ) ||
            ( rParam.mpCondSet && SFX_ITEM_SET ==
                rParam.mpCondSet->GetItemState(ATTR_FONT_HEIGHT, true) ) )
            bClip = true;
        else
            bSimClip = true;

        
        
        
        
        
        
        if ( nEngineHeight - aCellSize.Height() > 100 &&
             rParam.mbBreak && bMarkClipped &&
             ( rParam.mpEngine->GetParagraphCount() > 1 || rParam.mpEngine->GetLineCount(0) > 1 ) )
        {
            CellInfo* pClipMarkCell = NULL;
            if ( bMerged )
            {
                
                SCCOL nClipX = ( rParam.mnX < nX1 ) ? nX1 : rParam.mnX;
                pClipMarkCell = &pRowInfo[(rParam.mnArrY != 0) ? rParam.mnArrY : 1].pCellInfo[nClipX+1];
            }
            else
                pClipMarkCell = &rParam.mpThisRowInfo->pCellInfo[rParam.mnX+1];

            pClipMarkCell->nClipMark |= SC_CLIPMARK_RIGHT;      
            bAnyClipped = true;

            long nMarkPixel = (long)( SC_CLIPMARK_SIZE * mnPPTX );
            if ( aAreaParam.maClipRect.Right() - nMarkPixel > aAreaParam.maClipRect.Left() )
                aAreaParam.maClipRect.Right() -= nMarkPixel;
        }
    }

    Rectangle aLogicClip;
    if (bClip || bSimClip)
    {
        

        if (rParam.mbPixelToLogic)
            aLogicClip = mpRefDevice->PixelToLogic( aAreaParam.maClipRect );
        else
            aLogicClip = aAreaParam.maClipRect;

        if (bClip)  
        {
            if (bMetaFile)
            {
                mpDev->Push();
                mpDev->IntersectClipRegion( aLogicClip );
            }
            else
                mpDev->SetClipRegion( Region( aLogicClip ) );
        }
    }

    Point aLogicStart;
    if (rParam.mbPixelToLogic)
        aLogicStart = mpRefDevice->PixelToLogic( Point(nStartX,nStartY) );
    else
        aLogicStart = Point(nStartX, nStartY);

    if (rParam.meVerJust==SVX_VER_JUSTIFY_BOTTOM ||
        rParam.meVerJust==SVX_VER_JUSTIFY_STANDARD)
    {
        
        

        if (rParam.mbPixelToLogic)
            aLogicStart.Y() += mpRefDevice->PixelToLogic( Size(0, nTopM +
                            mpRefDevice->LogicToPixel(aCellSize).Height() -
                            mpRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height()
                            )).Height();
        else
            aLogicStart.Y() += nTopM + aCellSize.Height() - nEngineHeight;
    }
    else if (rParam.meVerJust==SVX_VER_JUSTIFY_CENTER)
    {
        if (rParam.mbPixelToLogic)
            aLogicStart.Y() += mpRefDevice->PixelToLogic( Size(0, nTopM + (
                            mpRefDevice->LogicToPixel(aCellSize).Height() -
                            mpRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height() )
                            / 2)).Height();
        else
            aLogicStart.Y() += nTopM + (aCellSize.Height() - nEngineHeight) / 2;
    }
    else        
    {
        if (rParam.mbPixelToLogic)
            aLogicStart.Y() += mpRefDevice->PixelToLogic(Size(0,nTopM)).Height();
        else
            aLogicStart.Y() += nTopM;
    }

    Point aURLStart = aLogicStart;      

    Size aPaperLogic = rParam.mpEngine->GetPaperSize();
    aPaperLogic.Width() = nEngineWidth;
    rParam.mpEngine->SetPaperSize(aPaperLogic);

    rParam.adjustForRTL();

    
    

    if (bSimClip)
    {
        
        

        Point aDocStart = aLogicClip.TopLeft();
        aDocStart -= aLogicStart;
        rParam.mpEngine->Draw( mpDev, aLogicClip, aDocStart, false );
    }
    else
    {
        rParam.mpEngine->Draw( mpDev, aLogicStart, 0 );
    }

    if (bClip)
    {
        if (bMetaFile)
            mpDev->Pop();
        else
            mpDev->SetClipRegion();
    }

    rParam.adjustForHyperlinkInPDF(aURLStart, mpDev);
}

void ScOutputData::DrawEditAsianVertical(DrawEditParam& rParam)
{
    
    
    OSL_ASSERT(rParam.meOrient == SVX_ORIENTATION_STANDARD);
    OSL_ASSERT(rParam.mbAsianVertical);
    OSL_ASSERT(rParam.meHorJustAttr != SVX_HOR_JUSTIFY_REPEAT);

    Size aRefOne = mpRefDevice->PixelToLogic(Size(1,1));

    bool bHidden = false;
    bool bRepeat = false;
    bool bShrink = !rParam.mbBreak && !bRepeat && lcl_GetBoolValue(*rParam.mpPattern, ATTR_SHRINKTOFIT, rParam.mpCondSet);
    long nAttrRotate = lcl_GetValue<SfxInt32Item, long>(*rParam.mpPattern, ATTR_ROTATE_VALUE, rParam.mpCondSet);

    if (nAttrRotate)
    {
        
        
        bHidden = true;     
    }

    
    /* TODO: is setting meHorJustContext and meHorJustResult unconditionally to
     * SVX_HOR_JUSTIFY_RIGHT really wanted? Seems this was done all the time,
     * also before context was introduced and everything was attr only. */
    if ( rParam.meHorJustAttr == SVX_HOR_JUSTIFY_STANDARD )
        rParam.meHorJustResult = rParam.meHorJustContext = SVX_HOR_JUSTIFY_RIGHT;

    if (bHidden)
        return;

    SvxCellHorJustify eOutHorJust = rParam.meHorJustContext;

    
    
    long nTopM, nLeftM, nBottomM, nRightM;
    rParam.calcMargins(nTopM, nLeftM, nBottomM, nRightM, mnPPTX, mnPPTY);

    SCCOL nXForPos = rParam.mnX;
    if ( nXForPos < nX1 )
    {
        nXForPos = nX1;
        rParam.mnPosX = rParam.mnInitPosX;
    }
    SCSIZE nArrYForPos = rParam.mnArrY;
    if ( nArrYForPos < 1 )
    {
        nArrYForPos = 1;
        rParam.mnPosY = nScrY;
    }

    OutputAreaParam aAreaParam;

    //
    
    //

    Size aPaperSize = Size( 1000000, 1000000 );
    

    
    GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, 0,
                   *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                   rParam.mbCellIsValue, true, false, aAreaParam );

    
    rParam.calcPaperSize(aPaperSize, aAreaParam.maAlignRect, mnPPTX, mnPPTY);

    if (rParam.mbPixelToLogic)
    {
        Size aLogicSize = mpRefDevice->PixelToLogic(aPaperSize);
        if ( rParam.mbBreak && !rParam.mbAsianVertical && mpRefDevice != pFmtDevice )
        {
            
            

            Fraction aFract(1,1);
            Rectangle aUtilRect = ScEditUtil( mpDoc, rParam.mnCellX, rParam.mnCellY, nTab, Point(0,0), pFmtDevice,
                HMM_PER_TWIPS, HMM_PER_TWIPS, aFract, aFract ).GetEditArea( rParam.mpPattern, false );
            aLogicSize.Width() = aUtilRect.GetWidth();
        }
        rParam.mpEngine->SetPaperSize(aLogicSize);
    }
    else
        rParam.mpEngine->SetPaperSize(aPaperSize);

    //
    
    //

    
    if ( rParam.meVerJust == SVX_VER_JUSTIFY_STANDARD )
        rParam.meVerJust = SVX_VER_JUSTIFY_TOP;

    rParam.setPatternToEngine(mbUseStyleColor);
    rParam.setAlignmentToEngine();

    

    bool bWrapFields = false;
    if (!rParam.readCellContent(mpDoc, mbShowNullValues, mbShowFormulas, mbSyntaxMode, mbUseStyleColor, mbForceAutoColor, bWrapFields))
        
        return;

    if ( mbSyntaxMode )
        SetEditSyntaxColor( *rParam.mpEngine, rParam.maCell );
    else if ( mbUseStyleColor && mbForceAutoColor )
        lcl_SetEditColor( *rParam.mpEngine, COL_AUTO );     

    rParam.mpEngine->SetUpdateMode( true );     

    //
    
    //

    long nEngineWidth, nEngineHeight;
    rParam.getEngineSize(rParam.mpEngine, nEngineWidth, nEngineHeight);

    long nNeededPixel = nEngineWidth;
    if (rParam.mbPixelToLogic)
        nNeededPixel = mpRefDevice->LogicToPixel(Size(nNeededPixel,0)).Width();
    nNeededPixel += nLeftM + nRightM;

    
    GetOutputArea( nXForPos, nArrYForPos, rParam.mnPosX, rParam.mnPosY, rParam.mnCellX, rParam.mnCellY, nNeededPixel,
                   *rParam.mpPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                   rParam.mbCellIsValue || bRepeat || bShrink, false, false, aAreaParam );

    if ( bShrink )
    {
        ShrinkEditEngine( *rParam.mpEngine, aAreaParam.maAlignRect,
            nLeftM, nTopM, nRightM, nBottomM, false,
            sal::static_int_cast<sal_uInt16>(rParam.meOrient), 0, rParam.mbPixelToLogic,
            nEngineWidth, nEngineHeight, nNeededPixel,
            aAreaParam.mbLeftClip, aAreaParam.mbRightClip );
    }
    if ( bRepeat && !aAreaParam.mbLeftClip && !aAreaParam.mbRightClip && rParam.mpEngine->GetParagraphCount() == 1 )
    {
        
        

        long nFormatted = nNeededPixel - nLeftM - nRightM;      
        long nAvailable = aAreaParam.maAlignRect.GetWidth() - nLeftM - nRightM;
        if ( nAvailable >= 2 * nFormatted )
        {
            
            OUString aCellStr = rParam.mpEngine->GetText();
            rParam.mpEngine->SetText( aCellStr );

            long nRepeatSize = (long) rParam.mpEngine->CalcTextWidth();
            if (rParam.mbPixelToLogic)
                nRepeatSize = mpRefDevice->LogicToPixel(Size(nRepeatSize,0)).Width();
            if ( pFmtDevice != mpRefDevice )
                ++nRepeatSize;
            if ( nRepeatSize > 0 )
            {
                long nRepeatCount = nAvailable / nRepeatSize;
                if ( nRepeatCount > 1 )
                {
                    OUString aRepeated = aCellStr;
                    for ( long nRepeat = 1; nRepeat < nRepeatCount; nRepeat++ )
                        aRepeated += aCellStr;
                    rParam.mpEngine->SetText( aRepeated );

                    nEngineHeight = rParam.mpEngine->GetTextHeight();
                    nEngineWidth = (long) rParam.mpEngine->CalcTextWidth();
                    if (rParam.mbPixelToLogic)
                        nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
                    else
                        nNeededPixel = nEngineWidth;
                    nNeededPixel += nLeftM + nRightM;
                }
            }
        }
    }

    if ( rParam.mbCellIsValue && ( aAreaParam.mbLeftClip || aAreaParam.mbRightClip ) )
    {
        rParam.mpEngine->SetText(OUString("###"));
        nEngineWidth = (long) rParam.mpEngine->CalcTextWidth();
        if (rParam.mbPixelToLogic)
            nNeededPixel = mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width();
        else
            nNeededPixel = nEngineWidth;
        nNeededPixel += nLeftM + nRightM;

        
    }

    if (eOutHorJust != SVX_HOR_JUSTIFY_LEFT)
    {
        aPaperSize.Width() = nNeededPixel + 1;
        if (rParam.mbPixelToLogic)
            rParam.mpEngine->SetPaperSize(mpRefDevice->PixelToLogic(aPaperSize));
        else
            rParam.mpEngine->SetPaperSize(aPaperSize);
    }

    long nStartX = aAreaParam.maAlignRect.Left();
    long nStartY = aAreaParam.maAlignRect.Top();
    long nCellWidth = aAreaParam.maAlignRect.GetWidth();
    long nOutWidth = nCellWidth - 1 - nLeftM - nRightM;
    long nOutHeight = aAreaParam.maAlignRect.GetHeight() - nTopM - nBottomM;

    
    
    

    nStartX += nLeftM;

    bool bOutside = (aAreaParam.maClipRect.Right() < nScrX || aAreaParam.maClipRect.Left() >= nScrX + nScrW);
    if (bOutside)
        return;

    if ( aAreaParam.maClipRect.Left() < nScrX )
    {
        aAreaParam.maClipRect.Left() = nScrX;
        aAreaParam.mbLeftClip = true;
    }
    if ( aAreaParam.maClipRect.Right() > nScrX + nScrW )
    {
        aAreaParam.maClipRect.Right() = nScrX + nScrW;          
        aAreaParam.mbRightClip = true;
    }

    bool bClip = aAreaParam.mbLeftClip || aAreaParam.mbRightClip;
    bool bSimClip = false;

    if ( bWrapFields )
    {
        
        bClip = true;
    }

    if ( aAreaParam.maClipRect.Top() < nScrY )
    {
        aAreaParam.maClipRect.Top() = nScrY;
        bClip = true;
    }
    if ( aAreaParam.maClipRect.Bottom() > nScrY + nScrH )
    {
        aAreaParam.maClipRect.Bottom() = nScrY + nScrH;     
        bClip = true;
    }

    Size aCellSize;         
    if (rParam.mbPixelToLogic)
        aCellSize = mpRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
    else
        aCellSize = Size( nOutWidth, nOutHeight );

    if ( nEngineHeight >= aCellSize.Height() + aRefOne.Height() )
    {
        const ScMergeAttr* pMerge =
                (ScMergeAttr*)&rParam.mpPattern->GetItem(ATTR_MERGE);
        bool bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

        
        
        
        if ( eType != OUTTYPE_PRINTER ||
            ( mpDoc->GetRowFlags( rParam.mnCellY, nTab ) & CR_MANUALSIZE ) ||
            ( rParam.mpCondSet && SFX_ITEM_SET ==
                rParam.mpCondSet->GetItemState(ATTR_FONT_HEIGHT, true) ) )
            bClip = true;
        else
            bSimClip = true;

        
        
        
        
        
        
        if ( nEngineHeight - aCellSize.Height() > 100 &&
             ( rParam.mbBreak || rParam.meOrient == SVX_ORIENTATION_STACKED ) &&
             !rParam.mbAsianVertical && bMarkClipped &&
             ( rParam.mpEngine->GetParagraphCount() > 1 || rParam.mpEngine->GetLineCount(0) > 1 ) )
        {
            CellInfo* pClipMarkCell = NULL;
            if ( bMerged )
            {
                
                SCCOL nClipX = ( rParam.mnX < nX1 ) ? nX1 : rParam.mnX;
                pClipMarkCell = &pRowInfo[(rParam.mnArrY != 0) ? rParam.mnArrY : 1].pCellInfo[nClipX+1];
            }
            else
                pClipMarkCell = &rParam.mpThisRowInfo->pCellInfo[rParam.mnX+1];

            pClipMarkCell->nClipMark |= SC_CLIPMARK_RIGHT;      
            bAnyClipped = true;

            long nMarkPixel = (long)( SC_CLIPMARK_SIZE * mnPPTX );
            if ( aAreaParam.maClipRect.Right() - nMarkPixel > aAreaParam.maClipRect.Left() )
                aAreaParam.maClipRect.Right() -= nMarkPixel;
        }
    }

    Rectangle aLogicClip;
    if (bClip || bSimClip)
    {
        

        if (rParam.mbPixelToLogic)
            aLogicClip = mpRefDevice->PixelToLogic( aAreaParam.maClipRect );
        else
            aLogicClip = aAreaParam.maClipRect;

        if (bClip)  
        {
            if (bMetaFile)
            {
                mpDev->Push();
                mpDev->IntersectClipRegion( aLogicClip );
            }
            else
                mpDev->SetClipRegion( Region( aLogicClip ) );
        }
    }

    Point aLogicStart;
    if (rParam.mbPixelToLogic)
        aLogicStart = mpRefDevice->PixelToLogic( Point(nStartX,nStartY) );
    else
        aLogicStart = Point(nStartX, nStartY);

    long nAvailWidth = aCellSize.Width();
    

    

    if (rParam.meHorJustResult==SVX_HOR_JUSTIFY_RIGHT)
        aLogicStart.X() += nAvailWidth - nEngineWidth;
    else if (rParam.meHorJustResult==SVX_HOR_JUSTIFY_CENTER)
        aLogicStart.X() += (nAvailWidth - nEngineWidth) / 2;

    
    aLogicStart.X() += nEngineWidth;

    
    if (rParam.mbPixelToLogic)
        aLogicStart.Y() += mpRefDevice->PixelToLogic(Size(0,nTopM)).Height();
    else
        aLogicStart.Y() += nTopM;

    Point aURLStart = aLogicStart;      

    rParam.adjustForRTL();

    
    

    
    
    aLogicStart.X() -= rParam.mpEngine->GetPaperSize().Width();

    rParam.mpEngine->Draw(mpDev, aLogicStart, 0);

    if (bClip)
    {
        if (bMetaFile)
            mpDev->Pop();
        else
            mpDev->SetClipRegion();
    }

    rParam.adjustForHyperlinkInPDF(aURLStart, mpDev);
}

void ScOutputData::DrawEdit(bool bPixelToLogic)
{
    ScFieldEditEngine* pEngine = NULL;
    bool bHyphenatorSet = false;
    const ScPatternAttr* pOldPattern = NULL;
    const SfxItemSet*    pOldCondSet = NULL;
    const SfxItemSet*    pOldPreviewFontSet = NULL;
    ScRefCellValue aCell;

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
        nInitPosX += nMirrorW - 1;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    
    SCCOL nLastContentCol = MAXCOL;
    if ( nX2 < MAXCOL )
        nLastContentCol = sal::static_int_cast<SCCOL>(
            nLastContentCol - mpDoc->GetEmptyLinesInBlock( nX2+1, nY1, nTab, MAXCOL, nY2, nTab, DIR_RIGHT ) );

    long nRowPosY = nScrY;
    for (SCSIZE nArrY=0; nArrY+1<nArrCount; nArrY++)            
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];

        if (nArrY==1) nRowPosY = nScrY;                         

        if ( pThisRowInfo->bChanged || nArrY==0 )
        {
            long nPosX = 0;
            for (SCCOL nX=0; nX<=nX2; nX++)                 
            {
                std::auto_ptr< ScPatternAttr > pPreviewPattr;
                if (nX==nX1) nPosX = nInitPosX;                 

                CellInfo*   pInfo = &pThisRowInfo->pCellInfo[nX+1];
                if (pInfo->bEditEngine)
                {
                    SCROW nY = pThisRowInfo->nRowNo;

                    SCCOL nCellX = nX;                  
                    SCROW nCellY = nY;
                    bool bDoCell = false;

                    long nPosY = nRowPosY;
                    if ( nArrY == 0 )
                    {
                        nPosY = nScrY;
                        nY = pRowInfo[1].nRowNo;
                        SCCOL nOverX;                   
                        SCROW nOverY;
                        if (GetMergeOrigin( nX,nY, 1, nOverX,nOverY, true ))
                        {
                            nCellX = nOverX;
                            nCellY = nOverY;
                            bDoCell = true;
                        }
                    }
                    else if ( nX == nX2 && pThisRowInfo->pCellInfo[nX+1].maCell.isEmpty() )
                    {
                        

                        SCCOL nTempX=nX;
                        while (nTempX < nLastContentCol && IsEmptyCellText( pThisRowInfo, nTempX, nY ))
                            ++nTempX;

                        if ( nTempX > nX &&
                             !IsEmptyCellText( pThisRowInfo, nTempX, nY ) &&
                             !mpDoc->HasAttrib( nTempX,nY,nTab, nX,nY,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                        {
                            nCellX = nTempX;
                            bDoCell = true;
                        }
                    }
                    else
                    {
                        bDoCell = true;
                    }

                    if ( bDoCell && bEditMode && nCellX == nEditCol && nCellY == nEditRow )
                        bDoCell = false;

                    const ScPatternAttr* pPattern = NULL;
                    const SfxItemSet* pCondSet = NULL;
                    if (bDoCell)
                    {
                        if ( nCellY == nY && nCellX >= nX1 && nCellX <= nX2 &&
                             !mpDoc->ColHidden(nCellX, nTab) )
                        {
                            CellInfo& rCellInfo = pThisRowInfo->pCellInfo[nCellX+1];
                            pPattern = rCellInfo.pPatternAttr;
                            pCondSet = rCellInfo.pConditionSet;
                            aCell = rCellInfo.maCell;
                        }
                        else        
                        {
                            pPattern = mpDoc->GetPattern( nCellX, nCellY, nTab );
                            pCondSet = mpDoc->GetCondResult( nCellX, nCellY, nTab );
                            GetVisibleCell( nCellX, nCellY, nTab, aCell );
                        }
                        if (aCell.isEmpty())
                            bDoCell = false;
                    }
                    if (bDoCell)
                    {
                        if ( mpDoc->GetPreviewCellStyle() )
                        {
                            if ( ScStyleSheet* pPreviewStyle = mpDoc->GetPreviewCellStyle( nCellX, nCellY, nTab ) )
                            {
                                pPreviewPattr.reset( new ScPatternAttr(*pPattern) );
                                pPreviewPattr->SetStyleSheet(pPreviewStyle);
                                pPattern = const_cast<ScPatternAttr*>(pPreviewPattr.get());
                            }
                        }
                        SfxItemSet* pPreviewFontSet = mpDoc->GetPreviewFont( nCellX, nCellY, nTab );
                        if (!pEngine)
                            pEngine = CreateOutputEditEngine();
                        else
                            lcl_ClearEdit( *pEngine );      

                        
                        OUString aStr = mpDoc->GetString(nCellX, nCellY, nTab);

                        DrawEditParam aParam(pPattern, pCondSet, lcl_SafeIsValue(aCell));
                        aParam.meHorJustContext = getAlignmentFromContext( aParam.meHorJustAttr,
                                aParam.mbCellIsValue, aStr, *pPattern, pCondSet, mpDoc, nTab);
                        aParam.meHorJustResult = (aParam.meHorJustAttr == SVX_HOR_JUSTIFY_BLOCK) ?
                                SVX_HOR_JUSTIFY_BLOCK : aParam.meHorJustContext;
                        aParam.mbPixelToLogic = bPixelToLogic;
                        aParam.mbHyphenatorSet = bHyphenatorSet;
                        aParam.mpEngine = pEngine;
                        aParam.maCell = aCell;
                        aParam.mnArrY = nArrY;
                        aParam.mnX = nX;
                        aParam.mnY = nY;
                        aParam.mnCellX = nCellX;
                        aParam.mnCellY = nCellY;
                        aParam.mnTab = nTab;
                        aParam.mnPosX = nPosX;
                        aParam.mnPosY = nPosY;
                        aParam.mnInitPosX = nInitPosX;
                        aParam.mpPreviewFontSet = pPreviewFontSet;
                        aParam.mpPreviewFontSet = pPreviewFontSet;
                        aParam.mpOldPattern = pOldPattern;
                        aParam.mpOldCondSet = pOldCondSet;
                        aParam.mpOldPreviewFontSet = pOldPreviewFontSet;
                        aParam.mpThisRowInfo = pThisRowInfo;
                        if (mpSpellCheckCxt)
                            aParam.mpMisspellRanges = mpSpellCheckCxt->getMisspellRanges(nCellX, nCellY);

                        if (aParam.meHorJustAttr == SVX_HOR_JUSTIFY_REPEAT)
                        {
                            
                            aParam.meOrient = SVX_ORIENTATION_STANDARD;
                        }
                        switch (aParam.meOrient)
                        {
                            case SVX_ORIENTATION_BOTTOMTOP:
                                DrawEditBottomTop(aParam);
                            break;
                            case SVX_ORIENTATION_TOPBOTTOM:
                                DrawEditTopBottom(aParam);
                            break;
                            case SVX_ORIENTATION_STACKED:
                                
                                DrawEditStacked(aParam);
                            break;
                            default:
                                DrawEditStandard(aParam);
                        }

                        
                        pOldPattern = aParam.mpOldPattern;
                        pOldCondSet = aParam.mpOldCondSet;
                        pOldPreviewFontSet = aParam.mpOldPreviewFontSet;
                        bHyphenatorSet = aParam.mbHyphenatorSet;
                    }
                }
                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
            }
        }
        nRowPosY += pRowInfo[nArrY].nHeight;
    }

    delete pEngine;

    if (bAnyRotated)
        DrawRotated(bPixelToLogic);     
}



void ScOutputData::DrawRotated(bool bPixelToLogic)
{
    
    SCCOL nRotMax = nX2;
    for (SCSIZE nRotY=0; nRotY<nArrCount; nRotY++)
        if (pRowInfo[nRotY].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nRotY].nRotMaxCol > nRotMax)
            nRotMax = pRowInfo[nRotY].nRotMaxCol;


    ScModule* pScMod = SC_MOD();
    sal_Int32 nConfBackColor = pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    bool bCellContrast = mbUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    ScFieldEditEngine* pEngine = NULL;
    bool bHyphenatorSet = false;
    const ScPatternAttr* pPattern;
    const SfxItemSet*    pCondSet;
    const ScPatternAttr* pOldPattern = NULL;
    const SfxItemSet*    pOldCondSet = NULL;
    ScRefCellValue aCell;

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
        nInitPosX += nMirrorW - 1;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    long nRowPosY = nScrY;
    for (SCSIZE nArrY=0; nArrY+1<nArrCount; nArrY++)            
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nCellHeight = (long) pThisRowInfo->nHeight;
        if (nArrY==1) nRowPosY = nScrY;                         

        if ( ( pThisRowInfo->bChanged || nArrY==0 ) && pThisRowInfo->nRotMaxCol != SC_ROTMAX_NONE )
        {
            long nPosX = 0;
            for (SCCOL nX=0; nX<=nRotMax; nX++)
            {
                if (nX==nX1) nPosX = nInitPosX;                 

                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                if ( pInfo->nRotateDir != SC_ROTDIR_NONE )
                {
                    SCROW nY = pThisRowInfo->nRowNo;

                    bool bHidden = false;
                    if (bEditMode)
                        if ( nX == nEditCol && nY == nEditRow )
                            bHidden = true;

                    if (!bHidden)
                    {
                        if (!pEngine)
                            pEngine = CreateOutputEditEngine();
                        else
                            lcl_ClearEdit( *pEngine );      

                        long nPosY = nRowPosY;
                        bool bVisChanged = false;

                        

                        bool bFromDoc = false;
                        pPattern = pInfo->pPatternAttr;
                        pCondSet = pInfo->pConditionSet;
                        if (!pPattern)
                        {
                            pPattern = mpDoc->GetPattern( nX, nY, nTab );
                            bFromDoc = true;
                        }
                        aCell = pInfo->maCell;
                        if (bFromDoc)
                            pCondSet = mpDoc->GetCondResult( nX, nY, nTab );

                        if (aCell.isEmpty() && nX>nX2)
                            GetVisibleCell( nX, nY, nTab, aCell );

                        if (aCell.isEmpty() || IsEmptyCellText(pThisRowInfo, nX, nY))
                            bHidden = true;     

                        long nCellWidth = (long) pRowInfo[0].pCellInfo[nX+1].nWidth;

                        SvxCellHorJustify eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                            pPattern->GetItem(ATTR_HOR_JUSTIFY, pCondSet)).GetValue();
                        bool bBreak = ( eHorJust == SVX_HOR_JUSTIFY_BLOCK ) ||
                                    ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK, pCondSet)).GetValue();
                        bool bRepeat = ( eHorJust == SVX_HOR_JUSTIFY_REPEAT && !bBreak );
                        bool bShrink = !bBreak && !bRepeat && static_cast<const SfxBoolItem&>
                                        (pPattern->GetItem( ATTR_SHRINKTOFIT, pCondSet )).GetValue();
                        SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );

                        const ScMergeAttr* pMerge =
                                (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                        bool bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

                        long nStartX = nPosX;
                        long nStartY = nPosY;
                        if (nX<nX1)
                        {
                            if ((bBreak || eOrient!=SVX_ORIENTATION_STANDARD) && !bMerged)
                                bHidden = true;
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

                        

                        if (!bHidden)
                        {
                            long nOutWidth = nCellWidth - 1;
                            long nOutHeight = nCellHeight;

                            if ( bMerged )                              
                            {
                                SCCOL nCountX = pMerge->GetColMerge();
                                for (SCCOL i=1; i<nCountX; i++)
                                    nOutWidth += (long) ( mpDoc->GetColWidth(nX+i,nTab) * mnPPTX );
                                SCROW nCountY = pMerge->GetRowMerge();
                                nOutHeight += (long) mpDoc->GetScaledRowHeight( nY+1, nY+nCountY-1, nTab, mnPPTY);
                            }

                            SvxCellVerJustify eVerJust = (SvxCellVerJustify)((const SvxVerJustifyItem&)
                                                pPattern->GetItem(ATTR_VER_JUSTIFY, pCondSet)).GetValue();

                            

                            
                            if ( pPattern != pOldPattern || pCondSet != pOldCondSet )
                            {
                                SfxItemSet* pSet = new SfxItemSet( pEngine->GetEmptyItemSet() );
                                pPattern->FillEditItemSet( pSet, pCondSet );

                                                                    
                                SvxAdjust eSvxAdjust = SVX_ADJUST_LEFT;
                                if (eOrient==SVX_ORIENTATION_STACKED)
                                    eSvxAdjust = SVX_ADJUST_CENTER;
                                
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
                                    
                                    com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
                                    pEngine->SetHyphenator( xXHyphenator );
                                    bHyphenatorSet = true;
                                }

                                Color aBackCol = ((const SvxBrushItem&)
                                    pPattern->GetItem( ATTR_BACKGROUND, pCondSet )).GetColor();
                                if ( mbUseStyleColor && ( aBackCol.GetTransparency() > 0 || bCellContrast ) )
                                    aBackCol.SetColor( nConfBackColor );
                                pEngine->SetBackgroundColor( aBackCol );
                            }

                            

                            

                            const SvxMarginItem* pMargin = (const SvxMarginItem*)
                                                    &pPattern->GetItem(ATTR_MARGIN, pCondSet);
                            sal_uInt16 nIndent = 0;
                            if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
                                nIndent = ((const SfxUInt16Item&)pPattern->
                                                    GetItem(ATTR_INDENT, pCondSet)).GetValue();

                            long nTotalHeight = nOutHeight; 
                            if ( bPixelToLogic )
                                nTotalHeight = mpRefDevice->PixelToLogic(Size(0,nTotalHeight)).Height();

                            long nLeftM = (long) ( (pMargin->GetLeftMargin() + nIndent) * mnPPTX );
                            long nTopM  = (long) ( pMargin->GetTopMargin() * mnPPTY );
                            long nRightM  = (long) ( pMargin->GetRightMargin() * mnPPTX );
                            long nBottomM = (long) ( pMargin->GetBottomMargin() * mnPPTY );
                            nStartX += nLeftM;
                            nStartY += nTopM;
                            nOutWidth -= nLeftM + nRightM;
                            nOutHeight -= nTopM + nBottomM;

                            
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
                                        eRotMode = SVX_ROTATE_MODE_STANDARD;    

                                    if ( bLayoutRTL )
                                        nAttrRotate = -nAttrRotate;

                                    double nRealOrient = nAttrRotate * F_PI18000;   
                                    nCos = cos( nRealOrient );
                                    nSin = sin( nRealOrient );
                                }
                            }

                            Size aPaperSize = Size( 1000000, 1000000 );
                            if (eOrient==SVX_ORIENTATION_STACKED)
                                aPaperSize.Width() = nOutWidth;             
                            else if (bBreak)
                            {
                                if (nAttrRotate)
                                {
                                    
                                    
                                    
                                    
                                    aPaperSize.Width() = (long)(nOutHeight / fabs(nSin));
                                }
                                else if (eOrient == SVX_ORIENTATION_STANDARD)
                                    aPaperSize.Width() = nOutWidth;
                                else
                                    aPaperSize.Width() = nOutHeight - 1;
                            }
                            if (bPixelToLogic)
                                pEngine->SetPaperSize(mpRefDevice->PixelToLogic(aPaperSize));
                            else
                                pEngine->SetPaperSize(aPaperSize);  

                            

                            if (aCell.meType == CELLTYPE_EDIT)
                            {
                                if (aCell.mpEditText)
                                    pEngine->SetText(*aCell.mpEditText);
                                else
                                {
                                    OSL_FAIL("pData == 0");
                                }
                            }
                            else
                            {
                                sal_uLong nFormat = pPattern->GetNumberFormat(
                                                            mpDoc->GetFormatTable(), pCondSet );
                                OUString aString;
                                Color* pColor;
                                ScCellFormat::GetString( aCell,
                                                         nFormat,aString, &pColor,
                                                         *mpDoc->GetFormatTable(),
                                                         mpDoc,
                                                         mbShowNullValues,
                                                         mbShowFormulas,
                                                         ftCheck );

                                pEngine->SetText(aString);
                                if ( pColor && !mbSyntaxMode && !( mbUseStyleColor && mbForceAutoColor ) )
                                    lcl_SetEditColor( *pEngine, *pColor );
                            }

                            if ( mbSyntaxMode )
                            {
                                SetEditSyntaxColor(*pEngine, aCell);
                            }
                            else if ( mbUseStyleColor && mbForceAutoColor )
                                lcl_SetEditColor( *pEngine, COL_AUTO );     

                            pEngine->SetUpdateMode( true );     

                            long nEngineWidth  = (long) pEngine->CalcTextWidth();
                            long nEngineHeight = pEngine->GetTextHeight();

                            if (nAttrRotate && bBreak)
                            {
                                double nAbsCos = fabs( nCos );
                                double nAbsSin = fabs( nSin );

                                
                                int nSteps = 5;
                                while (nSteps > 0)
                                {
                                    
                                    long nEnginePixel = mpRefDevice->LogicToPixel(
                                                            Size(0,nEngineHeight)).Height();
                                    long nEffHeight = nOutHeight - (long)(nEnginePixel * nAbsCos) + 2;
                                    long nNewWidth = (long)(nEffHeight / nAbsSin) + 2;
                                    bool bFits = ( nNewWidth >= aPaperSize.Width() );
                                    if ( bFits )
                                        nSteps = 0;
                                    else
                                    {
                                        if ( nNewWidth < 4 )
                                        {
                                            
                                            nEffHeight = nOutHeight / 2;
                                            nNewWidth = (long)(nEffHeight / nAbsSin) + 2;
                                            nSteps = 0;
                                        }
                                        else
                                            --nSteps;

                                        
                                        aPaperSize.Width() = nNewWidth;
                                        if (bPixelToLogic)
                                            pEngine->SetPaperSize(mpRefDevice->PixelToLogic(aPaperSize));
                                        else
                                            pEngine->SetPaperSize(aPaperSize);  
                                        
                                        nEngineWidth  = (long) pEngine->CalcTextWidth();
                                        nEngineHeight = pEngine->GetTextHeight();
                                    }
                                }
                            }

                            long nRealWidth  = nEngineWidth;
                            long nRealHeight = nEngineHeight;

                            
                            if (nAttrRotate)
                            {
                                double nAbsCos = fabs( nCos );
                                double nAbsSin = fabs( nSin );

                                if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                                    nEngineWidth = (long) ( nRealWidth * nAbsCos +
                                                            nRealHeight * nAbsSin );
                                else
                                    nEngineWidth = (long) ( nRealHeight / nAbsSin );
                                

                                nEngineHeight = (long) ( nRealHeight * nAbsCos +
                                                         nRealWidth * nAbsSin );
                            }

                            if (!nAttrRotate)           
                                bHidden = true;         

                            

                            if (!bHidden)
                            {
                                bool bClip = false;
                                Size aClipSize = Size( nScrX+nScrW-nStartX, nScrY+nScrH-nStartY );

                                

                                Size aCellSize;
                                if (bPixelToLogic)
                                    aCellSize = mpRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
                                else
                                    aCellSize = Size( nOutWidth, nOutHeight );  

                                long nGridWidth = nEngineWidth;
                                bool bNegative = false;
                                if ( eRotMode != SVX_ROTATE_MODE_STANDARD )
                                {
                                    nGridWidth = aCellSize.Width() +
                                            std::abs((long) ( aCellSize.Height() * nCos / nSin ));
                                    bNegative = ( pInfo->nRotateDir == SC_ROTDIR_LEFT );
                                    if ( bLayoutRTL )
                                        bNegative = !bNegative;
                                }

                                
                                
                                OutputAreaParam aAreaParam;

                                SCCOL nCellX = nX;
                                SCROW nCellY = nY;
                                SvxCellHorJustify eOutHorJust = eHorJust;
                                if ( eRotMode != SVX_ROTATE_MODE_STANDARD )
                                    eOutHorJust = bNegative ? SVX_HOR_JUSTIFY_RIGHT : SVX_HOR_JUSTIFY_LEFT;
                                long nNeededWidth = nGridWidth;     
                                if ( bPixelToLogic )
                                    nNeededWidth =  mpRefDevice->LogicToPixel(Size(nNeededWidth,0)).Width();

                                GetOutputArea( nX, nArrY, nCellStartX, nPosY, nCellX, nCellY, nNeededWidth,
                                                *pPattern, sal::static_int_cast<sal_uInt16>(eOutHorJust),
                                                false, false, true, aAreaParam );

                                if ( bShrink )
                                {
                                    long nPixelWidth = bPixelToLogic ?
                                        mpRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width() : nEngineWidth;
                                    long nNeededPixel = nPixelWidth + nLeftM + nRightM;

                                    aAreaParam.mbLeftClip = aAreaParam.mbRightClip = true;

                                    
                                    ShrinkEditEngine( *pEngine, aAreaParam.maAlignRect, nLeftM, nTopM, nRightM, nBottomM,
                                        false, sal::static_int_cast<sal_uInt16>(eOrient), nAttrRotate, bPixelToLogic,
                                        nEngineWidth, nEngineHeight, nNeededPixel, aAreaParam.mbLeftClip, aAreaParam.mbRightClip );

                                    if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                                    {
                                        
                                        ShrinkEditEngine( *pEngine, aAreaParam.maAlignRect, nLeftM, nTopM, nRightM, nBottomM,
                                            true, sal::static_int_cast<sal_uInt16>(eOrient), nAttrRotate, bPixelToLogic,
                                            nEngineWidth, nEngineHeight, nNeededPixel, aAreaParam.mbLeftClip, aAreaParam.mbRightClip );
                                    }

                                    
                                    
                                    nRealWidth  = (long) pEngine->CalcTextWidth();
                                    nRealHeight = pEngine->GetTextHeight();

                                    if ( eRotMode != SVX_ROTATE_MODE_STANDARD )
                                        nEngineWidth = (long) ( nRealHeight / fabs( nSin ) );
                                }

                                long nClipStartX = nStartX;
                                if (nX<nX1)
                                {
                                    

                                    bClip = true;                   
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
                                        bClip = true;
                                        nClipStartY = nRowPosY;
                                        aClipSize.Height() -= nDif;
                                    }
                                }

                                bClip = true;       

                                
                                if (bClip)
                                {
                                    if ( nAttrRotate /* && eRotMode != SVX_ROTATE_MODE_STANDARD */ )
                                    {
                                        
                                        
                                        nClipStartX = nScrX;
                                        aClipSize.Width() = nScrW;
                                    }

                                    if (bPixelToLogic)
                                        aAreaParam.maClipRect = mpRefDevice->PixelToLogic( Rectangle(
                                                        Point(nClipStartX,nClipStartY), aClipSize ) );
                                    else
                                        aAreaParam.maClipRect = Rectangle(Point(nClipStartX, nClipStartY),
                                                                aClipSize );    

                                    if (bMetaFile)
                                    {
                                        mpDev->Push();
                                        mpDev->IntersectClipRegion( aAreaParam.maClipRect );
                                    }
                                    else
                                        mpDev->SetClipRegion( Region( aAreaParam.maClipRect ) );
                                }

                                Point aLogicStart;
                                if (bPixelToLogic)
                                    aLogicStart = mpRefDevice->PixelToLogic( Point(nStartX,nStartY) );
                                else
                                    aLogicStart = Point(nStartX, nStartY);
                                if ( eOrient!=SVX_ORIENTATION_STANDARD || !bBreak )
                                {
                                    long nAvailWidth = aCellSize.Width();
                                    if (eType==OUTTYPE_WINDOW &&
                                            eOrient!=SVX_ORIENTATION_STACKED &&
                                            pInfo->bAutoFilter)
                                    {
                                        
                                        if (bPixelToLogic)
                                            nAvailWidth -= mpRefDevice->PixelToLogic(Size(0,DROPDOWN_BITMAP_SIZE)).Height();
                                        else
                                            nAvailWidth -= DROPDOWN_BITMAP_SIZE;
                                        long nComp = nEngineWidth;
                                        if (nAvailWidth<nComp) nAvailWidth=nComp;
                                    }

                                    

                                    if (eOrient==SVX_ORIENTATION_STANDARD && !nAttrRotate)
                                    {
                                        if (eHorJust==SVX_HOR_JUSTIFY_RIGHT ||
                                            eHorJust==SVX_HOR_JUSTIFY_CENTER)
                                        {
                                            pEngine->SetUpdateMode( false );

                                            SvxAdjust eSvxAdjust =
                                                (eHorJust==SVX_HOR_JUSTIFY_RIGHT) ?
                                                    SVX_ADJUST_RIGHT : SVX_ADJUST_CENTER;
                                            pEngine->SetDefaultItem(
                                                SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );

                                            aPaperSize.Width() = nOutWidth;
                                            if (bPixelToLogic)
                                                pEngine->SetPaperSize(mpRefDevice->PixelToLogic(aPaperSize));
                                            else
                                                pEngine->SetPaperSize(aPaperSize);

                                            pEngine->SetUpdateMode( true );
                                        }
                                    }
                                    else
                                    {
                                        
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
                                        aLogicStart.X() -= mpRefDevice->PixelToLogic(
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
                                            aLogicStart.Y() += mpRefDevice->PixelToLogic( Size(0,
                                                            mpRefDevice->LogicToPixel(aCellSize).Height() -
                                                            mpRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height()
                                                            )).Height();
                                        else
                                            aLogicStart.Y() += aCellSize.Height() - nEngineHeight;
                                    }

                                    else if (eVerJust==SVX_VER_JUSTIFY_CENTER)
                                    {
                                        if (bPixelToLogic)
                                            aLogicStart.Y() += mpRefDevice->PixelToLogic( Size(0,(
                                                            mpRefDevice->LogicToPixel(aCellSize).Height() -
                                                            mpRefDevice->LogicToPixel(Size(0,nEngineHeight)).Height())
                                                            / 2)).Height();
                                        else
                                            aLogicStart.Y() += (aCellSize.Height() - nEngineHeight) / 2;
                                    }
                                }

                                
                                OSL_ENSURE( eOrient == SVX_ORIENTATION_STANDARD && nAttrRotate,
                                            "DrawRotated: no rotation" );

                                long nOriVal = 0;
                                if ( nAttrRotate )
                                {
                                    
                                    nOriVal = nAttrRotate / 10;

                                    double nAddX = 0.0;
                                    double nAddY = 0.0;
                                    if ( nCos > 0.0 && eRotMode != SVX_ROTATE_MODE_STANDARD )
                                    {
                                        
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
                                        else    
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

                                

                                if ( pEngine->IsRightToLeft( 0 ) )
                                {
                                    
                                    
                                    
                                    Size aLogicPaper = pEngine->GetPaperSize();
                                    if ( aLogicPaper.Width() > USHRT_MAX )
                                    {
                                        aLogicPaper.Width() = USHRT_MAX;
                                        pEngine->SetPaperSize(aLogicPaper);
                                    }
                                }

                                pEngine->Draw( mpDev, aLogicStart, (short)nOriVal );

                                if (bClip)
                                {
                                    if (bMetaFile)
                                        mpDev->Pop();
                                    else
                                        mpDev->SetClipRegion();
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
