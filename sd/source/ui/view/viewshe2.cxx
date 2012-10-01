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
#include <sot/clsids.hxx>

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

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

using namespace com::sun::star;

#ifndef DISABLE_DYNLOADING // otherwise use the one in sw...
const String aEmptyStr;
#endif

namespace sd {

/*************************************************************************
|*
|* Scrollbar-Update: Thumbpos und VisibleSize anpassen
|*
\************************************************************************/

void ViewShell::UpdateScrollBars()
{
    if (mpHorizontalScrollBar.get() != NULL)
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

    if (mpVerticalScrollBar.get() != NULL)
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
/*************************************************************************
|*
|* Handling fuer horizontale Scrollbars
|*
\************************************************************************/

IMPL_LINK_INLINE_START(ViewShell, HScrollHdl, ScrollBar *, pHScroll )
{
    return VirtHScrollHdl(pHScroll);
}
IMPL_LINK_INLINE_END(ViewShell, HScrollHdl, ScrollBar *, pHScroll )

/*************************************************************************
|*
|* virtueller Scroll-Handler fuer horizontale Scrollbars
|*
\************************************************************************/

long ViewShell::VirtHScrollHdl(ScrollBar* pHScroll)
{
    long nDelta = pHScroll->GetDelta();

    if (nDelta != 0)
    {
        double fX = (double) pHScroll->GetThumbPos() / pHScroll->GetRange().Len();

        // alle Fenster der Spalte scrollen
        ::sd::View* pView = GetView();
        OutlinerView* pOLV = NULL;

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

    return 0;
}

/*************************************************************************
|*
|* Handling fuer vertikale Scrollbars
|*
\************************************************************************/

IMPL_LINK_INLINE_START(ViewShell, VScrollHdl, ScrollBar *, pVScroll )
{
    return VirtVScrollHdl(pVScroll);
}
IMPL_LINK_INLINE_END(ViewShell, VScrollHdl, ScrollBar *, pVScroll )

/*************************************************************************
|*
|* Handling fuer vertikale Scrollbars
|*
\************************************************************************/

long ViewShell::VirtVScrollHdl(ScrollBar* pVScroll)
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
        OutlinerView* pOLV = NULL;

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

    return 0;
}

SvxRuler* ViewShell::CreateHRuler(::sd::Window* , sal_Bool )
{
    return NULL;
}

SvxRuler* ViewShell::CreateVRuler(::sd::Window* )
{
    return NULL;
}

void ViewShell::UpdateHRuler()
{
}

void ViewShell::UpdateVRuler()
{
}

long ViewShell::GetHCtrlWidth()
{
    return 0;
}

/*************************************************************************
|*
|* Eine bestimmte Anzahl von Zeilen scrollen (wird beim automatischen
|* Scrollen (Zeichen/Draggen) verwendet)
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Window um nScrollX, nScrollY scrollen
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Den Zoomfaktor fuer alle Split-Windows setzen
|*
\************************************************************************/

void ViewShell::SetZoom(long nZoom)
{
    Fraction aUIScale(nZoom, 100);
    aUIScale *= GetDoc()->GetUIScale();

    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetZoom(aUIScale);

    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->SetZoom(aUIScale);

    if (mpContentWindow.get() != NULL)
    {
        mpContentWindow->SetZoomIntegral(nZoom);

        // #i74769# Here is a 2nd way (besides Window::Scroll) to set the visible prt
        // of the window. It needs - like Scroll(SCROLL_CHILDREN) does - also to move
        // the child windows. I am trying INVALIDATE_CHILDREN here which makes things better,
        // but does not solve the problem completely. Neet to ask PL.
        mpContentWindow->Invalidate(INVALIDATE_CHILDREN);
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

/*************************************************************************
|*
|* Zoomrechteck fuer aktives Fenster einstellen und alle Split-Windows
|* auf den gleichen Zoomfaktor setzen
|*
\************************************************************************/

void ViewShell::SetZoomRect(const Rectangle& rZoomRect)
{
    long nZoom = GetActiveWindow()->SetZoomRect(rZoomRect);
    Fraction aUIScale(nZoom, 100);
    aUIScale *= GetDoc()->GetUIScale();

    Point aPos = GetActiveWindow()->GetWinViewPos();

    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetZoom(aUIScale);

    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->SetZoom(aUIScale);

    if (mpContentWindow.get() != NULL)
    {
        Point aNewPos = mpContentWindow->GetWinViewPos();
        aNewPos.X() = aPos.X();
        aNewPos.Y() = aPos.Y();
        mpContentWindow->SetZoomIntegral(nZoom);
        mpContentWindow->SetWinViewPos(aNewPos);
        mpContentWindow->UpdateMapOrigin();

        // #i74769# see above
        mpContentWindow->Invalidate(INVALIDATE_CHILDREN);
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

/*************************************************************************
|*
|* Abbildungsparameter fuer alle Split-Windows initialisieren
|*
\************************************************************************/

void ViewShell::InitWindows(const Point& rViewOrigin, const Size& rViewSize,
                              const Point& rWinPos, sal_Bool bUpdate)
{
    if (mpContentWindow.get() != NULL)
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

/*************************************************************************
|*
|* Alle Split-Windows unter dem uebergebenen Rechteck invalidieren
|*
\************************************************************************/

void ViewShell::InvalidateWindows()
{
    if (mpContentWindow.get() != NULL)
        mpContentWindow->Invalidate();
}


/*************************************************************************
|*
|* Auf allen Split-Windows ein Markierungsrechteck mit dem
|* uebergebenen Pen zeichnen
|*
\************************************************************************/

void ViewShell::DrawMarkRect(const Rectangle& rRect) const
{
    if (mpContentWindow.get() != NULL)
    {
        mpContentWindow->InvertTracking(rRect, SHOWTRACK_OBJECT | SHOWTRACK_WINDOW);
    }
}

/*************************************************************************
|*
|* Groesse und Raender aller Seiten setzen
|*
\************************************************************************/

void ViewShell::SetPageSizeAndBorder(PageKind ePageKind, const Size& rNewSize,
                                       long nLeft, long nRight,
                                       long nUpper, long nLower, sal_Bool bScaleAll,
                                       Orientation eOrientation, sal_uInt16 nPaperBin,
                                       sal_Bool bBackgroundFullSize)
{
    SdPage* pPage = 0;
    SdUndoGroup* pUndoGroup = NULL;
    pUndoGroup = new SdUndoGroup(GetDoc());
    String aString(SdResId(STR_UNDO_CHANGE_PAGEFORMAT));
    pUndoGroup->SetComment(aString);
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);

    sal_uInt16 i, nPageCnt = GetDoc()->GetMasterSdPageCount(ePageKind);

    Broadcast (ViewShellHint(ViewShellHint::HINT_PAGE_RESIZE_START));

    for (i = 0; i < nPageCnt; i++)
    {
        /**********************************************************************
        * Erst alle MasterPages bearbeiten
        **********************************************************************/
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

        if ( ePageKind == PK_STANDARD )
            GetDoc()->GetMasterSdPage(i, PK_NOTES)->CreateTitleAndLayout();

        pPage->CreateTitleAndLayout();
    }

    nPageCnt = GetDoc()->GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        /**********************************************************************
        * Danach alle Pages bearbeiten
        **********************************************************************/
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

        if ( ePageKind == PK_STANDARD )
        {
            SdPage* pNotesPage = GetDoc()->GetSdPage(i, PK_NOTES);
            pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
        }

        pPage->SetAutoLayout( pPage->GetAutoLayout() );
    }

    // Handoutseite an neues Format der Standardseiten anpassen
    if( (ePageKind == PK_STANDARD) || (ePageKind == PK_HANDOUT) )
        GetDoc()->GetSdPage(0, PK_HANDOUT)->CreateTitleAndLayout(sal_True);

    // Undo Gruppe dem Undo Manager uebergeben
    pViewShell->GetViewFrame()->GetObjectShell()
        ->GetUndoManager()->AddUndoAction(pUndoGroup);

    long nWidth = pPage->GetSize().Width();
    long nHeight = pPage->GetSize().Height();

    Point aPageOrg = Point(nWidth, nHeight / 2);
    Size aViewSize = Size(nWidth * 3, nHeight * 2);

    InitWindows(aPageOrg, aViewSize, Point(-1, -1), sal_True);

    Point aVisAreaPos;

    if ( GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
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

    // auf (neue) Seitengroesse zoomen
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

    Broadcast (ViewShellHint(ViewShellHint::HINT_PAGE_RESIZE_END));
}

/*************************************************************************
|*
|* Zoom-Faktor fuer InPlace einstellen
|*
\************************************************************************/

void ViewShell::SetZoomFactor(const Fraction& rZoomX, const Fraction&)
{
    long nZoom = (long)((double) rZoomX * 100);
    SetZoom(nZoom);
}


/*************************************************************************
|*
|* Aktives Fenster setzen
|*
\************************************************************************/

void ViewShell::SetActiveWindow (::sd::Window* pWin)
{
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);

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

    if (mpActiveWindow != pWin)
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



/*************************************************************************
|*
|* RequestHelp event
|*
\************************************************************************/

sal_Bool ViewShell::RequestHelp(const HelpEvent& rHEvt, ::sd::Window*)
{
    sal_Bool bReturn = sal_False;

    if (rHEvt.GetMode())
    {
        if( GetView() )
            bReturn = GetView()->getSmartTags().RequestHelp(rHEvt);

        if(!bReturn && HasCurrentFunction())
        {
            bReturn = GetCurrentFunction()->RequestHelp(rHEvt);
        }
    }

    return(bReturn);
}




FrameView* ViewShell::GetFrameView (void)
{
    return mpFrameView;
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

/*************************************************************************
|*
|* OLE-Object aktivieren
|*
\************************************************************************/

sal_Bool ViewShell::ActivateObject(SdrOle2Obj* pObj, long nVerb)
{
    ErrCode aErrCode = 0;

    SfxErrorContext aEC(ERRCTX_SO_DOVERB, GetActiveWindow(), RID_SO_ERRCTX);
    sal_Bool bAbort = sal_False;
    GetDocSh()->SetWaitCursor( sal_True );
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);
    bool bChangeDefaultsForChart = false;
    rtl::OUString aName;

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    if ( !xObj.is() )
    {
        /**********************************************************
        * Leeres OLE-Objekt mit OLE-Objekt versehen
        **********************************************************/
        aName = pObj->GetProgName();
        ::rtl::OUString aObjName;
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
            aName = "";

            // Dialog "OLE-Objekt einfuegen" aufrufen
            GetDocSh()->SetWaitCursor( sal_False );
            pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                SID_INSERT_OBJECT,
                SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
            xObj = pObj->GetObjRef();
            GetDocSh()->SetWaitCursor( sal_True );

            if (!xObj.is())
            {
                bAbort = sal_True;
            }
        }

        if ( xObj.is() )
        {
            /******************************************************
            * OLE-Objekt ist nicht mehr leer
            ******************************************************/
            pObj->SetEmptyPresObj(sal_False);
            pObj->SetOutlinerParaObject(NULL);
            pObj->SetGraphic(NULL);

            /******************************************************
            * Das leere OLE-Objekt bekommt ein neues IPObj
            ******************************************************/
            if (!aName.isEmpty())
            {
                pObj->SetObjRef(xObj);
                pObj->SetName(aObjName);
                pObj->SetPersistName(aObjName);
            }
            else
            {
                // Das Einfuegen hat der Dialog schon gemacht
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

            nVerb = SVVERB_SHOW;
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

        // sichtbarer Ausschnitt wird nur inplace veraendert!
        aRect.SetSize(aObjAreaSize);
        // the object area size must be set after scaling, since it triggers the resizing
        pSdClient->SetObjArea(aRect);

        if( bChangeDefaultsForChart && xObj.is())
        {
            AdaptDefaultsForChart( xObj );
        }

        pSdClient->DoVerb(nVerb);   // ErrCode wird ggf. vom Sfx ausgegeben
        pViewShell->GetViewFrame()->GetBindings().Invalidate(
            SID_NAVIGATOR_STATE, sal_True, sal_False);
    }

    GetDocSh()->SetWaitCursor( sal_False );

    if (aErrCode != 0 && !bAbort)
    {
        ErrorHandler::HandleError(* new StringErrorInfo(aErrCode, String() ) );
    }

    return aErrCode == 0;
}

/*************************************************************************
|*
|* umschliessendes Rechteck aller (Split-)Fenster zurueckgeben.
|*
\************************************************************************/

const Rectangle& ViewShell::GetAllWindowRect()
{
    maAllWindowRectangle.SetPos(
        mpContentWindow->OutputToScreenPixel(Point(0,0)));
    return maAllWindowRectangle;
}

/*************************************************************************
|*
|* Read user data
|*
\************************************************************************/
void ViewShell::ReadUserData(const String&)
{
    // Auf an FrameView gemerkte VisArea zoomen
    GetViewShell()->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_VISAREA,
        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
|* Write user data
|*
\************************************************************************/

void ViewShell::WriteUserData(String&)
{
    // Das Schreiben unserer Daten erfolgt stets in WriteFrameViewData()
    WriteFrameViewData();
}


/*************************************************************************
|*
|* Lineale ein- / ausschalten
|*
\************************************************************************/

void ViewShell::SetRuler(sal_Bool bRuler)
{
    mbHasRulers = ( bRuler && !GetDocSh()->IsPreview() ); // no rulers on preview mode

    if (mpHorizontalRuler.get() != NULL)
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

    if (mpVerticalRuler.get() != NULL)
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

    OSL_ASSERT(GetViewShell()!=NULL);
    if (IsMainViewShell())
        GetViewShell()->InvalidateBorder();
}

/*************************************************************************
|*
|* AcceptDrop
|*
\************************************************************************/

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

/*************************************************************************
|*
|* ExecuteDrop
|*
\************************************************************************/

sal_Int8 ViewShell::ExecuteDrop (
    const ExecuteDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    ::sd::View* pView = GetView();
    return( pView ? pView->ExecuteDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer ) : DND_ACTION_NONE );
}

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

void ViewShell::WriteUserDataSequence ( ::com::sun::star::uno::Sequence <
    ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse)
{
    const sal_Int32 nIndex = rSequence.getLength();
    rSequence.realloc( nIndex + 1 );

    OSL_ASSERT (GetViewShell()!=NULL);
    // Get the view id from the view shell in the center pane.  This will
    // usually be the called view shell, but to be on the safe side we call
    // the main view shell explicitly.
    sal_uInt16 nViewID (IMPRESS_FACTORY_ID);
    if (GetViewShellBase().GetMainViewShell().get() != NULL)
        nViewID = GetViewShellBase().GetMainViewShell()->mpImpl->GetViewId();
    rSequence[nIndex].Name = rtl::OUString( sUNO_View_ViewId );
    rtl::OUStringBuffer sBuffer( "view" );
    sBuffer.append( static_cast<sal_Int32>(nViewID));
    rSequence[nIndex].Value <<= sBuffer.makeStringAndClear();

    mpFrameView->WriteUserDataSequence( rSequence, bBrowse );
}


void ViewShell::ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    mpFrameView->ReadUserDataSequence( rSequence, bBrowse );
}

void ViewShell::VisAreaChanged(const Rectangle& rRect)
{
    OSL_ASSERT (GetViewShell()!=NULL);
    GetViewShell()->VisAreaChanged(rRect);
}

void ViewShell::SetWinViewPos(const Point& rWinPos, bool bUpdate)
{
    if (mpContentWindow.get() != NULL)
    {
        mpContentWindow->SetWinViewPos(rWinPos);

        if ( bUpdate )
        {
            mpContentWindow->UpdateMapOrigin();
            mpContentWindow->Invalidate();
        }
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

void ViewShell::AdaptDefaultsForChart(
    const uno::Reference < embed::XEmbeddedObject > & xEmbObj )
{
    if( xEmbObj.is())
    {
        uno::Reference< chart2::XChartDocument > xChartDoc( xEmbObj->getComponent(), uno::UNO_QUERY );
        OSL_ENSURE( xChartDoc.is(), "Trying to set chart property to non-chart OLE" );
        if( !xChartDoc.is())
            return;

        try
        {
            // set background to transparent (none)
            uno::Reference< beans::XPropertySet > xPageProp( xChartDoc->getPageBackground());
            if( xPageProp.is())
                xPageProp->setPropertyValue( "FillStyle" , uno::makeAny( drawing::FillStyle_NONE ));
            // set no border
            if( xPageProp.is())
                xPageProp->setPropertyValue( "LineStyle" , uno::makeAny( drawing::LineStyle_NONE ));
        }
        catch( const uno::Exception & )
        {
            OSL_FAIL( "Exception caught in AdaptDefaultsForChart" );
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
