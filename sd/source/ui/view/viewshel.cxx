/*************************************************************************
 *
 *  $RCSfile: viewshel.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dl $ $Date: 2000-10-18 08:54:19 $
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

#include "app.hrc"
#include "helpids.h"
#include "strings.hrc"
#include "res_bmp.hrc"

#include "viewshel.hxx"
#include "sdview.hxx"
#include "sdclient.hxx"
#include "sdresid.hxx"
#include "docshell.hxx"
#include "fuslshow.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "zoomlist.hxx"
#include "grviewsh.hxx"
#include "prevchld.hxx"
#include "preview.hxx"

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

#define DELTA_ZOOM  10

/*************************************************************************
|*
|* SFX-Slotmap und Standardinterface deklarieren
|*
\************************************************************************/
TYPEINIT1( SdViewShell, SfxViewShell);

/*************************************************************************
|*
|* gemeinsamer Initialiserungsanteil der beiden Konstruktoren
|*
\************************************************************************/

void SdViewShell::Construct(void)
{
    pDocSh->Connect(this);

    pZoomList = new ZoomList( this );

    pWindow = NULL;
    SetActiveWindow(new SdWindow(&GetViewFrame()->GetWindow()));
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

    pHScrlArray[0] = new ScrollBar(&GetViewFrame()->GetWindow(),
                                    WinBits(WB_HSCROLL | WB_DRAG));
    pHScrlArray[0]->SetRange(Range(0, 32000));
    pHScrlArray[0]->SetScrollHdl(LINK(this, SdViewShell, HScrollHdl));
    pHScrlArray[0]->Show();

    pVScrlArray[0] = new ScrollBar(&GetViewFrame()->GetWindow(),
                                    WinBits(WB_VSCROLL | WB_DRAG));
    pVScrlArray[0]->SetRange(Range(0, 32000));
    pVScrlArray[0]->SetScrollHdl(LINK(this, SdViewShell, VScrollHdl));
    pVScrlArray[0]->Show();
    aScrBarWH = Size(pVScrlArray[0]->GetSizePixel().Width(),
                     pHScrlArray[0]->GetSizePixel().Height());

    aHSplit.SetSplitHdl(LINK(this, SdViewShell, SplitHdl));
    aVSplit.SetSplitHdl(LINK(this, SdViewShell, SplitHdl));
    aHSplit.Show();
    aVSplit.Show();

    if (pScrlBox)
        pScrlBox->Show();

    if( !this->ISA( SdGraphicViewShell ) )
    {
        aDrawBtn.SetImage( Image( Bitmap( SdResId( BMP_SW_DRAW ) ), IMAGE_STDBTN_COLOR ) );
        aDrawBtn.SetClickHdl(LINK(this, SdViewShell, ModeBtnHdl));
        aDrawBtn.SetQuickHelpText( String( SdResId( STR_DRAW_MODE ) ) );
        aDrawBtn.SetHelpId( HID_SD_BTN_DRAW );
        aDrawBtn.Show();

        aSlideBtn.SetImage( Image( Bitmap( SdResId( BMP_SW_SLIDE ) ), IMAGE_STDBTN_COLOR ) );
        aSlideBtn.SetClickHdl(LINK(this, SdViewShell, ModeBtnHdl));
        aSlideBtn.SetQuickHelpText( String( SdResId( STR_SLIDE_MODE ) ) );
        aSlideBtn.SetHelpId( HID_SD_BTN_SLIDE );
        aSlideBtn.Show();

        aOutlineBtn.SetImage( Image( Bitmap( SdResId( BMP_SW_OUTLINE ) ), IMAGE_STDBTN_COLOR ) );
        aOutlineBtn.SetClickHdl(LINK(this, SdViewShell, ModeBtnHdl));
        aOutlineBtn.SetQuickHelpText( String( SdResId( STR_OUTLINE_MODE ) ) );
        aOutlineBtn.SetHelpId( HID_SD_BTN_OUTLINE );
        aOutlineBtn.Show();

        aNotesBtn.SetImage( Image( Bitmap( SdResId( BMP_SW_NOTES ) ), IMAGE_STDBTN_COLOR ) );
        aNotesBtn.SetClickHdl(LINK(this, SdViewShell, ModeBtnHdl));
        aNotesBtn.SetQuickHelpText( String( SdResId( STR_NOTES_MODE ) ) );
        aNotesBtn.SetHelpId( HID_SD_BTN_NOTES );
        aNotesBtn.Show();

        aHandoutBtn.SetImage( Image( Bitmap( SdResId( BMP_SW_HANDOUT ) ), IMAGE_STDBTN_COLOR ) );
        aHandoutBtn.SetClickHdl(LINK(this, SdViewShell, ModeBtnHdl));
        aHandoutBtn.SetQuickHelpText( String( SdResId( STR_HANDOUT_MODE ) ) );
        aHandoutBtn.SetHelpId( HID_SD_BTN_HANDOUT );
        aHandoutBtn.Show();

        aPresentationBtn.SetImage( Image( Bitmap( SdResId( BMP_SW_PRESENTATION ) ), IMAGE_STDBTN_COLOR ) );
        aPresentationBtn.SetClickHdl(LINK(this, SdViewShell, ModeBtnHdl));
        aPresentationBtn.SetQuickHelpText( String( SdResId( STR_START_PRESENTATION ) ) );
        aPresentationBtn.SetHelpId( HID_SD_BTN_PRESENTATION );
        aPresentationBtn.Show();

        if (pDocSh->IsPreview())
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

    pDoc->StartOnlineSpelling(FALSE);
}

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

SdViewShell::SdViewShell(SfxViewFrame* pFrame, Window *pParent,
                         BOOL bAllowCenter) :
    SfxViewShell(pFrame, SFX_VIEW_MAXIMIZE_FIRST      |
                         SFX_VIEW_OPTIMIZE_EACH       |
                         SFX_VIEW_DISABLE_ACCELS      |
                         SFX_VIEW_OBJECTSIZE_EMBEDDED |
                         SFX_VIEW_CAN_PRINT           |
                         SFX_VIEW_HAS_PRINTOPTIONS),
    aHSplit(&pFrame->GetWindow(), WB_HSCROLL),
    aVSplit(&pFrame->GetWindow(), WB_VSCROLL),
    aDrawBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aSlideBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aOutlineBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aNotesBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aHandoutBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aPresentationBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    pDocSh( (SdDrawDocShell*) GetViewFrame()->GetObjectShell() ),
    pDoc( pDocSh->GetDoc() ),
    pFuActual(NULL),
    pFuOld(NULL),
    pFuSearch(NULL),
    pFuSlideShow(NULL),
    pZoomList(NULL),
    nCurrentObjectBar(0),
    bObjectBarSwitchEnabled(TRUE),
    pFrameView(NULL),
    bCenterAllowed(bAllowCenter),
    bStartShowWithDialog( FALSE ),
    pScrlBox(NULL),
    pView(NULL),
    nPrintedHandoutPageNum(1)
{
    pScrlBox = new ScrollBarBox(&pFrame->GetWindow(), WB_3DLOOK | WB_SIZEABLE );
    Construct();
}

/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

SdViewShell::SdViewShell(SfxViewFrame* pFrame, const SdViewShell& rShell) :
    SfxViewShell(pFrame, SFX_VIEW_MAXIMIZE_FIRST      |
                         SFX_VIEW_OPTIMIZE_EACH       |
                         SFX_VIEW_DISABLE_ACCELS      |
                         SFX_VIEW_OBJECTSIZE_EMBEDDED |
                         SFX_VIEW_HAS_PRINTOPTIONS),
    aHSplit(&pFrame->GetWindow(), WB_HSCROLL),
    aVSplit(&pFrame->GetWindow(), WB_VSCROLL),
    aDrawBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aSlideBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aOutlineBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aNotesBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aHandoutBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aPresentationBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    pDocSh( (SdDrawDocShell*) GetViewFrame()->GetObjectShell() ),
    pDoc( pDocSh->GetDoc() ),
    pFuActual(NULL),
    pFuOld(NULL),
    pFuSearch(NULL),
    pFuSlideShow(NULL),
    pZoomList(NULL),
    nCurrentObjectBar(0),
    bObjectBarSwitchEnabled(TRUE),
    pFrameView(NULL),
    bCenterAllowed(rShell.bCenterAllowed),
    pScrlBox(NULL),
    pView(NULL),
    nPrintedHandoutPageNum(1)
{
    pScrlBox = new ScrollBarBox(&pFrame->GetWindow(), WB_3DLOOK | WB_SIZEABLE );
    Construct();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdViewShell::~SdViewShell()
{
    CancelSearching();

    pDocSh->Disconnect(this);

    SetWindow(NULL);

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

    // Shells fuer Object Bars loeschen
    // vorm Zerstoeren der ObjectBarShells den Dispatcher flushen
    GetViewFrame()->GetDispatcher()->Flush();

    SfxShell* pObjBarShell = (SfxShell*)aShellTable.First();
    while (pObjBarShell)
    {
        delete pObjBarShell;
        pObjBarShell = (SfxShell*)aShellTable.Next();
    }

    delete pScrlBox;
}

/*************************************************************************
|*
|* Diese Methode deaktiviert und loescht die aktuelle Funktion. Falls es
|* eine alte Funktion gibt, wird sie als aktuelle Funktion eingesetzt und
|* aktiviert.
|*
\************************************************************************/

void SdViewShell::Cancel()
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

void __EXPORT SdViewShell::Activate(BOOL bIsMDIActivate)
{
    SfxViewShell::Activate(bIsMDIActivate);

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
        GetViewFrame()->GetDispatcher()->Execute(
            SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

        SfxBindings& rBindings = GetViewFrame()->GetBindings();
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

        if(!pDocSh->IsUIActive())
        {
            // ggfs. Preview den neuen Kontext mitteilen
            SfxChildWindow* pPreviewChildWindow =
                GetViewFrame()->GetChildWindow(SdPreviewChildWindow::GetChildWindowId());
            if (pPreviewChildWindow)
            {
                SdPreviewWin* pPreviewWin =
                    (SdPreviewWin*)pPreviewChildWindow->GetWindow();
                if (pPreviewWin)
                {
                    USHORT nSdPageNo = (GetActualPage()->GetPageNum() - 1) / 2;
                    pPreviewWin->SetContext(pDoc, nSdPageNo, pFrameView);
                }
            }
        }

        SdView* pView = GetView();

        if (pView)
        {
            pView->ShowMarkHdl(NULL);
        }
    }

    pDocSh->Connect(this);
}

/*************************************************************************
|*
|* Deaktivierung
|*
\************************************************************************/

void __EXPORT SdViewShell::Deactivate(BOOL bIsMDIActivate)
{
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
            GetViewFrame()->GetChildWindow(SdPreviewChildWindow::GetChildWindowId());
        if (pPreviewChildWindow)
        {
            SdPreviewWin* pPreviewWin =
                (SdPreviewWin*)pPreviewChildWindow->GetWindow();
            if (pPreviewWin && pPreviewWin->GetDoc() == pDoc)
            {
                pPreviewWin->SetContext(NULL, 0, NULL);
            }
        }

        SdView* pView = GetView();

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

    SfxViewShell::Deactivate(bIsMDIActivate);
}

/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

BOOL SdViewShell::KeyInput(const KeyEvent& rKEvt, SdWindow* pWin)
{
    BOOL bReturn = FALSE;

    if (pWin)
    {
        SetActiveWindow(pWin);
    }

    if (pFuSlideShow)
    {
        bReturn = pFuSlideShow->KeyInput(rKEvt);
    }
    else if (pFuActual)
    {
        bReturn = pFuActual->KeyInput(rKEvt);
    }

    if (!bReturn && pWindow)
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

    if( !bReturn )
        bReturn = SfxViewShell::KeyInput(rKEvt);

    return(bReturn);
}

/*************************************************************************
|*
|* MouseButtonDown event
|*
\************************************************************************/

void SdViewShell::MouseButtonDown(const MouseEvent& rMEvt, SdWindow* pWin)
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

void SdViewShell::MouseMove(const MouseEvent& rMEvt, SdWindow* pWin)
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

void SdViewShell::MouseButtonUp(const MouseEvent& rMEvt, SdWindow* pWin)
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

void SdViewShell::Command(const CommandEvent& rCEvt, SdWindow* pWin)
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
                const long  nOldZoom = pWindow->GetZoom();
                long        nNewZoom;

                if( pData->GetDelta() < 0L )
                    nNewZoom = Max( (long) pWin->GetMinZoom(), (long)(nOldZoom - DELTA_ZOOM) );
                else
                    nNewZoom = Min( (long) pWin->GetMaxZoom(), (long)(nOldZoom + DELTA_ZOOM) );

                SetZoom( nNewZoom );
                GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
                bDone = TRUE;
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

void __EXPORT SdViewShell::InnerResizePixel(const Point &rPos, const Size &rSize)
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

void __EXPORT SdViewShell::OuterResizePixel(const Point &rPos, const Size &rSize)
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

    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        pDocSh->SetVisArea(aVisArea);
    }

    VisAreaChanged(aVisArea);

    SdView* pView = GetView();

    if (pView)
    {
        pView->VisAreaChanged(pWindow);
    }
}

/*************************************************************************
|*
|* View-Groesse (und Position bei OLE) aendern und
|* Scrollbars/TabControl entsprechend anpassen
|*
\************************************************************************/

void SdViewShell::AdjustPosSizePixel(const Point &rNewPos, const Size &rNewSize)
{
    if ( !rNewSize.Width() || !rNewSize.Height() )
        return;

    // Rahmen fuer InPlace-Editing berechnen
    long nLeft = 0;
    long nTop  = 0;

    if ( bHasRuler )
    {
        if ( pVRulerArray[0] )
            nLeft = pVRulerArray[0]->GetSizePixel().Width();
        if ( pHRulerArray[0] )
            nTop  = pHRulerArray[0]->GetSizePixel().Height();
    }

    long nRight = pVScrlArray[0]->GetSizePixel().Width();
    long nBottom = pHScrlArray[0]->GetSizePixel().Height();
    BOOL bSlideShowActive = pFuSlideShow != NULL &&
                            !pFuSlideShow->IsTerminated() &&
                            !pFuSlideShow->IsFullScreen() &&
                            pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_SHOW;

    if( !bSlideShowActive )
    {
        SvBorder aSvBorder(nLeft, nTop, nRight, nBottom);
        // der Sfx darf immer nur das erste Fenster setzen
        SetActiveWindow(pWinArray[0][0]);
        // Sfx loest ein Resize fuer die Gesamtgroesse aus; bei aktiven Splittern
        // darf dann nicht der minimale Zoom neu berechnet werden. Falls kein
        // Splitter aktiv ist, wird die Berechnung am Ende der Methode nachgeholt
    //  pWinArray[0][0]->SetMinZoomAutoCalc(FALSE);
        SetBorderPixel(aSvBorder);
    //  pWinArray[0][0]->SetMinZoomAutoCalc(TRUE);
        SetActiveWindow(pWindow);
    }

    Size    aHBarSize;
    Size    aVBarSize;
    Size    aHSplitSize = aHSplit.GetSizePixel();
    Size    aVSplitSize = aVSplit.GetSizePixel();
    Size    aBtnSize(aScrBarWH);
    Point   aHSplitPos, aVSplitPos;
    long    nPosX = rNewPos.X();
    long    nPosY = rNewPos.Y();
    long    nSizeX = rNewSize.Width();
    long    nSizeY = rNewSize.Height();
    long    nSplitSize = aScrBarWH.Width() / 4;
    // TabControl oder aehnliches vorhanden?
    long    aHCtrlWidth = GetHCtrlWidth();
    aViewPos  = rNewPos;
    aViewSize = rNewSize;
    nSizeX -= aScrBarWH.Width();
    nSizeY -= aScrBarWH.Height();

    // Horizontaler Splitter
    aHSplitPos = aHSplit.GetPosPixel();
    aHSplitPos.Y() = nPosY + nSizeY;
    aHSplit.SetDragRectPixel(
            Rectangle(Point(nPosX, nPosY), Size(nSizeX, nSizeY)),
            &GetViewFrame()->GetWindow());
    if ( !bIsHSplit || (bIsHSplit &&
        (aHSplitPos.X() < nPosX + MIN_SCROLLBAR_SIZE ||
         aHSplitPos.X() > nPosX + nSizeX - MIN_SCROLLBAR_SIZE)) )
    {
        aHSplitPos.X() = nPosX + nSizeX - nSplitSize;
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

    // Horizontale Scrollbars
    Point aHPos(nPosX + aHCtrlWidth, nPosY + nSizeY );

    aHBarSize = pHScrlArray[0]->GetSizePixel();
    aHBarSize.Width() = aHSplitPos.X() - aHPos.X();
    if ( aHBarSize.Width() < MIN_SCROLLBAR_SIZE )
    {
        aHPos.X() = aHSplitPos.X() - MIN_SCROLLBAR_SIZE;
        aHBarSize.Width() = MIN_SCROLLBAR_SIZE;
    }
    pHScrlArray[0]->SetPosSizePixel(aHPos, aHBarSize);
    aHPos.X() += aHBarSize.Width() + nSplitSize;

    if ( pHScrlArray[1] )
    {
        aHBarSize = pHScrlArray[1]->GetSizePixel();
        aHBarSize.Width() = nSizeX - aHPos.X();
        pHScrlArray[1]->SetPosSizePixel(aHPos, aHBarSize);
    }

    // Modus-Buttons
    Point aVPos(nPosX + nSizeX, nPosY);
    if( !this->ISA( SdGraphicViewShell ) )
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

    // Vertikaler Splitter
    aVSplitPos = aVSplit.GetPosPixel();
    aVSplitPos.X() = nPosX + nSizeX;
    aVSplit.SetDragRectPixel(
            Rectangle(Point(nPosX, nPosY), Size(nSizeX, nSizeY)),
            &GetViewFrame()->GetWindow());
    if ( !bIsVSplit || (bIsVSplit &&
        (aVSplitPos.Y() < nPosY + MIN_SCROLLBAR_SIZE ||
         aVSplitPos.Y() > nPosY + nSizeY - MIN_SCROLLBAR_SIZE)) )
    {
        aVSplitPos.Y() = nPosY + nSizeY - nSplitSize;
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

    // Vertikaler Scrollbar
    aVBarSize = pVScrlArray[0]->GetSizePixel();
    aVBarSize.Height() = aVSplitPos.Y() - aVPos.Y();
    if ( aVBarSize.Height() < MIN_SCROLLBAR_SIZE )
    {
        aVPos.Y() = aVSplitPos.Y() - MIN_SCROLLBAR_SIZE;
        aVBarSize.Height() = MIN_SCROLLBAR_SIZE;
    }
    pVScrlArray[0]->SetPosSizePixel(aVPos, aVBarSize);
    aVPos.Y() += aVBarSize.Height() + nSplitSize;

    if ( pVScrlArray[1] )
    {
        aVBarSize = pVScrlArray[1]->GetSizePixel();
        aVBarSize.Height() = nSizeY - aVPos.Y();
        pVScrlArray[1]->SetPosSizePixel(aVPos, aVBarSize);
        aVPos.Y() += aVBarSize.Height();
    }

    if (pScrlBox)
        pScrlBox->SetPosSizePixel(aVPos, aBtnSize);

    aAllWindowRect = Rectangle(pWinArray[0][0]->GetPosPixel(),
                               pWinArray[0][0]->GetSizePixel());

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
                        aSize.Width() = nSizeX;
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
                        aSize.Height() = nSizeY;
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
//                  pWinArray[0][0]->CalcMinZoom();
                }
                pWinArray[nX][nY]->UpdateMapOrigin();
            }
        }
    }
    UpdateScrollBars();
}

/*************************************************************************
|*
|* Splitter-Handling
|*
\************************************************************************/

IMPL_LINK( SdViewShell, SplitHdl, Splitter *, pSplit )
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
    SdClient* pIPClient = (SdClient*)GetIPClient();
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

void SdViewShell::CreateHSplitElems(long nSplitXPixel)
{
    if ( bIsHSplit )
    {   // ggf. ScrollBars und Fenster erzeugen
        if ( !pHScrlArray[1] )
        {
            pHScrlArray[1] = new ScrollBar(&GetViewFrame()->GetWindow(),
                                            WinBits(WB_HSCROLL | WB_DRAG));
            pHScrlArray[1]->SetRange(Range(0, 32000));
            pHScrlArray[1]->SetScrollHdl(LINK(this, SdViewShell, HScrollHdl));
            pHScrlArray[1]->Show();

            pWinArray[1][0] = new SdWindow(&GetViewFrame()->GetWindow());
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
                pWinArray[1][1] = new SdWindow(&GetViewFrame()->GetWindow());
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

void SdViewShell::CreateVSplitElems(long nSplitYPixel)
{
    if ( bIsVSplit )
    {   // ggf. ScrollBars und Fenster erzeugen
        if ( !pVScrlArray[1] )
        {
            pVScrlArray[1] = new ScrollBar(&GetViewFrame()->GetWindow(),
                                            WinBits(WB_VSCROLL | WB_DRAG));
            pVScrlArray[1]->SetRange(Range(0, 32000));
            pVScrlArray[1]->SetScrollHdl(LINK(this, SdViewShell, VScrollHdl));
            pVScrlArray[1]->Show();

            pWinArray[0][1] = new SdWindow(&GetViewFrame()->GetWindow());
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
                pWinArray[1][1] = new SdWindow(&GetViewFrame()->GetWindow());
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

void SdViewShell::SetUIUnit(FieldUnit eUnit)
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
void SdViewShell::SetDefTabHRuler( UINT16 nDefTab )
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
USHORT SdViewShell::PrepareClose( BOOL bUI, BOOL bForBrowsing )
{
    USHORT nRet = SfxViewShell::PrepareClose( bUI, bForBrowsing );

    if( nRet == TRUE )
    {
        FmFormShell* pShell = (FmFormShell*)aShellTable.Get( RID_FORMLAYER_TOOLBOX );
        if( pShell )
            nRet = pShell->PrepareClose( bUI, bForBrowsing );
    }

    return nRet;
}


