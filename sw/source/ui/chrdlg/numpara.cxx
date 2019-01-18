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

#include <cmdid.h>
#include <swtypes.hxx>
#include <hintids.hxx>
#include <globals.hrc>
#include <strings.hrc>
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

SwParagraphNumTabPage::SwParagraphNumTabPage(TabPageParent pParent, const SfxItemSet& rAttr)
    : SfxTabPage(pParent, "modules/swriter/ui/numparapage.ui", "NumParaPage", &rAttr)
    , msOutlineNumbering(SwResId(STR_OUTLINE_NUMBERING ))
    , bModified(false)
    , bCurNumrule(false)
    , m_xOutlineStartBX(m_xBuilder->weld_widget("boxOUTLINE"))
    , m_xOutlineLvLB(m_xBuilder->weld_combo_box("comboLB_OUTLINE_LEVEL"))
    , m_xNumberStyleBX(m_xBuilder->weld_widget("boxNUMBER_STYLE"))
    , m_xNumberStyleLB(m_xBuilder->weld_combo_box("comboLB_NUMBER_STYLE"))
    , m_xEditNumStyleBtn(m_xBuilder->weld_button("editnumstyle"))
    , m_xNewStartCB(m_xBuilder->weld_check_button("checkCB_NEW_START"))
    , m_xNewStartBX(m_xBuilder->weld_widget("boxNEW_START"))
    , m_xNewStartNumberCB(m_xBuilder->weld_check_button("checkCB_NUMBER_NEW_START"))
    , m_xNewStartNF(m_xBuilder->weld_spin_button("spinNF_NEW_START"))
    , m_xCountParaFram(m_xBuilder->weld_widget("frameFL_COUNT_PARA"))
    , m_xCountParaCB(m_xBuilder->weld_check_button("checkCB_COUNT_PARA"))
    , m_xRestartParaCountCB(m_xBuilder->weld_check_button("checkCB_RESTART_PARACOUNT"))
    , m_xRestartBX(m_xBuilder->weld_widget("boxRESTART_NO"))
    , m_xRestartNF(m_xBuilder->weld_spin_button("spinNF_RESTART_PARA"))
{
    m_xNewStartCB->set_state(TRISTATE_FALSE);
    m_xNewStartNumberCB->set_state(TRISTATE_FALSE);
    m_xCountParaCB->set_state(TRISTATE_FALSE);
    m_xRestartParaCountCB->set_state(TRISTATE_FALSE);

    int numSelectPos = m_xNumberStyleLB->get_active();
    if (numSelectPos == 0)
        m_xEditNumStyleBtn->set_sensitive(false);
    else
        m_xEditNumStyleBtn->set_sensitive(true);

    const SfxPoolItem* pItem;
    SfxObjectShell* pObjSh;
    if(SfxItemState::SET == rAttr.GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( nullptr != ( pObjSh = SfxObjectShell::Current()) &&
          nullptr != (pItem = pObjSh->GetItem(SID_HTML_MODE))))
    {
        const sal_uInt16 nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

        if (HTMLMODE_ON & nHtmlMode)
            m_xCountParaFram->hide();
    }

    m_xNewStartCB->connect_toggled(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    m_xNewStartNumberCB->connect_toggled(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    m_xNumberStyleLB->connect_changed(LINK(this, SwParagraphNumTabPage, StyleHdl_Impl));
    m_xCountParaCB->connect_toggled(LINK(this, SwParagraphNumTabPage, LineCountHdl_Impl));
    m_xRestartParaCountCB->connect_toggled(LINK(this, SwParagraphNumTabPage, LineCountHdl_Impl));
    m_xNumberStyleLB->connect_changed(LINK(this, SwParagraphNumTabPage, EditNumStyleSelectHdl_Impl));
    m_xEditNumStyleBtn->connect_clicked(LINK(this, SwParagraphNumTabPage, EditNumStyleHdl_Impl));
}

SwParagraphNumTabPage::~SwParagraphNumTabPage()
{
}

VclPtr<SfxTabPage> SwParagraphNumTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwParagraphNumTabPage>::Create(pParent, *rSet);
}

bool SwParagraphNumTabPage::FillItemSet( SfxItemSet* rSet )
{
    if (m_xOutlineLvLB->get_value_changed_from_saved())
    {
        const sal_uInt16 aOutlineLv = m_xOutlineLvLB->get_active();
        const SfxUInt16Item* pOldOutlineLv = static_cast<const SfxUInt16Item*>(GetOldItem( *rSet, SID_ATTR_PARA_OUTLINE_LEVEL));
        if (pOldOutlineLv)
        {
            std::unique_ptr<SfxUInt16Item> pOutlineLv(static_cast<SfxUInt16Item*>(pOldOutlineLv->Clone()));
            pOutlineLv->SetValue( aOutlineLv );
            rSet->Put(*pOutlineLv);
            bModified = true;
        }
    }

    if (m_xNumberStyleLB->get_value_changed_from_saved())
    {
        OUString aStyle;
        if (m_xNumberStyleLB->get_active())
            aStyle = m_xNumberStyleLB->get_active_text();
        const SfxStringItem* pOldRule = static_cast<const SfxStringItem*>(GetOldItem( *rSet, SID_ATTR_PARA_NUMRULE));
        if (pOldRule)
        {
            std::unique_ptr<SfxStringItem> pRule(static_cast<SfxStringItem*>(pOldRule->Clone()));
            pRule->SetValue(aStyle);
            rSet->Put(*pRule);
            bModified = true;
        }
    }
    if (m_xNewStartCB->get_state_changed_from_saved() ||
        m_xNewStartNumberCB->get_state_changed_from_saved()||
        m_xNewStartNF->get_value_changed_from_saved())
    {
        bModified = true;
        bool bNewStartChecked = TRISTATE_TRUE == m_xNewStartCB->get_state();
        bool bNumberNewStartChecked = TRISTATE_TRUE == m_xNewStartNumberCB->get_state();
        rSet->Put(SfxBoolItem(FN_NUMBER_NEWSTART, bNewStartChecked));
        rSet->Put(SfxUInt16Item(FN_NUMBER_NEWSTART_AT,
                  bNumberNewStartChecked && bNewStartChecked ? static_cast<sal_uInt16>(m_xNewStartNF->get_value()) : USHRT_MAX));
    }

    if (m_xCountParaCB->get_state_changed_from_saved()||
        m_xRestartParaCountCB->get_state_changed_from_saved() ||
        m_xRestartNF->get_value_changed_from_saved())
    {
        SwFormatLineNumber aFormat;
        aFormat.SetStartValue( static_cast< sal_uLong >(m_xRestartParaCountCB->get_state() == TRISTATE_TRUE ?
                                m_xRestartNF->get_value() : 0 ));
        aFormat.SetCountLines(m_xCountParaCB->get_active());
        rSet->Put(aFormat);
        bModified = true;
    }
    return bModified;
}

void SwParagraphNumTabPage::ChangesApplied()
{
    m_xOutlineLvLB->save_value();
    m_xNumberStyleLB->save_value();
    m_xNewStartCB->save_state();
    m_xNewStartNumberCB->save_state();
    m_xCountParaCB->save_state();
    m_xRestartParaCountCB->save_state();
    m_xRestartNF->save_value();
}

void SwParagraphNumTabPage::Reset(const SfxItemSet* rSet)
{
    bool bHasNumberStyle = false;

    SfxItemState eItemState = rSet->GetItemState( GetWhich(SID_ATTR_PARA_OUTLINE_LEVEL) );

    sal_Int16 nOutlineLv;
    if( eItemState >= SfxItemState::DEFAULT )
    {
        nOutlineLv = static_cast<const SfxUInt16Item &>(rSet->Get( GetWhich(SID_ATTR_PARA_OUTLINE_LEVEL) )).GetValue();
        m_xOutlineLvLB->set_active(nOutlineLv) ;
    }
    else
    {
        m_xOutlineLvLB->set_active(-1);
    }
    m_xOutlineLvLB->save_value();

    eItemState = rSet->GetItemState( GetWhich(SID_ATTR_PARA_NUMRULE) );

    if( eItemState >= SfxItemState::DEFAULT )
    {
        OUString aStyle = static_cast<const SfxStringItem &>(rSet->Get( GetWhich(SID_ATTR_PARA_NUMRULE) )).GetValue();
        if(aStyle.isEmpty())
            aStyle = m_xNumberStyleLB->get_text(0);

        if( aStyle == "Outline")
        {
            //add it, select it, remove it ? do we really want set_active(-1) instead ?
            m_xNumberStyleLB->append_text(msOutlineNumbering);
            m_xNumberStyleLB->set_active_text(msOutlineNumbering);
            m_xNumberStyleLB->remove_text(msOutlineNumbering);
            m_xNumberStyleLB->save_value();
        }
        else
            m_xNumberStyleLB->set_active_text(aStyle);

        bHasNumberStyle = true;
    }
    else
    {
        m_xNumberStyleLB->set_active(-1);
    }

    m_xNumberStyleLB->save_value();

    eItemState = rSet->GetItemState( FN_NUMBER_NEWSTART );
    if(eItemState > SfxItemState::DEFAULT )
    {
        bCurNumrule = true;
        const SfxBoolItem& rStart = static_cast<const SfxBoolItem&>(rSet->Get(FN_NUMBER_NEWSTART));

        m_xNewStartCB->set_state(rStart.GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
        m_xNewStartCB->set_state(bHasNumberStyle ? TRISTATE_FALSE : TRISTATE_INDET);

    m_xNewStartCB->save_state();

    eItemState = rSet->GetItemState( FN_NUMBER_NEWSTART_AT);
    if( eItemState > SfxItemState::DEFAULT )
    {
        const sal_uInt16 nNewStart = static_cast<const SfxUInt16Item&>(rSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
        const bool bNotMax = USHRT_MAX != nNewStart;
        m_xNewStartNumberCB->set_active(bNotMax);
        m_xNewStartNF->set_value(bNotMax ? nNewStart : 1);
    }
    else
        m_xNewStartCB->set_state(TRISTATE_INDET);
    NewStartHdl_Impl(*m_xNewStartCB);
    m_xNewStartNF->save_value();
    m_xNewStartNumberCB->save_state();
    StyleHdl_Impl(*m_xNumberStyleLB);
    if( SfxItemState::DEFAULT <= rSet->GetItemState(RES_LINENUMBER))
    {
        const SwFormatLineNumber& rNum = rSet->Get(RES_LINENUMBER);
        sal_uLong nStartValue = rNum.GetStartValue();
        bool bCount = rNum.IsCount();
        m_xCountParaCB->set_state(bCount ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_xRestartParaCountCB->set_state(0 != nStartValue ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_xRestartNF->set_value(nStartValue == 0 ? 1 : nStartValue);
        LineCountHdl_Impl(*m_xCountParaCB);
    }
    else
    {
        m_xCountParaCB->set_state(TRISTATE_INDET);
        m_xRestartParaCountCB->set_state(TRISTATE_INDET);
    }
    m_xCountParaCB->save_state();
    m_xRestartParaCountCB->save_state();
    m_xRestartNF->save_value();

    bModified = false;
}

void SwParagraphNumTabPage::DisableOutline()
{
    m_xOutlineStartBX->set_sensitive(false);
    m_xOutlineStartBX->set_tooltip_text( SwResId(STR_OUTLINENUMBERING_DISABLED) );
}

void SwParagraphNumTabPage::DisableNumbering()
{
    m_xNumberStyleBX->set_sensitive(false);
    m_xNumberStyleBX->set_tooltip_text( SwResId(STR_OUTLINENUMBERING_DISABLED) );
}

void SwParagraphNumTabPage::EnableNewStart()
{
    m_xNewStartCB->show();
    m_xNewStartBX->show();
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, NewStartHdl_Impl, weld::ToggleButton&, void)
{
    bool bEnable = m_xNewStartCB->get_active();
    m_xNewStartNumberCB->set_sensitive(bEnable);
    m_xNewStartNF->set_sensitive(bEnable && m_xNewStartNumberCB->get_active());
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, LineCountHdl_Impl, weld::ToggleButton&, void)
{
    m_xRestartParaCountCB->set_sensitive(m_xCountParaCB->get_active());

    bool bEnableRestartValue = m_xRestartParaCountCB->get_sensitive() &&
                               m_xRestartParaCountCB->get_active();
    m_xRestartBX->set_sensitive(bEnableRestartValue);
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, EditNumStyleSelectHdl_Impl, weld::ComboBox&, void)
{
    int numSelectPos = m_xNumberStyleLB->get_active();
    if (numSelectPos == 0)
        m_xEditNumStyleBtn->set_sensitive(false);
    else
        m_xEditNumStyleBtn->set_sensitive(true);
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, EditNumStyleHdl_Impl, weld::Button&, void)
{
    OUString aTemplName(m_xNumberStyleLB->get_active_text());
    ExecuteEditNumStyle_Impl( SID_STYLE_EDIT, aTemplName, SfxStyleFamily::Pseudo );
}

// Internal: Perform functions through the Dispatcher
bool SwParagraphNumTabPage::ExecuteEditNumStyle_Impl(
    sal_uInt16 nId, const OUString &rStr, SfxStyleFamily nFamily)
{

    SfxDispatcher &rDispatcher = *SfxViewShell::Current()->GetDispatcher();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, static_cast<sal_uInt16>(nFamily));
    const SfxPoolItem* pItems[ 3 ];
    sal_uInt16 nCount = 0;
    if( !rStr.isEmpty() )
        pItems[ nCount++ ] = &aItem;
    pItems[ nCount++ ] = &aFamily;

    pItems[ nCount++ ] = nullptr;

    const SfxPoolItem* pItem = rDispatcher.Execute(
        nId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
        pItems );

    return pItem != nullptr;

}

IMPL_LINK(SwParagraphNumTabPage, StyleHdl_Impl, weld::ComboBox&, rBox, void)
{
    bool bEnable = bCurNumrule || rBox.get_active() > 0;
    m_xNewStartCB->set_sensitive(bEnable);
    NewStartHdl_Impl(*m_xNewStartCB);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
