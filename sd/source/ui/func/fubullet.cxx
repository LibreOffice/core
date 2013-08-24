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

#include "app.hrc"

namespace sd {

const sal_Unicode CHAR_HARDBLANK    =   ((sal_Unicode)0x00A0);
const sal_Unicode CHAR_HARDHYPHEN   =   ((sal_Unicode)0x2011);
const sal_Unicode CHAR_SOFTHYPHEN   =   ((sal_Unicode)0x00AD);
const sal_Unicode CHAR_RLM          =   ((sal_Unicode)0x200F);
const sal_Unicode CHAR_LRM          =   ((sal_Unicode)0x200E);
const sal_Unicode CHAR_ZWSP         =   ((sal_Unicode)0x200B);
const sal_Unicode CHAR_ZWNBSP       =   ((sal_Unicode)0x2060);

TYPEINIT1( FuBullet, FuPoor );


FuBullet::FuBullet (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* _pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, _pView, pDoc, rReq)
{
}

FunctionReference FuBullet::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuBullet( pViewSh, pWin, pView, pDoc, rReq ) );
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
    OutlinerView* pOV = NULL;
    ::Outliner*   pOL = NULL;

    // depending on ViewShell set Outliner and OutlinerView
    if (mpViewShell->ISA(DrawViewShell))
    {
        pOV = mpView->GetTextEditOutlinerView();
        if (pOV)
            pOL = mpView->GetTextEditOutliner();
    }
    else if (mpViewShell->ISA(OutlineViewShell))
    {
        pOL = static_cast<OutlineView*>(mpView)->GetOutliner();
        pOV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
            mpViewShell->GetActiveWindow());
    }

    // insert string
    if(pOV && pOL)
    {
        // prevent flickering
        pOV->HideCursor();
        pOL->SetUpdateMode(sal_False);

        // remove old selected text
        pOV->InsertText( aEmptyStr );

        // prepare undo
        ::svl::IUndoManager& rUndoMgr =  pOL->GetUndoManager();
        rUndoMgr.EnterListAction(SD_RESSTR(STR_UNDO_INSERT_SPECCHAR),
                                    aEmptyStr );

        // insert given text
        OUString aStr( cMark );
        pOV->InsertText( aStr, sal_True);

        ESelection aSel = pOV->GetSelection();
        aSel.nStartPara = aSel.nEndPara;
        aSel.nStartPos = aSel.nEndPos;
        pOV->SetSelection(aSel);

        rUndoMgr.LeaveListAction();

        // restart repainting
        pOL->SetUpdateMode(sal_True);
        pOV->ShowCursor();
    }
}

void FuBullet::InsertSpecialCharacter( SfxRequest& rReq )
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    if( pArgs )
        pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_CHARMAP), sal_False, &pItem);

    OUString aChars;
    Font aFont;
    if ( pItem )
    {
        aChars = ((const SfxStringItem*)pItem)->GetValue();
        const SfxPoolItem* pFtItem = NULL;
        pArgs->GetItemState( mpDoc->GetPool().GetWhich(SID_ATTR_SPECIALCHAR), sal_False, &pFtItem);
        const SfxStringItem* pFontItem = PTR_CAST( SfxStringItem, pFtItem );
        if ( pFontItem )
        {
            OUString aFontName = pFontItem->GetValue();
            aFont = Font( aFontName, Size(1,1) );
        }
        else
        {
            SfxItemSet aFontAttr( mpDoc->GetPool() );
            mpView->GetAttributes( aFontAttr );
            const SvxFontItem* pFItem = (const SvxFontItem*)aFontAttr.GetItem( SID_ATTR_CHAR_FONT );
            if( pFItem )
                aFont = Font( pFItem->GetFamilyName(), pFItem->GetStyleName(), Size( 1, 1 ) );
        }
    }

    if (aChars.isEmpty())
    {
        SfxAllItemSet aSet( mpDoc->GetPool() );
        aSet.Put( SfxBoolItem( FN_PARAM_1, sal_False ) );

        SfxItemSet aFontAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aFontAttr );
        const SvxFontItem* pFontItem = (const SvxFontItem*)aFontAttr.GetItem( SID_ATTR_CHAR_FONT );
        if( pFontItem )
            aSet.Put( *pFontItem );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        SfxAbstractDialog* pDlg = pFact ? pFact->CreateSfxDialog( &mpView->GetViewShell()->GetViewFrame()->GetWindow(), aSet,
            mpView->GetViewShell()->GetViewFrame()->GetFrame().GetFrameInterface(),
            RID_SVXDLG_CHARMAP ) : 0;
        if( !pDlg )
            return;

        // If a character is selected, it can be shown
        // pDLg->SetFont( );
        // pDlg->SetChar( );
        sal_uInt16 nResult = pDlg->Execute();
        if( nResult == RET_OK )
        {
            SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pCItem, SfxStringItem, SID_CHARMAP, sal_False );
            SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pFItem, SvxFontItem, SID_ATTR_CHAR_FONT, sal_False );
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

        delete( pDlg );
    }

    if (!aChars.isEmpty())
    {
        OutlinerView* pOV = NULL;
        ::Outliner*   pOL = NULL;

        // determine depending on ViewShell Outliner and OutlinerView
        if(mpViewShell && mpViewShell->ISA(DrawViewShell))
        {
            pOV = mpView->GetTextEditOutlinerView();
            if (pOV)
            {
                pOL = mpView->GetTextEditOutliner();
            }
        }
        else if(mpViewShell && mpViewShell->ISA(OutlineViewShell))
        {
            pOL = static_cast<OutlineView*>(mpView)->GetOutliner();
            pOV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
                mpViewShell->GetActiveWindow());
        }

        // insert special character
        if (pOV)
        {
            // prevent flicker
            pOV->HideCursor();
            pOL->SetUpdateMode(sal_False);

            /* remember old attributes:
               To do that, remove selected area before (it has to go anyway).
               With that, we get unique attributes (and since there is no
               DeleteSelected() in OutlinerView, it is deleted by inserting an
               empty string). */
            pOV->InsertText( aEmptyStr );

            SfxItemSet aOldSet( mpDoc->GetPool(), EE_CHAR_FONTINFO, EE_CHAR_FONTINFO, 0 );
            aOldSet.Put( pOV->GetAttribs() );

            ::svl::IUndoManager& rUndoMgr =  pOL->GetUndoManager();
            rUndoMgr.EnterListAction(SD_RESSTR(STR_UNDO_INSERT_SPECCHAR),
                                     aEmptyStr );
            pOV->InsertText(aChars, sal_True);

            // set attributes (set font)
            SfxItemSet aSet(pOL->GetEmptyItemSet());
            SvxFontItem aFontItem (aFont.GetFamily(),    aFont.GetName(),
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
            pOL->SetUpdateMode(sal_True);
            pOV->ShowCursor();
        }
    }
}

void FuBullet::GetSlotState( SfxItemSet& rSet, ViewShell* pViewShell, SfxViewFrame* pViewFrame )
{
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CHARMAP ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( FN_INSERT_SOFT_HYPHEN ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( FN_INSERT_HARDHYPHEN ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( FN_INSERT_HARD_SPACE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_RLM ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_LRM ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_ZWNBSP ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_ZWSP ))
    {
        ::sd::View* pView = pViewShell ? pViewShell->GetView() : 0;
        OutlinerView* pOLV = pView ? pView->GetTextEditOutlinerView() : 0;

        const bool bTextEdit = pOLV;

        SvtCTLOptions aCTLOptions;
        const sal_Bool bCtlEnabled = aCTLOptions.IsCTLFontEnabled();

        if(!bTextEdit )
        {
            rSet.DisableItem(FN_INSERT_SOFT_HYPHEN);
            rSet.DisableItem(FN_INSERT_HARDHYPHEN);
            rSet.DisableItem(FN_INSERT_HARD_SPACE);
        }

        if( !bTextEdit && (dynamic_cast<OutlineViewShell*>( pViewShell ) == 0) )
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
