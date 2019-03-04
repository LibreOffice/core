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

#include <unotools/localedatawrapper.hxx>

#include <colrowba.hxx>
#include <document.hxx>
#include <scmod.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <appoptio.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <markdata.hxx>
#include <tabview.hxx>
#include <columnspanset.hxx>

static OUString lcl_MetricString( long nTwips, const OUString& rText )
{
    if ( nTwips <= 0 )
        return ScResId(STR_TIP_HIDE);
    else
    {
        FieldUnit eUserMet = SC_MOD()->GetAppOptions().GetAppMetric();

        sal_Int64 nUserVal = MetricField::ConvertValue( nTwips*100, 1, 2, FieldUnit::TWIP, eUserMet );

        OUString aStr = rText + " "
                        + ScGlobal::pLocaleData->getNum( nUserVal, 2 )
                        + " " + SdrFormatter::GetUnitStr(eUserMet);
        return aStr;
    }
}

ScColBar::ScColBar( vcl::Window* pParent, ScHSplitPos eWhich,
                    ScHeaderFunctionSet* pFuncSet, ScHeaderSelectionEngine* pEng,
                    ScTabView* pTab ) :
            ScHeaderControl( pParent, pEng, MAXCOL+1, false, pTab ),
            meWhich( eWhich ),
            mpFuncSet( pFuncSet )
{
    Show();
}

ScColBar::~ScColBar()
{
}

SCCOLROW ScColBar::GetPos() const
{
    return pTabView->GetViewData().GetPosX(meWhich);
}

sal_uInt16 ScColBar::GetEntrySize( SCCOLROW nEntryNo ) const
{
    const ScViewData& rViewData = pTabView->GetViewData();
    ScDocument* pDoc = rViewData.GetDocument();
    SCTAB nTab = rViewData.GetTabNo();
    if (pDoc->ColHidden(static_cast<SCCOL>(nEntryNo), nTab))
        return 0;
    else
        return static_cast<sal_uInt16>(ScViewData::ToPixel( pDoc->GetColWidth( static_cast<SCCOL>(nEntryNo), nTab ), rViewData.GetPPTX() ));
}

OUString ScColBar::GetEntryText( SCCOLROW nEntryNo ) const
{
    return pTabView->GetViewData().GetDocument()->GetAddressConvention() == formula::FormulaGrammar::CONV_XL_R1C1
        ? OUString::number(nEntryNo + 1)
        : ScColToAlpha( static_cast<SCCOL>(nEntryNo) );
}

void ScColBar::SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize )
{
    const ScViewData& rViewData = pTabView->GetViewData();
    sal_uInt16 nSizeTwips;
    ScSizeMode eMode = SC_SIZE_DIRECT;
    if (nNewSize < 10) nNewSize = 10; // pixels

    if ( nNewSize == HDR_SIZE_OPTIMUM )
    {
        nSizeTwips = STD_EXTRA_WIDTH;
        eMode = SC_SIZE_OPTIMAL;
    }
    else
        nSizeTwips = static_cast<sal_uInt16>( nNewSize / rViewData.GetPPTX() );

    const ScMarkData& rMark = rViewData.GetMarkData();

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
                aRanges.emplace_back(nStart,nEnd);
                nStart = nEnd+1;
            }
            else
                nStart = MAXCOL+1;
        }
    }
    else
    {
        aRanges.emplace_back(nPos,nPos);
    }

    rViewData.GetView()->SetWidthOrHeight(true, aRanges, eMode, nSizeTwips);
}

void ScColBar::HideEntries( SCCOLROW nStart, SCCOLROW nEnd )
{
    std::vector<sc::ColRowSpan> aRanges(1, sc::ColRowSpan(nStart,nEnd));
    pTabView->GetViewData().GetView()->SetWidthOrHeight(true, aRanges, SC_SIZE_DIRECT, 0);
}

void ScColBar::SetMarking( bool bSet )
{
    pTabView->GetViewData().GetMarkData().SetMarking( bSet );
    if (!bSet)
    {
        pTabView->GetViewData().GetView()->UpdateAutoFillMark();
    }
}

void ScColBar::SelectWindow()
{
    const ScViewData& rViewData = pTabView->GetViewData();
    ScTabViewShell* pViewSh = rViewData.GetViewShell();

    pViewSh->SetActive();           // Appear and SetViewFrame
    pViewSh->DrawDeselectAll();

    ScSplitPos eActive = rViewData.GetActivePart();
    if (meWhich==SC_SPLIT_LEFT)
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

    mpFuncSet->SetColumn( true );
    mpFuncSet->SetWhich( eActive );

    pViewSh->ActiveGrabFocus();
}

bool ScColBar::IsDisabled() const
{
    ScModule* pScMod = SC_MOD();
    return pScMod->IsModalMode();
}

bool ScColBar::ResizeAllowed() const
{
    const ScViewData& rViewData = pTabView->GetViewData();
    return !rViewData.HasEditView( rViewData.GetActivePart() );
}

void ScColBar::DrawInvert( long nDragPosP )
{
    tools::Rectangle aRect( nDragPosP,0, nDragPosP+HDR_SLIDERSIZE-1,GetOutputSizePixel().Width()-1 );
    Update();
    Invert(aRect);

    pTabView->GetViewData().GetView()->InvertVertical(meWhich,nDragPosP);
}

OUString ScColBar::GetDragHelp( long nVal )
{
    long nTwips = static_cast<long>( nVal / pTabView->GetViewData().GetPPTX() );
    return lcl_MetricString( nTwips, ScResId(STR_TIP_WIDTH) );
}

bool ScColBar::IsLayoutRTL() const        // override only for columns
{
    const ScViewData& rViewData = pTabView->GetViewData();
    return rViewData.GetDocument()->IsLayoutRTL( rViewData.GetTabNo() );
}

ScRowBar::ScRowBar( vcl::Window* pParent, ScVSplitPos eWhich,
                    ScHeaderFunctionSet* pFuncSet, ScHeaderSelectionEngine* pEng,
                    ScTabView* pTab ) :
            ScHeaderControl( pParent, pEng, MAXROW+1, true, pTab ),
            meWhich( eWhich ),
            mpFuncSet( pFuncSet )
{
    Show();
}

ScRowBar::~ScRowBar()
{
}

SCCOLROW ScRowBar::GetPos() const
{
    return pTabView->GetViewData().GetPosY(meWhich);
}

sal_uInt16 ScRowBar::GetEntrySize( SCCOLROW nEntryNo ) const
{
    const ScViewData& rViewData = pTabView->GetViewData();
    ScDocument* pDoc = rViewData.GetDocument();
    SCTAB nTab = rViewData.GetTabNo();
    SCROW nLastRow = -1;
    if (pDoc->RowHidden(nEntryNo, nTab, nullptr, &nLastRow))
        return 0;
    else
        return static_cast<sal_uInt16>(ScViewData::ToPixel( pDoc->GetOriginalHeight( nEntryNo,
                    nTab ), rViewData.GetPPTY() ));
}

OUString ScRowBar::GetEntryText( SCCOLROW nEntryNo ) const
{
    return OUString::number( nEntryNo + 1 );
}

void ScRowBar::SetEntrySize( SCCOLROW nPos, sal_uInt16 nNewSize )
{
    const ScViewData& rViewData = pTabView->GetViewData();
    sal_uInt16 nSizeTwips;
    ScSizeMode eMode = SC_SIZE_DIRECT;
    if (nNewSize < 10) nNewSize = 10; // pixels

    if ( nNewSize == HDR_SIZE_OPTIMUM )
    {
        nSizeTwips = 0;
        eMode = SC_SIZE_OPTIMAL;
    }
    else
        nSizeTwips = static_cast<sal_uInt16>( nNewSize / rViewData.GetPPTY() );

    const ScMarkData& rMark = rViewData.GetMarkData();

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
                aRanges.emplace_back(nStart,nEnd);
                nStart = nEnd+1;
            }
            else
                nStart = MAXROW+1;
        }
    }
    else
    {
        aRanges.emplace_back(nPos,nPos);
    }

    rViewData.GetView()->SetWidthOrHeight(false, aRanges, eMode, nSizeTwips);
}

void ScRowBar::HideEntries( SCCOLROW nStart, SCCOLROW nEnd )
{
    std::vector<sc::ColRowSpan> aRange(1, sc::ColRowSpan(nStart,nEnd));
    pTabView->GetViewData().GetView()->SetWidthOrHeight(false, aRange, SC_SIZE_DIRECT, 0);
}

void ScRowBar::SetMarking( bool bSet )
{
    pTabView->GetViewData().GetMarkData().SetMarking( bSet );
    if (!bSet)
    {
        pTabView->GetViewData().GetView()->UpdateAutoFillMark();
    }
}

void ScRowBar::SelectWindow()
{
    const ScViewData& rViewData = pTabView->GetViewData();
    ScTabViewShell* pViewSh = rViewData.GetViewShell();

    pViewSh->SetActive();           // Appear and SetViewFrame
    pViewSh->DrawDeselectAll();

    ScSplitPos eActive = rViewData.GetActivePart();
    if (meWhich==SC_SPLIT_TOP)
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

    mpFuncSet->SetColumn( false );
    mpFuncSet->SetWhich( eActive );

    pViewSh->ActiveGrabFocus();
}

bool ScRowBar::IsDisabled() const
{
    ScModule* pScMod = SC_MOD();
    return pScMod->IsModalMode();
}

bool ScRowBar::ResizeAllowed() const
{
    const ScViewData& rViewData = pTabView->GetViewData();
    return !rViewData.HasEditView( rViewData.GetActivePart() );
}

void ScRowBar::DrawInvert( long nDragPosP )
{
    tools::Rectangle aRect( 0,nDragPosP, GetOutputSizePixel().Width()-1,nDragPosP+HDR_SLIDERSIZE-1 );
    Update();
    Invert(aRect);

    pTabView->GetViewData().GetView()->InvertHorizontal(meWhich,nDragPosP);
}

OUString ScRowBar::GetDragHelp( long nVal )
{
    long nTwips = static_cast<long>( nVal / pTabView->GetViewData().GetPPTY() );
    return lcl_MetricString( nTwips, ScResId(STR_TIP_HEIGHT) );
}

SCCOLROW ScRowBar::GetHiddenCount( SCCOLROW nEntryNo ) const // override only for rows
{
    const ScViewData& rViewData = pTabView->GetViewData();
    ScDocument* pDoc = rViewData.GetDocument();
    SCTAB nTab = rViewData.GetTabNo();
    return pDoc->GetHiddenRowCount( nEntryNo, nTab );
}

bool ScRowBar::IsMirrored() const // override only for rows
{
    const ScViewData& rViewData = pTabView->GetViewData();
    return rViewData.GetDocument()->IsLayoutRTL( rViewData.GetTabNo() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
