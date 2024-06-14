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
#include <strings.hrc>
#include <sfx2/objsh.hxx>
#include <sfx2/htmlmode.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <fmtline.hxx>
#include <numpara.hxx>

#include <officecfg/Office/Common.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewsh.hxx>

// tdf#115871 - reset outline and list options to parent settings
const WhichRangesContainer SwParagraphNumTabPage::s_aPageRg(
    svl::Items<RES_LINENUMBER, RES_LINENUMBER, SID_ATTR_PARA_NUMRULE, SID_ATTR_PARA_NUMRULE,
               SID_ATTR_PARA_OUTLINE_LEVEL, SID_ATTR_PARA_OUTLINE_LEVEL, FN_NUMBER_NEWSTART,
               FN_NUMBER_NEWSTART_AT>);

SwParagraphNumTabPage::SwParagraphNumTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttr)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/numparapage.ui"_ustr, u"NumParaPage"_ustr, &rAttr)
    , msOutlineNumbering(SwResId(STR_OUTLINE_NUMBERING ))
    , m_bModified(false)
    , m_bCurNumrule(false)
    , m_xOutlineStartBX(m_xBuilder->weld_widget(u"boxOUTLINE"_ustr))
    , m_xOutlineLvLB(m_xBuilder->weld_combo_box(u"comboLB_OUTLINE_LEVEL"_ustr))
    , m_xNumberStyleBX(m_xBuilder->weld_widget(u"boxNUMBER_STYLE"_ustr))
    , m_xNumberStyleLB(m_xBuilder->weld_combo_box(u"comboLB_NUMBER_STYLE"_ustr))
    , m_xEditNumStyleBtn(m_xBuilder->weld_button(u"editnumstyle"_ustr))
    , m_xListLvBX(m_xBuilder->weld_widget(u"boxLIST_LEVEL"_ustr))
    , m_xListLvLB(m_xBuilder->weld_combo_box(u"comboLB_LIST_LEVEL"_ustr))
    , m_xNewStartCB(m_xBuilder->weld_check_button(u"checkCB_NEW_START"_ustr))
    , m_xNewStartBX(m_xBuilder->weld_widget(u"boxNEW_START"_ustr))
    , m_xNewStartNumberCB(m_xBuilder->weld_check_button(u"checkCB_NUMBER_NEW_START"_ustr))
    , m_xNewStartNF(m_xBuilder->weld_spin_button(u"spinNF_NEW_START"_ustr))
    , m_xCountParaFram(m_xBuilder->weld_widget(u"frameFL_COUNT_PARA"_ustr))
    , m_xCountParaCB(m_xBuilder->weld_check_button(u"checkCB_COUNT_PARA"_ustr))
    , m_xRestartParaCountCB(m_xBuilder->weld_check_button(u"checkCB_RESTART_PARACOUNT"_ustr))
    , m_xRestartBX(m_xBuilder->weld_widget(u"boxRESTART_NO"_ustr))
    , m_xRestartNF(m_xBuilder->weld_spin_button(u"spinNF_RESTART_PARA"_ustr))
{
    m_xNewStartCB->set_state(TRISTATE_FALSE);
    m_xNewStartNumberCB->set_state(TRISTATE_FALSE);
    m_xCountParaCB->set_state(TRISTATE_FALSE);
    m_xRestartParaCountCB->set_state(TRISTATE_FALSE);
    m_xEditNumStyleBtn->set_sensitive(false);

    const SfxUInt16Item* pItem = rAttr.GetItemIfSet(SID_HTML_MODE, false);
    if (!pItem)
    {
        if (SfxObjectShell* pObjSh = SfxObjectShell::Current())
            pItem = pObjSh->GetItem(SID_HTML_MODE);
    }
    if(pItem)
    {
        const sal_uInt16 nHtmlMode = pItem->GetValue();

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

    if (officecfg::Office::Common::Misc::ExperimentalMode::get())
        m_xListLvBX->show();
    else
        m_xListLvBX->hide();
}

SwParagraphNumTabPage::~SwParagraphNumTabPage()
{
}

std::unique_ptr<SfxTabPage> SwParagraphNumTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<SwParagraphNumTabPage>(pPage, pController, *rSet);
}

bool SwParagraphNumTabPage::FillItemSet( SfxItemSet* rSet )
{
    if (m_xOutlineLvLB->get_value_changed_from_saved())
    {
        const sal_uInt16 aOutlineLv = m_xOutlineLvLB->get_active();
        const SfxUInt16Item* pOldOutlineLv = GetOldItem( *rSet, SID_ATTR_PARA_OUTLINE_LEVEL);
        if (pOldOutlineLv)
        {
            std::unique_ptr<SfxUInt16Item> pOutlineLv(pOldOutlineLv->Clone());
            pOutlineLv->SetValue( aOutlineLv );
            rSet->Put(std::move(pOutlineLv));
            m_bModified = true;

            // Does List Level need to be set to be the same as Outline Level?
            if (!m_xListLvLB->get_active() && m_xListLvBX->get_visible()
                && !m_xListLvLB->get_value_changed_from_saved())
            {
                sal_Int16 nListLevel = std::max<sal_Int16>(1, aOutlineLv);
                --nListLevel; // Outline Level is 1 based. List Level is zero based.
                rSet->Put(SfxInt16Item(RES_PARATR_LIST_LEVEL, nListLevel));
            }
        }
    }

    if (m_xListLvLB->get_value_changed_from_saved())
    {
        if (m_xListLvBX->get_visible())
        {
            sal_Int16 nListLevel = m_xListLvLB->get_active();
            // Does List Level need to be set to be the same as Outline Level?
            if (!nListLevel)
            {
                nListLevel = std::max<sal_Int16>(1, m_xOutlineLvLB->get_active());
            }
            --nListLevel; // List Level is zero based, but both listboxes are 1-based.

            rSet->Put(SfxInt16Item(RES_PARATR_LIST_LEVEL, nListLevel));
            m_bModified = true;
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
            std::unique_ptr<SfxStringItem> pRule(pOldRule->Clone());
            pRule->SetValue(aStyle);
            rSet->Put(std::move(pRule));
            m_bModified = true;
        }
    }
    if (m_xNewStartCB->get_state_changed_from_saved() ||
        m_xNewStartNumberCB->get_state_changed_from_saved()||
        m_xNewStartNF->get_value_changed_from_saved())
    {
        m_bModified = true;
        bool bNewStartChecked = TRISTATE_TRUE == m_xNewStartCB->get_state();
        bool bNumberNewStartChecked = TRISTATE_TRUE == m_xNewStartNumberCB->get_state();
        rSet->Put(SfxBoolItem(FN_NUMBER_NEWSTART, bNewStartChecked));
        rSet->Put(SfxUInt16Item(FN_NUMBER_NEWSTART_AT,
                  bNumberNewStartChecked && bNewStartChecked ? o3tl::narrowing<sal_uInt16>(m_xNewStartNF->get_value()) : USHRT_MAX));
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
        m_bModified = true;
    }
    return m_bModified;
}

void SwParagraphNumTabPage::ChangesApplied()
{
    m_xOutlineLvLB->save_value();
    m_xNumberStyleLB->save_value();
    m_xListLvLB->save_value();
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

    sal_Int16 nOutlineLv = 1; // 0 is Text Body, 1 is level 1
    if( eItemState >= SfxItemState::DEFAULT )
    {
        nOutlineLv = rSet->Get( GetWhich(SID_ATTR_PARA_OUTLINE_LEVEL) ).GetValue();
        m_xOutlineLvLB->set_active(nOutlineLv) ;
    }
    else
    {
        m_xOutlineLvLB->set_active(-1);
    }
    m_xOutlineLvLB->save_value();

    eItemState = rSet->GetItemState(RES_PARATR_LIST_LEVEL);
    if (eItemState >= SfxItemState::DEFAULT)
    {
        sal_Int16 nListLevel = rSet->Get(RES_PARATR_LIST_LEVEL).GetValue(); // 0 is level 1
        // Although listLevel doesn't have outline's "Text Body" level, treat it the same as level 1
        // so that if the outline level is either 0 or 1, it is considered equal to list level 1.
        // This is a rather crucial discrepancy - otherwise the user will rarely see
        // list level using the special "Same as outline level,
        // and the highly desirable state of keeping the two in sync will rarely be achieved.
        if ((!nOutlineLv && !nListLevel) || nListLevel == nOutlineLv - 1)
            m_xListLvLB->set_active(0); // highly encourage using "Same as outline level"
        else
            m_xListLvLB->set_active(nListLevel + 1);
    }
    else
    {
        m_xListLvBX->hide();
    }
    m_xListLvLB->save_value();

    eItemState = rSet->GetItemState( GetWhich(SID_ATTR_PARA_NUMRULE) );

    if( eItemState >= SfxItemState::DEFAULT )
    {
        OUString aStyle = static_cast<const SfxStringItem &>(rSet->Get( GetWhich(SID_ATTR_PARA_NUMRULE) )).GetValue();
        if(aStyle.isEmpty())
            aStyle = m_xNumberStyleLB->get_text(0);

        if( aStyle == "Outline")
        {
            if (m_xNumberStyleLB->find_id(u"pseudo"_ustr) == -1)
            {
                // tdf#145804 show "Chapter Numbering"
                m_xNumberStyleLB->append(u"pseudo"_ustr, msOutlineNumbering);
            }
            m_xNumberStyleLB->set_active_id(u"pseudo"_ustr);
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

    if (m_xNumberStyleBX->get_sensitive())
        EditNumStyleSelectHdl_Impl(*m_xNumberStyleLB);

    m_xNumberStyleLB->save_value();

    eItemState = rSet->GetItemState( FN_NUMBER_NEWSTART );
    if(eItemState > SfxItemState::DEFAULT )
    {
        m_bCurNumrule = true;
        const SfxBoolItem& rStart = static_cast<const SfxBoolItem&>(rSet->Get(FN_NUMBER_NEWSTART));

        m_xNewStartCB->set_state(rStart.GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
        m_xNewStartCB->set_state(bHasNumberStyle ? TRISTATE_FALSE : TRISTATE_INDET);

    m_xNewStartCB->save_state();

    eItemState = rSet->GetItemState( FN_NUMBER_NEWSTART_AT);
    if( eItemState > SfxItemState::DEFAULT )
    {
        const sal_uInt16 nNewStart = rSet->Get(FN_NUMBER_NEWSTART_AT).GetValue();
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

    m_bModified = false;
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
    m_xListLvBX->set_sensitive(false);
}

void SwParagraphNumTabPage::EnableNewStart()
{
    m_xNewStartCB->show();
    m_xNewStartBX->show();
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, NewStartHdl_Impl, weld::Toggleable&, void)
{
    bool bEnable = m_xNewStartCB->get_active();
    m_xNewStartNumberCB->set_sensitive(bEnable);
    m_xNewStartNF->set_sensitive(bEnable && m_xNewStartNumberCB->get_active());
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, LineCountHdl_Impl, weld::Toggleable&, void)
{
    m_xRestartParaCountCB->set_sensitive(m_xCountParaCB->get_active());

    bool bEnableRestartValue = m_xRestartParaCountCB->get_sensitive() &&
                               m_xRestartParaCountCB->get_active();
    m_xRestartBX->set_sensitive(bEnableRestartValue);
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, EditNumStyleSelectHdl_Impl, weld::ComboBox&, void)
{
    int numSelectPos = m_xNumberStyleLB->get_active();
    // 0 is "None" and -1 is unselected state and a "pseudo" is uneditable "Chapter Numbering"
    if (numSelectPos == 0 || numSelectPos == -1 || m_xNumberStyleLB->get_active_id() == "pseudo")
    {
        m_xEditNumStyleBtn->set_sensitive(false);
        m_xListLvBX->set_sensitive(false);
    }
    else
    {
        m_xEditNumStyleBtn->set_sensitive(true);
        m_xListLvBX->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(SwParagraphNumTabPage, EditNumStyleHdl_Impl, weld::Button&, void)
{
    OUString aTemplName(m_xNumberStyleLB->get_active_text());
    ExecuteEditNumStyle_Impl( SID_STYLE_EDIT, aTemplName, SfxStyleFamily::Pseudo );
}

// Internal: Perform functions through the Dispatcher
void SwParagraphNumTabPage::ExecuteEditNumStyle_Impl(
    sal_uInt16 nId, const OUString &rStr, SfxStyleFamily nFamily)
{
    SfxViewShell* pViewShell = SfxViewShell::Current();

    if( !pViewShell)
        return;

    SfxDispatcher* pDispatcher = pViewShell->GetDispatcher();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, static_cast<sal_uInt16>(nFamily));
    const SfxPoolItem* pItems[ 3 ];
    sal_uInt16 nCount = 0;
    if( !rStr.isEmpty() )
        pItems[ nCount++ ] = &aItem;
    pItems[ nCount++ ] = &aFamily;

    pItems[ nCount++ ] = nullptr;

    // tdf#145363 we want the current dialog to be the parent of the new dialog
    weld::Window* pDialogParent = GetFrameWeld();
    css::uno::Any aAny(pDialogParent->GetXWindow());
    SfxUnoAnyItem aDialogParent(SID_DIALOG_PARENT, aAny);
    const SfxPoolItem* pInternalItems[ 2 ];
    pInternalItems[ 0 ] = &aDialogParent;
    pInternalItems[ 1 ] = nullptr;

    pDispatcher->Execute(
        nId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
        pItems, 0, pInternalItems);
}

IMPL_LINK(SwParagraphNumTabPage, StyleHdl_Impl, weld::ComboBox&, rBox, void)
{
    bool bEnable = m_bCurNumrule || rBox.get_active() > 0;
    m_xNewStartCB->set_sensitive(bEnable);
    NewStartHdl_Impl(*m_xNewStartCB);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
