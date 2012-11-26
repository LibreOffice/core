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
    pView->UnmarkAllObj();
}

bool FuFormatPaintBrush::MouseButtonDown(const MouseEvent& rMEvt)
{
    if(mpView&&mpWindow)
    {
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
        const double fHitLog(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(HITPIX, 0.0)).getLength());

        if( (eHit == SDRHIT_TEXTEDIT) || (eHit == SDRHIT_TEXTEDITOBJ && ( mpViewShell->GetFrameView()->IsQuickEdit() || dynamic_cast< sdr::table::SdrTableObj* >( aVEvt.mpObj ) != NULL ) ))
        {
            SdrObject* pPickObj=0;
            const basegfx::B2DPoint aPnt(mpWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));
            mpView->PickObj( aPnt, fHitLog, pPickObj, SDRSEARCH_PICKMARKABLE);

            if( pPickObj && !pPickObj->IsEmptyPresObj() )
            {
                // if we text hit another shape than the one currently selected, unselect the old one now
                const SdrObjectVector aSelection(mpView->getSelectedSdrObjectVectorFromSdrMarkView());

                if( aSelection.size() )
                {
                    if( 1 == aSelection.size() )
                    {
                        if( aSelection[0] != pPickObj )
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

            if( aVEvt.mpObj == 0 )
                aVEvt.mpObj = pPickObj;
        }

        unmarkimpl( mpView );

        if( aVEvt.mpObj )
        {
            const bool bToggle(false);
            const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
            const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);

            mpView->MarkObj(aLogicPos, fHitLog, bToggle, false);

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
            mpWindow->SetPointer(Pointer(POINTER_FILL));
        }
        else
        {
            const double fHitLog(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(HITPIX, 0.0)).getLength());
            SdrObject* pObj=0;
            const basegfx::B2DPoint aPnt(mpWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));
            bool bOverMarkableObject = mpView->PickObj( aPnt, fHitLog, pObj, SDRSEARCH_PICKMARKABLE);

            if(bOverMarkableObject && pObj && HasContentForThisType(*pObj) )
                mpWindow->SetPointer(Pointer(POINTER_FILL));
            else
                mpWindow->SetPointer(Pointer(POINTER_ARROW));
        }
    }
    return bReturn;
}

bool FuFormatPaintBrush::MouseButtonUp(const MouseEvent& rMEvt)
{
    if( mpItemSet.get() && mpView && mpView->areSdrObjectsSelected() )
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
    if( (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE) && mpViewShell )
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

bool FuFormatPaintBrush::HasContentForThisType( const SdrObject& rSdrObject ) const
{
    if( mpItemSet.get() == 0 )
        return false;
    if( !mpView || (!mpView->SupportsFormatPaintbrush( rSdrObject ) ) )
        return false;
    return true;
}

void FuFormatPaintBrush::Paste( bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    if(mpItemSet.get())
    {
        SdrObject* pSelected = mpView->getSelectedIfSingle();

        if(pSelected)
        {
            if( mpDoc->IsUndoEnabled() )
            {
                String sLabel( mpViewShell->GetViewShellBase().RetrieveLabelFromCommand( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormatPaintbrush" ) ) ) );
                mpDoc->BegUndo( sLabel );
                    mpDoc->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoAttrObject(*pSelected, false, true));
            }

            mpView->ApplyFormatPaintBrush( *mpItemSet.get(), bNoCharacterFormats, bNoParagraphFormats );

            if( mpDoc->IsUndoEnabled() )
            {
                mpDoc->EndUndo();
            }
        }
    }
}

/* static */ void FuFormatPaintBrush::GetMenuState( DrawViewShell& rDrawViewShell, SfxItemSet &rSet )
{
    const SdrObject* pSelected = rDrawViewShell.GetDrawView()->getSelectedIfSingle();

    if( pSelected )
    {
        if( rDrawViewShell.GetDrawView()->SupportsFormatPaintbrush(*pSelected) )
            return;
    }

    rSet.DisableItem( SID_FORMATPAINTBRUSH );
}


} // end of namespace sd
