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

#include <editeng/eeitem.hxx>

#include <svx/svxdlg.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/editids.hrc>
#include <svx/svxids.hrc>
#include <vcl/transfer.hxx>

#include <drtxtob.hxx>
#include <drawview.hxx>
#include <viewdata.hxx>
#include <gridwin.hxx>

#include <scabstdlg.hxx>

bool ScDrawTextObjectBar::ExecuteCharDlg( const SfxItemSet& rArgs,
                                                SfxItemSet& rOutSet , sal_uInt16 nSlot)
{
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScCharDlg(
        pViewData->GetFrameWeld(), &rArgs,
        pViewData->GetSfxDocShell()));
    if (nSlot == SID_CHAR_DLG_EFFECT)
    {
        pDlg->SetCurPageId("fonteffects");
    }
    bool bRet = ( pDlg->Execute() == RET_OK );

    if ( bRet )
    {
        const SfxItemSet* pNewAttrs = pDlg->GetOutputItemSet();
        if ( pNewAttrs )
            rOutSet.Put( *pNewAttrs );
    }

    return bRet;
}

bool ScDrawTextObjectBar::ExecuteParaDlg( const SfxItemSet& rArgs,
                                                SfxItemSet& rOutSet )
{
    SfxItemPool* pArgPool = rArgs.GetPool();
    SfxItemSet aNewAttr(
        *pArgPool,
        svl::Items<
            EE_ITEMS_START, EE_ITEMS_END,
            SID_ATTR_PARA_PAGEBREAK, SID_ATTR_PARA_WIDOWS>{});
    aNewAttr.Put( rArgs );

    // Values have been taken over once to show the dialog.
    // Has to be changed
    // aNewAttr.Put( SvxParaDlgLimitsItem( 567 * 50, 5670) );

    aNewAttr.Put( SvxHyphenZoneItem( false, SID_ATTR_PARA_HYPHENZONE ) );
    aNewAttr.Put( SvxFormatBreakItem( SvxBreak::NONE, SID_ATTR_PARA_PAGEBREAK ) );
    aNewAttr.Put( SvxFormatSplitItem( true, SID_ATTR_PARA_SPLIT)  );
    aNewAttr.Put( SvxWidowsItem( 0, SID_ATTR_PARA_WIDOWS) );
    aNewAttr.Put( SvxOrphansItem( 0, SID_ATTR_PARA_ORPHANS) );

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScParagraphDlg(
        pViewData->GetFrameWeld(), &aNewAttr));
    bool bRet = ( pDlg->Execute() == RET_OK );

    if ( bRet )
    {
        const SfxItemSet* pNewAttrs = pDlg->GetOutputItemSet();
        if ( pNewAttrs )
            rOutSet.Put( *pNewAttrs );
    }

    return bRet;
}

void ScDrawTextObjectBar::ExecutePasteContents( SfxRequest & /* rReq */ )
{
    SdrView* pView = pViewData->GetScDrawView();
    OutlinerView* pOutView = pView->GetTextEditOutlinerView();
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    vcl::Window* pWin = pViewData->GetDialogParent();
    ScopedVclPtr<SfxAbstractPasteDialog> pDlg(pFact->CreatePasteDialog(pWin ? pWin->GetFrameWeld() : nullptr));

    pDlg->Insert( SotClipboardFormatId::STRING, EMPTY_OUSTRING );
    pDlg->Insert( SotClipboardFormatId::RTF,    EMPTY_OUSTRING );
    pDlg->Insert( SotClipboardFormatId::RICHTEXT,  EMPTY_OUSTRING );

    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pViewData->GetActiveWin() ) );

    SotClipboardFormatId nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );

    //! test if outliner view is still valid

    if (nFormat != SotClipboardFormatId::NONE)
    {
        if (nFormat == SotClipboardFormatId::STRING)
            pOutView->Paste();
        else
            pOutView->PasteSpecial();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
