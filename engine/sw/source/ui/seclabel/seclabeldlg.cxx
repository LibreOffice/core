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

#include <set>

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

    // Obsolete values are hidden for new labels (must still render when editing
    // an existing label that uses them — once label loading exists).
    for (const auto& rClass : m_aPolicy.aClassifications)
    {
        if (!rClass.bObsolete)
            m_xClassification->append_text(rClass.aName);
    }
    if (m_xClassification->get_count())
        m_xClassification->set_active(0);

    m_xCategories->enable_toggle_buttons(weld::ColumnToggleType::Check);
    PopulateCategories();

    m_xClassification->connect_changed(LINK(this, SwSecurityLabelDlg, ClassificationHdl));
    m_xCategories->connect_toggled(LINK(this, SwSecurityLabelDlg, CategoryToggleHdl));

    UpdatePreview();
}

SwSecurityLabelDlg::~SwSecurityLabelDlg() {}

void SwSecurityLabelDlg::PopulateCategories()
{
    // Rebuild the list, dropping categories the current classification excludes
    // while preserving the checks of those that survive. Identity is the owning
    // tag index plus the category name (stable across rebuilds).
    std::set<std::pair<sal_Int32, OUString>> aChecked;
    const int nOld = m_xCategories->n_children();
    for (int i = 0; i < nOld; ++i)
    {
        if (m_xCategories->get_toggle(i) == TRISTATE_TRUE)
            aChecked.emplace(m_aRowTag[i], m_xCategories->get_text(i, 0));
    }

    const OUString sClassification = m_xClassification->get_active_text();

    m_xCategories->clear();
    m_aRowTag.clear();
    m_aTagSingle.clear();

    auto xIter = m_xCategories->make_iterator();
    sal_Int32 nTag = 0;
    for (const auto& rTagSet : m_aPolicy.aTagSets)
    {
        for (const auto& rTag : rTagSet.aTags)
        {
            m_aTagSingle.push_back(rTag.bSingleSelection);
            for (const auto& rCategory : rTag.aCategories)
            {
                if (!rCategory.isSelectable(sClassification))
                    continue;
                m_xCategories->append(xIter.get());
                const bool bChecked = aChecked.count({ nTag, rCategory.aName }) != 0;
                m_xCategories->set_toggle(*xIter, bChecked ? TRISTATE_TRUE : TRISTATE_FALSE);
                m_xCategories->set_text(*xIter, rCategory.aName, 0);
                m_aRowTag.push_back(nTag);
            }
            ++nTag;
        }
    }
}

void SwSecurityLabelDlg::UpdatePreview()
{
    const int nCount = m_xCategories->n_children();
    std::vector<bool> aSelected(nCount);
    for (int i = 0; i < nCount; ++i)
        aSelected[i] = m_xCategories->get_toggle(i) == TRISTATE_TRUE;

    m_xPreview->set_label(m_aPolicy.buildMarking(m_xClassification->get_active_text(), aSelected));
}

IMPL_LINK_NOARG(SwSecurityLabelDlg, ClassificationHdl, weld::ComboBox&, void)
{
    PopulateCategories();
    UpdatePreview();
}

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
