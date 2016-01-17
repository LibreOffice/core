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

#include "fubullet.hxx"

#include <sfx2/bindings.hxx>
#include <editeng/eeitem.hxx>
#include <svl/poolitem.hxx>
#include <editeng/fontitem.hxx>
#include "OutlineViewShell.hxx"
#include "DrawViewShell.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "strings.hrc"
#include "sdresid.hxx"
#include <svx/svdoutl.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <svl/ctloptions.hxx>
#include <svl/itempool.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "drawview.hxx"
#include <memory>

#include "app.hrc"

namespace sd {

const sal_Unicode CHAR_HARDBLANK    =   ((sal_Unicode)0x00A0);
const sal_Unicode CHAR_HARDHYPHEN   =   ((sal_Unicode)0x2011);
const sal_Unicode CHAR_SOFTHYPHEN   =   ((sal_Unicode)0x00AD);
const sal_Unicode CHAR_RLM          =   ((sal_Unicode)0x200F);
const sal_Unicode CHAR_LRM          =   ((sal_Unicode)0x200E);
const sal_Unicode CHAR_ZWSP         =   ((sal_Unicode)0x200B);
const sal_Unicode CHAR_ZWNBSP       =   ((sal_Unicode)0x2060);


FuBullet::FuBullet (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* _pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, _pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuBullet::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuBullet( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuBullet::DoExecute( SfxRequest& rReq )
{
    if( rReq.GetSlot() == SID_CHARMAP )
        InsertSpecialCharacter(rReq);
    else
    {
        sal_Unicode cMark = 0;
        switch( rReq.GetSlot() )
        {
            case FN_INSERT_SOFT_HYPHEN: cMark = CHAR_SOFTHYPHEN ; break;
            case FN_INSERT_HARDHYPHEN:  cMark = CHAR_HARDHYPHEN ; break;
            case FN_INSERT_HARD_SPACE:  cMark = CHAR_HARDBLANK ; break;
            case SID_INSERT_RLM : cMark = CHAR_RLM ; break;
            case SID_INSERT_LRM : cMark = CHAR_LRM ; break;
            case SID_INSERT_ZWSP : cMark = CHAR_ZWSP ; break;
            case SID_INSERT_ZWNBSP: cMark = CHAR_ZWNBSP; break;
        }

        DBG_ASSERT( cMark != 0, "FuBullet::FuBullet(), illegal slot used!" );

        if( cMark )
            InsertFormattingMark( cMark );
    }

}

void FuBullet::InsertFormattingMark( sal_Unicode cMark )
{
    OutlinerView* pOV = nullptr;
    ::Outliner*   pOL = nullptr;

    // depending on ViewShell set Outliner and OutlinerView
    if( dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr)
    {
        pOV = mpView->GetTextEditOutlinerView();
        if (pOV)
            pOL = mpView->GetTextEditOutliner();
    }
    else if( dynamic_cast< const OutlineViewShell *>( mpViewShell ) !=  nullptr)
    {
        pOL = &static_cast<OutlineView*>(mpView)->GetOutliner();
        pOV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
            mpViewShell->GetActiveWindow());
    }

    // insert string
    if(pOV && pOL)
    {
        // prevent flickering
        pOV->HideCursor();
        pOL->SetUpdateMode(false);

        // remove old selected text
        pOV->InsertText( "" );

        // prepare undo
        ::svl::IUndoManager& rUndoMgr =  pOL->GetUndoManager();
        rUndoMgr.EnterListAction(SD_RESSTR(STR_UNDO_INSERT_SPECCHAR),
                                    "" );

        // insert given text
        OUString aStr( cMark );
        pOV->InsertText( aStr, true);

        ESelection aSel = pOV->GetSelection();
        aSel.nStartPara = aSel.nEndPara;
        aSel.nStartPos = aSel.nEndPos;
        pOV->SetSelection(aSel);

        rUndoMgr.LeaveListAction();

        // restart repainting
        pOL->SetUpdateMode(true);
        pOV->ShowCursor();
    }
}

void FuBullet::InsertSpecialCharacter( SfxRequest& rReq )
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;
    if( pArgs )
        pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_CHARMAP), false, &pItem);

    OUString aChars;
    vcl::Font aFont;
    if ( pItem )
    {
        aChars = static_cast<const SfxStringItem*>(pItem)->GetValue();
        const SfxPoolItem* pFtItem = nullptr;
        pArgs->GetItemState( mpDoc->GetPool().GetWhich(SID_ATTR_SPECIALCHAR), false, &pFtItem);
        const SfxStringItem* pFontItem = dynamic_cast<const SfxStringItem*>( pFtItem  );
        if ( pFontItem )
        {
            OUString aFontName = pFontItem->GetValue();
            aFont = vcl::Font( aFontName, Size(1,1) );
        }
        else
        {
            SfxItemSet aFontAttr( mpDoc->GetPool() );
            mpView->GetAttributes( aFontAttr );
            const SvxFontItem* pFItem = static_cast<const SvxFontItem*>(aFontAttr.GetItem( SID_ATTR_CHAR_FONT ));
            if( pFItem )
                aFont = vcl::Font( pFItem->GetFamilyName(), pFItem->GetStyleName(), Size( 1, 1 ) );
        }
    }

    if (aChars.isEmpty())
    {
        SfxAllItemSet aSet( mpDoc->GetPool() );
        aSet.Put( SfxBoolItem( FN_PARAM_1, false ) );

        SfxItemSet aFontAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aFontAttr );
        const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(aFontAttr.GetItem( SID_ATTR_CHAR_FONT ));
        if( pFontItem )
            aSet.Put( *pFontItem );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        std::unique_ptr<SfxAbstractDialog> pDlg(pFact ? pFact->CreateSfxDialog( &mpView->GetViewShell()->GetViewFrame()->GetWindow(), aSet,
            mpView->GetViewShell()->GetViewFrame()->GetFrame().GetFrameInterface(),
            RID_SVXDLG_CHARMAP ) : nullptr);
        if( !pDlg )
            return;

        // If a character is selected, it can be shown
        // pDLg->SetFont( );
        // pDlg->SetChar( );
        sal_uInt16 nResult = pDlg->Execute();
        if( nResult == RET_OK )
        {
            const SfxStringItem* pCItem = SfxItemSet::GetItem<SfxStringItem>(pDlg->GetOutputItemSet(), SID_CHARMAP, false);
            const SvxFontItem* pFItem = SfxItemSet::GetItem<SvxFontItem>(pDlg->GetOutputItemSet(), SID_ATTR_CHAR_FONT, false);
            if ( pFItem )
            {
                aFont.SetName( pFItem->GetFamilyName() );
                aFont.SetStyleName( pFItem->GetStyleName() );
                aFont.SetCharSet( pFItem->GetCharSet() );
                aFont.SetPitch( pFItem->GetPitch() );
            }

            if ( pCItem )
                aChars  = pCItem->GetValue();
        }
    }

    if (!aChars.isEmpty())
    {
        OutlinerView* pOV = nullptr;
        ::Outliner*   pOL = nullptr;

        // determine depending on ViewShell Outliner and OutlinerView
        if(mpViewShell && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr)
        {
            pOV = mpView->GetTextEditOutlinerView();
            if (pOV)
            {
                pOL = mpView->GetTextEditOutliner();
            }
        }
        else if(mpViewShell && dynamic_cast< const OutlineViewShell *>( mpViewShell ) !=  nullptr)
        {
            pOL = &static_cast<OutlineView*>(mpView)->GetOutliner();
            pOV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
                mpViewShell->GetActiveWindow());
        }

        // insert special character
        if (pOV)
        {
            // prevent flicker
            pOV->HideCursor();
            pOL->SetUpdateMode(false);

            /* remember old attributes:
               To do that, remove selected area before (it has to go anyway).
               With that, we get unique attributes (and since there is no
               DeleteSelected() in OutlinerView, it is deleted by inserting an
               empty string). */
            pOV->InsertText( "" );

            SfxItemSet aOldSet( mpDoc->GetPool(), EE_CHAR_FONTINFO, EE_CHAR_FONTINFO, 0 );
            aOldSet.Put( pOV->GetAttribs() );

            ::svl::IUndoManager& rUndoMgr =  pOL->GetUndoManager();
            rUndoMgr.EnterListAction(SD_RESSTR(STR_UNDO_INSERT_SPECCHAR),
                                     "" );
            pOV->InsertText(aChars, true);

            // set attributes (set font)
            SfxItemSet aSet(pOL->GetEmptyItemSet());
            SvxFontItem aFontItem (aFont.GetFamily(), aFont.GetFamilyName(),
                                   aFont.GetStyleName(), aFont.GetPitch(),
                                   aFont.GetCharSet(),
                                   EE_CHAR_FONTINFO);
            aSet.Put(aFontItem);
            aSet.Put(aFontItem, EE_CHAR_FONTINFO_CJK);
            aSet.Put(aFontItem, EE_CHAR_FONTINFO_CTL);
            pOV->SetAttribs(aSet);

            ESelection aSel = pOV->GetSelection();
            aSel.nStartPara = aSel.nEndPara;
            aSel.nStartPos = aSel.nEndPos;
            pOV->SetSelection(aSel);

            // do not go ahead with setting attributes of special characters
            pOV->GetOutliner()->QuickSetAttribs(aOldSet, aSel);

            rUndoMgr.LeaveListAction();

            // show it again
            pOL->SetUpdateMode(true);
            pOV->ShowCursor();
        }
    }
}

void FuBullet::GetSlotState( SfxItemSet& rSet, ViewShell* pViewShell, SfxViewFrame* pViewFrame )
{
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_CHARMAP ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( FN_INSERT_SOFT_HYPHEN ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( FN_INSERT_HARDHYPHEN ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( FN_INSERT_HARD_SPACE ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_INSERT_RLM ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_INSERT_LRM ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_INSERT_ZWNBSP ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_INSERT_ZWSP ))
    {
        ::sd::View* pView = pViewShell ? pViewShell->GetView() : nullptr;
        OutlinerView* pOLV = pView ? pView->GetTextEditOutlinerView() : nullptr;

        const bool bTextEdit = pOLV;

        SvtCTLOptions aCTLOptions;
        const bool bCtlEnabled = aCTLOptions.IsCTLFontEnabled();

        if(!bTextEdit )
        {
            rSet.DisableItem(FN_INSERT_SOFT_HYPHEN);
            rSet.DisableItem(FN_INSERT_HARDHYPHEN);
            rSet.DisableItem(FN_INSERT_HARD_SPACE);
        }

        if( !bTextEdit && (dynamic_cast<OutlineViewShell*>( pViewShell ) == nullptr) )
            rSet.DisableItem(SID_CHARMAP);

        if(!bTextEdit || !bCtlEnabled )
        {
            rSet.DisableItem(SID_INSERT_RLM);
            rSet.DisableItem(SID_INSERT_LRM);
            rSet.DisableItem(SID_INSERT_ZWNBSP);
            rSet.DisableItem(SID_INSERT_ZWSP);
        }

        if( pViewFrame )
        {
            SfxBindings& rBindings = pViewFrame->GetBindings();

            rBindings.SetVisibleState( SID_INSERT_RLM, bCtlEnabled );
            rBindings.SetVisibleState( SID_INSERT_LRM, bCtlEnabled );
            rBindings.SetVisibleState( SID_INSERT_ZWNBSP, bCtlEnabled );
            rBindings.SetVisibleState( SID_INSERT_ZWSP, bCtlEnabled );
        }
    }
}
} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
