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
#include <uitool.hxx>
#include <comphelper/string.hxx>
#include <svl/urihelper.hxx>
#include <svl/PasswordHelper.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <sot/formats.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/sizeitem.hxx>
#include <svtools/htmlcfg.hxx>
#include <section.hxx>
#include <docary.hxx>
#include <regionsw.hxx>
#include <basesh.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>
#include <column.hxx>
#include <fmtfsize.hxx>
#include <shellio.hxx>
#include <cmdid.h>
#include <globals.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/dlgutil.hxx>
#include <swabstdlg.hxx>
#include <tgrditem.hxx>
#include <memory>

void SwBaseShell::InsertRegionDialog(SfxRequest& rReq)
{
    SwWrtShell& rSh = GetShell();
    const SfxItemSet *pSet = rReq.GetArgs();

    SfxItemSet aSet(
        GetPool(),
        svl::Items<
            RES_FRM_SIZE, RES_FRM_SIZE,
            RES_LR_SPACE, RES_LR_SPACE,
            RES_BACKGROUND, RES_BACKGROUND,
            RES_COL, RES_COL,
            RES_FTN_AT_TXTEND, RES_FRAMEDIR,
            XATTR_FILL_FIRST, XATTR_FILL_LAST,
            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE>{});

    if (!pSet || pSet->Count()==0)
    {
        SwRect aRect;
        rSh.CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);

        long nWidth = aRect.Width();
        aSet.Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth));

        // height=width for more consistent preview (analog to edit region)
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractInsertSectionTabDialog> aTabDlg(pFact->CreateInsertSectionTabDialog(
            &GetView().GetViewFrame()->GetWindow(), aSet , rSh));
        aTabDlg->Execute();
        rReq.Ignore();
    }
    else
    {
        const SfxPoolItem *pItem = nullptr;
        OUString aTmpStr;
        if ( SfxItemState::SET ==
                pSet->GetItemState(FN_PARAM_REGION_NAME, true, &pItem) )
        {
            const OUString sRemoveWhenUniStringIsGone = static_cast<const SfxStringItem *>(pItem)->GetValue();
            aTmpStr = rSh.GetUniqueSectionName(&sRemoveWhenUniStringIsGone);
        }
        else
            aTmpStr = rSh.GetUniqueSectionName();

        SwSectionData aSection(CONTENT_SECTION, aTmpStr);
        rReq.SetReturnValue(SfxStringItem(FN_INSERT_REGION, aTmpStr));

        aSet.Put( *pSet );
        if(SfxItemState::SET == pSet->GetItemState(SID_ATTR_COLUMNS, false, &pItem)||
            SfxItemState::SET == pSet->GetItemState(FN_INSERT_REGION, false, &pItem))
        {
            SwFormatCol aCol;
            SwRect aRect;
            rSh.CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);
            long nWidth = aRect.Width();

            sal_uInt16 nCol = static_cast<const SfxUInt16Item *>(pItem)->GetValue();
            if(nCol)
            {
                aCol.Init( nCol, 0, static_cast< sal_uInt16 >(nWidth) );
                aSet.Put(aCol);
            }
        }
        else if(SfxItemState::SET == pSet->GetItemState(RES_COL, false, &pItem))
        {
            aSet.Put(*pItem);
        }

        const bool bHidden = SfxItemState::SET == pSet->GetItemState(FN_PARAM_REGION_HIDDEN, true, &pItem) &&
                             static_cast<const SfxBoolItem *>(pItem)->GetValue();
        const bool bProtect = SfxItemState::SET == pSet->GetItemState(FN_PARAM_REGION_PROTECT, true, &pItem) &&
                              static_cast<const SfxBoolItem *>(pItem)->GetValue();
        // #114856# edit in readonly sections
        const bool bEditInReadonly = SfxItemState::SET == pSet->GetItemState(FN_PARAM_REGION_EDIT_IN_READONLY, true, &pItem) &&
                                     static_cast<const SfxBoolItem *>(pItem)->GetValue();

        aSection.SetProtectFlag(bProtect);
        aSection.SetHidden(bHidden);
        // #114856# edit in readonly sections
        aSection.SetEditInReadonlyFlag(bEditInReadonly);

        if(SfxItemState::SET ==
                pSet->GetItemState(FN_PARAM_REGION_CONDITION, true, &pItem))
            aSection.SetCondition(static_cast<const SfxStringItem *>(pItem)->GetValue());

        OUString aFile, aSub;
        if(SfxItemState::SET ==
                pSet->GetItemState(FN_PARAM_1, true, &pItem))
            aFile = static_cast<const SfxStringItem *>(pItem)->GetValue();

        if(SfxItemState::SET ==
                pSet->GetItemState(FN_PARAM_3, true, &pItem))
            aSub = static_cast<const SfxStringItem *>(pItem)->GetValue();

        if(!aFile.isEmpty() || !aSub.isEmpty())
        {
            OUString sLinkFileName = OUStringLiteral1(sfx2::cTokenSeparator)
                + OUStringLiteral1(sfx2::cTokenSeparator);
            sLinkFileName = comphelper::string::setToken(sLinkFileName, 0, sfx2::cTokenSeparator, aFile);

            if(SfxItemState::SET ==
                    pSet->GetItemState(FN_PARAM_2, true, &pItem))
            {
                sLinkFileName = comphelper::string::setToken(sLinkFileName, 1, sfx2::cTokenSeparator,
                    static_cast<const SfxStringItem *>(pItem)->GetValue());
            }

            sLinkFileName += aSub;
            aSection.SetType( FILE_LINK_SECTION );
            aSection.SetLinkFileName(sLinkFileName);
        }
        rSh.InsertSection(aSection, aSet.Count() ? &aSet : nullptr);
        rReq.Done();
    }
}

IMPL_LINK( SwWrtShell, InsertRegionDialog, void*, p, void )
{
    SwSectionData* pSect = static_cast<SwSectionData*>(p);
    std::unique_ptr<SwSectionData> xSectionData(pSect);
    if (!xSectionData.get())
        return;

    SfxItemSet aSet(
        GetView().GetPool(),
        svl::Items<
            RES_FRM_SIZE, RES_FRM_SIZE,
            RES_BACKGROUND, RES_BACKGROUND,
            RES_COL, RES_COL,
            XATTR_FILL_FIRST, XATTR_FILL_LAST,
            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE>{});
    SwRect aRect;
    CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);
    long nWidth = aRect.Width();
    aSet.Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth));
    // height=width for more consistent preview (analog to edit region)
    aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractInsertSectionTabDialog> aTabDlg(pFact->CreateInsertSectionTabDialog(
        &GetView().GetViewFrame()->GetWindow(),aSet , *this));
    aTabDlg->SetSectionData(*xSectionData);
    aTabDlg->Execute();

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
            vcl::Window* pParentWin = &GetView().GetViewFrame()->GetWindow();
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractEditRegionDlg> pEditRegionDlg(pFact->CreateEditRegionDlg(pParentWin, rWrtShell));
                if(pItem && dynamic_cast< const SfxStringItem *>( pItem ) !=  nullptr)
                {
                    pEditRegionDlg->SelectSection(static_cast<const SfxStringItem*>(pItem)->GetValue());
                }
                pEditRegionDlg->Execute();
            }
        }
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
