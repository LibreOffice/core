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
#include <swundo.hxx>                   // fuer Undo-Ids
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

/*--------------------------------------------------------------------
    Beschreibung:   Bereiche einfuegen
 --------------------------------------------------------------------*/


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

        // Hoehe=Breite fuer konsistentere Vorschau (analog zu Bereich bearbeiten)
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractInsertSectionTabDialog* aTabDlg = pFact->CreateInsertSectionTabDialog( DLG_INSERT_SECTION,
                                                        &GetView().GetViewFrame()->GetWindow(), aSet , rSh);
        DBG_ASSERT(aTabDlg, "Dialogdiet fail!");
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
        // --> FME 2004-06-22 #114856# edit in readonly sections
        const sal_Bool bEditInReadonly = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_EDIT_IN_READONLY, sal_True, &pItem)?
            (sal_Bool)((const SfxBoolItem *)pItem)->GetValue():sal_False;
        // <--

        aSection.SetProtectFlag(bProtect);
        aSection.SetHidden(bHidden);
        // --> FME 2004-06-22 #114856# edit in readonly sections
        aSection.SetEditInReadonlyFlag(bEditInReadonly);
        // <--

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
        // Hoehe=Breite fuer konsistentere Vorschau (analog zu Bereich bearbeiten)
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractInsertSectionTabDialog* aTabDlg = pFact->CreateInsertSectionTabDialog( DLG_INSERT_SECTION,
                                                        &pThis->GetView().GetViewFrame()->GetWindow(),aSet , *pThis);
        DBG_ASSERT(aTabDlg, "Dialogdiet fail!");
        aTabDlg->SetSectionData(*pSectionData);
        aTabDlg->Execute();

        delete aTabDlg;
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Bereich bearbeiten
 --------------------------------------------------------------------*/

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
                DBG_ASSERT(pFact, "Dialogdiet fail!");
                AbstractEditRegionDlg* pEditRegionDlg = pFact->CreateEditRegionDlg( MD_EDIT_REGION,
                                                        pParentWin, rWrtShell);
                DBG_ASSERT(pEditRegionDlg, "Dialogdiet fail!");
                if(pItem && dynamic_cast< const SfxStringItem* >(pItem))
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
