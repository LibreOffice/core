/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuformatpaintbrush.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:50:20 $
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
    if(pView&&pWindow)
    {
        USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );
        BOOL bToggle = FALSE;
        pView->UnMarkAll();
        pView->MarkObj(pWindow->PixelToLogic( rMEvt.GetPosPixel() ), nHitLog, bToggle, FALSE);
    }
    return FALSE;
}

BOOL FuFormatPaintBrush::MouseMove(const MouseEvent& rMEvt)
{
    SdFormatClipboard* pFormatClipboard = 0;
    if(pViewShell)
        pFormatClipboard = pViewShell->GetDocSh()->pFormatClipboard;
    if(pView&&pWindow&&pFormatClipboard&&pFormatClipboard->HasContent())
    {
        USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );
        SdrObject* pObj=0;
        SdrPageView* pPV=0;
        BOOL bOverMarkableObject = pView->PickObj(
             pWindow->PixelToLogic( rMEvt.GetPosPixel() )
            ,nHitLog, pObj, pPV, SDRSEARCH_PICKMARKABLE);

        if(bOverMarkableObject && pFormatClipboard->HasContentForThisType(pObj->GetObjInventor(),pObj->GetObjIdentifier()) )
            pWindow->SetPointer(Pointer(POINTER_FILL));
        else
            pWindow->SetPointer(Pointer(POINTER_ARROW));
    }
    else
        pWindow->SetPointer(Pointer(POINTER_ARROW));
    return FALSE;
}

BOOL FuFormatPaintBrush::MouseButtonUp(const MouseEvent& rMEvt)
{
    SdFormatClipboard* pFormatClipboard = 0;
    if(pViewShell)
        pFormatClipboard = pViewShell->GetDocSh()->pFormatClipboard;
    if( pFormatClipboard && pView && pView->AreObjectsMarked() )
    {
        bool bNoCharacterFormats = false;
        bool bNoParagraphFormats = false;
        {
            if( (rMEvt.GetModifier()&KEY_MOD1) && (rMEvt.GetModifier()&KEY_SHIFT) )
                bNoCharacterFormats = true;
            else if( rMEvt.GetModifier() & KEY_MOD1 )
                bNoParagraphFormats = true;
        }
        pFormatClipboard->Paste( *pView, bNoCharacterFormats, bNoParagraphFormats );
        if(pViewShell)
            pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
    }
    if(pViewShell && pFormatClipboard && !pFormatClipboard->HasContent() )
        pViewShell->Cancel();
    return TRUE;
}

BOOL FuFormatPaintBrush::KeyInput(const KeyEvent& rKEvt)
{
    if( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE && pViewShell )
    {
        SdFormatClipboard* pFormatClipboard = pViewShell->GetDocSh()->pFormatClipboard;
        if(pFormatClipboard)
        {
            pFormatClipboard->Erase();
            pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
            pViewShell->Cancel();

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
