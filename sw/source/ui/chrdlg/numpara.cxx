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
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/viewsh.hxx>

const sal_uInt16 SwParagraphNumTabPage::aPageRg[] = {
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
    get(m_pEditNumStyleBtn,        "editnumstyle");
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

    sal_Int32 numSelectPos = m_pNumberStyleLB->GetSelectEntryPos();
    if ( numSelectPos == 0 )
        m_pEditNumStyleBtn->Disable();
    else
        m_pEditNumStyleBtn->Enable();

    const SfxPoolItem* pItem;
    SfxObjectShell* pObjSh;
    if(SfxItemState::SET == rAttr.GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( 0 != ( pObjSh = SfxObjectShell::Current()) &&
          0 != (pItem = pObjSh->GetItem(SID_HTML_MODE))))
    {
        const sal_uInt16 nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

        if (HTMLMODE_ON & nHtmlMode)
            m_pCountParaFram->Hide();
    }

    m_pNewStartCB->SetClickHdl(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    m_pNewStartNumberCB->SetClickHdl(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    m_pNumberStyleLB->SetSelectHdl(LINK(this, SwParagraphNumTabPage, StyleHdl_Impl));
    m_pCountParaCB->SetClickHdl(LINK(this, SwParagraphNumTabPage, LineCountHdl_Impl));
    m_pRestartParaCountCB->SetClickHdl( LINK(this, SwParagraphNumTabPage, LineCountHdl_Impl));
    m_pNumberStyleLB->SetSelectHdl( LINK( this, SwParagraphNumTabPage, EditNumStyleSelectHdl_Impl ) );
    m_pEditNumStyleBtn->SetClickHdl( LINK(this, SwParagraphNumTabPage, EditNumStyleHdl_Impl));
}

SwParagraphNumTabPage::~SwParagraphNumTabPage()
{
    disposeOnce();
}

void SwParagraphNumTabPage::dispose()
{
    m_pOutlineStartBX.clear();
    m_pOutlineLvLB.clear();
    m_pNumberStyleBX.clear();
    m_pNumberStyleLB.clear();
    m_pEditNumStyleBtn.clear();
    m_pNewStartCB.clear();
    m_pNewStartBX.clear();
    m_pNewStartNumberCB.clear();
    m_pNewStartNF.clear();
    m_pCountParaFram.clear();
    m_pCountParaCB.clear();
    m_pRestartParaCountCB.clear();
    m_pRestartBX.clear();
    m_pRestartNF.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwParagraphNumTabPage::Create( vcl::Window* pParent,
                                                  const SfxItemSet* rSet )
{
    return VclPtr<SwParagraphNumTabPage>::Create(pParent, *rSet);
}

bool SwParagraphNumTabPage::FillItemSet( SfxItemSet* rSet )
{
    if( m_pOutlineLvLB->IsValueChangedFromSaved())
    {
        const sal_uInt16 aOutlineLv = m_pOutlineLvLB->GetSelectEntryPos();
        const SfxUInt16Item* pOldOutlineLv = static_cast<const SfxUInt16Item*>(GetOldItem( *rSet, SID_ATTR_PARA_OUTLINE_LEVEL));
        if (pOldOutlineLv)
        {
            SfxUInt16Item* pOutlineLv = static_cast<SfxUInt16Item*>(pOldOutlineLv->Clone());
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
        const SfxStringItem* pOldRule = static_cast<const SfxStringItem*>(GetOldItem( *rSet, SID_ATTR_PARA_NUMRULE));
        SfxStringItem* pRule = pOldRule ? static_cast<SfxStringItem*>(pOldRule->Clone()) : NULL;
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
        SwFormatLineNumber aFormat;
        aFormat.SetStartValue( static_cast< sal_uLong >(m_pRestartParaCountCB->GetState() == TRISTATE_TRUE ?
                                m_pRestartNF->GetValue() : 0 ));
        aFormat.SetCountLines( m_pCountParaCB->IsChecked() );
        rSet->Put(aFormat);
        bModified = true;
    }
    return bModified;
}

void SwParagraphNumTabPage::ChangesApplied()
{
    m_pOutlineLvLB->SaveValue();
    m_pNumberStyleLB->SaveValue();
    m_pNewStartCB->SaveValue();
    m_pNewStartNumberCB->SaveValue();
    m_pCountParaCB->SaveValue();
    m_pRestartParaCountCB->SaveValue();
    m_pRestartNF->SaveValue();
}
void    SwParagraphNumTabPage::Reset(const SfxItemSet* rSet)
{
    bool bHasNumberStyle = false;

    SfxItemState eItemState = rSet->GetItemState( GetWhich(SID_ATTR_PARA_OUTLINE_LEVEL) );

    sal_Int16 nOutlineLv;
    if( eItemState >= SfxItemState::DEFAULT )
    {
        nOutlineLv = static_cast<const SfxUInt16Item &>(rSet->Get( GetWhich(SID_ATTR_PARA_OUTLINE_LEVEL) )).GetValue();
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
        OUString aStyle = static_cast<const SfxStringItem &>(rSet->Get( GetWhich(SID_ATTR_PARA_NUMRULE) )).GetValue();
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
        const SfxBoolItem& rStart = static_cast<const SfxBoolItem&>(rSet->Get(FN_NUMBER_NEWSTART));

        m_pNewStartCB->SetState(rStart.GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );

        m_pNewStartCB->EnableTriState(false);
    }
    else
        m_pNewStartCB->SetState(bHasNumberStyle ? TRISTATE_FALSE : TRISTATE_INDET);

    m_pNewStartCB->SaveValue();

    eItemState = rSet->GetItemState( FN_NUMBER_NEWSTART_AT);
    if( eItemState > SfxItemState::DEFAULT )
    {
        const sal_uInt16 nNewStart = static_cast<const SfxUInt16Item&>(rSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
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
    StyleHdl_Impl(*m_pNumberStyleLB.get());
    if( SfxItemState::DEFAULT <= rSet->GetItemState(RES_LINENUMBER))
    {
        const SwFormatLineNumber& rNum = static_cast<const SwFormatLineNumber&>(rSet->Get(RES_LINENUMBER));
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

IMPL_LINK_NOARG_TYPED(SwParagraphNumTabPage, NewStartHdl_Impl, Button*, void)
{
    bool bEnable = m_pNewStartCB->IsChecked();
    m_pNewStartNumberCB->Enable(bEnable);
    m_pNewStartNF->Enable(bEnable && m_pNewStartNumberCB->IsChecked());
}


IMPL_LINK_NOARG_TYPED(SwParagraphNumTabPage, LineCountHdl_Impl, Button*, void)
{
    m_pRestartParaCountCB->Enable(m_pCountParaCB->IsChecked());

    bool bEnableRestartValue = m_pRestartParaCountCB->IsEnabled() &&
                               m_pRestartParaCountCB->IsChecked();
    m_pRestartBX->Enable(bEnableRestartValue);
}

IMPL_LINK_NOARG_TYPED( SwParagraphNumTabPage, EditNumStyleSelectHdl_Impl, ListBox&, void )
{
    sal_Int32 numSelectPos = m_pNumberStyleLB->GetSelectEntryPos();
    if ( numSelectPos == 0 )
        m_pEditNumStyleBtn->Disable();
    else
        m_pEditNumStyleBtn->Enable();
}

IMPL_LINK_NOARG_TYPED(SwParagraphNumTabPage, EditNumStyleHdl_Impl, Button*, void)
{
    OUString aTemplName(m_pNumberStyleLB->GetSelectEntry());
    ExecuteEditNumStyle_Impl( SID_STYLE_EDIT, aTemplName, OUString(),SFX_STYLE_FAMILY_PSEUDO, 0 );
}

// Internal: Perform functions through the Dispatcher
bool SwParagraphNumTabPage::ExecuteEditNumStyle_Impl(
    sal_uInt16 nId, const OUString &rStr, const OUString& rRefStr, sal_uInt16 nFamily,
    sal_uInt16 nMask, const sal_uInt16* pModifier)
{

    SfxDispatcher &rDispatcher = *SfxViewShell::Current()->GetDispatcher();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, nFamily);
    SfxUInt16Item aMask( SID_STYLE_MASK, nMask );
    SfxStringItem aUpdName(SID_STYLE_UPD_BY_EX_NAME, rStr);
    SfxStringItem aRefName( SID_STYLE_REFERENCE, rRefStr );
    const SfxPoolItem* pItems[ 6 ];
    sal_uInt16 nCount = 0;
    if( !rStr.isEmpty() )
        pItems[ nCount++ ] = &aItem;
    pItems[ nCount++ ] = &aFamily;
    if( nMask )
        pItems[ nCount++ ] = &aMask;
    if( !rRefStr.isEmpty() )
        pItems[ nCount++ ] = &aRefName;

    pItems[ nCount++ ] = 0;

    sal_uInt16 nModi = pModifier ? *pModifier : 0;
    const SfxPoolItem* mpItem = rDispatcher.Execute(
        nId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD | SfxCallMode::MODAL,
        pItems, nModi );

    if ( !mpItem )
        return false;

    return true;

}

IMPL_LINK_TYPED( SwParagraphNumTabPage, StyleHdl_Impl, ListBox&, rBox, void )
{
    bool bEnable = bCurNumrule || rBox.GetSelectEntryPos() > 0;
    m_pNewStartCB->Enable(bEnable);
    NewStartHdl_Impl(m_pNewStartCB);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
