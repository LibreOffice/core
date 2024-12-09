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

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/sizeitem.hxx>
#include <section.hxx>
#include <basesh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <fmtclds.hxx>
#include <fmtfsize.hxx>
#include <cmdid.h>
#include <swabstdlg.hxx>
#include <IDocumentContentOperations.hxx>
#include <translatehelper.hxx>
#include <IDocumentUndoRedo.hxx>

void SwBaseShell::InsertRegionDialog(SfxRequest& rReq)
{
    SwWrtShell& rSh = GetShell();
    const SfxItemSet *pSet = rReq.GetArgs();

    SfxItemSetFixed<
            RES_FRM_SIZE, RES_FRM_SIZE,
            RES_LR_SPACE, RES_LR_SPACE,
            RES_BACKGROUND, RES_BACKGROUND,
            RES_COL, RES_COL,
            RES_FTN_AT_TXTEND, RES_FRAMEDIR,
            XATTR_FILL_FIRST, XATTR_FILL_LAST,
            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE>
        aSet( GetPool() );

    if (!pSet || pSet->Count()==0)
    {
        SwRect aRect;
        rSh.CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);

        tools::Long nWidth = aRect.Width();
        aSet.Put(SwFormatFrameSize(SwFrameSize::Variable, nWidth));

        // height=width for more consistent preview (analog to edit region)
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        VclPtr<AbstractInsertSectionTabDialog> aTabDlg(pFact->CreateInsertSectionTabDialog(
            GetView().GetFrameWeld(), aSet , rSh));
        aTabDlg->StartExecuteAsync([aTabDlg](sal_Int32 /*nResult*/){
            aTabDlg->disposeOnce();
        });
        rReq.Ignore();
    }
    else
    {
        OUString aTmpStr;
        if ( const SfxStringItem* pItem = pSet->GetItemIfSet(FN_PARAM_REGION_NAME) )
        {
            const OUString sRemoveWhenUniStringIsGone = pItem->GetValue();
            aTmpStr = rSh.GetUniqueSectionName(&sRemoveWhenUniStringIsGone);
        }
        else
            aTmpStr = rSh.GetUniqueSectionName();

        SwSectionData aSection(SectionType::Content, UIName(aTmpStr));
        rReq.SetReturnValue(SfxStringItem(FN_INSERT_REGION, aTmpStr));

        aSet.Put( *pSet );
        const SfxUInt16Item *pColRegionItem = nullptr;
        if((pColRegionItem = pSet->GetItemIfSet(SID_ATTR_COLUMNS, false)) ||
            (pColRegionItem = pSet->GetItemIfSet(FN_INSERT_REGION, false)))
        {
            SwFormatCol aCol;
            SwRect aRect;
            rSh.CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);
            tools::Long nWidth = aRect.Width();

            sal_uInt16 nCol = pColRegionItem->GetValue();
            if(nCol)
            {
                aCol.Init( nCol, 0, static_cast< sal_uInt16 >(nWidth) );
                aSet.Put(aCol);
            }
        }
        else if(const SwFormatCol* pFormatCol = pSet->GetItemIfSet(RES_COL, false))
        {
            aSet.Put(*pFormatCol);
        }

        const SfxBoolItem* pBoolItem;
        const bool bHidden = (pBoolItem = pSet->GetItemIfSet(FN_PARAM_REGION_HIDDEN)) &&
                             pBoolItem->GetValue();
        const bool bProtect = (pBoolItem = pSet->GetItemIfSet(FN_PARAM_REGION_PROTECT)) &&
                              pBoolItem->GetValue();
        // #114856# edit in readonly sections
        const bool bEditInReadonly = (pBoolItem = pSet->GetItemIfSet(FN_PARAM_REGION_EDIT_IN_READONLY)) &&
                                     pBoolItem->GetValue();

        aSection.SetProtectFlag(bProtect);
        aSection.SetHidden(bHidden);
        // #114856# edit in readonly sections
        aSection.SetEditInReadonlyFlag(bEditInReadonly);

        if(const SfxStringItem* pConditionItem =
                pSet->GetItemIfSet(FN_PARAM_REGION_CONDITION))
            aSection.SetCondition(pConditionItem->GetValue());

        OUString aFile, aSub;
        const SfxPoolItem* pItem;
        if(SfxItemState::SET ==
                pSet->GetItemState(FN_PARAM_1, true, &pItem))
            aFile = static_cast<const SfxStringItem *>(pItem)->GetValue();

        if(SfxItemState::SET ==
                pSet->GetItemState(FN_PARAM_3, true, &pItem))
            aSub = static_cast<const SfxStringItem *>(pItem)->GetValue();

        if(!aFile.isEmpty() || !aSub.isEmpty())
        {
            OUString sLinkFileName = OUStringChar(sfx2::cTokenSeparator)
                + OUStringChar(sfx2::cTokenSeparator);
            sLinkFileName = comphelper::string::setToken(sLinkFileName, 0, sfx2::cTokenSeparator, aFile);

            if(SfxItemState::SET ==
                    pSet->GetItemState(FN_PARAM_2, true, &pItem))
            {
                sLinkFileName = comphelper::string::setToken(sLinkFileName, 1, sfx2::cTokenSeparator,
                    static_cast<const SfxStringItem *>(pItem)->GetValue());
            }

            sLinkFileName += aSub;
            aSection.SetType( SectionType::FileLink );
            aSection.SetLinkFileName(sLinkFileName);
        }
        rSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSSECTION, nullptr);
        rSh.StartAction();
        rSh.InsertSection(aSection, aSet.Count() ? &aSet : nullptr);

        const SfxStringItem* pSectionContent = rReq.GetArg<SfxStringItem>(FN_PARAM_4);
        if (pSectionContent)
        {
            OUString aSectionContent = pSectionContent->GetValue();
            SwPaM* pCursorPos = rSh.GetCursor();
            pCursorPos->Move(fnMoveBackward, GoInContent);
            // Paste HTML content.
            SwTranslateHelper::PasteHTMLToPaM(rSh, pCursorPos, aSectionContent.toUtf8());
            if (pCursorPos->GetPoint()->GetContentIndex() == 0)
            {
                // The paste created a last empty text node, remove it.
                SwPaM aPam(*pCursorPos->GetPoint());
                aPam.SetMark();
                aPam.Move(fnMoveBackward, GoInContent);
                rSh.GetDoc()->getIDocumentContentOperations().DeleteAndJoin(aPam);
            }
        }
        rSh.EndAction();
        rSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSSECTION, nullptr);

        rReq.Done();
    }
}

void SwWrtShell::StartInsertRegionDialog(const SwSectionData& rSectionData)
{
    SfxItemSetFixed<
            RES_FRM_SIZE, RES_FRM_SIZE,
            RES_BACKGROUND, RES_BACKGROUND,
            RES_COL, RES_COL,
            XATTR_FILL_FIRST, XATTR_FILL_LAST,
            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE>
        aSet( GetView().GetPool() );
    SwRect aRect;
    CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);
    tools::Long nWidth = aRect.Width();
    aSet.Put(SwFormatFrameSize(SwFrameSize::Variable, nWidth));
    // height=width for more consistent preview (analog to edit region)
    aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    VclPtr<AbstractInsertSectionTabDialog> aTabDlg(pFact->CreateInsertSectionTabDialog(
        GetView().GetFrameWeld(), aSet, *this));
    aTabDlg->SetSectionData(rSectionData);
    aTabDlg->StartExecuteAsync([aTabDlg](sal_Int32 /*nResult*/){
        aTabDlg->disposeOnce();
    });
}

void SwBaseShell::EditRegionDialog(SfxRequest const & rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxPoolItem* pItem = nullptr;
    if(pArgs)
        pArgs->GetItemState(nSlot, false, &pItem);
    SwWrtShell& rWrtShell = GetShell();

    switch ( nSlot )
    {
        case FN_EDIT_REGION:
        case FN_EDIT_CURRENT_REGION:
        {
            weld::Window* pParentWin = GetView().GetFrameWeld();

            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            VclPtr<AbstractEditRegionDlg> pEditRegionDlg(pFact->CreateEditRegionDlg(pParentWin, rWrtShell));

            if(auto pStringItem = dynamic_cast< const SfxStringItem *>( pItem ))
            {
                pEditRegionDlg->SelectSection(pStringItem->GetValue());
            }
            pEditRegionDlg->StartExecuteAsync([pEditRegionDlg](sal_Int32 /*nResult */){
                pEditRegionDlg->disposeOnce();
            });
        }
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
