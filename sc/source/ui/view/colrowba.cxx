/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <svx/svdtrans.hxx>
#include <unotools/localedatawrapper.hxx>

#include "colrowba.hxx"
#include "document.hxx"
#include "scmod.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "appoptio.hxx"
#include "globstr.hrc"
#include "markdata.hxx"

// STATIC DATA -----------------------------------------------------------

//==================================================================

static String lcl_MetricString( long nTwips, const String& rText )
{
    if ( nTwips <= 0 )
        return ScGlobal::GetRscString(STR_TIP_HIDE);
    else
    {
        FieldUnit eUserMet = SC_MOD()->GetAppOptions().GetAppMetric();

        sal_Int64 nUserVal = MetricField::ConvertValue( nTwips*100, 1, 2, FUNIT_TWIP, eUserMet );

        String aStr = rText;
        aStr += ' ';
        aStr += ScGlobal::pLocaleData->getNum( nUserVal, 2 );
        aStr += ' ';
        aStr += SdrFormatter::GetUnitStr(eUserMet);

        return aStr;
    }
}

//==================================================================

ScColBar::ScColBar( Window* pParent, ScViewData* pData, ScHSplitPos eWhichPos,
                    ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng ) :
            ScHeaderControl( pParent, pEng, MAXCOL+1, false ),
            pViewData( pData ),
            eWhich( eWhichPos ),
            pFuncSet( pFunc )
{
    Show();
}

ScColBar::~ScColBar()
{
}

inline sal_Bool ScColBar::UseNumericHeader() const
{
    return pViewData->GetDocument()->GetAddressConvention() == formula::FormulaGrammar::CONV_XL_R1C1;
}

SCCOLROW ScColBar::GetPos()
{
    return pViewData->GetPosX(eWhich);
}

sal_uInt16 ScColBar::GetEntrySize( SCCOLROW nEntryNo )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    if (pDoc->ColHidden(static_cast<SCCOL>(nEntryNo), nTab))
        return 0;
    else
        return (sal_uInt16) ScViewData::ToPixel( pDoc->GetColWidth( static_cast<SCCOL>(nEntryNo), nTab ), pViewData->GetPPTX() );
}

String ScColBar::GetEntryText( SCCOLROW nEntryNo )
{
    return UseNumericHeader()
        ? String::CreateFromInt32( nEntryNo + 1 )
        : ScColToAlpha( static_cast<SCCOL>(nEntryNo) );
}

void ScColBar::SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize )
{
    sal_uInt16 nSizeTwips;
    ScSizeMode eMode = SC_SIZE_DIRECT;
    if (nNewSize>0 && nNewSize<10) nNewSize=10;             // (Pixel)

    if ( nNewSize == HDR_SIZE_OPTIMUM )
    {
        nSizeTwips = STD_EXTRA_WIDTH;
        eMode = SC_SIZE_OPTIMAL;
    }
    else
        nSizeTwips = (sal_uInt16) ( nNewSize / pViewData->GetPPTX() );

    ScMarkData& rMark = pViewData->GetMarkData();

    SCCOLROW* pRanges = new SCCOLROW[MAXCOL+1];
    SCCOL nRangeCnt = 0;
    if ( rMark.IsColumnMarked( static_cast<SCCOL>(nPos) ) )
    {
        SCCOL nStart = 0;
        while (nStart<=MAXCOL)
        {
            while (nStart<MAXCOL && !rMark.IsColumnMarked(nStart))
                ++nStart;
            if (rMark.IsColumnMarked(nStart))
            {
                SCCOL nEnd = nStart;
                while (nEnd<MAXCOL && rMark.IsColumnMarked(nEnd))
                    ++nEnd;
                if (!rMark.IsColumnMarked(nEnd))
                    --nEnd;
                pRanges[static_cast<size_t>(2*nRangeCnt)  ] = nStart;
                pRanges[static_cast<size_t>(2*nRangeCnt+1)] = nEnd;
                ++nRangeCnt;
                nStart = nEnd+1;
            }
            else
                nStart = MAXCOL+1;
        }
    }
    else
    {
        pRanges[0] = nPos;
        pRanges[1] = nPos;
        nRangeCnt = 1;
    }

    pViewData->GetView()->SetWidthOrHeight( sal_True, nRangeCnt, pRanges, eMode, nSizeTwips );
    delete[] pRanges;
}

void ScColBar::HideEntries( SCCOLROW nStart, SCCOLROW nEnd )
{
    SCCOLROW nRange[2];
    nRange[0] = nStart;
    nRange[1] = nEnd;
    pViewData->GetView()->SetWidthOrHeight( sal_True, 1, nRange, SC_SIZE_DIRECT, 0 );
}

void ScColBar::SetMarking( sal_Bool bSet )
{
    pViewData->GetMarkData().SetMarking( bSet );
    if (!bSet)
    {
        pViewData->GetView()->UpdateAutoFillMark();
    }
}

void ScColBar::SelectWindow()
{
    ScTabViewShell* pViewSh = pViewData->GetViewShell();

    pViewSh->SetActive();           // Appear and SetViewFrame
    pViewSh->DrawDeselectAll();

    ScSplitPos eActive = pViewData->GetActivePart();
    if (eWhich==SC_SPLIT_LEFT)
    {
        if (eActive==SC_SPLIT_TOPRIGHT)     eActive=SC_SPLIT_TOPLEFT;
        if (eActive==SC_SPLIT_BOTTOMRIGHT)  eActive=SC_SPLIT_BOTTOMLEFT;
    }
    else
    {
        if (eActive==SC_SPLIT_TOPLEFT)      eActive=SC_SPLIT_TOPRIGHT;
        if (eActive==SC_SPLIT_BOTTOMLEFT)   eActive=SC_SPLIT_BOTTOMRIGHT;
    }
    pViewSh->ActivatePart( eActive );

    pFuncSet->SetColumn( sal_True );
    pFuncSet->SetWhich( eActive );

    pViewSh->ActiveGrabFocus();
}

sal_Bool ScColBar::IsDisabled()
{
    ScModule* pScMod = SC_MOD();
    return pScMod->IsFormulaMode() || pScMod->IsModalMode();
}

sal_Bool ScColBar::ResizeAllowed()
{
    return !pViewData->HasEditView( pViewData->GetActivePart() );
}

void ScColBar::DrawInvert( long nDragPosP )
{
    Rectangle aRect( nDragPosP,0, nDragPosP+HDR_SLIDERSIZE-1,GetOutputSizePixel().Width()-1 );
    Update();
    Invert(aRect);

    pViewData->GetView()->InvertVertical(eWhich,nDragPosP);
}

String ScColBar::GetDragHelp( long nVal )
{
    long nTwips = (long) ( nVal / pViewData->GetPPTX() );
    return lcl_MetricString( nTwips, ScGlobal::GetRscString(STR_TIP_WIDTH) );
}

sal_Bool ScColBar::IsLayoutRTL()        // overloaded only for columns
{
    return pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );
}

//==================================================================

ScRowBar::ScRowBar( Window* pParent, ScViewData* pData, ScVSplitPos eWhichPos,
                    ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng ) :
            ScHeaderControl( pParent, pEng, MAXROW+1, true ),
            pViewData( pData ),
            eWhich( eWhichPos ),
            pFuncSet( pFunc )
{
    Show();
}

ScRowBar::~ScRowBar()
{
}

SCCOLROW ScRowBar::GetPos()
{
    return pViewData->GetPosY(eWhich);
}

sal_uInt16 ScRowBar::GetEntrySize( SCCOLROW nEntryNo )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    SCROW nLastRow = -1;
    if (pDoc->RowHidden(nEntryNo, nTab, NULL, &nLastRow))
        return 0;
    else
        return (sal_uInt16) ScViewData::ToPixel( pDoc->GetOriginalHeight( nEntryNo,
                    nTab ), pViewData->GetPPTY() );
}

String ScRowBar::GetEntryText( SCCOLROW nEntryNo )
{
    return String::CreateFromInt32( nEntryNo + 1 );
}

void ScRowBar::SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize )
{
    sal_uInt16 nSizeTwips;
    ScSizeMode eMode = SC_SIZE_DIRECT;
    if (nNewSize>0 && nNewSize<10) nNewSize=10;             // (Pixel)

    if ( nNewSize == HDR_SIZE_OPTIMUM )
    {
        nSizeTwips = 0;
        eMode = SC_SIZE_OPTIMAL;
    }
    else
        nSizeTwips = (sal_uInt16) ( nNewSize / pViewData->GetPPTY() );

    ScMarkData& rMark = pViewData->GetMarkData();

    SCCOLROW* pRanges = new SCCOLROW[MAXROW+1];
    SCROW nRangeCnt = 0;
    if ( rMark.IsRowMarked( nPos ) )
    {
        SCROW nStart = 0;
        while (nStart<=MAXROW)
        {
            while (nStart<MAXROW && !rMark.IsRowMarked(nStart))
                ++nStart;
            if (rMark.IsRowMarked(nStart))
            {
                SCROW nEnd = nStart;
                while (nEnd<MAXROW && rMark.IsRowMarked(nEnd))
                    ++nEnd;
                if (!rMark.IsRowMarked(nEnd))
                    --nEnd;
                pRanges[static_cast<size_t>(2*nRangeCnt)  ] = nStart;
                pRanges[static_cast<size_t>(2*nRangeCnt+1)] = nEnd;
                ++nRangeCnt;
                nStart = nEnd+1;
            }
            else
                nStart = MAXROW+1;
        }
    }
    else
    {
        pRanges[0] = nPos;
        pRanges[1] = nPos;
        nRangeCnt = 1;
    }

    pViewData->GetView()->SetWidthOrHeight( false, nRangeCnt, pRanges, eMode, nSizeTwips );
    delete[] pRanges;
}

void ScRowBar::HideEntries( SCCOLROW nStart, SCCOLROW nEnd )
{
    SCCOLROW nRange[2];
    nRange[0] = nStart;
    nRange[1] = nEnd;
    pViewData->GetView()->SetWidthOrHeight( false, 1, nRange, SC_SIZE_DIRECT, 0 );
}

void ScRowBar::SetMarking( sal_Bool bSet )
{
    pViewData->GetMarkData().SetMarking( bSet );
    if (!bSet)
    {
        pViewData->GetView()->UpdateAutoFillMark();
    }
}

void ScRowBar::SelectWindow()
{
    ScTabViewShell* pViewSh = pViewData->GetViewShell();

    pViewSh->SetActive();           // Appear and SetViewFrame
    pViewSh->DrawDeselectAll();

    ScSplitPos eActive = pViewData->GetActivePart();
    if (eWhich==SC_SPLIT_TOP)
    {
        if (eActive==SC_SPLIT_BOTTOMLEFT)   eActive=SC_SPLIT_TOPLEFT;
        if (eActive==SC_SPLIT_BOTTOMRIGHT)  eActive=SC_SPLIT_TOPRIGHT;
    }
    else
    {
        if (eActive==SC_SPLIT_TOPLEFT)      eActive=SC_SPLIT_BOTTOMLEFT;
        if (eActive==SC_SPLIT_TOPRIGHT)     eActive=SC_SPLIT_BOTTOMRIGHT;
    }
    pViewSh->ActivatePart( eActive );

    pFuncSet->SetColumn( false );
    pFuncSet->SetWhich( eActive );

    pViewSh->ActiveGrabFocus();
}

sal_Bool ScRowBar::IsDisabled()
{
    ScModule* pScMod = SC_MOD();
    return pScMod->IsFormulaMode() || pScMod->IsModalMode();
}

sal_Bool ScRowBar::ResizeAllowed()
{
    return !pViewData->HasEditView( pViewData->GetActivePart() );
}

void ScRowBar::DrawInvert( long nDragPosP )
{
    Rectangle aRect( 0,nDragPosP, GetOutputSizePixel().Width()-1,nDragPosP+HDR_SLIDERSIZE-1 );
    Update();
    Invert(aRect);

    pViewData->GetView()->InvertHorizontal(eWhich,nDragPosP);
}

String ScRowBar::GetDragHelp( long nVal )
{
    long nTwips = (long) ( nVal / pViewData->GetPPTY() );
    return lcl_MetricString( nTwips, ScGlobal::GetRscString(STR_TIP_HEIGHT) );
}

SCROW ScRowBar::GetHiddenCount( SCROW nEntryNo )   // overloaded only for rows
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    return pDoc->GetHiddenRowCount( nEntryNo, nTab );
}

sal_Bool ScRowBar::IsMirrored()         // overloaded only for rows
{
    return pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
