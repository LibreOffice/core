/*************************************************************************
 *
 *  $RCSfile: tabcontr.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-17 17:22:35 $
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

#include "TabControl.hxx"

#ifndef _SVDLAYER_HXX
#include <svx/svdlayer.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#pragma hdrstop

#include "sdattr.hxx"
#include "app.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#include "helpids.h"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "sdpage.hxx"
#include "drawdoc.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "unmodpg.hxx"
#include "DrawDocShell.hxx"
#include "sdresid.hxx"


namespace sd {

#define SWITCH_TIMEOUT  20

// -----------------------------------------
// - SdTabControl::SdPageObjsTransferable -
// -----------------------------------------

TabControl::TabControlTransferable::~TabControlTransferable()
{
}

// -----------------------------------------------------------------------------

void TabControl::TabControlTransferable::AddSupportedFormats()
{
    AddFormat( SOT_FORMATSTR_ID_STARDRAW_TABBAR );
}

// -----------------------------------------------------------------------------

sal_Bool TabControl::TabControlTransferable::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    return sal_False;
}

// -----------------------------------------------------------------------------

void TabControl::TabControlTransferable::DragFinished( sal_Int8 nDropAction )
{
    mrParent.DragFinished( nDropAction );
}

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

TabControl::TabControl(DrawViewShell* pViewSh, Window* pParent) :
    TabBar( pParent, WinBits( WB_BORDER | WB_3DLOOK | WB_SCROLL | WB_SIZEABLE | WB_DRAG) ),
    DragSourceHelper( this ),
    DropTargetHelper( this ),
    pDrViewSh(pViewSh),
    bInternalMove(FALSE)
{
    EnableEditMode();
    SetSizePixel(Size(0, 0));
    SetMaxPageWidth( 150 );
    SetHelpId( HID_SD_TABBAR_PAGES );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

TabControl::~TabControl()
{
}

/*************************************************************************
|*
\************************************************************************/

void TabControl::Select()
{
    SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
    pDispatcher->Execute(SID_SWITCHPAGE, SFX_CALLMODE_ASYNCHRON |
                            SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
\************************************************************************/

void  TabControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft()
        && !rMEvt.IsMod1()
        && !rMEvt.IsMod2()
        && !rMEvt.IsShift())
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

    // A single left click with pressed control key on a tab page first
    // switches to that page before the usual handling (copying with drag
    // and drop) takes place.
    else if (rMEvt.IsLeft() && rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsShift())
    {
        pDrViewSh->SwitchPage (GetPageId (rMEvt.GetPosPixel()) - 1);
    }

    // When only the right button is pressed then first process a
    // synthesized left button click to make the page the current one
    // whose tab has been clicked.  When then the actual right button
    // click is processed the resulting context menu relates to the
    // now current page.
    if (rMEvt.IsRight() && ! rMEvt.IsLeft())
    {
        MouseEvent aSyntheticEvent (
            rMEvt.GetPosPixel(),
            rMEvt.GetClicks(),
            rMEvt.GetMode(),
            MOUSE_LEFT,
            rMEvt.GetModifier());
        TabBar::MouseButtonDown(aSyntheticEvent);
    }

    TabBar::MouseButtonDown(rMEvt);
}

/*************************************************************************
|*
\************************************************************************/

void TabControl::DoubleClick()
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
|* StartDrag-Request
|*
\************************************************************************/

void TabControl::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    bInternalMove = TRUE;

    // object is delete by reference mechanismn
    ( new TabControl::TabControlTransferable( *this ) )->StartDrag( this, DND_ACTION_COPYMOVE );
}

/*************************************************************************
|*
|* DragFinished
|*
\************************************************************************/

void TabControl::DragFinished( sal_Int8 nDropAction )
{
    bInternalMove = FALSE;
}

/*************************************************************************
|*
|* AcceptDrop-Event
|*
\************************************************************************/

sal_Int8 TabControl::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( rEvt.mbLeaving )
        EndSwitchPage();

    if( !pDrViewSh->GetDocSh()->IsReadOnly() )
    {
        SdDrawDocument* pDoc = pDrViewSh->GetDoc();
        Point           aPos( rEvt.maPosPixel );

        if( bInternalMove )
        {
            if( rEvt.mbLeaving || ( pDrViewSh->GetEditMode() == EM_MASTERPAGE ) )
                HideDropPos();
            else
            {
                ShowDropPos( aPos );
                nRet = rEvt.mnAction;
            }
        }
        else
        {
            HideDropPos();

            USHORT nPageId = GetPageId( aPos ) - 1;

            if( ( nPageId >= 0 ) && pDoc->GetPage( nPageId ) )
            {
                ::sd::Window* pWindow = NULL;

                nRet = pDrViewSh->AcceptDrop( rEvt, *this, NULL, nPageId, SDRLAYER_NOTFOUND );
                SwitchPage( aPos );
            }
        }
    }

    return nRet;
}

/*************************************************************************
|*
|* ExecuteDrop-Event
|*
\************************************************************************/

sal_Int8 TabControl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    SdDrawDocument* pDoc = pDrViewSh->GetDoc();
    Point           aPos( rEvt.maPosPixel );
    sal_Int8        nRet = DND_ACTION_NONE;

    if( bInternalMove )
    {
        USHORT nPageId = ShowDropPos( aPos ) - 1;

        switch (rEvt.mnAction)
        {
            case DND_ACTION_MOVE:
                if( pDrViewSh->IsSwitchPageAllowed() && pDoc->MovePages( nPageId ) )
                {
                    SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
                    pDispatcher->Execute(SID_SWITCHPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                }
                break;

            case DND_ACTION_COPY:
            {
                // Copying the selected page to the place that rEvt points
                // takes place in three steps:
                // 1. Create a copy of the selected page.  This copy will
                // lie directly behind the selected page.
                // 2. Move the copy to the desired place.
                // 3. Select the copy.
                if (pDrViewSh->IsSwitchPageAllowed())
                {
                    // 1. Create a copy.
                    USHORT nPageNumOfCopy = pDoc->DuplicatePage (GetCurPageId() - 1);
                    // 2. Move page.  For this first switch to the copy:
                    // MovePages operates on the currently selected page(s).
                    pDrViewSh->SwitchPage (nPageNumOfCopy);
                    // Adapt target page id when necessary, i.e. page copy
                    // has been inserted in front of the target page.
                    USHORT nPageNum = nPageId;
                    if ((nPageNumOfCopy <= nPageNum) && (nPageNum != (USHORT)-1))
                        nPageNum += 1;
                    if (pDoc->MovePages(nPageNum))
                    {
                        // 3. Switch to the copy that has been moved to its
                        // final destination.  Use an asynchron slot call to
                        // be executed after the still pending ones.
                        if (nPageNumOfCopy >= nPageNum || (nPageNum == (USHORT)-1))
                            nPageNum += 1;
                        SetCurPageId (GetPageId(nPageNum));
                        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
                        pDispatcher->Execute(SID_SWITCHPAGE,
                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                    }
                }

                break;
            }
        }

        nRet = rEvt.mnAction;
    }
    else
    {
        USHORT nPageId = GetPageId( aPos ) - 1;

        if( ( nPageId >= 0 ) && pDoc->GetPage( nPageId ) )
        {
            nRet = pDrViewSh->ExecuteDrop( rEvt, *this, NULL, nPageId, SDRLAYER_NOTFOUND );
        }
    }

    HideDropPos();
    EndSwitchPage();

    return nRet;
}

/*************************************************************************
|*
\************************************************************************/

void TabControl::Command(const CommandEvent& rCEvt)
{
    USHORT nCmd = rCEvt.GetCommand();

    if ( nCmd == COMMAND_CONTEXTMENU )
    {
        BOOL bGraphicShell = pDrViewSh->ISA(GraphicViewShell);
        USHORT nResId = bGraphicShell ? RID_GRAPHIC_PAGETAB_POPUP :
                                        RID_DRAW_PAGETAB_POPUP;
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->ExecutePopup( SdResId( nResId ) );
    }
}

/*************************************************************************
|*
\************************************************************************/

long TabControl::StartRenaming()
{
    BOOL bOK = FALSE;

    if (pDrViewSh->GetPageKind() == PK_STANDARD)
    {
        bOK = TRUE;

        ::sd::View* pView = pDrViewSh->GetView();

        if ( pView->IsTextEdit() )
            pView->EndTextEdit();
    }

    return( bOK );
}

/*************************************************************************
|*
\************************************************************************/

long TabControl::AllowRenaming()
{
    BOOL bOK = TRUE;

    String aNewName( GetEditText() );
    String aCompareName( GetPageText( GetEditPageId() ) );

    if( aCompareName != aNewName )
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
    return( bOK );
}

/*************************************************************************
|*
\************************************************************************/

void TabControl::EndRenaming()
{
    if( !IsEditModeCanceled() )
        pDrViewSh->RenameSlide( GetEditPageId(), GetEditText() );
}


/*************************************************************************
|*
\************************************************************************/

void TabControl::ActivatePage()
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

long TabControl::DeactivatePage()
{
    return pDrViewSh->IsSwitchPageAllowed();
}




void TabControl::SendActivatePageEvent (void)
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}




void TabControl::SendDeactivatePageEvent (void)
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEDEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}

} // end of namespace sd
