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

#include <svl/cjkoptions.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>

#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>
#include <svx/xcolit.hxx>

#include <paragr.hxx>
#include <sdattr.hrc>

namespace {

class SdParagraphNumTabPage : public SfxTabPage
{
public:
    SdParagraphNumTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet );

    static WhichRangesContainer GetRanges();

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

private:
    bool mbModified;
    std::unique_ptr<weld::CheckButton> m_xNewStartCB;
    std::unique_ptr<weld::CheckButton> m_xNewStartNumberCB;
    std::unique_ptr<weld::SpinButton> m_xNewStartNF;

    DECL_LINK( ImplNewStartHdl, weld::Toggleable&, void );
};

}

SdParagraphNumTabPage::SdParagraphNumTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttr)
    : SfxTabPage(pPage, pController, u"modules/sdraw/ui/paranumberingtab.ui"_ustr, u"DrawParaNumbering"_ustr, &rAttr)
    , mbModified(false)
    , m_xNewStartCB(m_xBuilder->weld_check_button(u"checkbuttonCB_NEW_START"_ustr))
    , m_xNewStartNumberCB(m_xBuilder->weld_check_button(u"checkbuttonCB_NUMBER_NEW_START"_ustr))
    , m_xNewStartNF(m_xBuilder->weld_spin_button(u"spinbuttonNF_NEW_START"_ustr))
{
    m_xNewStartCB->connect_toggled(LINK(this, SdParagraphNumTabPage, ImplNewStartHdl));
    m_xNewStartNumberCB->connect_toggled(LINK(this, SdParagraphNumTabPage, ImplNewStartHdl));
}

std::unique_ptr<SfxTabPage> SdParagraphNumTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet * rAttrSet)
{
    return std::make_unique<SdParagraphNumTabPage>(pPage, pController, *rAttrSet);
}

WhichRangesContainer SdParagraphNumTabPage::GetRanges()
{
    return WhichRangesContainer(svl::Items<ATTR_PARANUMBERING_START, ATTR_PARANUMBERING_END>);
}

bool SdParagraphNumTabPage::FillItemSet( SfxItemSet* rSet )
{
    if (m_xNewStartCB->get_state_changed_from_saved() ||
        m_xNewStartNumberCB->get_state_changed_from_saved()||
        m_xNewStartNF->get_value_changed_from_saved())
    {
        mbModified = true;
        bool bNewStartChecked = TRISTATE_TRUE == m_xNewStartCB->get_state();
        bool bNumberNewStartChecked = TRISTATE_TRUE == m_xNewStartNumberCB->get_state();
        rSet->Put(SfxBoolItem(ATTR_NUMBER_NEWSTART, bNewStartChecked));

        const sal_Int16 nStartAt = static_cast<sal_Int16>(m_xNewStartNF->get_value());
        rSet->Put(SfxInt16Item(ATTR_NUMBER_NEWSTART_AT, bNumberNewStartChecked && bNewStartChecked ? nStartAt : -1));
    }

    return mbModified;
}

void SdParagraphNumTabPage::Reset( const SfxItemSet* rSet )
{
    SfxItemState eItemState = rSet->GetItemState( ATTR_NUMBER_NEWSTART );
    if(eItemState > SfxItemState::DEFAULT )
    {
        const SfxBoolItem& rStart = rSet->Get(ATTR_NUMBER_NEWSTART);
        m_xNewStartCB->set_state( rStart.GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
    {
        m_xNewStartCB->set_state(TRISTATE_INDET);
        m_xNewStartCB->set_sensitive(false);
    }
    m_xNewStartCB->save_state();

    eItemState = rSet->GetItemState( ATTR_NUMBER_NEWSTART_AT);
    if( eItemState > SfxItemState::DEFAULT )
    {
        sal_Int16 nNewStart = rSet->Get(ATTR_NUMBER_NEWSTART_AT).GetValue();
        m_xNewStartNumberCB->set_active(-1 != nNewStart);
        if(-1 == nNewStart)
            nNewStart = 1;

        m_xNewStartNF->set_value(nNewStart);
    }
    else
    {
        m_xNewStartCB->set_state(TRISTATE_INDET);
    }
    ImplNewStartHdl(*m_xNewStartCB);
    m_xNewStartNF->save_value();
    m_xNewStartNumberCB->save_state();
    mbModified = false;
}

IMPL_LINK_NOARG(SdParagraphNumTabPage, ImplNewStartHdl, weld::Toggleable&, void)
{
    bool bEnable = m_xNewStartCB->get_active();
    m_xNewStartNumberCB->set_sensitive(bEnable);
    m_xNewStartNF->set_sensitive(bEnable && m_xNewStartNumberCB->get_active());
}

SdParagraphDlg::SdParagraphDlg(weld::Window* pParent, const SfxItemSet* pAttr)
    : SfxTabDialogController(pParent, u"modules/sdraw/ui/drawparadialog.ui"_ustr,
                             u"DrawParagraphPropertiesDialog"_ustr, pAttr)
{
    AddTabPage( u"labelTP_PARA_STD"_ustr, RID_SVXPAGE_STD_PARAGRAPH);

    if( SvtCJKOptions::IsAsianTypographyEnabled() )
        AddTabPage( u"labelTP_PARA_ASIAN"_ustr, RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage( u"labelTP_PARA_ASIAN"_ustr );

    AddTabPage( u"labelTP_PARA_ALIGN"_ustr, RID_SVXPAGE_ALIGN_PARAGRAPH);

    static const bool bShowParaNumbering = ( getenv( "SD_SHOW_NUMBERING_PAGE" ) != nullptr );
    if( bShowParaNumbering )
        AddTabPage( u"labelNUMBERING"_ustr, SdParagraphNumTabPage::Create, SdParagraphNumTabPage::GetRanges );
    else
        RemoveTabPage( u"labelNUMBERING"_ustr );

    AddTabPage(u"labelTP_TABULATOR"_ustr, RID_SVXPAGE_TABULATOR);
}

void SdParagraphDlg::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "labelTP_PARA_STD")
    {
        aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, MM50/2));
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
