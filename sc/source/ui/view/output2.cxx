/*************************************************************************
 *
 *  $RCSfile: output2.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:06:40 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop


// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/adjitem.hxx>
#include <svx/algitem.hxx>
#include <svx/brshitem.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/colritem.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/forbiddencharacterstable.hxx>
#include <svx/frmdiritem.hxx>
#include <svx/langitem.hxx>
#include <svx/rotmodit.hxx>
#include <svx/scripttypeitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/unolingu.hxx>
#include <svtools/zforlist.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <math.h>

#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
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

            //! Autofilter-Breite mit column.cxx zusammenfassen
#define DROPDOWN_BITMAP_SIZE        17

#define DRAWTEXT_MAX    32767

const USHORT SC_SHRINKAGAIN_MAX = 7;

// STATIC DATA -----------------------------------------------------------


// -----------------------------------------------------------------------

class ScDrawStringsVars
{
    ScOutputData*       pOutput;                // Verbindung
    SvNumberFormatter*  pFormatter;

    const ScPatternAttr* pPattern;              // Attribute
    const SfxItemSet*   pCondSet;               // aus bedingter Formatierung

    Font                aFont;                  // aus Attributen erzeugt
    FontMetric          aMetric;
    long                nAscentPixel;           // always pixels
    SvxCellOrientation  eAttrOrient;
    SvxCellHorJustify   eAttrHorJust;
    SvxCellVerJustify   eAttrVerJust;
    const SvxMarginItem* pMargin;
    USHORT              nIndent;
    BOOL                bRotated;

    String              aString;                // Inhalte
    Size                aTextSize;
    long                nOriginalWidth;

    ScBaseCell*         pLastCell;
    ULONG               nValueFormat;
    BOOL                bLineBreak;
    BOOL                bRepeat;
    BOOL                bShrink;

    BOOL                bPixelToLogic;
    BOOL                bCellContrast;

    Color               aBackConfigColor;       // used for ScPatternAttr::GetFont calls
    Color               aTextConfigColor;

public:
                ScDrawStringsVars(ScOutputData* pData, BOOL bPTL);
                ~ScDrawStringsVars();

                //  SetPattern = ex-SetVars
                //  SetPatternSimple: ohne Font

    void        SetPattern( const ScPatternAttr* pNew, const SfxItemSet* pSet, ScBaseCell* pCell, BYTE nScript );
    void        SetPatternSimple( const ScPatternAttr* pNew, const SfxItemSet* pSet );

    BOOL        SetText( ScBaseCell* pCell );   // TRUE -> pOldPattern vergessen
    void        ResetText();
    void        SetHashText();
    void        SetAutoText( const String& rAutoText );

    const ScPatternAttr*    GetPattern() const      { return pPattern; }
    SvxCellOrientation      GetOrient() const       { return eAttrOrient; }
    SvxCellHorJustify       GetHorJust() const      { return eAttrHorJust; }
    SvxCellVerJustify       GetVerJust() const      { return eAttrVerJust; }
    const SvxMarginItem*    GetMargin() const       { return pMargin; }

    USHORT  GetLeftTotal() const        { return pMargin->GetLeftMargin() + nIndent; }

    const String&           GetString() const       { return aString; }
    const Size&             GetTextSize() const     { return aTextSize; }
    long                    GetOriginalWidth() const { return nOriginalWidth; }

    ULONG   GetValueFormat() const                  { return nValueFormat; }
    BOOL    GetLineBreak() const                    { return bLineBreak; }
    BOOL    IsRepeat() const                        { return bRepeat; }
    BOOL    IsShrink() const                        { return bShrink; }

    long    GetAscent() const   { return nAscentPixel; }
    BOOL    IsRotated() const   { return bRotated; }

    void    SetShrinkScale( long nScale, BYTE nScript );

    BOOL    HasCondHeight() const   { return pCondSet && SFX_ITEM_SET ==
                                        pCondSet->GetItemState( ATTR_FONT_HEIGHT, TRUE ); }

    BOOL    IsRightToLeftAttr() const;
};

//==================================================================

ScDrawStringsVars::ScDrawStringsVars(ScOutputData* pData, BOOL bPTL) :
    pOutput     ( pData ),
    bPixelToLogic( bPTL ),
    pPattern    ( NULL ),
    pCondSet    ( NULL ),
    pMargin     ( NULL ),
    nOriginalWidth( 0 ),
    nIndent     ( 0 ),
    bRotated    ( FALSE ),
    eAttrHorJust( SVX_HOR_JUSTIFY_STANDARD ),
    eAttrVerJust( SVX_VER_JUSTIFY_BOTTOM ),
    eAttrOrient ( SVX_ORIENTATION_STANDARD ),
    bLineBreak  ( FALSE ),
    bRepeat     ( FALSE ),
    bShrink     ( FALSE ),
    nValueFormat( 0 ),
    pLastCell   ( NULL )
{
    pFormatter = pData->pDoc->GetFormatTable();

    ScModule* pScMod = SC_MOD();
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed TRUE)
    bCellContrast = pOutput->bUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    const svtools::ColorConfig& rColorConfig = pScMod->GetColorConfig();
    aBackConfigColor.SetColor( rColorConfig.GetColorValue(svtools::DOCCOLOR).nColor );
    aTextConfigColor.SetColor( rColorConfig.GetColorValue(svtools::FONTCOLOR).nColor );
}

ScDrawStringsVars::~ScDrawStringsVars()
{
}

void ScDrawStringsVars::SetShrinkScale( long nScale, BYTE nScript )
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
    aFont.SetHeight( aTmpFont.GetHeight() );

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
                                    ScBaseCell* pCell, BYTE nScript )
{
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
            bRotated = FALSE;
            break;
        case SVX_ORIENTATION_TOPBOTTOM:
            nRot = 2700;
            bRotated = FALSE;
            break;
        case SVX_ORIENTATION_BOTTOMTOP:
            nRot = 900;
            bRotated = FALSE;
            break;
        default:
            DBG_ERROR("Falscher SvxCellOrientation Wert");
            nRot = 0;
            bRotated = FALSE;
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

    //  Zahlenformat

    ULONG nOld = nValueFormat;
    nValueFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );

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
    //  wird gerufen, wenn sich die Font-Variablen nicht aendern (!StringDiffer)

    pPattern = pNew;
    pCondSet = pSet;        //! noetig ???

    //  Zahlenformat

    ULONG nOld = nValueFormat;
//  nValueFormat = pPattern->GetNumberFormat( pFormatter );
    const SfxPoolItem* pFormItem;
    if ( !pCondSet || pCondSet->GetItemState(ATTR_VALUE_FORMAT,TRUE,&pFormItem) != SFX_ITEM_SET )
        pFormItem = &pPattern->GetItem(ATTR_VALUE_FORMAT);
    const SfxPoolItem* pLangItem;
    if ( !pCondSet || pCondSet->GetItemState(ATTR_LANGUAGE_FORMAT,TRUE,&pLangItem) != SFX_ITEM_SET )
        pLangItem = &pPattern->GetItem(ATTR_LANGUAGE_FORMAT);
    nValueFormat = pFormatter->GetFormatForLanguageIfBuiltIn(
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
}

inline BOOL SameValue( ScBaseCell* pCell, ScBaseCell* pOldCell )    // pCell ist != 0
{
    return pOldCell && pOldCell->GetCellType() == CELLTYPE_VALUE &&
            pCell->GetCellType() == CELLTYPE_VALUE &&
            ((ScValueCell*)pCell)->GetValue() == ((ScValueCell*)pOldCell)->GetValue();
}

BOOL ScDrawStringsVars::SetText( ScBaseCell* pCell )
{
    BOOL bChanged = FALSE;

    if (pCell)
    {
        if ( !SameValue( pCell, pLastCell ) )
        {
            pLastCell = pCell;          //  Zelle merken

            Color* pColor;
            ULONG nFormat = GetValueFormat();
            ScCellFormat::GetString( pCell,
                                     nFormat, aString, &pColor,
                                     *pFormatter,
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
                bChanged = TRUE;
                pLastCell = NULL;       // naechstes Mal wieder hierherkommen
            }

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

void ScDrawStringsVars::ResetText()
{
    aString.Erase();
    pLastCell = NULL;
    aTextSize = Size(0,0);
    nOriginalWidth = 0;
}

void ScDrawStringsVars::SetHashText()
{
    SetAutoText( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("###")) );
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

BOOL ScDrawStringsVars::IsRightToLeftAttr() const
{
    SvxFrameDirection eCellDir = (SvxFrameDirection)((const SvxFrameDirectionItem&)
                                    pPattern->GetItem( ATTR_WRITINGDIR, pCondSet )).GetValue();
    return ( eCellDir == FRMDIR_HORI_RIGHT_TOP ||
            ( eCellDir == FRMDIR_ENVIRONMENT && pOutput->nTabTextDirection == EE_HTEXTDIR_R2L ) );
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
//  Strings ausgeben
//

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
        }
        lcl_SetEditColor( rEngine, aColor );
    }
}

BOOL ScOutputData::GetMergeOrigin( SCCOL nX, SCROW nY, SCSIZE nArrY,
                                    SCCOL& rOverX, SCROW& rOverY,
                                    BOOL bVisRowChanged )
{
    BOOL bDoMerge = FALSE;
    BOOL bIsLeft = ( nX == nVisX1 );
    BOOL bIsTop  = ( nY == nVisY1 ) || bVisRowChanged;

    CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nX+1];
    if ( pInfo->bHOverlapped && pInfo->bVOverlapped )
        bDoMerge = bIsLeft && bIsTop;
    else if ( pInfo->bHOverlapped )
        bDoMerge = bIsLeft;
    else if ( pInfo->bVOverlapped )
        bDoMerge = bIsTop;

                                    // weiter solange versteckt
/*  if (!bDoMerge)
        return FALSE;
*/

    rOverX = nX;
    rOverY = nY;
    BOOL bHOver = pInfo->bHOverlapped;
    BOOL bVOver = pInfo->bVOverlapped;
    BOOL bHidden;

    while (bHOver)              // nY konstant
    {
        --rOverX;
        bHidden = ( (pDoc->GetColFlags(rOverX,nTab) & CR_HIDDEN) != 0 );
        if ( !bDoMerge && !bHidden )
            return FALSE;

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
            USHORT nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                rOverX, rOverY, nTab, ATTR_MERGE_FLAG ))->GetValue();
            bHOver = ((nOverlap & SC_MF_HOR) != 0);
            bVOver = ((nOverlap & SC_MF_VER) != 0);
        }
    }

    while (bVOver)
    {
        --rOverY;
        bHidden = ( (pDoc->GetRowFlags(rOverY,nTab) & CR_HIDDEN) != 0 );
        if ( !bDoMerge && !bHidden )
            return FALSE;

        if (nArrY>0)
            --nArrY;                        // lokale Kopie !

        if (rOverX >= nX1 && rOverY >= nY1 &&
            (pDoc->GetColFlags(rOverX,nTab) & CR_HIDDEN) == 0 &&
            (pDoc->GetRowFlags(rOverY,nTab) & CR_HIDDEN) == 0 &&
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
            USHORT nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                rOverX, rOverY, nTab, ATTR_MERGE_FLAG ))->GetValue();
            bHOver = ((nOverlap & SC_MF_HOR) != 0);
            bVOver = ((nOverlap & SC_MF_VER) != 0);
        }
    }

    return TRUE;
}

inline BOOL StringDiffer( const ScPatternAttr*& rpOldPattern, const ScPatternAttr*& rpNewPattern )
{
    DBG_ASSERT( rpNewPattern, "pNewPattern" );

    if ( rpNewPattern == rpOldPattern )
        return FALSE;
    else if ( !rpOldPattern )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT ) != &rpOldPattern->GetItem( ATTR_FONT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT ) != &rpOldPattern->GetItem( ATTR_CJK_FONT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT ) != &rpOldPattern->GetItem( ATTR_CTL_FONT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_HEIGHT ) != &rpOldPattern->GetItem( ATTR_FONT_HEIGHT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT_HEIGHT ) != &rpOldPattern->GetItem( ATTR_CJK_FONT_HEIGHT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT_HEIGHT ) != &rpOldPattern->GetItem( ATTR_CTL_FONT_HEIGHT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_WEIGHT ) != &rpOldPattern->GetItem( ATTR_FONT_WEIGHT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT_WEIGHT ) != &rpOldPattern->GetItem( ATTR_CJK_FONT_WEIGHT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT_WEIGHT ) != &rpOldPattern->GetItem( ATTR_CTL_FONT_WEIGHT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_POSTURE ) != &rpOldPattern->GetItem( ATTR_FONT_POSTURE ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_CJK_FONT_POSTURE ) != &rpOldPattern->GetItem( ATTR_CJK_FONT_POSTURE ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_CTL_FONT_POSTURE ) != &rpOldPattern->GetItem( ATTR_CTL_FONT_POSTURE ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_UNDERLINE ) != &rpOldPattern->GetItem( ATTR_FONT_UNDERLINE ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_WORDLINE ) != &rpOldPattern->GetItem( ATTR_FONT_WORDLINE ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_CROSSEDOUT ) != &rpOldPattern->GetItem( ATTR_FONT_CROSSEDOUT ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_CONTOUR ) != &rpOldPattern->GetItem( ATTR_FONT_CONTOUR ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_SHADOWED ) != &rpOldPattern->GetItem( ATTR_FONT_SHADOWED ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_COLOR ) != &rpOldPattern->GetItem( ATTR_FONT_COLOR ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_HOR_JUSTIFY ) != &rpOldPattern->GetItem( ATTR_HOR_JUSTIFY ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_VER_JUSTIFY ) != &rpOldPattern->GetItem( ATTR_VER_JUSTIFY ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_STACKED ) != &rpOldPattern->GetItem( ATTR_STACKED ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_LINEBREAK ) != &rpOldPattern->GetItem( ATTR_LINEBREAK ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_MARGIN ) != &rpOldPattern->GetItem( ATTR_MARGIN ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_ROTATE_VALUE ) != &rpOldPattern->GetItem( ATTR_ROTATE_VALUE ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FORBIDDEN_RULES ) != &rpOldPattern->GetItem( ATTR_FORBIDDEN_RULES ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_EMPHASISMARK ) != &rpOldPattern->GetItem( ATTR_FONT_EMPHASISMARK ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_FONT_RELIEF ) != &rpOldPattern->GetItem( ATTR_FONT_RELIEF ) )
        return TRUE;
    else if ( &rpNewPattern->GetItem( ATTR_BACKGROUND ) != &rpOldPattern->GetItem( ATTR_BACKGROUND ) )
        return TRUE;    // needed with automatic text color
    else
    {
        rpOldPattern = rpNewPattern;
        return FALSE;
    }
}

inline void lcl_CreateInterpretProgress( BOOL& bProgress, ScDocument* pDoc,
        ScFormulaCell* pFCell )
{
    if ( !bProgress && pFCell->GetDirty() )
    {
        ScProgress::CreateInterpretProgress( pDoc, TRUE );
        bProgress = TRUE;
    }
}

BOOL lcl_IsValueDataAtPos( BOOL& bProgress, ScDocument* pDoc,
        SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    ScBaseCell* pCell;
    pDoc->GetCell( nCol, nRow, nTab, pCell );
    if ( pCell )
    {
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
        {   // kein weiteres Interpret anstossen
            ScFormulaCell* pFCell = (ScFormulaCell*) pCell;
            if ( pFCell->IsRunning() )
                return TRUE;
            else
            {
                lcl_CreateInterpretProgress( bProgress, pDoc, pFCell );
                return pFCell->HasValueData();
            }
        }
        else
            return pCell->HasValueData();
    }
    else
        return FALSE;
}

inline BYTE GetScriptType( ScDocument* pDoc, ScBaseCell* pCell,
                            const ScPatternAttr* pPattern,
                            const SfxItemSet* pCondSet )
{
    return pDoc->GetCellScriptType( pCell, pPattern->GetNumberFormat( pDoc->GetFormatTable(), pCondSet ) );
}

inline BOOL IsAmbiguousScript( BYTE nScript )
{
    return ( nScript != SCRIPTTYPE_LATIN &&
             nScript != SCRIPTTYPE_ASIAN &&
             nScript != SCRIPTTYPE_COMPLEX );
}

BOOL ScOutputData::IsEmptyCellText( RowInfo* pThisRowInfo, SCCOL nX, SCROW nY )
{
    // pThisRowInfo may be NULL

    BOOL bEmpty;
    if ( pThisRowInfo && nX <= nX2 )
        bEmpty = pThisRowInfo->pCellInfo[nX+1].bEmptyCellText;
    else
        bEmpty = ( pDoc->GetCell( ScAddress( nX, nY, nTab ) ) == NULL );

    if ( !bEmpty && ( nX < nX1 || nX > nX2 || !pThisRowInfo ) )
    {
        //  for the range nX1..nX2 in RowInfo, cell protection attribute is already evaluated
        //  into bEmptyCellText in ScDocument::FillInfo / lcl_HidePrint (printfun)

        BOOL bIsPrint = ( eType == OUTTYPE_PRINTER );

        if ( bIsPrint || bTabProtected )
        {
            const ScProtectionAttr* pAttr = (const ScProtectionAttr*)
                    pDoc->GetEffItem( nX, nY, nTab, ATTR_PROTECTION );
            if ( bIsPrint && pAttr->GetHidePrint() )
                bEmpty = TRUE;
            else if ( bTabProtected )
            {
                if ( pAttr->GetHideCell() )
                    bEmpty = TRUE;
                else if ( bShowFormulas && pAttr->GetHideFormula() )
                {
                    ScBaseCell* pCell = pDoc->GetCell( ScAddress( nX, nY, nTab ) );
                    if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
                        bEmpty = TRUE;
                }
            }
        }
    }
    return bEmpty;
}

void ScOutputData::GetVisibleCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell*& rpCell )
{
    pDoc->GetCell( nCol, nRow, nTab, rpCell );
    if ( rpCell && IsEmptyCellText( NULL, nCol, nRow ) )
        rpCell = NULL;
}

BOOL ScOutputData::IsAvailable( SCCOL nX, SCROW nY )
{
    //  apply the same logic here as in DrawStrings/DrawEdit:
    //  Stop at non-empty or merged or overlapped cell,
    //  where a note is empty as well as a cell that's hidden by protection settings

    const ScBaseCell* pCell = pDoc->GetCell( ScAddress( nX, nY, nTab ) );
    if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE && !IsEmptyCellText( NULL, nX, nY ) )
    {
        return FALSE;
    }

    const ScPatternAttr* pPattern = pDoc->GetPattern( nX, nY, nTab );
    if ( ((const ScMergeAttr&)pPattern->GetItem(ATTR_MERGE)).IsMerged() ||
         ((const ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).IsOverlapped() )
    {
        return FALSE;
    }

    return TRUE;
}

long ScOutputData::GetAvailableWidth( SCCOL nX, SCROW nY, long nNeeded )
{
    //  get the pixel width that's available for the cell's text,
    //  including cells outside of the current screen area

    long nAvailable = (long) ( pDoc->GetColWidth( nX, nTab ) * nPPTX );     // cell itself

    const ScMergeAttr* pMerge = (const ScMergeAttr*)pDoc->GetAttr( nX, nY, nTab, ATTR_MERGE );
    if ( pMerge->IsMerged() )
    {
        //  for merged cells, allow only the merged area
        SCCOL nCount = pMerge->GetColMerge();
        for (SCCOL nAdd=1; nAdd<nCount; nAdd++)
            nAvailable += (long) ( pDoc->GetColWidth( nX + nAdd, nTab ) * nPPTX );
    }
    else
    {
        //  look for empty cells into which the text can be extended
        while ( nAvailable < nNeeded && nX < MAXCOL && IsAvailable( nX+1, nY ) )
        {
            ++nX;
            nAvailable += (long) ( pDoc->GetColWidth( nX, nTab ) * nPPTX );
        }
    }

    if ( bMarkClipped && nAvailable < nNeeded )
        nAvailable -= (long)( SC_CLIPMARK_SIZE * nPPTX );

    return nAvailable;
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
// rAlignRect:      output: single or merged cell, used for alignment (visual rectangle)
// rClipRect:       output: total output area, for clipping (visual)
// rLeftClip:       output: need to clip at rClipRect left (visual) edge
// rRightClip:      output: same for right

void ScOutputData::GetOutputArea( SCCOL nX, SCSIZE nArrY, long nPosX, long nPosY,
                                    SCCOL nCellX, SCROW nCellY, long nNeeded,
                                    const ScPatternAttr& rPattern,
                                    USHORT nHorJustify, BOOL bCellIsValue,
                                    BOOL bBreak, BOOL bOverwrite,
                                    Rectangle& rAlignRect, Rectangle& rClipRect,
                                    BOOL& rLeftClip, BOOL& rRightClip )
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
            USHORT nDocHeight = pDoc->GetRowHeight( nCompRow, nTab );
            if ( nDocHeight )
                nCellPosY += (long) ( nDocHeight * nPPTY );
            ++nCompRow;
        }
    }
    while ( nCellY < nCompRow )
    {
        --nCompRow;
        USHORT nDocHeight = pDoc->GetRowHeight( nCompRow, nTab );
        if ( nDocHeight )
            nCellPosY -= (long) ( nDocHeight * nPPTY );
    }

    const ScMergeAttr* pMerge = (const ScMergeAttr*)&rPattern.GetItem( ATTR_MERGE );
    BOOL bMerged = pMerge->IsMerged();
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
                (long) ( pDoc->GetColWidth( nCellX+i, nTab ) * nPPTX );
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
    for ( i=nDirect; i<nMergeRows; i++ )
    {
        // following rows always from document
        nMergeSizeY += (long) ( pDoc->GetRowHeight( nCellY+static_cast<SCROW>(i), nTab ) * nPPTY );
    }

    --nMergeSizeX;      // leave out the grid horizontally, also for alignment (align between grid lines)

    //
    // construct the rectangles using logical left/right values (justify is called at the end)
    //

    //  rAlignRect is the single cell or merged area, used for alignment.

    rAlignRect.Left() = nCellPosX;
    rAlignRect.Right() = nCellPosX + ( nMergeSizeX - 1 ) * nLayoutSign;
    rAlignRect.Top() = nCellPosY;
    rAlignRect.Bottom() = nCellPosY + nMergeSizeY - 1;

    //  rClipRect is all cells that are used for output.
    //  For merged cells this is the same as rAlignRect, otherwise neighboring cells can also be used.

    rClipRect = rAlignRect;
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
                rClipRect.Right() += nAdd * nLayoutSign;

                if ( rThisRowInfo.nRowNo == nCellY && nRightX >= nX1 && nRightX <= nX2 )
                    rThisRowInfo.pCellInfo[nRightX].bHideGrid = TRUE;
            }

            while ( nLeftMissing > 0 && nLeftX > 0 && ( bOverwrite || IsAvailable( nLeftX-1, nCellY ) ) )
            {
                if ( rThisRowInfo.nRowNo == nCellY && nLeftX >= nX1 && nLeftX <= nX2 )
                    rThisRowInfo.pCellInfo[nLeftX].bHideGrid = TRUE;

                --nLeftX;
                long nAdd = (long) ( pDoc->GetColWidth( nLeftX, nTab ) * nPPTX );
                nLeftMissing -= nAdd;
                rClipRect.Left() -= nAdd * nLayoutSign;
            }
        }

        //  Set flag and reserve space for clipping mark triangle,
        //  even if rThisRowInfo isn't for nCellY (merged cells).
        if ( nRightMissing > 0 && bMarkClipped && nRightX >= nX1 && nRightX <= nX2 && !bBreak && !bCellIsValue )
        {
            rThisRowInfo.pCellInfo[nRightX+1].nClipMark |= SC_CLIPMARK_RIGHT;
            bAnyClipped = TRUE;
            long nMarkPixel = (long)( SC_CLIPMARK_SIZE * nPPTX );
            rClipRect.Right() -= nMarkPixel * nLayoutSign;
        }
        if ( nLeftMissing > 0 && bMarkClipped && nLeftX >= nX1 && nLeftX <= nX2 && !bBreak && !bCellIsValue )
        {
            rThisRowInfo.pCellInfo[nLeftX+1].nClipMark |= SC_CLIPMARK_LEFT;
            bAnyClipped = TRUE;
            long nMarkPixel = (long)( SC_CLIPMARK_SIZE * nPPTX );
            rClipRect.Left() += nMarkPixel * nLayoutSign;
        }

        rLeftClip = ( nLeftMissing > 0 );
        rRightClip = ( nRightMissing > 0 );
    }
    else
        rLeftClip = rRightClip = FALSE;

    //  justify both rectangles for alignment calculation, use with DrawText etc.

    rAlignRect.Justify();
    rClipRect.Justify();

#if 0
    //! Test !!!
    pDev->Push();
    pDev->SetLineColor();
    pDev->SetFillColor( COL_LIGHTGREEN );
    pDev->DrawRect( pDev->PixelToLogic(rClipRect) );
    pDev->DrawRect( rClipRect );    // print preview
    pDev->Pop();
    //! Test !!!
#endif
}

void ScOutputData::DrawStrings( BOOL bPixelToLogic )
{
    DBG_ASSERT( pDev == pRefDevice ||
                pDev->GetMapMode().GetMapUnit() == pRefDevice->GetMapMode().GetMapUnit(),
                "DrawStrings: unterschiedliche MapUnits ?!?!" );

    BOOL bWasIdleDisabled = pDoc->IsIdleDisabled();
    pDoc->DisableIdle( TRUE );
    Size aMinSize = pRefDevice->PixelToLogic(Size(0,100));      // erst darueber wird ausgegeben
    UINT32 nMinHeight = aMinSize.Height() / 200;                // 1/2 Pixel

    ScDrawStringsVars aVars( this, bPixelToLogic );

    const ScPatternAttr* pOldPattern = NULL;
    const SfxItemSet* pOldCondSet = NULL;
    BYTE nOldScript = 0;

    BOOL bProgress = FALSE;

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
        nInitPosX += nMirrorW - 1;              // pixels
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    SCCOL nLastContentCol = MAXCOL;
    if ( nX2 < MAXCOL )
        nLastContentCol -= pDoc->GetEmptyLinesInBlock( nX2+1, nY1, nTab, MAXCOL, nY2, nTab, DIR_RIGHT );
    SCCOL nLoopStartX = nX1;
    if ( nX1 > 0 )
        --nLoopStartX;          // start before nX1 for rest of long text to the left

    // variables for GetOutputArea
    BOOL bCellIsValue = FALSE;
    BOOL bLeftClip = FALSE;
    BOOL bRightClip = FALSE;
    long nNeededWidth = 0;
    Rectangle aAlignRect;
    Rectangle aClipRect;
    SvxCellHorJustify eOutHorJust = SVX_HOR_JUSTIFY_STANDARD;
    const ScPatternAttr* pPattern = NULL;
    const SfxItemSet* pCondSet = NULL;

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->bChanged )
        {
            SCROW nY = pThisRowInfo->nRowNo;
            long nCellHeight = (long) pThisRowInfo->nHeight;
            long nPosX = nInitPosX;
            if ( nLoopStartX < nX1 )
                nPosX -= pRowInfo[0].pCellInfo[nLoopStartX+1].nWidth * nLayoutSign;
            for (SCCOL nX=nLoopStartX; nX<=nX2; nX++)
            {
                BOOL bMergeEmpty = FALSE;
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                BOOL bEmpty = nX < nX1 || pInfo->bEmptyCellText;

                SCCOL nCellX = nX;                  // position where the cell really starts
                SCROW nCellY = nY;
                BOOL bDoCell = FALSE;
                BOOL bNeedEdit = FALSE;

                //
                //  Part of a merged cell?
                //

                if ( pInfo->bHOverlapped || pInfo->bVOverlapped )
                {
                    bEmpty = TRUE;

                    SCCOL nOverX;                   // start of the merged cells
                    SCROW nOverY;
                    BOOL bVisChanged = !pRowInfo[nArrY-1].bChanged;
                    if (GetMergeOrigin( nX,nY, nArrY, nOverX,nOverY, bVisChanged ))
                    {
                        nCellX = nOverX;
                        nCellY = nOverY;
                        bDoCell = TRUE;
                    }
                    else
                        bMergeEmpty = TRUE;
                }

                //
                //  Rest of a long text further to the left?
                //

                if ( bEmpty && !bMergeEmpty && nX < nX1 )
                {
                    SCCOL nTempX=nX1;
                    while (nTempX > 0 && IsEmptyCellText( pThisRowInfo, nTempX, nY ))
                        --nTempX;

                    if ( nTempX < nX1 &&
                         !IsEmptyCellText( pThisRowInfo, nTempX, nY ) &&
                         !pDoc->HasAttrib( nTempX,nY,nTab, nX1,nY,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                    {
                        nCellX = nTempX;
                        bDoCell = TRUE;
                    }
                }

                //
                //  Rest of a long text further to the right?
                //

                if ( bEmpty && !bMergeEmpty && nX == nX2 )
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
                        bDoCell = TRUE;
                    }
                }

                //
                //  normal visible cell
                //

                if (!bEmpty)
                    bDoCell = TRUE;

                //
                //  don't output the cell that's being edited
                //

                if ( bDoCell && bEditMode && nCellX == nEditCol && nCellY == nEditRow )
                    bDoCell = FALSE;

                //
                //  output the cell text
                //

                ScBaseCell* pCell = NULL;
                if (bDoCell)
                {
                    if ( nCellY == nY && nCellX >= nX1 && nCellX <= nX2 )
                        pCell = pThisRowInfo->pCellInfo[nCellX+1].pCell;
                    else
                        GetVisibleCell( nCellX, nCellY, nTab, pCell );      // get from document
                    if ( !pCell )
                        bDoCell = FALSE;
                    else if ( pCell->GetCellType() == CELLTYPE_EDIT )
                        bNeedEdit = TRUE;
                }
                if (bDoCell && !bNeedEdit)
                {
                    if ( nCellY == nY && nCellX >= nX1 && nCellX <= nX2 )
                    {
                        CellInfo& rCellInfo = pThisRowInfo->pCellInfo[nCellX+1];
                        pPattern = rCellInfo.pPatternAttr;
                        pCondSet = rCellInfo.pConditionSet;
                    }
                    else        // get from document
                    {
                        pPattern = pDoc->GetPattern( nCellX, nCellY, nTab );
                        pCondSet = pDoc->GetCondResult( nCellX, nCellY, nTab );
                    }

                    BYTE nScript = GetScriptType( pDoc, pCell, pPattern, pCondSet );
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
                        bNeedEdit = TRUE;
                }
                if (bDoCell && !bNeedEdit)
                {
                    if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                        lcl_CreateInterpretProgress( bProgress, pDoc, (ScFormulaCell*)pCell );
                    if ( aVars.SetText(pCell) )
                        pOldPattern = NULL;

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

                    BOOL bBreak = ( aVars.GetLineBreak() || aVars.GetHorJust() == SVX_HOR_JUSTIFY_BLOCK );
                    BOOL bRepeat = aVars.IsRepeat() && !bBreak;
                    BOOL bShrink = aVars.IsShrink() && !bBreak && !bRepeat;

                    long nTotalMargin = (long) ( aVars.GetLeftTotal() * nPPTX ) +
                                        (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                    nNeededWidth = aVars.GetTextSize().Width() + nTotalMargin;
                    // GetOutputArea gives justfied rectangles
                    GetOutputArea( nX, nArrY, nPosX, nPosY, nCellX, nCellY, nNeededWidth,
                                    *pPattern, eOutHorJust, bCellIsValue || bRepeat || bShrink, bBreak, FALSE,
                                    aAlignRect, aClipRect, bLeftClip, bRightClip );

                    if ( bShrink && ( bLeftClip || bRightClip ) )
                    {
                        long nAvailable = aAlignRect.GetWidth() - nTotalMargin;
                        long nScaleSize = aVars.GetTextSize().Width();         // without margin
                        long nScale = ( nAvailable * 100 ) / nScaleSize;

                        aVars.SetShrinkScale( nScale, nOldScript );
                        long nNewSize = aVars.GetTextSize().Width();

                        USHORT nShrinkAgain = 0;
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
                            bLeftClip = bRightClip = FALSE;

                        pOldPattern = NULL;
                    }

                    if ( bRepeat && !bLeftClip && !bRightClip )
                    {
                        long nAvailable = aAlignRect.GetWidth() - nTotalMargin;
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
                            bNeedEdit = ( bLeftClip || bRightClip );
                        else
                        {
                            long nHeight = aVars.GetTextSize().Height() +
                                            (long)(aVars.GetMargin()->GetTopMargin()*nPPTY) +
                                            (long)(aVars.GetMargin()->GetBottomMargin()*nPPTY);
                            bNeedEdit = ( nHeight > aClipRect.GetHeight() );
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
                    pMarkRowInfo->pCellInfo[nMarkX+1].bEditEngine = TRUE;
                    bDoCell = FALSE;    // don't draw here
                }
                if ( bDoCell )
                {
                    if ( bCellIsValue && ( bLeftClip || bRightClip ) )
                    {
                        aVars.SetHashText();
                        nNeededWidth = aVars.GetTextSize().Width() +
                                    (long) ( aVars.GetLeftTotal() * nPPTX ) +
                                    (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                        if ( nNeededWidth <= aClipRect.GetWidth() )
                            bLeftClip = bRightClip = FALSE;

                        //  If the "###" replacement doesn't fit into the cells, no clip marks
                        //  are shown, as the "###" already denotes too little space.
                        //  The rectangles from the first GetOutputArea call remain valid.
                    }

                    long nJustPosX = aAlignRect.Left();     // "justified" - effect of alignment will be added
                    long nJustPosY = aAlignRect.Top();
                    long nAvailWidth = aAlignRect.GetWidth();
                    long nOutHeight = aAlignRect.GetHeight();

                    BOOL bOutside = ( aClipRect.Right() < nScrX || aClipRect.Left() >= nScrX + nScrW );
                    if ( aClipRect.Left() < nScrX )
                    {
                        aClipRect.Left() = nScrX;
                        bLeftClip = TRUE;
                    }
                    if ( aClipRect.Right() > nScrX + nScrW )
                    {
                        aClipRect.Right() = nScrX + nScrW;          //! minus one?
                        bRightClip = TRUE;
                    }

                    BOOL bHClip = bLeftClip || bRightClip;
                    BOOL bVClip = FALSE;

                    //
                    //      horizontalen Platz testen
                    //

                    BOOL bRightAdjusted = FALSE;        // to correct text width calculation later
                    BOOL bNeedEditEngine = FALSE;
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
                                bRightAdjusted = TRUE;
                                break;
                            case SVX_HOR_JUSTIFY_CENTER:
                                nJustPosX += ( nAvailWidth - aVars.GetTextSize().Width() +
                                            (long) ( aVars.GetLeftTotal() * nPPTX ) -
                                            (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX ) ) / 2;
                                break;
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
                        }

                        if ( nTestClipHeight > nOutHeight )
                        {
                            //  kein vertikales Clipping beim Drucken von Zellen mit
                            //  optimaler Hoehe, ausser bei Groesse in bedingter Formatierung
                            if ( eType != OUTTYPE_PRINTER ||
                                    ( pDoc->GetRowFlags( nCellY, nTab ) & CR_MANUALSIZE ) ||
                                    ( aVars.HasCondHeight() ) )
                                bVClip = TRUE;
                        }

                        if ( bHClip || bVClip )
                        {
                            //  nur die betroffene Dimension clippen,
                            //  damit bei nicht-proportionalem Resize nicht alle
                            //  rechtsbuendigen Zahlen abgeschnitten werden:

                            if (!bHClip)
                            {
                                aClipRect.Left() = nScrX;
                                aClipRect.Right() = nScrX+nScrW;
                            }
                            if (!bVClip)
                            {
                                aClipRect.Top() = nScrY;
                                aClipRect.Bottom() = nScrY+nScrH;
                            }

                            //  aClipRect is not used after SetClipRegion/IntersectClipRegion,
                            //  so it can be modified here
                            if (bPixelToLogic)
                                aClipRect = pRefDevice->PixelToLogic( aClipRect );

                            if (bMetaFile)
                            {
                                pDev->Push();
                                pDev->IntersectClipRegion( aClipRect );
                            }
                            else
                                pDev->SetClipRegion( Region( aClipRect ) );
                        }

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
                            long* pDX = new long[aString.Len()];
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

Size lcl_GetVertPaperSize( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    const double nPPTY = HMM_PER_TWIPS;

    const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
    const ScMergeAttr& rMerge = (const ScMergeAttr&)pPattern->GetItem(ATTR_MERGE);

    long nCellY = (long) ( pDoc->GetRowHeight(nRow,nTab) * nPPTY );
    if ( rMerge.GetRowMerge() > 1 )
    {
        SCROW nCountY = rMerge.GetRowMerge();
        for (SCROW i=1; i<nCountY; i++)
            nCellY += (long) ( pDoc->GetRowHeight(nRow+i,nTab) * nPPTY );
    }

    //  only top/bottom margin are interesting
    const SvxMarginItem& rMargin = (const SvxMarginItem&)pPattern->GetItem(ATTR_MARGIN);
    nCellY -= (long) ( rMargin.GetTopMargin() * nPPTY );
    nCellY -= (long) ( rMargin.GetBottomMargin() * nPPTY );

    return Size( nCellY - 1, 1000000 );     // cell height as width for PaperSize
}

void lcl_ClearEdit( EditEngine& rEngine )       // Text und Attribute
{
    rEngine.SetUpdateMode( FALSE );

    rEngine.SetText(EMPTY_STRING);
    //  keine Para-Attribute uebrigbehalten...
    const SfxItemSet& rPara = rEngine.GetParaAttribs(0);
    if (rPara.Count())
        rEngine.SetParaAttribs( 0,
                    SfxItemSet( *rPara.GetPool(), rPara.GetRanges() ) );
}

BOOL lcl_SafeIsValue( ScBaseCell* pCell )
{
    if (!pCell)
        return FALSE;

    BOOL bRet = FALSE;
    switch ( pCell->GetCellType() )
    {
        case CELLTYPE_VALUE:
            bRet = TRUE;
            break;
        case CELLTYPE_FORMULA:
            {
                ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                if ( pFCell->IsRunning() || pFCell->IsValue() )
                    bRet = TRUE;
            }
            break;
    }
    return bRet;
}

void lcl_ScaleFonts( EditEngine& rEngine, long nPercent )
{
    BOOL bUpdateMode = rEngine.GetUpdateMode();
    if ( bUpdateMode )
        rEngine.SetUpdateMode( FALSE );

    USHORT nParCount = rEngine.GetParagraphCount();
    for (USHORT nPar=0; nPar<nParCount; nPar++)
    {
        SvUShorts aPortions;
        rEngine.GetPortions( nPar, aPortions );

        USHORT nPCount = aPortions.Count();
        USHORT nStart = 0;
        for ( USHORT nPos=0; nPos<nPCount; nPos++ )
        {
            USHORT nEnd = aPortions.GetObject( nPos );
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
        rEngine.SetUpdateMode( TRUE );
}

void ScOutputData::DrawEdit(BOOL bPixelToLogic)
{
    Size aMinSize = pRefDevice->PixelToLogic(Size(0,100));      // erst darueber wird ausgegeben
    UINT32 nMinHeight = aMinSize.Height() / 200;                // 1/2 Pixel

    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

    ScModule* pScMod = SC_MOD();
    sal_Int32 nConfBackColor = pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed TRUE)
    BOOL bCellContrast = bUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    ScFieldEditEngine* pEngine = NULL;
    BOOL bHyphenatorSet = FALSE;
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
        nLastContentCol -= pDoc->GetEmptyLinesInBlock( nX2+1, nY1, nTab, MAXCOL, nY2, nTab, DIR_RIGHT );

    long nRowPosY = nScrY;
    for (SCSIZE nArrY=0; nArrY+1<nArrCount; nArrY++)            // 0 fuer Reste von zusammengefassten
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nCellHeight = (long) pThisRowInfo->nHeight;
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
                    BOOL bDoCell = FALSE;

                    long nPosY = nRowPosY;
                    if ( nArrY == 0 )
                    {
                        nPosY = nScrY;
                        nY = pRowInfo[1].nRowNo;
                        SCCOL nOverX;                   // start of the merged cells
                        SCROW nOverY;
                        if (GetMergeOrigin( nX,nY, 1, nOverX,nOverY, TRUE ))
                        {
                            nCellX = nOverX;
                            nCellY = nOverY;
                            bDoCell = TRUE;
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
                            bDoCell = TRUE;
                        }
                    }
                    else
                    {
                        bDoCell = TRUE;
                    }

                    if ( bDoCell && bEditMode && nCellX == nEditCol && nCellY == nEditRow )
                        bDoCell = FALSE;

                    const ScPatternAttr* pPattern;
                    const SfxItemSet* pCondSet;
                    if (bDoCell)
                    {
                        if ( nCellY == nY && nCellX >= nX1 && nCellX <= nX2 )
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
                            bDoCell = FALSE;
                    }
                    if (bDoCell)
                    {
                        BOOL bHidden = FALSE;

                        //
                        //  Create EditEngine
                        //

                        if (!pEngine)
                        {
                            //  Ein RefDevice muss auf jeden Fall gesetzt werden,
                            //  sonst legt sich die EditEngine ein VirtualDevice an!
                            pEngine = new ScFieldEditEngine( pDoc->GetEnginePool() );
                            pEngine->SetUpdateMode( FALSE );
                            pEngine->SetRefDevice( pFmtDevice );    // always set
                            ULONG nCtrl = pEngine->GetControlWord();
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
                            lcl_ClearEdit( *pEngine );      // also calls SetUpdateMode(FALSE)


                        BOOL bVisChanged = FALSE;
                        BOOL bCellIsValue = lcl_SafeIsValue(pCell);

                        SvxCellHorJustify eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                            pPattern->GetItem(ATTR_HOR_JUSTIFY, pCondSet)).GetValue();
                        BOOL bBreak = ( eHorJust == SVX_HOR_JUSTIFY_BLOCK ) ||
                                        ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK, pCondSet)).GetValue();
                        BOOL bRepeat = ( eHorJust == SVX_HOR_JUSTIFY_REPEAT && !bBreak );
                        BOOL bShrink = !bBreak && !bRepeat && static_cast<const SfxBoolItem&>
                                        (pPattern->GetItem( ATTR_SHRINKTOFIT, pCondSet )).GetValue();
                        SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );
                        long nAttrRotate = ((const SfxInt32Item&)pPattern->
                                            GetItem(ATTR_ROTATE_VALUE, pCondSet)).GetValue();
                        if ( eHorJust == SVX_HOR_JUSTIFY_REPEAT )
                        {
                            // ignore orientation/rotation if "repeat" is active
                            eOrient = SVX_ORIENTATION_STANDARD;
                            nAttrRotate = 0;
                        }
                        if ( eOrient==SVX_ORIENTATION_STANDARD && nAttrRotate )
                        {
                            //! Flag setzen, um die Zelle in DrawRotated wiederzufinden ?
                            //! (oder Flag schon bei DrawBackground, dann hier keine Abfrage)
                            bHidden = TRUE;     // gedreht wird getrennt ausgegeben
                        }

                        BOOL bAsianVertical = ( eOrient == SVX_ORIENTATION_STACKED &&
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
//!                         bHidden = TRUE;
//!                     }

                        if (!bHidden)
                        {
                            //! mirror margin values for RTL?
                            //! move margin down to after final GetOutputArea call

                            const SvxMarginItem* pMargin = (const SvxMarginItem*)
                                                    &pPattern->GetItem(ATTR_MARGIN, pCondSet);
                            USHORT nIndent = 0;
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

                            Rectangle aAlignRect;
                            Rectangle aClipRect;
                            BOOL bLeftClip = FALSE;
                            BOOL bRightClip = FALSE;

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
                                                *pPattern, eOutHorJust, bCellIsValue, TRUE, FALSE,
                                                aAlignRect, aClipRect, bLeftClip, bRightClip );

                                //! special ScEditUtil handling if formatting for printer

                                if ( eOrient == SVX_ORIENTATION_TOPBOTTOM || eOrient == SVX_ORIENTATION_BOTTOMTOP )
                                    aPaperSize.Width() = aAlignRect.GetHeight() - nTopM - nBottomM;
                                else
                                    aPaperSize.Width() = aAlignRect.GetWidth() - nLeftM - nRightM;

                                if (bAsianVertical && bBreak)
                                {
                                    //  add some extra height (default margin value) for safety
                                    //  as long as GetEditArea isn't used below
                                    long nExtraHeight = (long)( 20 * nPPTY );
                                    aPaperSize.Height() = aAlignRect.GetHeight() - nTopM - nBottomM + nExtraHeight;
                                }
                            }
                            if (bPixelToLogic)
                                pEngine->SetPaperSize(pRefDevice->PixelToLogic(aPaperSize));
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

                                ULONG nControl = pEngine->GetControlWord();
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
                                    bHyphenatorSet = TRUE;
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

                            //  Raender

                            //!     Position und Papersize auf EditUtil umstellen !!!

/*                          Rectangle aPixRect = ScEditUtil( pDoc,
                                                nX,nY,nTab, Point(nStartX,nStartY),
                                                pDev, nPPTX, nPPTY, nZoom )
                                                .GetEditArea( pPattern );

                            pDev->SetFillInBrush(Brush(Color(COL_LIGHTRED)));
                            pDev->DrawRect(pDev->PixelToLogic(aPixRect));
*/

#if 0
                            Size aPaperSize = Size( 1000000, 1000000 );
                            if ( eOrient==SVX_ORIENTATION_STACKED )
                                aPaperSize.Width() = nOutWidth;             // zum Zentrieren
                            else if (bAsianVertical)
                            {
                                aPaperSize.Width() = nOutWidth;
                                if (bBreak)
                                {
                                    //  add some extra height (default margin value) for safety
                                    //  as long as GetEditArea isn't used below
                                    long nExtraHeight = (long)( 20 * nPPTY );
                                    aPaperSize.Height() = nOutHeight + nExtraHeight;
                                }
                            }
                            else if (bBreak)
                            {
                                if (eOrient == SVX_ORIENTATION_STANDARD)
                                {
                                    if (eType==OUTTYPE_WINDOW &&
                                            eOrient!=SVX_ORIENTATION_STACKED &&
                                            pInfo && pInfo->bAutoFilter)
                                    {
                                        long nSub = Min( pRowInfo[nArrY].nHeight,
                                                        (USHORT) DROPDOWN_BITMAP_SIZE );
                                        if ( nOutWidth > nSub )
                                            nOutWidth -= nSub;
                                    }
                                    aPaperSize.Width() = nOutWidth;
                                }
                                else
                                    aPaperSize.Width() = nOutHeight - 1;
                            }
                            if (bPixelToLogic)
                            {
                                //! also handle bAsianVertical in GetEditArea
                                if ( bBreak && pFmtDevice != pRefDevice && !bAsianVertical )
                                {
                                    //  calculate PaperSize for automatic line breaks from logic size,
                                    //  not pixel sizes, to get the same breaks at all scales

                                    if ( eOrient == SVX_ORIENTATION_STANDARD )
                                    {
                                        Fraction aFract(1,1);
                                        Rectangle aUtilRect = ScEditUtil( pDoc,nX,nY,nTab,
                                                                Point(nStartX,nStartY), pDev,
                                                                HMM_PER_TWIPS, HMM_PER_TWIPS, aFract, aFract )
                                                            .GetEditArea( pPattern, FALSE );
                                        Size aLogic = aUtilRect.GetSize();
                                        pEngine->SetPaperSize( aLogic );
                                    }
                                    else
                                        pEngine->SetPaperSize( lcl_GetVertPaperSize(pDoc,nX,nY,nTab) );
                                }
                                else
                                    pEngine->SetPaperSize(pRefDevice->PixelToLogic(aPaperSize));
                            }
                            else
                                pEngine->SetPaperSize(aPaperSize);
#endif

                            //  Read content from cell

                            BOOL bWrapFields = FALSE;
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

                                            bWrapFields = TRUE;
                                        }
                                    }
                                    else
                                        DBG_ERROR("pData == 0");
                                }
                                else
                                {
                                    ULONG nFormat = pPattern->GetNumberFormat(
                                                                pFormatter, pCondSet );
                                    String aString;
                                    Color* pColor;
                                    ScCellFormat::GetString( pCell,
                                                             nFormat,aString, &pColor,
                                                             *pFormatter,
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
                                DBG_ERROR("pCell == NULL");

                            pEngine->SetVertical( bAsianVertical );
                            pEngine->SetUpdateMode( TRUE );     // after SetText, before CalcTextWidth/GetTextHeight

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

                            if ( ( !bBreak && eOrient != SVX_ORIENTATION_STACKED ) || bAsianVertical )
                            {
                                // for break, the first GetOutputArea call is sufficient
                                GetOutputArea( nXForPos, nArrYForPos, nPosX, nPosY, nCellX, nCellY, nNeededPixel,
                                                *pPattern, eOutHorJust, bCellIsValue || bRepeat || bShrink, FALSE, FALSE,
                                                aAlignRect, aClipRect, bLeftClip, bRightClip );

                                if ( bShrink && ( bLeftClip || bRightClip ) )
                                {
                                    long nAvailable = aAlignRect.GetWidth() - nLeftM - nRightM;
                                    long nScaleSize = nNeededPixel - nLeftM - nRightM;      // without margin
                                    long nScale = ( nAvailable * 100 ) / nScaleSize;

                                    lcl_ScaleFonts( *pEngine, nScale );
                                    nEngineWidth = (long) pEngine->CalcTextWidth();
                                    long nNewSize = bPixelToLogic ?
                                        pRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width() : nEngineWidth;

                                    USHORT nShrinkAgain = 0;
                                    while ( nNewSize > nAvailable && nShrinkAgain < SC_SHRINKAGAIN_MAX )
                                    {
                                        // further reduce, like in DrawStrings
                                        lcl_ScaleFonts( *pEngine, 90 );     // reduce by 10%
                                        nEngineWidth = (long) pEngine->CalcTextWidth();
                                        nNewSize = bPixelToLogic ?
                                            pRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width() : nEngineWidth;
                                        ++nShrinkAgain;
                                    }
                                    if ( nNewSize <= nAvailable )
                                        bLeftClip = bRightClip = FALSE;

                                    // sizes for further processing (alignment etc):
                                    nNeededPixel = nNewSize + nLeftM + nRightM;
                                    nEngineHeight = pEngine->GetTextHeight();
                                }

                                if ( bRepeat && !bLeftClip && !bRightClip && pEngine->GetParagraphCount() == 1 )
                                {
                                    // First check if twice the space for the formatted text is available
                                    // (otherwise just keep it unchanged).

                                    long nFormatted = nNeededPixel - nLeftM - nRightM;      // without margin
                                    long nAvailable = aAlignRect.GetWidth() - nLeftM - nRightM;
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

                                if ( bCellIsValue && ( bLeftClip || bRightClip ) )
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

                            long nStartX = aAlignRect.Left();
                            long nStartY = aAlignRect.Top();
                            long nCellWidth = aAlignRect.GetWidth();
                            long nOutWidth = nCellWidth - 1 - nLeftM - nRightM;
                            long nOutHeight = aAlignRect.GetHeight() - nTopM - nBottomM;

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

                            BOOL bOutside = ( aClipRect.Right() < nScrX || aClipRect.Left() >= nScrX + nScrW );
                            if ( aClipRect.Left() < nScrX )
                            {
                                aClipRect.Left() = nScrX;
                                bLeftClip = TRUE;
                            }
                            if ( aClipRect.Right() > nScrX + nScrW )
                            {
                                aClipRect.Right() = nScrX + nScrW;          //! minus one?
                                bRightClip = TRUE;
                            }

                            if ( !bHidden && !bOutside )
                            {
                                BOOL bClip = bLeftClip || bRightClip;
                                BOOL bSimClip = FALSE;

                                if ( bWrapFields )
                                {
                                    //  Fields in a cell with automatic breaks: clip to cell width
                                    bClip = TRUE;
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
                                    BOOL bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

                                    //  Don't clip for text height when printing rows with optimal height,
                                    //  except when font size is from conditional formatting.
                                    //! Allow clipping when vertically merged?
                                    if ( eType != OUTTYPE_PRINTER ||
                                        ( pDoc->GetRowFlags( nCellY, nTab ) & CR_MANUALSIZE ) ||
                                        ( pCondSet && SFX_ITEM_SET ==
                                            pCondSet->GetItemState(ATTR_FONT_HEIGHT, TRUE) ) )
                                        bClip = TRUE;
                                    else
                                        bSimClip = TRUE;

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
                                        bAnyClipped = TRUE;

                                        long nMarkPixel = (long)( SC_CLIPMARK_SIZE * nPPTX );
                                        if ( aClipRect.Right() - nMarkPixel > aClipRect.Left() )
                                            aClipRect.Right() -= nMarkPixel;
                                    }
                                }

#if 0
                                long nClipStartY = nStartY;
                                if (nArrY==0 || bVisChanged)
                                {
                                    if ( nClipStartY < nRowPosY )
                                    {
                                        long nDif = nRowPosY - nClipStartY;
                                        bClip = TRUE;
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
                                        aLogicClip = pRefDevice->PixelToLogic( aClipRect );
                                    else
                                        aLogicClip = aClipRect;

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
                                    if (eType==OUTTYPE_WINDOW &&
                                            eOrient!=SVX_ORIENTATION_STACKED &&
                                            pInfo && pInfo->bAutoFilter)
                                    {
                                        USHORT nSub = Min( pRowInfo[nArrY].nHeight,
                                                            (USHORT) DROPDOWN_BITMAP_SIZE );
                                        if (bPixelToLogic)
                                            nAvailWidth -= pRefDevice->PixelToLogic(Size(0,nSub)).Height();
                                        else
                                            nAvailWidth -= nSub;
                                        if (nAvailWidth < nEngineWidth) nAvailWidth = nEngineWidth;

                                        nOutWidth -= nSub;
                                        long nEnginePixel = bPixelToLogic ?
                                                pRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width() :
                                                nEngineWidth;
                                        if (nOutWidth <= nEnginePixel) nOutWidth = nEnginePixel+1;
                                    }

                                    //  horizontale Ausrichtung

                                    if (eOrient==SVX_ORIENTATION_STANDARD && !bAsianVertical)
                                    {
                                        if (eHorJust==SVX_HOR_JUSTIFY_RIGHT ||
                                            eHorJust==SVX_HOR_JUSTIFY_CENTER ||
                                            (eHorJust==SVX_HOR_JUSTIFY_STANDARD && bCellIsValue) )
                                        {
                                            pEngine->SetUpdateMode( FALSE );

                                            SvxAdjust eSvxAdjust =
                                                (eHorJust==SVX_HOR_JUSTIFY_CENTER) ?
                                                    SVX_ADJUST_CENTER : SVX_ADJUST_RIGHT;
                                            pEngine->SetDefaultItem(
                                                SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );

                                            // #55142# reset adjustment for the next cell
                                            pOldPattern = NULL;

                                            pEngine->SetUpdateMode( TRUE );
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
                                    //  of USHORT type, so the PaperSize must be limited to USHRT_MAX.
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
                                    pEngine->Draw( pDev, aLogicClip, aDocStart, FALSE );
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

void ScOutputData::DrawRotated(BOOL bPixelToLogic)
{
    //! nRotMax speichern
    SCCOL nRotMax = nX2;
    for (SCSIZE nRotY=0; nRotY<nArrCount; nRotY++)
        if (pRowInfo[nRotY].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nRotY].nRotMaxCol > nRotMax)
            nRotMax = pRowInfo[nRotY].nRotMaxCol;


    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

    ScModule* pScMod = SC_MOD();
    sal_Int32 nConfBackColor = pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed TRUE)
    BOOL bCellContrast = bUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    ScFieldEditEngine* pEngine = NULL;
    BOOL bHyphenatorSet = FALSE;
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

                    BOOL bHidden = FALSE;
                    if (bEditMode)
                        if ( nX == nEditCol && nY == nEditRow )
                            bHidden = TRUE;

                    if (!bHidden)
                    {
                        if (!pEngine)
                        {
                            //  Ein RefDevice muss auf jeden Fall gesetzt werden,
                            //  sonst legt sich die EditEngine ein VirtualDevice an!
                            pEngine = new ScFieldEditEngine( pDoc->GetEnginePool() );
                            pEngine->SetUpdateMode( FALSE );
                            pEngine->SetRefDevice( pFmtDevice );    // always set
                            ULONG nCtrl = pEngine->GetControlWord();
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
                            lcl_ClearEdit( *pEngine );      // also calls SetUpdateMode(FALSE)

                        long nPosY = nRowPosY;
                        BOOL bVisChanged = FALSE;

                        //! Rest von zusammengefasster Zelle weiter oben funktioniert nicht!

                        BOOL bFromDoc = FALSE;
                        pPattern = pInfo->pPatternAttr;
                        pCondSet = pInfo->pConditionSet;
                        if (!pPattern)
                        {
                            pPattern = pDoc->GetPattern( nX, nY, nTab );
                            bFromDoc = TRUE;
                        }
                        pCell = pInfo->pCell;
                        if (bFromDoc)
                            pCondSet = pDoc->GetCondResult( nX, nY, nTab );

                        if (!pCell && nX>nX2)
                            GetVisibleCell( nX, nY, nTab, pCell );

                        if ( !pCell || IsEmptyCellText( pThisRowInfo, nX, nY ) )
                            bHidden = TRUE;     // nRotateDir is also set without a cell

                        long nCellWidth = (long) pRowInfo[0].pCellInfo[nX+1].nWidth;

                        SvxCellHorJustify eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                            pPattern->GetItem(ATTR_HOR_JUSTIFY, pCondSet)).GetValue();
                        BOOL bBreak = ( eHorJust == SVX_HOR_JUSTIFY_BLOCK ) ||
                                    ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK, pCondSet)).GetValue();
                        SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );

                        const ScMergeAttr* pMerge =
                                (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                        BOOL bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

                        long nStartX = nPosX;
                        long nStartY = nPosY;
                        if (nX<nX1)
                        {
                            if ((bBreak || eOrient!=SVX_ORIENTATION_STANDARD) && !bMerged)
                                bHidden = TRUE;
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
                                const ScMergeAttr* pMerge =
                                        (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                                SCCOL nCountX = pMerge->GetColMerge();
                                for (SCCOL i=1; i<nCountX; i++)
                                    nOutWidth += (long) ( pDoc->GetColWidth(nX+i,nTab) * nPPTX );
                                SCROW nCountY = pMerge->GetRowMerge();
                                for (SCROW j=1; j<nCountY; j++)
                                    nOutHeight += (long) ( pDoc->GetRowHeight(nY+j,nTab) * nPPTY );
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

                                ULONG nControl = pEngine->GetControlWord();
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
                                    bHyphenatorSet = TRUE;
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
                            USHORT nIndent = 0;
                            if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
                                nIndent = ((const SfxUInt16Item&)pPattern->
                                                    GetItem(ATTR_INDENT, pCondSet)).GetValue();

                            long nTotalHeight = nOutHeight; // ohne Rand abzuziehen
                            if ( bPixelToLogic )
                                nTotalHeight = pRefDevice->PixelToLogic(Size(0,nTotalHeight)).Height();

                            long nLeftM = (long) ( (pMargin->GetLeftMargin() + nIndent) * nPPTX );
                            long nTopM  = (long) ( pMargin->GetTopMargin() * nPPTY );
                            nStartX += nLeftM;
                            nStartY += nTopM;
                            nOutWidth -= nLeftM + (long) ( pMargin->GetRightMargin() * nPPTX );
                            nOutHeight -= nTopM + (long) ( pMargin->GetBottomMargin() * nPPTY );

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
                                        DBG_ERROR("pData == 0");
                                }
                                else
                                {
                                    ULONG nFormat = pPattern->GetNumberFormat(
                                                                pFormatter, pCondSet );
                                    String aString;
                                    Color* pColor;
                                    ScCellFormat::GetString( pCell,
                                                             nFormat,aString, &pColor,
                                                             *pFormatter,
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
                                DBG_ERROR("pCell == NULL");

                            pEngine->SetUpdateMode( TRUE );     // after SetText, before CalcTextWidth/GetTextHeight

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
                                    BOOL bFits = ( nNewWidth >= aPaperSize.Width() );
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
                                        //pEngine->QuickFormatDoc( TRUE );
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
                                bHidden = TRUE;         //! vorher abfragen !!!

                            //! weglassen, was nicht hereinragt

                            if (!bHidden)
                            {
                                BOOL bClip = FALSE;
                                Size aClipSize = Size( nScrX+nScrW-nStartX, nScrY+nScrH-nStartY );

                                //  weiterschreiben

                                Size aCellSize;
                                if (bPixelToLogic)
                                    aCellSize = pRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
                                else
                                    aCellSize = Size( nOutWidth, nOutHeight );  // Scale ist 1

                                long nGridWidth = nEngineWidth;
                                BOOL bNegative = FALSE;
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
                                BOOL bLeftClip = FALSE;
                                BOOL bRightClip = FALSE;
                                Rectangle aAlignRect;
                                Rectangle aClipRect;

                                SCCOL nCellX = nX;
                                SCROW nCellY = nY;
                                SvxCellHorJustify eOutHorJust = eHorJust;
                                if ( eRotMode != SVX_ROTATE_MODE_STANDARD )
                                    eOutHorJust = bNegative ? SVX_HOR_JUSTIFY_RIGHT : SVX_HOR_JUSTIFY_LEFT;
                                long nNeededWidth = nGridWidth;     // in pixel for GetOutputArea
                                if ( bPixelToLogic )
                                    nNeededWidth =  pRefDevice->LogicToPixel(Size(nNeededWidth,0)).Width();

                                GetOutputArea( nX, nArrY, nCellStartX, nPosY, nCellX, nCellY, nNeededWidth,
                                                *pPattern, eOutHorJust, FALSE, FALSE, TRUE,
                                                aAlignRect, aClipRect, bLeftClip, bRightClip );

                                // BOOL bVClip = ( nEngineHeight > aCellSize.Height() );

                                long nClipStartX = nStartX;
                                if (nX<nX1)
                                {
                                    //! Clipping unnoetig, wenn links am Fenster

                                    bClip = TRUE;                   // nur Rest ausgeben!
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
                                        bClip = TRUE;
                                        nClipStartY = nRowPosY;
                                        aClipSize.Height() -= nDif;
                                    }
                                }

                                bClip = TRUE;       // always clip at the window/page border

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
                                        aClipRect = pRefDevice->PixelToLogic( Rectangle(
                                                        Point(nClipStartX,nClipStartY), aClipSize ) );
                                    else
                                        aClipRect = Rectangle(Point(nClipStartX, nClipStartY),
                                                                aClipSize );    // Scale = 1

                                    if (bMetaFile)
                                    {
                                        pDev->Push();
                                        pDev->IntersectClipRegion( aClipRect );
                                    }
                                    else
                                        pDev->SetClipRegion( Region( aClipRect ) );
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
                                        if (pRowInfo[nArrY].nHeight < DROPDOWN_BITMAP_SIZE)
                                        {
                                            if (bPixelToLogic)
                                                nAvailWidth -= pRefDevice->PixelToLogic(Size(0,pRowInfo[nArrY].nHeight)).Height();
                                            else
                                                nAvailWidth -= pRowInfo[nArrY].nHeight;
                                        }
                                        else
                                        {
                                            if (bPixelToLogic)
                                                nAvailWidth -= pRefDevice->PixelToLogic(Size(0,DROPDOWN_BITMAP_SIZE)).Height();
                                            else
                                                nAvailWidth -= DROPDOWN_BITMAP_SIZE;
                                        }
                                        long nComp = nEngineWidth;
                                        if (nAvailWidth<nComp) nAvailWidth=nComp;
                                    }

                                    //  horizontale Ausrichtung

                                    if (eOrient==SVX_ORIENTATION_STANDARD && !nAttrRotate)
                                    {
                                        if (eHorJust==SVX_HOR_JUSTIFY_RIGHT ||
                                            eHorJust==SVX_HOR_JUSTIFY_CENTER)
                                        {
                                            pEngine->SetUpdateMode( FALSE );

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

                                            pEngine->SetUpdateMode( TRUE );
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
                                    //  of USHORT type, so the PaperSize must be limited to USHRT_MAX.
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



