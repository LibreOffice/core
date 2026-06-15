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
// Categories are still stubbed; tag-set parsing comes next.
const char* const aStubCategories[] = { "CANADA", "UNITED KINGDOM" };

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
    for (const char* pName : aStubCategories)
        m_xCategories->append_text(OUString::createFromAscii(pName));

    if (m_xClassification->get_count())
        m_xClassification->set_active(0);
    m_xClassification->connect_changed(LINK(this, SwSecurityLabelDlg, ClassificationHdl));
    m_xCategories->connect_selection_changed(LINK(this, SwSecurityLabelDlg, CategoryHdl));

    UpdatePreview();
}

SwSecurityLabelDlg::~SwSecurityLabelDlg() {}

void SwSecurityLabelDlg::UpdatePreview()
{
    // Stub marking string; the real layered derivation comes with the SPIF parser.
    OUString sMarking = m_xClassification->get_active_text();
    int nSel = m_xCategories->get_selected_index();
    if (nSel != -1)
        sMarking += u"//"_ustr + m_xCategories->get_text(nSel) + u"."_ustr;
    m_xPreview->set_label(sMarking);
}

IMPL_LINK_NOARG(SwSecurityLabelDlg, ClassificationHdl, weld::ComboBox&, void) { UpdatePreview(); }

IMPL_LINK_NOARG(SwSecurityLabelDlg, CategoryHdl, weld::TreeView&, void) { UpdatePreview(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
