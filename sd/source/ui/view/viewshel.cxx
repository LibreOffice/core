/*************************************************************************
 *
 *  $RCSfile: viewshel.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 16:27:33 $
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

#ifndef SD_VIEW_SHELL_BASE
#include "ViewShellBase.hxx"
#endif

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

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

ViewShell::ViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    bool bAllowCenter)
    : SfxShell (&rViewShellBase),
      mpWindowUpdater (new ::sd::WindowUpdater()),
      aHSplit(&pFrame->GetWindow(), WB_HSCROLL),
      aVSplit(&pFrame->GetWindow(), WB_VSCROLL),
    aDrawBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aSlideBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aOutlineBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aNotesBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aHandoutBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aPresentationBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    pFuActual(NULL),
    pFuOld(NULL),
    pFuSearch(NULL),
    pFuSlideShow(NULL),
    pZoomList(NULL),
    pFrameView(NULL),
    bCenterAllowed(bAllowCenter),
    bStartShowWithDialog( FALSE ),
    pScrlBox(NULL),
    pView(NULL),
    nPrintedHandoutPageNum(1),
//  mbPrintDirectSelected( FALSE ),
    mxController(),
    mpController(NULL),
    meShellType (ST_NONE)
{
    pScrlBox = new ScrollBarBox(&pFrame->GetWindow(), WB_3DLOOK | WB_SIZEABLE );
    Construct();
}

/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

ViewShell::ViewShell(SfxViewFrame* pFrame, const ViewShell& rShell)
    : SfxShell (rShell.GetViewShell()),
    aHSplit(&pFrame->GetWindow(), WB_HSCROLL),
    aVSplit(&pFrame->GetWindow(), WB_VSCROLL),
    aDrawBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aSlideBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aOutlineBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aNotesBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aHandoutBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aPresentationBtn(&pFrame->GetWindow(),
        WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    pFuActual(NULL),
    pFuOld(NULL),
    pFuSearch(NULL),
    pFuSlideShow(NULL),
    pZoomList(NULL),
//  nCurrentObjectBar(0),
//  bObjectBarSwitchEnabled(TRUE),
    pFrameView(NULL),
    bCenterAllowed(rShell.bCenterAllowed),
    pScrlBox(NULL),
    pView(NULL),
    nPrintedHandoutPageNum(1),
//  mbPrintDirectSelected( FALSE ),
    mxController(),
    mpController(NULL),
    meShellType (ST_NONE)
{
    pScrlBox = new ScrollBarBox(&pFrame->GetWindow(), WB_3DLOOK | WB_SIZEABLE );
    Construct();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

ViewShell::~ViewShell()
{
    // Dispose the controller of this sub-shell.  The disposing event sent
    // to the main controller will result in it asking the sub shell manager
    // for the controller of the new main sub-shell.
    Reference<lang::XComponent> xComponent (mxController, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);
    CancelSearching();

    // Stop listening for window events.
    pViewShell->GetViewFrame()->GetWindow().RemoveEventListener (
        LINK(this,ViewShell,FrameWindowEventListener));

    GetDocSh()->Disconnect(this);

    pViewShell->SetWindow(NULL);

    delete pZoomList;

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        // Zeiger immer gueltig oder NULL
        delete pHScrlArray[nX];
        delete pHRulerArray[nX];

        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            if ( pVScrlArray[nY] )
            {
                delete pVScrlArray[nY];
                pVScrlArray[nY] = NULL;
            }
            if ( pVRulerArray[nY] )
            {
                delete pVRulerArray[nY];
                pVRulerArray[nY] = NULL;
            }
            if ( pWinArray[nX][nY] )
                delete pWinArray[nX][nY];
        }
    }

    /*af
    // Shells fuer Object Bars loeschen
    // vorm Zerstoeren der ObjectBarShells den Dispatcher flushen
    GetDispatcher()->Flush();

    SfxShell* pObjBarShell = static_cast<SfxShell*>(aShellTable.First());
    while (pObjBarShell)
    {
        delete pObjBarShell;
        pObjBarShell = static_cast<SfxShell*>(aShellTable.Next());
    }
    */
    delete pScrlBox;
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

    GetDocSh()->Connect (this);

    pZoomList = new ZoomList( this );

    // Remove all currently active object bars.
    GetObjectBarManager().Clear ();
    GetObjectBarManager().EnableObjectBarSwitching();

    pWindow = NULL;
    SetActiveWindow(new ::sd::Window(&pViewShell->GetViewFrame()->GetWindow()));
//    pWindow->GrabFocus();

    // alle Zeiger mit NULL initialisieren
    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        pHScrlArray[nX] = NULL;
        pHRulerArray[nX] = NULL;

        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            pVScrlArray[nY] = NULL;
            pVRulerArray[nY] = NULL;
            pWinArray[nX][nY] = NULL;
        }
    }
    // Splitter zu Anfang aus
    bIsHSplit = bIsVSplit = FALSE;
    // Lineal an
    bHasRuler = TRUE;

    pWindow->SetCenterAllowed(bCenterAllowed);

    pWinArray[0][0] = pWindow;
    pWinArray[0][0]->SetViewShell(this);

    pHScrlArray[0] = new ScrollBar(&pViewShell->GetViewFrame()->GetWindow(),
                                    WinBits(WB_HSCROLL | WB_DRAG));
    pHScrlArray[0]->EnableRTL (FALSE);
    pHScrlArray[0]->SetRange(Range(0, 32000));
    pHScrlArray[0]->SetScrollHdl(LINK(this, ViewShell, HScrollHdl));
    pHScrlArray[0]->Show();

    pVScrlArray[0] = new ScrollBar(&pViewShell->GetViewFrame()->GetWindow(),
                                    WinBits(WB_VSCROLL | WB_DRAG));
    pVScrlArray[0]->SetRange(Range(0, 32000));
    pVScrlArray[0]->SetScrollHdl(LINK(this, ViewShell, VScrollHdl));
    pVScrlArray[0]->Show();
    aScrBarWH = Size(pVScrlArray[0]->GetSizePixel().Width(),
                     pHScrlArray[0]->GetSizePixel().Height());

    aHSplit.SetSplitHdl(LINK(this, ViewShell, SplitHdl));
    aVSplit.SetSplitHdl(LINK(this, ViewShell, SplitHdl));
    aHSplit.Show();
    aVSplit.Show();

    if (pScrlBox)
        pScrlBox->Show();

    if( !this->ISA(GraphicViewShell ) )
    {
        aDrawBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_DRAW ) ), IMAGE_STDBTN_COLOR ) );
        aDrawBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_DRAW_H ) ), IMAGE_STDBTN_COLOR_HC ), BMP_COLOR_HIGHCONTRAST );
        aDrawBtn.SetClickHdl(LINK(this, ViewShell, ModeBtnHdl));
        aDrawBtn.SetQuickHelpText( String( SdResId( STR_DRAW_MODE ) ) );
        aDrawBtn.SetHelpId( HID_SD_BTN_DRAW );
        aDrawBtn.Show();

        aSlideBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_SLIDE ) ), IMAGE_STDBTN_COLOR ) );
        aSlideBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_SLIDE_H ) ), IMAGE_STDBTN_COLOR_HC ), BMP_COLOR_HIGHCONTRAST );
        aSlideBtn.SetClickHdl(LINK(this, ViewShell, ModeBtnHdl));
        aSlideBtn.SetQuickHelpText( String( SdResId( STR_SLIDE_MODE ) ) );
        aSlideBtn.SetHelpId( HID_SD_BTN_SLIDE );
        aSlideBtn.Show();

        aOutlineBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_OUTLINE ) ), IMAGE_STDBTN_COLOR ) );
        aOutlineBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_OUTLINE_H ) ), IMAGE_STDBTN_COLOR_HC ), BMP_COLOR_HIGHCONTRAST );
        aOutlineBtn.SetClickHdl(LINK(this, ViewShell, ModeBtnHdl));
        aOutlineBtn.SetQuickHelpText( String( SdResId( STR_OUTLINE_MODE ) ) );
        aOutlineBtn.SetHelpId( HID_SD_BTN_OUTLINE );
        aOutlineBtn.Show();

        aNotesBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_NOTES ) ), IMAGE_STDBTN_COLOR ) );
        aNotesBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_NOTES_H ) ), IMAGE_STDBTN_COLOR_HC ), BMP_COLOR_HIGHCONTRAST );
        aNotesBtn.SetClickHdl(LINK(this, ViewShell, ModeBtnHdl));
        aNotesBtn.SetQuickHelpText( String( SdResId( STR_NOTES_MODE ) ) );
        aNotesBtn.SetHelpId( HID_SD_BTN_NOTES );
        aNotesBtn.Show();

        aHandoutBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_HANDOUT ) ), IMAGE_STDBTN_COLOR ) );
        aHandoutBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_HANDOUT_H ) ), IMAGE_STDBTN_COLOR_HC ), BMP_COLOR_HIGHCONTRAST );
        aHandoutBtn.SetClickHdl(LINK(this, ViewShell, ModeBtnHdl));
        aHandoutBtn.SetQuickHelpText( String( SdResId( STR_HANDOUT_MODE ) ) );
        aHandoutBtn.SetHelpId( HID_SD_BTN_HANDOUT );
        aHandoutBtn.Show();

        aPresentationBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_PRESENTATION ) ), IMAGE_STDBTN_COLOR ) );
        aPresentationBtn.SetModeImage( Image( Bitmap( SdResId( BMP_SW_PRESENTATION_H ) ), IMAGE_STDBTN_COLOR_HC ), BMP_COLOR_HIGHCONTRAST );
        aPresentationBtn.SetClickHdl(LINK(this, ViewShell, ModeBtnHdl));
        aPresentationBtn.SetQuickHelpText( String( SdResId( STR_START_PRESENTATION ) ) );
        aPresentationBtn.SetHelpId( HID_SD_BTN_PRESENTATION );
        aPresentationBtn.Show();

        if (GetDocSh()->IsPreview())
        {
            aPresentationBtn.Disable();
        }
        else
        {
            aPresentationBtn.Enable();
        }
    }

    String aName( RTL_CONSTASCII_USTRINGPARAM( "ViewShell" ));
    SetName (aName);

    GetDoc()->StartOnlineSpelling(FALSE);

    mpWindowUpdater->SetViewShell (*this);
    mpWindowUpdater->SetDocument (GetDoc());
}




void ViewShell::Init (void)
{
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);

    // Listen for events from the frame window so that we can layout
    // the controls when the size of the window changes.  Do this only
    // when not in in-place mode which is handled differently.
    if ( ! GetDocSh()->GetProtocol().IsInPlaceActive())
    {
        pViewShell->GetViewFrame()->GetWindow().AddEventListener (
            LINK(this,ViewShell,FrameWindowEventListener));

        // Initiate a re-arragangement of the window controls.
        OuterResizePixel (Point(),
            pViewShell->GetViewFrame()->GetWindow().GetOutputSizePixel());
    }
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
    SfxShell::Activate (bIsMDIActivate);
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);

    // Laut MI darf keiner GrabFocus rufen, der nicht genau weiss von
    // welchem Window der Focus gegrabt wird. Da Activate() vom SFX teilweise
    // asynchron verschickt wird, kann es sein, dass ein falsches Window
    // den Focus hat (#29682#):
    //GetViewFrame()->GetWindow().GrabFocus();

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
        if ( pHRulerArray[nX] )
            pHRulerArray[nX]->SetActive(TRUE);
    for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        if ( pVRulerArray[nY] )
            pVRulerArray[nY]->SetActive(TRUE);

    if (bIsMDIActivate)
    {
        // Damit der Navigator auch einen aktuellen Status bekommt
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, TRUE );
        GetDispatcher()->Execute(
            SID_NAVIGATOR_INIT,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

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

    GetDocSh()->Connect(this);
}

void ViewShell::UIActivate( SvInPlaceObject *pIPObj )
{
    OSL_ASSERT (GetViewShell()!=NULL);

    // #94252# Disable draw view controls when going inactive
    aDrawBtn.Disable();
    aOutlineBtn.Disable();
    aSlideBtn.Disable();
    aNotesBtn.Disable();
    aHandoutBtn.Disable();
    aPresentationBtn.Disable();
}

void ViewShell::UIDeactivate( SvInPlaceObject *pIPObj )
{
    // #94252# Enable draw view controls when going active
    aDrawBtn.Enable();
    aOutlineBtn.Enable();
    aSlideBtn.Enable();
    aNotesBtn.Enable();
    aHandoutBtn.Enable();
    aPresentationBtn.Enable();

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

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
        if ( pHRulerArray[nX] )
            pHRulerArray[nX]->SetActive(FALSE);
    for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        if ( pVRulerArray[nY] )
            pVRulerArray[nY]->SetActive(FALSE);

    SfxShell::Deactivate (bIsMDIActivate);
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

    if(!bReturn && pWindow)
    {
        KeyCode aKeyCode = rKEvt.GetKeyCode();

        if (aKeyCode.IsMod1() && aKeyCode.IsShift()
            && aKeyCode.GetCode() == KEY_R)
        {
            // 3D-Kontext wieder zerstoeren
            Base3D* pBase3D = (Base3D*) pWindow->Get3DContext();

            if (pBase3D)
            {
                pBase3D->Destroy(pWindow);
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
    pView->SetMouseEvent(rMEvt);

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
    pView->SetMouseEvent(rMEvt);

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
    pView->SetMouseEvent(rMEvt);

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
                    const long  nOldZoom = pWindow->GetZoom();
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

                for( USHORT nX = 0; nX < MAX_HSPLIT_CNT && !bDone; nX++ )
                {
                    for( USHORT nY = 0; nY < MAX_VSPLIT_CNT; nY++ )
                    {
                        if( pWinArray[ nX ][ nY ] == pWin )
                        {
                            bDone = pWin->HandleScrollCommand( rCEvt,
                                                               pHScrlArray[ nX ],
                                                               pVScrlArray[ nY ] );
                            break;
                        }
                    }
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


/*************************************************************************
|*
|* Ersatz fuer AdjustPosSizePixel ab Sfx 248a
|*
\************************************************************************/

void ViewShell::InnerResizePixel(const Point &rPos, const Size &rSize)
{
    Point rP = rPos;
    Size rS = rSize;
    rS.Width() += aScrBarWH.Width();
    rS.Height() += aScrBarWH.Height();

    if ( bHasRuler )
    {
        long nHRulerOfs = 0;

        if ( !pVRulerArray[0] )
        {
            pVRulerArray[0] = CreateVRuler(pWindow);
            if ( pVRulerArray[0] )
            {
                nHRulerOfs = pVRulerArray[0]->GetSizePixel().Width();
                pVRulerArray[0]->SetActive(TRUE);
                pVRulerArray[0]->Show();
            }
        }
        if ( !pHRulerArray[0] )
        {
            pHRulerArray[0] = CreateHRuler(pWindow, TRUE);
            if ( pHRulerArray[0] )
            {
                pHRulerArray[0]->SetWinPos(nHRulerOfs);
                pHRulerArray[0]->SetActive(TRUE);
                pHRulerArray[0]->Show();
            }
        }

        if ( pVRulerArray[0] )
            rS.Width() += pVRulerArray[0]->GetSizePixel().Width();
        if ( pHRulerArray[0] )
            rS.Height() += pHRulerArray[0]->GetSizePixel().Height();
    }

    AdjustPosSizePixel(rP, rS);
}

/*************************************************************************
|*
|* Ersatz fuer AdjustPosSizePixel ab Sfx 248a
|*
\************************************************************************/

void ViewShell::OuterResizePixel(const Point &rPos, const Size &rSize)
{
    long nHRulerOfs = 0;

    if( !pFuSlideShow || ( ANIMATIONMODE_PREVIEW == pFuSlideShow->GetAnimationMode() ) )
    {
        if ( !pVRulerArray[0] )
        {
            pVRulerArray[0] = CreateVRuler(pWindow);
            if ( pVRulerArray[0] )
            {
                nHRulerOfs = pVRulerArray[0]->GetSizePixel().Width();
                pVRulerArray[0]->SetActive(TRUE);
                pVRulerArray[0]->Show();
            }
        }

        if ( !pHRulerArray[0] )
        {
            pHRulerArray[0] = CreateHRuler(pWindow, TRUE);
            if ( pHRulerArray[0] )
            {
                pHRulerArray[0]->SetWinPos(nHRulerOfs);
                pHRulerArray[0]->SetActive(TRUE);
                pHRulerArray[0]->Show();
            }
        }
    }

    AdjustPosSizePixel(rPos, rSize);

    Size aVisSizePixel = pWindow->GetOutputSizePixel();
    Rectangle aVisArea = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );

    if ( GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        GetDocSh()->SetVisArea(aVisArea);
    }

    VisAreaChanged(aVisArea);

    ::sd::View* pView = GetView();

    if (pView)
    {
        pView->VisAreaChanged(pWindow);
    }
}


/** After a simple consistency check the given values are stored so that
    they can be accessed by the <member>ArrageGUIElements</member> method
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




void ViewShell::ArrangeGUIElements (void)
{
    static bool bFunctionIsRunning = false;
    if (bFunctionIsRunning)
        return;
    bFunctionIsRunning = true;
    // Some shortcuts to the outer rectangle that will include all
    // controls and windows.
    const long nPosX = aViewPos.X();
    const long nPosY = aViewPos.Y();
    const long nSizeX = aViewSize.Width();
    const long nSizeY = aViewSize.Height();

    // Calculate border for in-place editing.
    long nLeft = nPosX;
    long nTop  = nPosY;
    const long nRight = nPosX + nSizeX - aScrBarWH.Width();
    const long nBottom = nPosY + nSizeY - aScrBarWH.Height();
    if (bHasRuler)
    {
        if (pVRulerArray[0])
            nLeft = pVRulerArray[0]->GetSizePixel().Width();
        if (pHRulerArray[0])
            nTop  = pHRulerArray[0]->GetSizePixel().Height();
    }

    BOOL bSlideShowActive =
        pFuSlideShow != NULL
        && ! pFuSlideShow->IsTerminated()
        && ! pFuSlideShow->IsFullScreen()
        && pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_SHOW;
    if( !bSlideShowActive )
    {
        // der Sfx darf immer nur das erste Fenster setzen
        SetActiveWindow(pWinArray[0][0]);
        // Sfx loest ein Resize fuer die Gesamtgroesse aus; bei aktiven Splittern
        // darf dann nicht der minimale Zoom neu berechnet werden. Falls kein
        // Splitter aktiv ist, wird die Berechnung am Ende der Methode nachgeholt
        SfxViewShell* pViewShell = GetViewShell();
        OSL_ASSERT (pViewShell!=NULL);

        // Not being a true view shell anymore we manage the border ourselves.
        /*        aViewPos = Point(nLeft,nTop);
        aViewSize = Size(
            aViewSize.Width() - nLeft - nRight,
            aViewSize.Height() - nTop - nBottom);
        */

        // Call SetBorderPixel() to set the size of the window of the
        // frame and/or the size of the main document window.  In
        // in-place mode the outer (frame) window is enlarged, in
        // out-place mode the inner (document) window is made smaller.
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

        SetActiveWindow(pWindow);
    }

    // Buttons in the border have the same width as vertical
    // scrollbars and have the same height as horizontal scrollbars.
    const Size aBtnSize(aScrBarWH);


    const long nSplitSize = aScrBarWH.Width() / 4;
    // TabControl oder aehnliches vorhanden?
    long    aHCtrlWidth = GetHCtrlWidth();


    // Horizontal splitter (sits between the horizontal scrollbars.)
    Size aHSplitSize = aHSplit.GetSizePixel();
    Point aHSplitPos = aHSplit.GetPosPixel();
    aHSplitPos.Y() = nBottom;
    aHSplit.SetDragRectPixel(
            Rectangle(Point(nPosX, nPosY), Size(nSizeX, nSizeY)),
            &GetViewFrame()->GetWindow());
    if ( !bIsHSplit || (bIsHSplit &&
        (aHSplitPos.X() < nPosX + MIN_SCROLLBAR_SIZE ||
         aHSplitPos.X() > nPosX + nSizeX - MIN_SCROLLBAR_SIZE)) )
    {
        aHSplitPos.X() = nRight - nSplitSize;
        bIsHSplit = FALSE;
    }
    CreateHSplitElems(aHSplitPos.X() + nSplitSize);
    aHSplitSize.Width()  = nSplitSize;
    aHSplitSize.Height() = aScrBarWH.Height();

    if ( bIsHSplit )
    {
        aHSplitPos.Y() -= nSizeY;
        aHSplitSize.Height() += nSizeY;
    }
    aHSplit.SetPosPixel(aHSplitPos);
    aHSplit.SetSizePixel(aHSplitSize);


    // Horizontal scrollbars.
    Size aHBarSize;
    Point aHPos(nPosX + aHCtrlWidth, nPosY + nSizeY - aScrBarWH.Height());
    aHBarSize.Width() = aHSplitPos.X() - aHPos.X();
    aHBarSize.Height() = aScrBarWH.Height();
    if ( aHBarSize.Width() < MIN_SCROLLBAR_SIZE )
    {
        aHPos.X() = aHSplitPos.X() - MIN_SCROLLBAR_SIZE;
        aHBarSize.Width() = MIN_SCROLLBAR_SIZE;
    }
    pHScrlArray[0]->SetPosSizePixel(aHPos, aHBarSize);

    aHPos.X() += aHBarSize.Width() + nSplitSize;
    if (pHScrlArray[1] != NULL)
    {
        aHBarSize = pHScrlArray[1]->GetSizePixel();
        aHBarSize.Width() = nRight - aHPos.X();
        pHScrlArray[1]->SetPosSizePixel(aHPos, aHBarSize);
    }


    // Buttons for switching the view shells (located above the
    // vertical scrollbars.)
    Point aVPos(nPosX + nSizeX - aBtnSize.Width(), nPosY);
    if ( ! ISA(GraphicViewShell))
    {
        aDrawBtn.SetPosSizePixel(aVPos, aBtnSize);
        aVPos.Y() += aBtnSize.Height();
        aOutlineBtn.SetPosSizePixel(aVPos, aBtnSize);
        aVPos.Y() += aBtnSize.Height();
        aSlideBtn.SetPosSizePixel(aVPos, aBtnSize);
        aVPos.Y() += aBtnSize.Height();
        aNotesBtn.SetPosSizePixel(aVPos, aBtnSize);
        aVPos.Y() += aBtnSize.Height();
        aHandoutBtn.SetPosSizePixel(aVPos, aBtnSize);
        aVPos.Y() += aBtnSize.Height();
        aPresentationBtn.SetPosSizePixel(aVPos, aBtnSize);
        aVPos.Y() += aBtnSize.Height();
    }
    else
    {
        // Komisch, dass das hier gemacht werden muss.
        // Eigentlich sollte man meinen, dass das in Construct() reichen wurde.
        aDrawBtn.Hide();
        aSlideBtn.Hide();
        aOutlineBtn.Hide();
        aNotesBtn.Hide();
        aHandoutBtn.Hide();
        aPresentationBtn.Hide();
    }


    // Vertical splitter (sits between vertical scrollbars.)
    Size aVSplitSize = aVSplit.GetSizePixel();
    Point aVSplitPos = aVSplit.GetPosPixel();
    aVSplitPos.X() = nRight;
    aVSplit.SetDragRectPixel(
            Rectangle(Point(nPosX, nPosY), Size(nSizeX, nSizeY)),
            &GetViewFrame()->GetWindow());

    if ( !bIsVSplit || (bIsVSplit &&
        (aVSplitPos.Y() < nPosY + MIN_SCROLLBAR_SIZE ||
         aVSplitPos.Y() > nPosY + nSizeY - MIN_SCROLLBAR_SIZE)) )
    {
        aVSplitPos.Y() = nBottom - nSplitSize;
        bIsVSplit = FALSE;
    }

    CreateVSplitElems(aVSplitPos.Y() + nSplitSize);
    aVSplitSize.Width()  = aScrBarWH.Width();
    aVSplitSize.Height() = nSplitSize;

    if ( bIsVSplit )
    {
        aVSplitPos.X() -= nSizeX;
        aVSplitSize.Width() += nSizeX;
    }
    aVSplit.SetPosPixel(aVSplitPos);
    aVSplit.SetSizePixel(aVSplitSize);


    // Vertical scrollbars.
    Size aVBarSize;
    aVBarSize.Width() = aScrBarWH.Width();
    aVBarSize.Height() = aVSplitPos.Y() - aVPos.Y();
/*
    if ( aVBarSize.Height() < MIN_SCROLLBAR_SIZE )
    {
        aVPos.Y() = aVSplitPos.Y() - MIN_SCROLLBAR_SIZE;
        aVBarSize.Height() = MIN_SCROLLBAR_SIZE;
    }
*/
    pVScrlArray[0]->SetPosSizePixel(aVPos, aVBarSize);
    aVPos.Y() += aVBarSize.Height() + nSplitSize;

    if ( pVScrlArray[1] )
    {
        aVBarSize = pVScrlArray[1]->GetSizePixel();
        aVBarSize.Height() = nBottom - aVPos.Y();
        pVScrlArray[1]->SetPosSizePixel(aVPos, aVBarSize);
        aVPos.Y() += aVBarSize.Height();
    }

    if (pScrlBox)
        pScrlBox->SetPosSizePixel(aVPos, aBtnSize);


    // Windows in the center and rulers at the left and top side.
    aAllWindowRect = Rectangle(
        aViewPos,
        Size(nSizeX-aScrBarWH.Width(), nSizeY-aScrBarWH.Height()));

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            Point   aPos;
            Size    aSize;

            if ( pWinArray[nX][nY] )
            {
                if ( nX == 0 )
                {
                    aPos.X() = nPosX;
                    aSize.Width() = aHSplitPos.X() - nPosX;
                    if ( !bIsHSplit )
                        aSize.Width() = nSizeX - aScrBarWH.Width();
                }
                else
                {
                    aPos.X() = pHScrlArray[nX]->GetPosPixel().X();
                    aSize.Width() = pHScrlArray[nX]->GetSizePixel().Width();
                }

                if ( nY == 0 )
                {
                    aPos.Y() = nPosY;
                    aSize.Height() = aVSplitPos.Y() - nPosY;
                    if ( !bIsVSplit )
                        aSize.Height() = nSizeY - aScrBarWH.Height();
                }
                else
                {
                    aPos.Y() = pVScrlArray[nY]->GetPosPixel().Y();
                    aSize.Height() = pVScrlArray[nY]->GetSizePixel().Height();
                }

                Size aHRulerSize(0,0);
                Size aVRulerSize(0,0);

                if ( bHasRuler )
                {
                    // Lineale am linken und oberen Rand anpassen
                    if ( pHRulerArray[nX] && nY == 0 )
                    {
                        aHRulerSize = pHRulerArray[nX]->GetSizePixel();
                        aHRulerSize.Width() = aSize.Width();
                        pHRulerArray[nX]->SetPosSizePixel(aPos, aHRulerSize);
                        if ( nX == 0 && pVRulerArray[nY] )
                            pHRulerArray[nX]->SetBorderPos(
                                pVRulerArray[nY]->GetSizePixel().Width()-1);
                        aPos.Y() += aHRulerSize.Height();
                        aSize.Height() -= aHRulerSize.Height();
                    }
                    if ( pVRulerArray[nY] && nX == 0 )
                    {
                        aVRulerSize = pVRulerArray[nY]->GetSizePixel();
                        aVRulerSize.Height() = aSize.Height();
                        pVRulerArray[nY]->SetPosSizePixel(aPos, aVRulerSize);
                        aPos.X() += aVRulerSize.Width();
                        aSize.Width() -= aVRulerSize.Width();
                    }
                }
                if ( bIsHSplit || bIsVSplit )
                {
                    pWinArray[nX][nY]->SetPosSizePixel(aPos, aSize);
                    aAllWindowRect.Union(Rectangle(aPos, aSize));
                }
                else    // hier wird die oben verhinderte MinZoom-Berechnung
                {       // nachgeholt
                    // pWinArray[nX][nY]->SetPosSizePixel(aPos, aSize);
                    // pWinArray[0][0]->CalcMinZoom();
                }
                pWinArray[nX][nY]->UpdateMapOrigin();
            }
        }
    }
    UpdateScrollBars();
    bFunctionIsRunning = false;
}

/*************************************************************************
|*
|* Splitter-Handling
|*
\************************************************************************/

IMPL_LINK( ViewShell, SplitHdl, Splitter *, pSplit )
{
    Point   aSplitPos = pSplit->GetPosPixel();
    long    nNewSplit = pSplit->GetSplitPosPixel();

    if ( pSplit == &aHSplit )
    {
        bIsHSplit = TRUE;
        aSplitPos.X() = nNewSplit;
    }
    if ( pSplit == &aVSplit )
    {
        bIsVSplit = TRUE;
        aSplitPos.Y() = nNewSplit;
    }

    // Wenn IP aktiv, wird der IP-Modus abgebrochen
    OSL_ASSERT (GetViewShell()!=NULL);
    Client* pIPClient = static_cast<Client*>(GetViewShell()->GetIPClient());
    if (pIPClient)
    {
        pIPClient->GetProtocol().Reset2Open();
    }

    pSplit->SetPosPixel(aSplitPos);
    AdjustPosSizePixel(aViewPos, aViewSize);
    return 0;
}

/*************************************************************************
|*
|* Fenster und ScrollBars fuer Horizontales Splitting erzeugen
|* bzw. entfernen
|*
\************************************************************************/

void ViewShell::CreateHSplitElems(long nSplitXPixel)
{
    if ( bIsHSplit )
    {   // ggf. ScrollBars und Fenster erzeugen
        if ( !pHScrlArray[1] )
        {
            pHScrlArray[1] = new ScrollBar(&GetViewFrame()->GetWindow(),
                                            WinBits(WB_HSCROLL | WB_DRAG));
            pHScrlArray[1]->SetRange(Range(0, 32000));
            pHScrlArray[1]->SetScrollHdl(LINK(this, ViewShell, HScrollHdl));
            pHScrlArray[1]->Show();

            pWinArray[1][0] = new ::sd::Window(&GetViewFrame()->GetWindow());
            pWinArray[1][0]->SetCenterAllowed(bCenterAllowed);
            pWinArray[1][0]->SetViewShell(this);
            Point aPos = pWinArray[0][0]->GetWinViewPos();
            aPos.X() += pWindow->PixelToLogic(Size(nSplitXPixel, 0)).Width();
            pWinArray[1][0]->ShareViewArea(pWinArray[0][0]);
            pWinArray[1][0]->SetWinViewPos(aPos);
            AddWindow(pWinArray[1][0]);
            pWinArray[1][0]->Show();

            pHRulerArray[1] = CreateHRuler(pWinArray[1][0], FALSE);
            if ( bHasRuler && pHRulerArray[1] )
            {
                pHRulerArray[1]->Show();
                pHRulerArray[1]->SetActive(TRUE);
            }
            if ( bIsVSplit )
            {
                pWinArray[1][1] = new ::sd::Window(&GetViewFrame()->GetWindow());
                pWinArray[1][1]->SetCenterAllowed(bCenterAllowed);
                pWinArray[1][1]->SetViewShell(this);
                aPos.Y() = pWinArray[0][1]->GetWinViewPos().Y();
                pWinArray[1][1]->ShareViewArea(pWinArray[1][0]);
                pWinArray[1][1]->SetWinViewPos(aPos);
                AddWindow(pWinArray[1][1]);
                pWinArray[1][1]->Show();
            }
        }
    }
    else
    {   // Nicht benutzte Elemente loeschen
        delete pHScrlArray[1];
        pHScrlArray[1] = NULL;

        if ( pWindow == pWinArray[1][0] || pWindow == pWinArray[1][1] )
        {
            SetActiveWindow(pWinArray[0][0]);
        }
        if ( pWinArray[1][0] )
            RemoveWindow(pWinArray[1][0]);
        delete pWinArray[1][0];
        pWinArray[1][0] = NULL;

        if ( pWinArray[1][1] )
            RemoveWindow(pWinArray[1][1]);
        delete pWinArray[1][1];
        pWinArray[1][1] = NULL;

        delete pHRulerArray[1];
        pHRulerArray[1] = NULL;
    }
}

/*************************************************************************
|*
|* Fenster und ScrollBars fuer Vertikales Splitting erzeugen
|* bzw. entfernen
|*
\************************************************************************/

void ViewShell::CreateVSplitElems(long nSplitYPixel)
{
    if ( bIsVSplit )
    {   // ggf. ScrollBars und Fenster erzeugen
        if ( !pVScrlArray[1] )
        {
            pVScrlArray[1] = new ScrollBar(&GetViewFrame()->GetWindow(),
                                            WinBits(WB_VSCROLL | WB_DRAG));
            pVScrlArray[1]->SetRange(Range(0, 32000));
            pVScrlArray[1]->SetScrollHdl(LINK(this, ViewShell, VScrollHdl));
            pVScrlArray[1]->Show();

            pWinArray[0][1] = new ::sd::Window(&GetViewFrame()->GetWindow());
            pWinArray[0][1]->SetCenterAllowed(bCenterAllowed);
            pWinArray[0][1]->SetViewShell(this);
            Point aPos = pWinArray[0][0]->GetWinViewPos();
            aPos.Y() += pWindow->PixelToLogic(Size(0, nSplitYPixel)).Height();
            pWinArray[0][1]->ShareViewArea(pWinArray[0][0]);
            pWinArray[0][1]->SetWinViewPos(aPos);
            AddWindow(pWinArray[0][1]);
            pWinArray[0][1]->Show();

            pVRulerArray[1] = CreateVRuler(pWinArray[0][1]);
            if ( bHasRuler && pVRulerArray[1] )
            {
                pVRulerArray[1]->Show();
                pVRulerArray[1]->SetActive(TRUE);
            }
            if ( bIsHSplit )
            {
                pWinArray[1][1] = new ::sd::Window(&GetViewFrame()->GetWindow());
                pWinArray[1][1]->SetCenterAllowed(bCenterAllowed);
                pWinArray[1][1]->SetViewShell(this);
                aPos.X() = pWinArray[1][0]->GetWinViewPos().X();
                pWinArray[1][1]->ShareViewArea(pWinArray[0][1]);
                pWinArray[1][1]->SetWinViewPos(aPos);
                AddWindow(pWinArray[1][1]);
                pWinArray[1][1]->Show();
            }
        }
    }
    else
    {   // Nicht benutzte ScrollBars und Fenster loeschen
        delete pVScrlArray[1];
        pVScrlArray[1] = NULL;

        if ( pWindow == pWinArray[0][1] || pWindow == pWinArray[1][1] )
        {
            SetActiveWindow(pWinArray[0][0]);
        }
        if ( pWinArray[0][1] )
            RemoveWindow(pWinArray[0][1]);
        delete pWinArray[0][1];
        pWinArray[0][1] = NULL;

        if ( pWinArray[1][1] )
            RemoveWindow(pWinArray[1][1]);
        delete pWinArray[1][1];
        pWinArray[1][1] = NULL;

        delete pVRulerArray[1];
        pVRulerArray[1] = NULL;
    }
}

/*************************************************************************
|*
|* Metrik setzen
|*
\************************************************************************/

void ViewShell::SetUIUnit(FieldUnit eUnit)
{
    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        // Metrik an den horizontalen Linealen setzen
        if (pHRulerArray[nX])
        {
            pHRulerArray[nX]->SetUnit(eUnit);
        }
    }
    for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
    {
        // Metrik an den vertikalen Linealen setzen
        if (pVRulerArray[nY])
        {
            pVRulerArray[nY]->SetUnit(eUnit);
        }
    }
}

/*************************************************************************
|*
|* DefTab an den horizontalen Linealen setzen
|*
\************************************************************************/
void ViewShell::SetDefTabHRuler( UINT16 nDefTab )
{
    for (USHORT nIndex=0; nIndex<MAX_HSPLIT_CNT; nIndex++)
    {
        if (pHRulerArray[nIndex])
        {
            pHRulerArray[nIndex]->SetDefTabDist( nDefTab );
        }
    }
}

/*************************************************************************
|*
|* Der FmShell das PrepareClose mitteilen, damit diese ihre Datensaetze
|* speichern kann.
|*
\************************************************************************/
USHORT ViewShell::PrepareClose( BOOL bUI, BOOL bForBrowsing )
{
    USHORT nResult = 0;

    FmFormShell* pFormShell = GetObjectBarManager().GetFormShell();
    if (pFormShell != NULL)
        nResult = pFormShell->PrepareClose (bUI, bForBrowsing);

    return nResult;
}

/*************************************************************************
|*
|* Update preview context
|*
\************************************************************************/
void ViewShell::UpdatePreview( SdPage* pPage, BOOL bInit )
{
    SfxChildWindow* pPreviewChildWindow = GetViewFrame()->GetChildWindow(
        PreviewChildWindow::GetChildWindowId() );
    if ( pPreviewChildWindow )
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
        if(bHasRuler)
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
        if(bHasRuler)
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

            pWindow->SetDrawMode( nMode );
            pFrameView->SetDrawMode( nMode );
// #110094#-7
//            GetView()->ReleaseMasterPagePaintCache();
            pWindow->Invalidate();

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
::Window* ViewShell::GetWindow (void) const
{
    OSL_ASSERT(GetViewShell()!=NULL);
    return GetViewShell()->GetWindow();
}




ViewShellBase& ViewShell::GetViewShellBase (void) const
{
    return *static_cast<ViewShellBase*>(GetViewShell());
}




ObjectBarManager& ViewShell::GetObjectBarManager (void) const
{
    return static_cast<ViewShellBase*>(GetViewShell())->GetObjectBarManager();
}




DrawController* ViewShell::GetController (void)
{
    return mpController;
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
            case VCLEVENT_WINDOW_SHOW:
            case VCLEVENT_WINDOW_ACTIVATE:
            case VCLEVENT_WINDOW_RESIZE:
            {
                if ( ! GetDocSh()->GetProtocol().IsInPlaceActive())
                {
                    // Forward the event only when in in-place mode
                    // which is handled differently (InnerResize from
                    // the ViewShellBase class is forwarded).
                    ::Window* pWindow = pWindowEvent->GetWindow();
                    if (pWindow != NULL)
                        OuterResizePixel (Point(),
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


} // end of namespace sd
