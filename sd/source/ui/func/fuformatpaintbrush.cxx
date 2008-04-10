/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fuformatpaintbrush.cxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#define _SD_DLL // fuer SD_MOD()
#include "sdmod.hxx"

#include "fuformatpaintbrush.hxx"
#include "formatclipboard.hxx"
#include "View.hxx"
#include "ViewShell.hxx"
#include "DrawDocShell.hxx"
// header for class SfxBindings
#include <sfx2/bindings.hxx>
/*
#include <svx/svxids.hrc>
*/

#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif

namespace sd {

TYPEINIT1( FuFormatPaintBrush, FuPoor );

FuFormatPaintBrush::FuFormatPaintBrush(
            ViewShell* pViewSh
            , ::sd::Window* pWin
            , ::sd::View* pView
            , SdDrawDocument* pDoc
            , SfxRequest& rReq )
        : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuFormatPaintBrush::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuFormatPaintBrush( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}

BOOL FuFormatPaintBrush::MouseButtonDown(const MouseEvent& rMEvt)
{
    if(mpView&&mpWindow)
    {
        USHORT nHitLog = USHORT ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
        BOOL bToggle = FALSE;
        mpView->UnMarkAll();
        mpView->MarkObj(mpWindow->PixelToLogic( rMEvt.GetPosPixel() ), nHitLog, bToggle, FALSE);
    }
    return FALSE;
}

BOOL FuFormatPaintBrush::MouseMove(const MouseEvent& rMEvt)
{
    SdFormatClipboard* pFormatClipboard = 0;
    if(mpViewShell)
        pFormatClipboard = mpViewShell->GetDocSh()->mpFormatClipboard;
    if(mpView&&mpWindow&&pFormatClipboard&&pFormatClipboard->HasContent())
    {
        USHORT nHitLog = USHORT ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
        SdrObject* pObj=0;
        SdrPageView* pPV=0;
        BOOL bOverMarkableObject = mpView->PickObj(
             mpWindow->PixelToLogic( rMEvt.GetPosPixel() )
            ,nHitLog, pObj, pPV, SDRSEARCH_PICKMARKABLE);

        if(bOverMarkableObject && pFormatClipboard->HasContentForThisType(pObj->GetObjInventor(),pObj->GetObjIdentifier()) )
            mpWindow->SetPointer(Pointer(POINTER_FILL));
        else
            mpWindow->SetPointer(Pointer(POINTER_ARROW));
    }
    else
        mpWindow->SetPointer(Pointer(POINTER_ARROW));
    return FALSE;
}

BOOL FuFormatPaintBrush::MouseButtonUp(const MouseEvent& rMEvt)
{
    SdFormatClipboard* pFormatClipboard = 0;
    if(mpViewShell)
        pFormatClipboard = mpViewShell->GetDocSh()->mpFormatClipboard;
    if( pFormatClipboard && mpView && mpView->AreObjectsMarked() )
    {
        bool bNoCharacterFormats = false;
        bool bNoParagraphFormats = false;
        {
            if( (rMEvt.GetModifier()&KEY_MOD1) && (rMEvt.GetModifier()&KEY_SHIFT) )
                bNoCharacterFormats = true;
            else if( rMEvt.GetModifier() & KEY_MOD1 )
                bNoParagraphFormats = true;
        }
        pFormatClipboard->Paste( *mpView, bNoCharacterFormats, bNoParagraphFormats );
        if(mpViewShell)
            mpViewShell->GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
    }
    if(mpViewShell && pFormatClipboard && !pFormatClipboard->HasContent() )
        mpViewShell->Cancel();
    return TRUE;
}

BOOL FuFormatPaintBrush::KeyInput(const KeyEvent& rKEvt)
{
    if( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE && mpViewShell )
    {
        SdFormatClipboard* pFormatClipboard = mpViewShell->GetDocSh()->mpFormatClipboard;
        if(pFormatClipboard)
        {
            pFormatClipboard->Erase();
            mpViewShell->GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
            mpViewShell->Cancel();

            return TRUE;
        }
    }
    return FuPoor::KeyInput(rKEvt);
}

void FuFormatPaintBrush::Activate()
{
}

void FuFormatPaintBrush::Deactivate()
{
}
} // end of namespace sd
