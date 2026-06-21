/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <seclabeldlg.hxx>

#include <SecLabelApply.hxx>
#include <StanagLabel.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <strings.hrc>
#include <swtypes.hxx>
#include <wrtsh.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/uuid.h>
#include <tools/datetime.hxx>
#include <tools/stream.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>
#include <unotools/datetime.hxx>

#include <set>

using namespace css;

namespace
{
// TODO dev stopgap: fixed policy path. Replaced by WOPI provisioning (Phase F).
// The file sits beside the sample TSCP policies in the installation.
OUString getDevPolicyUrl()
{
    OUString sUrl(u"$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER
                  "/classification/spif-collabora.xml"_ustr);
    rtl::Bootstrap::expandMacros(sUrl);
    return sUrl;
}

OUString formatViolation(const sw::seclabel::SpifViolation& rViolation)
{
    using T = sw::seclabel::SpifViolationType;
    switch (rViolation.eType)
    {
        case T::MinSelection:
            return SwResId(STR_SECLABEL_MIN)
                .replaceFirst(u"%1", OUString::number(rViolation.nMinSelection))
                .replaceFirst(u"%2", rViolation.aName);
        case T::MaxSelection:
            return SwResId(STR_SECLABEL_MAX)
                .replaceFirst(u"%1", OUString::number(rViolation.nMaxSelection))
                .replaceFirst(u"%2", rViolation.aName);
        case T::ExcludedCategory:
            return SwResId(STR_SECLABEL_EXCLUDED).replaceFirst(u"%1", rViolation.aName);
        case T::RequiredCategory:
            return SwResId(STR_SECLABEL_REQUIRED).replaceFirst(u"%1", rViolation.aName);
    }
    return OUString();
}

// Random itemID for the customXml part: {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}.
OUString makeGuid()
{
    sal_uInt8 aId[16];
    rtl_createUuid(aId, nullptr, false);
    OUStringBuffer aBuf(u"{");
    for (int i = 0; i < 16; ++i)
    {
        if (i == 4 || i == 6 || i == 8 || i == 10)
            aBuf.append(u"-");
        const OUString aByte = OUString::number(aId[i], 16).toAsciiUpperCase();
        if (aByte.getLength() == 1)
            aBuf.append(u"0");
        aBuf.append(aByte);
    }
    aBuf.append(u"}");
    return aBuf.makeStringAndClear();
}

// The page style of the current view cursor (consistent with PageStyles getByName).
OUString getCurrentPageStyle(const uno::Reference<frame::XModel>& xModel)
{
    OUString sName(u"Standard"_ustr);
    uno::Reference<text::XTextViewCursorSupplier> xSupplier(xModel->getCurrentController(),
                                                            uno::UNO_QUERY);
    if (xSupplier.is())
    {
        uno::Reference<beans::XPropertySet> xProps(xSupplier->getViewCursor(), uno::UNO_QUERY);
        if (xProps.is())
            xProps->getPropertyValue(u"PageStyleName"_ustr) >>= sName;
    }
    return sName;
}
}

SwSecurityLabelDlg::SwSecurityLabelDlg(weld::Window* pParent, SwWrtShell& rSh)
    : GenericDialogController(pParent, u"modules/swriter/ui/seclabeldialog.ui"_ustr,
                              u"SecurityLabelDialog"_ustr)
    , m_rSh(rSh)
    , m_xClassification(m_xBuilder->weld_combo_box(u"classification"_ustr))
    , m_xCategories(m_xBuilder->weld_tree_view(u"categories"_ustr))
    , m_xPreview(m_xBuilder->weld_label(u"preview"_ustr))
    , m_xWarning(m_xBuilder->weld_label(u"seclabelwarning"_ustr))
    , m_xOkBtn(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xCategories->set_size_request(m_xCategories->get_approximate_digit_width() * 32,
                                    m_xCategories->get_height_rows(6));

    SvFileStream aStream(getDevPolicyUrl(), StreamMode::READ);
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
    m_xOkBtn->connect_clicked(LINK(this, SwSecurityLabelDlg, OkHdl));

    m_xWarning->set_label_type(weld::LabelType::Warning);

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

std::vector<bool> SwSecurityLabelDlg::collectSelection() const
{
    const int nCount = m_xCategories->n_children();
    std::vector<bool> aSelected(nCount);
    for (int i = 0; i < nCount; ++i)
        aSelected[i] = m_xCategories->get_toggle(i) == TRISTATE_TRUE;
    return aSelected;
}

void SwSecurityLabelDlg::UpdatePreview()
{
    const OUString sClassification = m_xClassification->get_active_text();
    const std::vector<bool> aSelected = collectSelection();

    m_xPreview->set_label(m_aPolicy.buildMarking(sClassification, aSelected));

    OUString sWarning;
    for (const auto& rViolation : m_aPolicy.validate(sClassification, aSelected))
    {
        if (!sWarning.isEmpty())
            sWarning += u"\n";
        sWarning += formatViolation(rViolation);
    }
    m_xWarning->set_label(sWarning);
    m_xWarning->set_visible(!sWarning.isEmpty());
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

void SwSecurityLabelDlg::applyLabel(const OUString& rClassification,
                                   const std::vector<bool>& rSelected)
{
    SwDocShell* pDocShell = m_rSh.GetDoc()->GetDocShell();
    if (!pDocShell)
        return;
    uno::Reference<frame::XModel> xModel(pDocShell->GetModel());
    if (!xModel.is())
        return;

    const DateTime aNow(DateTime::SYSTEM);
    DateTime aReview(aNow);
    aReview.AddYears(1);

    const sw::seclabel::StanagLabel aLabel
        = m_aPolicy.buildLabel(rClassification, rSelected, utl::toISO8601(aNow.GetUNODateTime()),
                               utl::toISO8601(aReview.GetUNODateTime()));
    const OUString sItemProps = sw::seclabel::buildItemProps(
        makeGuid(), u"urn:nato:stanag:4778:bindinginformation:1:0"_ustr);
    sw::seclabel::storeLabelPart(xModel, aLabel.toBindingXml(), sItemProps);

    sal_Int32 nColor = 0;
    for (const auto& rClass : m_aPolicy.aClassifications)
    {
        if (rClass.aName == rClassification)
        {
            nColor = sw::seclabel::resolveColor(rClass.aColor);
            break;
        }
    }
    sw::seclabel::applyMarking(xModel, m_aPolicy.buildMarking(rClassification, rSelected), nColor,
                               getCurrentPageStyle(xModel));
}

IMPL_LINK_NOARG(SwSecurityLabelDlg, OkHdl, weld::Button&, void)
{
    const OUString sClassification = m_xClassification->get_active_text();
    const std::vector<bool> aSelected = collectSelection();
    if (!m_aPolicy.validate(sClassification, aSelected).empty())
        return; // the warning label already shows why; keep the dialog open
    applyLabel(sClassification, aSelected);
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
