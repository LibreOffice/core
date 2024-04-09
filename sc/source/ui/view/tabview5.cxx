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

#include <svx/fmshell.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdoutl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <osl/diagnose.h>

#include <tabview.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>
#include <gridwin.hxx>
#include <olinewin.hxx>
#include <tabsplit.hxx>
#include <colrowba.hxx>
#include <tabcont.hxx>
#include <sc.hrc>
#include <pagedata.hxx>
#include <hiranges.hxx>
#include <drawview.hxx>
#include <drwlayer.hxx>
#include <fusel.hxx>
#include <seltrans.hxx>
#include <scmod.hxx>
#include <docsh.hxx>
#include <viewuno.hxx>
#include <postit.hxx>
#include <spellcheckcontext.hxx>

#include <vcl/settings.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/scopeguard.hxx>
#include <officecfg/Office/Calc.hxx>

using namespace com::sun::star;

void ScTabView::Init()
{
    /*  RTL layout of the view windows is done manually, because it depends on
        the sheet orientation, not the UI setting. Note: controls that are
        already constructed (e.g. scroll bars) have the RTL setting of the GUI.
        Eventually this has to be disabled manually (see below). */
    pFrameWin->EnableRTL( false );

    sal_uInt16 i;

    mbInlineWithScrollbar = officecfg::Office::Calc::Layout::Other::TabbarInlineWithScrollbar::get();

    aScrollTimer.SetTimeout(10);
    aScrollTimer.SetInvokeHandler( LINK( this, ScTabView, TimerHdl ) );

    for (i=0; i<4; i++)
        pGridWin[i] = nullptr;
    pGridWin[SC_SPLIT_BOTTOMLEFT] = VclPtr<ScGridWindow>::Create( pFrameWin, aViewData, SC_SPLIT_BOTTOMLEFT );

    pSelEngine.reset( new ScViewSelectionEngine( pGridWin[SC_SPLIT_BOTTOMLEFT], this,
                                                SC_SPLIT_BOTTOMLEFT ) );
    aFunctionSet.SetSelectionEngine( pSelEngine.get() );

    pHdrSelEng.reset( new ScHeaderSelectionEngine( pFrameWin, &aHdrFunc ) );

    pColBar[SC_SPLIT_LEFT] = VclPtr<ScColBar>::Create( pFrameWin, SC_SPLIT_LEFT,
                                                       &aHdrFunc, pHdrSelEng.get(), this );
    pColBar[SC_SPLIT_RIGHT] = nullptr;
    pRowBar[SC_SPLIT_BOTTOM] = VclPtr<ScRowBar>::Create( pFrameWin, SC_SPLIT_BOTTOM,
                                                         &aHdrFunc, pHdrSelEng.get(), this );
    pRowBar[SC_SPLIT_TOP] = nullptr;
    for (i=0; i<2; i++)
        pColOutline[i] = pRowOutline[i] = nullptr;

    pHSplitter = VclPtr<ScTabSplitter>::Create( pFrameWin, WinBits( WB_HSCROLL ), &aViewData );
    pVSplitter = VclPtr<ScTabSplitter>::Create( pFrameWin, WinBits( WB_VSCROLL ), &aViewData );

    // SSA: override default keyboard step size to allow snap to row/column
    pHSplitter->SetKeyboardStepSize( 1 );
    pVSplitter->SetKeyboardStepSize( 1 );

    pTabControl = VclPtr<ScTabControl>::Create(pFrameWin, &aViewData);
    if (mbInlineWithScrollbar)
        pTabControl->SetStyle(pTabControl->GetStyle() | WB_SIZEABLE);

    /*  #i97900# The tab control has to remain in RTL mode if GUI is RTL, this
        is needed to draw the 3D effect correctly. The base TabBar implements
        mirroring independent from the GUI direction. Have to set RTL mode
        explicitly because the parent frame window is already RTL disabled. */
    pTabControl->EnableRTL( AllSettings::GetLayoutRTL() );

    InitScrollBar( *aHScrollLeft,    aViewData.GetDocument().MaxCol()+1, LINK(this, ScTabView, HScrollLeftHdl) );
    InitScrollBar( *aHScrollRight,   aViewData.GetDocument().MaxCol()+1, LINK(this, ScTabView, HScrollRightHdl) );
    InitScrollBar( *aVScrollTop,     aViewData.GetDocument().MaxRow()+1, LINK(this, ScTabView, VScrollTopHdl) );
    InitScrollBar( *aVScrollBottom,  aViewData.GetDocument().MaxRow()+1, LINK(this, ScTabView, VScrollBottomHdl) );
    /*  #i97900# scrollbars remain in correct RTL mode, needed mirroring etc.
        is now handled correctly at the respective places. */

    //  Don't show anything here, because still in wrong order
    //  Show is received from UpdateShow during first resize
    //      pTabControl, pGridWin, aHScrollLeft, aVScrollBottom,
    //      aCornerButton, pHSplitter, pVSplitter

    //      fragment

    pHSplitter->SetSplitHdl( LINK( this, ScTabView, SplitHdl ) );
    pVSplitter->SetSplitHdl( LINK( this, ScTabView, SplitHdl ) );

    //  UpdateShow is done during resize or a copy of an existing view from ctor

    pDrawActual = nullptr;
    pDrawOld = nullptr;

    //  DrawView cannot be create in the TabView - ctor
    //  when the ViewShell isn't constructed yet...
    //  The also applies to ViewOptionsHasChanged()

    TestHintWindow();
}

ScTabView::~ScTabView()
{
    sal_uInt16 i;

    //  remove selection object
    ScModule* pScMod = SC_MOD();
    ScSelectionTransferObj* pOld = pScMod->GetSelectionTransfer();
    if ( pOld && pOld->GetView() == this )
    {
        pOld->ForgetView();
        pScMod->SetSelectionTransfer( nullptr );
        TransferableHelper::ClearPrimarySelection();       // may delete pOld
    }

    pBrushDocument.reset();
    pDrawBrushSet.reset();

    pPageBreakData.reset();

    delete pDrawActual;
    pDrawActual = nullptr;
    delete pDrawOld;
    pDrawOld = nullptr;

    if (comphelper::LibreOfficeKit::isActive())
    {
        ScTabViewShell* pThisViewShell = GetViewData().GetViewShell();

        auto lRemoveWindows =
                [pThisViewShell] (ScTabViewShell* pOtherViewShell)
                {
                    ScViewData& rOtherViewData = pOtherViewShell->GetViewData();
                    for (int k = 0; k < 4; ++k)
                    {
                        if (rOtherViewData.HasEditView(static_cast<ScSplitPos>(k)))
                            pThisViewShell->RemoveWindowFromForeignEditView(pOtherViewShell, static_cast<ScSplitPos>(k));
                    }
                };

        SfxLokHelper::forEachOtherView(pThisViewShell, lRemoveWindows);
    }

    aViewData.KillEditView();           // as long as GridWins still exist

    if (pDrawView)
    {
        for (i=0; i<4; i++)
            if (pGridWin[i])
            {
                pDrawView->DeleteDeviceFromPaintView(*pGridWin[i]->GetOutDev());
            }

        pDrawView->HideSdrPage();
        pDrawView.reset();
    }

    pSelEngine.reset();

    if (mpSpellCheckCxt)
        mpSpellCheckCxt->dispose();
    mpSpellCheckCxt.reset();

    mxInputHintOO.reset();
    for (i=0; i<4; i++)
        pGridWin[i].disposeAndClear();

    pHdrSelEng.reset();

    for (i=0; i<2; i++)
    {
        pColBar[i].disposeAndClear();
        pRowBar[i].disposeAndClear();
        pColOutline[i].disposeAndClear();
        pRowOutline[i].disposeAndClear();
    }

    aCornerButton.disposeAndClear();
    aTopButton.disposeAndClear();
    aHScrollLeft.disposeAndClear();
    aHScrollRight.disposeAndClear();
    aVScrollTop.disposeAndClear();
    aVScrollBottom.disposeAndClear();

    pHSplitter.disposeAndClear();
    pVSplitter.disposeAndClear();
    pTabControl.disposeAndClear();
}

void ScTabView::MakeDrawView( TriState nForceDesignMode )
{
    if (pDrawView)
        return;

    ScDrawLayer* pLayer = aViewData.GetDocument().GetDrawLayer();
    OSL_ENSURE(pLayer, "Where is the Draw Layer ??");

    sal_uInt16 i;
    pDrawView.reset( new ScDrawView( pGridWin[SC_SPLIT_BOTTOMLEFT]->GetOutDev(), &aViewData ) );
    for (i=0; i<4; i++)
        if (pGridWin[i])
        {
            if ( SC_SPLIT_BOTTOMLEFT != static_cast<ScSplitPos>(i) )
                pDrawView->AddDeviceToPaintView(*pGridWin[i]->GetOutDev(), nullptr);
        }
    pDrawView->RecalcScale();
    for (i=0; i<4; i++)
        if (pGridWin[i])
        {
            pGridWin[i]->SetMapMode(pGridWin[i]->GetDrawMapMode());

            pGridWin[i]->PaintImmediately(); // because of Invalidate in DrawView ctor (ShowPage),
                                             // so that immediately can be drawn
        }
    SfxRequest aSfxRequest(SID_OBJECT_SELECT, SfxCallMode::SLOT, aViewData.GetViewShell()->GetPool());
    SetDrawFuncPtr(new FuSelection(*aViewData.GetViewShell(), GetActiveWin(), pDrawView.get(),
                                   pLayer,aSfxRequest));

    //  used when switching back from page preview: restore saved design mode state
    //  (otherwise, keep the default from the draw view ctor)
    if ( nForceDesignMode != TRISTATE_INDET )
        pDrawView->SetDesignMode( nForceDesignMode != TRISTATE_FALSE );

    //  register at FormShell
    FmFormShell* pFormSh = aViewData.GetViewShell()->GetFormShell();
    if (pFormSh)
        pFormSh->SetView(pDrawView.get());

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxHint(SfxHintId::ScAccMakeDrawLayer));
}

void ScTabView::DoAddWin( ScGridWindow* pWin )
{
    if (pDrawView)
    {
        pDrawView->AddDeviceToPaintView(*pWin->GetOutDev(), nullptr);
        pWin->DrawLayerCreated();
    }
    pWin->SetAutoSpellContext(mpSpellCheckCxt);
}

void ScTabView::ImplTabChanged(bool bSameTabButMoved)
{
    // For kit ignore invalidations during tab change
    ScTabViewShell* pViewShell = aViewData.GetViewShell();
    SfxLokCallbackInterface* pCallback = pViewShell->getLibreOfficeKitViewCallback();
    pViewShell->setLibreOfficeKitViewCallback(nullptr);
    comphelper::ScopeGuard aOutputGuard(
        [this, pViewShell, pCallback] {
            pViewShell->setLibreOfficeKitViewCallback(pCallback);
            // But possibly update any out of date formulas on the tab we switched to
            UpdateFormulas();
        });

    if (pDrawView)
    {
        DrawDeselectAll();      // end also text edit mode

        SCTAB nTab = aViewData.GetTabNo();
        pDrawView->HideSdrPage();
        pDrawView->ShowSdrPage(pDrawView->GetModel()->GetPage(nTab));

        UpdateLayerLocks();

        pDrawView->RecalcScale();
        pDrawView->UpdateWorkArea();    // PageSize is different per page
    }

    SfxBindings& rBindings = aViewData.GetBindings();

    //  There is no easy way to invalidate all slots of the FormShell
    //  (for disabled slots on protected tables), therefore simply everything...
    rBindings.InvalidateAll(false);

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
    {
        SfxHint aAccHint(SfxHintId::ScAccTableChanged);
        aViewData.GetViewShell()->BroadcastAccessibility(aAccHint);
    }

    // notification for XActivationBroadcaster
    SfxViewFrame* pViewFrame = aViewData.GetViewShell()->GetViewFrame();
    if (pViewFrame)
    {
        uno::Reference<frame::XController> xController = pViewFrame->GetFrame().GetController();
        if (xController.is())
        {
            ScTabViewObj* pImp = comphelper::getFromUnoTunnel<ScTabViewObj>( xController );
            if (pImp)
                pImp->SheetChanged( bSameTabButMoved );
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (pGridWin[i])
        {
            pGridWin[i]->initiatePageBreaks();
            // Trigger calculating page breaks only once.
            break;
        }
    }
}

void ScTabView::TabChanged( bool bSameTabButMoved )
{
    ImplTabChanged(bSameTabButMoved);

    if (!comphelper::LibreOfficeKit::isActive())
        return;

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScModelObj* pModelObj = pDocSh ? comphelper::getFromUnoTunnel<ScModelObj>( pDocSh->GetModel()) : nullptr;

    if (!pModelObj)
        return;

    Size aDocSize = pModelObj->getDocumentSize();
    std::stringstream ss;
    ss << aDocSize.Width() << ", " << aDocSize.Height();
    OString sRect = ss.str().c_str();
    ScTabViewShell* pViewShell = aViewData.GetViewShell();
    ScModelObj* pModel = comphelper::getFromUnoTunnel<ScModelObj>(pViewShell->GetCurrentDocument());
    SfxLokHelper::notifyDocumentSizeChanged(pViewShell, sRect, pModel, false);
}

void ScTabView::UpdateLayerLocks()
{
    if (!pDrawView)
        return;

    SCTAB nTab = aViewData.GetTabNo();
    bool bEx = aViewData.GetViewShell()->IsDrawSelMode();
    bool bProt = aViewData.GetDocument().IsTabProtected( nTab ) ||
                 aViewData.GetSfxDocShell()->IsReadOnly();
    bool bShared = aViewData.GetDocShell()->IsDocShared();

    SdrLayer* pLayer;
    SdrLayerAdmin& rAdmin = pDrawView->GetModel()->GetLayerAdmin();
    pLayer = rAdmin.GetLayerPerID(SC_LAYER_BACK);
    if (pLayer)
        pDrawView->SetLayerLocked( pLayer->GetName(), bProt || !bEx || bShared );
    pLayer = rAdmin.GetLayerPerID(SC_LAYER_INTERN);
    if (pLayer)
        pDrawView->SetLayerLocked( pLayer->GetName() );
    pLayer = rAdmin.GetLayerPerID(SC_LAYER_FRONT);
    if (pLayer)
        pDrawView->SetLayerLocked( pLayer->GetName(), bProt || bShared );
    pLayer = rAdmin.GetLayerPerID(SC_LAYER_CONTROLS);
    if (pLayer)
        pDrawView->SetLayerLocked( pLayer->GetName(), bProt || bShared );
    pLayer = rAdmin.GetLayerPerID(SC_LAYER_HIDDEN);
    if (pLayer)
    {
        pDrawView->SetLayerLocked( pLayer->GetName(), bProt || bShared );
        pDrawView->SetLayerVisible( pLayer->GetName(), false);
    }
    pTabControl->SetAddButtonEnabled(aViewData.GetDocument().IsDocEditable());
}

void ScTabView::DrawDeselectAll()
{
    if (!pDrawView)
        return;

    ScTabViewShell* pViewSh = aViewData.GetViewShell();
    if ( pDrawActual &&
        ( pViewSh->IsDrawTextShell() || pDrawActual->GetSlotID() == SID_DRAW_NOTEEDIT ) )
    {
        // end text edit (as if escape pressed, in FuDraw)
        aViewData.GetDispatcher().Execute( pDrawActual->GetSlotID(),
                                    SfxCallMode::SLOT | SfxCallMode::RECORD );
    }

    pDrawView->ScEndTextEdit();
    pDrawView->UnmarkAll();

    if (!pViewSh->IsDrawSelMode())
        pViewSh->SetDrawShell( false );
}

bool ScTabView::IsDrawTextEdit() const
{
    if (pDrawView)
        return pDrawView->IsTextEdit();
    else
        return false;
}

SvxZoomType ScTabView::GetZoomType() const
{
    return aViewData.GetZoomType();
}

void ScTabView::SetZoomType( SvxZoomType eNew, bool bAll )
{
    aViewData.SetZoomType( eNew, bAll );
}

void ScTabView::SetZoom( const Fraction& rNewX, const Fraction& rNewY, bool bAll )
{
    aViewData.SetZoom( rNewX, rNewY, bAll );
    if (pDrawView)
        pDrawView->RecalcScale();
    ZoomChanged();
}

void ScTabView::RefreshZoom()
{
    aViewData.RefreshZoom();
    if (pDrawView)
        pDrawView->RecalcScale();
    ZoomChanged();
}

void ScTabView::SetPagebreakMode( bool bSet )
{
    aViewData.SetPagebreakMode(bSet);
    if (pDrawView)
        pDrawView->RecalcScale();
    ZoomChanged();
}

void ScTabView::ResetDrawDragMode()
{
    if (pDrawView)
        pDrawView->SetDragMode( SdrDragMode::Move );
}

void ScTabView::ViewOptionsHasChanged( bool bHScrollChanged, bool bGraphicsChanged )
{
    //  create DrawView when grid should be displayed
    if ( !pDrawView && aViewData.GetOptions().GetGridOptions().GetGridVisible() )
        MakeDrawLayer();

    if (pDrawView)
        pDrawView->UpdateUserViewOptions();

    if (bGraphicsChanged)
        DrawEnableAnim(true);   // DrawEnableAnim checks the options state

    // if TabBar is set to visible, make sure its size is not 0
    bool bGrow = ( aViewData.IsTabMode() && pTabControl->GetSizePixel().Width() <= 0 );

    // if ScrollBar is set to visible, TabBar must make room
    bool bShrink = ( bHScrollChanged && aViewData.IsTabMode() && aViewData.IsHScrollMode() &&
                     pTabControl->GetSizePixel().Width() > SC_TABBAR_DEFWIDTH );

    if ( bGrow || bShrink )
    {
        Size aSize = pTabControl->GetSizePixel();
        aSize.setWidth( SC_TABBAR_DEFWIDTH );             // initial size
        pTabControl->SetSizePixel(aSize);               // DoResize is called later...
    }
}

// helper function against including the drawing layer

void ScTabView::DrawMarkListHasChanged()
{
    if ( pDrawView )
        pDrawView->MarkListHasChanged();
}

void ScTabView::UpdateAnchorHandles()
{
    if ( pDrawView )
        pDrawView->AdjustMarkHdl();
}

void ScTabView::UpdateIMap( SdrObject* pObj )
{
    if ( pDrawView )
        pDrawView->UpdateIMap( pObj );
}

void ScTabView::DrawEnableAnim(bool bSet)
{
    sal_uInt16 i;
    if ( !pDrawView )
        return;

    //  don't start animations if display of graphics is disabled
    //  graphics are controlled by VOBJ_TYPE_OLE
    if ( bSet && aViewData.GetOptions().GetObjMode(VOBJ_TYPE_OLE) == VOBJ_MODE_SHOW )
    {
        if ( !pDrawView->IsAnimationEnabled() )
        {
            pDrawView->SetAnimationEnabled();

            //  animated GIFs must be restarted:
            ScDocument& rDoc = aViewData.GetDocument();
            for (i=0; i<4; i++)
                if ( pGridWin[i] && pGridWin[i]->IsVisible() )
                    rDoc.StartAnimations( aViewData.GetTabNo() );
        }
    }
    else
    {
        pDrawView->SetAnimationEnabled(false);
    }
}

void ScTabView::UpdateDrawTextOutliner()
{
    if ( pDrawView )
    {
        Outliner* pOL = pDrawView->GetTextEditOutliner();
        if (pOL)
            aViewData.UpdateOutlinerFlags( *pOL );
    }
}

void ScTabView::DigitLanguageChanged()
{
    LanguageType eNewLang = SC_MOD()->GetOptDigitLanguage();
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
        if ( pWin )
            pWin->GetOutDev()->SetDigitLanguage( eNewLang );
}

void ScTabView::ScrollToObject( const SdrObject* pDrawObj )
{
    if ( pDrawObj )
    {
        // #i118524# use the BoundRect, this defines the visible area
        MakeVisible(pDrawObj->GetCurrentBoundRect());
    }
}

void ScTabView::MakeVisible( const tools::Rectangle& rHMMRect )
{
    vcl::Window* pWin = GetActiveWin();
    Size aWinSize = pWin->GetOutputSizePixel();
    SCTAB nTab = aViewData.GetTabNo();

    tools::Rectangle aRect = pWin->LogicToPixel( rHMMRect );

    tools::Long nScrollX=0, nScrollY=0;        // pixel

    if ( aRect.Right() >= aWinSize.Width() )                // right out
    {
        nScrollX = aRect.Right() - aWinSize.Width() + 1;    // right border visible
        if ( aRect.Left() < nScrollX )
            nScrollX = aRect.Left();                        // left visible (if too big)
    }
    if ( aRect.Bottom() >= aWinSize.Height() )              // bottom out
    {
        nScrollY = aRect.Bottom() - aWinSize.Height() + 1;  // bottom border visible
        if ( aRect.Top() < nScrollY )
            nScrollY = aRect.Top();                         // top visible (if too big)
    }

    if ( aRect.Left() < 0 )             // left out
        nScrollX = aRect.Left();        // left border visible
    if ( aRect.Top() < 0 )              // top out
        nScrollY = aRect.Top();         // top border visible

    if (!(nScrollX || nScrollY))
        return;

    ScDocument& rDoc = aViewData.GetDocument();
    if ( rDoc.IsNegativePage( nTab ) )
        nScrollX = -nScrollX;

    double nPPTX = aViewData.GetPPTX();
    double nPPTY = aViewData.GetPPTY();
    ScSplitPos eWhich = aViewData.GetActivePart();
    SCCOL nPosX = aViewData.GetPosX(WhichH(eWhich));
    SCROW nPosY = aViewData.GetPosY(WhichV(eWhich));

    tools::Long nLinesX=0, nLinesY=0;      // columns/rows - scroll at least nScrollX/Y

    if (nScrollX > 0)
        while (nScrollX > 0 && nPosX < rDoc.MaxCol())
        {
            nScrollX -= static_cast<tools::Long>( rDoc.GetColWidth(nPosX, nTab) * nPPTX );
            ++nPosX;
            ++nLinesX;
        }
    else if (nScrollX < 0)
        while (nScrollX < 0 && nPosX > 0)
        {
            --nPosX;
            nScrollX += static_cast<tools::Long>( rDoc.GetColWidth(nPosX, nTab) * nPPTX );
            --nLinesX;
        }

    if (nScrollY > 0)
        while (nScrollY > 0 && nPosY < rDoc.MaxRow())
        {
            nScrollY -= static_cast<tools::Long>( rDoc.GetRowHeight(nPosY, nTab) * nPPTY );
            ++nPosY;
            ++nLinesY;
        }
    else if (nScrollY < 0)
        while (nScrollY < 0 && nPosY > 0)
        {
            --nPosY;
            nScrollY += static_cast<tools::Long>( rDoc.GetRowHeight(nPosY, nTab) * nPPTY );
            --nLinesY;
        }

    ScrollLines( nLinesX, nLinesY );                    // execute
}

void ScTabView::SetBrushDocument( ScDocumentUniquePtr pNew, bool bLock )
{
    pDrawBrushSet.reset();
    pBrushDocument = std::move(pNew);

    bLockPaintBrush = bLock;

    aViewData.GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
}

void ScTabView::SetDrawBrushSet( std::unique_ptr<SfxItemSet> pNew, bool bLock )
{
    pBrushDocument.reset();
    pDrawBrushSet = std::move(pNew);

    bLockPaintBrush = bLock;

    aViewData.GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
}

void ScTabView::ResetBrushDocument()
{
    if ( HasPaintBrush() )
    {
        SetBrushDocument( nullptr, false );
        SetActivePointer( aViewData.IsThemedCursor() ? PointerStyle::FatCross : PointerStyle::Arrow ); // switch pointers also when ended with escape key
    }
}

void ScTabView::OnLOKNoteStateChanged(const ScPostIt* pNote)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    const SdrCaptionObj* pCaption = pNote->GetCaption();
    if (!pCaption) return;

    tools::Rectangle aRect = pCaption->GetLogicRect();
    basegfx::B2DRange aTailRange = pCaption->getTailPolygon().getB2DRange();
    tools::Rectangle aTailRect(aTailRange.getMinX(), aTailRange.getMinY(),
                        aTailRange.getMaxX(), aTailRange.getMaxY());
    aRect.Union( aTailRect );

    // This is a temporary workaround: sometime in tiled rendering mode
    // the tip of the note arrow is misplaced by a fixed offset.
    // The value used below is enough to get the tile, where the arrow tip is
    // placed, invalidated.
    const int nBorderSize = 200;
    tools::Rectangle aInvalidRect = aRect;
    aInvalidRect.AdjustLeft( -nBorderSize );
    aInvalidRect.AdjustRight( nBorderSize );
    aInvalidRect.AdjustTop( -nBorderSize );
    aInvalidRect.AdjustBottom( nBorderSize );

    SfxViewShell* pCurrentViewShell = SfxViewShell::Current();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
        if (pTabViewShell && pViewShell->GetDocId() == pCurrentViewShell->GetDocId())
        {
            for (auto& pWin: pTabViewShell->pGridWin)
            {
                if (pWin && pWin->IsVisible())
                {
                    pWin->Invalidate(aInvalidRect);
                }
            }
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
