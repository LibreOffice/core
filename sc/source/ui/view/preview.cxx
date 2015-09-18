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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svtools/colorcfg.hxx>
#include <svx/fmview.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svl/itemset.hxx>
#include <tools/multisel.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>

#include "preview.hxx"
#include "prevwsh.hxx"
#include "prevloc.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "printfun.hxx"
#include "printopt.hxx"
#include "stlpool.hxx"
#include "undostyl.hxx"
#include "drwlayer.hxx"
#include "scmod.hxx"
#include "markdata.hxx"
#include "globstr.hrc"
#include "sc.hrc"
#include "AccessibleDocumentPagePreview.hxx"
#include <vcl/lineinfo.hxx>
#include <svx/algitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include "attrib.hxx"
#include "pagepar.hxx"
#include <com/sun/star/accessibility/XAccessible.hpp>
#include "AccessibilityHints.hxx"
#include <vcl/svapp.hxx>
#include "viewutil.hxx"
#include <columnspanset.hxx>
#include <docpool.hxx>
#include <patattr.hxx>

#include <memory>

// STATIC DATA -----------------------------------------------------------

#define SC_PREVIEW_SHADOWSIZE   2

static long lcl_GetDisplayStart( SCTAB nTab, ScDocument* pDoc, std::vector<long>& nPages )
{
    long nDisplayStart = 0;
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
    nPages(),
    nFirstAttr(),
    nTab( 0 ),
    nTabPage( 0 ),
    nTabStart( 0 ),
    nDisplayStart( 0 ),
    aDate( Date::SYSTEM ),
    aTime( tools::Time::SYSTEM ),
    nTotalPages( 0 ),
    pLocationData( NULL ),
    pDrawView( NULL ),
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
    nColNumberButttonDown( 0 ),
    nHeaderHeight ( 0 ),
    nFooterHeight ( 0 )
{
    SetOutDevViewType( OUTDEV_VIEWTYPE_PRINTPREVIEW ); //#106611#
    SetBackground();

    SetHelpId( HID_SC_WIN_PREVIEW );
    SetUniqueId( HID_SC_WIN_PREVIEW );

    SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );

    for (SCCOL i=0; i<=MAXCOL; i++)
        nRight[i] = 0;                  // initialized with actual positions when markers are drawn
}

ScPreview::~ScPreview()
{
    disposeOnce();
}

void ScPreview::dispose()
{
    delete pDrawView;
    delete pLocationData;
    vcl::Window::dispose();
}

void ScPreview::UpdateDrawView()        // nTab must be right
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScDrawLayer* pModel = rDoc.GetDrawLayer();     // is not 0

    // #114135#
    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(nTab);
        if ( pDrawView && ( !pDrawView->GetSdrPageView() || pDrawView->GetSdrPageView()->GetPage() != pPage ) )
        {
            // convert the displayed Page of drawView (see below) does not work?!?
            delete pDrawView;
            pDrawView = NULL;
        }

        if ( !pDrawView )                                   // New Drawing?
        {
            pDrawView = new FmFormView( pModel, this );

            // The DrawView takes over the Design-Mode from the Model
            // (Settings "In opening Draftmode"), therefore to restore here
            pDrawView->SetDesignMode();
            pDrawView->SetPrintPreview();
            pDrawView->ShowSdrPage(pPage);
        }
    }
    else if ( pDrawView )
    {
        delete pDrawView;           // for this Chart is not needed
        pDrawView = NULL;
    }
}

void ScPreview::TestLastPage()
{
    if (nPageNo >= nTotalPages)
    {
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
            aState.nPrintTab = 0;
                        aState.nStartCol = aState.nEndCol = 0;
                        aState.nStartRow = aState.nEndRow = 0;
                        aState.nZoom = 0;
            aState.nPagesX = aState.nPagesY = 0;
            aState.nTabPages = aState.nTotalPages =
            aState.nPageStart = aState.nDocPages = 0;
        }
    }
}

void ScPreview::CalcPages()
{
    WaitObject aWait( this );

    ScDocument& rDoc = pDocShell->GetDocument();
    nTabCount = rDoc.GetTableCount();

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
    ScPrintOptions aOptions = SC_MOD()->GetPrintOptions();

    while (nStart > static_cast<SCTAB>(nPages.size()))
        nPages.push_back(0);
    while (nStart > static_cast<SCTAB>(nFirstAttr.size()))
        nFirstAttr.push_back(0);

    for (SCTAB i=nStart; i<nTabCount; i++)
    {
        if ( i == static_cast<SCTAB>(nPages.size()))
            nPages.push_back(0);
        if ( i == static_cast<SCTAB>(nFirstAttr.size()))
            nFirstAttr.push_back(0);
        if (!aOptions.GetAllSheets() && maSelectedTabs.count(i) == 0)
        {
            nPages[i] = 0;
            nFirstAttr[i] = 0;
            continue;
        }

        long nAttrPage = i > 0 ? nFirstAttr[i-1] : 1;

        long nThisStart = nTotalPages;
        ScPrintFunc aPrintFunc( this, pDocShell, i, nAttrPage, 0, NULL, &aOptions );
        long nThisTab = aPrintFunc.GetTotalPages();
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
            aPageSize = aPrintFunc.GetPageSize();
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
        long nPartPages = 0;
        for (SCTAB i=0; i<nTabsTested && nTab < static_cast<SCTAB>(nPages.size()); i++)
        {
            long nThisStart = nPartPages;
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
        UpdateDrawView();       // Spreedsheet eventually changes
    }

    Fraction aPreviewZoom( nZoom, 100 );
    Fraction aHorPrevZoom( (long)( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode aMMMode( MAP_100TH_MM, Point(), aHorPrevZoom, aPreviewZoom );

    bool bDoPrint = ( pFillLocation == NULL );
    bool bValidPage = ( nPageNo < nTotalPages );

    ScModule* pScMod = SC_MOD();
    const svtools::ColorConfig& rColorCfg = pScMod->GetColorConfig();
    Color aBackColor( rColorCfg.GetColorValue(svtools::APPBACKGROUND).nColor );

    if ( bDoPrint && ( aOffset.X() < 0 || aOffset.Y() < 0 ) && bValidPage )
    {
        SetMapMode( aMMMode );
        SetLineColor();
        SetFillColor(aBackColor);

        Size aWinSize = GetOutputSize();
        if ( aOffset.X() < 0 )
            DrawRect(Rectangle( 0, 0, -aOffset.X(), aWinSize.Height() ));
        if ( aOffset.Y() < 0 )
            DrawRect(Rectangle( 0, 0, aWinSize.Width(), -aOffset.Y() ));
    }

    long   nLeftMargin = 0;
    long   nRightMargin = 0;
    long   nTopMargin = 0;
    long   nBottomMargin = 0;
    bool   bHeaderOn = false;
    bool   bFooterOn = false;

    ScDocument& rDoc = pDocShell->GetDocument();
    bool   bLayoutRTL = rDoc.IsLayoutRTL( nTab );

    Size aLocalPageSize;
    if ( bValidPage )
    {
        ScPrintOptions aOptions = pScMod->GetPrintOptions();

        ScPrintFunc* pPrintFunc;
        if (bStateValid)
            pPrintFunc = new ScPrintFunc( this, pDocShell, aState, &aOptions );
        else
            pPrintFunc = new ScPrintFunc( this, pDocShell, nTab, nFirstAttr[nTab], nTotalPages, NULL, &aOptions );

        pPrintFunc->SetOffset(aOffset);
        pPrintFunc->SetManualZoom(nZoom);
        pPrintFunc->SetDateTime(aDate,aTime);
        pPrintFunc->SetClearFlag(true);
        pPrintFunc->SetUseStyleColor( pScMod->GetAccessOptions().GetIsForPagePreviews() );

        pPrintFunc->SetDrawView( pDrawView );

        // MultiSelection for the one Page must produce something inconvenient
        Range aPageRange( nPageNo+1, nPageNo+1 );
        MultiSelection aPage( aPageRange );
        aPage.SetTotalRange( Range(0,RANGE_MAX) );
        aPage.Select( aPageRange );

        long nPrinted = pPrintFunc->DoPrint( aPage, nTabStart, nDisplayStart, bDoPrint, pFillLocation );
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
            Rectangle aPixRect;
            Rectangle aRectCellPosition;
            Rectangle aRectPosition;
            pLocationData->GetMainCellRange( aPageArea, aPixRect );
            if( !bLayoutRTL )
            {
                pLocationData->GetCellPosition( aPageArea.aStart, aRectPosition );
                nLeftPosition = aRectPosition.Left();
                for( SCCOL i = aPageArea.aStart.Col(); i <= aPageArea.aEnd.Col(); i++ )
                {
                    pLocationData->GetCellPosition( ScAddress( i,aPageArea.aStart.Row(),aPageArea.aStart.Tab()),aRectCellPosition );
                    nRight[i] = aRectCellPosition.Right();
                }
            }
            else
            {
                pLocationData->GetCellPosition( aPageArea.aEnd, aRectPosition );
                nLeftPosition = aRectPosition.Right()+1;

                pLocationData->GetCellPosition( aPageArea.aStart,aRectCellPosition );
                nRight[ aPageArea.aEnd.Col() ] = aRectCellPosition.Left();
                for( SCCOL i = aPageArea.aEnd.Col(); i > aPageArea.aStart.Col(); i-- )
                {
                    pLocationData->GetCellPosition( ScAddress( i,aPageArea.aEnd.Row(),aPageArea.aEnd.Tab()),aRectCellPosition );
                    nRight[ i-1 ] = nRight[ i ] + aRectCellPosition.Right() - aRectCellPosition.Left() + 1;
                }
            }
        }

        if (nPrinted)   // if not, draw everything grey
        {
            aLocalPageSize = pPrintFunc->GetPageSize();
            aLocalPageSize.Width()  = (long) (aLocalPageSize.Width()  * HMM_PER_TWIPS );
            aLocalPageSize.Height() = (long) (aLocalPageSize.Height() * HMM_PER_TWIPS );

            nLeftMargin = (long) ( nLeftMargin * HMM_PER_TWIPS );
            nRightMargin = (long) ( nRightMargin * HMM_PER_TWIPS );
            nTopMargin = (long) ( nTopMargin * HMM_PER_TWIPS );
            nBottomMargin = (long) ( nBottomMargin * HMM_PER_TWIPS );
            nHeaderHeight = (long) ( nHeaderHeight * HMM_PER_TWIPS * mnScale / 100 + nTopMargin );
            nFooterHeight = (long) ( nFooterHeight * HMM_PER_TWIPS * mnScale / 100 + nBottomMargin );
        }

        if (!bStateValid)
        {
            pPrintFunc->GetPrintState( aState );
            aState.nDocPages = nTotalPages;
            bStateValid = true;
        }
        delete pPrintFunc;
    }

    if ( bDoPrint )
    {
        long nPageEndX = aLocalPageSize.Width()  - aOffset.X();
        long nPageEndY = aLocalPageSize.Height() - aOffset.Y();
        if ( !bValidPage )
            nPageEndX = nPageEndY = 0;

        Size aWinSize = GetOutputSize();
        Point aWinEnd( aWinSize.Width(), aWinSize.Height() );
        bool bRight  = nPageEndX <= aWinEnd.X();
        bool bBottom = nPageEndY <= aWinEnd.Y();

        if (!nTotalPages)
        {
            // There is no data to print. Print a friendly warning message and
            // bail out.

            SetMapMode(aMMMode);

            // Draw background first.
            SetLineColor();
            SetFillColor(aBackColor);
            DrawRect(Rectangle(0, 0, aWinEnd.X(), aWinEnd.Y()));

            const ScPatternAttr& rDefPattern =
                static_cast<const ScPatternAttr&>(
                    rDoc.GetPool()->GetDefaultItem(ATTR_PATTERN));

            std::unique_ptr<ScEditEngineDefaulter> pEditEng(
                new ScEditEngineDefaulter(EditEngine::CreatePool(), true));

            pEditEng->SetRefMapMode(aMMMode);
            SfxItemSet* pEditDefaults = new SfxItemSet( pEditEng->GetEmptyItemSet() );
            rDefPattern.FillEditItemSet(pEditDefaults);
            pEditEng->SetDefaults(pEditDefaults);

            Color aTextColor(COL_LIGHTGRAY);
            pEditDefaults->Put(SvxColorItem(aTextColor, EE_CHAR_COLOR));

            OUString aEmptyMsg;
            if (mbHasEmptyRangeTable)
                aEmptyMsg = ScGlobal::GetRscString(STR_PRINT_PREVIEW_EMPTY_RANGE);
            else
                aEmptyMsg = ScGlobal::GetRscString(STR_PRINT_PREVIEW_NODATA);

            long nHeight = 3000;
            pEditEng->SetDefaultItem(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT));
            pEditEng->SetDefaultItem(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
            pEditEng->SetDefaultItem(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT_CTL));

            pEditEng->SetText(aEmptyMsg);

            Point aCenter(
                (aWinEnd.X() - pEditEng->CalcTextWidth())/2,
                (aWinEnd.Y() - pEditEng->GetTextHeight())/2);

            pEditEng->Draw(this, aCenter);

            return;
        }

        if( bPageMargin && bValidPage )
        {
            SetMapMode(aMMMode);
            SetLineColor( COL_BLACK );
            DrawInvert( (long)( nTopMargin - aOffset.Y() ), PointerStyle::VSizeBar );
            DrawInvert( (long)(nPageEndY - nBottomMargin ), PointerStyle::VSizeBar );
            DrawInvert( (long)( nLeftMargin - aOffset.X() ), PointerStyle::HSizeBar );
            DrawInvert( (long)( nPageEndX - nRightMargin ) , PointerStyle::HSizeBar );
            if( bHeaderOn )
            {
                DrawInvert( nHeaderHeight - aOffset.Y(), PointerStyle::VSizeBar );
            }
            if( bFooterOn )
            {
                DrawInvert( nPageEndY - nFooterHeight, PointerStyle::VSizeBar );
            }

            SetMapMode( MapMode( MAP_PIXEL ) );
            for( int i= aPageArea.aStart.Col(); i<= aPageArea.aEnd.Col(); i++ )
            {
                Point aColumnTop = LogicToPixel( Point( 0, -aOffset.Y() ) ,aMMMode );
                SetLineColor( COL_BLACK );
                SetFillColor( COL_BLACK );
                DrawRect( Rectangle( Point( nRight[i] - 2, aColumnTop.Y() ),Point( nRight[i] + 2 , 4 + aColumnTop.Y()) ));
                DrawLine( Point( nRight[i], aColumnTop.Y() ), Point( nRight[i],  10 + aColumnTop.Y()) );
            }
            SetMapMode( aMMMode );
        }

        if (bRight || bBottom)
        {
            SetMapMode(aMMMode);
            SetLineColor();
            SetFillColor(aBackColor);
            if (bRight)
                DrawRect(Rectangle(nPageEndX,0, aWinEnd.X(),aWinEnd.Y()));
            if (bBottom)
            {
                if (bRight)
                    DrawRect(Rectangle(0,nPageEndY, nPageEndX,aWinEnd.Y()));    // Corner not duplicated
                else
                    DrawRect(Rectangle(0,nPageEndY, aWinEnd.X(),aWinEnd.Y()));
            }
        }

        if ( bValidPage )
        {
            Color aBorderColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );

            //  draw border

            if ( aOffset.X() <= 0 || aOffset.Y() <= 0 || bRight || bBottom )
            {
                SetLineColor( aBorderColor );
                SetFillColor();

                Rectangle aPixel( LogicToPixel( Rectangle( -aOffset.X(), -aOffset.Y(), nPageEndX, nPageEndY ) ) );
                --aPixel.Right();
                --aPixel.Bottom();
                DrawRect( PixelToLogic( aPixel ) );
            }

            //  draw shadow

            SetLineColor();
            SetFillColor( aBorderColor );

            Rectangle aPixel;

            aPixel = LogicToPixel( Rectangle( nPageEndX, -aOffset.Y(), nPageEndX, nPageEndY ) );
            aPixel.Top() += SC_PREVIEW_SHADOWSIZE;
            aPixel.Right() += SC_PREVIEW_SHADOWSIZE - 1;
            aPixel.Bottom() += SC_PREVIEW_SHADOWSIZE - 1;
            DrawRect( PixelToLogic( aPixel ) );

            aPixel = LogicToPixel( Rectangle( -aOffset.X(), nPageEndY, nPageEndX, nPageEndY ) );
            aPixel.Left() += SC_PREVIEW_SHADOWSIZE;
            aPixel.Right() += SC_PREVIEW_SHADOWSIZE - 1;
            aPixel.Bottom() += SC_PREVIEW_SHADOWSIZE - 1;
            DrawRect( PixelToLogic( aPixel ) );
        }
    }
}

void ScPreview::Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& /* rRect */ )
{
    bool bWasInPaint = bInPaint;        // nested calls shouldn't be necessary, but allow for now
    bInPaint = true;

    if (bPageMargin)
        GetLocationData();              // fill location data for column positions
    DoPrint( NULL );
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
            case KEY_ADD:      nSlot = SID_PREVIEW_ZOOMIN;  break;
            case KEY_ESCAPE:   nSlot = ScViewUtil::IsFullScreen( *pViewShell ) ? SID_CANCEL : SID_PREVIEW_CLOSE; break;
            case KEY_SUBTRACT: nSlot = SID_PREVIEW_ZOOMOUT; break;
        }
        if(nSlot)
        {
            bHandled = true;
            pViewShell->GetViewFrame()->GetDispatcher()->Execute( nSlot, SfxCallMode::ASYNCHRON );
        }
    }

    if ( !bHandled && !pViewShell->KeyInput(rKEvt) )
        Window::KeyInput(rKEvt);
}

const ScPreviewLocationData& ScPreview::GetLocationData()
{
    if ( !pLocationData )
    {
        pLocationData = new ScPreviewLocationData( &pDocShell->GetDocument(), this );
        bLocationValid = false;
    }
    if ( !bLocationValid )
    {
        pLocationData->Clear();
        DoPrint( pLocationData );
        bLocationValid = true;
    }
    return *pLocationData;
}

void ScPreview::DataChanged(bool bNewTime)
{
    if (bNewTime)
    {
        aDate = Date( Date::SYSTEM );
        aTime = tools::Time( tools::Time::SYSTEM );
    }

    bValid = false;
    InvalidateLocationData( SC_HINT_DATACHANGED );
    Invalidate();
}

OUString ScPreview::GetPosString()
{
    if (!bValid)
    {
        CalcPages();
        UpdateDrawView();       // The table eventually changes
    }

    OUString aString = ScGlobal::GetRscString( STR_PAGE ) +
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
    if (nNewZoom != nZoom)
    {
        nZoom = nNewZoom;

        //  apply new MapMode and call UpdateScrollBars to update aOffset

        Fraction aPreviewZoom( nZoom, 100 );
        Fraction aHorPrevZoom( (long)( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
        MapMode aMMMode( MAP_100TH_MM, Point(), aHorPrevZoom, aPreviewZoom );
        SetMapMode( aMMMode );

        bInSetZoom = true;              // don't scroll during SetYOffset in UpdateScrollBars
        pViewShell->UpdateNeededScrollBars(true);
        bInSetZoom = false;

        bStateValid = false;
        InvalidateLocationData( SC_HINT_ACC_VISAREACHANGED );
        DoInvalidate();
        Invalidate();
    }
}

void ScPreview::SetPageNo( long nPage )
{
    nPageNo = nPage;
    RecalcPages();
    UpdateDrawView();       // The table eventually changes
    InvalidateLocationData( SC_HINT_DATACHANGED );
    Invalidate();
}

long ScPreview::GetFirstPage(SCTAB nTabP)
{
    SCTAB nDocTabCount = pDocShell->GetDocument().GetTableCount();
    if (nTabP >= nDocTabCount)
        nTabP = nDocTabCount-1;

    long nPage = 0;
    if (nTabP>0)
    {
        CalcPages();
        if (nTabP >= static_cast<SCTAB>(nPages.size()) )
            OSL_FAIL("nPages out ouf bounds, FIX IT");
        UpdateDrawView();       // The table eventually changes

        for (SCTAB i=0; i<nTabP; i++)
            nPage += nPages[i];

        // An empty Table on the previous Page

        if ( nPages[nTabP]==0 && nPage > 0 )
            --nPage;
    }

    return nPage;
}

static Size lcl_GetDocPageSize( ScDocument* pDoc, SCTAB nTab )
{
    OUString aName = pDoc->GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aName, SFX_STYLE_FAMILY_PAGE );
    if ( pStyleSheet )
    {
        SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
        return static_cast<const SvxSizeItem&>( rStyleSet.Get(ATTR_PAGE_SIZE)).GetSize();
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
    Size aMarginSize( LogicToPixel( Size( 100, 100 ), MAP_100TH_MM ) );
    aWinSize.Width()  -= 2 * aMarginSize.Width();
    aWinSize.Height() -= 2 * aMarginSize.Height();

    Size aLocalPageSize = lcl_GetDocPageSize( &pDocShell->GetDocument(), nTab );
    if ( aLocalPageSize.Width() && aLocalPageSize.Height() )
    {
        long nZoomX = (long) ( aWinSize.Width() * 100  / ( aLocalPageSize.Width() * nWinScaleX ));
        long nZoomY = (long) ( aWinSize.Height() * 100 / ( aLocalPageSize.Height() * nWinScaleY ));

        long nOptimal = nZoomX;
        if (!bWidthOnly && nZoomY<nOptimal)
            nOptimal = nZoomY;

        if (nOptimal<20)
            nOptimal = 20;
        if (nOptimal>400)
            nOptimal = 400;

        return (sal_uInt16) nOptimal;
    }
    else
        return nZoom;
}

void ScPreview::SetXOffset( long nX )
{
    if ( aOffset.X() == nX )
        return;

    if (bValid)
    {
        long nDif = LogicToPixel(aOffset).X() - LogicToPixel(Point(nX,0)).X();
        aOffset.X() = nX;
        if (nDif && !bInSetZoom)
        {
            MapMode aOldMode = GetMapMode(); SetMapMode(MAP_PIXEL);
            Scroll( nDif, 0 );
            SetMapMode(aOldMode);
        }
    }
    else
    {
        aOffset.X() = nX;
        if (!bInSetZoom)
            Invalidate();
    }
    InvalidateLocationData( SC_HINT_ACC_VISAREACHANGED );
    Invalidate();
}

void ScPreview::SetYOffset( long nY )
{
    if ( aOffset.Y() == nY )
        return;

    if (bValid)
    {
        long nDif = LogicToPixel(aOffset).Y() - LogicToPixel(Point(0,nY)).Y();
        aOffset.Y() = nY;
        if (nDif && !bInSetZoom)
        {
            MapMode aOldMode = GetMapMode(); SetMapMode(MAP_PIXEL);
            Scroll( 0, nDif );
            SetMapMode(aOldMode);
        }
    }
    else
    {
        aOffset.Y() = nY;
        if (!bInSetZoom)
            Invalidate();
    }
    InvalidateLocationData( SC_HINT_ACC_VISAREACHANGED );
    Invalidate();
}

void ScPreview::DoInvalidate()
{
    //  If the whole GetState of the shell is called
    //  The Invalidate must come behind asynchronously

   if (bInGetState)
        Application::PostUserEvent( LINK( this, ScPreview, InvalidateHdl ), NULL, true );
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
    rBindings.Invalidate(SID_ROWCOL_SELCOUNT);
    rBindings.Invalidate(SID_STATUS_PAGESTYLE);
    rBindings.Invalidate(SID_PREVIEW_PREVIOUS);
    rBindings.Invalidate(SID_PREVIEW_NEXT);
    rBindings.Invalidate(SID_PREVIEW_FIRST);
    rBindings.Invalidate(SID_PREVIEW_LAST);
    rBindings.Invalidate(SID_ATTR_ZOOM);
    rBindings.Invalidate(SID_PREVIEW_ZOOMIN);
    rBindings.Invalidate(SID_PREVIEW_ZOOMOUT);
    rBindings.Invalidate(SID_PREVIEW_SCALINGFACTOR);
    rBindings.Invalidate(SID_ATTR_ZOOMSLIDER);
}

IMPL_STATIC_LINK_NOARG_TYPED( ScPreview, InvalidateHdl, void*, void )
{
    StaticInvalidate();
}

void ScPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged(rDCEvt);

    if ( (rDCEvt.GetType() == DataChangedEventType::PRINTER) ||
         (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
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
            InvalidateLocationData( SC_HINT_DATACHANGED );
        }
    }
}

void ScPreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    Fraction  aPreviewZoom( nZoom, 100 );
    Fraction  aHorPrevZoom( (long)( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode   aMMMode( MAP_100TH_MM, Point(), aHorPrevZoom, aPreviewZoom );

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

    if( rMEvt.IsLeft() && GetPointer() == PointerStyle::HSplit )
    {
        Point  aNowPt = rMEvt.GetPosPixel();
        SCCOL i = 0;
        for( i = aPageArea.aStart.Col(); i<= aPageArea.aEnd.Col(); i++ )
        {
            if( aNowPt.X() < nRight[i] + 2 && aNowPt.X() > nRight[i] - 2 )
            {
                nColNumberButttonDown = i;
                break;
            }
        }
        if( i == aPageArea.aEnd.Col()+1 )
            return;

        SetMapMode( aMMMode );
        if( nColNumberButttonDown == aPageArea.aStart.Col() )
            DrawInvert( PixelToLogic( Point( nLeftPosition, 0 ),aMMMode ).X() ,PointerStyle::HSplit );
        else
            DrawInvert( PixelToLogic( Point( nRight[ nColNumberButttonDown-1 ], 0 ),aMMMode ).X() ,PointerStyle::HSplit );

        DrawInvert( aButtonDownChangePoint.X(), PointerStyle::HSplit );
        bColRulerMove = true;
    }
}

void ScPreview::MouseButtonUp( const MouseEvent& rMEvt )
{
        Fraction  aPreviewZoom( nZoom, 100 );
        Fraction  aHorPrevZoom( (long)( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
        MapMode   aMMMode( MAP_100TH_MM, Point(), aHorPrevZoom, aPreviewZoom );

        aButtonUpPt = PixelToLogic( rMEvt.GetPosPixel(),aMMMode );

        long  nWidth = (long) lcl_GetDocPageSize(&pDocShell->GetDocument(), nTab).Width();
        long  nHeight = (long) lcl_GetDocPageSize(&pDocShell->GetDocument(), nTab).Height();

        if( rMEvt.IsLeft() && GetPointer() == PointerStyle::HSizeBar )
        {
            SetPointer( Pointer( PointerStyle::Arrow ) );

            ScDocument& rDoc = pDocShell->GetDocument();
            OUString aOldName = rDoc.GetPageStyle( nTab );
            bool bUndo = rDoc.IsUndoEnabled();
            ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
            SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aOldName, SFX_STYLE_FAMILY_PAGE );

            if ( pStyleSheet )
            {
                bool bMoveRulerAction= true;
                ScStyleSaveData aOldData;
                if( bUndo )
                    aOldData.InitFromStyle( pStyleSheet );

                SfxItemSet&  rStyleSet = pStyleSheet->GetItemSet();

                SvxLRSpaceItem aLRItem = static_cast<const SvxLRSpaceItem&>( rStyleSet.Get( ATTR_LRSPACE ) );

                if(( bLeftRulerChange || bRightRulerChange ) && ( aButtonUpPt.X() <= ( 0 - aOffset.X() ) || aButtonUpPt.X() > nWidth * HMM_PER_TWIPS - aOffset.X() ) )
                {
                    bMoveRulerAction = false;
                    Invalidate(Rectangle(0, 0, 10000, 10000));
                }
                else if( bLeftRulerChange && ( aButtonUpPt.X() / HMM_PER_TWIPS > nWidth - aLRItem.GetRight() - aOffset.X() / HMM_PER_TWIPS ) )
                {
                    bMoveRulerAction = false;
                    Invalidate(Rectangle(0, 0, 10000, 10000));
                }
                else if( bRightRulerChange && ( aButtonUpPt.X() / HMM_PER_TWIPS < aLRItem.GetLeft() - aOffset.X() / HMM_PER_TWIPS ) )
                {
                    bMoveRulerAction = false;
                    Invalidate(Rectangle(0, 0, 10000, 10000));
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
                       aLRItem.SetLeft( (long)( aButtonUpPt.X() / HMM_PER_TWIPS + aOffset.X() / HMM_PER_TWIPS ));
                       rStyleSet.Put( aLRItem );
                       pDocShell->SetModified();
                    }
                    else if( bRightRulerChange && bRightRulerMove )
                    {
                        aLRItem.SetRight( (long)( nWidth - aButtonUpPt.X() / HMM_PER_TWIPS - aOffset.X() / HMM_PER_TWIPS ));
                        rStyleSet.Put( aLRItem );
                        pDocShell->SetModified();
                    }

                    ScStyleSaveData aNewData;
                    aNewData.InitFromStyle( pStyleSheet );
                    if( bUndo )
                    {
                        pDocShell->GetUndoManager()->AddUndoAction(
                            new ScUndoModifyStyle( pDocShell, SFX_STYLE_FAMILY_PAGE,
                            aOldData, aNewData ) );
                    }

                    if ( ValidTab( nTab ) )
                    {
                        ScPrintFunc aPrintFunc( this, pDocShell, nTab );
                        aPrintFunc.UpdatePages();
                    }

                    Rectangle aRect(0,0,10000,10000);
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
            if( ( bTopRulerChange || bBottomRulerChange || bHeaderRulerChange || bFooterRulerChange ) && ( aButtonUpPt.Y() <= ( 0 - aOffset.Y() ) || aButtonUpPt.Y() > nHeight * HMM_PER_TWIPS -aOffset.Y() ) )
            {
                bMoveRulerAction = false;
                Invalidate(Rectangle(0, 0, 10000, 10000));
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
                SfxStyleSheetBase* pStyleSheet = pStylePool->Find( rDoc.GetPageStyle( nTab ), SFX_STYLE_FAMILY_PAGE );
                OSL_ENSURE( pStyleSheet, "PageStyle not found" );
                if ( pStyleSheet )
                {
                    ScDocShellModificator aModificator( *pDocShell );
                    ScStyleSaveData aOldData;
                    if( bUndo )
                        aOldData.InitFromStyle( pStyleSheet );

                    SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();

                    SvxULSpaceItem aULItem = static_cast<const SvxULSpaceItem&>( rStyleSet.Get( ATTR_ULSPACE ) );

                    if( bTopRulerMove && bTopRulerChange )
                    {
                        aULItem.SetUpperValue( (sal_uInt16)( aButtonUpPt.Y() / HMM_PER_TWIPS + aOffset.Y() / HMM_PER_TWIPS ) );
                        rStyleSet.Put( aULItem );
                        pDocShell->SetModified();
                    }
                    else if( bBottomRulerMove && bBottomRulerChange )
                    {
                        aULItem.SetLowerValue( (sal_uInt16)( nHeight - aButtonUpPt.Y() / HMM_PER_TWIPS - aOffset.Y() / HMM_PER_TWIPS ) );
                        rStyleSet.Put( aULItem );
                        pDocShell->SetModified();
                    }
                    else if( bHeaderRulerMove && bHeaderRulerChange )
                    {
                        const SfxPoolItem* pItem = NULL;
                        if ( rStyleSet.GetItemState( ATTR_PAGE_HEADERSET, false, &pItem ) == SfxItemState::SET )
                        {
                            const SfxItemSet& pHeaderSet = static_cast<const SvxSetItem*>(pItem)->GetItemSet();
                            Size  aHeaderSize = static_cast<const SvxSizeItem&>(pHeaderSet.Get(ATTR_PAGE_SIZE)).GetSize();
                            aHeaderSize.Height() = (long)( aButtonUpPt.Y() / HMM_PER_TWIPS + aOffset.Y() / HMM_PER_TWIPS - aULItem.GetUpper());
                            aHeaderSize.Height() = aHeaderSize.Height() * 100 / mnScale;
                            SvxSetItem  aNewHeader( static_cast<const SvxSetItem&>(rStyleSet.Get(ATTR_PAGE_HEADERSET)) );
                            aNewHeader.GetItemSet().Put( SvxSizeItem( ATTR_PAGE_SIZE, aHeaderSize ) );
                            rStyleSet.Put( aNewHeader );
                            pDocShell->SetModified();
                        }
                    }
                    else if( bFooterRulerMove && bFooterRulerChange )
                    {
                        const SfxPoolItem* pItem = NULL;
                        if( rStyleSet.GetItemState( ATTR_PAGE_FOOTERSET, false, &pItem ) == SfxItemState::SET )
                        {
                            const SfxItemSet& pFooterSet = static_cast<const SvxSetItem*>(pItem)->GetItemSet();
                            Size aFooterSize = static_cast<const SvxSizeItem&>(pFooterSet.Get(ATTR_PAGE_SIZE)).GetSize();
                            aFooterSize.Height() = (long)( nHeight - aButtonUpPt.Y() / HMM_PER_TWIPS - aOffset.Y() / HMM_PER_TWIPS - aULItem.GetLower() );
                            aFooterSize.Height() = aFooterSize.Height() * 100 / mnScale;
                            SvxSetItem  aNewFooter( static_cast<const SvxSetItem&>(rStyleSet.Get(ATTR_PAGE_FOOTERSET)) );
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
                            new ScUndoModifyStyle( pDocShell, SFX_STYLE_FAMILY_PAGE,
                            aOldData, aNewData ) );
                    }

                    if ( ValidTab( nTab ) )
                    {
                        ScPrintFunc aPrintFunc( this, pDocShell, nTab );
                        aPrintFunc.UpdatePages();
                    }

                    Rectangle aRect(0, 0, 10000, 10000);
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
                if( nColNumberButttonDown == aPageArea.aStart.Col() )
                    DrawInvert( PixelToLogic( Point( nLeftPosition, 0 ),aMMMode ).X() ,PointerStyle::HSplit );
                else
                    DrawInvert( PixelToLogic( Point( nRight[ nColNumberButttonDown-1 ], 0 ),aMMMode ).X() ,PointerStyle::HSplit );
                DrawInvert( aButtonUpPt.X(), PointerStyle::HSplit );
            }
            if( bMoveRulerAction )
            {
                long  nNewColWidth = 0;
                std::vector<sc::ColRowSpan> aCols(1, sc::ColRowSpan(nColNumberButttonDown,nColNumberButttonDown));

                if( !bLayoutRTL )
                {
                    nNewColWidth = (long) ( PixelToLogic( Point( rMEvt.GetPosPixel().X() - nRight[ nColNumberButttonDown ], 0), aMMMode ).X() / HMM_PER_TWIPS ) * 100 / mnScale;
                    nNewColWidth += pDocShell->GetDocument().GetColWidth( nColNumberButttonDown, nTab );
                }
                else
                {

                    nNewColWidth = (long) ( PixelToLogic( Point( nRight[ nColNumberButttonDown ] - rMEvt.GetPosPixel().X(), 0), aMMMode ).X() / HMM_PER_TWIPS ) * 100 / mnScale;
                    nNewColWidth += pDocShell->GetDocument().GetColWidth( nColNumberButttonDown, nTab );
                }

                if( nNewColWidth >= 0 )
                {
                    pDocShell->GetDocFunc().SetWidthOrHeight(
                        true, aCols, nTab, SC_SIZE_DIRECT, (sal_uInt16)nNewColWidth, true, true);
                    pDocShell->SetModified();
                }
                if ( ValidTab( nTab ) )
                {
                    ScPrintFunc aPrintFunc( this, pDocShell, nTab );
                    aPrintFunc.UpdatePages();
                }
                Rectangle aRect(0, 0, 10000, 10000);
                Invalidate(aRect);
            }
            bColRulerMove = false;
        }
        ReleaseMouse();
}

void ScPreview::MouseMove( const MouseEvent& rMEvt )
{
    Fraction aPreviewZoom( nZoom, 100 );
    Fraction aHorPrevZoom( (long)( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode  aMMMode( MAP_100TH_MM, Point(), aHorPrevZoom, aPreviewZoom );
    Point    aMouseMovePoint = PixelToLogic( rMEvt.GetPosPixel(), aMMMode );

    long    nLeftMargin = 0;
    long    nRightMargin = 0;
    long    nTopMargin = 0;
    long    nBottomMargin = 0;

    long    nWidth = (long) lcl_GetDocPageSize(&pDocShell->GetDocument(), nTab).Width();
    long    nHeight = (long) lcl_GetDocPageSize(&pDocShell->GetDocument(), nTab).Height();

    if ( nPageNo < nTotalPages )
    {
        ScPrintOptions aOptions = SC_MOD()->GetPrintOptions();

        ScPrintFunc* pPrintFunc;

        if (bStateValid)
            pPrintFunc = new ScPrintFunc( this, pDocShell, aState, &aOptions );
        else
            pPrintFunc = new ScPrintFunc( this, pDocShell, nTab, nFirstAttr[nTab], nTotalPages, NULL, &aOptions );

        nLeftMargin = (long)( pPrintFunc->GetLeftMargin() * HMM_PER_TWIPS - aOffset.X() );
        nRightMargin = (long)( pPrintFunc->GetRightMargin() * HMM_PER_TWIPS );
        nRightMargin = (long)( nWidth * HMM_PER_TWIPS - nRightMargin - aOffset.X() );
        nTopMargin = (long)( pPrintFunc->GetTopMargin() * HMM_PER_TWIPS - aOffset.Y() );
        nBottomMargin = (long)( pPrintFunc->GetBottomMargin() * HMM_PER_TWIPS );
        nBottomMargin = (long)( nHeight * HMM_PER_TWIPS - nBottomMargin - aOffset.Y() );
        if( mnScale > 0 )
        {
            nHeaderHeight = (long)( nTopMargin + pPrintFunc->GetHeader().nHeight * HMM_PER_TWIPS * mnScale / 100 );
            nFooterHeight = (long)( nBottomMargin - pPrintFunc->GetFooter().nHeight * HMM_PER_TWIPS * mnScale / 100 );
        }
        else
        {
            nHeaderHeight = (long)( nTopMargin + pPrintFunc->GetHeader().nHeight * HMM_PER_TWIPS );
            nFooterHeight = (long)( nBottomMargin - pPrintFunc->GetFooter().nHeight * HMM_PER_TWIPS );
        }
        delete pPrintFunc;
    }

    Point   aPixPt( rMEvt.GetPosPixel() );
    Point   aLeftTop = LogicToPixel( Point( nLeftMargin, -aOffset.Y() ) , aMMMode );
    Point   aLeftBottom = LogicToPixel( Point( nLeftMargin ,(long)(nHeight * HMM_PER_TWIPS - aOffset.Y()) ), aMMMode );
    Point   aRightTop = LogicToPixel( Point( nRightMargin, -aOffset.Y() ), aMMMode );
    Point   aTopLeft = LogicToPixel( Point( -aOffset.X(), nTopMargin ), aMMMode );
    Point   aTopRight = LogicToPixel( Point( (long)(nWidth * HMM_PER_TWIPS - aOffset.X()), nTopMargin ), aMMMode );
    Point   aBottomLeft = LogicToPixel( Point( -aOffset.X(), nBottomMargin ), aMMMode );
    Point   aHeaderLeft = LogicToPixel( Point(  -aOffset.X(), nHeaderHeight ), aMMMode );
    Point   aFooderLeft = LogicToPixel( Point( -aOffset.X(), nFooterHeight ), aMMMode );

    bool bOnColRulerChange = false;

    for( SCCOL i=aPageArea.aStart.Col(); i<= aPageArea.aEnd.Col(); i++ )
    {
        Point   aColumnTop = LogicToPixel( Point( 0, -aOffset.Y() ) ,aMMMode );
        Point   aColumnBottom = LogicToPixel( Point( 0, (long)( nHeight * HMM_PER_TWIPS - aOffset.Y()) ), aMMMode );
        if( aPixPt.X() < ( nRight[i] + 2 ) && ( aPixPt.X() > ( nRight[i] - 2 ) ) && ( aPixPt.X() < aRightTop.X() ) && ( aPixPt.X() > aLeftTop.X() )
            && ( aPixPt.Y() > aColumnTop.Y() ) && ( aPixPt.Y() < aColumnBottom.Y() ) && !bLeftRulerMove && !bRightRulerMove
            && !bTopRulerMove && !bBottomRulerMove && !bHeaderRulerMove && !bFooterRulerMove )
        {
            bOnColRulerChange = true;
            if( !rMEvt.GetButtons() && GetPointer() == PointerStyle::HSplit )
                nColNumberButttonDown = i;
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

    if( bPageMargin )
    {
        if(( (aPixPt.X() < ( aLeftTop.X() + 2 ) && aPixPt.X() > ( aLeftTop.X() - 2 )) || bLeftRulerMove ||
            ( aPixPt.X() < ( aRightTop.X() + 2 ) && aPixPt.X() > ( aRightTop.X() - 2 ) ) || bRightRulerMove || bOnColRulerChange || bColRulerMove )
            && aPixPt.Y() > aLeftTop.Y() && aPixPt.Y() < aLeftBottom.Y() )
        {
            if( bOnColRulerChange || bColRulerMove )
            {
                SetPointer( Pointer( PointerStyle::HSplit ) );
                if( bColRulerMove )
                {
                    if( aMouseMovePoint.X() > -aOffset.X() && aMouseMovePoint.X() < nWidth * HMM_PER_TWIPS - aOffset.X() )
                       DragMove( aMouseMovePoint.X(), PointerStyle::HSplit );
                }
            }
            else
            {
                if( bLeftRulerChange && !bTopRulerMove && !bBottomRulerMove && !bHeaderRulerMove && !bFooterRulerMove )
                {
                    SetPointer( Pointer( PointerStyle::HSizeBar ) );
                    if( bLeftRulerMove )
                    {
                       if( aMouseMovePoint.X() > -aOffset.X() && aMouseMovePoint.X() < nWidth * HMM_PER_TWIPS - aOffset.X() )
                           DragMove( aMouseMovePoint.X(), PointerStyle::HSizeBar );
                    }
                }
                else if( bRightRulerChange && !bTopRulerMove && !bBottomRulerMove && !bHeaderRulerMove && !bFooterRulerMove )
                {
                    SetPointer( Pointer( PointerStyle::HSizeBar ) );
                    if( bRightRulerMove )
                    {
                       if( aMouseMovePoint.X() > -aOffset.X() && aMouseMovePoint.X() < nWidth * HMM_PER_TWIPS - aOffset.X() )
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
                    SetPointer( Pointer( PointerStyle::VSizeBar ) );
                    if( bTopRulerMove )
                    {
                        if( aMouseMovePoint.Y() > -aOffset.Y() && aMouseMovePoint.Y() < nHeight * HMM_PER_TWIPS - aOffset.Y() )
                            DragMove( aMouseMovePoint.Y(), PointerStyle::VSizeBar );
                    }
                }
                else if( bBottomRulerChange )
                {
                    SetPointer( Pointer( PointerStyle::VSizeBar ) );
                    if( bBottomRulerMove )
                    {
                        if( aMouseMovePoint.Y() > -aOffset.Y() && aMouseMovePoint.Y() < nHeight * HMM_PER_TWIPS - aOffset.Y() )
                            DragMove( aMouseMovePoint.Y(), PointerStyle::VSizeBar );
                    }
                }
                else if( bHeaderRulerChange )
                {
                    SetPointer( Pointer( PointerStyle::VSizeBar ) );
                    if( bHeaderRulerMove )
                    {
                        if( aMouseMovePoint.Y() > -aOffset.Y() && aMouseMovePoint.Y() < nHeight * HMM_PER_TWIPS - aOffset.Y() )
                            DragMove( aMouseMovePoint.Y(), PointerStyle::VSizeBar );
                    }
                }
                else if( bFooterRulerChange )
                {
                    SetPointer( Pointer( PointerStyle::VSizeBar ) );
                    if( bFooterRulerMove )
                    {
                        if( aMouseMovePoint.Y() > -aOffset.Y() && aMouseMovePoint.Y() < nHeight * HMM_PER_TWIPS - aOffset.Y() )
                            DragMove( aMouseMovePoint.Y(), PointerStyle::VSizeBar );
                    }
                }
            }
            else
                SetPointer( Pointer( PointerStyle::Arrow ) );
        }
    }
}

void ScPreview::InvalidateLocationData(sal_uLong nId)
{
    bLocationValid = false;
    if (pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility( SfxSimpleHint( nId ) );
}

void ScPreview::GetFocus()
{
    Window::GetFocus();
    if (pViewShell && pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility( ScAccWinFocusGotHint(GetAccessible()) );
}

void ScPreview::LoseFocus()
{
    if (pViewShell && pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility( ScAccWinFocusLostHint(GetAccessible()) );
    Window::LoseFocus();
}

com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessible> ScPreview::CreateAccessible()
{
    com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessible> xAcc= GetAccessible(false);
    if (xAcc.is())
    {
        return xAcc;
    }

    ScAccessibleDocumentPagePreview* pAccessible =
        new ScAccessibleDocumentPagePreview( GetAccessibleParentWindow()->GetAccessible(), pViewShell );

    xAcc = pAccessible;
    SetAccessible(xAcc);
    pAccessible->Init();
    return xAcc;
}

void ScPreview::DragMove( long nDragMovePos, PointerStyle nFlags )
{
    Fraction aPreviewZoom( nZoom, 100 );
    Fraction aHorPrevZoom( (long)( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode  aMMMode( MAP_100TH_MM, Point(), aHorPrevZoom, aPreviewZoom );
    SetMapMode( aMMMode );
    long  nPos = nDragMovePos;
    if( nFlags == PointerStyle::HSizeBar || nFlags == PointerStyle::HSplit )
    {
        if( nDragMovePos != aButtonDownChangePoint.X() )
        {
            DrawInvert( aButtonDownChangePoint.X(), nFlags );
            aButtonDownChangePoint.X() = nPos;
            DrawInvert( aButtonDownChangePoint.X(), nFlags );
        }
    }
    else if( nFlags == PointerStyle::VSizeBar )
    {
        if( nDragMovePos != aButtonDownChangePoint.Y() )
        {
            DrawInvert( aButtonDownChangePoint.Y(), nFlags );
            aButtonDownChangePoint.Y() = nPos;
            DrawInvert( aButtonDownChangePoint.Y(), nFlags );
        }
    }
}

void ScPreview::DrawInvert( long nDragPos, PointerStyle nFlags )
{
    long  nHeight = (long) lcl_GetDocPageSize( &pDocShell->GetDocument(), nTab ).Height();
    long  nWidth = (long) lcl_GetDocPageSize( &pDocShell->GetDocument(), nTab ).Width();
    if( nFlags == PointerStyle::HSizeBar || nFlags == PointerStyle::HSplit )
    {
        Rectangle aRect( nDragPos, -aOffset.Y(), nDragPos + 1,(long)( ( nHeight * HMM_PER_TWIPS ) - aOffset.Y()));
        Invert( aRect, InvertFlags::N50 );
    }
    else if( nFlags == PointerStyle::VSizeBar )
    {
        Rectangle aRect( -aOffset.X(), nDragPos,(long)( ( nWidth * HMM_PER_TWIPS ) - aOffset.X() ), nDragPos + 1 );
        Invert( aRect, InvertFlags::N50 );
    }
}

void ScPreview::SetSelectedTabs(const ScMarkData& rMark)
{
    maSelectedTabs = rMark.GetSelectedTabs();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
