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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <officecfg/Office/Common.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/fmview.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/itemset.hxx>
#include <tools/multisel.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/settings.hxx>
#include <o3tl/deleter.hxx>
#include <o3tl/unit_conversion.hxx>

#include <preview.hxx>
#include <prevwsh.hxx>
#include <prevloc.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <printfun.hxx>
#include <printopt.hxx>
#include <stlpool.hxx>
#include <undostyl.hxx>
#include <drwlayer.hxx>
#include <scmod.hxx>
#include <markdata.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <sc.hrc>
#include <helpids.h>
#include <AccessibleDocumentPagePreview.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <AccessibilityHints.hxx>
#include <vcl/svapp.hxx>
#include <viewutil.hxx>
#include <patattr.hxx>
#include <columnspanset.hxx>

#include <memory>

#define SC_PREVIEW_SHADOWSIZE   2

static tools::Long lcl_GetDisplayStart( SCTAB nTab, const ScDocument* pDoc, std::vector<tools::Long>& nPages )
{
    tools::Long nDisplayStart = 0;
    for (SCTAB i=0; i<nTab; i++)
    {
        if ( pDoc->NeedPageResetAfterTab(i) )
            nDisplayStart = 0;
        else
            nDisplayStart += nPages[i];
    }
    return nDisplayStart;
}

ScPreview::ScPreview( vcl::Window* pParent, ScDocShell* pDocSh, ScPreviewShell* pViewSh ) :
    Window( pParent ),
    nPageNo( 0 ),
    nZoom( 100 ),
    nTabCount( 0 ),
    nTabsTested( 0 ),
    nTab( 0 ),
    nTabPage( 0 ),
    nTabStart( 0 ),
    nDisplayStart( 0 ),
    aDateTime( DateTime::SYSTEM ),
    nTotalPages( 0 ),
    pDocShell( pDocSh ),
    pViewShell( pViewSh ),
    bInGetState( false ),
    bValid( false ),
    bStateValid( false ),
    bLocationValid( false ),
    bInPaint( false ),
    bInSetZoom( false ),
    bLeftRulerMove( false ),
    bRightRulerMove( false ),
    bTopRulerMove( false ),
    bBottomRulerMove( false ),
    bHeaderRulerMove( false ),
    bFooterRulerMove( false ),
    bLeftRulerChange( false ),
    bRightRulerChange( false ),
    bTopRulerChange( false ),
    bBottomRulerChange( false ),
    bHeaderRulerChange( false ),
    bFooterRulerChange( false ),
    bPageMargin ( false ),
    bColRulerMove( false ),
    mbHasEmptyRangeTable(false),
    nLeftPosition( 0 ),
    mnScale( 0 ),
    nColNumberButtonDown( 0 ),
    nHeaderHeight ( 0 ),
    nFooterHeight ( 0 )
{
    GetOutDev()->SetOutDevViewType( OutDevViewType::PrintPreview );
    SetBackground();

    SetHelpId( HID_SC_WIN_PREVIEW );

    GetOutDev()->SetDigitLanguage( ScModule::GetOptDigitLanguage() );
}

ScPreview::~ScPreview()
{
    disposeOnce();
}

void ScPreview::dispose()
{
    pDrawView.reset();
    pLocationData.reset();
    vcl::Window::dispose();
}

void ScPreview::UpdateDrawView()        // nTab must be right
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScDrawLayer* pModel = rDoc.GetDrawLayer();     // is not 0

    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(nTab);
        if ( pDrawView && ( !pDrawView->GetSdrPageView() || pDrawView->GetSdrPageView()->GetPage() != pPage ) )
        {
            // convert the displayed Page of drawView (see below) does not work?!?
            pDrawView.reset();
        }

        if ( !pDrawView )                                   // New Drawing?
        {
            pDrawView.reset( new FmFormView( *pModel, GetOutDev()) );

            // The DrawView takes over the Design-Mode from the Model
            // (Settings "In opening Draftmode"), therefore to restore here
            pDrawView->SetDesignMode();
            pDrawView->SetPrintPreview();
            pDrawView->ShowSdrPage(pPage);
        }
    }
    else if ( pDrawView )
    {
        pDrawView.reset();   // for this Chart is not needed
    }
}

void ScPreview::TestLastPage()
{
    if (nPageNo < nTotalPages)
        return;

    if (nTotalPages)
    {
        nPageNo = nTotalPages - 1;
        nTab = static_cast<SCTAB>(nPages.size()) -1;
        while (nTab > 0 && !nPages[nTab])       // not the last empty Table
            --nTab;
        OSL_ENSURE(0 < static_cast<SCTAB>(nPages.size()),"are all tables empty?");
        nTabPage = nPages[nTab] - 1;
        nTabStart = 0;
        for (sal_uInt16 i=0; i<nTab; i++)
            nTabStart += nPages[i];

        ScDocument& rDoc = pDocShell->GetDocument();
        nDisplayStart = lcl_GetDisplayStart( nTab, &rDoc, nPages );
    }
    else        // empty Document
    {
        nTab = 0;
        nPageNo = nTabPage = nTabStart = nDisplayStart = 0;
        aState = ScPrintState();
    }
}

void ScPreview::CalcPages()
{
    weld::WaitObject aWait(GetFrameWeld());

    ScDocument& rDoc = pDocShell->GetDocument();
    nTabCount = rDoc.GetTableCount();

    if (maSelectedTabs.empty())
    {
        SCTAB nCurrentTab = ScDocShell::GetCurTab();
        maSelectedTabs.insert(nCurrentTab);
    }

    SCTAB nStart = nTabsTested;
    if (!bValid)
    {
        nStart = 0;
        nTotalPages = 0;
        nTabsTested = 0;
    }

    // update all pending row heights with a single progress bar,
    // instead of a separate progress for each sheet from ScPrintFunc
    pDocShell->UpdatePendingRowHeights( nTabCount-1, true );

    //  PrintOptions is passed to PrintFunc for SkipEmpty flag,
    //  but always all sheets are used (there is no selected sheet)
    ScPrintOptions aOptions = ScModule::get()->GetPrintOptions();

    while (nStart > static_cast<SCTAB>(nPages.size()))
        nPages.push_back(0);
    while (nStart > static_cast<SCTAB>(nFirstAttr.size()))
        nFirstAttr.push_back(1);

    for (SCTAB i=nStart; i<nTabCount; i++)
    {
        if ( i == static_cast<SCTAB>(nPages.size()))
            nPages.push_back(0);
        if ( i == static_cast<SCTAB>(nFirstAttr.size()))
            nFirstAttr.push_back(1);
        if (!aOptions.GetAllSheets() && maSelectedTabs.count(i) == 0)
        {
            nPages[i] = 0;
            nFirstAttr[i] = 1;
            continue;
        }

        tools::Long nAttrPage = i > 0 ? nFirstAttr[i-1] : 1;

        tools::Long nThisStart = nTotalPages;
        ScPrintFunc aPrintFunc( GetOutDev(), *pDocShell, i, nAttrPage, 0, nullptr, &aOptions );
        tools::Long nThisTab = aPrintFunc.GetTotalPages();
        if (!aPrintFunc.HasPrintRange())
            mbHasEmptyRangeTable = true;

        nPages[i] = nThisTab;
        nTotalPages += nThisTab;
        nFirstAttr[i] = aPrintFunc.GetFirstPageNo();    // to keep or from template

        if (nPageNo>=nThisStart && nPageNo<nTotalPages)
        {
            nTab = i;
            nTabPage = nPageNo - nThisStart;
            nTabStart = nThisStart;

            aPrintFunc.GetPrintState( aState );
        }
    }

    nDisplayStart = lcl_GetDisplayStart( nTab, &rDoc, nPages );

    if (nTabCount > nTabsTested)
        nTabsTested = nTabCount;

    TestLastPage();

    aState.nDocPages = nTotalPages;

    bValid = true;
    bStateValid = true;
    DoInvalidate();
}

void ScPreview::RecalcPages()           // only nPageNo is changed
{
    if (!bValid)
        return;                         // then CalcPages is called

    SCTAB nOldTab = nTab;

    bool bDone = false;
    while (nPageNo >= nTotalPages && nTabsTested < nTabCount)
    {
        CalcPages();
        bDone = true;
    }

    if (!bDone)
    {
        tools::Long nPartPages = 0;
        for (SCTAB i=0; i<nTabsTested && nTab < static_cast<SCTAB>(nPages.size()); i++)
        {
            tools::Long nThisStart = nPartPages;
            nPartPages += nPages[i];

            if (nPageNo>=nThisStart && nPageNo<nPartPages)
            {
                nTab = i;
                nTabPage = nPageNo - nThisStart;
                nTabStart = nThisStart;
            }
        }

        ScDocument& rDoc = pDocShell->GetDocument();
        nDisplayStart = lcl_GetDisplayStart( nTab, &rDoc, nPages );
    }

    TestLastPage();         // to test, if after last page

    if ( nTab != nOldTab )
        bStateValid = false;

    DoInvalidate();
}

void ScPreview::DoPrint( ScPreviewLocationData* pFillLocation )
{
    if (!bValid)
    {
        CalcPages();
        RecalcPages();
        UpdateDrawView();       // Spreadsheet eventually changes
    }

    Fraction aPreviewZoom( nZoom, 100 );
    Fraction aHorPrevZoom( static_cast<tools::Long>( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode aMMMode( MapUnit::Map100thMM, Point(), aHorPrevZoom, aPreviewZoom );

    bool bDoPrint = ( pFillLocation == nullptr );
    bool bValidPage = ( nPageNo < nTotalPages );

    ScModule* pScMod = ScModule::get();
    const svtools::ColorConfig& rColorCfg = pScMod->GetColorConfig();
    Color aBackColor( rColorCfg.GetColorValue(svtools::APPBACKGROUND).nColor );

    if ( bDoPrint && ( aOffset.X() < 0 || aOffset.Y() < 0 ) && bValidPage )
    {
        SetMapMode( aMMMode );
        GetOutDev()->SetLineColor();
        GetOutDev()->SetFillColor(aBackColor);

        Size aWinSize = GetOutDev()->GetOutputSize();
        if ( aOffset.X() < 0 )
            GetOutDev()->DrawRect(tools::Rectangle( 0, 0, -aOffset.X(), aWinSize.Height() ));
        if ( aOffset.Y() < 0 )
            GetOutDev()->DrawRect(tools::Rectangle( 0, 0, aWinSize.Width(), -aOffset.Y() ));
    }

    tools::Long   nLeftMargin = 0;
    tools::Long   nRightMargin = 0;
    tools::Long   nTopMargin = 0;
    tools::Long   nBottomMargin = 0;
    bool   bHeaderOn = false;
    bool   bFooterOn = false;

    ScDocument& rDoc = pDocShell->GetDocument();
    bool   bLayoutRTL = rDoc.IsLayoutRTL( nTab );

    Size aLocalPageSize;
    if ( bValidPage )
    {
        ScPrintOptions aOptions = pScMod->GetPrintOptions();

        std::unique_ptr<ScPrintFunc, o3tl::default_delete<ScPrintFunc>> pPrintFunc;
        if (bStateValid)
            pPrintFunc.reset(new ScPrintFunc(GetOutDev(), *pDocShell, aState, &aOptions));
        else
            pPrintFunc.reset(new ScPrintFunc(GetOutDev(), *pDocShell, nTab, nFirstAttr[nTab], nTotalPages, nullptr, &aOptions));

        pPrintFunc->SetOffset(aOffset);
        pPrintFunc->SetManualZoom(nZoom);
        pPrintFunc->SetDateTime(aDateTime);
        pPrintFunc->SetClearFlag(true);
        pPrintFunc->SetUseStyleColor(false); // tdf#101142 print preview should use a white background

        pPrintFunc->SetDrawView( pDrawView.get() );

        // MultiSelection for the one Page must produce something inconvenient
        Range aPageRange( nPageNo+1, nPageNo+1 );
        MultiSelection aPage( aPageRange );
        aPage.SetTotalRange( Range(0,RANGE_MAX) );
        aPage.Select( aPageRange );

        tools::Long nPrinted = pPrintFunc->DoPrint( aPage, nTabStart, nDisplayStart, bDoPrint, pFillLocation );
        OSL_ENSURE(nPrinted<=1, "What is happening?");

        SetMapMode(aMMMode);

        //init nLeftMargin ... in the ScPrintFunc::InitParam!!!
        nLeftMargin = pPrintFunc->GetLeftMargin();
        nRightMargin = pPrintFunc->GetRightMargin();
        nTopMargin = pPrintFunc->GetTopMargin();
        nBottomMargin = pPrintFunc->GetBottomMargin();
        nHeaderHeight = pPrintFunc->GetHeader().nHeight;
        nFooterHeight = pPrintFunc->GetFooter().nHeight;
        bHeaderOn = pPrintFunc->GetHeader().bEnable;
        bFooterOn = pPrintFunc->GetFooter().bEnable;
        mnScale = pPrintFunc->GetZoom();

        if ( bDoPrint && bPageMargin && pLocationData )     // don't make use of pLocationData while filling it
        {
            tools::Rectangle aPixRect;
            tools::Rectangle aRectCellPosition;
            tools::Rectangle aRectPosition;
            pLocationData->GetMainCellRange( aPageArea, aPixRect );
            mvRight.resize(aPageArea.aEnd.Col()+1);
            if( !bLayoutRTL )
            {
                aRectPosition = pLocationData->GetCellPosition(aPageArea.aStart);
                nLeftPosition = aRectPosition.Left();
                for( SCCOL i = aPageArea.aStart.Col(); i <= aPageArea.aEnd.Col(); i++ )
                {
                    aRectCellPosition = pLocationData->GetCellPosition(
                        ScAddress(i, aPageArea.aStart.Row(), aPageArea.aStart.Tab()));
                    mvRight[i] = aRectCellPosition.Right();
                }
            }
            else
            {
                aRectPosition = pLocationData->GetCellPosition(aPageArea.aEnd);
                nLeftPosition = aRectPosition.Right()+1;

                aRectCellPosition = pLocationData->GetCellPosition(aPageArea.aStart);
                mvRight[ aPageArea.aEnd.Col() ] = aRectCellPosition.Left();
                for( SCCOL i = aPageArea.aEnd.Col(); i > aPageArea.aStart.Col(); i-- )
                {
                    aRectCellPosition = pLocationData->GetCellPosition(
                        ScAddress(i, aPageArea.aEnd.Row(), aPageArea.aEnd.Tab()));
                    mvRight[ i-1 ] = mvRight[ i ] + aRectCellPosition.Right() - aRectCellPosition.Left() + 1;
                }
            }
        }

        if (nPrinted)   // if not, draw everything grey
        {
            aLocalPageSize = pPrintFunc->GetPageSize();
            aLocalPageSize.setWidth(
                o3tl::convert(aLocalPageSize.Width(), o3tl::Length::twip, o3tl::Length::mm100));
            aLocalPageSize.setHeight(
                o3tl::convert(aLocalPageSize.Height(), o3tl::Length::twip, o3tl::Length::mm100));

            nLeftMargin = o3tl::convert(nLeftMargin, o3tl::Length::twip, o3tl::Length::mm100);
            nRightMargin = o3tl::convert(nRightMargin, o3tl::Length::twip, o3tl::Length::mm100);
            nTopMargin = o3tl::convert(nTopMargin, o3tl::Length::twip, o3tl::Length::mm100);
            nBottomMargin = o3tl::convert(nBottomMargin, o3tl::Length::twip, o3tl::Length::mm100);
            constexpr auto md = o3tl::getConversionMulDiv(o3tl::Length::twip, o3tl::Length::mm10);
            const auto m = md.first * mnScale, d = md.second * 100;
            nHeaderHeight = o3tl::convert(nHeaderHeight, m, d) + nTopMargin;
            nFooterHeight = o3tl::convert(nFooterHeight, m, d) + nBottomMargin;
        }

        if (!bStateValid)
        {
            pPrintFunc->GetPrintState( aState );
            aState.nDocPages = nTotalPages;
            bStateValid = true;
        }
    }

    if ( !bDoPrint )
        return;

    tools::Long nPageEndX = aLocalPageSize.Width()  - aOffset.X();
    tools::Long nPageEndY = aLocalPageSize.Height() - aOffset.Y();
    if ( !bValidPage )
        nPageEndX = nPageEndY = 0;

    Size aWinSize = GetOutDev()->GetOutputSize();
    Point aWinEnd( aWinSize.Width(), aWinSize.Height() );
    bool bRight  = nPageEndX <= aWinEnd.X();
    bool bBottom = nPageEndY <= aWinEnd.Y();

    if (!nTotalPages)
    {
        // There is no data to print. Print a friendly warning message and
        // bail out.

        SetMapMode(aMMMode);

        // Draw background first.
        GetOutDev()->SetLineColor();
        GetOutDev()->SetFillColor(aBackColor);
        GetOutDev()->DrawRect(tools::Rectangle(0, 0, aWinEnd.X(), aWinEnd.Y()));

        const ScPatternAttr& rDefPattern(rDoc.getCellAttributeHelper().getDefaultCellAttribute());

        std::unique_ptr<ScEditEngineDefaulter> pEditEng(
            new ScEditEngineDefaulter(EditEngine::CreatePool().get(), true));

        pEditEng->SetRefMapMode(aMMMode);
        SfxItemSet aEditDefaults( pEditEng->GetEmptyItemSet() );
        rDefPattern.FillEditItemSet(&aEditDefaults);
        aEditDefaults.Put(SvxColorItem(COL_LIGHTGRAY, EE_CHAR_COLOR));
        pEditEng->SetDefaults(std::move(aEditDefaults));

        OUString aEmptyMsg;
        if (mbHasEmptyRangeTable)
            aEmptyMsg = ScResId(STR_PRINT_PREVIEW_EMPTY_RANGE);
        else
            aEmptyMsg = ScResId(STR_PRINT_PREVIEW_NODATA);

        tools::Long nHeight = 3000;
        pEditEng->SetDefaultItem(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT));
        pEditEng->SetDefaultItem(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
        pEditEng->SetDefaultItem(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT_CTL));

        pEditEng->SetTextCurrentDefaults(aEmptyMsg);

        Point aCenter(
            (aWinEnd.X() - pEditEng->CalcTextWidth())/2,
            (aWinEnd.Y() - pEditEng->GetTextHeight())/2);

        pEditEng->DrawText_ToPosition(*GetOutDev(), aCenter);

        return;
    }

    if( bPageMargin && bValidPage )
    {
        SetMapMode(aMMMode);
        GetOutDev()->SetLineColor( COL_BLACK );
        DrawInvert( static_cast<tools::Long>( nTopMargin - aOffset.Y() ), PointerStyle::VSizeBar );
        DrawInvert( static_cast<tools::Long>(nPageEndY - nBottomMargin ), PointerStyle::VSizeBar );
        DrawInvert( static_cast<tools::Long>( nLeftMargin - aOffset.X() ), PointerStyle::HSizeBar );
        DrawInvert( static_cast<tools::Long>( nPageEndX - nRightMargin ) , PointerStyle::HSizeBar );
        if( bHeaderOn )
        {
            DrawInvert( nHeaderHeight - aOffset.Y(), PointerStyle::VSizeBar );
        }
        if( bFooterOn )
        {
            DrawInvert( nPageEndY - nFooterHeight, PointerStyle::VSizeBar );
        }

        SetMapMode( MapMode( MapUnit::MapPixel ) );
        for( int i= aPageArea.aStart.Col(); i<= aPageArea.aEnd.Col(); i++ )
        {
            Point aColumnTop = LogicToPixel( Point( 0, -aOffset.Y() ) ,aMMMode );
            GetOutDev()->SetLineColor( COL_BLACK );
            GetOutDev()->SetFillColor( COL_BLACK );
            GetOutDev()->DrawRect( tools::Rectangle( Point( mvRight[i] - 2, aColumnTop.Y() ),Point( mvRight[i] + 2 , 4 + aColumnTop.Y()) ));
            GetOutDev()->DrawLine( Point( mvRight[i], aColumnTop.Y() ), Point( mvRight[i],  10 + aColumnTop.Y()) );
        }
        SetMapMode( aMMMode );
    }

    if (bRight || bBottom)
    {
        SetMapMode(aMMMode);
        GetOutDev()->SetLineColor();
        GetOutDev()->SetFillColor(aBackColor);
        if (bRight)
            GetOutDev()->DrawRect(tools::Rectangle(nPageEndX,0, aWinEnd.X(),aWinEnd.Y()));
        if (bBottom)
        {
            if (bRight)
                GetOutDev()->DrawRect(tools::Rectangle(0,nPageEndY, nPageEndX,aWinEnd.Y()));    // Corner not duplicated
            else
                GetOutDev()->DrawRect(tools::Rectangle(0,nPageEndY, aWinEnd.X(),aWinEnd.Y()));
        }
    }

    if ( !bValidPage )
        return;

    Color aBorderColor(ScModule::get()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor);

    //  draw border

    if ( aOffset.X() <= 0 || aOffset.Y() <= 0 || bRight || bBottom )
    {
        GetOutDev()->SetLineColor( aBorderColor );
        GetOutDev()->SetFillColor();

        tools::Rectangle aPixel( LogicToPixel( tools::Rectangle( -aOffset.X(), -aOffset.Y(), nPageEndX, nPageEndY ) ) );
        aPixel.AdjustRight( -1 );
        aPixel.AdjustBottom( -1 );
        GetOutDev()->DrawRect( PixelToLogic( aPixel ) );
    }

    //  draw shadow

    GetOutDev()->SetLineColor();
    GetOutDev()->SetFillColor( aBorderColor );

    tools::Rectangle aPixel;

    aPixel = LogicToPixel( tools::Rectangle( nPageEndX, -aOffset.Y(), nPageEndX, nPageEndY ) );
    aPixel.AdjustTop(SC_PREVIEW_SHADOWSIZE );
    aPixel.AdjustRight(SC_PREVIEW_SHADOWSIZE - 1 );
    aPixel.AdjustBottom(SC_PREVIEW_SHADOWSIZE - 1 );
    GetOutDev()->DrawRect( PixelToLogic( aPixel ) );

    aPixel = LogicToPixel( tools::Rectangle( -aOffset.X(), nPageEndY, nPageEndX, nPageEndY ) );
    aPixel.AdjustLeft(SC_PREVIEW_SHADOWSIZE );
    aPixel.AdjustRight(SC_PREVIEW_SHADOWSIZE - 1 );
    aPixel.AdjustBottom(SC_PREVIEW_SHADOWSIZE - 1 );
    GetOutDev()->DrawRect( PixelToLogic( aPixel ) );
}

void ScPreview::Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& /* rRect */ )
{
    bool bWasInPaint = bInPaint;        // nested calls shouldn't be necessary, but allow for now
    bInPaint = true;

    if (bPageMargin)
        GetLocationData();              // fill location data for column positions
    DoPrint( nullptr );
    pViewShell->UpdateScrollBars();

    bInPaint = bWasInPaint;
}

void ScPreview::Command( const CommandEvent& rCEvt )
{
    CommandEventId nCmd = rCEvt.GetCommand();
    if ( nCmd == CommandEventId::Wheel || nCmd == CommandEventId::StartAutoScroll || nCmd == CommandEventId::AutoScroll )
    {
        bool bDone = pViewShell->ScrollCommand( rCEvt );
        if (!bDone)
            Window::Command(rCEvt);
    }
    else if ( nCmd == CommandEventId::ContextMenu )
        SfxDispatcher::ExecutePopup();
    else
        Window::Command( rCEvt );
}

void ScPreview::KeyInput( const KeyEvent& rKEvt )
{
    //  The + and - keys can't be configured as accelerator entries, so they must be handled directly
    //  (in ScPreview, not ScPreviewShell -> only if the preview window has the focus)

    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nKey = rKeyCode.GetCode();
    bool bHandled = false;
    if(!rKeyCode.GetModifier())
    {
        sal_uInt16 nSlot = 0;
        switch(nKey)
        {
            case KEY_ADD:      nSlot = SID_ZOOM_IN;  break;
            case KEY_ESCAPE:   nSlot = ScViewUtil::IsFullScreen( *pViewShell ) ? SID_CANCEL : SID_PREVIEW_CLOSE; break;
            case KEY_SUBTRACT: nSlot = SID_ZOOM_OUT; break;
        }
        if(nSlot)
        {
            bHandled = true;
            pViewShell->GetViewFrame().GetDispatcher()->Execute( nSlot, SfxCallMode::ASYNCHRON );
        }
    }

    if ( !bHandled && !pViewShell->KeyInput(rKEvt) )
        Window::KeyInput(rKEvt);
}

const ScPreviewLocationData& ScPreview::GetLocationData()
{
    if ( !pLocationData )
    {
        pLocationData.reset( new ScPreviewLocationData( &pDocShell->GetDocument(), GetOutDev() ) );
        bLocationValid = false;
    }
    if ( !bLocationValid )
    {
        pLocationData->Clear();
        DoPrint( pLocationData.get() );
        bLocationValid = true;
    }
    return *pLocationData;
}

void ScPreview::DataChanged(bool bNewTime)
{
    if (bNewTime)
        aDateTime = DateTime( DateTime::SYSTEM );

    bValid = false;
    InvalidateLocationData( SfxHintId::ScDataChanged );
    Invalidate();
}

OUString ScPreview::GetPosString()
{
    if (!bValid)
    {
        CalcPages();
        UpdateDrawView();       // The table eventually changes
    }

    OUString aString = ScResId( STR_PAGE ) +
                       " " + OUString::number(nPageNo+1);

    if (nTabsTested >= nTabCount)
        aString += " / " + OUString::number(nTotalPages);

    return aString;
}

void ScPreview::SetZoom(sal_uInt16 nNewZoom)
{
    if (nNewZoom < 20)
        nNewZoom = 20;
    if (nNewZoom > 400)
        nNewZoom = 400;
    if (nNewZoom == nZoom)
        return;

    nZoom = nNewZoom;

    //  apply new MapMode and call UpdateScrollBars to update aOffset

    Fraction aPreviewZoom( nZoom, 100 );
    Fraction aHorPrevZoom( static_cast<tools::Long>( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode aMMMode( MapUnit::Map100thMM, Point(), aHorPrevZoom, aPreviewZoom );
    SetMapMode( aMMMode );

    bInSetZoom = true;              // don't scroll during SetYOffset in UpdateScrollBars
    pViewShell->UpdateNeededScrollBars(true);
    bInSetZoom = false;

    InvalidateLocationData( SfxHintId::ScAccVisAreaChanged );
    DoInvalidate();
    Invalidate();
}

void ScPreview::SetPageNo( tools::Long nPage )
{
    nPageNo = nPage;
    RecalcPages();
    UpdateDrawView();       // The table eventually changes
    InvalidateLocationData( SfxHintId::ScDataChanged );
    Invalidate();
}

tools::Long ScPreview::GetFirstPage(SCTAB nTabP)
{
    SCTAB nDocTabCount = pDocShell->GetDocument().GetTableCount();
    if (nTabP >= nDocTabCount)
        nTabP = nDocTabCount-1;

    tools::Long nPage = 0;
    if (nTabP>0)
    {
        CalcPages();
        if (nTabP >= static_cast<SCTAB>(nPages.size()) )
            OSL_FAIL("nPages out of bounds, FIX IT");
        UpdateDrawView();       // The table eventually changes

        for (SCTAB i=0; i<nTabP; i++)
            nPage += nPages[i];

        // An empty Table on the previous Page

        if ( nPages[nTabP]==0 && nPage > 0 )
            --nPage;
    }

    return nPage;
}

static Size lcl_GetDocPageSize( const ScDocument* pDoc, SCTAB nTab )
{
    OUString aName = pDoc->GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aName, SfxStyleFamily::Page );
    if ( pStyleSheet )
    {
        SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
        return rStyleSet.Get(ATTR_PAGE_SIZE).GetSize();
    }
    else
    {
        OSL_FAIL( "PageStyle not found" );
        return Size();
    }
}

sal_uInt16 ScPreview::GetOptimalZoom(bool bWidthOnly)
{
    double nWinScaleX = ScGlobal::nScreenPPTX / pDocShell->GetOutputFactor();
    double nWinScaleY = ScGlobal::nScreenPPTY;
    Size aWinSize = GetOutputSizePixel();

    //  desired margin is 0.25cm in default MapMode (like Writer),
    //  but some additional margin is introduced by integer scale values
    //  -> add only 0.10cm, so there is some margin in all cases.
    Size aMarginSize( LogicToPixel(Size(100, 100), MapMode(MapUnit::Map100thMM)) );
    aWinSize.AdjustWidth( -(2 * aMarginSize.Width()) );
    aWinSize.AdjustHeight( -(2 * aMarginSize.Height()) );

    Size aLocalPageSize = lcl_GetDocPageSize( &pDocShell->GetDocument(), nTab );
    if ( aLocalPageSize.Width() && aLocalPageSize.Height() )
    {
        tools::Long nZoomX = static_cast<tools::Long>( aWinSize.Width() * 100  / ( aLocalPageSize.Width() * nWinScaleX ));
        tools::Long nZoomY = static_cast<tools::Long>( aWinSize.Height() * 100 / ( aLocalPageSize.Height() * nWinScaleY ));

        tools::Long nOptimal = nZoomX;
        if (!bWidthOnly && nZoomY<nOptimal)
            nOptimal = nZoomY;

        if (nOptimal<20)
            nOptimal = 20;
        if (nOptimal>400)
            nOptimal = 400;

        return static_cast<sal_uInt16>(nOptimal);
    }
    else
        return nZoom;
}

void ScPreview::SetXOffset( tools::Long nX )
{
    if ( aOffset.X() == nX )
        return;

    if (bValid)
    {
        tools::Long nDif = LogicToPixel(aOffset).X() - LogicToPixel(Point(nX,0)).X();
        aOffset.setX( nX );
        if (nDif && !bInSetZoom)
        {
            MapMode aOldMode = GetMapMode();
            SetMapMode(MapMode(MapUnit::MapPixel));
            Scroll( nDif, 0 );
            SetMapMode(aOldMode);
        }
    }
    else
    {
        aOffset.setX( nX );
        if (!bInSetZoom)
            Invalidate();
    }
    InvalidateLocationData( SfxHintId::ScAccVisAreaChanged );
    Invalidate();
}

void ScPreview::SetYOffset( tools::Long nY )
{
    if ( aOffset.Y() == nY )
        return;

    if (bValid)
    {
        tools::Long nDif = LogicToPixel(aOffset).Y() - LogicToPixel(Point(0,nY)).Y();
        aOffset.setY( nY );
        if (nDif && !bInSetZoom)
        {
            MapMode aOldMode = GetMapMode();
            SetMapMode(MapMode(MapUnit::MapPixel));
            Scroll( 0, nDif );
            SetMapMode(aOldMode);
        }
    }
    else
    {
        aOffset.setY( nY );
        if (!bInSetZoom)
            Invalidate();
    }
    InvalidateLocationData( SfxHintId::ScAccVisAreaChanged );
    Invalidate();
}

void ScPreview::DoInvalidate()
{
    //  If the whole GetState of the shell is called
    //  The Invalidate must come behind asynchronously

    if (bInGetState)
        Application::PostUserEvent( LINK( this, ScPreview, InvalidateHdl ), nullptr, true );
    else
        StaticInvalidate();     // Immediately
}

void ScPreview::StaticInvalidate()
{
    //  static method, because it's called asynchronously
    //  -> must use current viewframe

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    SfxBindings& rBindings = pViewFrm->GetBindings();
    rBindings.Invalidate(SID_STATUS_DOCPOS);
    rBindings.Invalidate(FID_AUTO_CALC);
    rBindings.Invalidate(SID_ROWCOL_SELCOUNT);
    rBindings.Invalidate(SID_STATUS_PAGESTYLE);
    rBindings.Invalidate(SID_PREVIEW_PREVIOUS);
    rBindings.Invalidate(SID_PREVIEW_NEXT);
    rBindings.Invalidate(SID_PREVIEW_FIRST);
    rBindings.Invalidate(SID_PREVIEW_LAST);
    rBindings.Invalidate(SID_ATTR_ZOOM);
    rBindings.Invalidate(SID_ZOOM_IN);
    rBindings.Invalidate(SID_ZOOM_OUT);
    rBindings.Invalidate(SID_PREVIEW_SCALINGFACTOR);
    rBindings.Invalidate(SID_ATTR_ZOOMSLIDER);
}

IMPL_STATIC_LINK_NOARG( ScPreview, InvalidateHdl, void*, void )
{
    StaticInvalidate();
}

void ScPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged(rDCEvt);

    if ( !((rDCEvt.GetType() == DataChangedEventType::PRINTER) ||
         (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))) )
        return;

    if ( rDCEvt.GetType() == DataChangedEventType::FONTS )
        pDocShell->UpdateFontList();

    // #i114518# Paint of form controls may modify the window's settings.
    // Ignore the event if it is called from within Paint.
    if ( !bInPaint )
    {
        if ( rDCEvt.GetType() == DataChangedEventType::SETTINGS &&
              (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
        {
            //  scroll bar size may have changed
            pViewShell->InvalidateBorder();     // calls OuterResizePixel
        }
        Invalidate();
        InvalidateLocationData( SfxHintId::ScDataChanged );
    }
}

void ScPreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    Fraction  aPreviewZoom( nZoom, 100 );
    Fraction  aHorPrevZoom( static_cast<tools::Long>( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode   aMMMode( MapUnit::Map100thMM, Point(), aHorPrevZoom, aPreviewZoom );

    aButtonDownChangePoint = PixelToLogic( rMEvt.GetPosPixel(),aMMMode );
    aButtonDownPt = PixelToLogic( rMEvt.GetPosPixel(),aMMMode );

    CaptureMouse();

    if( rMEvt.IsLeft() && GetPointer() == PointerStyle::HSizeBar )
    {
        SetMapMode( aMMMode );
        if( bLeftRulerChange )
        {
           DrawInvert( aButtonDownChangePoint.X(), PointerStyle::HSizeBar );
           bLeftRulerMove = true;
           bRightRulerMove = false;
        }
        else if( bRightRulerChange )
        {
           DrawInvert( aButtonDownChangePoint.X(), PointerStyle::HSizeBar );
           bLeftRulerMove = false;
           bRightRulerMove = true;
        }
    }

    if( rMEvt.IsLeft() && GetPointer() == PointerStyle::VSizeBar )
    {
        SetMapMode( aMMMode );
        if( bTopRulerChange )
        {
            DrawInvert( aButtonDownChangePoint.Y(), PointerStyle::VSizeBar );
            bTopRulerMove = true;
            bBottomRulerMove = false;
        }
        else if( bBottomRulerChange )
        {
            DrawInvert( aButtonDownChangePoint.Y(), PointerStyle::VSizeBar );
            bTopRulerMove = false;
            bBottomRulerMove = true;
        }
        else if( bHeaderRulerChange )
        {
            DrawInvert( aButtonDownChangePoint.Y(), PointerStyle::VSizeBar );
            bHeaderRulerMove = true;
            bFooterRulerMove = false;
        }
        else if( bFooterRulerChange )
        {
            DrawInvert( aButtonDownChangePoint.Y(), PointerStyle::VSizeBar );
            bHeaderRulerMove = false;
            bFooterRulerMove = true;
        }
    }

    if( !(rMEvt.IsLeft() && GetPointer() == PointerStyle::HSplit) )
        return;

    Point  aNowPt = rMEvt.GetPosPixel();
    SCCOL i = 0;
    for( i = aPageArea.aStart.Col(); i<= aPageArea.aEnd.Col(); i++ )
    {
        if( aNowPt.X() < mvRight[i] + 2 && aNowPt.X() > mvRight[i] - 2 )
        {
            nColNumberButtonDown = i;
            break;
        }
    }
    if( i == aPageArea.aEnd.Col()+1 )
        return;

    SetMapMode( aMMMode );
    if( nColNumberButtonDown == aPageArea.aStart.Col() )
        DrawInvert( PixelToLogic( Point( nLeftPosition, 0 ),aMMMode ).X() ,PointerStyle::HSplit );
    else
        DrawInvert( PixelToLogic( Point( mvRight[ nColNumberButtonDown-1 ], 0 ),aMMMode ).X() ,PointerStyle::HSplit );

    DrawInvert( aButtonDownChangePoint.X(), PointerStyle::HSplit );
    bColRulerMove = true;
}

void ScPreview::MouseButtonUp( const MouseEvent& rMEvt )
{
        Fraction  aPreviewZoom( nZoom, 100 );
        Fraction  aHorPrevZoom( static_cast<tools::Long>( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
        MapMode   aMMMode( MapUnit::Map100thMM, Point(), aHorPrevZoom, aPreviewZoom );

        aButtonUpPt = PixelToLogic( rMEvt.GetPosPixel(),aMMMode );

        tools::Long  nWidth = lcl_GetDocPageSize(&pDocShell->GetDocument(), nTab).Width();
        tools::Long  nHeight = lcl_GetDocPageSize(&pDocShell->GetDocument(), nTab).Height();

        if( rMEvt.IsLeft() && GetPointer() == PointerStyle::HSizeBar )
        {
            SetPointer( PointerStyle::Arrow );

            ScDocument& rDoc = pDocShell->GetDocument();
            OUString aOldName = rDoc.GetPageStyle( nTab );
            bool bUndo = rDoc.IsUndoEnabled();
            ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
            SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aOldName, SfxStyleFamily::Page );

            if ( pStyleSheet )
            {
                bool bMoveRulerAction= true;
                ScStyleSaveData aOldData;
                if( bUndo )
                    aOldData.InitFromStyle( pStyleSheet );

                SfxItemSet&  rStyleSet = pStyleSheet->GetItemSet();

                SvxLRSpaceItem aLRItem = rStyleSet.Get( ATTR_LRSPACE );

                if(( bLeftRulerChange || bRightRulerChange ) && ( aButtonUpPt.X() <= ( 0 - aOffset.X() ) || aButtonUpPt.X() > o3tl::convert(nWidth, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.X() ) )
                {
                    bMoveRulerAction = false;
                    Invalidate(tools::Rectangle(0, 0, 10000, 10000));
                }
                else if (bLeftRulerChange
                         && (o3tl::convert(aButtonUpPt.X(), o3tl::Length::mm100, o3tl::Length::twip)
                             > nWidth - aLRItem.ResolveRight({})
                                   - o3tl::convert(aOffset.X(), o3tl::Length::mm100,
                                                   o3tl::Length::twip)))
                {
                    bMoveRulerAction = false;
                    Invalidate(tools::Rectangle(0, 0, 10000, 10000));
                }
                else if (bRightRulerChange
                         && (o3tl::convert(aButtonUpPt.X(), o3tl::Length::mm100, o3tl::Length::twip)
                             < aLRItem.ResolveLeft({})
                                   - o3tl::convert(aOffset.X(), o3tl::Length::mm100,
                                                   o3tl::Length::twip)))
                {
                    bMoveRulerAction = false;
                    Invalidate(tools::Rectangle(0, 0, 10000, 10000));
                }
                else if( aButtonDownPt.X() == aButtonUpPt.X() )
                {
                    bMoveRulerAction = false;
                    DrawInvert( aButtonUpPt.X(), PointerStyle::HSizeBar );
                }
                if( bMoveRulerAction )
                {
                    ScDocShellModificator aModificator( *pDocShell );
                    if( bLeftRulerChange && bLeftRulerMove )
                    {
                        aLRItem.SetLeft(SvxIndentValue::twips(
                            o3tl::convert(aButtonUpPt.X(), o3tl::Length::mm100, o3tl::Length::twip)
                            + o3tl::convert(aOffset.X(), o3tl::Length::mm100, o3tl::Length::twip)));
                        rStyleSet.Put(aLRItem);
                        pDocShell->SetModified();
                    }
                    else if( bRightRulerChange && bRightRulerMove )
                    {
                        aLRItem.SetRight(SvxIndentValue::twips(
                            nWidth
                            - o3tl::convert(aButtonUpPt.X(), o3tl::Length::mm100,
                                            o3tl::Length::twip)
                            - o3tl::convert(aOffset.X(), o3tl::Length::mm100, o3tl::Length::twip)));
                        rStyleSet.Put( aLRItem );
                        pDocShell->SetModified();
                    }

                    ScStyleSaveData aNewData;
                    aNewData.InitFromStyle( pStyleSheet );
                    if( bUndo )
                    {
                        pDocShell->GetUndoManager()->AddUndoAction(
                            std::make_unique<ScUndoModifyStyle>( *pDocShell, SfxStyleFamily::Page,
                            aOldData, aNewData ) );
                    }

                    if ( ValidTab( nTab ) )
                    {
                        ScPrintFunc aPrintFunc( GetOutDev(), *pDocShell, nTab );
                        aPrintFunc.UpdatePages();
                    }

                    tools::Rectangle aRect(0,0,10000,10000);
                    Invalidate(aRect);
                    aModificator.SetDocumentModified();
                    bLeftRulerChange = false;
                    bRightRulerChange = false;
                }
            }
            bLeftRulerMove = false;
            bRightRulerMove = false;
        }

        if( rMEvt.IsLeft() && GetPointer() == PointerStyle::VSizeBar )
        {
            SetPointer( PointerStyle::Arrow );

            bool bMoveRulerAction = true;
            if( ( bTopRulerChange || bBottomRulerChange || bHeaderRulerChange || bFooterRulerChange ) && ( aButtonUpPt.Y() <= ( 0 - aOffset.Y() ) || aButtonUpPt.Y() > o3tl::convert(nHeight, o3tl::Length::twip, o3tl::Length::mm100) -aOffset.Y() ) )
            {
                bMoveRulerAction = false;
                Invalidate(tools::Rectangle(0, 0, 10000, 10000));
            }
            else if( aButtonDownPt.Y() == aButtonUpPt.Y() )
            {
                bMoveRulerAction = false;
                DrawInvert( aButtonUpPt.Y(), PointerStyle::VSizeBar );
            }
            if( bMoveRulerAction )
            {
                ScDocument& rDoc = pDocShell->GetDocument();
                bool bUndo = rDoc.IsUndoEnabled();
                ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
                SfxStyleSheetBase* pStyleSheet = pStylePool->Find( rDoc.GetPageStyle( nTab ), SfxStyleFamily::Page );
                OSL_ENSURE( pStyleSheet, "PageStyle not found" );
                if ( pStyleSheet )
                {
                    ScDocShellModificator aModificator( *pDocShell );
                    ScStyleSaveData aOldData;
                    if( bUndo )
                        aOldData.InitFromStyle( pStyleSheet );

                    SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();

                    SvxULSpaceItem aULItem = rStyleSet.Get( ATTR_ULSPACE );

                    if( bTopRulerMove && bTopRulerChange )
                    {
                        aULItem.SetUpperValue(o3tl::convert(aButtonUpPt.Y(), o3tl::Length::mm100, o3tl::Length::twip) + o3tl::convert(aOffset.Y(), o3tl::Length::mm100, o3tl::Length::twip));
                        rStyleSet.Put( aULItem );
                        pDocShell->SetModified();
                    }
                    else if( bBottomRulerMove && bBottomRulerChange )
                    {
                        aULItem.SetLowerValue(nHeight - o3tl::convert(aButtonUpPt.Y(), o3tl::Length::mm100, o3tl::Length::twip) - o3tl::convert(aOffset.Y(), o3tl::Length::mm100, o3tl::Length::twip));
                        rStyleSet.Put( aULItem );
                        pDocShell->SetModified();
                    }
                    else if( bHeaderRulerMove && bHeaderRulerChange )
                    {
                        if ( const SvxSetItem* pSetItem = rStyleSet.GetItemIfSet( ATTR_PAGE_HEADERSET, false ) )
                        {
                            const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
                            Size  aHeaderSize = rHeaderSet.Get(ATTR_PAGE_SIZE).GetSize();
                            aHeaderSize.setHeight(o3tl::convert( aButtonUpPt.Y(), o3tl::Length::mm100, o3tl::Length::twip) + o3tl::convert(aOffset.Y(), o3tl::Length::mm100, o3tl::Length::twip) - aULItem.GetUpper());
                            aHeaderSize.setHeight( aHeaderSize.Height() * 100 / mnScale );
                            SvxSetItem  aNewHeader( rStyleSet.Get(ATTR_PAGE_HEADERSET) );
                            aNewHeader.GetItemSet().Put( SvxSizeItem( ATTR_PAGE_SIZE, aHeaderSize ) );
                            rStyleSet.Put( aNewHeader );
                            pDocShell->SetModified();
                        }
                    }
                    else if( bFooterRulerMove && bFooterRulerChange )
                    {
                        if( const SvxSetItem* pSetItem = rStyleSet.GetItemIfSet( ATTR_PAGE_FOOTERSET, false ) )
                        {
                            const SfxItemSet& rFooterSet = pSetItem->GetItemSet();
                            Size aFooterSize = rFooterSet.Get(ATTR_PAGE_SIZE).GetSize();
                            aFooterSize.setHeight(nHeight - o3tl::convert(aButtonUpPt.Y(), o3tl::Length::mm100, o3tl::Length::twip) - o3tl::convert(aOffset.Y(), o3tl::Length::mm100, o3tl::Length::twip) - aULItem.GetLower());
                            aFooterSize.setHeight( aFooterSize.Height() * 100 / mnScale );
                            SvxSetItem  aNewFooter( rStyleSet.Get(ATTR_PAGE_FOOTERSET) );
                            aNewFooter.GetItemSet().Put( SvxSizeItem( ATTR_PAGE_SIZE, aFooterSize ) );
                            rStyleSet.Put( aNewFooter );
                            pDocShell->SetModified();
                        }
                    }

                    ScStyleSaveData aNewData;
                    aNewData.InitFromStyle( pStyleSheet );
                    if( bUndo )
                    {
                        pDocShell->GetUndoManager()->AddUndoAction(
                            std::make_unique<ScUndoModifyStyle>( *pDocShell, SfxStyleFamily::Page,
                            aOldData, aNewData ) );
                    }

                    if ( ValidTab( nTab ) )
                    {
                        ScPrintFunc aPrintFunc( GetOutDev(), *pDocShell, nTab );
                        aPrintFunc.UpdatePages();
                    }

                    tools::Rectangle aRect(0, 0, 10000, 10000);
                    Invalidate(aRect);
                    aModificator.SetDocumentModified();
                    bTopRulerChange = false;
                    bBottomRulerChange = false;
                    bHeaderRulerChange = false;
                    bFooterRulerChange = false;
                }
            }
            bTopRulerMove = false;
            bBottomRulerMove = false;
            bHeaderRulerMove = false;
            bFooterRulerMove = false;
        }
        if( rMEvt.IsLeft() && GetPointer() == PointerStyle::HSplit )
        {
            SetPointer(PointerStyle::Arrow);
            ScDocument& rDoc = pDocShell->GetDocument();
            bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
            bool bMoveRulerAction = true;
            if( aButtonDownPt.X() == aButtonUpPt.X() )
            {
                bMoveRulerAction = false;
                if( nColNumberButtonDown == aPageArea.aStart.Col() )
                    DrawInvert( PixelToLogic( Point( nLeftPosition, 0 ),aMMMode ).X() ,PointerStyle::HSplit );
                else
                    DrawInvert( PixelToLogic( Point( mvRight[ nColNumberButtonDown-1 ], 0 ),aMMMode ).X() ,PointerStyle::HSplit );
                DrawInvert( aButtonUpPt.X(), PointerStyle::HSplit );
            }
            if( bMoveRulerAction )
            {
                tools::Long  nNewColWidth = 0;
                std::vector<sc::ColRowSpan> aCols(1, sc::ColRowSpan(nColNumberButtonDown,nColNumberButtonDown));

                constexpr auto md = o3tl::getConversionMulDiv(o3tl::Length::mm100, o3tl::Length::twip);
                const auto m = md.first * 100, d = md.second * mnScale;
                if( !bLayoutRTL )
                {
                    nNewColWidth = o3tl::convert(PixelToLogic( Point( rMEvt.GetPosPixel().X() - mvRight[ nColNumberButtonDown ], 0), aMMMode ).X(), m, d);
                    nNewColWidth += pDocShell->GetDocument().GetColWidth( nColNumberButtonDown, nTab );
                }
                else
                {

                    nNewColWidth = o3tl::convert(PixelToLogic( Point( mvRight[ nColNumberButtonDown ] - rMEvt.GetPosPixel().X(), 0), aMMMode ).X(), m, d);
                    nNewColWidth += pDocShell->GetDocument().GetColWidth( nColNumberButtonDown, nTab );
                }

                if( nNewColWidth >= 0 )
                {
                    pDocShell->GetDocFunc().SetWidthOrHeight(
                        true, aCols, nTab, SC_SIZE_DIRECT, static_cast<sal_uInt16>(nNewColWidth), true, true);
                    pDocShell->SetModified();
                }
                if ( ValidTab( nTab ) )
                {
                    ScPrintFunc aPrintFunc( GetOutDev(), *pDocShell, nTab );
                    aPrintFunc.UpdatePages();
                }
                tools::Rectangle aRect(0, 0, 10000, 10000);
                Invalidate(aRect);
            }
            bColRulerMove = false;
        }
        ReleaseMouse();
}

void ScPreview::MouseMove( const MouseEvent& rMEvt )
{
    Fraction aPreviewZoom( nZoom, 100 );
    Fraction aHorPrevZoom( static_cast<tools::Long>( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode  aMMMode( MapUnit::Map100thMM, Point(), aHorPrevZoom, aPreviewZoom );
    Point    aMouseMovePoint = PixelToLogic( rMEvt.GetPosPixel(), aMMMode );

    tools::Long    nLeftMargin = 0;
    tools::Long    nRightMargin = 0;
    tools::Long    nTopMargin = 0;
    tools::Long    nBottomMargin = 0;

    tools::Long    nWidth = lcl_GetDocPageSize(&pDocShell->GetDocument(), nTab).Width();
    tools::Long    nHeight = lcl_GetDocPageSize(&pDocShell->GetDocument(), nTab).Height();

    if ( nPageNo < nTotalPages )
    {
        ScPrintOptions aOptions = ScModule::get()->GetPrintOptions();

        std::unique_ptr<ScPrintFunc, o3tl::default_delete<ScPrintFunc>> pPrintFunc;
        if (bStateValid)
            pPrintFunc.reset(new ScPrintFunc( GetOutDev(), *pDocShell, aState, &aOptions ));
        else
            pPrintFunc.reset(new ScPrintFunc( GetOutDev(), *pDocShell, nTab, nFirstAttr[nTab], nTotalPages, nullptr, &aOptions ));

        nLeftMargin = o3tl::convert(pPrintFunc->GetLeftMargin(), o3tl::Length::twip, o3tl::Length::mm100) - aOffset.X();
        nRightMargin = o3tl::convert(pPrintFunc->GetRightMargin(), o3tl::Length::twip, o3tl::Length::mm100);
        nRightMargin = o3tl::convert(nWidth, o3tl::Length::twip, o3tl::Length::mm100) - nRightMargin - aOffset.X();
        nTopMargin = o3tl::convert(pPrintFunc->GetTopMargin(), o3tl::Length::twip, o3tl::Length::mm100) - aOffset.Y();
        nBottomMargin = o3tl::convert(pPrintFunc->GetBottomMargin(), o3tl::Length::twip, o3tl::Length::mm100);
        nBottomMargin = o3tl::convert(nHeight, o3tl::Length::twip, o3tl::Length::mm100) - nBottomMargin - aOffset.Y();
        if( mnScale > 0 )
        {
            constexpr auto md = o3tl::getConversionMulDiv(o3tl::Length::twip, o3tl::Length::mm100);
            const auto m = md.first * mnScale, d = md.second * 100;
            nHeaderHeight = nTopMargin + o3tl::convert(pPrintFunc->GetHeader().nHeight, m, d);
            nFooterHeight = nBottomMargin - o3tl::convert(pPrintFunc->GetFooter().nHeight, m, d);
        }
        else
        {
            nHeaderHeight = nTopMargin + o3tl::convert(pPrintFunc->GetHeader().nHeight, o3tl::Length::twip, o3tl::Length::mm100);
            nFooterHeight = nBottomMargin - o3tl::convert(pPrintFunc->GetFooter().nHeight, o3tl::Length::twip, o3tl::Length::mm100);
        }
    }

    Point   aPixPt( rMEvt.GetPosPixel() );
    Point   aLeftTop = LogicToPixel( Point( nLeftMargin, -aOffset.Y() ) , aMMMode );
    Point   aLeftBottom = LogicToPixel( Point( nLeftMargin, o3tl::convert(nHeight, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.Y()), aMMMode );
    Point   aRightTop = LogicToPixel( Point( nRightMargin, -aOffset.Y() ), aMMMode );
    Point   aTopLeft = LogicToPixel( Point( -aOffset.X(), nTopMargin ), aMMMode );
    Point   aTopRight = LogicToPixel( Point( o3tl::convert(nWidth, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.X(), nTopMargin ), aMMMode );
    Point   aBottomLeft = LogicToPixel( Point( -aOffset.X(), nBottomMargin ), aMMMode );
    Point   aHeaderLeft = LogicToPixel( Point(  -aOffset.X(), nHeaderHeight ), aMMMode );
    Point   aFooderLeft = LogicToPixel( Point( -aOffset.X(), nFooterHeight ), aMMMode );

    bool bOnColRulerChange = false;

    for( SCCOL i=aPageArea.aStart.Col(); i<= aPageArea.aEnd.Col(); i++ )
    {
        Point   aColumnTop = LogicToPixel( Point( 0, -aOffset.Y() ) ,aMMMode );
        Point   aColumnBottom = LogicToPixel( Point( 0, o3tl::convert(nHeight, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.Y()), aMMMode );
        tools::Long nRight = i < static_cast<SCCOL>(mvRight.size()) ? mvRight[i] : 0;
        if( aPixPt.X() < ( nRight + 2 ) && ( aPixPt.X() > ( nRight - 2 ) ) && ( aPixPt.X() < aRightTop.X() ) && ( aPixPt.X() > aLeftTop.X() )
            && ( aPixPt.Y() > aColumnTop.Y() ) && ( aPixPt.Y() < aColumnBottom.Y() ) && !bLeftRulerMove && !bRightRulerMove
            && !bTopRulerMove && !bBottomRulerMove && !bHeaderRulerMove && !bFooterRulerMove )
        {
            bOnColRulerChange = true;
            if( !rMEvt.GetButtons() && GetPointer() == PointerStyle::HSplit )
                nColNumberButtonDown = i;
            break;
        }
    }

    if( aPixPt.X() < ( aLeftTop.X() + 2 ) && aPixPt.X() > ( aLeftTop.X() - 2 ) && !bRightRulerMove )
    {
        bLeftRulerChange = true;
        bRightRulerChange = false;
    }
    else if( aPixPt.X() < ( aRightTop.X() + 2 ) && aPixPt.X() > ( aRightTop.X() - 2 ) && !bLeftRulerMove )
    {
        bLeftRulerChange = false;
        bRightRulerChange = true;
    }
    else if( aPixPt.Y() < ( aTopLeft.Y() + 2 ) && aPixPt.Y() > ( aTopLeft.Y() - 2 ) && !bBottomRulerMove && !bHeaderRulerMove && !bFooterRulerMove )
    {
        bTopRulerChange = true;
        bBottomRulerChange = false;
        bHeaderRulerChange = false;
        bFooterRulerChange = false;
    }
    else if( aPixPt.Y() < ( aBottomLeft.Y() + 2 ) && aPixPt.Y() > ( aBottomLeft.Y() - 2 ) && !bTopRulerMove && !bHeaderRulerMove && !bFooterRulerMove )
    {
        bTopRulerChange = false;
        bBottomRulerChange = true;
        bHeaderRulerChange = false;
        bFooterRulerChange = false;
    }
    else if( aPixPt.Y() < ( aHeaderLeft.Y() + 2 ) && aPixPt.Y() > ( aHeaderLeft.Y() - 2 ) && !bTopRulerMove && !bBottomRulerMove && !bFooterRulerMove )
    {
        bTopRulerChange = false;
        bBottomRulerChange = false;
        bHeaderRulerChange = true;
        bFooterRulerChange = false;
    }
    else if( aPixPt.Y() < ( aFooderLeft.Y() + 2 ) && aPixPt.Y() > ( aFooderLeft.Y() - 2 ) && !bTopRulerMove && !bBottomRulerMove && !bHeaderRulerMove )
    {
        bTopRulerChange = false;
        bBottomRulerChange = false;
        bHeaderRulerChange = false;
        bFooterRulerChange = true;
    }

    if( !bPageMargin )
        return;

    if(( (aPixPt.X() < ( aLeftTop.X() + 2 ) && aPixPt.X() > ( aLeftTop.X() - 2 )) || bLeftRulerMove ||
        ( aPixPt.X() < ( aRightTop.X() + 2 ) && aPixPt.X() > ( aRightTop.X() - 2 ) ) || bRightRulerMove || bOnColRulerChange || bColRulerMove )
        && aPixPt.Y() > aLeftTop.Y() && aPixPt.Y() < aLeftBottom.Y() )
    {
        if( bOnColRulerChange || bColRulerMove )
        {
            SetPointer( PointerStyle::HSplit );
            if( bColRulerMove )
            {
                if( aMouseMovePoint.X() > -aOffset.X() && aMouseMovePoint.X() < o3tl::convert(nWidth, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.X() )
                   DragMove( aMouseMovePoint.X(), PointerStyle::HSplit );
            }
        }
        else
        {
            if( bLeftRulerChange && !bTopRulerMove && !bBottomRulerMove && !bHeaderRulerMove && !bFooterRulerMove )
            {
                SetPointer( PointerStyle::HSizeBar );
                if( bLeftRulerMove )
                {
                   if( aMouseMovePoint.X() > -aOffset.X() && aMouseMovePoint.X() < o3tl::convert(nWidth, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.X() )
                       DragMove( aMouseMovePoint.X(), PointerStyle::HSizeBar );
                }
            }
            else if( bRightRulerChange && !bTopRulerMove && !bBottomRulerMove && !bHeaderRulerMove && !bFooterRulerMove )
            {
                SetPointer( PointerStyle::HSizeBar );
                if( bRightRulerMove )
                {
                   if( aMouseMovePoint.X() > -aOffset.X() && aMouseMovePoint.X() < o3tl::convert(nWidth, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.X() )
                       DragMove( aMouseMovePoint.X(), PointerStyle::HSizeBar );
                }
            }
        }
    }
    else
    {
        if( ( ( aPixPt.Y() < ( aTopLeft.Y() + 2 ) && aPixPt.Y() > ( aTopLeft.Y() - 2 ) ) || bTopRulerMove ||
            ( aPixPt.Y() < ( aBottomLeft.Y() + 2 ) && aPixPt.Y() > ( aBottomLeft.Y() - 2 ) ) || bBottomRulerMove ||
            ( aPixPt.Y() < ( aHeaderLeft.Y() + 2 ) && aPixPt.Y() > ( aHeaderLeft.Y() - 2 ) ) || bHeaderRulerMove ||
            ( aPixPt.Y() < ( aFooderLeft.Y() + 2 ) && aPixPt.Y() > ( aFooderLeft.Y() - 2 ) ) || bFooterRulerMove )
            && aPixPt.X() > aTopLeft.X() && aPixPt.X() < aTopRight.X() )
        {
            if( bTopRulerChange )
            {
                SetPointer( PointerStyle::VSizeBar );
                if( bTopRulerMove )
                {
                    if( aMouseMovePoint.Y() > -aOffset.Y() && aMouseMovePoint.Y() < o3tl::convert(nHeight, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.Y() )
                        DragMove( aMouseMovePoint.Y(), PointerStyle::VSizeBar );
                }
            }
            else if( bBottomRulerChange )
            {
                SetPointer( PointerStyle::VSizeBar );
                if( bBottomRulerMove )
                {
                    if( aMouseMovePoint.Y() > -aOffset.Y() && aMouseMovePoint.Y() < o3tl::convert(nHeight, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.Y() )
                        DragMove( aMouseMovePoint.Y(), PointerStyle::VSizeBar );
                }
            }
            else if( bHeaderRulerChange )
            {
                SetPointer( PointerStyle::VSizeBar );
                if( bHeaderRulerMove )
                {
                    if( aMouseMovePoint.Y() > -aOffset.Y() && aMouseMovePoint.Y() < o3tl::convert(nHeight, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.Y() )
                        DragMove( aMouseMovePoint.Y(), PointerStyle::VSizeBar );
                }
            }
            else if( bFooterRulerChange )
            {
                SetPointer( PointerStyle::VSizeBar );
                if( bFooterRulerMove )
                {
                    if( aMouseMovePoint.Y() > -aOffset.Y() && aMouseMovePoint.Y() < o3tl::convert(nHeight, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.Y() )
                        DragMove( aMouseMovePoint.Y(), PointerStyle::VSizeBar );
                }
            }
        }
        else
            SetPointer( PointerStyle::Arrow );
    }
}

void ScPreview::InvalidateLocationData(SfxHintId nId)
{
    bLocationValid = false;
    if (pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility( SfxHint( nId ) );
}

void ScPreview::GetFocus()
{
    Window::GetFocus();
    if (pViewShell && pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility( ScAccWinFocusGotHint() );
}

void ScPreview::LoseFocus()
{
    if (pViewShell && pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility( ScAccWinFocusLostHint() );
    Window::LoseFocus();
}

rtl::Reference<comphelper::OAccessible> ScPreview::CreateAccessible()
{
    rtl::Reference<ScAccessibleDocumentPagePreview> pAccessible =
        new ScAccessibleDocumentPagePreview(GetAccessibleParent(), pViewShell );
    SetAccessible(pAccessible);
    pAccessible->Init();
    return pAccessible;
}

void ScPreview::DragMove( tools::Long nDragMovePos, PointerStyle nFlags )
{
    Fraction aPreviewZoom( nZoom, 100 );
    Fraction aHorPrevZoom( static_cast<tools::Long>( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode  aMMMode( MapUnit::Map100thMM, Point(), aHorPrevZoom, aPreviewZoom );
    SetMapMode( aMMMode );
    tools::Long  nPos = nDragMovePos;
    if( nFlags == PointerStyle::HSizeBar || nFlags == PointerStyle::HSplit )
    {
        if( nDragMovePos != aButtonDownChangePoint.X() )
        {
            DrawInvert( aButtonDownChangePoint.X(), nFlags );
            aButtonDownChangePoint.setX( nPos );
            DrawInvert( aButtonDownChangePoint.X(), nFlags );
        }
    }
    else if( nFlags == PointerStyle::VSizeBar )
    {
        if( nDragMovePos != aButtonDownChangePoint.Y() )
        {
            DrawInvert( aButtonDownChangePoint.Y(), nFlags );
            aButtonDownChangePoint.setY( nPos );
            DrawInvert( aButtonDownChangePoint.Y(), nFlags );
        }
    }
}

void ScPreview::DrawInvert( tools::Long nDragPos, PointerStyle nFlags )
{
    tools::Long  nHeight = lcl_GetDocPageSize( &pDocShell->GetDocument(), nTab ).Height();
    tools::Long  nWidth = lcl_GetDocPageSize( &pDocShell->GetDocument(), nTab ).Width();
    if( nFlags == PointerStyle::HSizeBar || nFlags == PointerStyle::HSplit )
    {
        tools::Rectangle aRect( nDragPos, -aOffset.Y(), nDragPos + 1, o3tl::convert(nHeight, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.Y());
        GetOutDev()->Invert( aRect, InvertFlags::N50 );
    }
    else if( nFlags == PointerStyle::VSizeBar )
    {
        tools::Rectangle aRect( -aOffset.X(), nDragPos, o3tl::convert(nWidth, o3tl::Length::twip, o3tl::Length::mm100) - aOffset.X(), nDragPos + 1 );
        GetOutDev()->Invert( aRect, InvertFlags::N50 );
    }
}

void ScPreview::SetSelectedTabs(const ScMarkData& rMark)
{
    maSelectedTabs = rMark.GetSelectedTabs();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
