/*************************************************************************
 *
 *  $RCSfile: output2.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 09:45:13 $
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

#include "output.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "editutil.hxx"
#include "progress.hxx"
#include "scmod.hxx"

            //! Autofilter-Breite mit column.cxx zusammenfassen
#define DROPDOWN_BITMAP_SIZE        17

#define DRAWTEXT_MAX    32767


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

    long    GetAscent() const   { return nAscentPixel; }
    BOOL    IsRotated() const   { return bRotated; }

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

    const SfxPoolItem* pCondItem;
    if ( pCondSet && pCondSet->GetItemState( ATTR_ORIENTATION, TRUE, &pCondItem ) == SFX_ITEM_SET )
        eAttrOrient = (SvxCellOrientation)((const SvxOrientationItem*)pCondItem)->GetValue();
    else
        eAttrOrient = (SvxCellOrientation)
            ((const SvxOrientationItem&)pPattern->GetItem(ATTR_ORIENTATION)).GetValue();

    short nRot;
    switch (eAttrOrient)
    {
        case SVX_ORIENTATION_STANDARD:
            nRot = 0;
            long nRotate;
            if ( pCondSet && pCondSet->GetItemState( ATTR_ROTATE_VALUE, TRUE, &pCondItem ) ==
                                SFX_ITEM_SET )
                nRotate = ((const SfxInt32Item*)pCondItem)->GetValue();
            else
                nRotate = ((const SfxInt32Item&)pPattern->GetItem(ATTR_ROTATE_VALUE)).GetValue();
            bRotated = ( nRotate != 0 );
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

    Color aULineColor;
    if ( pCondSet && pCondSet->GetItemState( ATTR_FONT_UNDERLINE, TRUE, &pCondItem ) == SFX_ITEM_SET )
        aULineColor = ((const SvxUnderlineItem*)pCondItem)->GetColor();
    else
        aULineColor = ((const SvxUnderlineItem&)pPattern->GetItem(ATTR_FONT_UNDERLINE)).GetColor();
    pDev->SetTextLineColor( aULineColor );

    //  Ausrichtung

    if ( pCondSet && pCondSet->GetItemState( ATTR_HOR_JUSTIFY, TRUE, &pCondItem ) == SFX_ITEM_SET )
        eAttrHorJust = (SvxCellHorJustify)
            ((const SvxHorJustifyItem*)pCondItem)->GetValue();
    else
        eAttrHorJust = (SvxCellHorJustify)
            ((const SvxHorJustifyItem&)pPattern->GetItem(ATTR_HOR_JUSTIFY)).GetValue();

    if ( pCondSet && pCondSet->GetItemState( ATTR_VER_JUSTIFY, TRUE, &pCondItem ) == SFX_ITEM_SET )
        eAttrVerJust = (SvxCellVerJustify)
            ((const SvxVerJustifyItem*)pCondItem)->GetValue();
    else
        eAttrVerJust = (SvxCellVerJustify)
            ((const SvxVerJustifyItem&)pPattern->GetItem(ATTR_VER_JUSTIFY)).GetValue();
    if ( eAttrVerJust == SVX_VER_JUSTIFY_STANDARD )
        eAttrVerJust = SVX_VER_JUSTIFY_BOTTOM;

    //  Umbruch

    if ( pCondSet && pCondSet->GetItemState( ATTR_LINEBREAK, TRUE, &pCondItem ) == SFX_ITEM_SET )
        bLineBreak = ((const SfxBoolItem*)pCondItem)->GetValue();
    else
        bLineBreak = ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue();

    //  Zahlenformat

    ULONG nOld = nValueFormat;
    nValueFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );

/*  s.u.
    if (nValueFormat != nOld)
        pLastCell = NULL;           // immer neu formatieren
*/
    //  Raender

    if ( pCondSet && pCondSet->GetItemState( ATTR_MARGIN, TRUE, &pCondItem ) == SFX_ITEM_SET )
        pMargin = (const SvxMarginItem*)pCondItem;
    else
        pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);
    if ( eAttrHorJust == SVX_HOR_JUSTIFY_LEFT )
    {
        if ( pCondSet && pCondSet->GetItemState( ATTR_INDENT, TRUE, &pCondItem ) == SFX_ITEM_SET )
            nIndent = ((const SfxUInt16Item*)pCondItem)->GetValue();
        else
            nIndent = ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue();
    }
    else
        nIndent = 0;

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

    const SfxPoolItem* pCondItem;
    if ( pCondSet && pCondSet->GetItemState( ATTR_MARGIN, TRUE, &pCondItem ) == SFX_ITEM_SET )
        pMargin = (const SvxMarginItem*)pCondItem;
    else
        pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);
    if ( eAttrHorJust == SVX_HOR_JUSTIFY_LEFT )
    {
        if ( pCondSet && pCondSet->GetItemState( ATTR_INDENT, TRUE, &pCondItem ) == SFX_ITEM_SET )
            nIndent = ((const SfxUInt16Item*)pCondItem)->GetValue();
        else
            nIndent = ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue();
    }
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
    aString = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("###"));

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

BOOL ScOutputData::GetMergeOrigin( USHORT nX, USHORT nY, USHORT nArrY,
                                    USHORT& rOverX, USHORT& rOverY,
                                    long& rVirtPosX, long& rVirtPosY,
                                    BOOL bClipVirt, BOOL bVisRowChanged )
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
            rVirtPosX -= pRowInfo[0].pCellInfo[rOverX+1].nWidth;
            bHOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bHOverlapped;
            bVOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bVOverlapped;
        }
        else
        {
            if (!bClipVirt)
                rVirtPosX -= (long) (pDoc->GetColWidth( rOverX, nTab ) * nPPTX);
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
            rVirtPosY -= pRowInfo[nArrY].nHeight;
            bHOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bHOverlapped;
            bVOver = pRowInfo[nArrY].pCellInfo[rOverX+1].bVOverlapped;
        }
        else
        {
            if (!bClipVirt)
                rVirtPosY -= (long) (pDoc->GetRowHeight( rOverY, nTab ) * nPPTY);
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
    else if ( &rpNewPattern->GetItem( ATTR_ORIENTATION ) != &rpOldPattern->GetItem( ATTR_ORIENTATION ) )
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
        USHORT nCol, USHORT nRow, USHORT nTab )
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

BOOL ScOutputData::IsEmptyCellText( RowInfo* pThisRowInfo, USHORT nX, USHORT nY )
{
    // pThisRowInfo may be NULL

    BOOL bEmpty = pThisRowInfo && nX <= nX2 && pThisRowInfo->pCellInfo[nX+1].bEmptyCellText;
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

void ScOutputData::GetVisibleCell( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell*& rpCell )
{
    pDoc->GetCell( nCol, nRow, nTab, rpCell );
    if ( rpCell && IsEmptyCellText( NULL, nCol, nRow ) )
        rpCell = NULL;
}

BOOL ScOutputData::IsAvailable( USHORT nX, USHORT nY )
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

long ScOutputData::GetAvailableWidth( USHORT nX, USHORT nY, long nNeeded )
{
    //  get the pixel width that's available for the cell's text,
    //  including cells outside of the current screen area

    long nAvailable = (long) ( pDoc->GetColWidth( nX, nTab ) * nPPTX );     // cell itself

    const ScMergeAttr* pMerge = (const ScMergeAttr*)pDoc->GetAttr( nX, nY, nTab, ATTR_MERGE );
    if ( pMerge->IsMerged() )
    {
        //  for merged cells, allow only the merged area
        USHORT nCount = pMerge->GetColMerge();
        for (USHORT nAdd=1; nAdd<nCount; nAdd++)
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

    long nPosY = nScrY;
    for (USHORT nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->bChanged )
        {
            USHORT nY = pThisRowInfo->nRowNo;
            long nCellHeight = (long) pThisRowInfo->nHeight;
            long nPosX = nScrX;
            for (USHORT nX=nX1; nX<=nX2; nX++)
            {
                ScForceTextFmt eForceText = ftDontForce;

                BOOL bLeftOver  = FALSE;            // Ueberhang von langem String (immer linksb.)
                BOOL bMergeOver = FALSE;            // Rest von zusammengefassten Zellen
                BOOL bMergeEmpty= FALSE;
                long nVirtPosX  = nPosX;
                long nVirtPosY  = nPosY;
                long nOutWidth  = 0;
                long nOutHeight = nCellHeight;
                USHORT nOverX;                      // Start der zusammengefassten Zellen
                USHORT nOverY;

                CellInfo*   pInfo = &pThisRowInfo->pCellInfo[nX+1];
                BOOL bEmpty = pInfo->bEmptyCellText;

                USHORT nCellX = nX;                 // position where the cell really starts
                USHORT nCellY = nY;

                //
                //  Teil von zusammengefasster Zelle ?
                //

                if ( pInfo->bHOverlapped || pInfo->bVOverlapped )
                {
                    bEmpty = TRUE;

                    BOOL bVisChanged = !pRowInfo[nArrY-1].bChanged;
                    if (GetMergeOrigin( nX,nY, nArrY, nOverX,nOverY, nVirtPosX,nVirtPosY,
                                            FALSE, bVisChanged ))
                    {
                        ScBaseCell* pCell;
                        GetVisibleCell( nOverX, nOverY, nTab, pCell );
                        if (pCell)
                        {
                            BOOL bEdit = ( pCell->GetCellType() == CELLTYPE_EDIT );
                            if (!bEdit)
                            {
                                const ScPatternAttr* pPattern =
                                        pDoc->GetPattern( nOverX, nOverY, nTab );
                                const SfxItemSet* pCondSet =
                                        pDoc->GetCondResult( nOverX, nOverY, nTab );
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

                                if ( aVars.GetOrient() == SVX_ORIENTATION_STACKED ||
                                     aVars.IsRotated() || IsAmbiguousScript(nScript) )
                                    bEdit = TRUE;
                            }
                            if (!bEdit)
                            {
                                if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                                    lcl_CreateInterpretProgress( bProgress,
                                        pDoc, (ScFormulaCell*)pCell );
                                if ( aVars.SetText(pCell) )
                                    pOldPattern = NULL;

                                if (nOverX >= nX1)
                                    nOutWidth = ((long)pRowInfo[0].pCellInfo[nOverX+1].nWidth) - 1;
                                else
                                    nOutWidth = ((long)(pDoc->GetColWidth( nOverX, nTab ) *
                                                    nPPTX)) - 1;

                                nOutHeight = (long) (pDoc->GetRowHeight( nOverY, nTab ) * nPPTY);

                                bMergeOver = TRUE;
                                nCellX = nOverX;
                                nCellY = nOverY;
                            }
                            else                            // Edit-Zelle
                            {
                                if (nOverY == nY || bVisChanged)
                                    pThisRowInfo->pCellInfo[nOverX+1].bEditEngine = TRUE;
                                else
                                    pRowInfo[0].pCellInfo[nOverX+1].bEditEngine = TRUE;
                            }
                        }
                    }
                    bMergeEmpty = TRUE;
                }

                //
                //  Rest von langem Text weiter links ?
                //

                //  nJustPosX ist incl. Ausrichtung, nVirtPosX fuer Clipping ohne Ausrichtung
                long nJustPosX = nVirtPosX;
                BOOL bRightAdjusted = FALSE;        // to correct text width calculation later
                if ( bEmpty && !bMergeEmpty )
                {
                    if (nX==nX1)                    // links Zellen zum Weiterzeichnen ?
                    {
                        nOutWidth = ((long)pRowInfo[0].pCellInfo[nX+1].nWidth) - 1;
                        USHORT nTempX=nX;
                        while (nTempX > 0 && IsEmptyCellText( pThisRowInfo, nTempX, nY ))
                        {
                            --nTempX;
                            nVirtPosX -= pRowInfo[0].pCellInfo[nTempX+1].nWidth;
                            nOutWidth += pRowInfo[0].pCellInfo[nTempX+1].nWidth;
                        }
                        nJustPosX = nVirtPosX;
                        if ( !IsEmptyCellText( pThisRowInfo, nTempX, nY ) )
                        {
                            ScBaseCell* pCell = pThisRowInfo->pCellInfo[nTempX+1].pCell;
                            CellType eCellType = pCell->GetCellType();
                            if ( eCellType == CELLTYPE_STRING ||
                                (eCellType == CELLTYPE_FORMULA
                                    && !lcl_IsValueDataAtPos( bProgress, pDoc,
                                        nTempX, nY, nTab )) )
                            {
                                const ScPatternAttr* pPattern =
                                        pDoc->GetPattern( nTempX, nY, nTab );
                                const SfxItemSet* pCondSet =
                                        pDoc->GetCondResult( nTempX, nY, nTab );
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

                                if ( aVars.GetOrient() == SVX_ORIENTATION_STANDARD &&
                                        !aVars.GetLineBreak() && !aVars.IsRotated() &&
                                        aVars.GetHorJust() != SVX_HOR_JUSTIFY_BLOCK )
                                {
                                    if ( IsAmbiguousScript(nScript) )
                                    {
                                        // need edit engine
                                        if ( !pDoc->HasAttrib( nTempX,nY,nTab, nX,nY,nTab,
                                                            HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                                            pThisRowInfo->pCellInfo[nTempX+1].bEditEngine = TRUE;
                                    }
                                    else
                                    {
                                        if ( eCellType == CELLTYPE_FORMULA )
                                            lcl_CreateInterpretProgress( bProgress,
                                                pDoc, (ScFormulaCell*)pCell );
                                        if ( aVars.SetText(pCell) )
                                            pOldPattern = NULL;

                                        nJustPosX += (long) ( aVars.GetLeftTotal() * nPPTX );

                                        if ( nJustPosX + aVars.GetTextSize().Width() + 1 > nPosX &&
                                                !pDoc->HasAttrib( nTempX,nY,nTab, nX,nY,nTab,
                                                        HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                                        {
                                            bLeftOver = TRUE;
                                        }

                                        nCellX = nTempX;
                                    }
                                }
                            }
                            else if (eCellType == CELLTYPE_EDIT)
                            {
                                if ( !pDoc->HasAttrib( nTempX,nY,nTab, nX,nY,nTab,
                                                    HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                                    pThisRowInfo->pCellInfo[nTempX+1].bEditEngine = TRUE;
                            }
                        }
                    }
                }

                if (bEditMode)
                    if ( nX == nEditCol && nY == nEditRow )
                        bLeftOver = FALSE;

                if (!bEmpty)
                    if (pInfo->pCell->GetCellType() == CELLTYPE_EDIT)
                    {
                        pInfo->bEditEngine = TRUE;
                        bEmpty = TRUE;                          // Edit-Zelle hier nicht zeichnen
                    }

                //  zu kleinen Text in der Druckvorschau als Pixel zeichnen:

                if ( !bEmpty && eType == OUTTYPE_PRINTER &&
                    pDev->GetOutDevType() == OUTDEV_WINDOW &&
                    ((const SvxFontHeightItem&)pInfo->pPatternAttr->
                    GetItem(ATTR_FONT_HEIGHT)).GetHeight() <= nMinHeight )
                {
                    Point aPos(nPosX,nPosY);
                    Color aFontColor = ((const SvxColorItem&)pInfo->pPatternAttr->
                                            GetItem( ATTR_FONT_COLOR )).GetValue();
                    if ( ( aFontColor == COL_AUTO || bForceAutoColor ) && bUseStyleColor )
                        aFontColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );
                    pDev->DrawPixel( aPos, aFontColor );
                    bEmpty = TRUE;
                }

                if (!bEmpty)                                // bei MergeOver schon geschehen
                {
                    const ScPatternAttr* pPattern = pInfo->pPatternAttr;
                    const SfxItemSet* pCondSet = pInfo->pConditionSet;
                    BYTE nScript = GetScriptType( pDoc, pInfo->pCell, pPattern, pCondSet );
                    if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();
                    if ( pPattern != pOldPattern || pCondSet != pOldCondSet ||
                         nScript != nOldScript || bSyntaxMode )
                    {
                        if ( StringDiffer(pOldPattern,pPattern) ||
                             pCondSet != pOldCondSet || nScript != nOldScript || bSyntaxMode )
                            aVars.SetPattern( pPattern, pCondSet, pInfo->pCell, nScript );
                        else
                            aVars.SetPatternSimple( pPattern, pCondSet );
                        pOldPattern = pPattern;
                        pOldCondSet = pCondSet;
                        nOldScript = nScript;
                    }

                    if ( aVars.GetOrient() == SVX_ORIENTATION_STACKED ||
                         aVars.IsRotated() || IsAmbiguousScript(nScript) )
                    {
                        pInfo->bEditEngine = TRUE;
                        bEmpty = TRUE;
                    }
                }

                if ( !bEmpty || bLeftOver || bMergeOver )
                {
                    CellType            eCellType;
                    SvxCellHorJustify   eOutHorJust;

                    BOOL bMergeHClip = FALSE;           // Teil von zusammengefassten ausserhalb?
                    BOOL bMergeVClip = FALSE;

                    if ( !bEmpty || bMergeOver )
                    {
                        if (!bEmpty)
                        {
                            if ( pInfo->pCell->GetCellType() == CELLTYPE_FORMULA )
                                lcl_CreateInterpretProgress( bProgress,
                                    pDoc, (ScFormulaCell*)pInfo->pCell );
                            if ( aVars.SetText(pInfo->pCell) )
                                pOldPattern = NULL;

                            nOutWidth = ((long)pRowInfo[0].pCellInfo[nX+1].nWidth)-1;
                        }

                        BOOL bMerged = bMergeOver ? TRUE : pInfo->bMerged;

                        if ( bMerged )                      // Zusammengefasst
                        {
                            if (!bMergeOver)
                            {
                                nOverX = nX;
                                nOverY = nY;
                            }
                            USHORT i;
                            const ScMergeAttr* pMerge =
                                    (ScMergeAttr*)&aVars.GetPattern()->GetItem(ATTR_MERGE);
                            USHORT nCountX = pMerge->GetColMerge();
                            for (i=1; i<nCountX; i++)
                                nOutWidth += (long) ( pDoc->GetColWidth(nOverX+i,nTab) * nPPTX );
                            USHORT nCountY = pMerge->GetRowMerge();
                            for (i=1; i<nCountY; i++)
                                nOutHeight += (long) ( pDoc->GetRowHeight(nOverY+i,nTab) * nPPTY );

                            if (nOverX+nCountX > nX2+1)
                                bMergeHClip = TRUE;
                            if (nOverY+nCountY > nY2+1)
                                bMergeVClip = TRUE;
                        }

                        if (bEditMode)
                            if ( nX == nEditCol && nY == nEditRow )
                                aVars.ResetText();

                        // GetTextSize ist schon bei SetText passiert

                        if (bMergeOver)
                        {
                            ScBaseCell* pBCell;
                            GetVisibleCell( nOverX, nOverY, nTab, pBCell );
                            eCellType = (pBCell ? pBCell->GetCellType() : CELLTYPE_NONE);
                            if ( aVars.GetHorJust() != SVX_HOR_JUSTIFY_STANDARD )
                                eOutHorJust = aVars.GetHorJust();
                            else
                                switch( eCellType )
                                {
                                    case CELLTYPE_STRING:
                                        eOutHorJust = SVX_HOR_JUSTIFY_LEFT;
                                        break;
                                    case CELLTYPE_VALUE:
                                        eOutHorJust = ( eForceText == ftForce )
                                                    ? SVX_HOR_JUSTIFY_LEFT
                                                    : SVX_HOR_JUSTIFY_RIGHT;
                                        break;
                                    case CELLTYPE_FORMULA:
                                    {
                                        ScFormulaCell* pFCell = (ScFormulaCell*)pBCell;
                                        if ( pFCell->IsRunning() )
                                            eOutHorJust = SVX_HOR_JUSTIFY_RIGHT;
                                        else
                                        {
                                            lcl_CreateInterpretProgress( bProgress, pDoc, pFCell );
                                            if ( pFCell->HasValueData() )
                                                eOutHorJust = SVX_HOR_JUSTIFY_RIGHT;
                                            else
                                                eOutHorJust = SVX_HOR_JUSTIFY_LEFT;
                                        }
                                    }
                                        break;
                                    default:
                                        eOutHorJust = SVX_HOR_JUSTIFY_CENTER;       // unbekannt
                                        break;
                                }
                        }
                        else
                        {
                            eCellType = pInfo->pCell->GetCellType();
                            if ( aVars.GetHorJust() != SVX_HOR_JUSTIFY_STANDARD )
                                eOutHorJust = aVars.GetHorJust();
                            else
                                switch( eCellType )
                                {
                                    case CELLTYPE_STRING:
                                        eOutHorJust = SVX_HOR_JUSTIFY_LEFT;
                                        break;
                                    case CELLTYPE_VALUE:
                                        eOutHorJust = ( eForceText == ftForce )
                                                    ? SVX_HOR_JUSTIFY_LEFT
                                                    : SVX_HOR_JUSTIFY_RIGHT;
                                        break;
                                    case CELLTYPE_FORMULA:
                                    {
                                        ScFormulaCell* pFCell = (ScFormulaCell*)pInfo->pCell;
                                        if ( pFCell->IsRunning() )
                                            eOutHorJust = SVX_HOR_JUSTIFY_RIGHT;
                                        else
                                        {
                                            lcl_CreateInterpretProgress( bProgress, pDoc, pFCell );
                                            if ( pFCell->IsValue() )
                                                eOutHorJust = SVX_HOR_JUSTIFY_RIGHT;
                                            else
                                                eOutHorJust = SVX_HOR_JUSTIFY_LEFT;
                                        }
                                    }
                                        break;
                                    default:
                                        eOutHorJust = SVX_HOR_JUSTIFY_CENTER;       // unbekannt
                                        break;
                                }
                        }
                    }
                    else                                            // Leftover (langer String)
                    {
                        eCellType   = CELLTYPE_STRING;
                        eOutHorJust = SVX_HOR_JUSTIFY_LEFT;
                    }

                    if ( eOutHorJust == SVX_HOR_JUSTIFY_BLOCK || eOutHorJust == SVX_HOR_JUSTIFY_REPEAT )
                        eOutHorJust = SVX_HOR_JUSTIFY_LEFT;     // noch nicht implementiert


                                                    // Clipping testen
                    BOOL bHClip = FALSE;
                    BOOL bVClip = FALSE;
//                  BOOL bExtended = FALSE;
                    CellInfo* pClipRight = NULL;    // Zelle mit Clipping-Markierung (rechts)

                    long nJustOutWidth = nOutWidth;         // fuer Ausrichtung

                    long nNeededWidth;
                    if (eType == OUTTYPE_WINDOW)
                        nNeededWidth = aVars.GetTextSize().Width() +
                                        (long) ( aVars.GetLeftTotal() * nPPTX ) +
                                        (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                    else
                    {
                        nNeededWidth = aVars.GetTextSize().Width();         // in Margin zeichnen erlaubt
                        switch (eOutHorJust)
                        {
                            case SVX_HOR_JUSTIFY_LEFT:
                                nNeededWidth += (long) ( aVars.GetLeftTotal() * nPPTX );
                                break;
                            case SVX_HOR_JUSTIFY_RIGHT:
                                nNeededWidth += (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                                break;
                            case SVX_HOR_JUSTIFY_CENTER:
                                break;
                            default:
                                nNeededWidth += (long) ( aVars.GetLeftTotal() * nPPTX ) +
                                                (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                        }
                    }

                    //
                    //      horizontalen Platz testen
                    //

                    BOOL bNeedEditEngine = FALSE;
                    if ( aVars.GetLineBreak() ||
                         aVars.GetHorJust() == SVX_HOR_JUSTIFY_BLOCK )
                    {
                                                        //! hier immer Margin beruecksichtigen ???

                        BOOL bBig;

                        //if (aVars.GetString().Search((sal_Unicode)10) != STRING_NOTFOUND)
                        //  bBig = TRUE;
                        //else ...

                        if (aVars.GetOrient() == SVX_ORIENTATION_STANDARD)
                        {
                            long nCmpWidth = nOutWidth;     // auch hier AutoFilter abziehen
                            if ( eType==OUTTYPE_WINDOW && pInfo && pInfo->bAutoFilter )
                                nCmpWidth -= Min( pThisRowInfo->nHeight,
                                                    (USHORT) DROPDOWN_BITMAP_SIZE );
                            bBig = (nNeededWidth+1 > nCmpWidth);
                        }
                        else
                            bBig = (aVars.GetTextSize().Height() > nOutHeight);

                        if (bBig)
                        {
                            if (bMergeOver)
                            {
                                // BOOL bVisChanged = !pRowInfo[nArrY-1].bChanged
                                if (nOverY == nY || !pRowInfo[nArrY-1].bChanged )
                                    pThisRowInfo->pCellInfo[nOverX+1].bEditEngine = TRUE;
                                else
                                    pRowInfo[0].pCellInfo[nOverX+1].bEditEngine = TRUE;
                            }
                            else
                                pInfo->bEditEngine = TRUE;
                            bNeedEditEngine = TRUE;
                        }
                    }

                    if (!bNeedEditEngine)
                    {
                        if ( eCellType == CELLTYPE_STRING ||
                            (eCellType == CELLTYPE_FORMULA
                                && !lcl_IsValueDataAtPos( bProgress, pDoc,
                                    nX, nY, nTab )) )
                        {                                                       // rechts Platz suchen
                            if (nNeededWidth+1 > nOutWidth )
                            {
                                if ( bMergeOver )
                                {
                                    //  zusammengefasste zu klein - nicht weitersuchen
                                    eOutHorJust = SVX_HOR_JUSTIFY_LEFT;
                                    bHClip = TRUE;
                                    //  The flag for clip marks can be set for any cell
                                    //  in the merged area (handled by DrawClipMarks)
                                    pClipRight = &pThisRowInfo->pCellInfo[nX+1];
                                }
                                else
                                {
                                    if ( aVars.GetOrient() == SVX_ORIENTATION_STANDARD )
                                    {
                                        if ( !pThisRowInfo->pCellInfo[nX+1].bMerged )
                                        {
                                            USHORT nTempX = nX+1;
                                            BOOL bCont = TRUE;
                                            while (bCont)
                                            {
                                                BOOL bMayCont = pThisRowInfo->pCellInfo[nTempX+1].bEmptyCellText &&
                                                               !pThisRowInfo->pCellInfo[nTempX+1].bMerged &&
                                                               !pThisRowInfo->pCellInfo[nTempX+1].bHOverlapped &&
                                                               !pThisRowInfo->pCellInfo[nTempX+1].bVOverlapped;
                                                bCont = bMayCont && (nTempX<=nX2);
                                                if ( bEditMode && nTempX == nEditCol && nY == nEditRow )
                                                    bCont = FALSE;

                                                //  Grid (fuer nTempX-1) abschalten ?
                                                //
                                                if (nOutWidth < nNeededWidth)
                                                {
                                                    if (bMayCont)
                                                        pThisRowInfo->pCellInfo[nTempX].bHideGrid = TRUE;
                                                    else
                                                        pClipRight = &pThisRowInfo->pCellInfo[nTempX];
                                                        // auf "Hindernis" gestossen
                                                }

                                                if (bCont)
                                                    nOutWidth += (long) pRowInfo[0].pCellInfo[nTempX+1].nWidth;
                                                ++nTempX;
                                            }
                                        }
                                        else
                                        {
                                            //  The flag for clip marks can be set for any cell
                                            //  in the merged area (handled by DrawClipMarks)
                                            if ( nOutWidth < nNeededWidth )
                                                pClipRight = &pThisRowInfo->pCellInfo[nX+1];
                                        }
                                    }

//                                  bExtended = TRUE;
                                    eOutHorJust = SVX_HOR_JUSTIFY_LEFT;
                                    bHClip = bLeftOver || (nNeededWidth + 1 > nOutWidth);
                                }
                            }
                            else
                            {
                                bHClip = bLeftOver || bMergeOver;
                            }
                        }
                        else                                // Zahlen, Formeln
                        {
                            if (nNeededWidth + 1 > nOutWidth)
                            {
                                aVars.SetHashText();
                                nNeededWidth = aVars.GetTextSize().Width() +
                                            (long) ( aVars.GetLeftTotal() * nPPTX ) +
                                            (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                                bHClip = (nNeededWidth + 1 > nOutWidth);
                                if (bHClip)     // #56609# Clip-Markierung auch fuer Zahlen
                                    pClipRight = &pThisRowInfo->pCellInfo[nX+1];
                            }
                            if (bMergeOver)
                                bHClip = TRUE;
                        }

                        if ( !bLeftOver )                           // sonst ist schon
                        {
                            long nAvailWidth = nJustOutWidth;
                            if (eType==OUTTYPE_WINDOW && pInfo->bAutoFilter)
                            {
#ifndef MAC
                                if (pRowInfo[nArrY].nHeight < DROPDOWN_BITMAP_SIZE)
                                    nAvailWidth -= pRowInfo[nArrY].nHeight;
                                else
                                    nAvailWidth -= DROPDOWN_BITMAP_SIZE;        // Bitmap-Groesse
#else
                                nAvailWidth -= pRefDevice->LogicToPixel( Size(22,0), MAP_POINT ).Width();
#endif

                                long nComp = aVars.GetTextSize().Width() +
                                                (long) ( aVars.GetLeftTotal() * nPPTX ) +
                                                (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                                if (nAvailWidth<nComp) nAvailWidth=nComp;
                            }

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
                        }

                        long nTestClipHeight = aVars.GetTextSize().Height();

                        long nJustPosY = nVirtPosY;
//                      long nLongBot = (long) ( aVars.GetMargin()->GetBottomMargin() * nPPTY );
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
                                    ( pDoc->GetRowFlags( nY, nTab ) & CR_MANUALSIZE ) ||
                                    ( aVars.HasCondHeight() ) )
                                bVClip = TRUE;
                        }
                        if ( bMergeOver )
                            bVClip = TRUE;

                        Rectangle aClipRect( nPosX, nPosY, nVirtPosX+nOutWidth-1,
                                                            nVirtPosY+nOutHeight-1 );

                        if ( bMergeHClip )
                        {
                            //  don't limit to end of screen if clip mark is subtracted below
                            if ( aClipRect.Right() > nScrX+nScrW && !( pClipRight && bMarkClipped ) )
                                aClipRect.Right() = nScrX+nScrW;
                            bHClip = TRUE;
                        }
                        if ( bMergeVClip )
                        {
                            if ( aClipRect.Bottom() > nScrY+nScrH )
                                aClipRect.Bottom() = nScrY+nScrH;
                            bVClip = TRUE;
                        }

                        if ( bHClip || bVClip )
                        {
                            if ( pClipRight && bMarkClipped )
                            {
                                //  rechts markieren

                                pClipRight->bStandard = TRUE;   //! umbenennen in bClipped
                                bAnyClipped = TRUE;

                                long nMarkPixel = (long)( SC_CLIPMARK_SIZE * nPPTX );
                                aClipRect.Right() -= nMarkPixel;
                            }

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

/*                          //! Test
                            if ( pDev->GetOutDevType() != OUTDEV_PRINTER )
                            {
                                pDev->SetPen(PEN_NULL);
                                pDev->SetFillInBrush(Brush(Color(COL_LIGHTRED)));
                                pDev->DrawRect( aClipRect );
                            }
                            //! Test
*/

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

#ifdef MAC_DOCH_NICHT
                        nJustPosY++;    // Passt sonst nicht zur EditEngine
#endif
#ifdef OS2_AUCH_NICHT
                        nJustPosY++;    // Passt sonst nicht zur EditEngine
#endif

                        BOOL bMoveClipped = bHClip && aVars.IsRightToLeftAttr();
                        if ( bMoveClipped )
                        {
                            //  #i9731# if long text is clipped, and the cell is formatted as rtl,
                            //  move the start position so the right end is visible

                            long nNeeded = aVars.GetTextSize().Width() +
                                        (long) ( aVars.GetLeftTotal() * nPPTX ) +
                                        (long) ( aVars.GetMargin()->GetRightMargin() * nPPTX );
                            long nAvailable = GetAvailableWidth( nCellX, nCellY, nNeeded );
                            if ( nAvailable < nNeeded )
                                nJustPosX -= nNeeded - nAvailable;      // move the start position
                        }

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
                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth;
            }
        }
        nPosY += pRowInfo[nArrY].nHeight;
    }
    if ( bProgress )
        ScProgress::DeleteInterpretProgress();
    pDoc->DisableIdle( bWasIdleDisabled );
}

//  -------------------------------------------------------------------------------

Size lcl_GetVertPaperSize( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab )
{
    const double nPPTY = HMM_PER_TWIPS;

    const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
    const ScMergeAttr& rMerge = (const ScMergeAttr&)pPattern->GetItem(ATTR_MERGE);

    long nCellY = (long) ( pDoc->GetRowHeight(nRow,nTab) * nPPTY );
    if ( rMerge.GetRowMerge() > 1 )
    {
        USHORT nCountY = rMerge.GetRowMerge();
        for (USHORT i=1; i<nCountY; i++)
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
    const ScPatternAttr* pPattern;
    const SfxItemSet*    pCondSet;
    const ScPatternAttr* pOldPattern = NULL;
    const SfxItemSet*    pOldCondSet = NULL;
    ScBaseCell* pCell = NULL;

    long nRowPosY = nScrY;
    for (USHORT nArrY=0; nArrY+1<nArrCount; nArrY++)            // 0 fuer Reste von zusammengefassten
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nCellHeight = (long) pThisRowInfo->nHeight;
        if (nArrY==1) nRowPosY = nScrY;                         // vorher wird einzeln berechnet

        if ( pThisRowInfo->bChanged || nArrY==0 )
        {
            long nPosX = 0;
            for (USHORT nX=0; nX<=nX2; nX++)                    // wegen Ueberhaengen
            {
                if (nX==nX1) nPosX = nScrX;                     // vorher wird einzeln berechnet

                CellInfo*   pInfo = &pThisRowInfo->pCellInfo[nX+1];
                if (pInfo->bEditEngine)
                {
                    USHORT nY = pThisRowInfo->nRowNo;

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

                        if ( nArrY==0 )
                        {
                            nPosY = nScrY;
                            nY = pRowInfo[1].nRowNo;
                            pPattern = pDoc->GetPattern( nX, nY, nTab );
                            while (((ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).IsVerOverlapped())
                            {
                                --nY;
                                nPosY -= (long) (pDoc->FastGetRowHeight(nY,nTab) * nPPTY);
                                pPattern = pDoc->GetPattern( nX, nY, nTab );
                            }
                            pInfo = NULL;
                            pCondSet = pDoc->GetCondResult( nX, nY, nTab );
                            GetVisibleCell( nX, nY, nTab, pCell );
                        }
                        else
                        {
                            BOOL bFromDoc = FALSE;
                            pPattern = pInfo->pPatternAttr;
                            pCondSet = pInfo->pConditionSet;
                            if (!pPattern)
                            {
                                pPattern = pDoc->GetPattern( nX, nY, nTab );
                                bFromDoc = TRUE;
                            }
                            pCell = pInfo->pCell;
                            if ( bFromDoc && !pCell )
                                GetVisibleCell( nX, nY, nTab, pCell );

                            bVisChanged = !pRowInfo[nArrY-1].bChanged;
                            if (bVisChanged)
                            {
                                while (((ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).IsVerOverlapped())
                                {
                                    --nY;
                                    nPosY -= (long) (pDoc->FastGetRowHeight(nY,nTab) * nPPTY);
                                    pPattern = pDoc->GetPattern( nX, nY, nTab );
                                    bFromDoc = TRUE;
                                }
                                pInfo = NULL;
                                GetVisibleCell( nX, nY, nTab, pCell );
                            }
                            if (bFromDoc)
                                pCondSet = pDoc->GetCondResult( nX, nY, nTab );
                        }

                        BOOL bIsValue = lcl_SafeIsValue(pCell);

                        long nCellWidth = (long) pRowInfo[0].pCellInfo[nX+1].nWidth;

                        const SfxPoolItem* pCondItem;

                        SvxCellHorJustify eHorJust;
                        if ( pCondSet && pCondSet->GetItemState(
                                    ATTR_HOR_JUSTIFY, TRUE, &pCondItem ) == SFX_ITEM_SET )
                            eHorJust = (SvxCellHorJustify) ((const SvxHorJustifyItem*)
                                                                pCondItem)->GetValue();
                        else
                            eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
                                                    GetItem(ATTR_HOR_JUSTIFY)).GetValue();

                        BOOL bBreak;
                        if ( eHorJust == SVX_HOR_JUSTIFY_BLOCK )
                            bBreak = TRUE;
                        else if ( pCondSet && pCondSet->GetItemState(
                                    ATTR_LINEBREAK, TRUE, &pCondItem ) == SFX_ITEM_SET )
                            bBreak = ((const SfxBoolItem*)pCondItem)->GetValue();
                        else
                            bBreak = ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue();

                        SvxCellOrientation eOrient;
                        if ( pCondSet && pCondSet->GetItemState(
                                    ATTR_ORIENTATION, TRUE, &pCondItem ) == SFX_ITEM_SET )
                            eOrient = (SvxCellOrientation)((const SvxOrientationItem*)
                                            pCondItem)->GetValue();
                        else
                            eOrient = (SvxCellOrientation)((const SvxOrientationItem&)
                                            pPattern->GetItem(ATTR_ORIENTATION)).GetValue();

                        long nAttrRotate;
                        if ( pCondSet && pCondSet->GetItemState(
                                    ATTR_ROTATE_VALUE, TRUE, &pCondItem ) == SFX_ITEM_SET )
                            nAttrRotate = ((const SfxInt32Item*)pCondItem)->GetValue();
                        else
                            nAttrRotate = ((const SfxInt32Item&)pPattern->
                                            GetItem(ATTR_ROTATE_VALUE)).GetValue();
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

                        const ScMergeAttr* pMerge =
                                (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                        BOOL bMerged = pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1;

                        long nStartX = nPosX;
                        long nStartY = nPosY;
                        if (nX<nX1)
                        {
                            if ((bBreak || eOrient!=SVX_ORIENTATION_STANDARD) && !bMerged && !bAsianVertical)
                                bHidden = TRUE;
                            else
                            {
                                nStartX = nScrX;
                                USHORT nCol = nX1;
                                while (nCol > nX)
                                {
                                    --nCol;
                                    nStartX -= (long) pRowInfo[0].pCellInfo[nCol+1].nWidth;
                                }
                            }
                        }

                        if ( !bHidden && eType == OUTTYPE_PRINTER &&
                            pDev->GetOutDevType() == OUTDEV_WINDOW &&
                            ((const SvxFontHeightItem&)pPattern->
                            GetItem(ATTR_FONT_HEIGHT)).GetHeight() <= nMinHeight )
                        {
                            Point aPos( nStartX, nStartY );
                            pDev->DrawPixel( aPos,
                                            ((const SvxColorItem&)pPattern->
                                            GetItem( ATTR_FONT_COLOR )).GetValue() );
                            bHidden = TRUE;
                        }

                        if (!bHidden)
                        {
                            BOOL bMergeClip = FALSE;

                            long nOutWidth = nCellWidth - 1;
                            long nOutHeight;
                            if (pInfo)
                                nOutHeight = nCellHeight;
                            else
                                nOutHeight = (long) ( pDoc->GetRowHeight(nY,nTab) * nPPTY );

                            if ( bMerged )                              // Zusammengefasst
                            {
                                USHORT i;
                                const ScMergeAttr* pMerge =
                                        (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                                USHORT nCountX = pMerge->GetColMerge();
                                for (i=1; i<nCountX; i++)
                                    nOutWidth += (long) ( pDoc->GetColWidth(nX+i,nTab) * nPPTX );
                                USHORT nCountY = pMerge->GetRowMerge();
                                for (i=1; i<nCountY; i++)
                                    nOutHeight += (long) ( pDoc->GetRowHeight(nY+i,nTab) * nPPTY );

                                if ( nX+nCountX > nX2+1 || nY+nCountY > nY2+1 )
                                    bMergeClip = TRUE;
                            }

                            //  aCellRect: Margins nicht abgezogen
                            Rectangle aCellRect( nStartX, nStartY,
                                            nStartX+nOutWidth-1, nStartY+nOutHeight-1 );

                            SvxCellVerJustify eVerJust;
                            if ( pCondSet && pCondSet->GetItemState(
                                        ATTR_VER_JUSTIFY, TRUE, &pCondItem ) == SFX_ITEM_SET )
                                eVerJust = (SvxCellVerJustify) ((const SvxVerJustifyItem*)
                                                                    pCondItem)->GetValue();
                            else
                                eVerJust = (SvxCellVerJustify)((const SvxVerJustifyItem&)pPattern->
                                                        GetItem(ATTR_VER_JUSTIFY)).GetValue();

                            // default alignment for asian vertical mode is top-right
                            if ( bAsianVertical && eVerJust == SVX_VER_JUSTIFY_STANDARD )
                                eVerJust = SVX_VER_JUSTIFY_TOP;

                            // Syntax-Modus wird hier ignoriert...

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
                                            eSvxAdjust = bIsValue ? SVX_ADJUST_RIGHT : SVX_ADJUST_LEFT;
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
                            const SvxMarginItem* pMargin;
                            if ( pCondSet && pCondSet->GetItemState(
                                        ATTR_MARGIN, TRUE, &pCondItem ) == SFX_ITEM_SET )
                                pMargin = (const SvxMarginItem*)pCondItem;
                            else
                                pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);
                            USHORT nIndent = 0;
                            if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
                            {
                                if ( pCondSet && pCondSet->GetItemState(
                                            ATTR_INDENT, TRUE, &pCondItem ) == SFX_ITEM_SET )
                                    nIndent = ((const SfxUInt16Item*)pCondItem)->GetValue();
                                else
                                    nIndent = ((const SfxUInt16Item&)pPattern->
                                                    GetItem(ATTR_INDENT)).GetValue();
                            }
                            long nLeftM = (long) ( (pMargin->GetLeftMargin() + nIndent) * nPPTX );
                            long nTopM  = (long) ( pMargin->GetTopMargin() * nPPTY );
                            nStartX += nLeftM;
                            nStartY += nTopM;
                            nOutWidth -= nLeftM + (long) ( pMargin->GetRightMargin() * nPPTX );
                            nOutHeight -= nTopM + (long) ( pMargin->GetBottomMargin() * nPPTY );

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

                            if (nX<nX1 && !bMerged)
                            {
                                long nEnginePixel = bPixelToLogic ?
                                        pRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width() :
                                        nEngineWidth;
                                if ( nStartX + nEnginePixel < (long) nScrX )
                                    bHidden = TRUE;
                            }

                            if (!bHidden)
                            {
                                BOOL bExtend = FALSE;           // ueber Zellenrand geschrieben ?
                                BOOL bClip = bMergeClip;        // clip to draw only part of merged cell
                                CellInfo* pClipRight = NULL;    // Zelle mit Clipping-Markierung
                                BOOL bSimClip = FALSE;
                                Size aClipSize = Size( nScrX+nScrW-nStartX, nScrY+nScrH-nStartY );

                                if ( bWrapFields )
                                {
                                    //  Fields in a cell with automatic breaks: clip to cell width
                                    bClip = TRUE;
                                    aClipSize.Width() = nOutWidth;
                                }

                                //  Zelle, an der Clipping-Markierung gesetzt wird, wenn
                                //  vertikal zu klein
                                CellInfo* pEndInfo = &pThisRowInfo->pCellInfo[nX+1];

                                //  weiterschreiben

                                Size aCellSize;
                                if (bPixelToLogic)
                                    aCellSize = pRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
                                else
                                    aCellSize = Size( nOutWidth, nOutHeight );
                                if ( bIsValue )
                                {
                                    if ( nEngineWidth > aCellSize.Width() )
                                    {
                                        pEngine->SetText( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("###")) );
                                        //if ( bSyntaxMode )
                                        //  SetEditSyntaxColor( *pEngine, pCell );
                                        nEngineWidth = (long) pEngine->CalcTextWidth();

                                        if ( nEngineWidth > aCellSize.Width() )     // still too big?
                                        {
                                            // handle clipping like for text (may also be a merged cell)
                                            bExtend = TRUE;
                                            bClip = TRUE;
                                            aClipSize.Width() = nOutWidth;
                                            USHORT nClipX = ( nX < nX1 ) ? nX1 : nX;
                                            pClipRight = &pRowInfo[nArrY ? nArrY : 1].pCellInfo[nClipX+1];
                                        }
                                    }
                                }
                                else if ( !bBreak || eOrient!=SVX_ORIENTATION_STANDARD || bAsianVertical )
                                {
                                    if ( eOrient == SVX_ORIENTATION_STANDARD && !bMerged )
                                    {
                                        USHORT nNextX = nX + 1;
                                        BOOL bCont = TRUE;
                                        while (bCont)
                                        {
                                            BOOL bMayCont =
                                                    pThisRowInfo->pCellInfo[nNextX+1].bEmptyCellText &&
                                                    !pThisRowInfo->pCellInfo[nNextX+1].bMerged &&
                                                    !pThisRowInfo->pCellInfo[nNextX+1].bHOverlapped &&
                                                    !pThisRowInfo->pCellInfo[nNextX+1].bVOverlapped;
                                            bCont = bMayCont && (nNextX<=nX2);
                                            if ( bEditMode && nNextX==nEditCol && nY==nEditRow )
                                                bCont = FALSE;

                                            //  Grid (fuer nNextX-1) abschalten
                                            //
                                            if ( nEngineWidth > aCellSize.Width() )
                                            {
                                                if (bMayCont)
                                                {
                                                    pThisRowInfo->pCellInfo[nNextX].bHideGrid = TRUE;
                                                    pEndInfo = &pThisRowInfo->pCellInfo[nNextX+1];
                                                }
                                                else
                                                    pClipRight = &pThisRowInfo->pCellInfo[nNextX];
                                                    // auf "Hindernis" gestossen
                                            }
                                            else
                                                bCont = FALSE;  // #55142# bExtend nicht zu oft

                                            if (bCont)
                                            {
                                                long nAdd = pRowInfo[0].pCellInfo[nNextX+1].nWidth;
                                                aCellRect.Right() += nAdd;
                                                nOutWidth += nAdd;
                                                if (bPixelToLogic)
                                                    aCellSize = pRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
                                                else
                                                    aCellSize = Size( nOutWidth, nOutHeight );
                                                bExtend = TRUE;
                                            }
                                            ++nNextX;
                                        }
                                    }
                                    if ( nEngineWidth > aCellSize.Width() )
                                    {
                                        bExtend = TRUE;     // -> immer linksbuendig
                                        bClip = TRUE;
                                        aClipSize.Width() = nOutWidth;      // Pixel

                                        if ( bMerged )      // otherwise it's handled above
                                        {
                                            //  anywhere in the merged area...
                                            USHORT nClipX = ( nX < nX1 ) ? nX1 : nX;
                                            pClipRight = &pRowInfo[nArrY ? nArrY : 1].pCellInfo[nClipX+1];
                                        }
                                    }
                                }
                                if ( nEngineHeight > aCellSize.Height() )
                                {
                                    //  kein vertikales Clipping beim Drucken von Zellen mit
                                    //  optimaler Hoehe, ausser bei Groesse in bedingter
                                    //  Formatierung
                                    if ( eType != OUTTYPE_PRINTER ||
                                        ( pDoc->GetRowFlags( nY, nTab ) & CR_MANUALSIZE ) ||
                                        ( pCondSet && SFX_ITEM_SET ==
                                            pCondSet->GetItemState(ATTR_FONT_HEIGHT, TRUE) ) )
                                        bClip = TRUE;
                                    else
                                        bSimClip = TRUE;
                                    aClipSize.Height() = nOutHeight;        // Pixel

                                    //  Show clip marks if height is at least 5pt too small and
                                    //  there are several lines of text.
                                    //  Not for asian vertical text, because that would interfere
                                    //  with the default right position of the text.
                                    if ( nEngineHeight - aCellSize.Height() > 100 && !bAsianVertical &&
                                         ( pEngine->GetParagraphCount() > 1 ||
                                           pEngine->GetLineCount(0) > 1 ) )
                                    {
                                        if ( bMerged )
                                        {
                                            //  anywhere in the merged area...
                                            USHORT nClipX = ( nX < nX1 ) ? nX1 : nX;
                                            pClipRight = &pRowInfo[nArrY ? nArrY : 1].pCellInfo[nClipX+1];
                                        }
                                        else
                                            pClipRight = pEndInfo;
                                    }
                                }

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

                                Rectangle aClipRect;
                                if (bClip || bSimClip)
                                {
                                    if ( pClipRight && bMarkClipped )
                                    {
                                        //  rechts markieren

                                        pClipRight->bStandard = TRUE;   //! umbenennen in bClipped
                                        bAnyClipped = TRUE;

                                        long nMarkPixel = (long)( SC_CLIPMARK_SIZE * nPPTX );
                                        long nMarkLeft = aCellRect.Right()-nMarkPixel+1;
                                        long nClipEnd = nClipStartX + aClipSize.Width();
                                        if (nClipEnd > nMarkLeft)
                                            aClipSize.Width() -= nClipEnd - nMarkLeft;
                                    }

                                    if (bPixelToLogic)
                                        aClipRect = pRefDevice->PixelToLogic( Rectangle(
                                                        Point(nClipStartX,nClipStartY), aClipSize ) );
                                    else
                                        aClipRect = Rectangle(Point(nClipStartX, nClipStartY), aClipSize);

#if 0
                                    //! Test
                                    if ( pDev->GetOutDevType() != OUTDEV_PRINTER )
                                    {
                                        pDev->SetPen(PEN_NULL);
                                        if (bClip)
                                            pDev->SetFillInBrush(Brush(Color(COL_LIGHTGREEN)));
                                        else
                                            pDev->SetFillInBrush(Brush(Color(COL_LIGHTRED)));
                                        pDev->DrawRect( aClipRect );
                                    }
                                    //! Test
#endif

                                    if (bClip)  // bei bSimClip nur aClipRect initialisieren
                                    {
                                        if (bMetaFile)
                                        {
                                            pDev->Push();
                                            pDev->IntersectClipRegion( aClipRect );
                                        }
                                        else
                                            pDev->SetClipRegion( Region( aClipRect ) );
                                    }
                                }

                                Point aLogicStart;
                                if (bPixelToLogic)
                                    aLogicStart = pRefDevice->PixelToLogic( Point(nStartX,nStartY) );
                                else
                                    aLogicStart = Point(nStartX, nStartY);
                                if ( (eOrient!=SVX_ORIENTATION_STANDARD || bAsianVertical || !bBreak) && !bExtend )
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
                                            (eHorJust==SVX_HOR_JUSTIFY_STANDARD && bIsValue) )
                                        {
                                            pEngine->SetUpdateMode( FALSE );

                                            SvxAdjust eSvxAdjust =
                                                (eHorJust==SVX_HOR_JUSTIFY_CENTER) ?
                                                    SVX_ADJUST_CENTER : SVX_ADJUST_RIGHT;
                                            pEngine->SetDefaultItem(
                                                SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );

                                            // #55142# Adjustment fuer naechste Zelle wieder zurueck
                                            pOldPattern = NULL;

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
                                        if (eHorJust==SVX_HOR_JUSTIFY_RIGHT)
                                            aLogicStart.X() += nAvailWidth - nEngineWidth;
                                        else if (eHorJust==SVX_HOR_JUSTIFY_CENTER)
                                            aLogicStart.X() += (nAvailWidth - nEngineWidth) / 2;
                                    }
                                }

                                if ( bAsianVertical )
                                {
                                    // paper size is subtracted below (with or without bExtend)
                                    aLogicStart.X() += nEngineWidth;
                                }

                                if ( ( eOrient==SVX_ORIENTATION_STANDARD && !bAsianVertical ) ||
                                     eOrient==SVX_ORIENTATION_STACKED || !bBreak )
                                {
                                    if (eVerJust==SVX_VER_JUSTIFY_BOTTOM ||
                                        eVerJust==SVX_VER_JUSTIFY_STANDARD)
                                    {
                                        // if pRefDevice != pFmtDevice, stick with logic units
                                        if (bPixelToLogic && pRefDevice == pFmtDevice)
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
                                    aLogicStart.Y() += bBreak ? aCellSize.Height() :
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

                                BOOL bMoveClipped = FALSE;
                                if ( bExtend )
                                {
                                    SvxFrameDirection eCellDir = (SvxFrameDirection)((const SvxFrameDirectionItem&)
                                            pPattern->GetItem( ATTR_WRITINGDIR, pCondSet )).GetValue();
                                    bMoveClipped = ( eCellDir == FRMDIR_HORI_RIGHT_TOP ||
                                        ( eCellDir == FRMDIR_ENVIRONMENT && nTabTextDirection == EE_HTEXTDIR_R2L ) );
                                }
                                if ( bMoveClipped )
                                {
                                    //  #i9731# if long text is clipped, and the cell is formatted as rtl,
                                    //  move the start position so the right end is visible

                                    long nNeeded = bPixelToLogic ?
                                            pRefDevice->LogicToPixel(Size(nEngineWidth,0)).Width() :
                                            nEngineWidth;
                                    nNeeded += (long) ( (pMargin->GetLeftMargin() + nIndent) * nPPTX ) +
                                               (long) ( pMargin->GetRightMargin() * nPPTX );

                                    long nAvailable = GetAvailableWidth( nX, nY, nNeeded );
                                    if ( nAvailable < nNeeded )
                                    {
                                        long nDiff = nNeeded - nAvailable;
                                        aLogicStart.X() -= bPixelToLogic ?
                                                pRefDevice->PixelToLogic(Size(nDiff,0)).Width() :
                                                nDiff;
                                    }
                                }

                                if ( bSimClip && !nOriVal && !bAsianVertical )
                                {
                                    //  kein hartes Clipping, aber nur die betroffenen
                                    //  Zeilen ausgeben

                                    Point aDocStart = aClipRect.TopLeft();
                                    aDocStart -= aLogicStart;
                                    pEngine->Draw( pDev, aClipRect, aDocStart, FALSE );
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
                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth;
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
    USHORT nRotMax = nX2;
    for (USHORT nRotY=0; nRotY<nArrCount; nRotY++)
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

    long nRowPosY = nScrY;
    for (USHORT nArrY=0; nArrY+1<nArrCount; nArrY++)            // 0 fuer Reste von zusammengefassten
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nCellHeight = (long) pThisRowInfo->nHeight;
        if (nArrY==1) nRowPosY = nScrY;                         // vorher wird einzeln berechnet

        if ( ( pThisRowInfo->bChanged || nArrY==0 ) && pThisRowInfo->nRotMaxCol != SC_ROTMAX_NONE )
        {
            long nPosX = 0;
            for (USHORT nX=0; nX<=nRotMax; nX++)
            {
                if (nX==nX1) nPosX = nScrX;                     // vorher wird einzeln berechnet

                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                if ( pInfo->nRotateDir != SC_ROTDIR_NONE )
                {
                    USHORT nY = pThisRowInfo->nRowNo;

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

                        const SfxPoolItem* pCondItem;

                        SvxCellHorJustify eHorJust;
                        if ( pCondSet && pCondSet->GetItemState(
                                    ATTR_HOR_JUSTIFY, TRUE, &pCondItem ) == SFX_ITEM_SET )
                            eHorJust = (SvxCellHorJustify) ((const SvxHorJustifyItem*)
                                                                pCondItem)->GetValue();
                        else
                            eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
                                                    GetItem(ATTR_HOR_JUSTIFY)).GetValue();

                        BOOL bBreak;
                        if ( eHorJust == SVX_HOR_JUSTIFY_BLOCK )
                            bBreak = TRUE;
                        else if ( pCondSet && pCondSet->GetItemState(
                                    ATTR_LINEBREAK, TRUE, &pCondItem ) == SFX_ITEM_SET )
                            bBreak = ((const SfxBoolItem*)pCondItem)->GetValue();
                        else
                            bBreak = ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue();

                        SvxCellOrientation eOrient;
                        if ( pCondSet && pCondSet->GetItemState(
                                    ATTR_ORIENTATION, TRUE, &pCondItem ) == SFX_ITEM_SET )
                            eOrient = (SvxCellOrientation)((const SvxOrientationItem*)
                                            pCondItem)->GetValue();
                        else
                            eOrient = (SvxCellOrientation)((const SvxOrientationItem&)
                                            pPattern->GetItem(ATTR_ORIENTATION)).GetValue();

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
                                nStartX = nScrX;
                                USHORT nCol = nX1;
                                while (nCol > nX)
                                {
                                    --nCol;
                                    nStartX -= (long) pRowInfo[0].pCellInfo[nCol+1].nWidth;
                                }
                            }
                        }

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
                                USHORT i;
                                const ScMergeAttr* pMerge =
                                        (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                                USHORT nCountX = pMerge->GetColMerge();
                                for (i=1; i<nCountX; i++)
                                    nOutWidth += (long) ( pDoc->GetColWidth(nX+i,nTab) * nPPTX );
                                USHORT nCountY = pMerge->GetRowMerge();
                                for (i=1; i<nCountY; i++)
                                    nOutHeight += (long) ( pDoc->GetRowHeight(nY+i,nTab) * nPPTY );
                            }

                            SvxCellVerJustify eVerJust;
                            if ( pCondSet && pCondSet->GetItemState(
                                        ATTR_VER_JUSTIFY, TRUE, &pCondItem ) == SFX_ITEM_SET )
                                eVerJust = (SvxCellVerJustify) ((const SvxVerJustifyItem*)
                                                                    pCondItem)->GetValue();
                            else
                                eVerJust = (SvxCellVerJustify)((const SvxVerJustifyItem&)pPattern->
                                                        GetItem(ATTR_VER_JUSTIFY)).GetValue();

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

                            const SvxMarginItem* pMargin;
                            if ( pCondSet && pCondSet->GetItemState(
                                        ATTR_MARGIN, TRUE, &pCondItem ) == SFX_ITEM_SET )
                                pMargin = (const SvxMarginItem*)pCondItem;
                            else
                                pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);
                            USHORT nIndent = 0;
                            if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
                            {
                                if ( pCondSet && pCondSet->GetItemState(
                                            ATTR_INDENT, TRUE, &pCondItem ) == SFX_ITEM_SET )
                                    nIndent = ((const SfxUInt16Item*)pCondItem)->GetValue();
                                else
                                    nIndent = ((const SfxUInt16Item&)pPattern->
                                                    GetItem(ATTR_INDENT)).GetValue();
                            }

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
                                if ( pCondSet && pCondSet->GetItemState(
                                            ATTR_ROTATE_VALUE, TRUE, &pCondItem ) == SFX_ITEM_SET )
                                    nAttrRotate = ((const SfxInt32Item*)pCondItem)->GetValue();
                                else
                                    nAttrRotate = ((const SfxInt32Item&)pPattern->
                                                    GetItem(ATTR_ROTATE_VALUE)).GetValue();
                                if ( nAttrRotate )
                                {
                                    if ( pCondSet && pCondSet->GetItemState(
                                            ATTR_ROTATE_MODE, TRUE, &pCondItem ) == SFX_ITEM_SET )
                                        eRotMode = (SvxRotateMode)((const SvxRotateModeItem*)
                                                pCondItem)->GetValue();
                                    else
                                        eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
                                                pPattern->GetItem(ATTR_ROTATE_MODE)).GetValue();

                                    if ( nAttrRotate == 18000 )
                                        eRotMode = SVX_ROTATE_MODE_STANDARD;    // keinen Ueberlauf

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
                                BOOL bExtend = FALSE;           // ueber Zellenrand geschrieben ?
                                BOOL bClip = FALSE;
                                BOOL bSimClip = FALSE;
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
                                }

                                if ( !bBreak || eOrient!=SVX_ORIENTATION_STANDARD )
                                {
                                    if ( eOrient == SVX_ORIENTATION_STANDARD && !bMerged )
                                    {
                                        USHORT nNextX = bNegative ? (nX - 1) : (nX + 1);
                                        //  Unterlauf wird durch nNextX<=nX2 abgefangen
                                        while ( nGridWidth > aCellSize.Width() &&
                                                nNextX>=nX1 && nNextX<=nX2 &&
                                                !pThisRowInfo->pCellInfo[nNextX+1].bHOverlapped &&
                                                !pThisRowInfo->pCellInfo[nNextX+1].bVOverlapped &&
                                                !( bEditMode && nNextX==nEditCol && nY==nEditRow ) )
                                        {
                                            nOutWidth += pRowInfo[0].pCellInfo[nNextX+1].nWidth;
                                            if (bPixelToLogic)
                                                aCellSize = pRefDevice->PixelToLogic( Size( nOutWidth, nOutHeight ) );
                                            else
                                                aCellSize = Size( nOutWidth, nOutHeight );

                                            //  Grid abschalten
                                            if (bNegative)
                                            {
                                                pThisRowInfo->pCellInfo[nNextX+1].bHideGrid = TRUE;
                                                --nNextX;
                                            }
                                            else
                                            {
                                                pThisRowInfo->pCellInfo[nNextX].bHideGrid = TRUE;
                                                ++nNextX;
                                            }
                                            bExtend = TRUE;
                                        }
                                    }
                                    if ( nGridWidth > aCellSize.Width() )
                                    {
                                        bExtend = TRUE;     // -> immer linksbuendig
                                        bClip = TRUE;
                                        aClipSize.Width() = nOutWidth;      // Pixel
                                    }
                                }
                                if ( nEngineHeight > aCellSize.Height() )
                                {
                                    //  kein vertikales Clipping beim Drucken von Zellen mit
                                    //  optimaler Hoehe, ausser bei Groesse in bedingter
                                    //  Formatierung
                                    if ( eType != OUTTYPE_PRINTER ||
                                        ( pDoc->GetRowFlags( nY, nTab ) & CR_MANUALSIZE ) ||
                                        ( pCondSet && SFX_ITEM_SET ==
                                            pCondSet->GetItemState(ATTR_FONT_HEIGHT, TRUE) ) )
                                        bClip = TRUE;
                                    else
                                        bSimClip = TRUE;
                                    aClipSize.Height() = nOutHeight;        // Pixel
                                }

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

                                bClip = TRUE;       //! Test !!!!

                                Rectangle aClipRect;
                                if (bClip || bSimClip)
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

                                    if (bClip)  // bei bSimClip nur aClipRect initialisieren
                                    {
                                        if (bMetaFile)
                                        {
                                            pDev->Push();
                                            pDev->IntersectClipRegion( aClipRect );
                                        }
                                        else
                                            pDev->SetClipRegion( Region( aClipRect ) );
                                    }
                                }

                                Point aLogicStart;
                                if (bPixelToLogic)
                                    aLogicStart = pRefDevice->PixelToLogic( Point(nStartX,nStartY) );
                                else
                                    aLogicStart = Point(nStartX, nStartY);
                                if ( (eOrient!=SVX_ORIENTATION_STANDARD || !bBreak) && !bExtend )
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
                                else if (bExtend && eRotMode != SVX_ROTATE_MODE_STANDARD)
                                {
                                    //  Ausrichtung mit Bezugskante

                                    long nRealWidth = nCellWidth - 1;   //! Merge beruecksichtigen?
                                    if (bPixelToLogic)
                                        nRealWidth = pRefDevice->PixelToLogic(
                                                        Size( nRealWidth, 0 ) ).Width();

                                    //  hier ist Standard links (?)
                                    if (eHorJust==SVX_HOR_JUSTIFY_RIGHT)
                                        aLogicStart.X() += nRealWidth - nEngineWidth;
                                    else if (eHorJust==SVX_HOR_JUSTIFY_CENTER)
                                        aLogicStart.X() += (nRealWidth - nEngineWidth) / 2;
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

                                //! bSimClip is not used here (because nOriVal is set)

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
                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth;
            }
        }
        nRowPosY += pRowInfo[nArrY].nHeight;
    }

    delete pEngine;
}



