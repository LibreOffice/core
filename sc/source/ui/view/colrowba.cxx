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
#include <columnspanset.hxx>

// STATIC DATA -----------------------------------------------------------

static OUString lcl_MetricString( long nTwips, const OUString& rText )
{
    if ( nTwips <= 0 )
        return ScGlobal::GetRscString(STR_TIP_HIDE);
    else
    {
        FieldUnit eUserMet = SC_MOD()->GetAppOptions().GetAppMetric();

        sal_Int64 nUserVal = MetricField::ConvertValue( nTwips*100, 1, 2, FUNIT_TWIP, eUserMet );

        OUString aStr = rText;
        aStr += " ";
        aStr += ScGlobal::pLocaleData->getNum( nUserVal, 2 );
        aStr += " ";
        aStr += SdrFormatter::GetUnitStr(eUserMet);

        return aStr;
    }
}

ScColBar::ScColBar( vcl::Window* pParent, ScViewData* pData, ScHSplitPos eWhichPos,
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

inline bool ScColBar::UseNumericHeader() const
{
    return pViewData->GetDocument()->GetAddressConvention() == formula::FormulaGrammar::CONV_XL_R1C1;
}

SCCOLROW ScColBar::GetPos() const
{
    return pViewData->GetPosX(eWhich);
}

sal_uInt16 ScColBar::GetEntrySize( SCCOLROW nEntryNo ) const
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    if (pDoc->ColHidden(static_cast<SCCOL>(nEntryNo), nTab))
        return 0;
    else
        return (sal_uInt16) ScViewData::ToPixel( pDoc->GetColWidth( static_cast<SCCOL>(nEntryNo), nTab ), pViewData->GetPPTX() );
}

OUString ScColBar::GetEntryText( SCCOLROW nEntryNo ) const
{
    return UseNumericHeader()
        ? OUString::number(nEntryNo + 1)
        : ScColToAlpha( static_cast<SCCOL>(nEntryNo) );
}

void ScColBar::SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize )
{
    sal_uInt16 nSizeTwips;
    ScSizeMode eMode = SC_SIZE_DIRECT;
    if (nNewSize < 10) nNewSize = 10; // pixels

    if ( nNewSize == HDR_SIZE_OPTIMUM )
    {
        nSizeTwips = STD_EXTRA_WIDTH;
        eMode = SC_SIZE_OPTIMAL;
    }
    else
        nSizeTwips = (sal_uInt16) ( nNewSize / pViewData->GetPPTX() );

    ScMarkData& rMark = pViewData->GetMarkData();

    std::vector<sc::ColRowSpan> aRanges;
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
                aRanges.push_back(sc::ColRowSpan(nStart,nEnd));
                nStart = nEnd+1;
            }
            else
                nStart = MAXCOL+1;
        }
    }
    else
    {
        aRanges.push_back(sc::ColRowSpan(nPos,nPos));
    }

    pViewData->GetView()->SetWidthOrHeight(true, aRanges, eMode, nSizeTwips);
}

void ScColBar::HideEntries( SCCOLROW nStart, SCCOLROW nEnd )
{
    std::vector<sc::ColRowSpan> aRanges(1, sc::ColRowSpan(nStart,nEnd));
    pViewData->GetView()->SetWidthOrHeight(true, aRanges, SC_SIZE_DIRECT, 0);
}

void ScColBar::SetMarking( bool bSet )
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

    pFuncSet->SetColumn( true );
    pFuncSet->SetWhich( eActive );

    pViewSh->ActiveGrabFocus();
}

bool ScColBar::IsDisabled() const
{
    ScModule* pScMod = SC_MOD();
    return pScMod->IsFormulaMode() || pScMod->IsModalMode();
}

bool ScColBar::ResizeAllowed() const
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

OUString ScColBar::GetDragHelp( long nVal )
{
    long nTwips = (long) ( nVal / pViewData->GetPPTX() );
    return lcl_MetricString( nTwips, ScGlobal::GetRscString(STR_TIP_WIDTH) );
}

bool ScColBar::IsLayoutRTL() const        // overloaded only for columns
{
    return pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );
}

ScRowBar::ScRowBar( vcl::Window* pParent, ScViewData* pData, ScVSplitPos eWhichPos,
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

SCCOLROW ScRowBar::GetPos() const
{
    return pViewData->GetPosY(eWhich);
}

sal_uInt16 ScRowBar::GetEntrySize( SCCOLROW nEntryNo ) const
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

OUString ScRowBar::GetEntryText( SCCOLROW nEntryNo ) const
{
    return OUString::number( nEntryNo + 1 );
}

void ScRowBar::SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize )
{
    sal_uInt16 nSizeTwips;
    ScSizeMode eMode = SC_SIZE_DIRECT;
    if (nNewSize < 10) nNewSize = 10; // pixels

    if ( nNewSize == HDR_SIZE_OPTIMUM )
    {
        nSizeTwips = 0;
        eMode = SC_SIZE_OPTIMAL;
    }
    else
        nSizeTwips = (sal_uInt16) ( nNewSize / pViewData->GetPPTY() );

    ScMarkData& rMark = pViewData->GetMarkData();

    std::vector<sc::ColRowSpan> aRanges;
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
                aRanges.push_back(sc::ColRowSpan(nStart,nEnd));
                nStart = nEnd+1;
            }
            else
                nStart = MAXROW+1;
        }
    }
    else
    {
        aRanges.push_back(sc::ColRowSpan(nPos,nPos));
    }

    pViewData->GetView()->SetWidthOrHeight(false, aRanges, eMode, nSizeTwips);
}

void ScRowBar::HideEntries( SCCOLROW nStart, SCCOLROW nEnd )
{
    std::vector<sc::ColRowSpan> aRange(1, sc::ColRowSpan(nStart,nEnd));
    pViewData->GetView()->SetWidthOrHeight(false, aRange, SC_SIZE_DIRECT, 0);
}

void ScRowBar::SetMarking( bool bSet )
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

bool ScRowBar::IsDisabled() const
{
    ScModule* pScMod = SC_MOD();
    return pScMod->IsFormulaMode() || pScMod->IsModalMode();
}

bool ScRowBar::ResizeAllowed() const
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

OUString ScRowBar::GetDragHelp( long nVal )
{
    long nTwips = (long) ( nVal / pViewData->GetPPTY() );
    return lcl_MetricString( nTwips, ScGlobal::GetRscString(STR_TIP_HEIGHT) );
}

SCROW ScRowBar::GetHiddenCount( SCROW nEntryNo ) const   // overloaded only for rows
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    return pDoc->GetHiddenRowCount( nEntryNo, nTab );
}

bool ScRowBar::IsMirrored() const         // overloaded only for rows
{
    return pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
