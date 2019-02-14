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

#include <DrawViewShell.hxx>
#include <com/sun/star/scanner/ScannerManager.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/svdlayer.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/svdpagv.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopath.hxx>
#include <sfx2/docfile.hxx>
#include <svx/zoomslideritem.hxx>
#include <svl/eitem.hxx>

#include <sdcommands.h>
#include <svx/dialogs.hrc>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/clipfmtitem.hxx>

#include <sfx2/viewfrm.hxx>
#include <svx/fmshell.hxx>
#include <svtools/cliplistener.hxx>
#include <svx/float3d.hxx>
#include <svx/extedit.hxx>
#include <svx/sidebar/SelectionAnalyzer.hxx>
#include <svx/sidebar/SelectionChangeHandler.hxx>
#include <helpids.h>

#include <view/viewoverlaymanager.hxx>
#include <app.hrc>
#include <strings.hrc>
#include <sdmod.hxx>
#include <sdpage.hxx>
#include <FrameView.hxx>
#include <drawdoc.hxx>
#include <sdresid.hxx>
#include <DrawDocShell.hxx>
#include <Window.hxx>
#include <fupoor.hxx>
#include <fusel.hxx>
#include <funavig.hxx>
#include <drawview.hxx>
#include <SdUnoDrawView.hxx>
#include <ViewShellBase.hxx>
#include <slideshow.hxx>
#include <ToolBarManager.hxx>
#include <annotationmanager.hxx>
#include <DrawController.hxx>
#include <tools/diagnose_ex.h>
#include <LayerTabBar.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using vcl::EnumContext;

namespace sd {

bool DrawViewShell::mbPipette = false;


class ScannerEventListener : public ::cppu::WeakImplHelper< lang::XEventListener >
{
private:

    DrawViewShell*      mpParent;

public:

                            explicit ScannerEventListener( DrawViewShell* pParent ) : mpParent( pParent )  {}

    // XEventListener
    virtual void SAL_CALL   disposing( const lang::EventObject& rEventObject ) override;

    void                    ParentDestroyed() { mpParent = nullptr; }
};

void SAL_CALL ScannerEventListener::disposing( const lang::EventObject& /*rEventObject*/ )
{
    if( mpParent )
        mpParent->ScannerEvent();
}

DrawViewShell::DrawViewShell( ViewShellBase& rViewShellBase, vcl::Window* pParentWindow, PageKind ePageKind, FrameView* pFrameViewArgument )
    : ViewShell (pParentWindow, rViewShellBase)
    , maTabControl(VclPtr<sd::TabControl>::Create(this, pParentWindow))
    , mbIsLayerModeActive(false)
    , mbIsInSwitchPage(false)
    , mpSelectionChangeHandler(new svx::sidebar::SelectionChangeHandler(
          [this] () { return this->GetSidebarContextName(); },
          uno::Reference<frame::XController>(&rViewShellBase.GetDrawController()),
          vcl::EnumContext::Context::Default))
    , mbMouseButtonDown(false)
    , mbMouseSelecting(false)
{
    if (pFrameViewArgument != nullptr)
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView(GetDoc());
    Construct(GetDocSh(), ePageKind);

    mpSelectionChangeHandler->Connect();

    SetContextName(GetSidebarContextName());

    doShow();

    ConfigureAppBackgroundColor();
    SD_MOD()->GetColorConfig().AddListener(this);
}

DrawViewShell::~DrawViewShell()
{
    SD_MOD()->GetColorConfig().RemoveListener(this);

    mpSelectionChangeHandler->Disconnect();

    mpAnnotationManager.reset();
    mpViewOverlayManager.reset();

    OSL_ASSERT (GetViewShell()!=nullptr);

    if( mxScannerListener.is() )
        static_cast< ScannerEventListener* >( mxScannerListener.get() )->ParentDestroyed();

    // Remove references to items within Svx3DWin
    // (maybe do a listening sometime in Svx3DWin)
    sal_uInt16 nId = Svx3DChildWindow::GetChildWindowId();
    SfxChildWindow* pWindow = GetViewFrame() ? GetViewFrame()->GetChildWindow(nId) : nullptr;
    if(pWindow)
    {
        Svx3DWin* p3DWin = static_cast< Svx3DWin* > (pWindow->GetWindow());
        if(p3DWin)
            p3DWin->DocumentReload();
    }

    EndListening (*GetDoc());
    EndListening (*GetDocSh());

    if( SlideShow::IsRunning(*this) )
        StopSlideShow();

    DisposeFunctions();

    sal_uInt16 aPageCnt = GetDoc()->GetSdPageCount(mePageKind);

    for (sal_uInt16 i = 0; i < aPageCnt; i++)
    {
        SdPage* pPage = GetDoc()->GetSdPage(i, mePageKind);

        if (pPage == mpActualPage)
        {
            GetDoc()->SetSelected(pPage, true);
        }
        else
        {
            GetDoc()->SetSelected(pPage, false);
        }
    }

    if ( mxClipEvtLstnr.is() )
    {
        mxClipEvtLstnr->RemoveListener( GetActiveWindow() );
        mxClipEvtLstnr->ClearCallbackLink();        // prevent callback if another thread is waiting
        mxClipEvtLstnr.clear();
    }

    mpDrawView.reset();
    // Set mpView to NULL so that the destructor of the ViewShell base class
    // does not access it.
    mpView = nullptr;

    mpFrameView->Disconnect();
    maTabControl.disposeAndClear();
}

/**
 * common part of both constructors
 */
void DrawViewShell::Construct(DrawDocShell* pDocSh, PageKind eInitialPageKind)
{
    mpActualPage = nullptr;
    mbMousePosFreezed = false;
    mbReadOnly = GetDocSh()->IsReadOnly();
    mxClipEvtLstnr.clear();
    mbPastePossible = false;
    mbIsLayerModeActive = false;

    mpFrameView->Connect();

    OSL_ASSERT (GetViewShell()!=nullptr);

    SetPool( &GetDoc()->GetPool() );

    GetDoc()->CreateFirstPages();

    mpDrawView.reset( new DrawView(pDocSh, GetActiveWindow(), this) );
    mpView = mpDrawView.get();             // Pointer of base class ViewShell
    mpDrawView->SetSwapAsynchron(); // Asynchronous load of graphics

    // We do not read the page kind from the frame view anymore so we have
    // to set it in order to resync frame view and this view.
    mpFrameView->SetPageKind(eInitialPageKind);
    mePageKind = eInitialPageKind;
    meEditMode = EditMode::Page;
    DocumentType eDocType = GetDoc()->GetDocumentType(); // RTTI does not work here
    switch (mePageKind)
    {
        case PageKind::Standard:
            meShellType = ST_IMPRESS;
            break;

        case PageKind::Notes:
            meShellType = ST_NOTES;
            break;

        case PageKind::Handout:
            meShellType = ST_HANDOUT;
            break;
    }

    Size aPageSize( GetDoc()->GetSdPage(0, mePageKind)->GetSize() );
    Point aPageOrg( aPageSize.Width(), aPageSize.Height() / 2);
    Size aSize(aPageSize.Width() * 3, aPageSize.Height() * 2);
    InitWindows(aPageOrg, aSize, Point(-1, -1));

    Point aVisAreaPos;

    if ( pDocSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        aVisAreaPos = pDocSh->GetVisArea(ASPECT_CONTENT).TopLeft();
    }

    mpDrawView->SetWorkArea(::tools::Rectangle(Point() - aVisAreaPos - aPageOrg, aSize));

    // objects can not grow bigger than ViewSize
    GetDoc()->SetMaxObjSize(aSize);

    // Split-Handler for TabControls
    maTabControl->SetSplitHdl( LINK( this, DrawViewShell, TabSplitHdl ) );

    /* In order to set the correct EditMode of the FrameView, we select another
       one (small trick).  */
    if (mpFrameView->GetViewShEditMode(/*mePageKind*/) == EditMode::Page)
    {
        meEditMode = EditMode::MasterPage;
    }
    else
    {
        meEditMode = EditMode::Page;
    }

    // Use configuration of FrameView
    ReadFrameViewData(mpFrameView);

    if( eDocType == DocumentType::Draw )
    {
        GetActiveWindow()->SetHelpId( HID_SDGRAPHICVIEWSHELL );
    }
    else
    {
        if (mePageKind == PageKind::Notes)
        {
            GetActiveWindow()->SetHelpId( CMD_SID_NOTES_MODE );

            // AutoLayouts have to be created
            GetDoc()->StopWorkStartupDelay();
        }
        else if (mePageKind == PageKind::Handout)
        {
            GetActiveWindow()->SetHelpId( CMD_SID_HANDOUT_MASTER_MODE );

            // AutoLayouts have to be created
            GetDoc()->StopWorkStartupDelay();
        }
        else
        {
            GetActiveWindow()->SetHelpId( HID_SDDRAWVIEWSHELL );
        }
    }

    // start selection function
    SfxRequest aReq(SID_OBJECT_SELECT, SfxCallMode::SLOT, GetDoc()->GetItemPool());
    FuPermanent(aReq);
    mpDrawView->SetFrameDragSingles();

    if (pDocSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED)
    {
        mbZoomOnPage = false;
    }
    else
    {
        mbZoomOnPage = true;
    }

    mbIsRulerDrag = false;

    SetName ("DrawViewShell");

    mnLockCount = 0;

    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    try
    {
        mxScannerManager = scanner::ScannerManager::create( xContext );

        mxScannerListener.set( static_cast< ::cppu::OWeakObject* >( new ScannerEventListener( this ) ),
                               uno::UNO_QUERY );
    }
    catch (Exception const &)
    {
        // Eat the exception and log it
        // We can still continue if scanner manager is not available.
        DBG_UNHANDLED_EXCEPTION("sd");
    }

    mpAnnotationManager.reset( new AnnotationManager( GetViewShellBase() ) );
    mpViewOverlayManager.reset( new ViewOverlayManager( GetViewShellBase() ) );
}

void DrawViewShell::Init (bool bIsMainViewShell)
{
    ViewShell::Init(bIsMainViewShell);

    if (!IsListening(*GetDocSh()))
        StartListening (*GetDocSh());
}

void DrawViewShell::Shutdown()
{
    ViewShell::Shutdown();

    if(SlideShow::IsRunning( GetViewShellBase() ) )
    {
        // Turn off effects.
        GetDrawView()->SetAnimationMode(SdrAnimationMode::Disable);
    }
}

css::uno::Reference<css::drawing::XDrawSubController> DrawViewShell::CreateSubController()
{
    css::uno::Reference<css::drawing::XDrawSubController> xSubController;

    if (IsMainViewShell())
    {
        // Create uno sub controller for the main view shell.
        xSubController.set( new SdUnoDrawView( *this, *GetView()));
    }

    return xSubController;
}

bool DrawViewShell::RelocateToParentWindow (vcl::Window* pParentWindow)
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
    Polylines are represented by macros as a sequence of:
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
void DrawViewShell::SetupPage (Size const &rSize,
                                 long nLeft,
                                 long nRight,
                                 long nUpper,
                                 long nLower,
                                 bool bSize,
                                 bool bMargin,
                                 bool bScaleAll)
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
                ::tools::Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(rSize, aBorderRect, bScaleAll);
                pPage->SetSize(rSize);

            }
            if( bMargin )
            {
                pPage->SetLeftBorder(nLeft);
                pPage->SetRightBorder(nRight);
                pPage->SetUpperBorder(nUpper);
                pPage->SetLowerBorder(nLower);
            }

            if ( mePageKind == PageKind::Standard )
            {
                GetDoc()->GetMasterSdPage(i, PageKind::Notes)->CreateTitleAndLayout();
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
                ::tools::Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(rSize, aBorderRect, bScaleAll);
                pPage->SetSize(rSize);
            }
            if( bMargin )
            {
                pPage->SetLeftBorder(nLeft);
                pPage->SetRightBorder(nRight);
                pPage->SetUpperBorder(nUpper);
                pPage->SetLowerBorder(nLower);
            }

            if ( mePageKind == PageKind::Standard )
            {
                SdPage* pNotesPage = GetDoc()->GetSdPage(i, PageKind::Notes);
                pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
            }

            pPage->SetAutoLayout( pPage->GetAutoLayout() );
        }
    }

    if ( mePageKind == PageKind::Standard )
    {
        SdPage* pHandoutPage = GetDoc()->GetSdPage(0, PageKind::Handout);
        pHandoutPage->CreateTitleAndLayout(true);
    }

    long nWidth = mpActualPage->GetSize().Width();
    long nHeight = mpActualPage->GetSize().Height();

    Point aPageOrg(nWidth, nHeight / 2);
    Size aSize( nWidth * 3, nHeight * 2);

    InitWindows(aPageOrg, aSize, Point(-1, -1), true);

    Point aVisAreaPos;

    if ( GetDocSh()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        aVisAreaPos = GetDocSh()->GetVisArea(ASPECT_CONTENT).TopLeft();
    }

    GetView()->SetWorkArea(::tools::Rectangle(Point() - aVisAreaPos - aPageOrg, aSize));

    UpdateScrollBars();

    Point aNewOrigin(mpActualPage->GetLeftBorder(), mpActualPage->GetUpperBorder());
    GetView()->GetSdrPageView()->SetPageOrigin(aNewOrigin);

    GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);

    // zoom onto (new) page size
    GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE,
                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
}

void DrawViewShell::GetStatusBarState(SfxItemSet& rSet)
{
    /* Zoom-Item
       Here we should propagate the corresponding value (Optimal ?, page width
       or page) with the help of the ZoomItems !!!   */
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_ATTR_ZOOM ) )
    {
        if (GetDocSh()->IsUIActive() || (SlideShow::IsRunning(GetViewShellBase())) )
        {
            rSet.DisableItem( SID_ATTR_ZOOM );
        }
        else
        {
            std::unique_ptr<SvxZoomItem> pZoomItem;
            sal_uInt16 nZoom = static_cast<sal_uInt16>(GetActiveWindow()->GetZoom());

            if( mbZoomOnPage )
                pZoomItem.reset(new SvxZoomItem( SvxZoomType::WHOLEPAGE, nZoom ));
            else
                pZoomItem.reset(new SvxZoomItem( SvxZoomType::PERCENT, nZoom ));

            // constrain area
            SvxZoomEnableFlags nZoomValues = SvxZoomEnableFlags::ALL;
            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if( pPageView && pPageView->GetObjList()->GetObjCount() == 0 )
            {
                nZoomValues &= ~SvxZoomEnableFlags::OPTIMAL;
            }

            pZoomItem->SetValueSet( nZoomValues );
            rSet.Put( *pZoomItem );
        }
    }
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_ATTR_ZOOMSLIDER ) )
    {
        rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( GetDoc() ) );
        if (GetDocSh()->IsUIActive() || (xSlideshow.is() && xSlideshow->isRunning()) || !GetActiveWindow() )
        {
            rSet.DisableItem( SID_ATTR_ZOOMSLIDER );
        }
        else
        {
            sd::Window * pActiveWindow = GetActiveWindow();
            SvxZoomSliderItem aZoomItem( static_cast<sal_uInt16>(pActiveWindow->GetZoom()), static_cast<sal_uInt16>(pActiveWindow->GetMinZoom()), static_cast<sal_uInt16>(pActiveWindow->GetMaxZoom()) ) ;

            SdrPageView* pPageView = mpDrawView->GetSdrPageView();
            if( pPageView )
            {
                Point aPagePos(0, 0);
                Size aPageSize = pPageView->GetPage()->GetSize();

                aPagePos.AdjustX(aPageSize.Width()  / 2 );
                aPageSize.setWidth( static_cast<long>(aPageSize.Width() * 1.03) );

                aPagePos.AdjustY(aPageSize.Height() / 2 );
                aPageSize.setHeight( static_cast<long>(aPageSize.Height() * 1.03) );
                aPagePos.AdjustY( -(aPageSize.Height() / 2) );

                aPagePos.AdjustX( -(aPageSize.Width()  / 2) );

                ::tools::Rectangle aFullPageZoomRect( aPagePos, aPageSize );
                aZoomItem.AddSnappingPoint( pActiveWindow->GetZoomForRect( aFullPageZoomRect ) );
            }
            aZoomItem.AddSnappingPoint(100);
            rSet.Put( aZoomItem );
        }
    }

    SdrPageView* pPageView = mpDrawView->GetSdrPageView();
    if (pPageView)
    {
        Point aPos = GetActiveWindow()->PixelToLogic(maMousePos);
        pPageView->LogicToPagePos(aPos);
        Fraction aUIScale(GetDoc()->GetUIScale());
        aPos.setX( long(aPos.X() / aUIScale) );
        aPos.setY( long(aPos.Y() / aUIScale) );

        // position- and size items
        if ( mpDrawView->IsAction() )
        {
            ::tools::Rectangle aRect;
            mpDrawView->TakeActionRect( aRect );

            if ( aRect.IsEmpty() )
                rSet.Put( SfxPointItem(SID_ATTR_POSITION, aPos) );
            else
            {
                pPageView->LogicToPagePos(aRect);
                aPos = aRect.TopLeft();
                aPos.setX( long(aPos.X() / aUIScale) );
                aPos.setY( long(aPos.Y() / aUIScale) );
                rSet.Put( SfxPointItem( SID_ATTR_POSITION, aPos) );
                Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
                aSize.setHeight( long(aSize.Height() / aUIScale) );
                aSize.setWidth( long(aSize.Width()  / aUIScale) );
                rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize) );
            }
        }
        else
        {
            if ( mpDrawView->AreObjectsMarked() )
            {
                ::tools::Rectangle aRect = mpDrawView->GetAllMarkedRect();
                pPageView->LogicToPagePos(aRect);

                // Show the position of the selected shape(s)
                Point aShapePosition (aRect.TopLeft());
                aShapePosition.setX( long(aShapePosition.X() / aUIScale) );
                aShapePosition.setY( long(aShapePosition.Y() / aUIScale) );
                rSet.Put (SfxPointItem(SID_ATTR_POSITION, aShapePosition));

                Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
                aSize.setHeight( long(aSize.Height() / aUIScale) );
                aSize.setWidth( long(aSize.Width()  / aUIScale) );
                rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize) );
            }
            else
            {
                rSet.Put( SfxPointItem(SID_ATTR_POSITION, aPos) );
                rSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
            }
        }
    }

    // Display of current page and layer.
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_STATUS_PAGE ) )
    {
        sal_Int32 nPageCount = sal_Int32(GetDoc()->GetSdPageCount(mePageKind));
        sal_Int32 nActivePageCount = sal_Int32(GetDoc()->GetActiveSdPageCount());
        // Always show the slide/page number.
        OUString aOUString = (nPageCount == nActivePageCount) ? SdResId(STR_SD_PAGE_COUNT) : SdResId(STR_SD_PAGE_COUNT_CUSTOM);

        aOUString = aOUString.replaceFirst("%1", OUString::number(maTabControl->GetCurPagePos() + 1));
        aOUString = aOUString.replaceFirst("%2", OUString::number(nPageCount));
        if(nPageCount != nActivePageCount)
            aOUString = aOUString.replaceFirst("%3", OUString::number(nActivePageCount));

        // If in layer mode additionally show the layer that contains all
        // selected shapes of the page.  If the shapes are distributed on
        // more than one layer, no layer name is shown.
        if (IsLayerModeActive())
        {
            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            SdrLayerID nLayer(0), nOldLayer(0);
            SdrObject* pObj = nullptr;
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            const size_t nMarkCount = rMarkList.GetMarkCount();
            bool bOneLayer = true;

            // Use the first ten selected shapes as a (hopefully
            // representative) sample of all shapes of the current page.
            // Detect whether they belong to the same layer.
            for( size_t j = 0; j < nMarkCount && bOneLayer && j < 10; ++j )
            {
                pObj = rMarkList.GetMark( j )->GetMarkedSdrObj();
                if( pObj )
                {
                    nLayer = pObj->GetLayer();

                    if( j != 0 && nLayer != nOldLayer )
                        bOneLayer = false;

                    nOldLayer = nLayer;
                }
            }

            // Append the layer name to the current page number.
            if( bOneLayer && nMarkCount )
            {
                SdrLayer* pLayer = rLayerAdmin.GetLayerPerID( nLayer );
                if( pLayer )
                {
                    aOUString += " (" ;
                    aOUString += LayerTabBar::convertToLocalizedName(pLayer->GetName());
                    aOUString += ")";
                }
            }
        }

        rSet.Put (SfxStringItem (SID_STATUS_PAGE, aOUString));
    }
    // Layout
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_STATUS_LAYOUT ) )
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
    if (rHint.GetId()!=SfxHintId::ModeChanged)
        return;

    // Change to selection when turning on read-only mode.
    if(GetDocSh()->IsReadOnly() && dynamic_cast< FuSelection* >( GetCurrentFunction().get() ) )
    {
        SfxRequest aReq(SID_OBJECT_SELECT, SfxCallMode::SLOT, GetDoc()->GetItemPool());
        FuPermanent(aReq);
    }

    // Turn on design mode when document is not read-only.
    if (GetDocSh()->IsReadOnly() != mbReadOnly )
    {
        mbReadOnly = GetDocSh()->IsReadOnly();

        SfxBoolItem aItem( SID_FM_DESIGN_MODE, !mbReadOnly );
        GetViewFrame()->GetDispatcher()->ExecuteList(SID_FM_DESIGN_MODE,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });
    }

}

void DrawViewShell::ExecuteAnnotation (SfxRequest const & rRequest)
{
    if (mpAnnotationManager)
        mpAnnotationManager->ExecuteAnnotation( rRequest );
}

void DrawViewShell::GetAnnotationState (SfxItemSet& rItemSet )
{
    if (mpAnnotationManager)
        mpAnnotationManager->GetAnnotationState( rItemSet );
}

OUString const & DrawViewShell::GetSidebarContextName() const
{
    svx::sidebar::SelectionAnalyzer::ViewType eViewType (svx::sidebar::SelectionAnalyzer::ViewType::Standard);
    switch (mePageKind)
    {
        case PageKind::Handout:
            eViewType = svx::sidebar::SelectionAnalyzer::ViewType::Handout;
            break;
        case PageKind::Notes:
            eViewType = svx::sidebar::SelectionAnalyzer::ViewType::Notes;
            break;
        case PageKind::Standard:
            if (meEditMode == EditMode::MasterPage)
                eViewType = svx::sidebar::SelectionAnalyzer::ViewType::Master;
            else
                eViewType = svx::sidebar::SelectionAnalyzer::ViewType::Standard;
            break;
    }
    return EnumContext::GetContextName(
        svx::sidebar::SelectionAnalyzer::GetContextForSelection_SD(
            mpDrawView->GetMarkedObjectList(),
            eViewType));
}

void DrawViewShell::ExecGoToNextPage (SfxRequest& rReq)
{
    SetCurrentFunction( FuNavigation::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq) );
    Cancel();
}

void DrawViewShell::GetStateGoToNextPage (SfxItemSet& rSet)
{
    SdPage* pPage = GetActualPage();
    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;
    sal_uInt16 totalPages = GetDoc()->GetSdPageCount(pPage->GetPageKind());
    if (nSdPage + 1 >= totalPages)
        rSet.DisableItem( SID_GO_TO_NEXT_PAGE );
}

void DrawViewShell::ExecGoToPreviousPage (SfxRequest& rReq)
{
    SetCurrentFunction( FuNavigation::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq) );
    Cancel();
}

void DrawViewShell::GetStateGoToPreviousPage (SfxItemSet& rSet)
{
    SdPage* pPage = GetActualPage();
    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;
    if (nSdPage == 0)
        rSet.DisableItem( SID_GO_TO_PREVIOUS_PAGE );
}


void DrawViewShell::ExecGoToFirstPage (SfxRequest& rReq)
{
    SetCurrentFunction( FuNavigation::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq) );
    Cancel();
}

void DrawViewShell::GetStateGoToFirstPage (SfxItemSet& rSet)
{
    SdPage* pPage = GetActualPage();
    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;
    if (nSdPage == 0)
        rSet.DisableItem( SID_GO_TO_FIRST_PAGE );
}

void DrawViewShell::ExecGoToLastPage (SfxRequest& rReq)
{
    SetCurrentFunction( FuNavigation::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq) );
    Cancel();
}

void DrawViewShell::GetStateGoToLastPage (SfxItemSet& rSet)
{
    SdPage* pPage = GetActualPage();
    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;
    sal_uInt16 totalPages = GetDoc()->GetSdPageCount(pPage->GetPageKind());
    if (nSdPage + 1 >= totalPages)
        rSet.DisableItem( SID_GO_TO_LAST_PAGE );
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
