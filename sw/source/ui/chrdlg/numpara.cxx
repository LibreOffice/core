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
#include "swtypes.hxx"
#include "hintids.hxx"
#include "globals.hrc"
#include "helpid.h"
#include <sfx2/objsh.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/svxids.hrc>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <fmtline.hxx>
#include <numpara.hxx>

#include <app.hrc>

static const sal_uInt16 aPageRg[] = {
    FN_NUMBER_NEWSTART, FN_NUMBER_NEWSTART_AT,
    0
};

SwParagraphNumTabPage::SwParagraphNumTabPage(vcl::Window* pParent, const SfxItemSet& rAttr ) :
    SfxTabPage(pParent, "NumParaPage", "modules/swriter/ui/numparapage.ui", &rAttr),

    msOutlineNumbering( SW_RES( STR_OUTLINE_NUMBERING ) ),
    bModified(false),
    bCurNumrule(false)
{
    get(m_pOutlineStartBX,         "boxOUTLINE");
    get(m_pOutlineLvLB,            "comboLB_OUTLINE_LEVEL");

    get(m_pNumberStyleBX,          "boxNUMBER_STYLE");
    get(m_pNumberStyleLB,          "comboLB_NUMBER_STYLE");

    get(m_pNewStartBX,             "boxNEW_START");
    get(m_pNewStartCB,             "checkCB_NEW_START");
    m_pNewStartCB->SetState(TRISTATE_FALSE);
    get(m_pNewStartNumberCB,       "checkCB_NUMBER_NEW_START");
    m_pNewStartNumberCB->SetState(TRISTATE_FALSE);
    get(m_pNewStartNF,             "spinNF_NEW_START");

    get(m_pCountParaFram,          "frameFL_COUNT_PARA");
    get(m_pCountParaCB,            "checkCB_COUNT_PARA");
    m_pCountParaCB->SetState(TRISTATE_FALSE);
    get(m_pRestartParaCountCB,     "checkCB_RESTART_PARACOUNT");
    m_pRestartParaCountCB->SetState(TRISTATE_FALSE);

    get(m_pRestartBX,              "boxRESTART_NO");
    get(m_pRestartNF,              "spinNF_RESTART_PARA");

    const SfxPoolItem* pItem;
    SfxObjectShell* pObjSh;
    if(SfxItemState::SET == rAttr.GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( 0 != ( pObjSh = SfxObjectShell::Current()) &&
          0 != (pItem = pObjSh->GetItem(SID_HTML_MODE))))
    {
        const sal_uInt16 nHtmlMode = ((const SfxUInt16Item*)pItem)->GetValue();

        if (HTMLMODE_ON & nHtmlMode)
            m_pCountParaFram->Hide();
    }

    m_pNewStartCB->SetClickHdl(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    m_pNewStartNumberCB->SetClickHdl(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    m_pNumberStyleLB->SetSelectHdl(LINK(this, SwParagraphNumTabPage, StyleHdl_Impl));
    m_pCountParaCB->SetClickHdl(LINK(this, SwParagraphNumTabPage, LineCountHdl_Impl));
    m_pRestartParaCountCB->SetClickHdl( LINK(this, SwParagraphNumTabPage, LineCountHdl_Impl));
}

SwParagraphNumTabPage::~SwParagraphNumTabPage()
{
}

SfxTabPage* SwParagraphNumTabPage::Create(  vcl::Window* pParent,
                                const SfxItemSet* rSet )
{
    return new SwParagraphNumTabPage(pParent, *rSet);
}

const sal_uInt16* SwParagraphNumTabPage::GetRanges()
{
    return aPageRg;
}

bool    SwParagraphNumTabPage::FillItemSet( SfxItemSet* rSet )
{
    if( m_pOutlineLvLB->IsValueChangedFromSaved())
    {
        const sal_uInt16 aOutlineLv = m_pOutlineLvLB->GetSelectEntryPos();
        const SfxUInt16Item* pOldOutlineLv = (const SfxUInt16Item*)GetOldItem( *rSet, SID_ATTR_PARA_OUTLINE_LEVEL);
        if (pOldOutlineLv)
        {
            SfxUInt16Item* pOutlineLv = (SfxUInt16Item*)pOldOutlineLv->Clone();
            pOutlineLv->SetValue( aOutlineLv );
            rSet->Put(*pOutlineLv);
            delete pOutlineLv;
            bModified = true;
        }
    }

    if( m_pNumberStyleLB->IsValueChangedFromSaved())
    {
        OUString aStyle;
        if(m_pNumberStyleLB->GetSelectEntryPos())
            aStyle = m_pNumberStyleLB->GetSelectEntry();
        const SfxStringItem* pOldRule = (const SfxStringItem*)GetOldItem( *rSet, SID_ATTR_PARA_NUMRULE);
        SfxStringItem* pRule = pOldRule ? (SfxStringItem*)pOldRule->Clone() : NULL;
        if (pRule)
        {
            pRule->SetValue(aStyle);
            rSet->Put(*pRule);
            delete pRule;
            bModified = true;
        }
    }
    if(m_pNewStartCB->IsValueChangedFromSaved() ||
       m_pNewStartNumberCB->IsValueChangedFromSaved()||
       m_pNewStartNF->IsValueChangedFromSaved())
    {
        bModified = true;
        bool bNewStartChecked = TRISTATE_TRUE == m_pNewStartCB->GetState();
        bool bNumberNewStartChecked = TRISTATE_TRUE == m_pNewStartNumberCB->GetState();
        rSet->Put(SfxBoolItem(FN_NUMBER_NEWSTART, bNewStartChecked));
        rSet->Put(SfxUInt16Item(FN_NUMBER_NEWSTART_AT,
                  bNumberNewStartChecked && bNewStartChecked ? (sal_uInt16)m_pNewStartNF->GetValue() : USHRT_MAX));
    }

    if(m_pCountParaCB->IsValueChangedFromSaved()||
       m_pRestartParaCountCB->IsValueChangedFromSaved() ||
       m_pRestartNF->IsValueChangedFromSaved() )
    {
        SwFmtLineNumber aFmt;
        aFmt.SetStartValue( static_cast< sal_uLong >(m_pRestartParaCountCB->GetState() == TRISTATE_TRUE ?
                                m_pRestartNF->GetValue() : 0 ));
        aFmt.SetCountLines( m_pCountParaCB->IsChecked() );
        rSet->Put(aFmt);
        bModified = true;
    }
    return bModified;
}

void    SwParagraphNumTabPage::Reset( const SfxItemSet* rSet )
{
    bool bHasNumberStyle = false;

    SfxItemState eItemState = rSet->GetItemState( GetWhich(SID_ATTR_PARA_OUTLINE_LEVEL) );

    sal_Int16 nOutlineLv;
    if( eItemState >= SfxItemState::DEFAULT )
    {
        nOutlineLv = ((const SfxUInt16Item &)rSet->Get( GetWhich(SID_ATTR_PARA_OUTLINE_LEVEL) )).GetValue();
        m_pOutlineLvLB->SelectEntryPos( nOutlineLv ) ;
    }
    else
    {
        m_pOutlineLvLB->SetNoSelection();
    }
    m_pOutlineLvLB->SaveValue();

    eItemState = rSet->GetItemState( GetWhich(SID_ATTR_PARA_NUMRULE) );

    if( eItemState >= SfxItemState::DEFAULT )
    {
        OUString aStyle = ((const SfxStringItem &)rSet->Get( GetWhich(SID_ATTR_PARA_NUMRULE) )).GetValue();
        if(aStyle.isEmpty())
            aStyle = m_pNumberStyleLB->GetEntry(0);

        if( aStyle == "Outline")
        {
            m_pNumberStyleLB->InsertEntry( msOutlineNumbering );
            m_pNumberStyleLB->SelectEntry( msOutlineNumbering );
            m_pNumberStyleLB->RemoveEntry(msOutlineNumbering);
            m_pNumberStyleLB->SaveValue();
        }
        else
            m_pNumberStyleLB->SelectEntry( aStyle );

        bHasNumberStyle = true;
    }
    else
    {
        m_pNumberStyleLB->SetNoSelection();
    }

    m_pNumberStyleLB->SaveValue();

    eItemState = rSet->GetItemState( FN_NUMBER_NEWSTART );
    if(eItemState > SfxItemState::DEFAULT )
    {
        bCurNumrule = true;
        const SfxBoolItem& rStart = (const SfxBoolItem&)rSet->Get(FN_NUMBER_NEWSTART);

        m_pNewStartCB->SetState(rStart.GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );

        m_pNewStartCB->EnableTriState(false);
    }
    else
        m_pNewStartCB->SetState(bHasNumberStyle ? TRISTATE_FALSE : TRISTATE_INDET);

    m_pNewStartCB->SaveValue();

    eItemState = rSet->GetItemState( FN_NUMBER_NEWSTART_AT);
    if( eItemState > SfxItemState::DEFAULT )
    {
        const sal_uInt16 nNewStart = ((const SfxUInt16Item&)rSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
        const bool bNotMax = USHRT_MAX != nNewStart;
        m_pNewStartNumberCB->Check(bNotMax);
        m_pNewStartNF->SetValue(bNotMax ? nNewStart : 1);
        m_pNewStartNumberCB->EnableTriState(false);
    }
    else
        m_pNewStartCB->SetState(TRISTATE_INDET);
    NewStartHdl_Impl(m_pNewStartCB);
    m_pNewStartNF->SaveValue();
    m_pNewStartNumberCB->SaveValue();
    StyleHdl_Impl(m_pNumberStyleLB);
    if( SfxItemState::DEFAULT <= rSet->GetItemState(RES_LINENUMBER))
    {
        SwFmtLineNumber& rNum = (SwFmtLineNumber&)rSet->Get(RES_LINENUMBER);
        sal_uLong nStartValue = rNum.GetStartValue();
        bool bCount = rNum.IsCount();
        m_pCountParaCB->SetState( bCount ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pRestartParaCountCB->SetState( 0 != nStartValue ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pRestartNF->SetValue(nStartValue == 0 ? 1 : nStartValue);
        LineCountHdl_Impl(m_pCountParaCB);
        m_pCountParaCB->EnableTriState(false);
        m_pRestartParaCountCB->EnableTriState(false);
    }
    else
    {
        m_pCountParaCB->SetState(TRISTATE_INDET);
        m_pRestartParaCountCB->SetState(TRISTATE_INDET);
    }
    m_pCountParaCB->SaveValue();
    m_pRestartParaCountCB->SaveValue();
    m_pRestartNF->SaveValue();

    bModified = false;
}

void SwParagraphNumTabPage::DisableOutline()
{
    m_pOutlineStartBX->Disable();
}

void SwParagraphNumTabPage::DisableNumbering()
{
    m_pNumberStyleBX->Disable();
}

void SwParagraphNumTabPage::EnableNewStart()
{
    m_pNewStartCB->Show();
    m_pNewStartBX->Show();
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, NewStartHdl_Impl)
{
    bool bEnable = m_pNewStartCB->IsChecked();
    m_pNewStartNumberCB->Enable(bEnable);
    m_pNewStartNF->Enable(bEnable && m_pNewStartNumberCB->IsChecked());
    return 0;
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, LineCountHdl_Impl)
{
    m_pRestartParaCountCB->Enable(m_pCountParaCB->IsChecked());

    bool bEnableRestartValue = m_pRestartParaCountCB->IsEnabled() &&
                               m_pRestartParaCountCB->IsChecked();
    m_pRestartBX->Enable(bEnableRestartValue);

    return 0;
}

IMPL_LINK( SwParagraphNumTabPage, StyleHdl_Impl, ListBox*, pBox )
{
    bool bEnable = bCurNumrule || pBox->GetSelectEntryPos() > 0;
    m_pNewStartCB->Enable(bEnable);
    NewStartHdl_Impl(m_pNewStartCB);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
