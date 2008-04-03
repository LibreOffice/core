/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drviewsa.cxx,v $
 *
 *  $Revision: 1.48 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:16:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SVX_SIZEITEM
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVDLAYER_HXX
#include <svx/svdlayer.hxx>
#endif
#ifndef _SVX_ZOOMITEM
#include <svx/zoomitem.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SVX_EXTRUSION_BAR_HXX
#include <svx/extrusionbar.hxx>
#endif
#ifndef _SVX_FONTWORK_BAR_HXX
#include <svx/fontworkbar.hxx>
#endif
#ifndef _SVX_CLIPFMTITEM_HXX
#include <svx/clipfmtitem.hxx>
#endif


#include <sfx2/viewfrm.hxx>

#ifndef _SVX_FMSHELL_HXX            // XXX nur temp (dg)
#include <svx/fmshell.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _CLIPLISTENER_HXX
#include <svtools/cliplistener.hxx>
#endif
#ifndef _SVX_FLOAT3D_HXX
#include <svx/float3d.hxx>
#endif

#include "app.hrc"
#include "helpids.h"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdpage.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif
#ifndef SD_FU_SELECTION_HXX
#include "fusel.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "SdUnoDrawView.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#include "SdUnoDrawView.hxx"
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif
#include "ToolBarManager.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {
static const ::rtl::OUString MASTER_VIEW_TOOL_BAR_NAME(
    ::rtl::OUString::createFromAscii("masterviewtoolbar"));
}

namespace sd {

BOOL DrawViewShell::mbPipette = FALSE;

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

DrawViewShell::DrawViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    ::Window* pParentWindow,
    PageKind ePageKind,
    FrameView* pFrameViewArgument)
    : ViewShell (pFrame, pParentWindow, rViewShellBase),
    maTabControl(this, pParentWindow)
{
    if (pFrameViewArgument != NULL)
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView(GetDoc());
    Construct(GetDocSh(), ePageKind);
}

/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

DrawViewShell::DrawViewShell (
    SfxViewFrame* pFrame,
    ::Window* pParentWindow,
    const DrawViewShell& rShell)
    : ViewShell(pFrame, pParentWindow, rShell),
      maTabControl(this, pParentWindow)
{
    mpFrameView = new FrameView(GetDoc());
    Construct (GetDocSh(), PK_STANDARD);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

DrawViewShell::~DrawViewShell()
{
    OSL_ASSERT (GetViewShell()!=NULL);

    if( mxScannerListener.is() )
        static_cast< ScannerEventListener* >( mxScannerListener.get() )->ParentDestroyed();

    // #96642# Remove references to items within Svx3DWin
    // (maybe do a listening sometime in Svx3DWin)
    USHORT nId = Svx3DChildWindow::GetChildWindowId();
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
    USHORT aPageCnt = GetDoc()->GetSdPageCount(mePageKind);

    for (USHORT i = 0; i < aPageCnt; i++)
    {
        pPage = GetDoc()->GetSdPage(i, mePageKind);

        if (pPage == mpActualPage)
        {
            GetDoc()->SetSelected(pPage, TRUE);
        }
        else
        {
            GetDoc()->SetSelected(pPage, FALSE);
        }
    }

    if ( mpClipEvtLstnr )
    {
        mpClipEvtLstnr->AddRemoveListener( GetActiveWindow(), FALSE );
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
    mbMousePosFreezed = FALSE;
    mbReadOnly = GetDocSh()->IsReadOnly();
    mpSlotArray = 0;
    mpClipEvtLstnr = 0;
    mbPastePossible = FALSE;
    mbIsLayerModeActive = false;

    mpFrameView->Connect();

    OSL_ASSERT (GetViewShell()!=NULL);

    // Array fuer Slot-/ImageMapping:
    // Gerader Eintrag: Haupt-/ToolboxSlot
    // Ungerader Eintrag: gemappter Slot
    // Achtung: Anpassen von GetIdBySubId() !!!
    // Reihenfolge (insbesondere Zoom) darf nicht geaendert werden !!!
    mpSlotArray = new USHORT[ SLOTARRAY_COUNT ];
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
    mpView = mpDrawView;             // Pointer der Basisklasse ViewShell
    mpDrawView->SetSwapAsynchron(TRUE); // Asynchrones Laden von Graphiken

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

    // Objekte koennen max. so gross wie die ViewSize werden
    GetDoc()->SetMaxObjSize(aSize);

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
        GetActiveWindow()->SetHelpId( SD_IF_SDGRAPHICVIEWSHELL );
        GetActiveWindow()->SetUniqueId( SD_IF_SDGRAPHICVIEWSHELL );
    }
    else
    {
        if (mePageKind == PK_NOTES)
        {
            SetHelpId( SID_NOTESMODE );
            GetActiveWindow()->SetHelpId( SID_NOTESMODE );
            GetActiveWindow()->SetUniqueId( SID_NOTESMODE );

            // AutoLayouts muessen erzeugt sein
            GetDoc()->StopWorkStartupDelay();
        }
        else if (mePageKind == PK_HANDOUT)
        {
            SetHelpId( SID_HANDOUTMODE );
            GetActiveWindow()->SetHelpId( SID_HANDOUTMODE );
            GetActiveWindow()->SetUniqueId( SID_HANDOUTMODE );

            // AutoLayouts muessen erzeugt sein
            GetDoc()->StopWorkStartupDelay();
        }
        else
        {
            SetHelpId( SD_IF_SDDRAWVIEWSHELL );
            GetActiveWindow()->SetHelpId( SD_IF_SDDRAWVIEWSHELL );
            GetActiveWindow()->SetUniqueId( SD_IF_SDDRAWVIEWSHELL );
        }
    }

    // Selektionsfunktion starten
    SfxRequest aReq(SID_OBJECT_SELECT, 0, GetDoc()->GetItemPool());
    FuPermanent(aReq);
    mpDrawView->SetFrameDragSingles(TRUE);

    if (pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)
    {
        mbZoomOnPage = FALSE;
    }
    else
    {
        mbZoomOnPage = TRUE;
    }

    mbIsRulerDrag = FALSE;

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
    // moment, so return <FALSE/> except when the given parent window is the
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

void DrawViewShell::SetupPage (Size &rSize,
                                 long nLeft,
                                 long nRight,
                                 long nUpper,
                                 long nLower,
                                 BOOL bSize,
                                 BOOL bMargin,
                                 BOOL bScaleAll)
{
    USHORT nPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);
    USHORT i;

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
        /**********************************************************************
        * Danach alle Pages bearbeiten
        **********************************************************************/
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
        pHandoutPage->CreateTitleAndLayout(TRUE);
    }

    long nWidth = mpActualPage->GetSize().Width();
    long nHeight = mpActualPage->GetSize().Height();

    Point aPageOrg(nWidth, nHeight / 2);
    Size aSize( nWidth * 3, nHeight * 2);

    InitWindows(aPageOrg, aSize, Point(-1, -1), TRUE);

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
            UINT16 nZoom = (UINT16) GetActiveWindow()->GetZoom();

            if( mbZoomOnPage )
                pZoomItem = new SvxZoomItem( SVX_ZOOM_WHOLEPAGE, nZoom );
            else
                pZoomItem = new SvxZoomItem( SVX_ZOOM_PERCENT, nZoom );

            // Bereich einschraenken
            USHORT nZoomValues = SVX_ZOOM_ENABLE_ALL;
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

    Point aPos = GetActiveWindow()->PixelToLogic(maMousePos);
    mpDrawView->GetSdrPageView()->LogicToPagePos(aPos);
    Fraction aUIScale(GetDoc()->GetUIScale());
    aPos.X() = Fraction(aPos.X()) / aUIScale;
    aPos.Y() = Fraction(aPos.Y()) / aUIScale;

    // Position- und Groesse-Items
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
            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            SdrLayerID nLayer = 0, nOldLayer = 0;
            SdrLayer*  pLayer = NULL;
            SdrObject* pObj = NULL;
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            ULONG nMarkCount = rMarkList.GetMarkCount();
            FASTBOOL bOneLayer = TRUE;

            // Use the first ten selected shapes as a (hopefully
            // representative) sample of all shapes of the current page.
            // Detect whether they belong to the same layer.
            for( ULONG j = 0; j < nMarkCount && bOneLayer && j < 10; j++ )
            {
                pObj = rMarkList.GetMark( j )->GetMarkedSdrObj();
                if( pObj )
                {
                    nLayer = pObj->GetLayer();

                    if( j != 0 && nLayer != nOldLayer )
                        bOneLayer = FALSE;

                    nOldLayer = nLayer;
                }
            }

            // Append the layer name to the current page number.
            if( bOneLayer && nMarkCount )
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
        if (GetDocSh()->IsReadOnly() != mbReadOnly )
        {
            mbReadOnly = GetDocSh()->IsReadOnly();

            SfxBoolItem aItem( SID_FM_DESIGN_MODE, !mbReadOnly );
            GetViewFrame()->GetDispatcher()->Execute( SID_FM_DESIGN_MODE,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
        }
    }

}


} // end of namespace sd
