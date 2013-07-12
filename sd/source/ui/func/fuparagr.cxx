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

#include "fuparagr.hxx"
#include <editeng/eeitem.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>
#include <editeng/editdata.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/svdoutl.hxx>

#include "app.hrc"
#include "View.hxx"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "sdabstdlg.hxx"
//#include "paragr.hrc"
#include "sdattr.hrc"

namespace sd {

TYPEINIT1( FuParagraph, FuPoor );

FuParagraph::FuParagraph (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuParagraph::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuParagraph( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuParagraph::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();

    OutlinerView* pOutlView = mpView->GetTextEditOutlinerView();
    ::Outliner* pOutliner = mpView->GetTextEditOutliner();

    if( !pArgs )
    {
        SfxItemSet aEditAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aEditAttr );
        SfxItemPool *pPool =  aEditAttr.GetPool();
        SfxItemSet aNewAttr( *pPool,
                             EE_ITEMS_START, EE_ITEMS_END,
                             SID_ATTR_TABSTOP_OFFSET, SID_ATTR_TABSTOP_OFFSET,
                             ATTR_PARANUMBERING_START, ATTR_PARANUMBERING_END,
                             0 );

        aNewAttr.Put( aEditAttr );

        // left border is offset
        const long nOff = ( (SvxLRSpaceItem&)aNewAttr.Get( EE_PARA_LRSPACE ) ).GetTxtLeft();
        // conversion since TabulatorTabPage always uses Twips!
        SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
        aNewAttr.Put( aOff );

        if( pOutlView && pOutliner )
        {
            ESelection eSelection = pOutlView->GetSelection();
            aNewAttr.Put( SfxInt16Item( ATTR_NUMBER_NEWSTART_AT, pOutliner->GetNumberingStartValue( eSelection.nStartPara ) ) );
            aNewAttr.Put( SfxBoolItem( ATTR_NUMBER_NEWSTART, pOutliner->IsParaIsNumberingRestart( eSelection.nStartPara ) ) );
        }

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        SfxAbstractTabDialog* pDlg = pFact ? pFact->CreateSdParagraphTabDlg(NULL, &aNewAttr ) : 0;
        if( pDlg )
        {
            sal_uInt16 nResult = pDlg->Execute();

            switch( nResult )
            {
                case RET_OK:
                {
                    rReq.Done( *( pDlg->GetOutputItemSet() ) );

                    pArgs = rReq.GetArgs();
                }
                break;

                default:
                {
                    delete pDlg;
                }
                return; // Cancel
            }
            delete( pDlg );
        }
    }
    mpView->SetAttributes( *pArgs );

    if( pOutlView && pOutliner )
    {
        ESelection eSelection = pOutlView->GetSelection();

        const SfxPoolItem *pItem = 0;
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_NUMBER_NEWSTART, sal_False, &pItem ) )
        {
            const sal_Bool bNewStart = ((SfxBoolItem*)pItem)->GetValue() ? sal_True : sal_False;
            pOutliner->SetParaIsNumberingRestart( eSelection.nStartPara, bNewStart );
        }

        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_NUMBER_NEWSTART_AT, sal_False, &pItem ) )
        {
            const sal_Int16 nStartAt = ((SfxInt16Item*)pItem)->GetValue();
            pOutliner->SetNumberingStartValue( eSelection.nStartPara, nStartAt );
        }
    }

    // invalidate slots
    static sal_uInt16 SidArray[] = {
        SID_ATTR_TABSTOP,
    SID_ATTR_PARA_LINESPACE,
    SID_ATTR_PARA_ULSPACE,
        SID_ATTR_PARA_ADJUST_LEFT,
        SID_ATTR_PARA_ADJUST_RIGHT,
        SID_ATTR_PARA_ADJUST_CENTER,
        SID_ATTR_PARA_ADJUST_BLOCK,
        SID_ATTR_PARA_LINESPACE_10,
        SID_ATTR_PARA_LINESPACE_15,
        SID_ATTR_PARA_LINESPACE_20,
        SID_ATTR_PARA_LRSPACE,
        SID_ATTR_PARA_LEFT_TO_RIGHT,
        SID_ATTR_PARA_RIGHT_TO_LEFT,
        SID_RULER_TEXT_RIGHT_TO_LEFT,
        SID_PARASPACE_INCREASE,
        SID_PARASPACE_DECREASE,
        0 };

    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );
}

void FuParagraph::Activate()
{
}

void FuParagraph::Deactivate()
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
