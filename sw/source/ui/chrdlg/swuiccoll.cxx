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
#include "ccoll.hrc"
#include <vcl/svapp.hxx>

#include "swuiccoll.hxx"

static sal_uInt16 aPageRg[] = {
    FN_COND_COLL, FN_COND_COLL,
    0
};

// Warning! This table is indicated directly in code (0, 1, ...)
static long nTabs[] =
    {   2, // Number of Tabs
        0, 100
    };

SwCondCollPage::SwCondCollPage(Window *pParent, const SfxItemSet &rSet)

    : SfxTabPage(pParent, SW_RES(TP_CONDCOLL), rSet),
    aConditionFL( this, SW_RES( FL_CONDITION )),
    aConditionCB( this, SW_RES( CB_CONDITION ) ),
    aContextFT  ( this, SW_RES( FT_CONTEXT  ) ),
    aUsedFT     ( this, SW_RES( FT_USED         ) ),
    aTbLinks(     this, SW_RES( TB_CONDCOLLS ) ),
    aStyleFT    ( this, SW_RES( FT_STYLE    ) ),
    aStyleLB    ( this, SW_RES( LB_STYLE    ) ),
    aFilterLB   ( this, SW_RES( LB_FILTER   ) ),
    aRemovePB   ( this, SW_RES( PB_REMOVE   ) ),
    aAssignPB   ( this, SW_RES( PB_ASSIGN   ) ),
    sNoTmpl     (       SW_RES( STR_NOTEMPL  ) ),
    aStrArr     (       SW_RES( STR_REGIONS  ) ),
    rSh(::GetActiveView()->GetWrtShell()),
    pCmds( SwCondCollItem::GetCmds() ),
    pFmt(0),

    bNewTemplate(sal_False)
{
    FreeResource();
    SetExchangeSupport();

    aRemovePB.SetAccessibleRelationMemberOf(&aConditionFL);
    aAssignPB.SetAccessibleRelationMemberOf(&aConditionFL);
    aTbLinks.SetAccessibleRelationLabeledBy(&aConditionCB);

    // Install handlers
    aConditionCB.SetClickHdl(   LINK(this, SwCondCollPage, OnOffHdl));
    aTbLinks.SetDoubleClickHdl( LINK(this, SwCondCollPage, AssignRemoveHdl ));
    aStyleLB.SetDoubleClickHdl( LINK(this, SwCondCollPage, AssignRemoveHdl ));
    aRemovePB.SetClickHdl(      LINK(this, SwCondCollPage, AssignRemoveHdl ));
    aAssignPB.SetClickHdl(      LINK(this, SwCondCollPage, AssignRemoveHdl ));
    aTbLinks.SetSelectHdl(      LINK(this, SwCondCollPage, SelectHdl));
    aStyleLB.SetSelectHdl(      LINK(this, SwCondCollPage, SelectHdl));
    aFilterLB.SetSelectHdl(     LINK(this, SwCondCollPage, SelectHdl));

    aTbLinks.SetStyle(aTbLinks.GetStyle()|WB_HSCROLL|WB_CLIPCHILDREN);
    aTbLinks.SetSelectionMode( SINGLE_SELECTION );
    aTbLinks.SetTabs( &nTabs[0], MAP_APPFONT );
    aTbLinks.Resize();  // OS: Hack for the right selection
    aTbLinks.SetSpaceBetweenEntries( 0 );
    aTbLinks.SetHelpId(HID_COND_COLL_TABLIST);

    SfxStyleFamilies aFamilies(SW_RES(DLG_STYLE_DESIGNER));
    const SfxStyleFamilyItem* pFamilyItem = 0;

    size_t nCount = aFamilies.size();
    for( size_t i = 0; i < nCount; ++i )
    {
        if(SFX_STYLE_FAMILY_PARA == (sal_uInt16)(pFamilyItem = aFamilies.at( i ))->GetFamily())
            break;
    }

    const SfxStyleFilter& rFilterList = pFamilyItem->GetFilterList();
    for( size_t i = 0; i < rFilterList.size(); ++i )
    {
        aFilterLB.InsertEntry( rFilterList[ i ]->aName);
        sal_uInt16* pFilter = new sal_uInt16(rFilterList[i]->nFlags);
        aFilterLB.SetEntryData(i, pFilter);
    }
    aFilterLB.SelectEntryPos(1);

    aTbLinks.Show();

}

/****************************************************************************
Page: Dtor
****************************************************************************/


SwCondCollPage::~SwCondCollPage()
{
    for(sal_uInt16 i = 0; i < aFilterLB.GetEntryCount(); ++i)
        delete (sal_uInt16*)aFilterLB.GetEntryData(i);

}


int SwCondCollPage::DeactivatePage(SfxItemSet * _pSet)
{
    if( _pSet )
        FillItemSet(*_pSet);

    return LEAVE_PAGE;
}

/****************************************************************************
Page: Factory
****************************************************************************/


SfxTabPage* SwCondCollPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwCondCollPage(pParent, rSet);
}

/****************************************************************************
Page: FillItemSet-Overload
****************************************************************************/


sal_Bool SwCondCollPage::FillItemSet(SfxItemSet &rSet)
{
    sal_Bool bModified = sal_True;
    SwCondCollItem aCondItem;
    for(sal_uInt16 i = 0; i < aStrArr.Count(); i++)
    {
        String sEntry = aTbLinks.GetEntryText(i, 1);
        aCondItem.SetStyle( &sEntry, i);
    }
    rSet.Put(aCondItem);
    return bModified;
}

/****************************************************************************
Page: Reset-Overload
****************************************************************************/


void SwCondCollPage::Reset(const SfxItemSet &/*rSet*/)
{
    if(bNewTemplate)
        aConditionCB.Enable();
    if(RES_CONDTXTFMTCOLL == pFmt->Which())
        aConditionCB.Check();
    OnOffHdl(&aConditionCB);

    aTbLinks.Clear();

    SfxStyleSheetBasePool* pPool = rSh.GetView().GetDocShell()->GetStyleSheetPool();
    pPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL);
    aStyleLB.Clear();
    const SfxStyleSheetBase* pBase = pPool->First();
    while( pBase )
    {
        if(!pFmt || pBase->GetName() != pFmt->GetName())
            aStyleLB.InsertEntry(pBase->GetName());
        pBase = pPool->Next();
    }
    aStyleLB.SelectEntryPos(0);

    for( sal_uInt16 n = 0; n < aStrArr.Count(); n++)
    {
        String aEntry( aStrArr.GetString(n) );
        aEntry += '\t';

        const SwCollCondition* pCond = 0;
        if( pFmt && RES_CONDTXTFMTCOLL == pFmt->Which() &&
            0 != ( pCond = ((SwConditionTxtFmtColl*)pFmt)->
            HasCondition( SwCollCondition( 0, pCmds[n].nCnd, pCmds[n].nSubCond ) ) )
            && pCond->GetTxtFmtColl() )
        {
            aEntry += pCond->GetTxtFmtColl()->GetName();
        }

        SvTreeListEntry* pE = aTbLinks.InsertEntryToColumn( aEntry, n );
        if(0 == n)
            aTbLinks.Select(pE);
    }

}

sal_uInt16* SwCondCollPage::GetRanges()
{
    return aPageRg;
}

IMPL_LINK( SwCondCollPage, OnOffHdl, CheckBox*, pBox )
{
    const sal_Bool bEnable = pBox->IsChecked();
    aContextFT.Enable( bEnable );
    aUsedFT   .Enable( bEnable );
    aTbLinks  .EnableList( bEnable != sal_False );
    aStyleFT  .Enable( bEnable );
    aStyleLB  .Enable( bEnable );
    aFilterLB .Enable( bEnable );
    aRemovePB .Enable( bEnable );
    aAssignPB .Enable( bEnable );
    if( bEnable )
        SelectHdl(0);
    return 0;
}

IMPL_LINK( SwCondCollPage, AssignRemoveHdl, PushButton*, pBtn)
{
    SvTreeListEntry* pE = aTbLinks.FirstSelected();
    sal_uLong nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = aTbLinks.GetModel()->GetAbsPos( pE ) ) )
    {
        OSL_ENSURE( pE, "where's the empty entry from?" );
        return 0;
    }

    String sSel = aStrArr.GetString( sal_uInt16(nPos) );
    sSel += '\t';

    const sal_Bool bAssEnabled = pBtn != &aRemovePB && aAssignPB.IsEnabled();
    aAssignPB.Enable( !bAssEnabled );
    aRemovePB.Enable(  bAssEnabled );
    if ( bAssEnabled )
        sSel += aStyleLB.GetSelectEntry();

    aTbLinks.SetUpdateMode(sal_False);
    aTbLinks.GetModel()->Remove(pE);
    pE = aTbLinks.InsertEntryToColumn(sSel, nPos);
    aTbLinks.Select(pE);
    aTbLinks.MakeVisible(pE);
    aTbLinks.SetUpdateMode(sal_True);
    return 0;
}

IMPL_LINK( SwCondCollPage, SelectHdl, ListBox*, pBox)
{
    if(pBox == &aFilterLB)
    {
        aStyleLB.Clear();
        sal_uInt16 nSearchFlags = pBox->GetSelectEntryPos();
        nSearchFlags = *(sal_uInt16*)aFilterLB.GetEntryData(nSearchFlags);
        SfxStyleSheetBasePool* pPool = rSh.GetView().GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, nSearchFlags);
        const SfxStyleSheetBase* pBase = pPool->First();

        while( pBase )
        {
            if(!pFmt || pBase->GetName() != pFmt->GetName())
                aStyleLB.InsertEntry(pBase->GetName());
            pBase = pPool->Next();
        }
        aStyleLB.SelectEntryPos(0);
        SelectHdl(&aStyleLB);

    }
    else
    {
        String sTbEntry;
        SvTreeListEntry* pE = aTbLinks.FirstSelected();
        if(pE)
            sTbEntry = aTbLinks.GetEntryText(pE);
        sTbEntry = sTbEntry.GetToken(1, '\t');
        String sStyle = aStyleLB.GetSelectEntry();

        aAssignPB.Enable( sStyle != sTbEntry && aConditionCB.IsChecked() );

        if(pBox != &aStyleLB)
            aRemovePB.Enable( aConditionCB.IsChecked() && sTbEntry.Len() );
    }
    return 0;
}

void SwCondCollPage::SetCollection( SwFmt* pFormat, sal_Bool bNew )
{
    pFmt = pFormat;
    bNewTemplate = bNew;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
