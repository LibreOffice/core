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
#include <vcl/msgbox.hxx>
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
#include <helpid.h>
#include <cmdid.h>
#include <regionsw.hrc>
#include <comcore.hrc>
#include <globals.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/dlgutil.hxx>
#include "swabstdlg.hxx"
#include <boost/scoped_ptr.hpp>

void SwBaseShell::InsertRegionDialog(SfxRequest& rReq)
{
    SwWrtShell& rSh = GetShell();
    const SfxItemSet *pSet = rReq.GetArgs();

    SfxItemSet aSet(GetPool(),
            RES_COL, RES_COL,
            RES_LR_SPACE, RES_LR_SPACE,
            RES_COLUMNBALANCE, RES_FRAMEDIR,
            RES_BACKGROUND, RES_BACKGROUND,
            RES_FRM_SIZE, RES_FRM_SIZE,
            RES_FTN_AT_TXTEND, RES_END_AT_TXTEND,
            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
            0);

    if (!pSet || pSet->Count()==0)
    {
        SwRect aRect;
        rSh.CalcBoundRect(aRect, FLY_AS_CHAR);

        long nWidth = aRect.Width();
        aSet.Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth));

        // height=width for more consistent preview (analog to edit region)
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "Dialogdiet fail!");
        AbstractInsertSectionTabDialog* aTabDlg = pFact->CreateInsertSectionTabDialog(
            &GetView().GetViewFrame()->GetWindow(), aSet , rSh);
        OSL_ENSURE(aTabDlg, "Dialogdiet fail!");
        aTabDlg->Execute();
        rReq.Ignore();
        delete aTabDlg;
    }
    else
    {
        const SfxPoolItem *pItem = 0;
        OUString aTmpStr;
        if ( SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_REGION_NAME, true, &pItem) )
        {
            const OUString sRemoveWhenUniStringIsGone = ((const SfxStringItem *)pItem)->GetValue();
            aTmpStr = rSh.GetUniqueSectionName(&sRemoveWhenUniStringIsGone);
        }
        else
            aTmpStr = rSh.GetUniqueSectionName();

        SwSectionData aSection(CONTENT_SECTION, aTmpStr);
        rReq.SetReturnValue(SfxStringItem(FN_INSERT_REGION, aTmpStr));

        aSet.Put( *pSet );
        if(SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_COLUMNS, false, &pItem)||
            SFX_ITEM_SET == pSet->GetItemState(FN_INSERT_REGION, false, &pItem))
        {
            SwFmtCol aCol;
            SwRect aRect;
            rSh.CalcBoundRect(aRect, FLY_AS_CHAR);
            long nWidth = aRect.Width();

            sal_uInt16 nCol = ((SfxUInt16Item *)pItem)->GetValue();
            if(nCol)
            {
                aCol.Init( nCol, 0, static_cast< sal_uInt16 >(nWidth) );
                aSet.Put(aCol);
            }
        }
        else if(SFX_ITEM_SET == pSet->GetItemState(RES_COL, false, &pItem))
        {
            aSet.Put(*pItem);
        }

        const sal_Bool bHidden = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_HIDDEN, true, &pItem)?
            (sal_Bool)((const SfxBoolItem *)pItem)->GetValue():sal_False;
        const sal_Bool bProtect = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_PROTECT, true, &pItem)?
            (sal_Bool)((const SfxBoolItem *)pItem)->GetValue():sal_False;
        // #114856# edit in readonly sections
        const sal_Bool bEditInReadonly = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_EDIT_IN_READONLY, true, &pItem)?
            (sal_Bool)((const SfxBoolItem *)pItem)->GetValue():sal_False;

        aSection.SetProtectFlag(bProtect);
        aSection.SetHidden(bHidden);
        // #114856# edit in readonly sections
        aSection.SetEditInReadonlyFlag(bEditInReadonly);

        if(SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_REGION_CONDITION, true, &pItem))
            aSection.SetCondition(((const SfxStringItem *)pItem)->GetValue());

        OUString aFile, aSub;
        if(SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_1, true, &pItem))
            aFile = ((const SfxStringItem *)pItem)->GetValue();

        if(SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_3, true, &pItem))
            aSub = ((const SfxStringItem *)pItem)->GetValue();


        if(!aFile.isEmpty() || !aSub.isEmpty())
        {
            OUString sLinkFileName = OUString(sfx2::cTokenSeparator);
            sLinkFileName += OUString(sfx2::cTokenSeparator);
            sLinkFileName = comphelper::string::setToken(sLinkFileName, 0, sfx2::cTokenSeparator, aFile);

            if(SFX_ITEM_SET ==
                    pSet->GetItemState(FN_PARAM_2, true, &pItem))
            {
                sLinkFileName = comphelper::string::setToken(sLinkFileName, 1, sfx2::cTokenSeparator,
                    ((const SfxStringItem *)pItem)->GetValue());
            }

            sLinkFileName += aSub;
            aSection.SetType( FILE_LINK_SECTION );
            aSection.SetLinkFileName(sLinkFileName);
        }
        rSh.InsertSection(aSection, aSet.Count() ? &aSet : 0);
        rReq.Done();
    }
}

IMPL_STATIC_LINK( SwWrtShell, InsertRegionDialog, SwSectionData*, pSect )
{
    boost::scoped_ptr<SwSectionData> xSectionData(pSect);
    if (xSectionData.get())
    {
        SfxItemSet aSet(pThis->GetView().GetPool(),
                RES_COL, RES_COL,
                RES_BACKGROUND, RES_BACKGROUND,
                RES_FRM_SIZE, RES_FRM_SIZE,
                SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                0);
        SwRect aRect;
        pThis->CalcBoundRect(aRect, FLY_AS_CHAR);
        long nWidth = aRect.Width();
        aSet.Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth));
        // height=width for more consistent preview (analog to edit region)
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "Dialogdiet fail!");
        AbstractInsertSectionTabDialog* aTabDlg = pFact->CreateInsertSectionTabDialog(
            &pThis->GetView().GetViewFrame()->GetWindow(),aSet , *pThis);
        OSL_ENSURE(aTabDlg, "Dialogdiet fail!");
        aTabDlg->SetSectionData(*xSectionData);
        aTabDlg->Execute();

        delete aTabDlg;
    }
    return 0;
}

void SwBaseShell::EditRegionDialog(SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxPoolItem* pItem = 0;
    if(pArgs)
        pArgs->GetItemState(nSlot, false, &pItem);
    SwWrtShell& rWrtShell = GetShell();

    switch ( nSlot )
    {
        case FN_EDIT_REGION:
        {
            Window* pParentWin = &GetView().GetViewFrame()->GetWindow();
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialogdiet fail!");
                AbstractEditRegionDlg* pEditRegionDlg = pFact->CreateEditRegionDlg(pParentWin, rWrtShell);
                OSL_ENSURE(pEditRegionDlg, "Dialogdiet fail!");
                if(pItem && pItem->ISA(SfxStringItem))
                {
                    pEditRegionDlg->SelectSection(((const SfxStringItem*)pItem)->GetValue());
                }
                pEditRegionDlg->Execute();
                delete pEditRegionDlg;
            }
        }
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
