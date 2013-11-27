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

#include "TabControl.hxx"

#include <sfx2/viewfrm.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/dispatch.hxx>


#include "sdattr.hxx"
#include "app.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"
#include "DrawViewShell.hxx"
#include "GraphicViewShell.hxx"
#include "helpids.h"
#include "View.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "Window.hxx"
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

sal_Bool TabControl::TabControlTransferable::GetData( const ::com::sun::star::datatransfer::DataFlavor& )
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
    RrePageID(1),
    pDrViewSh(pViewSh),
    bInternalMove(false)
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
        sal_uInt16 aPageId = GetPageId(aPos);

//IAccessibility2 Implementation 2009-----
        //Solution: initialize
        if(RrePageID!=aPageId)
            pDrViewSh->FreshNavigatrEntry();
        RrePageID=aPageId;
//-----IAccessibility2 Implementation 2009
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

void TabControl::StartDrag( sal_Int8, const Point& )
{
    bInternalMove = true;

    // object is delete by reference mechanismn
    ( new TabControl::TabControlTransferable( *this ) )->StartDrag( this, DND_ACTION_COPYMOVE );
}

/*************************************************************************
|*
|* DragFinished
|*
\************************************************************************/

void TabControl::DragFinished( sal_Int8 )
{
    bInternalMove = false;
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

            const sal_uInt32 nPageId(GetPageId( aPos ));

            if( ( nPageId >= 1 ) && pDoc->GetPage( nPageId - 1 ) )
            {
                nRet = pDrViewSh->AcceptDrop( rEvt, *this, NULL, nPageId - 1, SDRLAYER_NOTFOUND );
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
        sal_uInt16 nPageId = ShowDropPos( aPos ) - 1;

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
                    sal_uInt32 nPageNumOfCopy = pDoc->DuplicatePage (GetCurPageId() - 1);
                    // 2. Move page.  For this first switch to the copy:
                    // MovePages operates on the currently selected page(s).
                    pDrViewSh->SwitchPage (nPageNumOfCopy);
                    // Adapt target page id when necessary, i.e. page copy
                    // has been inserted in front of the target page.
                    sal_uInt32 nPageNum = nPageId;
                    if ((nPageNumOfCopy <= nPageNum) && (nPageNum != (sal_uInt16)-1))
                        nPageNum += 1;
                    if (pDoc->MovePages(nPageNum))
                    {
                        // 3. Switch to the copy that has been moved to its
                        // final destination.  Use an asynchron slot call to
                        // be executed after the still pending ones.
                        if (nPageNumOfCopy >= nPageNum || (nPageNum == (sal_uInt16)-1))
                            nPageNum += 1;
                        SetCurPageId(GetPageId(static_cast< sal_uInt16 >(nPageNum)));
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
        const sal_uInt32 nPageId(GetPageId( aPos ));

        if( ( nPageId >= 1 ) && pDoc->GetPage( nPageId - 1 ) )
        {
            nRet = pDrViewSh->ExecuteDrop( rEvt, *this, NULL, nPageId - 1, SDRLAYER_NOTFOUND );
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
    sal_uInt16 nCmd = rCEvt.GetCommand();

    if ( nCmd == COMMAND_CONTEXTMENU )
    {
        bool bGraphicShell = dynamic_cast< GraphicViewShell* >(pDrViewSh);
        sal_uInt16 nResId = bGraphicShell ? RID_GRAPHIC_PAGETAB_POPUP :
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
    bool bOK = false;

    if (pDrViewSh->GetPageKind() == PK_STANDARD)
    {
        bOK = true;

        ::sd::View* pView = pDrViewSh->GetView();

        if ( pView->IsTextEdit() )
            pView->SdrEndTextEdit();
    }

    return( bOK );
}

/*************************************************************************
|*
\************************************************************************/

long TabControl::AllowRenaming()
{
    bool bOK = true;

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
            bOK = false;
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
