/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <uitool.hxx>
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
#include <swundo.hxx>                   // for Undo-Ids
#include <column.hxx>
#include <fmtfsize.hxx>
#include <swunodef.hxx>
#include <shellio.hxx>
#include <helpid.h>
#include <cmdid.h>
#include <regionsw.hrc>
#include <comcore.hrc>
#include <globals.hrc>
#include <sfx2/bindings.hxx>
#include <svx/htmlmode.hxx>
#include <svx/dlgutil.hxx>
#include "swabstdlg.hxx"

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
        AbstractInsertSectionTabDialog* aTabDlg = pFact->CreateInsertSectionTabDialog( DLG_INSERT_SECTION,
                                                        &GetView().GetViewFrame()->GetWindow(), aSet , rSh);
        OSL_ENSURE(aTabDlg, "Dialogdiet fail!");
        aTabDlg->Execute();
        rReq.Ignore();
        delete aTabDlg;
    }
    else
    {
        const SfxPoolItem *pItem = 0;
        String aTmpStr;
        if ( SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_REGION_NAME, sal_True, &pItem) )
            aTmpStr = rSh.GetUniqueSectionName(
                    &((const SfxStringItem *)pItem)->GetValue() );
        else
            aTmpStr = rSh.GetUniqueSectionName();

        SwSectionData aSection(CONTENT_SECTION, aTmpStr);
        rReq.SetReturnValue(SfxStringItem(FN_INSERT_REGION, aTmpStr));

        aSet.Put( *pSet );
        if(SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_COLUMNS, sal_False, &pItem)||
            SFX_ITEM_SET == pSet->GetItemState(FN_INSERT_REGION, sal_False, &pItem))
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
        else if(SFX_ITEM_SET == pSet->GetItemState(RES_COL, sal_False, &pItem))
        {
            aSet.Put(*pItem);
        }

        const sal_Bool bHidden = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_HIDDEN, sal_True, &pItem)?
            (sal_Bool)((const SfxBoolItem *)pItem)->GetValue():sal_False;
        const sal_Bool bProtect = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_PROTECT, sal_True, &pItem)?
            (sal_Bool)((const SfxBoolItem *)pItem)->GetValue():sal_False;
        // #114856# edit in readonly sections
        const sal_Bool bEditInReadonly = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_EDIT_IN_READONLY, sal_True, &pItem)?
            (sal_Bool)((const SfxBoolItem *)pItem)->GetValue():sal_False;

        aSection.SetProtectFlag(bProtect);
        aSection.SetHidden(bHidden);
        // #114856# edit in readonly sections
        aSection.SetEditInReadonlyFlag(bEditInReadonly);

        if(SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_REGION_CONDITION, sal_True, &pItem))
            aSection.SetCondition(((const SfxStringItem *)pItem)->GetValue());

        String aFile, aSub;
        if(SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_1, sal_True, &pItem))
            aFile = ((const SfxStringItem *)pItem)->GetValue();

        if(SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_3, sal_True, &pItem))
            aSub = ((const SfxStringItem *)pItem)->GetValue();


        if(aFile.Len() || aSub.Len())
        {
            String sLinkFileName(sfx2::cTokenSeperator);
            sLinkFileName += sfx2::cTokenSeperator;
            sLinkFileName.SetToken(0, sfx2::cTokenSeperator,aFile);

            if(SFX_ITEM_SET ==
                    pSet->GetItemState(FN_PARAM_2, sal_True, &pItem))
                sLinkFileName.SetToken(1, sfx2::cTokenSeperator,
                    ((const SfxStringItem *)pItem)->GetValue());

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
    ::std::auto_ptr<SwSectionData> pSectionData(pSect);
    if (pSectionData.get())
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
        AbstractInsertSectionTabDialog* aTabDlg = pFact->CreateInsertSectionTabDialog( DLG_INSERT_SECTION,
                                                        &pThis->GetView().GetViewFrame()->GetWindow(),aSet , *pThis);
        OSL_ENSURE(aTabDlg, "Dialogdiet fail!");
        aTabDlg->SetSectionData(*pSectionData);
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
        pArgs->GetItemState(nSlot, sal_False, &pItem);
    SwWrtShell& rWrtShell = GetShell();

    switch ( nSlot )
    {
        case FN_EDIT_REGION:
        {
            Window* pParentWin = &GetView().GetViewFrame()->GetWindow();
            sal_Bool bStart = sal_True;
            if(bStart)
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialogdiet fail!");
                AbstractEditRegionDlg* pEditRegionDlg = pFact->CreateEditRegionDlg( MD_EDIT_REGION,
                                                        pParentWin, rWrtShell);
                OSL_ENSURE(pEditRegionDlg, "Dialogdiet fail!");
                if(pItem && pItem->ISA(SfxStringItem))
                {
                    pEditRegionDlg->SelectSection(((const SfxStringItem*)pItem)->GetValue());
                }
                pEditRegionDlg->Execute();
                delete pEditRegionDlg;
            }
            else
                InfoBox(pParentWin, SW_RES(REG_WRONG_PASSWORD)).Execute();
        }
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
