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

#include <memory>
#include <OutlineBulletDlg.hxx>

#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>

#include <editeng/numitem.hxx>

#include <tools/debug.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdmark.hxx>
#include <View.hxx>
#include <svx/svdobj.hxx>
#include <svl/style.hxx>
#include <svl/intitem.hxx>
#include <drawdoc.hxx>

#include <strings.hxx>
#include <bulmaper.hxx>
#include <DrawDocShell.hxx>

namespace sd {

/**
 * Constructor of tab dialog: append pages to the dialog
 */
OutlineBulletDlg::OutlineBulletDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView)
    : SfxTabDialogController(pParent, u"modules/sdraw/ui/bulletsandnumbering.ui"_ustr, u"BulletsAndNumberingDialog"_ustr)
    , m_aInputSet(*pAttr)
    , m_bTitle(false)
    , m_pSdView(pView)
{
    m_aInputSet.MergeRange(SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL);
    m_aInputSet.Put(*pAttr);

    m_xOutputSet.reset( new SfxItemSet( *pAttr ) );
    m_xOutputSet->ClearItem();

    bool bOutliner = false;

    // special treatment if a title object is selected
    if (pView)
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        const size_t nCount = rMarkList.GetMarkCount();
        for(size_t nNum = 0; nNum < nCount; ++nNum)
        {
            SdrObject* pObj = rMarkList.GetMark(nNum)->GetMarkedSdrObj();
            if( pObj->GetObjInventor() == SdrInventor::Default )
            {
                switch(pObj->GetObjIdentifier())
                {
                case SdrObjKind::TitleText:
                    m_bTitle = true;
                    break;
                case SdrObjKind::OutlineText:
                    bOutliner = true;
                    break;
                default:
                    break;
                }
            }
        }
    }

    if( SfxItemState::SET != m_aInputSet.GetItemState(EE_PARA_NUMBULLET))
    {
        const SvxNumBulletItem *pItem = nullptr;
        if(bOutliner)
        {
            SfxStyleSheetBasePool* pSSPool = pView->GetDocSh()->GetStyleSheetPool();
            SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( STR_LAYOUT_OUTLINE + " 1", SfxStyleFamily::Pseudo);
            if( pFirstStyleSheet )
                pItem = pFirstStyleSheet->GetItemSet().GetItemIfSet(EE_PARA_NUMBULLET, false);
        }

        if( pItem == nullptr )
            pItem = m_aInputSet.GetPool()->GetSecondaryPool()->GetUserDefaultItem(EE_PARA_NUMBULLET);

        assert(pItem && "No EE_PARA_NUMBULLET in Pool! [CL]");

        m_aInputSet.Put(pItem->CloneSetWhich(EE_PARA_NUMBULLET));
    }

    if (m_bTitle && m_aInputSet.GetItemState(EE_PARA_NUMBULLET) == SfxItemState::SET )
    {
        const SvxNumBulletItem* pItem = m_aInputSet.GetItem<SvxNumBulletItem>(EE_PARA_NUMBULLET);
        const SvxNumRule& rRule = pItem->GetNumRule();
        SvxNumRule aNewRule( rRule );
        aNewRule.SetFeatureFlag( SvxNumRuleFlags::NO_NUMBERS );

        SvxNumBulletItem aNewItem( std::move(aNewRule), EE_PARA_NUMBULLET );
        m_aInputSet.Put(aNewItem);
    }

    SetInputSet(&m_aInputSet);

    if (m_bTitle)
        RemoveTabPage(u"singlenum"_ustr);

    AddTabPage(u"customize"_ustr, RID_SVXPAGE_NUM_OPTIONS);
    AddTabPage(u"position"_ustr, RID_SVXPAGE_NUM_POSITION);
}

OutlineBulletDlg::~OutlineBulletDlg()
{
}

void OutlineBulletDlg::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    if (!m_pSdView)
        return;
    if (rId == "customize")
    {
        FieldUnit eMetric = m_pSdView->GetDoc().GetUIUnit();
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put ( SfxUInt16Item(SID_METRIC_ITEM,static_cast<sal_uInt16>(eMetric)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "position")
    {
        FieldUnit eMetric = m_pSdView->GetDoc().GetUIUnit();
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put ( SfxUInt16Item(SID_METRIC_ITEM,static_cast<sal_uInt16>(eMetric)));
        rPage.PageCreated(aSet);
    }
}

const SfxItemSet* OutlineBulletDlg::GetBulletOutputItemSet() const
{
    SfxItemSet aSet(*GetOutputItemSet());
    m_xOutputSet->Put(aSet);

    const SfxPoolItem *pItem = nullptr;
    if( SfxItemState::SET == m_xOutputSet->GetItemState(m_xOutputSet->GetPool()->GetWhichIDFromSlotID(SID_ATTR_NUMBERING_RULE), false, &pItem ))
    {
        SdBulletMapper::MapFontsInNumRule(const_cast<SvxNumRule&>(static_cast<const SvxNumBulletItem*>(pItem)->GetNumRule()), *m_xOutputSet);
        // #i35937 - removed EE_PARA_BULLETSTATE setting
    }

    if (m_bTitle && m_xOutputSet->GetItemState(EE_PARA_NUMBULLET) == SfxItemState::SET)
    {
        const SvxNumBulletItem* pBulletItem = m_xOutputSet->GetItem<SvxNumBulletItem>(EE_PARA_NUMBULLET);
        SvxNumRule& rRule = const_cast<SvxNumRule&>(pBulletItem->GetNumRule());
        rRule.SetFeatureFlag( SvxNumRuleFlags::NO_NUMBERS, false );
    }

    return m_xOutputSet.get();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
