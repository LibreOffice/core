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

#include "OutlineBulletDlg.hxx"

#include <svx/svxids.hrc>
#include <sfx2/objsh.hxx>
#include <svx/drawitem.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/numitem.hxx>

#include <svx/dialogs.hrc>
#include <svl/intitem.hxx>
#include <svx/svdmark.hxx>
#include "View.hxx"
#include <svx/svdobj.hxx>
#include <svl/style.hxx>
#include <drawdoc.hxx>

#include "sdresid.hxx"

#include "glob.hrc"
#include "bulmaper.hxx"
#include "DrawDocShell.hxx"
#include <svl/aeitem.hxx>

namespace sd {

/**
 * Constructor of tab dialog: append pages to the dialog
 */
OutlineBulletDlg::OutlineBulletDlg(
    vcl::Window* pParent,
    const SfxItemSet* pAttr,
    ::sd::View* pView )
    : SfxTabDialog( pParent, "BulletsAndNumberingDialog",
        "modules/sdraw/ui/bulletsandnumbering.ui")
    , aInputSet( *pAttr )
    , bTitle(false)
    , pSdView(pView)
{
    aInputSet.MergeRange( SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL );
    aInputSet.Put( *pAttr );

    pOutputSet = new SfxItemSet( *pAttr );
    pOutputSet->ClearItem();

    bool bOutliner = false;

    // special treatment if a title object is selected
    if( pView )
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        const size_t nCount = rMarkList.GetMarkCount();
        for(size_t nNum = 0; nNum < nCount; ++nNum)
        {
            SdrObject* pObj = rMarkList.GetMark(nNum)->GetMarkedSdrObj();
            if( pObj->GetObjInventor() == SdrInventor )
            {

                switch(pObj->GetObjIdentifier())
                {
                case OBJ_TITLETEXT:
                    bTitle = true;
                    break;
                case OBJ_OUTLINETEXT:
                    bOutliner = true;
                    break;
                }
            }
        }
    }

    if( SfxItemState::SET != aInputSet.GetItemState(EE_PARA_NUMBULLET))
    {
        const SvxNumBulletItem *pItem = nullptr;
        if(bOutliner)
        {
            SfxStyleSheetBasePool* pSSPool = pView->GetDocSh()->GetStyleSheetPool();
            OUString aStyleName(SD_RESSTR(STR_LAYOUT_OUTLINE) + " 1");
            SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);
            if( pFirstStyleSheet )
                pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, false, reinterpret_cast<const SfxPoolItem**>(&pItem));
        }

        if( pItem == nullptr )
            pItem = static_cast<const SvxNumBulletItem*>( aInputSet.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET) );

        DBG_ASSERT( pItem, "No EE_PARA_NUMBULLET in Pool! [CL]" );

        aInputSet.Put(*pItem, EE_PARA_NUMBULLET);
    }

    if(bTitle && aInputSet.GetItemState(EE_PARA_NUMBULLET) == SfxItemState::SET )
    {
        const SvxNumBulletItem* pItem = static_cast<const SvxNumBulletItem*>( aInputSet.GetItem(EE_PARA_NUMBULLET) );
        SvxNumRule* pRule = pItem->GetNumRule();
        if(pRule)
        {
            SvxNumRule aNewRule( *pRule );
            aNewRule.SetFeatureFlag( SvxNumRuleFlags::NO_NUMBERS );

            SvxNumBulletItem aNewItem( aNewRule, EE_PARA_NUMBULLET );
            aInputSet.Put(aNewItem);
        }
    }

    SetInputSet( &aInputSet );

    if(!bTitle)
        AddTabPage("singlenum", RID_SVXPAGE_PICK_SINGLE_NUM);
    else
        RemoveTabPage("singlenum");

    AddTabPage("bullets", RID_SVXPAGE_PICK_BULLET);
    AddTabPage("graphics", RID_SVXPAGE_PICK_BMP);
    m_nOptionsId = AddTabPage("customize", RID_SVXPAGE_NUM_OPTIONS);
    m_nPositionId = AddTabPage("position", RID_SVXPAGE_NUM_POSITION);
}

OutlineBulletDlg::~OutlineBulletDlg()
{
    disposeOnce();
}

void OutlineBulletDlg::dispose()
{
    delete pOutputSet;
    SfxTabDialog::dispose();
}

void OutlineBulletDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nOptionsId)
    {
        if( pSdView )
        {
            FieldUnit eMetric = pSdView->GetDoc().GetUIUnit();
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM,(sal_uInt16)eMetric));
            rPage.PageCreated(aSet);
        }
    }
    else if (nId == m_nPositionId)
    {
        if( pSdView )
        {
            FieldUnit eMetric = pSdView->GetDoc().GetUIUnit();
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM,(sal_uInt16)eMetric));
            rPage.PageCreated(aSet);
        }
    }
}

const SfxItemSet* OutlineBulletDlg::GetOutputItemSet() const
{
    SfxItemSet aSet( *SfxTabDialog::GetOutputItemSet() );
    pOutputSet->Put( aSet );

    const SfxPoolItem *pItem = nullptr;
    if( SfxItemState::SET == pOutputSet->GetItemState(pOutputSet->GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE), false, &pItem ))
    {
        SdBulletMapper::MapFontsInNumRule( *static_cast<const SvxNumBulletItem*>(pItem)->GetNumRule(), *pOutputSet );

// #i35937 - removed EE_PARA_BULLETSTATE setting
    }

    if(bTitle && pOutputSet->GetItemState(EE_PARA_NUMBULLET) == SfxItemState::SET )
    {
        const SvxNumBulletItem* pBulletItem = static_cast<const SvxNumBulletItem*>(pOutputSet->GetItem(EE_PARA_NUMBULLET));
        SvxNumRule* pRule = pBulletItem->GetNumRule();
        if(pRule)
            pRule->SetFeatureFlag( SvxNumRuleFlags::NO_NUMBERS, false );
    }

    return pOutputSet;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
