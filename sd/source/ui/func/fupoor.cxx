/*************************************************************************
 *
 *  $RCSfile: fupoor.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:36 $
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

#pragma hdrstop

#include <svx/svxids.hrc>
#include <basctl/idetemp.hxx>

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SV_SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SFXSTBMGR_HXX //autogen
#include <sfx2/stbmgr.hxx>
#endif

#include "app.hrc"

#include "fupoor.hxx"
#include "fusel.hxx"
#include "sdpage.hxx"
#include "drawview.hxx"
#include "drviewsh.hxx"
#include "sdwindow.hxx"
#include "drawdoc.hxx"
#include "dragserv.hxx"
#include "docshell.hxx"
#include "zoomlist.hxx"
#include "fuslshow.hxx"

TYPEINIT0( FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPoor::FuPoor(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
               SdDrawDocument* pDrDoc, SfxRequest& rReq) :
    pViewShell(pViewSh),
    pWindow(pWin),
    pView(pView),
    pDoc(pDrDoc),
    pDocSh( pDrDoc->GetDocSh() ),
    nSlotId( rReq.GetSlot() ),
    pDialog(NULL),
    bIsInDragMode(FALSE),
    bScrollable (FALSE),
    bDelayActive (FALSE),
    bNoScrollUntilInside (TRUE),
    nSlotValue(0)
{
    ReceiveRequest(rReq);

    aScrollTimer.SetTimeoutHdl( LINK(this, FuPoor, ScrollHdl) );
    aScrollTimer.SetTimeout(SELENG_AUTOREPEAT_INTERVAL);

    aDragTimer.SetTimeoutHdl( LINK(this, FuPoor, DragHdl) );
    aDragTimer.SetTimeout(SELENG_DRAGDROP_TIMEOUT);

    aDelayToScrollTimer.SetTimeoutHdl( LINK(this, FuPoor, DelayHdl) );
    aDelayToScrollTimer.SetTimeout(2000);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuPoor::~FuPoor()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
    aDelayToScrollTimer.Stop ();

    if (pDialog)
        delete pDialog;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuPoor::Activate()
{
    if (pDialog)
    {
        pDialog->Show();
    }
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuPoor::Deactivate()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
    aDelayToScrollTimer.Stop ();
        bScrollable  =
        bDelayActive = FALSE;

    if (pDialog)
    {
        pDialog->Hide();
    }

    if (pWindow) pWindow->ReleaseMouse ();
}

/*************************************************************************
|*
|* Scrollen bei Erreichen des Fensterrandes; wird von
|* MouseMove aufgerufen
|*
\************************************************************************/

void FuPoor::ForceScroll(const Point& aPixPos)
{
    aScrollTimer.Stop();

    if ( !pView->IsDragHelpLine() && !pView->IsSetPageOrg() &&
         !pViewShell->GetSlideShow() )
    {
/*      Size aSize = pWindow->GetSizePixel();
        short dx = 0, dy = 0;

        if ( aPixPos.X() <= 0              ) dx = -1;
        if ( aPixPos.X() >= aSize.Width()  ) dx =  1;
        if ( aPixPos.Y() <= 0              ) dy = -1;
        if ( aPixPos.Y() >= aSize.Height() ) dy =  1;
*/
        Point aPos = pWindow->OutputToScreenPixel(aPixPos);
        const Rectangle& rRect = pViewShell->GetAllWindowRect();

        if ( bNoScrollUntilInside )
        {
            if ( rRect.IsInside(aPos) )
                bNoScrollUntilInside = FALSE;
        }
        else
        {
            short dx = 0, dy = 0;

            if ( aPos.X() <= rRect.Left()   ) dx = -1;
            if ( aPos.X() >= rRect.Right()  ) dx =  1;
            if ( aPos.Y() <= rRect.Top()    ) dy = -1;
            if ( aPos.Y() >= rRect.Bottom() ) dy =  1;

            if ( dx != 0 || dy != 0 )
            {
                if (bScrollable)
                {
                    // Scrollaktion in abgeleiteter Klasse
                    ScrollStart();
                    pViewShell->ScrollLines(dx, dy);
                    ScrollEnd();
                    aScrollTimer.Start();
                }
                else if (! bDelayActive) StartDelayToScrollTimer ();
            }
        }
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Fensterscrolling
|*
\************************************************************************/

IMPL_LINK_INLINE_START( FuPoor, ScrollHdl, Timer *, pTimer )
{
    Point aPnt(pWindow->GetPointerPosPixel());
    MouseMove(MouseEvent(aPnt));
    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, ScrollHdl, Timer *, pTimer )

/*************************************************************************
|*
|* String in Applikations-Statuszeile ausgeben
|*
\************************************************************************/

void FuPoor::WriteStatus(const String& aStr)
{
/*
//  SFX_APP()->SetHelpText(aStr);
    SfxStatusBarManager* pStatBarMan = SFX_APP()->GetStatusBarManager();
    pStatBarMan->ShowHelpText(aStr);
*/
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuPoor::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;
    BOOL bSlideShow = FALSE;
    USHORT nCode = rKEvt.GetKeyCode().GetCode();

    FuSlideShow* pFuSlideShow = pViewShell->GetSlideShow();

    if (pFuSlideShow)
    {
        bSlideShow = TRUE;
    }

    switch (nCode)
    {
        case KEY_ESCAPE:
        {
            if ( !this->ISA(FuSelection) )
            {
                // In Selektion verzweigen
                bReturn = TRUE;
                pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }
        }
        break;

        case KEY_ADD:
        {
            if (!pView->IsTextEdit() && !bSlideShow)
            {
                // Zoom vergroessern
                pViewShell->SetZoom(pWindow->GetZoom() * 3 / 2);

                if (pViewShell->ISA(SdDrawViewShell))
                    ((SdDrawViewShell*) pViewShell)->SetZoomOnPage(FALSE);

                bReturn = TRUE;
            }
        }
        break;

        case KEY_SUBTRACT:
        {
            if (!pView->IsTextEdit() && !bSlideShow)
            {
                // Zoom verringern
                pViewShell->SetZoom(pWindow->GetZoom() * 2 / 3);

                if (pViewShell->ISA(SdDrawViewShell))
                    ((SdDrawViewShell*) pViewShell)->SetZoomOnPage(FALSE);

                bReturn = TRUE;
            }
        }
        break;

        case KEY_MULTIPLY:
        {
            if (!pView->IsTextEdit() && !bSlideShow)
            {
                // Zoom auf Seite
                pViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                bReturn = TRUE;
            }
        }
        break;

        case KEY_DIVIDE:
        {
            if (!pView->IsTextEdit() && !bSlideShow)
            {
                // Zoom auf selektierte Objekte
                pViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_OPTIMAL, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                bReturn = TRUE;
            }
        }
        break;

        case KEY_POINT:
        {
            ZoomList* pZoomList = pViewShell->GetZoomList();

            if (!pView->IsTextEdit() && !bSlideShow && pZoomList->IsNextPossible())
            {
                // Naechstes ZoomRect einstellen
                pViewShell->SetZoomRect(pZoomList->GetNextZoomRect());
                bReturn = TRUE;
            }
        }
        break;

        case KEY_COMMA:
        {
            ZoomList* pZoomList = pViewShell->GetZoomList();

            if (!pView->IsTextEdit() && !bSlideShow && pZoomList->IsPreviousPossible())
            {
                // Vorheriges ZoomRect einstellen
                pViewShell->SetZoomRect(pZoomList->GetPreviousZoomRect());
                bReturn = TRUE;
            }
        }
        break;

        case KEY_HOME:
        {
            if (!pView->IsTextEdit() && pViewShell->ISA(SdDrawViewShell))
            {
               // Sprung zu erster Seite
               ((SdDrawViewShell*) pViewShell)->SwitchPage(0);
               bReturn = TRUE;
            }
        }
        break;

        case KEY_END:
        {
            if (!pView->IsTextEdit() && pViewShell->ISA(SdDrawViewShell))
            {
                // Sprung zu letzter Seite
                SdPage* pPage = ((SdDrawViewShell*) pViewShell)->GetActualPage();
                ((SdDrawViewShell*) pViewShell)->SwitchPage(pDoc->GetSdPageCount(
                                                 pPage->GetPageKind()) - 1);
                bReturn = TRUE;
            }
        }
        break;

        case KEY_PAGEUP:
        {
            pView->EndTextEdit();
            if (!rKEvt.GetKeyCode().IsMod1() && pViewShell->ISA(SdDrawViewShell))
            {
                // Vorherige Seite
                bReturn = TRUE;
                SdPage* pPage = ((SdDrawViewShell*) pViewShell)->GetActualPage();
                USHORT nSdPage = (pPage->GetPageNum() - 1) / 2;

                if (nSdPage > 0)
                    ((SdDrawViewShell*) pViewShell)->SwitchPage(nSdPage - 1);
            }
            else if (!bSlideShow)
            {
                // Scroll nach unten
                ScrollStart();
                pViewShell->ScrollLines(0, -1);
                ScrollEnd();
            }
        }
        break;

        case KEY_PAGEDOWN:
        {
            pView->EndTextEdit();
            if (!rKEvt.GetKeyCode().IsMod1() && pViewShell->ISA(SdDrawViewShell))
            {
                // Naechste Seite
                bReturn = TRUE;
                SdPage* pPage = ((SdDrawViewShell*) pViewShell)->GetActualPage();
                USHORT nSdPage = (pPage->GetPageNum() - 1) / 2;

                if (nSdPage < pDoc->GetSdPageCount(pPage->GetPageKind()) - 1)
                {
                    ((SdDrawViewShell*) pViewShell)->SwitchPage(nSdPage + 1);
                }
            }
            else if (!bSlideShow)
            {
                // Scroll nach oben
                ScrollStart();
                pViewShell->ScrollLines(0, 1);
                ScrollEnd();
            }
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            if (!pView->IsTextEdit() && !bSlideShow)
            {
                long nX = 0;
                long nY = 0;

                if (nCode == KEY_UP)
                {
                    // Scroll nach oben
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    // Scroll nach unten
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    // Scroll nach links
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    // Scroll nach rechts
                    nX = 1;
                    nY = 0;
                }

                if (pView->HasMarkedObj() && !rKEvt.GetKeyCode().IsMod1() &&
                    !pDocSh->IsReadOnly())
                {
                    // Objekte verschieben
                    nX *= 100;
                    nY *= 100;
                    pView->MoveAllMarked(Size(nX, nY));
                }
                else
                {
                    // Seite scrollen
                    ScrollStart();
                    pViewShell->ScrollLines(nX, nY);
                    ScrollEnd();
                }

                bReturn = TRUE;
            }
        }
        break;
    }

    if (bReturn)
    {
        pWindow->ReleaseMouse();
    }

    return(bReturn);
}

/*************************************************************************
|*
|* Cut object to clipboard
|*
\************************************************************************/

void FuPoor::DoCut()
{
    if (pView)
    {
        pView->DoCut(pWindow);
    }
}

/*************************************************************************
|*
|* Copy object to clipboard
|*
\************************************************************************/

void FuPoor::DoCopy()
{
    if (pView)
    {
        pView->DoCopy(pWindow);
    }
}

/*************************************************************************
|*
|* Paste object from clipboard
|*
\************************************************************************/

void FuPoor::DoPaste()
{
    if (pView)
    {
        pView->DoPaste(pWindow);
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Drag&Drop
|*
\************************************************************************/

IMPL_LINK( FuPoor, DragHdl, Timer *, pTimer )
{
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );
    SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow);

    if ( pHdl==NULL && pView->IsMarkedHit(aMDPos, nHitLog)
         && !pView->IsPresObjSelected(FALSE, TRUE) )
    {
        pWindow->ReleaseMouse();
        bIsInDragMode = TRUE;

        pView->BeginDrag(pWindow, aMDPos);
    }
    return 0;
}

/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

BOOL FuPoor::Command(const CommandEvent& rCEvt)
{
    return( pView->Command(rCEvt,pWindow) );
}

/*************************************************************************
|*
|* Timer-Handler fuer Fensterscrolling
|*
\************************************************************************/

IMPL_LINK_INLINE_START( FuPoor, DelayHdl, Timer *, pTimer )
{
    aDelayToScrollTimer.Stop ();
    bScrollable = TRUE;

    Point aPnt(pWindow->GetPointerPosPixel());
    MouseMove(MouseEvent(aPnt));
    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, DelayHdl, Timer *, pTimer )

/*************************************************************************
|*
|* Handler fuer Maustaste
|*
\************************************************************************/

BOOL __EXPORT FuPoor::MouseButtonUp (const MouseEvent& rMEvt)
{
    aDelayToScrollTimer.Stop ();
    return bScrollable  =
           bDelayActive = FALSE;
}

/*************************************************************************
|*
|* Handler fuer Maustaste
|*
\************************************************************************/

void __EXPORT FuPoor::StartDelayToScrollTimer ()
{
    bDelayActive = TRUE;
    aDelayToScrollTimer.Start ();
}

/*************************************************************************
|*
|* Handler fuer Maustaste
|*
\************************************************************************/

long FuPoor::diffPoint (long pos1, long pos2)
{
    return (pos1 > pos2) ? pos1 - pos2
                         : pos2 - pos1;
}

/*************************************************************************
|*
|* Help-event
|*
\************************************************************************/

BOOL FuPoor::RequestHelp(const HelpEvent& rHEvt)
{
    BOOL bReturn = FALSE;

    SdrPageView* pPV = pView->GetPageViewPvNum(0);

    if (pPV)
    {
        SdPage* pPage = (SdPage*) pPV->GetPage();

        if (pPage)
        {
            bReturn = pPage->RequestHelp(pWindow, pView, rHEvt);
        }
    }

    return(bReturn);
}

/*************************************************************************
|*
|* Request verarbeiten
|*
\************************************************************************/

void FuPoor::ReceiveRequest(SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();

    if (pSet)
    {
        if( pSet->GetItemState( nSlotId ) == SFX_ITEM_SET )
        {
            const SfxPoolItem& rItem = pSet->Get( nSlotId );

            if( rItem.ISA( SfxAllEnumItem ) )
            {
                nSlotValue = ( ( const SfxAllEnumItem& ) rItem ).GetValue();
            }
        }
    }
}


