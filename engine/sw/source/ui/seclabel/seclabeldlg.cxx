/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <seclabeldlg.hxx>

#include <tools/stream.hxx>

namespace
{
// TODO dev stopgap: fixed policy path. Replaced by WOPI provisioning (Phase F).
constexpr OUString gsDevPolicyUrl = u"file:///etc/spif-collabora.xml"_ustr;
}

SwSecurityLabelDlg::SwSecurityLabelDlg(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/swriter/ui/seclabeldialog.ui"_ustr,
                              u"SecurityLabelDialog"_ustr)
    , m_xClassification(m_xBuilder->weld_combo_box(u"classification"_ustr))
    , m_xCategories(m_xBuilder->weld_tree_view(u"categories"_ustr))
    , m_xPreview(m_xBuilder->weld_label(u"preview"_ustr))
{
    m_xCategories->set_size_request(m_xCategories->get_approximate_digit_width() * 32,
                                    m_xCategories->get_height_rows(6));

    SvFileStream aStream(gsDevPolicyUrl, StreamMode::READ);
    if (aStream.IsOpen())
        m_aPolicy.parse(aStream);

    for (const auto& rClass : m_aPolicy.aClassifications)
        m_xClassification->append_text(rClass.aName);
    if (m_xClassification->get_count())
        m_xClassification->set_active(0);

    m_xCategories->enable_toggle_buttons(weld::ColumnToggleType::Check);
    auto xIter = m_xCategories->make_iterator();
    sal_Int32 nTag = 0;
    for (const auto& rTagSet : m_aPolicy.aTagSets)
    {
        for (const auto& rTag : rTagSet.aTags)
        {
            m_aTagSingle.push_back(rTag.bSingleSelection);
            for (const auto& rCategory : rTag.aCategories)
            {
                m_xCategories->append(xIter.get());
                m_xCategories->set_toggle(*xIter, TRISTATE_FALSE);
                m_xCategories->set_text(*xIter, rCategory.aName, 0);
                m_aRowTag.push_back(nTag);
            }
            ++nTag;
        }
    }

    m_xClassification->connect_changed(LINK(this, SwSecurityLabelDlg, ClassificationHdl));
    m_xCategories->connect_toggled(LINK(this, SwSecurityLabelDlg, CategoryToggleHdl));

    UpdatePreview();
}

SwSecurityLabelDlg::~SwSecurityLabelDlg() {}

void SwSecurityLabelDlg::UpdatePreview()
{
    // Stub marking string; the real layered derivation comes with marking rules.
    OUString sMarking = m_xClassification->get_active_text();
    OUString sCategories;
    const int nCount = m_xCategories->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        if (m_xCategories->get_toggle(i) == TRISTATE_TRUE)
        {
            if (!sCategories.isEmpty())
                sCategories += u" "_ustr;
            sCategories += m_xCategories->get_text(i, 0);
        }
    }
    if (!sCategories.isEmpty())
        sMarking += u"//"_ustr + sCategories + u"."_ustr;
    m_xPreview->set_label(sMarking);
}

IMPL_LINK_NOARG(SwSecurityLabelDlg, ClassificationHdl, weld::ComboBox&, void) { UpdatePreview(); }

IMPL_LINK(SwSecurityLabelDlg, CategoryToggleHdl, const weld::TreeView::iter_col&, rIterCol, void)
{
    // Single-selection tags: toggling one category clears the others of that tag.
    const int nRow = m_xCategories->get_iter_index_in_parent(rIterCol.first);
    if (nRow >= 0 && nRow < static_cast<int>(m_aRowTag.size()))
    {
        const sal_Int32 nTag = m_aRowTag[nRow];
        if (m_aTagSingle[nTag] && m_xCategories->get_toggle(nRow) == TRISTATE_TRUE)
        {
            const int nCount = m_xCategories->n_children();
            for (int i = 0; i < nCount; ++i)
            {
                if (i != nRow && m_aRowTag[i] == nTag)
                    m_xCategories->set_toggle(i, TRISTATE_FALSE);
            }
        }
    }
    UpdatePreview();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
