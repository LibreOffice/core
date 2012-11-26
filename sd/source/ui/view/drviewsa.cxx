/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"
#include <cppuhelper/implbase1.hxx>
#include <comphelper/processfactory.hxx>
#ifndef _SVX_SIZEITEM
#include <editeng/sizeitem.hxx>
#endif
#include <svx/svdlayer.hxx>
#ifndef _SVX_ZOOMITEM
#include <svx/zoomitem.hxx>
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

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/clipfmtitem.hxx>


#include <sfx2/viewfrm.hxx>
#include <svx/fmshell.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/cliplistener.hxx>
#include <svx/float3d.hxx>
#include "helpids.h"

#include "view/viewoverlaymanager.hxx"
#include "app.hrc"
#include "helpids.h"
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
#include "SdUnoDrawView.hxx"
#include "slideshow.hxx"
#include "ToolBarManager.hxx"
#include "annotationmanager.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {
static const ::rtl::OUString MASTER_VIEW_TOOL_BAR_NAME(
    ::rtl::OUString::createFromAscii("masterviewtoolbar"));
}

namespace sd {

bool DrawViewShell::mbPipette = false;

// ------------------------
// - ScannerEventListener -
// ------------------------

class ScannerEventListener : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
{
private:

    DrawViewShell*      mpParent;

public:

                            ScannerEventListener( DrawViewShell* pParent ) : mpParent( pParent )  {};
                            ~ScannerEventListener();

    // XEventListener
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& rEventObject ) throw (::com::sun::star::uno::RuntimeException);

    void                    ParentDestroyed() { mpParent = NULL; }
};

// -----------------------------------------------------------------------------

ScannerEventListener::~ScannerEventListener()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL ScannerEventListener::disposing( const ::com::sun::star::lang::EventObject& rEventObject ) throw (::com::sun::star::uno::RuntimeException)
{
    if( mpParent )
        mpParent->ScannerEvent( rEventObject );
}

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

DrawViewShell::DrawViewShell( SfxViewFrame* pFrame, ViewShellBase& rViewShellBase, ::Window* pParentWindow, PageKind ePageKind, FrameView* pFrameViewArgument )
: ViewShell (pFrame, pParentWindow, rViewShellBase)
, maTabControl(this, pParentWindow)
, mbIsInSwitchPage(false)
{
    if (pFrameViewArgument != NULL)
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView(*GetDoc());
    Construct(GetDocSh(), ePageKind);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

DrawViewShell::~DrawViewShell()
{
    mpAnnotationManager.reset();
    mpViewOverlayManager.reset();

    OSL_ASSERT (GetViewShell()!=NULL);

    if( mxScannerListener.is() )
        static_cast< ScannerEventListener* >( mxScannerListener.get() )->ParentDestroyed();

    // #96642# Remove references to items within Svx3DWin
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
    sal_uInt32 aPageCnt = GetDoc()->GetSdPageCount(mePageKind);

    for (sal_uInt32 i = 0; i < aPageCnt; i++)
    {
        pPage = GetDoc()->GetSdPage(i, mePageKind);

        if (pPage == mpActualPage)
        {
            GetDoc()->SetSelected(pPage, true);
        }
        else
        {
            GetDoc()->SetSelected(pPage, false);
        }
    }

    if ( mpClipEvtLstnr )
    {
        mpClipEvtLstnr->AddRemoveListener( GetActiveWindow(), false );
        mpClipEvtLstnr->ClearCallbackLink();        // #103849# prevent callback if another thread is waiting
        mpClipEvtLstnr->release();
    }

    delete mpDrawView;
    // Set mpView to NULL so that the destructor of the ViewShell base class
    // does not access it.
    mpView = mpDrawView = NULL;

    mpFrameView->Disconnect();
    delete [] mpSlotArray;
}

/*************************************************************************
|*
|* gemeinsamer Initialisierungsanteil der beiden Konstruktoren
|*
\************************************************************************/

void DrawViewShell::Construct(DrawDocShell* pDocSh, PageKind eInitialPageKind)
{
    mpActualPage = 0;
    mbMousePosFreezed = false;
    mbReadOnly = GetDocSh()->IsReadOnly();
    mpSlotArray = 0;
    mpClipEvtLstnr = 0;
    mbPastePossible = false;
    mbIsLayerModeActive = false;

    mpFrameView->Connect();

    OSL_ASSERT (GetViewShell()!=NULL);

    // Array fuer Slot-/ImageMapping:
    // Gerader Eintrag: Haupt-/ToolboxSlot
    // Ungerader Eintrag: gemappter Slot
    // Achtung: Anpassen von GetIdBySubId() !!!
    // Reihenfolge (insbesondere Zoom) darf nicht geaendert werden !!!
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

    SetPool( &GetDoc()->GetItemPool() );

    GetDoc()->CreateFirstPages();

    mpDrawView = new DrawView(pDocSh, GetActiveWindow(), this);
    mpView = mpDrawView;             // Pointer der Basisklasse ViewShell
    mpDrawView->SetSwapAsynchron(true); // Asynchrones Laden von Graphiken

    // We do not read the page kind from the frame view anymore so we have
    // to set it in order to resync frame view and this view.
    mpFrameView->SetPageKind(eInitialPageKind);
    mePageKind = eInitialPageKind;
    meEditMode = EM_PAGE;
    DocumentType eDocType = GetDoc()->GetDocumentType(); // RTTI fasst hier noch nicht
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

    const basegfx::B2DVector& rPageScale = GetDoc()->GetSdPage(0, mePageKind)->GetPageScale();
    const basegfx::B2DPoint aPageOrg( rPageScale.getX(), rPageScale.getY() * 0.5);
    const basegfx::B2DVector aViewSize(rPageScale.getX() * 3.0, rPageScale.getY() * 2.0);

    InitWindows(aPageOrg, aViewSize, basegfx::B2DPoint(-1.0, -1.0));
    basegfx::B2DPoint aVisAreaPos(0.0, 0.0);

    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        const Point aOld(pDocSh->GetVisArea(ASPECT_CONTENT).TopLeft());
        aVisAreaPos = basegfx::B2DPoint(aOld.X(), aOld.Y());
    }

    const basegfx::B2DPoint aWATopLeft(-aVisAreaPos - aPageOrg);
    mpDrawView->SetWorkArea(basegfx::B2DRange(aWATopLeft, aWATopLeft + aViewSize));

    // Objekte koennen max. so gross wie die ViewSize werden
    GetDoc()->SetMaxObjectScale(aViewSize);

    // Split-Handler fuer TabControls
    maTabControl.SetSplitHdl( LINK( this, DrawViewShell, TabSplitHdl ) );

    // Damit der richtige EditMode von der FrameView komplett eingestellt
    // werden kann, wird hier ein aktuell anderer gewaehlt (kleiner Trick)
    if (mpFrameView->GetViewShEditMode(mePageKind) == EM_PAGE)
    {
        meEditMode = EM_MASTERPAGE;
    }
    else
    {
        meEditMode = EM_PAGE;
    }

    // Einstellungen der FrameView uebernehmen
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

            // AutoLayouts muessen erzeugt sein
            GetDoc()->StopWorkStartupDelay();
        }
        else if (mePageKind == PK_HANDOUT)
        {
            SetHelpId( SID_HANDOUTMODE );
            GetActiveWindow()->SetHelpId( CMD_SID_HANDOUTMODE );
            GetActiveWindow()->SetUniqueId( CMD_SID_HANDOUTMODE );

            // AutoLayouts muessen erzeugt sein
            GetDoc()->StopWorkStartupDelay();
        }
        else
        {
            SetHelpId( SD_IF_SDDRAWVIEWSHELL );
            GetActiveWindow()->SetHelpId( HID_SDDRAWVIEWSHELL );
            GetActiveWindow()->SetUniqueId( HID_SDDRAWVIEWSHELL );
        }
    }

    // Selektionsfunktion starten
    SfxRequest aReq(SID_OBJECT_SELECT, 0, GetDoc()->GetItemPool());
    FuPermanent(aReq);
    mpDrawView->SetFrameHandles(true);

    if (pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)
    {
        mbZoomOnPage = false;
    }
    else
    {
        mbZoomOnPage = true;
    }

    mbIsRulerDrag = false;

    String aName( RTL_CONSTASCII_USTRINGPARAM("DrawViewShell"));
    SetName (aName);

    mnLockCount = 0UL;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );

    if( xMgr.is() )
    {
        mxScannerManager = ::com::sun::star::uno::Reference< ::com::sun::star::scanner::XScannerManager >(
                           xMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.scanner.ScannerManager" ) ),
                           ::com::sun::star::uno::UNO_QUERY );

        if( mxScannerManager.is() )
        {
            mxScannerListener = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >(
                                static_cast< ::cppu::OWeakObject* >( new ScannerEventListener( this ) ),
                                ::com::sun::star::uno::UNO_QUERY );
        }
    }

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
                GetViewShellBase().GetDrawController(),
                *this,
                *GetView()));
    }

    return xSubController;
}




bool DrawViewShell::RelocateToParentWindow (::Window* pParentWindow)
{
    // DrawViewShells can not be relocated to a new parent window at the
    // moment, so return <false/> except when the given parent window is the
    // parent window that is already in use.
    return pParentWindow==GetParentWindow();
}




/*************************************************************************
|*
|* pruefe ob linienzuege gezeichnet werden muessen
|*
\************************************************************************/

/*
    linienzuege werden ueber makros als folge von
        MoveTo (x, y)
        LineTo (x, y)   [oder BezierTo (x, y)]
        LineTo (x, y)
            :
    dargestellt. einen endbefehl fuer die linienzuege
    gibt es nicht, also muessen alle befehle in den
    requests nach LineTo (BezierTo) abgetestet und die
    punktparameter gesammelt werden.
    der erste nicht-LineTo fuehrt dann dazu, dass aus
    den gesammelten punkten der linienzug erzeugt wird
*/

void DrawViewShell::CheckLineTo(SfxRequest& rReq)
{
    (void)rReq;
#ifdef DBG_UTIL
    if(rReq.IsAPI())
    {
        if(SID_LINETO == rReq.GetSlot() || SID_BEZIERTO == rReq.GetSlot() || SID_MOVETO == rReq.GetSlot() )
        {
            DBG_ERROR("DrawViewShell::CheckLineTo: slots SID_LINETO, SID_BEZIERTO, SID_MOVETO no longer supported.");
        }
    }
#endif

    rReq.Ignore ();
}

/*************************************************************************
|*
|* veraendere die seitemparameter, wenn SID_PAGESIZE oder SID_PAGEMARGIN
|*
\************************************************************************/

void DrawViewShell::SetupPage (
    const basegfx::B2DVector& rSize,
    double fLeft,
    double fRight,
    double fTop,
    double fBottom,
    bool bSize,
    bool bMargin,
    bool bScaleAll)
{
    sal_uInt32 nPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);
    sal_uInt32 i;

    for (i = 0; i < nPageCnt; i++)
    {
        /**********************************************************************
        * Erst alle MasterPages bearbeiten
        **********************************************************************/
        SdPage *pPage = GetDoc()->GetMasterSdPage(i, mePageKind);

        if( pPage )
        {
            if( bSize )
            {
                pPage->ScaleObjects(rSize, fLeft, fTop, fRight, fBottom, bScaleAll);
                pPage->SetPageScale(rSize);

            }
            if( bMargin )
            {
                pPage->SetLeftPageBorder(fLeft);
                pPage->SetRightPageBorder(fRight);
                pPage->SetTopPageBorder(fTop);
                pPage->SetBottomPageBorder(fBottom);
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
        /**********************************************************************
        * Danach alle Pages bearbeiten
        **********************************************************************/
        SdPage *pPage = GetDoc()->GetSdPage(i, mePageKind);

        if( pPage )
        {
            if( bSize )
            {
                pPage->ScaleObjects(rSize, fLeft, fTop, fRight, fBottom, bScaleAll);
                pPage->SetPageScale(rSize);
            }
            if( bMargin )
            {
                pPage->SetLeftPageBorder(fLeft);
                pPage->SetRightPageBorder(fRight);
                pPage->SetTopPageBorder(fTop);
                pPage->SetBottomPageBorder(fBottom);
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
        pHandoutPage->CreateTitleAndLayout(true);
    }

    const basegfx::B2DPoint aPageOrg(mpActualPage->GetPageScale().getX(), mpActualPage->GetPageScale().getY() * 0.5);
    const basegfx::B2DVector aPageSize(mpActualPage->GetPageScale().getX() * 3.0, mpActualPage->GetPageScale().getY() * 2.0);

    InitWindows(aPageOrg, aPageSize, basegfx::B2DPoint(-1.0, -1.0), true);

    basegfx::B2DPoint aVisAreaPos(0.0, 0.0);

    if ( GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        const Point aOld(GetDocSh()->GetVisArea(ASPECT_CONTENT).TopLeft());
        aVisAreaPos = basegfx::B2DPoint(aOld.X(), aOld.Y());
    }

    const basegfx::B2DPoint aWATopLeft(-aVisAreaPos - aPageOrg);
    GetView()->SetWorkArea(basegfx::B2DRange(aWATopLeft, aWATopLeft + aPageSize));

    UpdateScrollBars();

    if(GetView()->GetSdrPageView())
    {
        const basegfx::B2DPoint aNewOrigin(mpActualPage->GetLeftPageBorder(), mpActualPage->GetTopPageBorder());
        GetView()->GetSdrPageView()->SetPageOrigin(aNewOrigin);
    }

    GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);

    // auf (neue) Seitengroesse zoomen
    GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
|* Statuswerte der Statusbar zurueckgeben
|*
\************************************************************************/

void DrawViewShell::GetStatusBarState(SfxItemSet& rSet)
{
    // Zoom-Item
    // Hier sollte der entsprechende Wert (Optimal ?, Seitenbreite oder
    // Seite) mit Hilfe des ZoomItems weitergegeben werden !!!
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

            // Bereich einschraenken
            sal_uInt16 nZoomValues = SVX_ZOOM_ENABLE_ALL;
            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if( ( pPageView && pPageView->GetCurrentObjectList()->GetObjCount() == 0 ) )
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
                basegfx::B2DPoint aPagePos(0.0, 0.0);
                basegfx::B2DVector aPageScale(pPageView->getSdrPageFromSdrPageView().GetPageScale());

                aPagePos.setX(aPagePos.getX() + (aPageScale.getX() * 0.5));
                aPageScale.setX(aPageScale.getX() * 1.03);

                aPagePos.setY(aPagePos.getY() + (aPageScale.getY() * 0.5));
                aPageScale.setY(aPageScale.getY() * 1.03);
                aPagePos.setY(aPagePos.getY() - (aPageScale.getY() * 0.5));

                aPagePos.setX(aPagePos.getX() - (aPageScale.getX() * 0.5));

                const basegfx::B2DRange aFullPageZoomRange( aPagePos, aPagePos + aPageScale );
                aZoomItem.AddSnappingPoint( pActiveWindow->GetZoomForRange( aFullPageZoomRange ) );
            }
            aZoomItem.AddSnappingPoint(100);
            rSet.Put( aZoomItem );
        }
    }

    basegfx::B2DPoint aPos(GetActiveWindow()->GetInverseViewTransformation() * maMousePos);
    const double fUIScale(GetDoc()->GetUIScale());

    if(mpDrawView->GetSdrPageView())
    {
        aPos -= mpDrawView->GetSdrPageView()->GetPageOrigin();
    }

    aPos /= fUIScale;

    // Position- und Groesse-Items
    if ( mpDrawView->IsAction() )
    {
        basegfx::B2DRange aRange(mpDrawView->TakeActionRange());

        if(aRange.isEmpty())
        {
            rSet.Put(SfxPointItem(SID_ATTR_POSITION, Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY()))));
        }
        else
        {
            if(mpDrawView->GetSdrPageView())
            {
                aRange.transform(basegfx::tools::createTranslateB2DHomMatrix(-mpDrawView->GetSdrPageView()->GetPageOrigin()));
            }

            aPos = aRange.getMinimum() / fUIScale;
            rSet.Put(SfxPointItem(SID_ATTR_POSITION, Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY()))));

            const basegfx::B2DVector aScale(aRange.getRange() / fUIScale);
            rSet.Put(SvxSizeItem(SID_ATTR_SIZE, Size(basegfx::fround(aScale.getX()), basegfx::fround(aScale.getY()))));
        }
    }
    else
    {
        if ( mpDrawView->areSdrObjectsSelected() )
        {
            basegfx::B2DRange aRange(mpDrawView->getMarkedObjectSnapRange());

            if(mpDrawView->GetSdrPageView())
            {
                aRange.transform(basegfx::tools::createTranslateB2DHomMatrix(-mpDrawView->GetSdrPageView()->GetPageOrigin()));
            }

            // Show the position of the selected shape(s)
            const basegfx::B2DPoint aShapePosition(aRange.getMinimum() / fUIScale);
            rSet.Put(SfxPointItem(SID_ATTR_POSITION, Point(basegfx::fround(aShapePosition.getX()), basegfx::fround(aShapePosition.getY()))));

            const basegfx::B2DVector aScale(aRange.getRange() / fUIScale);
            rSet.Put(SvxSizeItem(SID_ATTR_SIZE, Size(basegfx::fround(aScale.getX()), basegfx::fround(aScale.getY()))));
        }
        else
        {
            rSet.Put(SfxPointItem(SID_ATTR_POSITION, Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY()))));
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
        }
    }

    // Display of current page and layer.
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STATUS_PAGE ) )
    {
        // Allways show the slide/page number.
        String aString (SdResId( STR_SD_PAGE ));
        aString += sal_Unicode(' ');
        aString += UniString::CreateFromInt32( maTabControl.GetCurPageId() );
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " / " ));
        aString += UniString::CreateFromInt32( GetDoc()->GetSdPageCount( mePageKind ) );

        // If in layer mode additionally show the layer that contains all
        // selected shapes of the page.  If the shapes are distributed on
        // more than one layer, no layer name is shown.
        if (IsLayerModeActive())
        {
            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetModelLayerAdmin();
            SdrLayerID nLayer = 0, nOldLayer = 0;
            SdrLayer*  pLayer = NULL;
            bool bOneLayer = true;
            const SdrObjectVector aSelection(mpDrawView->getSelectedSdrObjectVectorFromSdrMarkView());

            // Use the first ten selected shapes as a (hopefully
            // representative) sample of all shapes of the current page.
            // Detect whether they belong to the same layer.
            for( sal_uInt32 j = 0; j < aSelection.size() && bOneLayer && j < 10; j++ )
            {
                SdrObject* pObj = aSelection[j];
                nLayer = pObj->GetLayer();

                if( j != 0 && nLayer != nOldLayer )
                    bOneLayer = false;

                nOldLayer = nLayer;
            }

            // Append the layer name to the current page number.
            if( bOneLayer && mpDrawView->areSdrObjectsSelected() )
            {
                pLayer = rLayerAdmin.GetLayerPerID( nLayer );
                if( pLayer )
                {
                    aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " (" ));
                    aString += pLayer->GetName();
                    aString += sal_Unicode(')');
                }
            }
        }

        rSet.Put (SfxStringItem (SID_STATUS_PAGE, aString));
    }
    // Layout
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STATUS_LAYOUT ) )
    {
        String aString = mpActualPage->GetLayoutName();
        aString.Erase( aString.SearchAscii( SD_LT_SEPARATOR ) );
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
        if ((bool)GetDocSh()->IsReadOnly() != mbReadOnly )
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


} // end of namespace sd
