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

#include <sal/config.h>

#include <officecfg/Office/Common.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/svapp.hxx>
#include <svtools/fontsubstconfig.hxx>
#include "fontsubs.hxx"
#include <helpids.h>

/*********************************************************************/
/*                                                                   */
/*  TabPage font replacement                                         */
/*                                                                   */
/*********************************************************************/

SvxFontSubstTabPage::SvxFontSubstTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optfontspage.ui"_ustr, u"OptFontsPage"_ustr, &rSet)
    , m_xUseTableCB(m_xBuilder->weld_check_button(u"usetable"_ustr))
    , m_xUseTableImg(m_xBuilder->weld_widget(u"lockusetable"_ustr))
    , m_xFont1CB(m_xBuilder->weld_combo_box(u"font1"_ustr))
    , m_xFont2CB(m_xBuilder->weld_combo_box(u"font2"_ustr))
    , m_xApply(m_xBuilder->weld_button(u"apply"_ustr))
    , m_xDelete(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xCheckLB(m_xBuilder->weld_tree_view(u"checklb"_ustr))
    , m_xFontNameLB(m_xBuilder->weld_combo_box(u"fontname"_ustr))
    , m_xFontNameLabel(m_xBuilder->weld_label(u"label8"_ustr))
    , m_xFontNameImg(m_xBuilder->weld_widget(u"lockfontname"_ustr))
    , m_xNonPropFontsOnlyCB(m_xBuilder->weld_check_button(u"nonpropfontonly"_ustr))
    , m_xNonPropFontsOnlyImg(m_xBuilder->weld_widget(u"locknonpropfontonly"_ustr))
    , m_xFontHeightLB(m_xBuilder->weld_combo_box(u"fontheight"_ustr))
    , m_xFontHeightLabel(m_xBuilder->weld_label(u"label9"_ustr))
    , m_xFontHeightImg(m_xBuilder->weld_widget(u"lockfontheight"_ustr))
{
    m_xFont1CB->make_sorted();
    m_xFont1CB->set_size_request(1, -1);
    m_xFont2CB->make_sorted();
    m_xFont2CB->set_size_request(1, -1);
    m_sAutomatic = m_xFontNameLB->get_text(0);
    assert(!m_sAutomatic.isEmpty());

    m_xCheckLB->set_size_request(m_xCheckLB->get_approximate_digit_width() * 60,
                                 m_xCheckLB->get_height_rows(8));
    m_xCheckLB->enable_toggle_buttons(weld::ColumnToggleType::Check);
    m_xCheckLB->set_help_id(HID_OFA_FONT_SUBST_CLB);
    m_xCheckLB->set_selection_mode(SelectionMode::Multiple);

    setColSizes(m_xCheckLB->get_size_request());
    m_xCheckLB->connect_size_allocate(LINK(this, SvxFontSubstTabPage, ResizeHdl));

    m_xCheckLB->set_centered_column(0);
    m_xCheckLB->set_centered_column(1);

    Link<weld::ComboBox&,void> aLink2(LINK(this, SvxFontSubstTabPage, SelectComboBoxHdl));
    Link<weld::Button&,void> aClickLink(LINK(this, SvxFontSubstTabPage, ClickHdl));

    m_xCheckLB->connect_changed(LINK(this, SvxFontSubstTabPage, TreeListBoxSelectHdl));
    m_xCheckLB->connect_column_clicked(LINK(this, SvxFontSubstTabPage, HeaderBarClick));
    m_xUseTableCB->connect_toggled(LINK(this, SvxFontSubstTabPage, ToggleHdl));
    m_xFont1CB->connect_changed(aLink2);
    m_xFont2CB->connect_changed(aLink2);
    m_xApply->connect_clicked(aClickLink);
    m_xDelete->connect_clicked(aClickLink);

    m_xNonPropFontsOnlyCB->connect_toggled(LINK(this, SvxFontSubstTabPage, NonPropFontsHdl));

    sal_uInt16 nHeight;
    for(nHeight = 6; nHeight <= 16; nHeight++)
        m_xFontHeightLB->append_text(OUString::number(nHeight));
    for(nHeight = 18; nHeight <= 28; nHeight+= 2)
        m_xFontHeightLB->append_text(OUString::number(nHeight));
    for(nHeight = 32; nHeight <= 48; nHeight+= 4)
        m_xFontHeightLB->append_text(OUString::number(nHeight));
    for(nHeight = 54; nHeight <= 72; nHeight+= 6)
        m_xFontHeightLB->append_text(OUString::number(nHeight));
    for(nHeight = 80; nHeight <= 96; nHeight+= 8)
        m_xFontHeightLB->append_text(OUString::number(nHeight));
}

IMPL_LINK(SvxFontSubstTabPage, HeaderBarClick, int, nColumn, void)
{
    bool bSortAtoZ = m_xCheckLB->get_sort_order();

    //set new arrow positions in headerbar
    if (nColumn == m_xCheckLB->get_sort_column())
    {
        bSortAtoZ = !bSortAtoZ;
        m_xCheckLB->set_sort_order(bSortAtoZ);
    }
    else
    {
        m_xCheckLB->set_sort_indicator(TRISTATE_INDET, m_xCheckLB->get_sort_column());
        m_xCheckLB->set_sort_column(nColumn);
    }

    if (nColumn != -1)
    {
        //sort lists
        m_xCheckLB->set_sort_indicator(bSortAtoZ ? TRISTATE_TRUE : TRISTATE_FALSE, nColumn);
    }
}

void SvxFontSubstTabPage::setColSizes(const Size& rSize)
{
    int nW1 = m_xCheckLB->get_pixel_size(m_xCheckLB->get_column_title(2)).Width();
    int nW2 = m_xCheckLB->get_pixel_size(m_xCheckLB->get_column_title(3)).Width();
    int nMax = std::max( nW1, nW2 ) + 6; // width of the longest header + a little offset
    int nMin = m_xCheckLB->get_checkbox_column_width();
    nMax = std::max(nMax, nMin);
    const int nDoubleMax = 2*nMax;
    const int nRest = rSize.Width() - nDoubleMax;
    std::vector<int> aWidths { nMax, nMax, nRest/2 };
    m_xCheckLB->set_column_fixed_widths(aWidths);
}

IMPL_LINK(SvxFontSubstTabPage, ResizeHdl, const Size&, rSize, void)
{
    setColSizes(rSize);
}

SvxFontSubstTabPage::~SvxFontSubstTabPage()
{
}

std::unique_ptr<SfxTabPage> SvxFontSubstTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                                const SfxItemSet* rAttrSet)
{
    return std::make_unique<SvxFontSubstTabPage>(pPage, pController, *rAttrSet);
}

OUString SvxFontSubstTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label4"_ustr, u"label2"_ustr, u"label3"_ustr, u"label1"_ustr, u"label8"_ustr, u"label9"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"usetable"_ustr, u"nonpropfontonly"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool  SvxFontSubstTabPage::FillItemSet( SfxItemSet* )
{
    std::vector<SubstitutionStruct> aNewFontSubs;

    m_xCheckLB->all_foreach([this, &aNewFontSubs](weld::TreeIter& rIter) {
        SubstitutionStruct aAdd;
        aAdd.sFont = m_xCheckLB->get_text(rIter, 2);
        aAdd.sReplaceBy = m_xCheckLB->get_text(rIter, 3);
        aAdd.bReplaceAlways = m_xCheckLB->get_toggle(rIter, 0);
        aAdd.bReplaceOnScreenOnly = m_xCheckLB->get_toggle(rIter, 1);
        aNewFontSubs.push_back(aAdd);
        return false;
    });

    svtools::SetFontSubstitutions(m_xUseTableCB->get_active(), aNewFontSubs);
    svtools::ApplyFontSubstitutionsToVcl();

    std::shared_ptr< comphelper::ConfigurationChanges > batch(
        comphelper::ConfigurationChanges::create());
    if (m_xFontHeightLB->get_value_changed_from_saved())
        officecfg::Office::Common::Font::SourceViewFont::FontHeight::set(
            static_cast< sal_Int16 >(m_xFontHeightLB->get_active_text().toInt32()),
            batch);
    if (m_xNonPropFontsOnlyCB->get_state_changed_from_saved())
        officecfg::Office::Common::Font::SourceViewFont::
            NonProportionalFontsOnly::set(
                m_xNonPropFontsOnlyCB->get_active(), batch);
    //font name changes cannot be detected by saved values
    OUString sFontName;
    if (m_xFontNameLB->get_active() != -1)
        sFontName = m_xFontNameLB->get_active_text();
    officecfg::Office::Common::Font::SourceViewFont::FontName::set(
        std::optional< OUString >(sFontName), batch);
    batch->commit();

    return false;
}

void  SvxFontSubstTabPage::Reset( const SfxItemSet* )
{
    m_xCheckLB->freeze();
    m_xCheckLB->clear();

    m_xFont1CB->freeze();
    m_xFont1CB->clear();
    m_xFont2CB->freeze();
    m_xFont2CB->clear();

    FontList aFntLst(Application::GetDefaultDevice());
    sal_uInt16 nFontCount = aFntLst.GetFontNameCount();
    for (sal_uInt16 i = 0; i < nFontCount; ++i)
    {
        const FontMetric& rFontMetric = aFntLst.GetFontName(i);
        m_xFont1CB->append_text(rFontMetric.GetFamilyName());
        m_xFont2CB->append_text(rFontMetric.GetFamilyName());
    }

    m_xFont2CB->thaw();
    m_xFont1CB->thaw();

    bool bEnable = !officecfg::Office::Common::Font::Substitution::Replacement::isReadOnly();
    m_xUseTableCB->set_active(svtools::IsFontSubstitutionsEnabled());
    m_xUseTableCB->set_sensitive(bEnable);
    m_xUseTableImg->set_visible(!bEnable);

    std::vector<SubstitutionStruct> aFontSubs = svtools::GetFontSubstitutions();
    std::unique_ptr<weld::TreeIter> xIter(m_xCheckLB->make_iterator());
    for (auto const & sub: aFontSubs)
    {
        m_xCheckLB->append(xIter.get());
        m_xCheckLB->set_toggle(*xIter, sub.bReplaceAlways ? TRISTATE_TRUE : TRISTATE_FALSE, 0);
        m_xCheckLB->set_toggle(*xIter, sub.bReplaceOnScreenOnly ? TRISTATE_TRUE : TRISTATE_FALSE, 1);
        m_xCheckLB->set_text(*xIter, sub.sFont, 2);
        m_xCheckLB->set_text(*xIter, sub.sReplaceBy, 3);
    }

    m_xCheckLB->thaw();

    m_xCheckLB->make_sorted();
    m_xCheckLB->set_sort_column(2);
    m_xCheckLB->set_sort_indicator(TRISTATE_TRUE, 2);

    SelectHdl(m_xFont1CB.get());

    //fill font name box first
    m_xNonPropFontsOnlyCB->set_active(
        officecfg::Office::Common::Font::SourceViewFont::
        NonProportionalFontsOnly::get());
    NonPropFontsHdl(*m_xNonPropFontsOnlyCB);
    OUString sFontName(
        officecfg::Office::Common::Font::SourceViewFont::FontName::get().
        value_or(OUString()));
    if(!sFontName.isEmpty())
        m_xFontNameLB->set_active_text(sFontName);
    else
        m_xFontNameLB->set_active(0);
    m_xFontHeightLB->set_active_text(
        OUString::number(
            officecfg::Office::Common::Font::SourceViewFont::FontHeight::
            get()));

    bEnable = !officecfg::Office::Common::Font::SourceViewFont::FontName::isReadOnly();
    m_xFontNameLB->set_sensitive(bEnable);
    m_xFontNameLabel->set_sensitive(bEnable);
    m_xFontNameImg->set_visible(!bEnable);

    m_xNonPropFontsOnlyCB->set_sensitive(bEnable);
    m_xNonPropFontsOnlyImg->set_visible(!bEnable);

    m_xFontHeightLB->set_sensitive(bEnable);
    m_xFontHeightLabel->set_sensitive(bEnable);
    m_xFontHeightImg->set_visible(!bEnable);

    m_xNonPropFontsOnlyCB->save_state();
    m_xFontHeightLB->save_value();
}

IMPL_LINK(SvxFontSubstTabPage, ToggleHdl, weld::Toggleable&, rButton, void)
{
    SelectHdl(&rButton);
}

IMPL_LINK(SvxFontSubstTabPage, ClickHdl, weld::Button&, rButton, void)
{
    SelectHdl(&rButton);
}

IMPL_LINK(SvxFontSubstTabPage, TreeListBoxSelectHdl, weld::TreeView&, rButton, void)
{
    SelectHdl(&rButton);
}

IMPL_LINK(SvxFontSubstTabPage, SelectComboBoxHdl, weld::ComboBox&, rBox, void)
{
    SelectHdl(&rBox);
}

namespace
{
    // search in the "font" column
    int findText(const weld::TreeView& rTreeView, std::u16string_view rCol)
    {
        for (int i = 0, nEntryCount = rTreeView.n_children(); i < nEntryCount; ++i)
        {
            if (rTreeView.get_text(i, 2) == rCol)
                return i;
        }
        return -1;
    }

    bool findRow(const weld::TreeView& rTreeView, std::u16string_view rCol1, std::u16string_view rCol2)
    {
        int nRow = findText(rTreeView, rCol1);
        if (nRow == -1)
            return false;
        return rTreeView.get_text(nRow, 3) == rCol2;
    }
}

void SvxFontSubstTabPage::SelectHdl(const weld::Widget* pWin)
{
    if (pWin == m_xApply.get() || pWin == m_xDelete.get())
    {
        int nPos = findText(*m_xCheckLB, m_xFont1CB->get_active_text());
        if (pWin == m_xApply.get())
        {
            m_xCheckLB->unselect_all();
            if (nPos != -1)
            {
                // change entry
                m_xCheckLB->set_text(nPos, m_xFont2CB->get_active_text(), 3);
                m_xCheckLB->select(nPos);
            }
            else
            {
                // new entry
                OUString sFont1 = m_xFont1CB->get_active_text();
                OUString sFont2 = m_xFont2CB->get_active_text();

                std::unique_ptr<weld::TreeIter> xIter(m_xCheckLB->make_iterator());
                m_xCheckLB->append(xIter.get());
                m_xCheckLB->set_toggle(*xIter, TRISTATE_FALSE, 0);
                m_xCheckLB->set_toggle(*xIter, TRISTATE_FALSE, 1);
                m_xCheckLB->set_text(*xIter, sFont1, 2);
                m_xCheckLB->set_text(*xIter, sFont2, 3);
                m_xCheckLB->select(*xIter);
            }
        }
        else if (pWin == m_xDelete.get())
        {
            m_xCheckLB->remove_selection();
        }
    }

    if (pWin == m_xCheckLB.get())
    {
        const int nSelectedRowCount = m_xCheckLB->count_selected_rows();
        if (nSelectedRowCount == 1)
        {
            int nRow = m_xCheckLB->get_selected_index();
            m_xFont1CB->set_entry_text(m_xCheckLB->get_text(nRow, 2));
            m_xFont2CB->set_entry_text(m_xCheckLB->get_text(nRow, 3));
        }
        else if (nSelectedRowCount > 1)
        {
            m_xFont1CB->set_entry_text(OUString());
            m_xFont2CB->set_entry_text(OUString());
        }
    }

    if (pWin == m_xFont1CB.get())
    {
        int nPos = findText(*m_xCheckLB, m_xFont1CB->get_active_text());

        if (nPos != -1)
        {
            int nSelectedRow = m_xCheckLB->get_selected_index();
            if (nPos != nSelectedRow)
            {
                m_xCheckLB->unselect_all();
                m_xCheckLB->select(nPos);
            }
        }
        else
            m_xCheckLB->unselect_all();
    }

    CheckEnable();
}

IMPL_LINK(SvxFontSubstTabPage, NonPropFontsHdl, weld::Toggleable&, rBox, void)
{
    OUString sFontName = m_xFontNameLB->get_active_text();
    bool bNonPropOnly = rBox.get_active();
    m_xFontNameLB->clear();
    FontList aFntLst( Application::GetDefaultDevice() );
    m_xFontNameLB->append_text(m_sAutomatic);
    sal_uInt16 nFontCount = aFntLst.GetFontNameCount();
    for(sal_uInt16 nFont = 0; nFont < nFontCount; nFont++)
    {
        const FontMetric& rFontMetric = aFntLst.GetFontName( nFont );
        if(!bNonPropOnly || rFontMetric.GetPitch() == PITCH_FIXED)
            m_xFontNameLB->append_text(rFontMetric.GetFamilyName());
    }
    m_xFontNameLB->set_active_text(sFontName);
}

void SvxFontSubstTabPage::CheckEnable()
{
    bool bEnableAll = m_xUseTableCB->get_active() && !officecfg::Office::Common::Font::SourceViewFont::FontName::isReadOnly();
    m_xCheckLB->set_sensitive(bEnableAll);
    m_xFont1CB->set_sensitive(bEnableAll);
    m_xFont2CB->set_sensitive(bEnableAll);

    bool bApply = bEnableAll, bDelete = bEnableAll;

    if (bEnableAll)
    {
        int nEntry = m_xCheckLB->get_selected_index();

        if (m_xFont1CB->get_active_text().isEmpty() || m_xFont2CB->get_active_text().isEmpty())
            bApply = false;
        else if (m_xFont1CB->get_active_text() == m_xFont2CB->get_active_text())
            bApply = false;
        else if (findRow(*m_xCheckLB, m_xFont1CB->get_active_text(), m_xFont2CB->get_active_text()))
            bApply = false;
        else if (nEntry != -1 && m_xCheckLB->count_selected_rows() != 1)
            bApply = false;
        else
            bApply = true;

        bDelete = nEntry != -1;
    }

    m_xApply->set_sensitive(bApply);
    m_xDelete->set_sensitive(bDelete);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
