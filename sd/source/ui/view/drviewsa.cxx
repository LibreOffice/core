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


#include "DrawViewShell.hxx"
#include <com/sun/star/scanner/ScannerManager.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/processfactory.hxx>
#ifndef _SVX_SIZEITEM
#include <editeng/sizeitem.hxx>
#endif
#include <svx/svdlayer.hxx>
#ifndef _SVX_ZOOMITEM
#include <sfx2/zoomitem.hxx>
#endif
#include <svx/svdpagv.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopath.hxx>
#include <sfx2/docfile.hxx>
#include <svx/zoomslideritem.hxx>
#include <svl/eitem.hxx>

#include <svx/dialogs.hrc>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/clipfmtitem.hxx>


#include <sfx2/viewfrm.hxx>
#include <svx/fmshell.hxx>
#include <svtools/cliplistener.hxx>
#include <svx/float3d.hxx>
#include <svx/sidebar/SelectionAnalyzer.hxx>
#include "helpids.h"

#include "view/viewoverlaymanager.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdpage.hxx"
#include "FrameView.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "Window.hxx"
#include "fupoor.hxx"
#include "fusel.hxx"
#include "drawview.hxx"
#include "SdUnoDrawView.hxx"
#include "ViewShellBase.hxx"
#include "slideshow.hxx"
#include "ToolBarManager.hxx"
#include "annotationmanager.hxx"
#include "DrawController.hxx"

#include <boost/bind.hpp>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using sfx2::sidebar::EnumContext;

namespace sd {

sal_Bool DrawViewShell::mbPipette = sal_False;

// ------------------------
// - ScannerEventListener -
// ------------------------

class ScannerEventListener : public ::cppu::WeakImplHelper1< lang::XEventListener >
{
private:

    DrawViewShell*      mpParent;

public:

                            ScannerEventListener( DrawViewShell* pParent ) : mpParent( pParent )  {};
                            ~ScannerEventListener();

    // XEventListener
    virtual void SAL_CALL   disposing( const lang::EventObject& rEventObject ) throw (uno::RuntimeException);

    void                    ParentDestroyed() { mpParent = NULL; }
};

// -----------------------------------------------------------------------------

ScannerEventListener::~ScannerEventListener()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL ScannerEventListener::disposing( const lang::EventObject& rEventObject ) throw (uno::RuntimeException)
{
    if( mpParent )
        mpParent->ScannerEvent( rEventObject );
}


DrawViewShell::DrawViewShell( SfxViewFrame* pFrame, ViewShellBase& rViewShellBase, ::Window* pParentWindow, PageKind ePageKind, FrameView* pFrameViewArgument )
    : ViewShell (pFrame, pParentWindow, rViewShellBase)
    , maTabControl(this, pParentWindow)
    , mbIsLayerModeActive(false)
    , mbIsInSwitchPage(false)
    , mpSelectionChangeHandler(new svx::sidebar::SelectionChangeHandler(
          ::boost::bind(&DrawViewShell::GetSidebarContextName, this),
          uno::Reference<frame::XController>(&rViewShellBase.GetDrawController()),
          sfx2::sidebar::EnumContext::Context_Default))
{
    if (pFrameViewArgument != NULL)
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView(GetDoc());
    Construct(GetDocSh(), ePageKind);

    mpSelectionChangeHandler->Connect();

    SetContextName(GetSidebarContextName());

    doShow();
}


DrawViewShell::~DrawViewShell()
{
    mpSelectionChangeHandler->Disconnect();

    mpAnnotationManager.reset();
    mpViewOverlayManager.reset();

    OSL_ASSERT (GetViewShell()!=NULL);

    if( mxScannerListener.is() )
        static_cast< ScannerEventListener* >( mxScannerListener.get() )->ParentDestroyed();

    // Remove references to items within Svx3DWin
    // (maybe do a listening sometime in Svx3DWin)
    sal_uInt16 nId = Svx3DChildWindow::GetChildWindowId();
    SfxChildWindow* pWindow = GetViewFrame() ? GetViewFrame()->GetChildWindow(nId) : NULL;
    if(pWindow)
    {
        Svx3DWin* p3DWin = static_cast< Svx3DWin* > (pWindow->GetWindow());
        if(p3DWin)
            p3DWin->DocumentReload();
    }

    EndListening (*GetDoc());
    EndListening (*GetDocSh());

    if( SlideShow::IsRunning(*this) )
        StopSlideShow(false);

    DisposeFunctions();

    SdPage* pPage;
    sal_uInt16 aPageCnt = GetDoc()->GetSdPageCount(mePageKind);

    for (sal_uInt16 i = 0; i < aPageCnt; i++)
    {
        pPage = GetDoc()->GetSdPage(i, mePageKind);

        if (pPage == mpActualPage)
        {
            GetDoc()->SetSelected(pPage, sal_True);
        }
        else
        {
            GetDoc()->SetSelected(pPage, sal_False);
        }
    }

    if ( mpClipEvtLstnr )
    {
        mpClipEvtLstnr->AddRemoveListener( GetActiveWindow(), sal_False );
        mpClipEvtLstnr->ClearCallbackLink();        // prevent callback if another thread is waiting
        mpClipEvtLstnr->release();
    }

    delete mpDrawView;
    // Set mpView to NULL so that the destructor of the ViewShell base class
    // does not access it.
    mpView = mpDrawView = NULL;

    mpFrameView->Disconnect();
    delete [] mpSlotArray;
}

/**
 * common part of both constructors
 */
void DrawViewShell::Construct(DrawDocShell* pDocSh, PageKind eInitialPageKind)
{
    mpActualPage = 0;
    mbMousePosFreezed = sal_False;
    mbReadOnly = GetDocSh()->IsReadOnly();
    mpSlotArray = 0;
    mpClipEvtLstnr = 0;
    mbPastePossible = sal_False;
    mbIsLayerModeActive = false;

    mpFrameView->Connect();

    OSL_ASSERT (GetViewShell()!=NULL);

    /* array for slot-/image mapping:
       even entry: main-/toolbox slot
       odd entry:  mapped slot
       Attention: adjust GetIdBySubId() !!!
       Do not change order (especial zoom) !!! */
    mpSlotArray = new sal_uInt16[ SLOTARRAY_COUNT ];
    mpSlotArray[ 0 ]  = SID_OBJECT_CHOOSE_MODE;
    mpSlotArray[ 1 ]  = SID_OBJECT_ROTATE;
    mpSlotArray[ 2 ]  = SID_OBJECT_ALIGN;
    mpSlotArray[ 3 ]  = SID_OBJECT_ALIGN_LEFT;
    mpSlotArray[ 4 ]  = SID_ZOOM_TOOLBOX;
    mpSlotArray[ 5 ]  = SID_ZOOM_TOOLBOX;
    mpSlotArray[ 6 ]  = SID_DRAWTBX_TEXT;
    mpSlotArray[ 7 ]  = SID_ATTR_CHAR;
    mpSlotArray[ 8 ]  = SID_DRAWTBX_RECTANGLES;
    mpSlotArray[ 9 ]  = SID_DRAW_RECT;
    mpSlotArray[ 10 ] = SID_DRAWTBX_ELLIPSES;
    mpSlotArray[ 11 ] = SID_DRAW_ELLIPSE;
    mpSlotArray[ 12 ] = SID_DRAWTBX_LINES;
    mpSlotArray[ 13 ] = SID_DRAW_FREELINE_NOFILL;
    mpSlotArray[ 14 ] = SID_DRAWTBX_3D_OBJECTS;
    mpSlotArray[ 15 ] = SID_3D_CUBE;
    mpSlotArray[ 16 ] = SID_DRAWTBX_INSERT;
    mpSlotArray[ 17 ] = SID_INSERT_DIAGRAM;
    mpSlotArray[ 18 ] = SID_POSITION;
    mpSlotArray[ 19 ] = SID_FRAME_TO_TOP;
    mpSlotArray[ 20 ] = SID_DRAWTBX_CONNECTORS;
    mpSlotArray[ 21 ] = SID_TOOL_CONNECTOR;
    mpSlotArray[ 22 ] = SID_DRAWTBX_ARROWS;
    mpSlotArray[ 23 ] = SID_LINE_ARROW_END;

    SetPool( &GetDoc()->GetPool() );

    GetDoc()->CreateFirstPages();

    mpDrawView = new DrawView(pDocSh, GetActiveWindow(), this);
    mpView = mpDrawView;             // Pointer of base class ViewShell
    mpDrawView->SetSwapAsynchron(sal_True); // Asynchronous load of graphics

    // We do not read the page kind from the frame view anymore so we have
    // to set it in order to resync frame view and this view.
    mpFrameView->SetPageKind(eInitialPageKind);
    mePageKind = eInitialPageKind;
    meEditMode = EM_PAGE;
    DocumentType eDocType = GetDoc()->GetDocumentType(); // RTTI does not work here
    switch (mePageKind)
    {
        case PK_STANDARD:
            meShellType = ST_IMPRESS;
            break;

        case PK_NOTES:
            meShellType = ST_NOTES;
            break;

        case PK_HANDOUT:
            meShellType = ST_HANDOUT;
            break;
    }

    Size aPageSize( GetDoc()->GetSdPage(0, mePageKind)->GetSize() );
    Point aPageOrg( aPageSize.Width(), aPageSize.Height() / 2);
    Size aSize(aPageSize.Width() * 3, aPageSize.Height() * 2);
    InitWindows(aPageOrg, aSize, Point(-1, -1));

    Point aVisAreaPos;

    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        aVisAreaPos = pDocSh->GetVisArea(ASPECT_CONTENT).TopLeft();
    }

    mpDrawView->SetWorkArea(Rectangle(Point() - aVisAreaPos - aPageOrg, aSize));

    // objects can not grow bigger than ViewSize
    GetDoc()->SetMaxObjSize(aSize);

    // Split-Handler for TabControls
    maTabControl.SetSplitHdl( LINK( this, DrawViewShell, TabSplitHdl ) );

    /* In order to set the correct EditMode of the FrameView, we select another
       one (small trick).  */
    if (mpFrameView->GetViewShEditMode(mePageKind) == EM_PAGE)
    {
        meEditMode = EM_MASTERPAGE;
    }
    else
    {
        meEditMode = EM_PAGE;
    }

    // Use configuration of FrameView
    ReadFrameViewData(mpFrameView);

    if( eDocType == DOCUMENT_TYPE_DRAW )
    {
        SetHelpId( SD_IF_SDGRAPHICVIEWSHELL );
        GetActiveWindow()->SetHelpId( HID_SDGRAPHICVIEWSHELL );
        GetActiveWindow()->SetUniqueId( HID_SDGRAPHICVIEWSHELL );
    }
    else
    {
        if (mePageKind == PK_NOTES)
        {
            SetHelpId( SID_NOTESMODE );
            GetActiveWindow()->SetHelpId( CMD_SID_NOTESMODE );
            GetActiveWindow()->SetUniqueId( CMD_SID_NOTESMODE );

            // AutoLayouts have to be created
            GetDoc()->StopWorkStartupDelay();
        }
        else if (mePageKind == PK_HANDOUT)
        {
            SetHelpId( SID_HANDOUTMODE );
            GetActiveWindow()->SetHelpId( CMD_SID_HANDOUTMODE );
            GetActiveWindow()->SetUniqueId( CMD_SID_HANDOUTMODE );

            // AutoLayouts have to be created
            GetDoc()->StopWorkStartupDelay();
        }
        else
        {
            SetHelpId( SD_IF_SDDRAWVIEWSHELL );
            GetActiveWindow()->SetHelpId( HID_SDDRAWVIEWSHELL );
            GetActiveWindow()->SetUniqueId( HID_SDDRAWVIEWSHELL );
        }
    }

    // start selection function
    SfxRequest aReq(SID_OBJECT_SELECT, 0, GetDoc()->GetItemPool());
    FuPermanent(aReq);
    mpDrawView->SetFrameDragSingles(sal_True);

    if (pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)
    {
        mbZoomOnPage = sal_False;
    }
    else
    {
        mbZoomOnPage = sal_True;
    }

    mbIsRulerDrag = sal_False;

    OUString aName( "DrawViewShell" );
    SetName (aName);

    mnLockCount = 0UL;

    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    mxScannerManager = scanner::ScannerManager::create( xContext );

    mxScannerListener = uno::Reference< lang::XEventListener >(
                        static_cast< ::cppu::OWeakObject* >( new ScannerEventListener( this ) ),
                        uno::UNO_QUERY );

    mpAnnotationManager.reset( new AnnotationManager( GetViewShellBase() ) );
    mpViewOverlayManager.reset( new ViewOverlayManager( GetViewShellBase() ) );
}




void DrawViewShell::Init (bool bIsMainViewShell)
{
    ViewShell::Init(bIsMainViewShell);

    StartListening (*GetDocSh());
}




void DrawViewShell::Shutdown (void)
{
    ViewShell::Shutdown();

    if(SlideShow::IsRunning( GetViewShellBase() ) )
    {
        // Turn off effects.
        GetDrawView()->SetAnimationMode(SDR_ANIMATION_DISABLE);
    }
}




css::uno::Reference<css::drawing::XDrawSubController> DrawViewShell::CreateSubController (void)
{
    css::uno::Reference<css::drawing::XDrawSubController> xSubController;

    if (IsMainViewShell())
    {
        // Create uno sub controller for the main view shell.
        xSubController = css::uno::Reference<css::drawing::XDrawSubController>(
            new SdUnoDrawView (
                *this,
                *GetView()));
    }

    return xSubController;
}




bool DrawViewShell::RelocateToParentWindow (::Window* pParentWindow)
{
    // DrawViewShells can not be relocated to a new parent window at the
    // moment, so return <FALSE/> except when the given parent window is the
    // parent window that is already in use.
    return pParentWindow==GetParentWindow();
}




/**
 * check if we have to draw a polyline
 */

/*
    Polylines are represented by makros as a sequence of:
        MoveTo (x, y)
        LineTo (x, y)   [or BezierTo (x, y)]
        LineTo (x, y)
            :
    There is no end command for polylines. Therefore, we have to test all
    commands in the requests for LineTo (BezierTo) and we have to gather
    the point-parameter. The first not-LineTo leads to the creation of the
    polyline from the gathered points.
*/

void DrawViewShell::CheckLineTo(SfxRequest& rReq)
{
    (void)rReq;
#ifdef DBG_UTIL
    if(rReq.IsAPI())
    {
        if(SID_LINETO == rReq.GetSlot() || SID_BEZIERTO == rReq.GetSlot() || SID_MOVETO == rReq.GetSlot() )
        {
            OSL_FAIL("DrawViewShell::CheckLineTo: slots SID_LINETO, SID_BEZIERTO, SID_MOVETO no longer supported.");
        }
    }
#endif

    rReq.Ignore ();
}

/**
 * Change page parameter if SID_PAGESIZE or SID_PAGEMARGIN
 */
void DrawViewShell::SetupPage (Size &rSize,
                                 long nLeft,
                                 long nRight,
                                 long nUpper,
                                 long nLower,
                                 sal_Bool bSize,
                                 sal_Bool bMargin,
                                 sal_Bool bScaleAll)
{
    sal_uInt16 nPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);
    sal_uInt16 i;

    for (i = 0; i < nPageCnt; i++)
    {
        // first, handle all master pages
        SdPage *pPage = GetDoc()->GetMasterSdPage(i, mePageKind);

        if( pPage )
        {
            if( bSize )
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(rSize, aBorderRect, bScaleAll);
                pPage->SetSize(rSize);

            }
            if( bMargin )
            {
                pPage->SetLftBorder(nLeft);
                pPage->SetRgtBorder(nRight);
                pPage->SetUppBorder(nUpper);
                pPage->SetLwrBorder(nLower);
            }

            if ( mePageKind == PK_STANDARD )
            {
                GetDoc()->GetMasterSdPage(i, PK_NOTES)->CreateTitleAndLayout();
            }

            pPage->CreateTitleAndLayout();
        }
    }

    nPageCnt = GetDoc()->GetSdPageCount(mePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        // then, handle all pages
        SdPage *pPage = GetDoc()->GetSdPage(i, mePageKind);

        if( pPage )
        {
            if( bSize )
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(rSize, aBorderRect, bScaleAll);
                pPage->SetSize(rSize);
            }
            if( bMargin )
            {
                pPage->SetLftBorder(nLeft);
                pPage->SetRgtBorder(nRight);
                pPage->SetUppBorder(nUpper);
                pPage->SetLwrBorder(nLower);
            }

            if ( mePageKind == PK_STANDARD )
            {
                SdPage* pNotesPage = GetDoc()->GetSdPage(i, PK_NOTES);
                pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
            }

            pPage->SetAutoLayout( pPage->GetAutoLayout() );
        }
    }

    if ( mePageKind == PK_STANDARD )
    {
        SdPage* pHandoutPage = GetDoc()->GetSdPage(0, PK_HANDOUT);
        pHandoutPage->CreateTitleAndLayout(sal_True);
    }

    long nWidth = mpActualPage->GetSize().Width();
    long nHeight = mpActualPage->GetSize().Height();

    Point aPageOrg(nWidth, nHeight / 2);
    Size aSize( nWidth * 3, nHeight * 2);

    InitWindows(aPageOrg, aSize, Point(-1, -1), sal_True);

    Point aVisAreaPos;

    if ( GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        aVisAreaPos = GetDocSh()->GetVisArea(ASPECT_CONTENT).TopLeft();
    }

    GetView()->SetWorkArea(Rectangle(Point() - aVisAreaPos - aPageOrg, aSize));

    UpdateScrollBars();

    Point aNewOrigin(mpActualPage->GetLftBorder(), mpActualPage->GetUppBorder());
    GetView()->GetSdrPageView()->SetPageOrigin(aNewOrigin);

    GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);

    // zoom onto (new) page size
    GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}


void DrawViewShell::GetStatusBarState(SfxItemSet& rSet)
{
    /* Zoom-Item
       Here we should propagate the corresponding value (Optimal ?, page width
       or page) with the help of the ZoomItems !!!   */
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_ZOOM ) )
    {
        if (GetDocSh()->IsUIActive() || (SlideShow::IsRunning(GetViewShellBase())) )
        {
            rSet.DisableItem( SID_ATTR_ZOOM );
        }
        else
        {
            SvxZoomItem* pZoomItem;
            sal_uInt16 nZoom = (sal_uInt16) GetActiveWindow()->GetZoom();

            if( mbZoomOnPage )
                pZoomItem = new SvxZoomItem( SVX_ZOOM_WHOLEPAGE, nZoom );
            else
                pZoomItem = new SvxZoomItem( SVX_ZOOM_PERCENT, nZoom );

            // constrain area
            sal_uInt16 nZoomValues = SVX_ZOOM_ENABLE_ALL;
            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if( ( pPageView && pPageView->GetObjList()->GetObjCount() == 0 ) )
                // || ( mpDrawView->GetMarkedObjectList().GetMarkCount() == 0 ) )
            {
                nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
            }

            pZoomItem->SetValueSet( nZoomValues );
            rSet.Put( *pZoomItem );
            delete pZoomItem;
        }
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_ZOOMSLIDER ) )
    {
        rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( GetDoc() ) );
        if (GetDocSh()->IsUIActive() || (xSlideshow.is() && xSlideshow->isRunning()) || !GetActiveWindow() )
        {
            rSet.DisableItem( SID_ATTR_ZOOMSLIDER );
        }
        else
        {
            sd::Window * pActiveWindow = GetActiveWindow();
            SvxZoomSliderItem aZoomItem( (sal_uInt16) pActiveWindow->GetZoom(), (sal_uInt16)pActiveWindow->GetMinZoom(), (sal_uInt16)pActiveWindow->GetMaxZoom() ) ;

            SdrPageView* pPageView = mpDrawView->GetSdrPageView();
            if( pPageView )
            {
                Point aPagePos(0, 0);
                Size aPageSize = pPageView->GetPage()->GetSize();

                aPagePos.X() += aPageSize.Width()  / 2;
                aPageSize.Width() = (long) (aPageSize.Width() * 1.03);

                aPagePos.Y() += aPageSize.Height() / 2;
                aPageSize.Height() = (long) (aPageSize.Height() * 1.03);
                aPagePos.Y() -= aPageSize.Height() / 2;

                aPagePos.X() -= aPageSize.Width()  / 2;

                Rectangle aFullPageZoomRect( aPagePos, aPageSize );
                aZoomItem.AddSnappingPoint( pActiveWindow->GetZoomForRect( aFullPageZoomRect ) );
            }
            aZoomItem.AddSnappingPoint(100);
            rSet.Put( aZoomItem );
        }
    }

    Point aPos = GetActiveWindow()->PixelToLogic(maMousePos);
    mpDrawView->GetSdrPageView()->LogicToPagePos(aPos);
    Fraction aUIScale(GetDoc()->GetUIScale());
    aPos.X() = Fraction(aPos.X()) / aUIScale;
    aPos.Y() = Fraction(aPos.Y()) / aUIScale;

    // position- and size items
    if ( mpDrawView->IsAction() )
    {
        Rectangle aRect;
        mpDrawView->TakeActionRect( aRect );

        if ( aRect.IsEmpty() )
            rSet.Put( SfxPointItem(SID_ATTR_POSITION, aPos) );
        else
        {
            mpDrawView->GetSdrPageView()->LogicToPagePos(aRect);
            aPos = aRect.TopLeft();
            aPos.X() = Fraction(aPos.X()) / aUIScale;
            aPos.Y() = Fraction(aPos.Y()) / aUIScale;
            rSet.Put( SfxPointItem( SID_ATTR_POSITION, aPos) );
            Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
            aSize.Height() = Fraction(aSize.Height()) / aUIScale;
            aSize.Width()  = Fraction(aSize.Width())  / aUIScale;
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize) );
        }
    }
    else
    {
        if ( mpDrawView->AreObjectsMarked() )
        {
            Rectangle aRect = mpDrawView->GetAllMarkedRect();
            mpDrawView->GetSdrPageView()->LogicToPagePos(aRect);

            // Show the position of the selected shape(s)
            Point aShapePosition (aRect.TopLeft());
            aShapePosition.X() = Fraction(aShapePosition.X()) / aUIScale;
            aShapePosition.Y() = Fraction(aShapePosition.Y()) / aUIScale;
            rSet.Put (SfxPointItem(SID_ATTR_POSITION, aShapePosition));

            Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
            aSize.Height() = Fraction(aSize.Height()) / aUIScale;
            aSize.Width()  = Fraction(aSize.Width())  / aUIScale;
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize) );
        }
        else
        {
            rSet.Put( SfxPointItem(SID_ATTR_POSITION, aPos) );
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
        }
    }

    // Display of current page and layer.
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STATUS_PAGE ) )
    {
        sal_Int32 nPageCount = sal_Int32(GetDoc()->GetSdPageCount(mePageKind));
        sal_Int32 nActivePageCount = sal_Int32(GetDoc()->GetActiveSdPageCount());
        // Always show the slide/page number.
        OUString aOUString = SD_RESSTR(STR_SD_PAGE);
        aOUString += " ";
        aOUString += OUString::number( maTabControl.GetCurPageId() );
        aOUString += " / " ;
        aOUString += OUString::number( nPageCount );
        if (nPageCount != nActivePageCount)
        {
            aOUString += " (";
            aOUString += OUString::number( nActivePageCount );
            aOUString += ")";
        }

        // If in layer mode additionally show the layer that contains all
        // selected shapes of the page.  If the shapes are distributed on
        // more than one layer, no layer name is shown.
        if (IsLayerModeActive())
        {
            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            SdrLayerID nLayer = 0, nOldLayer = 0;
            SdrLayer*  pLayer = NULL;
            SdrObject* pObj = NULL;
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            sal_uLong nMarkCount = rMarkList.GetMarkCount();
            bool bOneLayer = true;

            // Use the first ten selected shapes as a (hopefully
            // representative) sample of all shapes of the current page.
            // Detect whether they belong to the same layer.
            for( sal_uLong j = 0; j < nMarkCount && bOneLayer && j < 10; j++ )
            {
                pObj = rMarkList.GetMark( j )->GetMarkedSdrObj();
                if( pObj )
                {
                    nLayer = pObj->GetLayer();

                    if( j != 0 && nLayer != nOldLayer )
                        bOneLayer = sal_False;

                    nOldLayer = nLayer;
                }
            }

            // Append the layer name to the current page number.
            if( bOneLayer && nMarkCount )
            {
                pLayer = rLayerAdmin.GetLayerPerID( nLayer );
                if( pLayer )
                {
                    aOUString += " (" ;
                    aOUString += pLayer->GetName();
                    aOUString += ")";
                }
            }
        }

        rSet.Put (SfxStringItem (SID_STATUS_PAGE, aOUString));
    }
    // Layout
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STATUS_LAYOUT ) )
    {
        OUString aString = mpActualPage->GetLayoutName();
        sal_Int32 nPos = aString.indexOf(SD_LT_SEPARATOR);
        if (nPos != -1)
            aString = aString.copy(0, nPos);
        rSet.Put( SfxStringItem( SID_STATUS_LAYOUT, aString ) );
    }
}



void DrawViewShell::Notify (SfxBroadcaster&, const SfxHint& rHint)
{
    const SfxSimpleHint* pSimple = dynamic_cast< const SfxSimpleHint* >(&rHint);
    if (pSimple!=NULL && pSimple->GetId()==SFX_HINT_MODECHANGED)
    {
        // Change to selection when turning on read-only mode.
        if(GetDocSh()->IsReadOnly() && dynamic_cast< FuSelection* >( GetCurrentFunction().get() ) )
        {
            SfxRequest aReq(SID_OBJECT_SELECT, 0, GetDoc()->GetItemPool());
            FuPermanent(aReq);
        }

        // Turn on design mode when document is not read-only.
        if (GetDocSh()->IsReadOnly() != mbReadOnly )
        {
            mbReadOnly = GetDocSh()->IsReadOnly();

            SfxBoolItem aItem( SID_FM_DESIGN_MODE, !mbReadOnly );
            GetViewFrame()->GetDispatcher()->Execute( SID_FM_DESIGN_MODE,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
        }
    }

}

void DrawViewShell::ExecuteAnnotation (SfxRequest& rRequest)
{
    if( mpAnnotationManager.get() )
        mpAnnotationManager->ExecuteAnnotation( rRequest );
}

// --------------------------------------------------------------------

void DrawViewShell::GetAnnotationState (SfxItemSet& rItemSet )
{
    if( mpAnnotationManager.get() )
        mpAnnotationManager->GetAnnotationState( rItemSet );
}




::rtl::OUString DrawViewShell::GetSidebarContextName (void) const
{
    ::svx::sidebar::SelectionAnalyzer::ViewType eViewType (::svx::sidebar::SelectionAnalyzer::VT_Standard);
    switch (mePageKind)
    {
        case PK_HANDOUT:
            eViewType = ::svx::sidebar::SelectionAnalyzer::VT_Handout;
            break;
        case PK_NOTES:
            eViewType = ::svx::sidebar::SelectionAnalyzer::VT_Notes;
            break;
        case PK_STANDARD:
            if (meEditMode == EM_MASTERPAGE)
                eViewType = ::svx::sidebar::SelectionAnalyzer::VT_Master;
            else
                eViewType = ::svx::sidebar::SelectionAnalyzer::VT_Standard;
            break;
    }
    return EnumContext::GetContextName(
        ::svx::sidebar::SelectionAnalyzer::GetContextForSelection_SD(
            mpDrawView->GetMarkedObjectList(),
            eViewType));
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
