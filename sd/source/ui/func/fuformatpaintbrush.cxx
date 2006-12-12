/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuformatpaintbrush.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:18:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#define _SD_DLL // fuer SD_MOD()
#include "sdmod.hxx"

#include "fuformatpaintbrush.hxx"
#include "formatclipboard.hxx"

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
// header for class SfxBindings
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
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
