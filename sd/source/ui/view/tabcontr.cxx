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

#include "TabControl.hxx"

#include <sfx2/viewfrm.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/dispatch.hxx>

#include "sdattr.hxx"
#include "sdmod.hxx"
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

// - SdTabControl::SdPageObjsTransferable -

TabControl::TabControlTransferable::~TabControlTransferable()
{
}

void TabControl::TabControlTransferable::AddSupportedFormats()
{
    AddFormat( SotClipboardFormatId::STARDRAW_TABBAR );
}

bool TabControl::TabControlTransferable::GetData( const css::datatransfer::DataFlavor& /*rFlavor*/, const OUString& /*rDestDoc*/ )
{
    return false;
}

void TabControl::TabControlTransferable::DragFinished( sal_Int8 nDropAction )
{
    mrParent.DragFinished( nDropAction );
}

TabControl::TabControl(DrawViewShell* pViewSh, vcl::Window* pParent) :
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

TabControl::~TabControl()
{
}

void TabControl::Select()
{
    SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
    pDispatcher->Execute(SID_SWITCHPAGE, SfxCallMode::ASYNCHRON |
                            SfxCallMode::RECORD);
}

void  TabControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft()
        && !rMEvt.IsMod1()
        && !rMEvt.IsMod2()
        && !rMEvt.IsShift())
    {
        Point aPos = PixelToLogic( rMEvt.GetPosPixel() );
        sal_uInt16 aPageId = GetPageId(aPos);

        //initialize
        if(RrePageID!=aPageId)
            pDrViewSh->FreshNavigatrEntry();
        RrePageID=aPageId;
        if (aPageId == 0)
        {
            SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();

            pDispatcher->Execute(SID_INSERTPAGE_QUICK,
                                SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
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

void TabControl::DoubleClick()
{
    if (GetCurPageId() != 0)
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( SID_MODIFYPAGE,
                        SfxCallMode::SYNCHRON | SfxCallMode::RECORD );
    }
}

void TabControl::StartDrag( sal_Int8, const Point& )
{
    bInternalMove = true;

    // object is delete by reference mechanismn
    ( new TabControl::TabControlTransferable( *this ) )->StartDrag( this, DND_ACTION_COPYMOVE );
}

void TabControl::DragFinished( sal_Int8 )
{
    bInternalMove = false;
}

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

            sal_Int32 nPageId = GetPageId( aPos ) - 1;

            if( ( nPageId >= 0 ) && pDoc->GetPage( (sal_uInt16)nPageId ) )
            {
                nRet = pDrViewSh->AcceptDrop( rEvt, *this, nullptr, (sal_uInt16)nPageId, SDRLAYER_NOTFOUND );
                SwitchPage( aPos );
            }
        }
    }

    return nRet;
}

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
                    pDispatcher->Execute(SID_SWITCHPAGE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
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
                    sal_uInt16 nPageNumOfCopy = pDoc->DuplicatePage (GetCurPageId() - 1);
                    // 2. Move page.  For this first switch to the copy:
                    // MovePages operates on the currently selected page(s).
                    pDrViewSh->SwitchPage (nPageNumOfCopy);
                    // Adapt target page id when necessary, i.e. page copy
                    // has been inserted in front of the target page.
                    sal_uInt16 nPageNum = nPageId;
                    if ((nPageNumOfCopy <= nPageNum) && (nPageNum != (sal_uInt16)-1))
                        nPageNum += 1;
                    if (pDoc->MovePages(nPageNum))
                    {
                        // 3. Switch to the copy that has been moved to its
                        // final destination.  Use an asynchron slot call to
                        // be executed after the still pending ones.
                        if (nPageNumOfCopy >= nPageNum || (nPageNum == (sal_uInt16)-1))
                            nPageNum += 1;
                        SetCurPageId (GetPageId(nPageNum));
                        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
                        pDispatcher->Execute(SID_SWITCHPAGE,
                            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                    }
                }

                break;
            }
        }

        nRet = rEvt.mnAction;
    }
    else
    {
        sal_Int32 nPageId = GetPageId( aPos ) - 1;

        if( ( nPageId >= 0 ) && pDoc->GetPage( (sal_uInt16)nPageId ) )
        {
            nRet = pDrViewSh->ExecuteDrop( rEvt, *this, nullptr, (sal_uInt16)nPageId, SDRLAYER_NOTFOUND );
        }
    }

    HideDropPos();
    EndSwitchPage();

    return nRet;
}

void TabControl::Command(const CommandEvent& rCEvt)
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        bool bGraphicShell = dynamic_cast< GraphicViewShell *>( pDrViewSh ) !=  nullptr;
        sal_uInt16 nResId = bGraphicShell ? RID_GRAPHIC_PAGETAB_POPUP :
                                        RID_DRAW_PAGETAB_POPUP;
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->ExecutePopup( SdResId( nResId ) );
    }
}

bool TabControl::StartRenaming()
{
    bool bOK = false;

    if (pDrViewSh->GetPageKind() == PK_STANDARD)
    {
        bOK = true;

        ::sd::View* pView = pDrViewSh->GetView();

        if ( pView->IsTextEdit() )
            pView->SdrEndTextEdit();
    }

    return bOK;
}

TabBarAllowRenamingReturnCode TabControl::AllowRenaming()
{
    bool bOK = true;

    OUString aNewName( GetEditText() );
    OUString aCompareName( GetPageText( GetEditPageId() ) );

    if( aCompareName != aNewName )
    {
        // rename page
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
    return bOK ? TABBAR_RENAMING_YES : TABBAR_RENAMING_NO;
}

void TabControl::EndRenaming()
{
    if( !IsEditModeCanceled() )
        pDrViewSh->RenameSlide( GetEditPageId(), GetEditText() );
}

void TabControl::ActivatePage()
{
    if ( /*IsInSwitching && */ pDrViewSh->IsSwitchPageAllowed() )
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute(SID_SWITCHPAGE,
                             SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
    }
}

bool TabControl::DeactivatePage()
{
    return pDrViewSh->IsSwitchPageAllowed();
}

void TabControl::SendActivatePageEvent()
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}

void TabControl::SendDeactivatePageEvent()
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEDEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
