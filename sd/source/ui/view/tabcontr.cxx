/*************************************************************************
 *
 *  $RCSfile: tabcontr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-08 11:25:44 $
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

#ifndef _SVDLAYER_HXX
#include <svx/svdlayer.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif

#pragma hdrstop

#include "sdattr.hxx"
#include "app.hxx"
#include "tabcontr.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"
#include "drviewsh.hxx"
#include "grviewsh.hxx"
#include "sdview.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdwindow.hxx"
#include "unmodpg.hxx"
#include "docshell.hxx"
#include "sdresid.hxx"


#define SWITCH_TIMEOUT  20


/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

SdTabControl::SdTabControl(SdDrawViewShell* pViewSh, Window* pParent) :
    TabBar( pParent, WinBits( WB_BORDER | WB_3DLOOK | WB_SCROLL | WB_SIZEABLE | WB_DRAG) ),
    pDrViewSh(pViewSh),
    bInternalMove(FALSE)
{
    EnableEditMode();
    SetSizePixel(Size(0, 0));
    SetMaxPageWidth( 150 );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdTabControl::~SdTabControl()
{
}

/*************************************************************************
|*
\************************************************************************/

void SdTabControl::Select()
{
    SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
    pDispatcher->Execute(SID_SWITCHPAGE, SFX_CALLMODE_ASYNCHRON |
                            SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
\************************************************************************/

void  SdTabControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    // eine saubere linke Maustaste ohne verwaessernde Modifier (damit koennte
    //ja das Kontextmenue gemeint sein)
    if ( rMEvt.IsLeft() && !rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsShift())
    {
        Point aPos = PixelToLogic( rMEvt.GetPosPixel() );
        USHORT aPageId = GetPageId(aPos);

        if (aPageId == 0)
        {
            SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();

            pDispatcher->Execute(SID_INSERTPAGE_QUICK,
                                SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
        }
    }

    TabBar::MouseButtonDown(rMEvt);
}

/*************************************************************************
|*
\************************************************************************/

void SdTabControl::DoubleClick()
{
    if (GetCurPageId() != 0)
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( SID_MODIFYPAGE,
                        SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
    }
}


/*************************************************************************
|*
|* QueryDrop-Event
|*
\************************************************************************/

BOOL SdTabControl::QueryDrop(DropEvent& rEvt)
{
    BOOL bReturn = FALSE;

    if ( rEvt.IsLeaveWindow() )
        EndSwitchPage();

    if (!pDrViewSh->GetDocSh()->IsReadOnly())
    {
        SdDrawDocument* pDoc = pDrViewSh->GetDoc();
        Point aPos = PixelToLogic( rEvt.GetPosPixel() );

        if (bInternalMove)
        {
            if ( rEvt.IsLeaveWindow() || pDrViewSh->GetEditMode() == EM_MASTERPAGE )
            {
                HideDropPos();
            }
            else
            {
                ShowDropPos(aPos);
                bReturn = TRUE;
            }
        }
        else
        {
            HideDropPos();
            USHORT nPageId = GetPageId(aPos) - 1;

            if ( nPageId >= 0 && pDoc->GetPage(nPageId) )
            {
                SdWindow* pWindow = NULL;
//!!!           bReturn = pDrViewSh->QueryDrop(rEvt, pWindow, nPageId, SDRLAYER_NOTFOUND);

                SwitchPage( aPos );
            }
        }
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Drop-Event
|*
\************************************************************************/

BOOL SdTabControl::Drop(const DropEvent& rEvt)
{
    BOOL bReturn = FALSE;
    SdDrawDocument* pDoc = pDrViewSh->GetDoc();
    Point aPos = PixelToLogic( rEvt.GetPosPixel() );

    if (bInternalMove)
    {
        USHORT nPageId = ShowDropPos(aPos) - 1;

        if ( pDrViewSh->IsSwitchPageAllowed() && pDoc->MovePages(nPageId) )
        {
            SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
            pDispatcher->Execute(SID_SWITCHPAGE,
                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        }

        bReturn = TRUE;
    }
    else
    {
        USHORT nPageId = GetPageId(aPos) - 1;

        if ( nPageId >= 0 && pDoc->GetPage(nPageId) )
        {
            SdWindow* pWindow = NULL;
//!!!       bReturn = pDrViewSh->Drop(rEvt, pWindow, nPageId, SDRLAYER_NOTFOUND);
        }
    }

    HideDropPos();
    EndSwitchPage();

    return (bReturn);
}


/*************************************************************************
|*
\************************************************************************/

void SdTabControl::Command(const CommandEvent& rCEvt)
{
    USHORT nCmd = rCEvt.GetCommand();

    if ( nCmd == COMMAND_CONTEXTMENU )
    {
        BOOL bGraphicShell = pDrViewSh->ISA( SdGraphicViewShell );
        USHORT nResId = bGraphicShell ? RID_GRAPHIC_PAGETAB_POPUP :
                                        RID_DRAW_PAGETAB_POPUP;
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->ExecutePopup( SdResId( nResId ) );
    }
    else if ( nCmd == COMMAND_STARTDRAG )
    {
        Region aRegion( Rectangle(0,0,0,0) );

        if ( StartDrag(rCEvt, aRegion) )
        {
            // Eigenes Format anbieten (ohne es konkret zu unterstuetzen), so
            // dass das ExecuteDrag auch ausgefuehrt wird (OS/2)
            DragServer::CopyRequest(DragServer::RegisterFormatName(
                String( RTL_CONSTASCII_USTRINGPARAM( "StarDraw TabBar" ))));

            bInternalMove = TRUE;
            ExecuteDrag( POINTER_MOVEDATA, POINTER_COPYDATA, DROP_MOVE, &aRegion );
            bInternalMove = FALSE;
        }
    }
}

/*************************************************************************
|*
\************************************************************************/

long SdTabControl::StartRenaming()
{
    BOOL bOK = FALSE;

    if (pDrViewSh->GetPageKind() == PK_STANDARD)
    {
        bOK = TRUE;

        SdView* pView = pDrViewSh->GetView();

        if ( pView->IsTextEdit() )
            pView->EndTextEdit();
    }

    return( bOK );
}

/*************************************************************************
|*
\************************************************************************/

long SdTabControl::AllowRenaming()
{
    BOOL bOK = TRUE;

    String aNewName( GetEditText() );
    String aCompareName( GetPageText( GetEditPageId() ) );

    if( aCompareName != aNewName )
    {
        if (pDrViewSh->GetEditMode() == EM_PAGE)
        {
            // Seite umbenennen
            if( pDrViewSh->GetDocSh()->CheckPageName( this, aNewName ) )
            {
                SetEditText( aNewName );
                EndRenaming();
            }
            else
            {
                bOK = FALSE;
            }
        }
        else
        {
            // MasterPage umbenennen
        }
    }
    return( bOK );
}

/*************************************************************************
|*
\************************************************************************/

void SdTabControl::EndRenaming()
{
    if( !IsEditModeCanceled() )
    {
        SdView* pView = pDrViewSh->GetView();
        SdDrawDocument* pDoc = pView->GetDoc();
        String aNewName( GetEditText() );

        if (pDrViewSh->GetEditMode() == EM_PAGE)
        {
            // Seite umbenennen
            SdView* pView = pDrViewSh->GetView();
            SdDrawDocument* pDoc = pView->GetDoc();
            PageKind ePageKind = pDrViewSh->GetPageKind();
            SdPage* pActualPage = pDoc->GetSdPage(GetEditPageId() - 1, ePageKind);

            SdPage* pUndoPage = pActualPage;
            SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
            BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
            BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
            SetOfByte aVisibleLayers = pActualPage->GetMasterPageVisibleLayers(0);

            ModifyPageUndoAction* pAction = new ModifyPageUndoAction(pDoc, pUndoPage,
                                            aNewName, pActualPage->GetAutoLayout(),
                                            aVisibleLayers.IsSet(aBckgrnd),
                                            aVisibleLayers.IsSet(aBckgrndObj));

            pDoc->GetDocSh()->GetUndoManager()->AddUndoAction(pAction);

            pActualPage->SetName(aNewName);
            aNewName = pActualPage->GetName();

            if (ePageKind == PK_STANDARD)
            {
                SdPage* pNotesPage = pDoc->GetSdPage(GetEditPageId() - 1, PK_NOTES);
                pNotesPage->SetName(aNewName);
            }
        }
        else
        {
            // MasterPage umbenennen -> LayoutTemplate umbenennen
            SdPage* pActualPage = pDoc->GetMasterSdPage(GetEditPageId() - 1, pDrViewSh->GetPageKind());
            pDoc->RenameLayoutTemplate(pActualPage->GetLayoutName(), aNewName);
            aNewName = pActualPage->GetName();
        }

        // user edited page names may be changed by the page so update control
        SetPageText( GetEditPageId(), aNewName );

        pDoc->SetChanged(TRUE);

        // Damit der Navigator das mitbekommt
        SfxBoolItem aItem(SID_NAVIGATOR_INIT, TRUE);
        pDrViewSh->GetViewFrame()->GetDispatcher()->Execute(
            SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
    }
}


/*************************************************************************
|*
\************************************************************************/

void SdTabControl::ActivatePage()
{
    if ( /*IsInSwitching && */ pDrViewSh->IsSwitchPageAllowed() )
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute(SID_SWITCHPAGE,
                             SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    }
}


/*************************************************************************
|*
\************************************************************************/

long SdTabControl::DeactivatePage()
{
    return pDrViewSh->IsSwitchPageAllowed();
}





