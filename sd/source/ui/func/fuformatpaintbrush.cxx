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

#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include <svl/itemiter.hxx>

#include <svx/globl3d.hxx>
#include <svx/svxids.hrc>
#include <svx/svdotable.hxx>
#include <svx/svdundo.hxx>
#include <editeng/outliner.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <vcl/ptrstyle.hxx>

#include <sdmod.hxx>

#include <fuformatpaintbrush.hxx>
#include <drawview.hxx>
#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <FrameView.hxx>
#include <drawdoc.hxx>
#include <Outliner.hxx>
#include <ViewShellBase.hxx>

#include <Window.hxx>

namespace sd {


FuFormatPaintBrush::FuFormatPaintBrush( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
: FuText(pViewSh, pWin, pView, pDoc, rReq)
, mbPermanent( false )
, mbOldIsQuickTextEditMode( true )
{
}

rtl::Reference<FuPoor> FuFormatPaintBrush::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuFormatPaintBrush( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute( rReq );
    return xFunc;
}

void FuFormatPaintBrush::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    if( pArgs && pArgs->Count() >= 1 )
    {
        mbPermanent = pArgs->Get(SID_FORMATPAINTBRUSH).GetValue();
    }

    if( mpView )
    {
        mpView->TakeFormatPaintBrush( mxItemSet );
    }
}

void FuFormatPaintBrush::implcancel()
{
    if( mpViewShell && mpViewShell->GetViewFrame() )
    {
        SfxViewFrame* pViewFrame = mpViewShell->GetViewFrame();
        pViewFrame->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
        pViewFrame->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
    }
}

static void unmarkimpl( SdrView* pView )
{
    pView->SdrEndTextEdit();
    pView->UnMarkAll();
}

bool FuFormatPaintBrush::MouseButtonDown(const MouseEvent& rMEvt)
{
    if(mpView&&mpWindow)
    {
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

        if( (eHit == SdrHitKind::TextEdit) || (eHit == SdrHitKind::TextEditObj && ( mpViewShell->GetFrameView()->IsQuickEdit() || dynamic_cast< sdr::table::SdrTableObj* >( aVEvt.pObj ) != nullptr ) ))
        {
            SdrPageView* pPV=nullptr;
            sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
            SdrObject* pPickObj = mpView->PickObj(mpWindow->PixelToLogic(rMEvt.GetPosPixel()),nHitLog, pPV, SdrSearchOptions::PICKMARKABLE);
            if( (pPickObj != nullptr) && !pPickObj->IsEmptyPresObj() )
            {
                // if we text hit another shape than the one currently selected, unselect the old one now
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                if( rMarkList.GetMarkCount() > 0 )
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

            if( aVEvt.pObj == nullptr )
                aVEvt.pObj = pPickObj;
        }

        unmarkimpl( mpView );

        if( aVEvt.pObj )
        {
            sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
            mpView->MarkObj(mpWindow->PixelToLogic( rMEvt.GetPosPixel() ), nHitLog, false/*bToggle*/);
            return true;
        }
    }
    return false;
}

bool FuFormatPaintBrush::MouseMove(const MouseEvent& rMEvt)
{
    bool bReturn = false;
    if( mpWindow && mpView )
    {
        if ( mpView->IsTextEdit() )
        {
            bReturn = FuText::MouseMove( rMEvt );
            mpWindow->SetPointer(PointerStyle::Fill);
        }
        else
        {
            sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
            SdrPageView* pPV=nullptr;
            SdrObject* pObj = mpView->PickObj(mpWindow->PixelToLogic( rMEvt.GetPosPixel() ),nHitLog, pPV, SdrSearchOptions::PICKMARKABLE);
            if (pObj && HasContentForThisType(pObj->GetObjInventor(),pObj->GetObjIdentifier()) )
                mpWindow->SetPointer(PointerStyle::Fill);
            else
                mpWindow->SetPointer(PointerStyle::Arrow);
        }
    }
    return bReturn;
}

bool FuFormatPaintBrush::MouseButtonUp(const MouseEvent& rMEvt)
{
    if( mxItemSet.get() && mpView && mpView->AreObjectsMarked() )
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
            return true;
    }

    implcancel();
    return true;
}

bool FuFormatPaintBrush::KeyInput(const KeyEvent& rKEvt)
{
    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        implcancel();
        return true;
    }
    return FuPoor::KeyInput(rKEvt);
}

void FuFormatPaintBrush::Activate()
{
    mbOldIsQuickTextEditMode = mpViewShell->GetFrameView()->IsQuickEdit();
    if( !mbOldIsQuickTextEditMode  )
    {
        mpViewShell->GetFrameView()->SetQuickEdit(true);
        mpView->SetQuickTextEditMode(true);
    }
}

void FuFormatPaintBrush::Deactivate()
{
    if( !mbOldIsQuickTextEditMode  )
    {
        mpViewShell->GetFrameView()->SetQuickEdit(false);
        mpView->SetQuickTextEditMode(false);
    }
}

bool FuFormatPaintBrush::HasContentForThisType( SdrInventor nObjectInventor, sal_uInt16 nObjectIdentifier ) const
{
    if (mxItemSet == nullptr)
        return false;
    if( !mpView || (!SdrObjEditView::SupportsFormatPaintbrush( nObjectInventor, nObjectIdentifier) ) )
        return false;
    return true;
}

void FuFormatPaintBrush::Paste( bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if( !(mxItemSet.get() && ( rMarkList.GetMarkCount() == 1 )) )
        return;

    SdrObject* pObj( nullptr );
    bool bUndo = mpDoc->IsUndoEnabled();

    if( bUndo && !mpView->GetTextEditOutlinerView() )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    // n685123: ApplyFormatPaintBrush itself would store undo information
    // except in a few cases (?)
    if( pObj )
    {
        OUString sLabel( mpViewShell->GetViewShellBase().RetrieveLabelFromCommand(".uno:FormatPaintbrush" ) );
        mpDoc->BegUndo( sLabel );
        if (dynamic_cast< sdr::table::SdrTableObj* >( pObj ) == nullptr)
            mpDoc->AddUndo( mpDoc->GetSdrUndoFactory().CreateUndoAttrObject( *pObj, false, true ) );
    }

    mpView->ApplyFormatPaintBrush( *mxItemSet, bNoCharacterFormats, bNoParagraphFormats );

    if( pObj )
    {
        mpDoc->EndUndo();
    }
}

/* static */ void FuFormatPaintBrush::GetMenuState( DrawViewShell const & rDrawViewShell, SfxItemSet &rSet )
{
    const SdrMarkList& rMarkList = rDrawViewShell.GetDrawView()->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj && SdrObjEditView::SupportsFormatPaintbrush(pObj->GetObjInventor(),pObj->GetObjIdentifier()) )
            return;
    }
    rSet.DisableItem( SID_FORMATPAINTBRUSH );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
