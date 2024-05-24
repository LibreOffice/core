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

#include <fuoltext.hxx>

#include <sfx2/viewfrm.hxx>
#include <editeng/outliner.hxx>
#include <editeng/flditem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/debug.hxx>
#include <svl/stritem.hxx>

#include <svx/svxids.hrc>
#include <app.hrc>
#include <OutlineView.hxx>
#include <Window.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <OutlineViewShell.hxx>

#include <memory>

namespace sd {

const sal_uInt16 SidArray[] = {
                SID_STYLE_FAMILY2,
                SID_STYLE_FAMILY3,
                SID_STYLE_FAMILY5,
                SID_STYLE_UPDATE_BY_EXAMPLE,
                SID_CUT,
                SID_COPY,
                SID_PASTE,
                SID_SELECTALL,
                SID_ATTR_CHAR_FONT,
                SID_ATTR_CHAR_POSTURE,
                SID_ATTR_CHAR_WEIGHT,
                SID_ATTR_CHAR_SHADOWED,
                SID_ATTR_CHAR_STRIKEOUT,
                SID_ATTR_CHAR_UNDERLINE,
                SID_ATTR_CHAR_FONTHEIGHT,
                SID_ATTR_CHAR_COLOR,
                SID_ATTR_CHAR_KERNING,
                SID_OUTLINE_UP,
                SID_OUTLINE_DOWN,
                SID_OUTLINE_LEFT,
                SID_OUTLINE_RIGHT,
                //SID_OUTLINE_FORMAT,
                SID_OUTLINE_COLLAPSE_ALL,
                //SID_OUTLINE_BULLET,
                SID_OUTLINE_COLLAPSE,
                SID_OUTLINE_EXPAND_ALL,
                SID_OUTLINE_EXPAND,
                SID_SET_SUPER_SCRIPT,
                SID_SET_SUB_SCRIPT,
                SID_HYPERLINK_GETLINK,
                SID_DEC_INDENT,
                SID_INC_INDENT,
                SID_PARASPACE_INCREASE,
                SID_PARASPACE_DECREASE,
                SID_SCALE,
                SID_STATUS_PAGE,
                SID_STATUS_LAYOUT,
                SID_EXPAND_PAGE,
                SID_SUMMARY_PAGE,
                0 };

void FuOutlineText::UpdateForKeyPress (const KeyEvent& rEvent)
{
    FuSimpleOutlinerText::UpdateForKeyPress(rEvent);
    pOutlineViewShell->UpdatePreview(pOutlineViewShell->GetActualPage());
}

/**
 * Process keyboard input
 * @returns sal_True if a KeyEvent is being processed, sal_False otherwise
 */
bool FuOutlineText::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16 nKeyGroup = rKEvt.GetKeyCode().GetGroup();
    if( !mpDocSh->IsReadOnly() || nKeyGroup == KEYGROUP_CURSOR )
    {
        std::unique_ptr<OutlineViewModelChangeGuard, o3tl::default_delete<OutlineViewModelChangeGuard>> aGuard;
        if( (nKeyGroup != KEYGROUP_CURSOR) && (nKeyGroup != KEYGROUP_FKEYS) )
            aGuard.reset( new OutlineViewModelChangeGuard( *static_cast<OutlineView*>(mpSimpleOutlinerView) ) );

        return FuSimpleOutlinerText::KeyInput(rKEvt);
    }

    return false;
}

rtl::Reference<FuPoor> FuOutlineText::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::SimpleOutlinerView* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuOutlineText( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute( rReq );
    return xFunc;
}

FuOutlineText::FuOutlineText(ViewShell* pViewShell, ::sd::Window* pWindow,
                             ::sd::SimpleOutlinerView* pView, SdDrawDocument* pDoc,
                             SfxRequest& rReq)
       : FuSimpleOutlinerText(pViewShell, pWindow, pView, pDoc, rReq)
{}

FuSimpleOutlinerText::FuSimpleOutlinerText(ViewShell* pViewShell, ::sd::Window* pWindow,
                             ::sd::SimpleOutlinerView* pView, SdDrawDocument* pDoc,
                             SfxRequest& rReq)
       : FuPoor(pViewShell, pWindow, pView, pDoc, rReq),
         pOutlineViewShell (pViewShell),
         mpSimpleOutlinerView (pView)
{
}

/**
 * forward to OutlinerView
 */
bool FuSimpleOutlinerText::Command(const CommandEvent& rCEvt)
{
    bool bResult = false;

    OutlinerView* pOlView = mpSimpleOutlinerView->GetViewByWindow(mpWindow);

    DBG_ASSERT (pOlView, "no OutlineView found");

    if (pOlView)
    {
        pOlView->Command(rCEvt);        // unfortunately, we do not get a return value
        bResult = true;
    }
    return bResult;
}


rtl::Reference<FuPoor> FuSimpleOutlinerText::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::SimpleOutlinerView* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuSimpleOutlinerText( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute( rReq );
    return xFunc;
}

bool FuSimpleOutlinerText::MouseButtonDown(const MouseEvent& rMEvt)
{
    mpWindow->GrabFocus();

    bool bReturn = mpSimpleOutlinerView->GetViewByWindow(mpWindow)->MouseButtonDown(rMEvt);

    if (bReturn)
    {
        // Now the attributes of the current text position can be different
        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );
    }
    else
    {
        bReturn = FuPoor::MouseButtonDown(rMEvt);
    }

    return bReturn;
}

bool FuSimpleOutlinerText::MouseMove(const MouseEvent& rMEvt)
{
    bool bReturn = mpSimpleOutlinerView->GetViewByWindow(mpWindow)->MouseMove(rMEvt);

    if (!bReturn)
    {
        bReturn = FuPoor::MouseMove(rMEvt);
    }

    return bReturn;
}

bool FuSimpleOutlinerText::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = mpSimpleOutlinerView->GetViewByWindow(mpWindow)->MouseButtonUp(rMEvt);

    if (bReturn)
    {
        // Now the attributes of the current text position can be different
        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );
    }
    else
    {
        const SvxFieldItem* pFieldItem = mpSimpleOutlinerView->GetViewByWindow( mpWindow )->GetFieldUnderMousePointer();
        if( pFieldItem )
        {
            const SvxFieldData* pField = pFieldItem->GetField();

            if( auto pURLField = dynamic_cast< const SvxURLField *>( pField ) )
            {
                bReturn = true;
                mpWindow->ReleaseMouse();
                SfxStringItem aStrItem( SID_FILE_NAME, pURLField->GetURL() );
                SfxStringItem aReferer( SID_REFERER, mpDocSh->GetMedium()->GetName() );
                SfxBoolItem aBrowseItem( SID_BROWSE, true );
                SfxViewFrame* pFrame = mpViewShell->GetViewFrame();

                if ( rMEvt.IsMod1() )
                {
                    // open in new frame
                    pFrame->GetDispatcher()->ExecuteList(SID_OPENDOC,
                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                        { &aStrItem, &aBrowseItem, &aReferer });
                }
                else
                {
                    // open in current frame
                    SfxFrameItem aFrameItem( SID_DOCFRAME, pFrame );
                    pFrame->GetDispatcher()->ExecuteList(SID_OPENDOC,
                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                        { &aStrItem, &aFrameItem, &aBrowseItem, &aReferer });
                }
            }
        }
    }

    if( !bReturn )
        bReturn = FuPoor::MouseButtonUp(rMEvt);

    return bReturn;
}

/**
 * Process keyboard input
 * @returns sal_True if a KeyEvent is being processed, sal_False otherwise
 */
bool FuSimpleOutlinerText::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    sal_uInt16 nKeyGroup = rKEvt.GetKeyCode().GetGroup();
    if( !mpDocSh->IsReadOnly() || nKeyGroup == KEYGROUP_CURSOR )
    {
        mpWindow->GrabFocus();

        bReturn = mpSimpleOutlinerView->GetViewByWindow(mpWindow)->PostKeyEvent(rKEvt);

        if (bReturn)
        {
            UpdateForKeyPress (rKEvt);
        }
        else
        {
            bReturn = FuPoor::KeyInput(rKEvt);
        }
    }

    return bReturn;
}

void FuSimpleOutlinerText::UpdateForKeyPress (const KeyEvent& /*rEvent*/)
{
    // Attributes at the current text position may have changed.
    mpViewShell->GetViewFrame()->GetBindings().Invalidate(SidArray);
}

/**
 * Cut object to clipboard
 */
void FuSimpleOutlinerText::DoCut()
{
    mpSimpleOutlinerView->GetViewByWindow(mpWindow)->Cut();
}

/**
 * Copy object to clipboard
 */
void FuSimpleOutlinerText::DoCopy()
{
    mpSimpleOutlinerView->GetViewByWindow(mpWindow)->Copy();
}

/**
 * Paste object from clipboard
 */
void FuSimpleOutlinerText::DoPaste()
{
    mpSimpleOutlinerView->GetViewByWindow(mpWindow)->PasteSpecial();
}

/**
 * Paste object as unformatted text from clipboard
 */
void FuSimpleOutlinerText::DoPasteUnformatted()
{
   TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( mpViewShell->GetActiveWindow() ) );
   if (aDataHelper.GetTransferable().is())
   {
       OUString aText;
       if (aDataHelper.GetString(SotClipboardFormatId::STRING, aText))
           mpSimpleOutlinerView->GetViewByWindow(mpWindow)->InsertText(aText);
   }
}

} // end of namespace sd
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
