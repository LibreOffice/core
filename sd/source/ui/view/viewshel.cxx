/*************************************************************************
 *
 *  $RCSfile: viewshel.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:20:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "ViewShell.hxx"
#include "ViewShellImplementation.hxx"

#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#include "DrawController.hxx"
#include "LayerTabBar.hxx"

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif
#ifndef _B3D_BASE3D_HXX
#include "goodies/base3d.hxx"
#endif
#ifndef _SVX_FMSHELL_HXX
#include <svx/fmshell.hxx>
#endif
#ifndef SD_WINDOW_UPDATER_HXX
#include "WindowUpdater.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif
#ifndef _SD_SDXFER_HXX
#include <sdxfer.hxx>
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif

#include "app.hrc"
#include "helpids.h"
#include "strings.hrc"
#include "res_bmp.hrc"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_CLIENT_HXX
#include "Client.hxx"
#endif
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "zoomlist.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "optsitem.hxx"
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif
#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#include "DrawObjectBar.hxx"
#include "ImpressObjectBar.hxx"
#include "BezierObjectBar.hxx"
#include "GluePointsObjectBar.hxx"
#include "TextObjectBar.hxx"
#include "GraphicObjectBar.hxx"
#include "MediaObjectBar.hxx"
#include "ViewShellManager.hxx"
#include <svx/fmshell.hxx>
#include "ViewTabBar.hxx"
#include <svx/dialogs.hrc>
#include <svx/extrusionbar.hxx>

// #96090#
#ifndef _SFXSLSTITM_HXX
#include <svtools/slstitm.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {
class ViewShellObjectBarFactory
    : public ::sd::ObjectBarManager::ObjectBarFactory
{
public:
    ViewShellObjectBarFactory (::sd::ViewShell& rViewShell);
    virtual ~ViewShellObjectBarFactory (void);
    virtual SfxShell* CreateShell (
        ::sd::ShellId nId,
        ::Window* pParentWindow,
        ::sd::FrameView* pFrameView);
    virtual void ReleaseShell (SfxShell* pShell);
private:
    ::sd::ViewShell& mrViewShell;
    /** This cache holds the already created object bars.
    */
    typedef ::std::hash_map< ::sd::ShellId,SfxShell*> ShellCache;
    ShellCache maShellCache;
};
}


namespace sd {

static const int DELTA_ZOOM = 10;

// Use magenta as transparency color instead of the standard gray that is
// imported from vcl/image.hxx
#undef IMAGE_STDBTN_COLOR
#undef IMAGE_STDBTN_COLOR_HC
#define IMAGE_STDBTN_COLOR Color(0xff,0x00,0xff)
#define IMAGE_STDBTN_COLOR_HC Color(0xff,0x00,0xff)


SfxViewFrame* ViewShell::GetViewFrame (void) const
{
    OSL_ASSERT (GetViewShell()!=NULL);
    return GetViewShell()->GetViewFrame();
}


/*************************************************************************
|*
|* SFX-Slotmap und Standardinterface deklarieren
|*
\************************************************************************/
TYPEINIT1(ViewShell, SfxShell);


ViewShell::ViewShell (
    SfxViewFrame* pFrame,
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase,
    bool bAllowCenter)
    : SfxShell (&rViewShellBase),
      mpContentWindow(NULL),
      mpHorizontalScrollBar(NULL),
      mpVerticalScrollBar(NULL),
      mpHorizontalRuler(NULL),
      mpVerticalRuler(NULL),
      mpScrollBarBox(NULL),
      mbHasRulers(false),
      mpActiveWindow(NULL),
      mpView(NULL),
      pFrameView(NULL),
      pFuActual(NULL),
      pFuOld(NULL),
      pFuSearch(NULL),
      pFuSlideShow(NULL),
      pZoomList(NULL),
      aViewPos(),
      aViewSize(),
      aScrBarWH(),
      bCenterAllowed(bAllowCenter),
      bStartShowWithDialog(FALSE),
      nPrintedHandoutPageNum(1),
      maAllWindowRectangle(),
      meShellType(ST_NONE),
      mpController(NULL),
      mpParentWindow(pParentWindow),
      mpObjectBarManager(NULL),
      mpWindowUpdater (new ::sd::WindowUpdater()),
      mpImpl(new Implementation(*this))
{
    Construct();
}




ViewShell::ViewShell(
    SfxViewFrame* pFrame,
    ::Window* pParentWindow,
    const ViewShell& rShell)
    : SfxShell (rShell.GetViewShell()),
      mpContentWindow(NULL),
      mpHorizontalScrollBar(NULL),
      mpVerticalScrollBar(NULL),
      mpHorizontalRuler(NULL),
      mpVerticalRuler(NULL),
      mpScrollBarBox(NULL),
      mbHasRulers(false),
      mpActiveWindow(NULL),
      mpView(NULL),
      pFrameView(NULL),
      pFuActual(NULL),
      pFuOld(NULL),
      pFuSearch(NULL),
      pFuSlideShow(NULL),
      pZoomList(NULL),
      aViewPos(),
      aViewSize(),
      aScrBarWH(),
      bCenterAllowed(rShell.bCenterAllowed),
      bStartShowWithDialog(FALSE),
      nPrintedHandoutPageNum(1),
      maAllWindowRectangle(),
      meShellType(ST_NONE),
      mpController(NULL),
      mpParentWindow(pParentWindow),
      mpObjectBarManager(NULL),
      mpWindowUpdater (new ::sd::WindowUpdater()),
      mpImpl(new Implementation(*this))
{
    Construct();
}




ViewShell::~ViewShell()
{
    SetActiveWindow (NULL);

    // The sub shell manager will be destroyed in a short time.
    // Disable the switching of object bars now anyway just in case
    // the object bars would access invalid data when switched.
    GetObjectBarManager().DisableObjectBarSwitching();

    // Dispose the controller of this sub-shell.  The disposing event sent
    // to the main controller will result in it asking the sub shell manager
    // for the controller of the new main sub-shell.
    Reference<lang::XComponent> xComponent (mpController.getRef(), UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);
    CancelSearching();

    // Stop listening for window events.
    GetParentWindow()->RemoveEventListener (
        LINK(this,ViewShell,FrameWindowEventListener));

    if (IsMainViewShell())
        GetDocSh()->Disconnect(this);

    delete pZoomList;
}


/*************************************************************************
|*
|* gemeinsamer Initialiserungsanteil der beiden Konstruktoren
|*
\************************************************************************/

void ViewShell::Construct(void)
{
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);


    if (IsMainViewShell())
        GetDocSh()->Connect (this);

    pZoomList = new ZoomList( this );

    mpContentWindow.reset(new ::sd::Window(GetParentWindow()));
    SetActiveWindow (mpContentWindow.get());

    GetParentWindow()->SetBackground (Wallpaper());
    mpContentWindow->SetBackground (Wallpaper());
    mpContentWindow->SetCenterAllowed(bCenterAllowed);
    mpContentWindow->SetViewShell(this);
    mpContentWindow->Show();

    // Create scroll bars and the filler between the scroll bars.
    mpHorizontalScrollBar.reset (new ScrollBar(
        GetParentWindow(),
        WinBits(WB_HSCROLL | WB_DRAG)));
    mpHorizontalScrollBar->EnableRTL (FALSE);
    mpHorizontalScrollBar->SetRange(Range(0, 32000));
    mpHorizontalScrollBar->SetScrollHdl(LINK(this, ViewShell, HScrollHdl));
    mpHorizontalScrollBar->Show();

    mpVerticalScrollBar.reset (new ScrollBar(
        GetParentWindow(),
        WinBits(WB_VSCROLL | WB_DRAG)));
    mpVerticalScrollBar->SetRange(Range(0, 32000));
    mpVerticalScrollBar->SetScrollHdl(LINK(this, ViewShell, VScrollHdl));
    mpVerticalScrollBar->Show();

    aScrBarWH = Size(mpVerticalScrollBar->GetSizePixel().Width(),
                     mpHorizontalScrollBar->GetSizePixel().Height());

    mpScrollBarBox.reset(new ScrollBarBox(
        GetParentWindow(),
        WB_SIZEABLE));
    mpScrollBarBox->Show();

    String aName( RTL_CONSTASCII_USTRINGPARAM( "ViewShell" ));
    SetName (aName);

    GetDoc()->StartOnlineSpelling(FALSE);

    mpWindowUpdater->SetViewShell (*this);
    mpWindowUpdater->SetDocument (GetDoc());

    // Set up the object bar manager.
    mpObjectBarManager = ::std::auto_ptr<ObjectBarManager> (
        new ObjectBarManager (*this));
    GetObjectBarManager().RegisterDefaultFactory (
        ::std::auto_ptr<ObjectBarManager::ObjectBarFactory>(
            new ViewShellObjectBarFactory(*this)));
    GetObjectBarManager().Clear ();
    GetObjectBarManager().EnableObjectBarSwitching();
}

void ViewShell::Init (void)
{
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);
}




void ViewShell::Exit (void)
{
    Deactivate (TRUE);

    // Enable object bar switching so that Clear() deactivates every object
    // bar cleanly.
    GetObjectBarManager().EnableObjectBarSwitching();
    GetObjectBarManager().Clear();
}


/*************************************************************************
|*
|* Diese Methode deaktiviert und loescht die aktuelle Funktion. Falls es
|* eine alte Funktion gibt, wird sie als aktuelle Funktion eingesetzt und
|* aktiviert.
|*
\************************************************************************/

void ViewShell::Cancel()
{
    if (pFuActual && pFuActual != pFuOld)
    {
        FuPoor* pTemp = pFuActual;
        pFuActual     = NULL;
        pTemp->Deactivate();
        delete pTemp;
    }

    if (pFuOld)
    {
        pFuActual = pFuOld;
        pFuActual->Activate();
    }
}

/*************************************************************************
|*
|* Aktivierung: Arbeitsfenster den Fokus zuweisen
|*
\************************************************************************/

void ViewShell::Activate(BOOL bIsMDIActivate)
{
    SfxShell::Activate(bIsMDIActivate);

    // Laut MI darf keiner GrabFocus rufen, der nicht genau weiss von
    // welchem Window der Focus gegrabt wird. Da Activate() vom SFX teilweise
    // asynchron verschickt wird, kann es sein, dass ein falsches Window
    // den Focus hat (#29682#):
    //GetViewFrame()->GetWindow().GrabFocus();

    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetActive(TRUE);
    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->SetActive(TRUE);

    if (bIsMDIActivate)
    {
        // Damit der Navigator auch einen aktuellen Status bekommt
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, TRUE );
        if (GetDispatcher() != NULL)
            GetDispatcher()->Execute(
                SID_NAVIGATOR_INIT,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                &aItem,
                0L);

        SfxViewShell* pViewShell = GetViewShell();
        OSL_ASSERT (pViewShell!=NULL);
        SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_EFFECT_STATE, TRUE, FALSE );
        rBindings.Invalidate( SID_3D_STATE, TRUE, FALSE );

        if (pFuSlideShow && !pFuSlideShow->IsTerminated() )
        {
            pFuSlideShow->Activate();
        }
        if (pFuActual)
        {
            pFuActual->Activate();
        }

        if(!GetDocSh()->IsUIActive())
            UpdatePreview( GetActualPage(), TRUE );

        ::sd::View* pView = GetView();

        if (pView)
        {
            pView->ShowMarkHdl(NULL);
        }
    }

    ReadFrameViewData( pFrameView );

    if (IsMainViewShell())
        GetDocSh()->Connect(this);
}




void ViewShell::UIActivate( SvInPlaceObject *pIPObj )
{
    OSL_ASSERT (GetViewShell()!=NULL);
}




void ViewShell::UIDeactivate( SvInPlaceObject *pIPObj )
{
    OSL_ASSERT (GetViewShell()!=NULL);
}

/*************************************************************************
|*
|* Deaktivierung
|*
\************************************************************************/

void ViewShell::Deactivate(BOOL bIsMDIActivate)
{
    // remove view from a still active drag'n'drop session
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

    if (IsMainViewShell())
        GetDocSh()->Disconnect(this);

    if( pDragTransferable )
        pDragTransferable->SetView( NULL );

    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);

    // View-Attribute an der FrameView merken
    WriteFrameViewData();

    if (bIsMDIActivate)
    {
        if (pFuSlideShow)
        {
            pFuSlideShow->Deactivate();
        }
        if (pFuActual)
        {
            pFuActual->Deactivate();
        }

        // ggfs. Preview vom Kontext loesen
        SfxChildWindow* pPreviewChildWindow =
            pViewShell->GetViewFrame()->GetChildWindow(
                PreviewChildWindow::GetChildWindowId());
        if (pPreviewChildWindow)
        {
            PreviewWindow* pPreviewWin =
                static_cast<PreviewWindow*>(pPreviewChildWindow->GetWindow());
            if (pPreviewWin && pPreviewWin->GetDoc() == GetDoc())
            {
                pPreviewWin->SetContext(NULL, 0, NULL);
            }
        }

        ::sd::View* pView = GetView();

        if (pView)
        {
            pView->HideMarkHdl(NULL);
        }
    }

    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetActive(FALSE);
    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->SetActive(FALSE);

    SfxShell::Deactivate(bIsMDIActivate);
}




void ViewShell::Shutdown (void)
{
    Exit ();
}




/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

BOOL ViewShell::KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    BOOL bReturn(FALSE);

    if(pWin)
    {
        SetActiveWindow(pWin);
    }

    if(!bReturn)
    {
        // #76008#
        // give key input first to SfxViewShell to give CTRL+Key
        // (e.g. CTRL+SHIFT+'+', to front) priority.
        OSL_ASSERT (GetViewShell()!=NULL);
        bReturn = GetViewShell()->KeyInput(rKEvt);
    }

    if(!bReturn)
    {
        if(pFuSlideShow)
        {
            bReturn = pFuSlideShow->KeyInput(rKEvt);
        }
        else if(pFuActual)
        {
            bReturn = pFuActual->KeyInput(rKEvt);
        }
    }

    if(!bReturn && GetActiveWindow())
    {
        KeyCode aKeyCode = rKEvt.GetKeyCode();

        if (aKeyCode.IsMod1() && aKeyCode.IsShift()
            && aKeyCode.GetCode() == KEY_R)
        {
            // 3D-Kontext wieder zerstoeren
            Base3D* pBase3D = (Base3D*) GetActiveWindow()->Get3DContext();

            if (pBase3D)
            {
                pBase3D->Destroy(GetActiveWindow());
            }

            InvalidateWindows();
            bReturn = TRUE;
        }
    }

    return(bReturn);
}

/*************************************************************************
|*
|* MouseButtonDown event
|*
\************************************************************************/

void ViewShell::MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( pWin && !pWin->HasFocus() )
    {
        pWin->GrabFocus();
        SetActiveWindow(pWin);
//        GetViewFrame()->GetWindow().GrabFocus();
    }

    // MouseEvent in E3dView eintragen
    if (GetView() != NULL)
        GetView()->SetMouseEvent(rMEvt);

    if (pFuSlideShow)
    {
        pFuSlideShow->MouseButtonDown(rMEvt);
    }
    else if (pFuActual)
    {
        pFuActual->MouseButtonDown(rMEvt);
    }
}

/*************************************************************************
|*
|* MouseMove event
|*
\************************************************************************/

void ViewShell::MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( pWin )
    {
        SetActiveWindow(pWin);
    }

    // MouseEvent in E3dView eintragen
    if (GetView() != NULL)
        GetView()->SetMouseEvent(rMEvt);

    if (pFuSlideShow)
    {
        pFuSlideShow->MouseMove(rMEvt);
    }
    else if (pFuActual)
    {
        pFuActual->MouseMove(rMEvt);
    }
}

/*************************************************************************
|*
|* MouseButtonUp event
|*
\************************************************************************/

void ViewShell::MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( pWin )
    {
        SetActiveWindow(pWin);
    }

    // MouseEvent in E3dView eintragen
    if (GetView() != NULL)
        GetView()->SetMouseEvent(rMEvt);

    if (pFuSlideShow)
    {
        pFuSlideShow->MouseButtonUp(rMEvt);
    }
    else if (pFuActual)
    {
        pFuActual->MouseButtonUp(rMEvt);
    }
}


/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void ViewShell::Command(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    BOOL bDone = FALSE;

    switch( rCEvt.GetCommand() )
    {
        case( COMMAND_WHEEL ):
        case( COMMAND_STARTAUTOSCROLL ):
        case( COMMAND_AUTOSCROLL ):
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();

            if( pData && pData->IsMod1() )
            {
                if( !GetDocSh()->IsUIActive() )
                {
                    const long  nOldZoom = GetActiveWindow()->GetZoom();
                    long        nNewZoom;

                    if( pData->GetDelta() < 0L )
                        nNewZoom = Max( (long) pWin->GetMinZoom(), (long)(nOldZoom - DELTA_ZOOM) );
                    else
                        nNewZoom = Min( (long) pWin->GetMaxZoom(), (long)(nOldZoom + DELTA_ZOOM) );

                    SetZoom( nNewZoom );
                    Invalidate( SID_ATTR_ZOOM );
                    bDone = TRUE;
                }
            }
            else
            {
                ScrollBar*  pWinHScroll = NULL;
                ScrollBar*  pWinVScroll = NULL;

                if (mpContentWindow.get() == pWin)
                {
                    bDone = pWin->HandleScrollCommand( rCEvt,
                        mpHorizontalScrollBar.get(),
                        mpVerticalScrollBar.get());
                }
            }
        }
        break;

        default:
        break;
    }

    if( !bDone )
    {
        if (pFuSlideShow)
        {
            pFuSlideShow->Command(rCEvt);
        }
        else if (pFuActual)
        {
            pFuActual->Command(rCEvt);
        }
    }
}




void ViewShell::SetupRulers (void)
{
    if (mbHasRulers
        && mpContentWindow.get() != NULL
        && (pFuSlideShow==NULL
            || pFuSlideShow->GetAnimationMode()==ANIMATIONMODE_PREVIEW))
    {
        long nHRulerOfs = 0;

        if ( !mpVerticalRuler.get() != NULL )
        {
            mpVerticalRuler.reset(CreateVRuler(GetActiveWindow()));
            if ( mpVerticalRuler.get() != NULL )
            {
                nHRulerOfs = mpVerticalRuler->GetSizePixel().Width();
                mpVerticalRuler->SetActive(TRUE);
                mpVerticalRuler->Show();
            }
        }
        if ( !mpHorizontalRuler.get() != NULL )
        {
            mpHorizontalRuler.reset(CreateHRuler(GetActiveWindow(), TRUE));
            if ( mpHorizontalRuler.get() != NULL )
            {
                mpHorizontalRuler->SetWinPos(nHRulerOfs);
                mpHorizontalRuler->SetActive(TRUE);
                mpHorizontalRuler->Show();
            }
        }
    }
    /*
    SvBorder aBorder (
        bHasRuler && pVRulerArray[0]!=NULL
        ? pVRulerArray[0]->GetSizePixel().Width()
        : 0,
        bHasRuler && pHRulerArray[0]!=NULL
        ? pHRulerArray[0]->GetSizePixel().Height()
        : 0,
        aScrBarWH.Width(),
        aScrBarWH.Height());
    GetViewShellBase().SetBorderPixel (aBorder);
    */
}




BOOL ViewShell::HasRuler (void)
{
    return mbHasRulers;
}




/*************************************************************************
|*
|* Ersatz fuer AdjustPosSizePixel ab Sfx 248a
|*
\************************************************************************/

#if 0
void ViewShell::InnerResizePixel(const Point &rPos, const Size &rSize)
{
    Point rP = rPos;
    Size rS = rSize;
    rS.Width() += aScrBarWH.Width();
    rS.Height() += aScrBarWH.Height();

    SetupRulers ();

    if ( mpVerticalRuler.get() != NULL )
        rS.Width() += mpVerticalRuler->GetSizePixel().Width();
    if ( mpHorizontalRuler.get() != NULL )
        rS.Height() += mpHorizontalRuler->GetSizePixel().Height();
    AdjustPosSizePixel(rP, rS);
}
#endif

/*************************************************************************
|*
|* Ersatz fuer AdjustPosSizePixel ab Sfx 248a
|*
\************************************************************************/

void ViewShell::Resize (const Point& rPos, const Size& rSize)
{
    SetupRulers ();

    //  AdjustPosSizePixel(rPos, rSize);
    // Make sure that the new size is not degenerate.
    if ( !rSize.Width() || !rSize.Height() )
        return;

    // Remember the new position and size.
    aViewPos  = rPos;
    aViewSize = rSize;

    // Rearrange the UI elements to take care of the new position and size.
    ArrangeGUIElements ();
    // end of included AdjustPosSizePixel.

    Size aS (GetParentWindow()->GetOutputSizePixel());
    Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
    Rectangle aVisArea = GetParentWindow()->PixelToLogic(
        Rectangle( Point(0,0), aVisSizePixel));
    Rectangle aCurrentVisArea (GetDocSh()->GetVisArea(ASPECT_CONTENT));
    Rectangle aWindowRect = GetActiveWindow()->LogicToPixel(aCurrentVisArea);
    if (GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED
        && IsMainViewShell())
    {
        //        GetDocSh()->SetVisArea(aVisArea);
    }

    //  VisAreaChanged(aVisArea);

    ::sd::View* pView = GetView();

    if (pView)
    {
        pView->VisAreaChanged(GetActiveWindow());
    }
}

#if 0
void ViewShell::OuterResizePixel(const Point &rPos, const Size &rSize)
{
    long nHRulerOfs = 0;

    if( !pFuSlideShow || ( ANIMATIONMODE_PREVIEW == pFuSlideShow->GetAnimationMode() ) )
    {
        if ( !mpVerticalRuler.get() != NULL )
        {
            mpVerticalRuler.reset(CreateVRuler(GetActiveWindow()));
            if ( mpVerticalRuler.get() != NULL )
            {
                nHRulerOfs = mpVerticalRuler->GetSizePixel().Width();
                mpVerticalRuler->SetActive(TRUE);
                mpVerticalRuler->Show();
            }
        }

        if ( !mpHorizontalRuler.get() != NULL )
        {
            mpHorizontalRuler.reset(CreateHRuler(GetActiveWindow(), TRUE));
            if ( mpHorizontalRuler.get() != NULL )
            {
                mpHorizontalRuler->SetWinPos(nHRulerOfs);
                mpHorizontalRuler->SetActive(TRUE);
                mpHorizontalRuler->Show();
            }
        }
    }

    AdjustPosSizePixel(rPos, rSize);

    Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
    Rectangle aVisArea = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );

    if ( GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        GetDocSh()->SetVisArea(aVisArea);
    }

    VisAreaChanged(aVisArea);

    ::sd::View* pView = GetView();

    if (pView)
    {
        pView->VisAreaChanged(GetActiveWindow());
    }
}
#endif

#if 0
/** After a simple consistency check the given values are stored so that
    they can be accessed by the <member>ArrangeGUIElements</member> method
    which is finally called and performs the actual adjustment of sizes and
    positions of the GUI elements.
*/
void ViewShell::AdjustPosSizePixel(const Point &rNewPos, const Size &rNewSize)
{
    // Make sure that the new size is not degenerate.
    if ( !rNewSize.Width() || !rNewSize.Height() )
        return;

    // Remember the new position and size.
    aViewPos  = rNewPos;
    aViewSize = rNewSize;

    // Rearrange the UI elements to take care of the new position and size.
    ArrangeGUIElements ();
}
#endif


SvBorder ViewShell::GetBorder (bool bOuterResize)
{
    SvBorder aBorder;

    // Horizontal scrollbar.
    if (mpHorizontalScrollBar.get()!=NULL
        && mpHorizontalScrollBar->IsVisible())
    {
        aBorder.Bottom() = aScrBarWH.Height();
    }

    // Vertical scrollbar.
    if (mpVerticalScrollBar.get()!=NULL
        && mpVerticalScrollBar->IsVisible())
    {
        aBorder.Right() = aScrBarWH.Width();
    }

    // Place horizontal ruler below tab bar.
    if (mbHasRulers && mpContentWindow.get() != NULL)
    {
        SetupRulers();
        if (mpHorizontalRuler.get() != NULL)
            aBorder.Top() = mpHorizontalRuler->GetSizePixel().Height();
        if (mpVerticalRuler.get() != NULL)
            aBorder.Left() = mpVerticalRuler->GetSizePixel().Width();
    }

    return aBorder;
}




void ViewShell::ArrangeGUIElements (void)
{
    bool bVisible = mpContentWindow->IsVisible();
    static bool bFunctionIsRunning = false;
    if (bFunctionIsRunning)
        return;
    bFunctionIsRunning = true;

    // Calculate border for in-place editing.
    long nLeft = aViewPos.X();
    long nTop  = aViewPos.Y();
    long nRight = aViewPos.X() + aViewSize.Width();
    long nBottom = aViewPos.Y() + aViewSize.Height();

    // Horizontal scrollbar.
    if (mpHorizontalScrollBar.get()!=NULL
        && mpHorizontalScrollBar->IsVisible())
    {
        int nLocalLeft = nLeft;
        if (mpLayerTabBar.get()!=NULL && mpLayerTabBar->IsVisible())
            nLocalLeft += mpLayerTabBar->GetSizePixel().Width();
        nBottom -= aScrBarWH.Height();
        mpHorizontalScrollBar->SetPosSizePixel (
            Point(nLocalLeft,nBottom),
            Size(nRight-nLocalLeft-aScrBarWH.Width(),aScrBarWH.Height()));
    }

    // Vertical scrollbar.
    if (mpVerticalScrollBar.get()!=NULL
        && mpVerticalScrollBar->IsVisible())
    {
        nRight -= aScrBarWH.Width();
        mpVerticalScrollBar->SetPosSizePixel (
            Point(nRight,nTop),
            Size (aScrBarWH.Width(),nBottom-nTop));
    }

    // Filler in the lower right corner.
    if (mpScrollBarBox.get() != NULL)
        if (mpHorizontalScrollBar.get()!=NULL
            && mpHorizontalScrollBar->IsVisible()
            && mpVerticalScrollBar.get()!=NULL
            && mpVerticalScrollBar->IsVisible())
        {
            mpScrollBarBox->Show();
            mpScrollBarBox->SetPosSizePixel(Point(nRight, nBottom), aScrBarWH);
        }
        else
            mpScrollBarBox->Hide();

    // Place horizontal ruler below tab bar.
    if (mbHasRulers && mpContentWindow.get() != NULL)
    {
        if (mpHorizontalRuler.get() != NULL)
        {
            Size aRulerSize = mpHorizontalRuler->GetSizePixel();
            aRulerSize.Width() = nRight - nLeft;
            mpHorizontalRuler->SetPosSizePixel (
                Point(nLeft,nTop), aRulerSize);
            if (mpVerticalRuler.get() != NULL)
                mpHorizontalRuler->SetBorderPos(
                    mpVerticalRuler->GetSizePixel().Width()-1);
            nTop += aRulerSize.Height();
        }
        if (mpVerticalRuler.get() != NULL)
        {
            Size aRulerSize = mpVerticalRuler->GetSizePixel();
            aRulerSize.Height() = nBottom  - nTop;
            mpVerticalRuler->SetPosSizePixel (
                Point (nLeft,nTop), aRulerSize);
            nLeft += aRulerSize.Width();
        }
    }

    // The size of the window of the center pane is set differently from
    // that of the windows in the docking windows.
    bool bSlideShowActive =
        pFuSlideShow != NULL
        && ! pFuSlideShow->IsTerminated()
        && ! pFuSlideShow->IsFullScreen()
        && pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_SHOW;
    if ( ! bSlideShowActive)
    {
        // der Sfx darf immer nur das erste Fenster setzen
        //        SetActiveWindow(mpContentWindow.get());
        // Sfx loest ein Resize fuer die Gesamtgroesse aus; bei aktiven
        // Splittern darf dann nicht der minimale Zoom neu berechnet
        // werden. Falls kein Splitter aktiv ist, wird die Berechnung am
        // Ende der Methode nachgeholt
        SfxViewShell* pViewShell = GetViewShell();
        OSL_ASSERT (pViewShell!=NULL);

        if (IsMainViewShell())
        {
            // For the center pane the border is passed to the
            // ViewShellBase so that it can place it inside or outside a
            // fixed rectangle and calculate the size of the content window
            // accordingly.
            //            SetActiveWindow (mpContentWindow.get());
        }
        else
        {
            // For panes other than the center pane we set the size of the
            // content window directly by subtracting the border from the
            // box of the parent window.
            Rectangle aBox (
                Point(0,0),
                GetParentWindow()->GetSizePixel());

            mpContentWindow->SetPosSizePixel(
                aBox.TopLeft(),
                aBox.GetSize());
        }
    }

    // Windows in the center and rulers at the left and top side.
    maAllWindowRectangle = Rectangle(
        aViewPos,
        Size(aViewSize.Width()-aScrBarWH.Width(),
            aViewSize.Height()-aScrBarWH.Height()));

    if (mpContentWindow.get() != NULL)
    {
        mpContentWindow->UpdateMapOrigin();
    }

    UpdateScrollBars();
    bFunctionIsRunning = false;
}




void ViewShell::SetUIUnit(FieldUnit eUnit)
{
    // Set unit at horizontal and vertical rulers.
    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetUnit(eUnit);


    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->SetUnit(eUnit);
}

/*************************************************************************
|*
|* DefTab an den horizontalen Linealen setzen
|*
\************************************************************************/
void ViewShell::SetDefTabHRuler( UINT16 nDefTab )
{
    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetDefTabDist( nDefTab );
}




/** Tell the FmFormShell that the view shell is closing.  Give it the
    oportunity to prevent that.
*/
USHORT ViewShell::PrepareClose (BOOL bUI, BOOL bForBrowsing)
{
    USHORT nResult = TRUE;

    FmFormShell* pFormShell = static_cast<FmFormShell*>(
        GetObjectBarManager().GetObjectBar(RID_FORMLAYER_TOOLBOX));
    if (pFormShell != NULL)
        nResult = pFormShell->PrepareClose (bUI, bForBrowsing);

    return nResult;
}




/*************************************************************************
|*
|* Update preview context
|*
\************************************************************************/
void ViewShell::UpdatePreview (SdPage* pPage, BOOL bInit)
{
    SfxChildWindow* pPreviewChildWindow = GetViewFrame()->GetChildWindow(
        PreviewChildWindow::GetChildWindowId() );
    if (pPreviewChildWindow!=NULL && pPage!=NULL)
    {
        PreviewWindow* pPreviewWin = static_cast<PreviewWindow*>(
            pPreviewChildWindow->GetWindow());
        if ( pPreviewWin && ( bInit || pPreviewWin->GetDoc() == GetDoc() ) )
        {
            USHORT nSdPageNo = ( pPage->GetPageNum() - 1 ) / 2;
            pPreviewWin->SetContext(GetDoc(), nSdPageNo, pFrameView);
        }
    }
}




SfxUndoManager* ViewShell::ImpGetUndoManager (void) const
{
    // Return the undo manager of the currently active object bar.
    SfxShell* pObjectBar = GetObjectBarManager().GetTopObjectBar();
    // The object bar may be temporarily missing, i.e. when one is
    // replaced by another.  Return a NULL pointer in this case.
    if (pObjectBar != NULL)
        return pObjectBar->GetUndoManager();
    else
        return NULL;
}




void ViewShell::ImpGetUndoStrings(SfxItemSet &rSet) const
{
    SfxUndoManager* pUndoManager = ImpGetUndoManager();
    if(pUndoManager)
    {
        sal_uInt16 nCount(pUndoManager->GetUndoActionCount());
        if(nCount)
        {
            // prepare list
            List aStringList;
            sal_uInt16 a;

            for( a = 0; a < nCount; a++)
            {
                // generate one String in list per undo step
                String* pInsertString = new String(pUndoManager->GetUndoActionComment(a));
                aStringList.Insert(pInsertString, LIST_APPEND);
            }

            // set item
            rSet.Put(SfxStringListItem(SID_GETUNDOSTRINGS, &aStringList));

            // delete Strings again
            for(a = 0; a < nCount; a++)
                delete (String*)aStringList.GetObject(a);
        }
        else
        {
            rSet.DisableItem(SID_GETUNDOSTRINGS);
        }
    }
}

// -----------------------------------------------------------------------------

void ViewShell::ImpGetRedoStrings(SfxItemSet &rSet) const
{
    SfxUndoManager* pUndoManager = ImpGetUndoManager();
    if(pUndoManager)
    {
        sal_uInt16 nCount(pUndoManager->GetRedoActionCount());
        if(nCount)
        {
            // prepare list
            List aStringList;
            sal_uInt16 a;

            for( a = 0; a < nCount; a++)
            {
                // generate one String in list per undo step
                String* pInsertString = new String(pUndoManager->GetRedoActionComment(a));
                aStringList.Insert(pInsertString, LIST_APPEND);
            }

            // set item
            rSet.Put(SfxStringListItem(SID_GETREDOSTRINGS, &aStringList));

            // delete Strings again
            for(a = 0; a < nCount; a++)
                delete (String*)aStringList.GetObject(a);
        }
        else
        {
            rSet.DisableItem(SID_GETREDOSTRINGS);
        }
    }
}

// -----------------------------------------------------------------------------

void ViewShell::ImpSidUndo(BOOL bDrawViewShell, SfxRequest& rReq)
{
    SfxUndoManager* pUndoManager = ImpGetUndoManager();
    sal_uInt16 nNumber(1);
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    if(pReqArgs)
    {
        SfxUInt16Item* pUIntItem = (SfxUInt16Item*)&pReqArgs->Get(SID_UNDO);
        nNumber = pUIntItem->GetValue();
    }

    if(nNumber && pUndoManager)
    {
        if(bDrawViewShell)
        {
            List* pList = GetDoc()->GetDeletedPresObjList();
            if( pList )
                pList->Clear();
        }

        sal_uInt16 nCount(pUndoManager->GetUndoActionCount());
        if(nCount >= nNumber)
        {
            // #94637# when UndoStack is cleared by ModifyPageUndoAction
            // the nCount may have changed, so test GetUndoActionCount()
            while(nNumber-- && pUndoManager->GetUndoActionCount())
            {
                pUndoManager->Undo();
            }
        }

        // #91081# refresh rulers, maybe UNDO was move of TAB marker in ruler
        if (mbHasRulers)
        {
            Invalidate(SID_ATTR_TABSTOP);
        }
    }

    // This one is corresponding to the default handling
    // of SID_UNDO in sfx2
    GetViewFrame()->GetBindings().InvalidateAll(sal_False);

    rReq.Done();
}

// -----------------------------------------------------------------------------

void ViewShell::ImpSidRedo(BOOL bDrawViewShell, SfxRequest& rReq)
{
    SfxUndoManager* pUndoManager = ImpGetUndoManager();
    sal_uInt16 nNumber(1);
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    if(pReqArgs)
    {
        SfxUInt16Item* pUIntItem = (SfxUInt16Item*)&pReqArgs->Get(SID_REDO);
        nNumber = pUIntItem->GetValue();
    }

    if(nNumber && pUndoManager)
    {
        sal_uInt16 nCount(pUndoManager->GetRedoActionCount());
        if(nCount >= nNumber)
        {
            // #94637# when UndoStack is cleared by ModifyPageRedoAction
            // the nCount may have changed, so test GetRedoActionCount()
            while(nNumber-- && pUndoManager->GetRedoActionCount())
            {
                pUndoManager->Redo();
            }
        }

        // #91081# refresh rulers, maybe REDO was move of TAB marker in ruler
        if (mbHasRulers)
        {
            Invalidate(SID_ATTR_TABSTOP);
        }
    }

    // This one is corresponding to the default handling
    // of SID_UNDO in sfx2
    GetViewFrame()->GetBindings().InvalidateAll(sal_False);

    rReq.Done();
}

// -----------------------------------------------------------------------------

void ViewShell::ExecReq( SfxRequest& rReq )
{
    USHORT nSlot = rReq.GetSlot();
    switch( nSlot )
    {
        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            if( pFuActual )
            {
                pFuActual->ScrollStart();
                ScrollLines( 0, -1 );
                pFuActual->ScrollEnd();
            }

            rReq.Done();
        }
        break;

        case SID_OUTPUT_QUALITY_COLOR:
        case SID_OUTPUT_QUALITY_GRAYSCALE:
        case SID_OUTPUT_QUALITY_BLACKWHITE:
        case SID_OUTPUT_QUALITY_CONTRAST:
        {
            ULONG nMode = OUTPUT_DRAWMODE_COLOR;

            switch( nSlot )
            {
                case SID_OUTPUT_QUALITY_COLOR: nMode = OUTPUT_DRAWMODE_COLOR; break;
                case SID_OUTPUT_QUALITY_GRAYSCALE: nMode = OUTPUT_DRAWMODE_GRAYSCALE; break;
                case SID_OUTPUT_QUALITY_BLACKWHITE: nMode = OUTPUT_DRAWMODE_BLACKWHITE; break;
                case SID_OUTPUT_QUALITY_CONTRAST: nMode = OUTPUT_DRAWMODE_CONTRAST; break;
            }

            GetActiveWindow()->SetDrawMode( nMode );
            pFrameView->SetDrawMode( nMode );
// #110094#-7
//            GetView()->ReleaseMasterPagePaintCache();
            GetActiveWindow()->Invalidate();

            Invalidate();
            rReq.Done();
            break;
        }

        case SID_PREVIEW_QUALITY_COLOR:
        case SID_PREVIEW_QUALITY_GRAYSCALE:
        case SID_PREVIEW_QUALITY_BLACKWHITE:
        case SID_PREVIEW_QUALITY_CONTRAST:
        {
            ULONG nMode = PREVIEW_DRAWMODE_COLOR;

            switch( nSlot )
            {
                case SID_PREVIEW_QUALITY_COLOR:
                    nMode = PREVIEW_DRAWMODE_COLOR;
                    break;
                case SID_PREVIEW_QUALITY_GRAYSCALE:
                    nMode = PREVIEW_DRAWMODE_GRAYSCALE;
                    break;
                case SID_PREVIEW_QUALITY_BLACKWHITE:
                    nMode = PREVIEW_DRAWMODE_BLACKWHITE;
                    break;
                case SID_PREVIEW_QUALITY_CONTRAST:
                    nMode = PREVIEW_DRAWMODE_CONTRAST;
                    break;
            }

            SfxChildWindow* pPreviewChildWindow =
                GetViewFrame()->GetChildWindow(
                    PreviewChildWindow::GetChildWindowId());
            PreviewWindow* pPreviewWindow = static_cast<PreviewWindow*>(
                pPreviewChildWindow ? pPreviewChildWindow->GetWindow() : NULL);
            FuSlideShow* pShow = pPreviewWindow!=NULL
                ? pPreviewWindow->GetSlideShow()
                : NULL;
            ShowWindow* pShowWindow = const_cast<ShowWindow*>(
                pShow!=NULL ? pShow->GetShowWindow() : NULL);

            if( pShowWindow )
            {
                pShowWindow->SetDrawMode( nMode );
                pShow->Resize( pShowWindow->GetOutputSizePixel() );
                pShowWindow->Invalidate();
            }

            pFrameView->SetPreviewDrawMode( nMode );
            SdOptions* pOptions = SD_MOD()->GetSdOptions (
                GetDoc()->GetDocumentType() );
            pOptions->SetPreviewQuality( nMode );

            Invalidate();
            rReq.Done();
        }
        break;
    }
}




/** This default implemenation returns only an empty reference.  See derived
    classes for more interesting examples.
*/
::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible>
ViewShell::CreateAccessibleDocumentView (::sd::Window* pWindow)
{
    return ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> ();
}



::sd::WindowUpdater* ViewShell::GetWindowUpdater (void) const
{
    return mpWindowUpdater.get();
}




/** The implementation has to change in order to take care of at least
    one current/main/default window per ViewShell object.  Of these
    there will usually be more than one for every ViewShellBase object.
*/
/*::Window* ViewShell::GetWindow (void) const
{
    OSL_ASSERT(GetViewShell()!=NULL);
    //    return GetViewShell()->GetWindow();
    return const_cast<ViewShell*>(this)->GetActiveWindow();
}

*/


ViewShellBase& ViewShell::GetViewShellBase (void) const
{
    return *static_cast<ViewShellBase*>(GetViewShell());
}




DrawController* ViewShell::GetController (void)
{
    return mpController.get();
}




ViewShell::ShellType ViewShell::GetShellType (void) const
{
    return meShellType;
}




IMPL_LINK(ViewShell, FrameWindowEventListener,  VclSimpleEvent*, pEvent )
{
    if (pEvent!=NULL && pEvent->ISA(VclWindowEvent))
    {
        VclWindowEvent* pWindowEvent = static_cast<VclWindowEvent*>(pEvent);
        switch (pWindowEvent->GetId())
        {
            //            case VCLEVENT_WINDOW_SHOW:
            //            case VCLEVENT_WINDOW_ACTIVATE:
            case VCLEVENT_WINDOW_RESIZE:
            {
                if ( ! GetDocSh()->GetProtocol().IsInPlaceActive())
                {
                    // Forward the event only when in in-place mode
                    // which is handled differently (InnerResize from
                    // the ViewShellBase class is forwarded).
                    ::Window* pWindow = pWindowEvent->GetWindow();
                    if (pWindow != NULL)
                        Resize (Point(),
                            pWindow->GetOutputSizePixel());
                }
            }
            break;
        }
    }
    return 0;
}


DrawDocShell* ViewShell::GetDocSh (void) const
{
    return GetViewShellBase().GetDocShell();
}




SdDrawDocument* ViewShell::GetDoc (void) const
{
    return GetViewShellBase().GetDocument();
}




ErrCode ViewShell::DoVerb (long nVerb)
{
    return ERRCODE_NONE;
}




void ViewShell::SetCurrentFunction (FuPoor* pFunction)
{
    pFuActual = pFunction;
}




void ViewShell::SetOldFunction (FuPoor* pFunction)
{
    pFuOld = pFunction;
}




void ViewShell::SetSlideShowFunction (FuSlideShow* pFunction)
{
    pFuSlideShow = pFunction;
}




bool ViewShell::IsMainViewShell (void) const
{
    return GetViewShellBase().GetMainViewShell() == this;
}




void ViewShell::SetIsMainViewShell (bool bIsMainViewShell)
{
    if (bIsMainViewShell != mpImpl->mbIsMainViewShell)
    {
        mpImpl->mbIsMainViewShell = bIsMainViewShell;
        if (bIsMainViewShell)
        {
            GetDocSh()->Connect (this);
        }
        else
        {
            GetDocSh()->Disconnect (this);
        }
    }
}




ObjectBarManager& ViewShell::GetObjectBarManager (void) const
{
    return *mpObjectBarManager.get();
}




void ViewShell::GetLowerShellList (::std::vector<SfxShell*>& rShellList) const
{
    mpObjectBarManager->GetLowerShellList (rShellList);
}




void ViewShell::GetUpperShellList (::std::vector<SfxShell*>& rShellList) const
{
    mpObjectBarManager->GetUpperShellList (rShellList);
}





::sd::Window* ViewShell::GetActiveWindow (void) const
{
    return mpActiveWindow;
}




void ViewShell::Paint (const Rectangle& rRect, ::sd::Window* pWin)
{
}




void ViewShell::Draw(OutputDevice &rDev, const Region &rReg)
{
}




ZoomList* ViewShell::GetZoomList (void)
{
    return pZoomList;
}




void ViewShell::ShowUIControls (bool bVisible)
{
    mpImpl->mbIsShowingUIControls = bVisible;

    if (mbHasRulers)
    {
        if (mpHorizontalRuler.get() != NULL)
            mpHorizontalRuler->Show( bVisible );

        if (mpVerticalRuler.get() != NULL)
            mpVerticalRuler->Show( bVisible );
    }

    if (mpVerticalScrollBar.get() != NULL)
        mpVerticalScrollBar->Show( bVisible );

    if (mpHorizontalScrollBar.get() != NULL)
        mpHorizontalScrollBar->Show( bVisible );

    if (mpContentWindow.get() != NULL)
        mpContentWindow->Show( bVisible );
}




} // end of namespace sd





//===== ViewShellObjectBarFactory =============================================

namespace {

ViewShellObjectBarFactory::ViewShellObjectBarFactory (
    ::sd::ViewShell& rViewShell)
    : mrViewShell (rViewShell)
{
}




ViewShellObjectBarFactory::~ViewShellObjectBarFactory (void)
{
    for (ShellCache::iterator aI(maShellCache.begin());
         aI!=maShellCache.end();
         aI++)
    {
        delete aI->second;
    }
}




SfxShell* ViewShellObjectBarFactory::CreateShell (
    ::sd::ShellId nId,
    ::Window* pParentWindow,
    ::sd::FrameView* pFrameView)
{
    SfxShell* pShell = NULL;

    ShellCache::iterator aI (maShellCache.find(nId));
    if (aI == maShellCache.end())
    {
        ::sd::View* pView = mrViewShell.GetView();
        switch (nId)
        {
            case RID_DRAW_OBJ_TOOLBOX:
                if (mrViewShell.GetShellType() == ::sd::ViewShell::ST_DRAW)
                    pShell = new ::sd::DrawObjectBar(&mrViewShell, pView);
                else
                    pShell = new ::sd::ImpressObjectBar(&mrViewShell, pView);
                break;

            case RID_BEZIER_TOOLBOX:
                pShell = new ::sd::BezierObjectBar(&mrViewShell, pView);
                break;

            case RID_GLUEPOINTS_TOOLBOX:
                pShell = new ::sd::GluePointsObjectBar(&mrViewShell, pView);
                break;

            case RID_DRAW_TEXT_TOOLBOX:
                pShell = new ::sd::TextObjectBar(
                    &mrViewShell, mrViewShell.GetDoc()->GetPool(), pView);
                break;

            case RID_FORMLAYER_TOOLBOX:
                pShell = new FmFormShell(
                    &mrViewShell.GetViewShellBase(), pView);
                break;

            case RID_DRAW_GRAF_TOOLBOX:
                pShell = new ::sd::GraphicObjectBar(&mrViewShell, pView);
                break;

            case RID_DRAW_MEDIA_TOOLBOX:
                pShell = new ::sd::MediaObjectBar(&mrViewShell, pView);
                break;

            case RID_SVX_EXTRUSION_BAR:
                pShell = new ::svx::ExtrusionBar(
                    &mrViewShell.GetViewShellBase());
                break;

            default:
                pShell = NULL;
                break;
        }
        maShellCache[nId] = pShell;
    }
    else
        pShell = aI->second;

    return pShell;
}




void ViewShellObjectBarFactory::ReleaseShell (SfxShell* pShell)
{
    // Do nothing because the shells are stored in a cache.
}

} // end of anonymous namespace
