/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include <svl/itemiter.hxx>

#include <svx/globl3d.hxx>
#include <svx/svxids.hrc>
#include <svx/svdotable.hxx>
#include <editeng/outliner.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>

#define _SD_DLL // fuer SD_MOD()
#include "sdmod.hxx"

#include "fuformatpaintbrush.hxx"
#include "drawview.hxx"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "FrameView.hxx"
#include "drawdoc.hxx"
#include "Outliner.hxx"
#include "ViewShellBase.hxx"

#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif

namespace sd {

TYPEINIT1( FuFormatPaintBrush, FuText );

FuFormatPaintBrush::FuFormatPaintBrush( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
: FuText(pViewSh, pWin, pView, pDoc, rReq)
, mbPermanent( false )
, mbOldIsQuickTextEditMode( true )
{
}

FunctionReference FuFormatPaintBrush::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuFormatPaintBrush( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute( rReq );
    return xFunc;
}

void FuFormatPaintBrush::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    if( pArgs && pArgs->Count() >= 1 )
    {
        mbPermanent = static_cast<bool>(((SfxBoolItem &)pArgs->Get(SID_FORMATPAINTBRUSH)).GetValue());
    }

    if( mpView )
    {
        mpView->TakeFormatPaintBrush( mpItemSet );
    }
}

void FuFormatPaintBrush::implcancel()
{
    if( mpViewShell && mpViewShell->GetViewFrame() )
    {
        SfxViewFrame* pViewFrame = mpViewShell->GetViewFrame();
        pViewFrame->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
        pViewFrame->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
    }
}

static void unmarkimpl( SdrView* pView )
{
    pView->SdrEndTextEdit();
    pView->UnMarkAll();
}

sal_Bool FuFormatPaintBrush::MouseButtonDown(const MouseEvent& rMEvt)
{
    if(mpView&&mpWindow)
    {
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if( (eHit == SDRHIT_TEXTEDIT) || (eHit == SDRHIT_TEXTEDITOBJ && ( mpViewShell->GetFrameView()->IsQuickEdit() || dynamic_cast< sdr::table::SdrTableObj* >( aVEvt.pObj ) != NULL ) ))
        {
            SdrObject* pPickObj=0;
            SdrPageView* pPV=0;
            sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
            mpView->PickObj( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ),nHitLog, pPickObj, pPV, SDRSEARCH_PICKMARKABLE);

            if( (pPickObj != 0) && !pPickObj->IsEmptyPresObj() )
            {
                // if we text hit another shape than the one currently selected, unselect the old one now
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                if( rMarkList.GetMarkCount() >= 1 )
                {
                    if( rMarkList.GetMarkCount() == 1 )
                    {
                        if( rMarkList.GetMark(0)->GetMarkedSdrObj() != pPickObj )
                        {

                            // if current selected shape is not that of the hit text edit, deselect it
                            unmarkimpl( mpView );
                        }
                    }
                    else
                    {
                        // more than one shape selected, deselect all of them
                        unmarkimpl( mpView );
                    }
                }
                MouseEvent aMEvt( rMEvt.GetPosPixel(), rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), 0 );
                return FuText::MouseButtonDown(aMEvt);
            }

            if( aVEvt.pObj == 0 )
                aVEvt.pObj = pPickObj;
        }

        unmarkimpl( mpView );

        if( aVEvt.pObj )
        {
            sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
            sal_Bool bToggle = sal_False;
            mpView->MarkObj(mpWindow->PixelToLogic( rMEvt.GetPosPixel() ), nHitLog, bToggle, sal_False);
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool FuFormatPaintBrush::MouseMove(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;
    if( mpWindow && mpView )
    {
        if ( mpView->IsTextEdit() )
        {
            bReturn = FuText::MouseMove( rMEvt );
            mpWindow->SetPointer(Pointer(POINTER_FILL));
        }
        else
        {
            sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
            SdrObject* pObj=0;
            SdrPageView* pPV=0;
            sal_Bool bOverMarkableObject = mpView->PickObj( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ),nHitLog, pObj, pPV, SDRSEARCH_PICKMARKABLE);

            if(bOverMarkableObject && HasContentForThisType(pObj->GetObjInventor(),pObj->GetObjIdentifier()) )
                mpWindow->SetPointer(Pointer(POINTER_FILL));
            else
                mpWindow->SetPointer(Pointer(POINTER_ARROW));
        }
    }
    return bReturn;
}

sal_Bool FuFormatPaintBrush::MouseButtonUp(const MouseEvent& rMEvt)
{
    if( mpItemSet.get() && mpView && mpView->AreObjectsMarked() )
    {
        bool bNoCharacterFormats = false;
        bool bNoParagraphFormats = false;
        {
            if( (rMEvt.GetModifier()&KEY_MOD1) && (rMEvt.GetModifier()&KEY_SHIFT) )
                bNoCharacterFormats = true;
            else if( rMEvt.GetModifier() & KEY_MOD1 )
                bNoParagraphFormats = true;
        }

        OutlinerView* pOLV = mpView->GetTextEditOutlinerView();
        if( pOLV )
            pOLV->MouseButtonUp(rMEvt);

        Paste( bNoCharacterFormats, bNoParagraphFormats );
        if(mpViewShell)
            mpViewShell->GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);

        if( mbPermanent )
            return sal_True;
    }

    implcancel();
    return sal_True;
}

sal_Bool FuFormatPaintBrush::KeyInput(const KeyEvent& rKEvt)
{
    if( (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE) && mpViewShell )
    {
        implcancel();
        return sal_True;
    }
    return FuPoor::KeyInput(rKEvt);
}

void FuFormatPaintBrush::Activate()
{
    mbOldIsQuickTextEditMode = mpViewShell->GetFrameView()->IsQuickEdit();
    if( !mbOldIsQuickTextEditMode  )
    {
        mpViewShell->GetFrameView()->SetQuickEdit(sal_True);
        mpView->SetQuickTextEditMode(sal_True);
    }
}

void FuFormatPaintBrush::Deactivate()
{
    if( !mbOldIsQuickTextEditMode  )
    {
        mpViewShell->GetFrameView()->SetQuickEdit(sal_False);
        mpView->SetQuickTextEditMode(sal_False);
    }
}

bool FuFormatPaintBrush::HasContentForThisType( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier ) const
{
    if( mpItemSet.get() == 0 )
        return false;
    if( !mpView || (!mpView->SupportsFormatPaintbrush( nObjectInventor, nObjectIdentifier) ) )
        return false;
    return true;
}

void FuFormatPaintBrush::Paste( bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if(mpItemSet.get() && (rMarkList.GetMarkCount() == 1) )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

        if( mpDoc->IsUndoEnabled() )
        {
            String sLabel( mpViewShell->GetViewShellBase().RetrieveLabelFromCommand( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormatPaintbrush" ) ) ) );
            mpDoc->BegUndo( sLabel );
            mpDoc->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoAttrObject(*pObj,sal_False,sal_True));
        }

        mpView->ApplyFormatPaintBrush( *mpItemSet.get(), bNoCharacterFormats, bNoParagraphFormats );

        if( mpDoc->IsUndoEnabled() )
        {
            mpDoc->EndUndo();
        }
    }
}

/* static */ void FuFormatPaintBrush::GetMenuState( DrawViewShell& rDrawViewShell, SfxItemSet &rSet )
{
    const SdrMarkList& rMarkList = rDrawViewShell.GetDrawView()->GetMarkedObjectList();
    const sal_uLong nMarkCount = rMarkList.GetMarkCount();

    if( nMarkCount == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj && rDrawViewShell.GetDrawView()->SupportsFormatPaintbrush(pObj->GetObjInventor(),pObj->GetObjIdentifier()) )
            return;
    }
    rSet.DisableItem( SID_FORMATPAINTBRUSH );
}


} // end of namespace sd
