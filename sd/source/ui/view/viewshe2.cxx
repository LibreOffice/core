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

#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include "ViewShell.hxx"
#include "ViewShellHint.hxx"

#include "ViewShellImplementation.hxx"
#include "FactoryIds.hxx"

#include <svx/svxids.hrc>
#include <vcl/scrbar.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <svx/ruler.hxx>
#include <editeng/outliner.hxx>
#include <svtools/ehdl.hxx>
#include <svx/svdoole2.hxx>
#include <svtools/sfxecode.hxx>
#include <svx/fmshell.hxx>
#include <rtl/ustrbuf.hxx>
#include <unotools/moduleoptions.hxx>
#include <svx/dialogs.hrc>
#include <comphelper/classids.hxx>

#include "strings.hrc"
#include "app.hrc"
#include "unokywds.hxx"

#include "sdundogr.hxx"
#include "FrameView.hxx"
#include "undopage.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "View.hxx"
#include "fupoor.hxx"
#include "Client.hxx"
#include "DrawDocShell.hxx"
#include "fusearch.hxx"
#include "slideshow.hxx"
#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "ViewShellBase.hxx"

#include "Window.hxx"

#include <sfx2/viewfrm.hxx>
#include <svtools/soerr.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/charthelper.hxx>
#include <comphelper/lok.hxx>

using namespace com::sun::star;

namespace sd {

/**
 * adjust Thumbpos and VisibleSize
 */
void ViewShell::UpdateScrollBars()
{
    if (mpHorizontalScrollBar.get() != nullptr)
    {
        long nW = (long)(mpContentWindow->GetVisibleWidth() * 32000);
        long nX = (long)(mpContentWindow->GetVisibleX() * 32000);
        mpHorizontalScrollBar->SetVisibleSize(nW);
        mpHorizontalScrollBar->SetThumbPos(nX);
        nW = 32000 - nW;
        long nLine = (long) (mpContentWindow->GetScrlLineWidth() * nW);
        long nPage = (long) (mpContentWindow->GetScrlPageWidth() * nW);
        mpHorizontalScrollBar->SetLineSize(nLine);
        mpHorizontalScrollBar->SetPageSize(nPage);
    }

    if (mpVerticalScrollBar.get() != nullptr)
    {
        long nH = (long)(mpContentWindow->GetVisibleHeight() * 32000);
        long nY = (long)(mpContentWindow->GetVisibleY() * 32000);

        if(IsPageFlipMode()) // ie in zoom mode where no panning
        {
            SdPage* pPage = static_cast<DrawViewShell*>(this)->GetActualPage();
            sal_uInt16 nCurPage = (pPage->GetPageNum() - 1) / 2;
            sal_uInt16 nTotalPages = GetDoc()->GetSdPageCount(pPage->GetPageKind());
            mpVerticalScrollBar->SetRange(Range(0,256*nTotalPages));
            mpVerticalScrollBar->SetVisibleSize(256);
            mpVerticalScrollBar->SetThumbPos(256*nCurPage);
            mpVerticalScrollBar->SetLineSize(256);
            mpVerticalScrollBar->SetPageSize(256);
        }
        else
        {
            mpVerticalScrollBar->SetRange(Range(0,32000));
            mpVerticalScrollBar->SetVisibleSize(nH);
            mpVerticalScrollBar->SetThumbPos(nY);
            nH = 32000 - nH;
            long nLine = (long) (mpContentWindow->GetScrlLineHeight() * nH);
            long nPage = (long) (mpContentWindow->GetScrlPageHeight() * nH);
            mpVerticalScrollBar->SetLineSize(nLine);
            mpVerticalScrollBar->SetPageSize(nPage);
        }
    }

    if (mbHasRulers)
    {
        UpdateHRuler();
        UpdateVRuler();
    }

}
/**
 * Handling for horizontal Scrollbars
 */
IMPL_LINK(ViewShell, HScrollHdl, ScrollBar *, pHScroll, void )
{
    VirtHScrollHdl(pHScroll);
}

/**
 * virtual scroll handler for horizontal Scrollbars
 */
void ViewShell::VirtHScrollHdl(ScrollBar* pHScroll)
{
    long nDelta = pHScroll->GetDelta();

    if (nDelta != 0)
    {
        double fX = (double) pHScroll->GetThumbPos() / pHScroll->GetRange().Len();

        // scroll all windows of the column
        ::sd::View* pView = GetView();
        OutlinerView* pOLV = nullptr;

        if (pView)
            pOLV = pView->GetTextEditOutlinerView();

        if (pOLV)
            pOLV->HideCursor();

        mpContentWindow->SetVisibleXY(fX, -1);

        Rectangle aVisArea = GetDocSh()->GetVisArea(ASPECT_CONTENT);
        Point aVisAreaPos = GetActiveWindow()->PixelToLogic( Point(0,0) );
        aVisArea.SetPos(aVisAreaPos);
        GetDocSh()->SetVisArea(aVisArea);

        Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
        Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
        VisAreaChanged(aVisAreaWin);

        if (pView)
        {
            pView->VisAreaChanged(GetActiveWindow());
        }

        if (pOLV)
            pOLV->ShowCursor();

        if (mbHasRulers)
            UpdateHRuler();

    }
}

/**
 * handling for vertical Scrollbars
 */
IMPL_LINK(ViewShell, VScrollHdl, ScrollBar *, pVScroll, void )
{
    VirtVScrollHdl(pVScroll);
}

/**
 * handling for vertical Scrollbars
 */
void ViewShell::VirtVScrollHdl(ScrollBar* pVScroll)
{
    if(IsPageFlipMode())
    {
        SdPage* pPage = static_cast<DrawViewShell*>(this)->GetActualPage();
        sal_uInt16 nCurPage = (pPage->GetPageNum() - 1) >> 1;
        sal_uInt16 nNewPage = (sal_uInt16)pVScroll->GetThumbPos()/256;
        if( nCurPage != nNewPage )
            static_cast<DrawViewShell*>(this)->SwitchPage(nNewPage);
    }
    else //panning mode
    {
        double fY = (double) pVScroll->GetThumbPos() / pVScroll->GetRange().Len();

        ::sd::View* pView = GetView();
        OutlinerView* pOLV = nullptr;

        if (pView)
            pOLV = pView->GetTextEditOutlinerView();

        if (pOLV)
            pOLV->HideCursor();

        mpContentWindow->SetVisibleXY(-1, fY);

        Rectangle aVisArea = GetDocSh()->GetVisArea(ASPECT_CONTENT);
        Point aVisAreaPos = GetActiveWindow()->PixelToLogic( Point(0,0) );
        aVisArea.SetPos(aVisAreaPos);
        GetDocSh()->SetVisArea(aVisArea);

        Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
        Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
        VisAreaChanged(aVisAreaWin);

        if (pView)
        {
            pView->VisAreaChanged(GetActiveWindow());
        }

        if (pOLV)
            pOLV->ShowCursor();

        if (mbHasRulers)
            UpdateVRuler();

    }
}

VclPtr<SvxRuler> ViewShell::CreateHRuler(::sd::Window* )
{
    return nullptr;
}

VclPtr<SvxRuler> ViewShell::CreateVRuler(::sd::Window* )
{
    return nullptr;
}

void ViewShell::UpdateHRuler()
{
}

void ViewShell::UpdateVRuler()
{
}

/**
 * Scroll a specific number of lines. Is used in the automatic scrolling
 * (character/drag).
 */
void ViewShell::ScrollLines(long nLinesX, long nLinesY)
{
    if ( nLinesX )
    {
        nLinesX *= mpHorizontalScrollBar->GetLineSize();
    }
    if ( nLinesY )
    {
        nLinesY *= mpVerticalScrollBar->GetLineSize();
    }

    Scroll(nLinesX, nLinesY);
}

void ViewShell::Scroll(long nScrollX, long nScrollY)
{
    if (nScrollX)
    {
        long nNewThumb = mpHorizontalScrollBar->GetThumbPos() + nScrollX;
        mpHorizontalScrollBar->SetThumbPos(nNewThumb);
    }
    if (nScrollY)
    {
        long nNewThumb = mpVerticalScrollBar->GetThumbPos() + nScrollY;
        mpVerticalScrollBar->SetThumbPos(nNewThumb);
    }
    double  fX = (double) mpHorizontalScrollBar->GetThumbPos() /
                            mpHorizontalScrollBar->GetRange().Len();
    double  fY = (double) mpVerticalScrollBar->GetThumbPos() /
                            mpVerticalScrollBar->GetRange().Len();

    GetActiveWindow()->SetVisibleXY(fX, fY);

    Rectangle aVisArea = GetDocSh()->GetVisArea(ASPECT_CONTENT);
    Point aVisAreaPos = GetActiveWindow()->PixelToLogic( Point(0,0) );
    aVisArea.SetPos(aVisAreaPos);
    GetDocSh()->SetVisArea(aVisArea);

    Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
    Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    ::sd::View* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(GetActiveWindow());
    }

    if (mbHasRulers)
    {
        UpdateHRuler();
        UpdateVRuler();
    }
}

/**
 * Set zoom factor for all split windows.
 */
void ViewShell::SetZoom(long nZoom)
{
    Fraction aUIScale(nZoom, 100);
    aUIScale *= GetDoc()->GetUIScale();

    if (mpHorizontalRuler.get() != nullptr)
        mpHorizontalRuler->SetZoom(aUIScale);

    if (mpVerticalRuler.get() != nullptr)
        mpVerticalRuler->SetZoom(aUIScale);

    if (mpContentWindow.get() != nullptr)
    {
        mpContentWindow->SetZoomIntegral(nZoom);

        // #i74769# Here is a 2nd way (besides Window::Scroll) to set the visible prt
        // of the window. It needs - like Scroll(ScrollFlags::Children) does - also to move
        // the child windows. I am trying InvalidateFlags::Children here which makes things better,
        // but does not solve the problem completely. Need to ask PL.
        mpContentWindow->Invalidate(InvalidateFlags::Children);
    }

    Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
    Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    ::sd::View* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(GetActiveWindow());
    }

    UpdateScrollBars();
}

/**
 * Set zoom rectangle for active window. Sets all split windows to the same zoom
 * factor.
 */
void ViewShell::SetZoomRect(const Rectangle& rZoomRect)
{
    long nZoom = GetActiveWindow()->SetZoomRect(rZoomRect);
    Fraction aUIScale(nZoom, 100);
    aUIScale *= GetDoc()->GetUIScale();

    Point aPos = GetActiveWindow()->GetWinViewPos();

    if (mpHorizontalRuler.get() != nullptr)
        mpHorizontalRuler->SetZoom(aUIScale);

    if (mpVerticalRuler.get() != nullptr)
        mpVerticalRuler->SetZoom(aUIScale);

    if (mpContentWindow.get() != nullptr)
    {
        Point aNewPos = mpContentWindow->GetWinViewPos();
        aNewPos.X() = aPos.X();
        aNewPos.Y() = aPos.Y();
        mpContentWindow->SetZoomIntegral(nZoom);
        mpContentWindow->SetWinViewPos(aNewPos);
        mpContentWindow->UpdateMapOrigin();

        // When tiled rendering, UpdateMapOrigin() doesn't touch the map mode.
        if (!comphelper::LibreOfficeKit::isActive())
            // #i74769# see above
            mpContentWindow->Invalidate(InvalidateFlags::Children);
    }

    Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
    Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    ::sd::View* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(GetActiveWindow());
    }

    UpdateScrollBars();
}

/**
 * Initialize imaging parameters for all split windows.
 */
void ViewShell::InitWindows(const Point& rViewOrigin, const Size& rViewSize,
                              const Point& rWinPos, bool bUpdate)
{
    if (mpContentWindow.get() != nullptr)
    {
        mpContentWindow->SetViewOrigin(rViewOrigin);
        mpContentWindow->SetViewSize(rViewSize);
        mpContentWindow->SetWinViewPos(rWinPos);

        if ( bUpdate )
        {
            mpContentWindow->UpdateMapOrigin();
            mpContentWindow->Invalidate();
        }
    }

    Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
    Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    ::sd::View* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(GetActiveWindow());
    }
}

/**
 * Invalidate all split windows below the ?provided rectangle.
 */
void ViewShell::InvalidateWindows()
{
    if (mpContentWindow.get() != nullptr)
        mpContentWindow->Invalidate();
}

/**
 * Draw a selection rectangle with the ?provided pen on all split windows.
 */
void ViewShell::DrawMarkRect(const Rectangle& rRect) const
{
    if (mpContentWindow.get() != nullptr)
    {
        mpContentWindow->InvertTracking(rRect, ShowTrackFlags::Object | ShowTrackFlags::TrackWindow);
    }
}

void ViewShell::SetPageSizeAndBorder(PageKind ePageKind, const Size& rNewSize,
                                       long nLeft, long nRight,
                                       long nUpper, long nLower, bool bScaleAll,
                                       Orientation eOrientation, sal_uInt16 nPaperBin,
                                       bool bBackgroundFullSize)
{
    SdPage* pPage = nullptr;
    SdUndoGroup* pUndoGroup = nullptr;
    pUndoGroup = new SdUndoGroup(GetDoc());
    OUString aString(SdResId(STR_UNDO_CHANGE_PAGEFORMAT));
    pUndoGroup->SetComment(aString);
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=nullptr);

    sal_uInt16 i, nPageCnt = GetDoc()->GetMasterSdPageCount(ePageKind);

    Broadcast (ViewShellHint(ViewShellHint::HINT_PAGE_RESIZE_START));

    for (i = 0; i < nPageCnt; i++)
    {
        // first, handle all master pages
        pPage = GetDoc()->GetMasterSdPage(i, ePageKind);

        SdUndoAction* pUndo = new SdPageFormatUndoAction(GetDoc(), pPage,
                            pPage->GetSize(),
                            pPage->GetLftBorder(), pPage->GetRgtBorder(),
                            pPage->GetUppBorder(), pPage->GetLwrBorder(),
                            pPage->GetOrientation(),
                            pPage->GetPaperBin(),
                            pPage->IsBackgroundFullSize(),
                            rNewSize,
                            nLeft, nRight,
                            nUpper, nLower,
                            bScaleAll,
                            eOrientation,
                            nPaperBin,
                            bBackgroundFullSize);
        pUndoGroup->AddAction(pUndo);

        if (rNewSize.Width() > 0 ||
            nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0)
        {
            Rectangle aNewBorderRect(nLeft, nUpper, nRight, nLower);
            pPage->ScaleObjects(rNewSize, aNewBorderRect, bScaleAll);

            if (rNewSize.Width() > 0)
                pPage->SetSize(rNewSize);
        }

        if( nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0 )
        {
            pPage->SetBorder(nLeft, nUpper, nRight, nLower);
        }

        pPage->SetOrientation(eOrientation);
        pPage->SetPaperBin( nPaperBin );
        pPage->SetBackgroundFullSize( bBackgroundFullSize );

        if ( ePageKind == PageKind::Standard )
            GetDoc()->GetMasterSdPage(i, PageKind::Notes)->CreateTitleAndLayout();

        pPage->CreateTitleAndLayout();
    }

    nPageCnt = GetDoc()->GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        // then, handle all pages
        pPage = GetDoc()->GetSdPage(i, ePageKind);

        SdUndoAction* pUndo = new SdPageFormatUndoAction(GetDoc(), pPage,
                                pPage->GetSize(),
                                pPage->GetLftBorder(), pPage->GetRgtBorder(),
                                pPage->GetUppBorder(), pPage->GetLwrBorder(),
                                pPage->GetOrientation(),
                                pPage->GetPaperBin(),
                                pPage->IsBackgroundFullSize(),
                                rNewSize,
                                nLeft, nRight,
                                nUpper, nLower,
                                bScaleAll,
                                eOrientation,
                                nPaperBin,
                                bBackgroundFullSize);
        pUndoGroup->AddAction(pUndo);

        if (rNewSize.Width() > 0 ||
            nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0)
        {
            Rectangle aNewBorderRect(nLeft, nUpper, nRight, nLower);
            pPage->ScaleObjects(rNewSize, aNewBorderRect, bScaleAll);

            if (rNewSize.Width() > 0)
                pPage->SetSize(rNewSize);
        }

        if( nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0 )
        {
            pPage->SetBorder(nLeft, nUpper, nRight, nLower);
        }

        pPage->SetOrientation(eOrientation);
        pPage->SetPaperBin( nPaperBin );
        pPage->SetBackgroundFullSize( bBackgroundFullSize );

        if ( ePageKind == PageKind::Standard )
        {
            SdPage* pNotesPage = GetDoc()->GetSdPage(i, PageKind::Notes);
            pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
        }

        pPage->SetAutoLayout( pPage->GetAutoLayout() );
    }

    // adjust handout page to new format of the standard page
    if( (ePageKind == PageKind::Standard) || (ePageKind == PageKind::Handout) )
        GetDoc()->GetSdPage(0, PageKind::Handout)->CreateTitleAndLayout(true);

    // handed over undo group to undo manager
    pViewShell->GetViewFrame()->GetObjectShell()
        ->GetUndoManager()->AddUndoAction(pUndoGroup);

    long nWidth = pPage->GetSize().Width();
    long nHeight = pPage->GetSize().Height();

    Point aPageOrg(nWidth, nHeight / 2);
    Size aViewSize(nWidth * 3, nHeight * 2);

    InitWindows(aPageOrg, aViewSize, Point(-1, -1), true);

    Point aVisAreaPos;

    if ( GetDocSh()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        aVisAreaPos = GetDocSh()->GetVisArea(ASPECT_CONTENT).TopLeft();
    }

    ::sd::View* pView = GetView();
    if (pView)
    {
        pView->SetWorkArea(Rectangle(Point() - aVisAreaPos - aPageOrg, aViewSize));
    }

    UpdateScrollBars();

    Point aNewOrigin(pPage->GetLftBorder(), pPage->GetUppBorder());

    if (pView)
    {
        pView->GetSdrPageView()->SetPageOrigin(aNewOrigin);
    }

    pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);

    // zoom onto (new) page size
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);

    Broadcast (ViewShellHint(ViewShellHint::HINT_PAGE_RESIZE_END));
}

/**
 * Set zoom factor for InPlace
 */
void ViewShell::SetZoomFactor(const Fraction& rZoomX, const Fraction&)
{
    long nZoom = (long)((double) rZoomX * 100);
    SetZoom(nZoom);
}

void ViewShell::SetActiveWindow (::sd::Window* pWin)
{
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=nullptr);

    if (pViewShell->GetWindow() != pWin)
    {
        // #i31551# was wrong, it may have been a problem with the repaint at that time.
        // For transparent form controls, it is necessary to have that flag set, all apps
        // do set it. Enabling again.
        if (pWin)
        {
            pWin->EnableChildTransparentMode();
        }
    }

    if (mpActiveWindow.get() != pWin)
        mpActiveWindow = pWin;

    // The rest of this function is not guarded anymore against calling this
    // method with an already active window because the functions may still
    // point to the old window when the new one has already been assigned to
    // pWindow elsewhere.
    ::sd::View* pView = GetView();
    if (pView)
    {
        pView->SetActualWin(pWin);
    }
    if(HasCurrentFunction())
    {
        GetCurrentFunction()->SetWindow(pWin);
    }
}

bool ViewShell::RequestHelp(const HelpEvent& rHEvt, ::sd::Window*)
{
    bool bReturn = false;

    if (bool(rHEvt.GetMode()))
    {
        if( GetView() )
            bReturn = GetView()->getSmartTags().RequestHelp(rHEvt);

        if(!bReturn && HasCurrentFunction())
        {
            bReturn = GetCurrentFunction()->RequestHelp(rHEvt);
        }
    }

    return bReturn;
}

void ViewShell::SetFrameView (FrameView* pNewFrameView)
{
    mpFrameView = pNewFrameView;
    ReadFrameViewData (mpFrameView);
}

/*************************************************************************
|*
|* Read FrameViews data and set actual views data
|*
\************************************************************************/

void ViewShell::ReadFrameViewData(FrameView*)
{
}

/*************************************************************************
|*
|* Write actual views data to FrameView
|*
\************************************************************************/

void ViewShell::WriteFrameViewData()
{
}

bool ViewShell::ActivateObject(SdrOle2Obj* pObj, long nVerb)
{
    ErrCode aErrCode = 0;

    SfxErrorContext aEC(ERRCTX_SO_DOVERB, GetActiveWindow(), RID_SO_ERRCTX);
    bool bAbort = false;
    GetDocSh()->SetWaitCursor( true );
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=nullptr);
    bool bChangeDefaultsForChart = false;
    OUString aName;

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    if ( !xObj.is() )
    {
        // provide OLE object to empty OLE object
        aName = pObj->GetProgName();
        OUString aObjName;
        SvGlobalName aClass;

        if( aName == "StarChart"  || aName == "StarOrg" )
        {
            if( SvtModuleOptions().IsChart() )
            {
                aClass = SvGlobalName( SO3_SCH_CLASSID );
                bChangeDefaultsForChart = true;
            }
        }
        else if( aName == "StarCalc" )
        {
            if( SvtModuleOptions().IsCalc() )
                aClass = SvGlobalName( SO3_SC_CLASSID );
        }
        else if( aName == "StarMath" )
        {
            if( SvtModuleOptions().IsMath() )
                aClass = SvGlobalName( SO3_SM_CLASSID );
        }

        if ( aClass != SvGlobalName() )
            xObj = GetDocSh()->GetEmbeddedObjectContainer().CreateEmbeddedObject( aClass.GetByteSequence(), aObjName );

        if( !xObj.is() )
        {
            aName.clear();

            // call dialog "insert OLE object"
            GetDocSh()->SetWaitCursor( false );
            pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                SID_INSERT_OBJECT,
                SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
            xObj = pObj->GetObjRef();
            GetDocSh()->SetWaitCursor( true );

            if (!xObj.is())
            {
                bAbort = true;
            }
        }

        if ( xObj.is() )
        {
            // OLE object is no longer empty
            pObj->SetEmptyPresObj(false);
            pObj->SetOutlinerParaObject(nullptr);
            pObj->SetGraphic(nullptr);

            // the empty OLE object gets a new IPObj
            if (!aName.isEmpty())
            {
                pObj->SetObjRef(xObj);
                pObj->SetName(aObjName);
                pObj->SetPersistName(aObjName);
            }
            else
            {
                // insertion was done by the dialog
                pObj->SetObjRef(xObj);
            }

            Rectangle aRect = pObj->GetLogicRect();

            if ( pObj->GetAspect() != embed::Aspects::MSOLE_ICON )
            {
                awt::Size aSz;
                aSz.Width = aRect.GetWidth();
                aSz.Height = aRect.GetHeight();
                xObj->setVisualAreaSize( pObj->GetAspect(), aSz );
            }

            GetViewShellBase().SetVerbs( xObj->getSupportedVerbs() );

            nVerb = embed::EmbedVerbs::MS_OLEVERB_SHOW;
        }
        else
        {
            aErrCode = ERRCODE_SFX_OLEGENERAL;
        }
    }

    if( aErrCode == 0 )
    {
        ::sd::View* pView = GetView();

        if (pView->IsTextEdit())
        {
            pView->SdrEndTextEdit();
        }

        SfxInPlaceClient* pSdClient =
            static_cast<Client*>(pViewShell->FindIPClient(
                pObj->GetObjRef(), GetActiveWindow()));

        if ( !pSdClient )
        {
            pSdClient = new Client(pObj, this, GetActiveWindow());
        }

        Rectangle aRect = pObj->GetLogicRect();

        {
            // #i118485# center on BoundRect for activation,
            // OLE may be sheared/rotated now
            const Rectangle& rBoundRect = pObj->GetCurrentBoundRect();
            const Point aDelta(rBoundRect.Center() - aRect.Center());
            aRect.Move(aDelta.X(), aDelta.Y());
        }

        Size aDrawSize = aRect.GetSize();

        MapMode aMapMode( GetDoc()->GetScaleUnit() );
        Size aObjAreaSize = pObj->GetOrigObjSize( &aMapMode );
        if( pObj->IsChart() ) //charts never should be stretched see #i84323# for example
            aObjAreaSize = aDrawSize;

        Fraction aScaleWidth (aDrawSize.Width(),  aObjAreaSize.Width() );
        Fraction aScaleHeight(aDrawSize.Height(), aObjAreaSize.Height() );
        aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
        aScaleHeight.ReduceInaccurate(10);
        pSdClient->SetSizeScale(aScaleWidth, aScaleHeight);

        // visible section is only changed in-place!
        aRect.SetSize(aObjAreaSize);
        // the object area size must be set after scaling, since it triggers the resizing
        pSdClient->SetObjArea(aRect);

        if( bChangeDefaultsForChart && xObj.is())
        {
            ChartHelper::AdaptDefaultsForChart( xObj );
        }

        pSdClient->DoVerb(nVerb);   // if necessary, ErrCode is outputted by Sfx
        pViewShell->GetViewFrame()->GetBindings().Invalidate(
            SID_NAVIGATOR_STATE, true);
    }

    GetDocSh()->SetWaitCursor( false );

    if (aErrCode != 0 && !bAbort)
    {
        ErrorHandler::HandleError( (new StringErrorInfo(aErrCode, ""))->GetErrorCode() );
    }

    return aErrCode == 0;
}

/**
 * @returns enclosing rectangle of all (split-) windows.
 */
const Rectangle& ViewShell::GetAllWindowRect()
{
    maAllWindowRectangle.SetPos(
        mpContentWindow->OutputToScreenPixel(Point(0,0)));
    return maAllWindowRectangle;
}

void ViewShell::ReadUserData(const OUString&)
{
    // zoom onto VisArea from FrameView
    GetViewShell()->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_VISAREA,
        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
}

void ViewShell::WriteUserData(OUString&)
{
    // writing of our data is always done in WriteFrameViewData()
    WriteFrameViewData();
}

/**
 * Switch ruler on/off
 */
void ViewShell::SetRuler(bool bRuler)
{
    mbHasRulers = ( bRuler && !GetDocSh()->IsPreview() ); // no rulers on preview mode

    if (mpHorizontalRuler.get() != nullptr)
    {
        if (mbHasRulers)
        {
            mpHorizontalRuler->Show();
        }
        else
        {
            mpHorizontalRuler->Hide();
        }
    }

    if (mpVerticalRuler.get() != nullptr)
    {
        if (mbHasRulers)
        {
            mpVerticalRuler->Show();
        }
        else
        {
            mpVerticalRuler->Hide();
        }
    }

    OSL_ASSERT(GetViewShell()!=nullptr);
    if (IsMainViewShell())
        GetViewShell()->InvalidateBorder();
}

sal_Int8 ViewShell::AcceptDrop (
    const AcceptDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    ::sd::View* pView = GetView();
    return( pView ? pView->AcceptDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer ) : DND_ACTION_NONE );
}

sal_Int8 ViewShell::ExecuteDrop (
    const ExecuteDropEvent& rEvt,
    DropTargetHelper& /*rTargetHelper*/,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    ::sd::View* pView = GetView();
    return pView ? pView->ExecuteDrop( rEvt, pTargetWindow, nPage, nLayer ) : DND_ACTION_NONE;
}

void ViewShell::WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >& rSequence, bool bBrowse)
{
    const sal_Int32 nIndex = rSequence.getLength();
    rSequence.realloc( nIndex + 1 );

    OSL_ASSERT (GetViewShell()!=nullptr);
    // Get the view id from the view shell in the center pane.  This will
    // usually be the called view shell, but to be on the safe side we call
    // the main view shell explicitly.
    sal_uInt16 nViewID (IMPRESS_FACTORY_ID);
    if (GetViewShellBase().GetMainViewShell().get() != nullptr)
        nViewID = GetViewShellBase().GetMainViewShell()->mpImpl->GetViewId();
    rSequence[nIndex].Name = sUNO_View_ViewId;
    OUStringBuffer sBuffer( "view" );
    sBuffer.append( static_cast<sal_Int32>(nViewID));
    rSequence[nIndex].Value <<= sBuffer.makeStringAndClear();

    mpFrameView->WriteUserDataSequence( rSequence, bBrowse );
}

void ViewShell::ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >& rSequence, bool bBrowse )
{
    mpFrameView->ReadUserDataSequence( rSequence, bBrowse );
}

void ViewShell::VisAreaChanged(const Rectangle& rRect)
{
    OSL_ASSERT (GetViewShell()!=nullptr);
    GetViewShell()->VisAreaChanged(rRect);
}

void ViewShell::SetWinViewPos(const Point& rWinPos)
{
    if (mpContentWindow.get() != nullptr)
    {
        mpContentWindow->SetWinViewPos(rWinPos);

        mpContentWindow->UpdateMapOrigin();
        mpContentWindow->Invalidate();
    }

    if (mbHasRulers)
    {
        UpdateHRuler();
        UpdateVRuler();
    }

    UpdateScrollBars();

    Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
    Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    ::sd::View* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(GetActiveWindow());
    }
}

Point ViewShell::GetWinViewPos() const
{
    return mpContentWindow->GetWinViewPos();
}

Point ViewShell::GetViewOrigin() const
{
    return mpContentWindow->GetViewOrigin();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
