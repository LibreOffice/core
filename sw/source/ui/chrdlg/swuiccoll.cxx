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

#include "cmdid.h"
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"
#include "helpid.h"

#include <sfx2/styfitem.hxx>

#include "uitool.hxx"
#include "ccoll.hxx"
#include "fmtcol.hxx"
#include "hintids.hxx"
#include "docsh.hxx"
#include "docstyle.hxx"
#include "hints.hxx"

#include "chrdlg.hrc"
#include <vcl/svapp.hxx>

#include "swuiccoll.hxx"

static const sal_uInt16 aPageRg[] = {
    FN_COND_COLL, FN_COND_COLL,
    0
};

// Warning! This table is indicated directly in code (0, 1, ...)
static long nTabs[] =
    {   2, // Number of Tabs
        0, 100
    };

SwCondCollPage::SwCondCollPage(vcl::Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "ConditionPage",
        "modules/swriter/ui/conditionpage.ui", &rSet)
    ,
    rSh(::GetActiveView()->GetWrtShell()),
    pCmds( SwCondCollItem::GetCmds() ),
    pFmt(0),

    bNewTemplate(false)
{
    get(m_pConditionCB, "condstyle");
    get(m_pContextFT, "contextft");
    get(m_pUsedFT, "usedft");
    get(m_pStyleFT, "styleft");
    get(m_pTbLinks, "links");
    get(m_pStyleLB, "styles");
    m_pStyleLB->SetStyle(m_pStyleLB->GetStyle() | WB_SORT);
    m_pStyleLB->SetDropDownLineCount(12);
    m_pTbLinks->set_height_request(m_pStyleLB->GetOptimalSize().Height());
    get(m_pFilterLB, "filter");
    get(m_pRemovePB, "remove");
    get(m_pAssignPB, "apply");

    const sal_Int32 nStrCount = m_pFilterLB->GetEntryCount();
    for (sal_Int32 i = 0; i < nStrCount; ++i)
        m_aStrArr.push_back(m_pFilterLB->GetEntry(i));
    m_pFilterLB->Clear();

    SetExchangeSupport();

    // Install handlers
    m_pConditionCB->SetClickHdl(   LINK(this, SwCondCollPage, OnOffHdl));
    m_pTbLinks->SetDoubleClickHdl( LINK(this, SwCondCollPage, AssignRemoveHdl ));
    m_pStyleLB->SetDoubleClickHdl( LINK(this, SwCondCollPage, AssignRemoveHdl ));
    m_pRemovePB->SetClickHdl(      LINK(this, SwCondCollPage, AssignRemoveHdl ));
    m_pAssignPB->SetClickHdl(      LINK(this, SwCondCollPage, AssignRemoveHdl ));
    m_pTbLinks->SetSelectHdl(      LINK(this, SwCondCollPage, SelectHdl));
    m_pStyleLB->SetSelectHdl(      LINK(this, SwCondCollPage, SelectHdl));
    m_pFilterLB->SetSelectHdl(     LINK(this, SwCondCollPage, SelectHdl));

    m_pTbLinks->SetStyle(m_pTbLinks->GetStyle()|WB_HSCROLL|WB_CLIPCHILDREN);
    m_pTbLinks->SetSelectionMode( SINGLE_SELECTION );
    m_pTbLinks->SetTabs( &nTabs[0], MAP_APPFONT );
    m_pTbLinks->Resize();  // OS: Hack for the right selection
    m_pTbLinks->SetSpaceBetweenEntries( 0 );

    SfxStyleFamilies aFamilies(SW_RES(DLG_STYLE_DESIGNER));
    const SfxStyleFamilyItem* pFamilyItem = 0;

    size_t nCount = aFamilies.size();
    for( size_t i = 0; i < nCount; ++i )
    {
        if(SFX_STYLE_FAMILY_PARA == (pFamilyItem = aFamilies.at( i ))->GetFamily())
            break;
    }

    if (pFamilyItem)
    {
        const SfxStyleFilter& rFilterList = pFamilyItem->GetFilterList();
        for( size_t i = 0; i < rFilterList.size(); ++i )
        {
            m_pFilterLB->InsertEntry( rFilterList[ i ]->aName);
            sal_uInt16* pFilter = new sal_uInt16(rFilterList[i]->nFlags);
            m_pFilterLB->SetEntryData(i, pFilter);
        }
    }
    m_pFilterLB->SelectEntryPos(1);

    m_pTbLinks->Show();

}

SwCondCollPage::~SwCondCollPage()
{
    for(sal_Int32 i = 0; i < m_pFilterLB->GetEntryCount(); ++i)
        delete (sal_uInt16*)m_pFilterLB->GetEntryData(i);

}

int SwCondCollPage::DeactivatePage(SfxItemSet * _pSet)
{
    if( _pSet )
        FillItemSet(_pSet);

    return LEAVE_PAGE;
}

SfxTabPage* SwCondCollPage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return new SwCondCollPage(pParent, *rSet);
}

bool SwCondCollPage::FillItemSet(SfxItemSet *rSet)
{
    bool bModified = true;
    SwCondCollItem aCondItem;
    for (size_t i = 0; i < m_aStrArr.size(); ++i)
    {
        const OUString sEntry = m_pTbLinks->GetEntryText(i, 1);
        aCondItem.SetStyle( &sEntry, i);
    }
    rSet->Put(aCondItem);
    return bModified;
}

void SwCondCollPage::Reset(const SfxItemSet *)
{
    if(bNewTemplate)
        m_pConditionCB->Enable();
    if(RES_CONDTXTFMTCOLL == pFmt->Which())
        m_pConditionCB->Check();
    OnOffHdl(m_pConditionCB);

    m_pTbLinks->Clear();

    SfxStyleSheetBasePool* pPool = rSh.GetView().GetDocShell()->GetStyleSheetPool();
    pPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL);
    m_pStyleLB->Clear();
    const SfxStyleSheetBase* pBase = pPool->First();
    while( pBase )
    {
        if(!pFmt || pBase->GetName() != pFmt->GetName())
            m_pStyleLB->InsertEntry(pBase->GetName());
        pBase = pPool->Next();
    }
    m_pStyleLB->SelectEntryPos(0);

    for (size_t n = 0; n < m_aStrArr.size(); ++n)
    {
        OUString aEntry( m_aStrArr[n] + "\t" );

        const SwCollCondition* pCond = 0;
        if( pFmt && RES_CONDTXTFMTCOLL == pFmt->Which() &&
            0 != ( pCond = ((SwConditionTxtFmtColl*)pFmt)->
            HasCondition( SwCollCondition( 0, pCmds[n].nCnd, pCmds[n].nSubCond ) ) )
            && pCond->GetTxtFmtColl() )
        {
            aEntry += pCond->GetTxtFmtColl()->GetName();
        }

        SvTreeListEntry* pE = m_pTbLinks->InsertEntryToColumn( aEntry, n );
        if(0 == n)
            m_pTbLinks->Select(pE);
    }

}

const sal_uInt16* SwCondCollPage::GetRanges()
{
    return aPageRg;
}

IMPL_LINK( SwCondCollPage, OnOffHdl, CheckBox*, pBox )
{
    const bool bEnable = pBox->IsChecked();
    m_pContextFT->Enable( bEnable );
    m_pUsedFT->Enable( bEnable );
    m_pTbLinks->EnableList( bEnable );
    m_pStyleFT->Enable( bEnable );
    m_pStyleLB->Enable( bEnable );
    m_pFilterLB->Enable( bEnable );
    m_pRemovePB->Enable( bEnable );
    m_pAssignPB->Enable( bEnable );
    if( bEnable )
        SelectHdl(0);
    return 0;
}

IMPL_LINK( SwCondCollPage, AssignRemoveHdl, PushButton*, pBtn)
{
    SvTreeListEntry* pE = m_pTbLinks->FirstSelected();
    sal_uLong nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = m_pTbLinks->GetModel()->GetAbsPos( pE ) ) )
    {
        OSL_ENSURE( pE, "where's the empty entry from?" );
        return 0;
    }

    OUString sSel = m_aStrArr[nPos] + "\t";

    const bool bAssEnabled = pBtn != m_pRemovePB && m_pAssignPB->IsEnabled();
    m_pAssignPB->Enable( !bAssEnabled );
    m_pRemovePB->Enable(  bAssEnabled );
    if ( bAssEnabled )
        sSel += m_pStyleLB->GetSelectEntry();

    m_pTbLinks->SetUpdateMode(false);
    m_pTbLinks->GetModel()->Remove(pE);
    pE = m_pTbLinks->InsertEntryToColumn(sSel, nPos);
    m_pTbLinks->Select(pE);
    m_pTbLinks->MakeVisible(pE);
    m_pTbLinks->SetUpdateMode(true);
    return 0;
}

IMPL_LINK( SwCondCollPage, SelectHdl, ListBox*, pBox)
{
    if (pBox == m_pFilterLB)
    {
        m_pStyleLB->Clear();
        const sal_Int32 nSelPos = pBox->GetSelectEntryPos();
        const sal_uInt16 nSearchFlags = *(sal_uInt16*)m_pFilterLB->GetEntryData(nSelPos);
        SfxStyleSheetBasePool* pPool = rSh.GetView().GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, nSearchFlags);
        const SfxStyleSheetBase* pBase = pPool->First();

        while( pBase )
        {
            if(!pFmt || pBase->GetName() != pFmt->GetName())
                m_pStyleLB->InsertEntry(pBase->GetName());
            pBase = pPool->Next();
        }
        m_pStyleLB->SelectEntryPos(0);
        SelectHdl(m_pStyleLB);

    }
    else
    {
        SvTreeListEntry* pE = m_pTbLinks->FirstSelected();
        const OUString sTbEntry = pE
            ? m_pTbLinks->GetEntryText(pE).getToken(1, '\t')
            : OUString();
        const OUString sStyle = m_pStyleLB->GetSelectEntry();

        m_pAssignPB->Enable( sStyle != sTbEntry && m_pConditionCB->IsChecked() );

        if(pBox != m_pStyleLB)
            m_pRemovePB->Enable( m_pConditionCB->IsChecked() && !sTbEntry.isEmpty() );
    }
    return 0;
}

void SwCondCollPage::SetCollection( SwFmt* pFormat, bool bNew )
{
    pFmt = pFormat;
    bNewTemplate = bNew;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
