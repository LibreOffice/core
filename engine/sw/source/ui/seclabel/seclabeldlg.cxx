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
    , m_xEditBox(m_xBuilder->weld_widget(u"editbox"_ustr))
    , m_xPolicy(m_xBuilder->weld_combo_box(u"policy"_ustr))
    , m_xClassification(m_xBuilder->weld_combo_box(u"classification"_ustr))
    , m_xCategories(m_xBuilder->weld_tree_view(u"categories"_ustr))
    , m_xPreview(m_xBuilder->weld_label(u"preview"_ustr))
    , m_xWarning(m_xBuilder->weld_label(u"seclabelwarning"_ustr))
    , m_xOkBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xRelabelBtn(m_xBuilder->weld_button(u"relabel"_ustr))
{
    m_xCategories->set_size_request(m_xCategories->get_approximate_digit_width() * 32,
                                    m_xCategories->get_height_rows(6));
    m_xCategories->enable_toggle_buttons(weld::ColumnToggleType::Check);

    m_aPolicySet.loadFile(getDevPolicyUrl());

    PopulatePolicies();
    initFromExistingLabel();

    m_xPolicy->connect_changed(LINK(this, SwSecurityLabelDlg, PolicyHdl));
    m_xClassification->connect_changed(LINK(this, SwSecurityLabelDlg, ClassificationHdl));
    m_xCategories->connect_toggled(LINK(this, SwSecurityLabelDlg, CategoryToggleHdl));
    m_xOkBtn->connect_clicked(LINK(this, SwSecurityLabelDlg, OkHdl));
    m_xRelabelBtn->connect_clicked(LINK(this, SwSecurityLabelDlg, RelabelHdl));

    m_xWarning->set_label_type(weld::LabelType::Warning);

    UpdatePreview();
}

SwSecurityLabelDlg::~SwSecurityLabelDlg() {}

void SwSecurityLabelDlg::PopulatePolicies()
{
    m_xPolicy->clear();
    for (const auto& rPolicy : m_aPolicySet.aPolicies)
        m_xPolicy->append_text(rPolicy.aName);

    if (m_aPolicySet.empty())
    {
        // Nothing to label with: hide the editor and Apply, leaving only the notice.
        m_pPolicy = nullptr;
        m_xWarning->set_label(SwResId(STR_SECLABEL_NOPOLICY));
        m_xWarning->set_visible(true);
        m_xEditBox->set_visible(false);
        m_xOkBtn->set_visible(false);
        return;
    }

    m_xPolicy->set_active(0);
    setActivePolicy(0);
}

void SwSecurityLabelDlg::setActivePolicy(int nIndex)
{
    if (nIndex < 0 || nIndex >= static_cast<int>(m_aPolicySet.aPolicies.size()))
    {
        m_pPolicy = nullptr;
        return;
    }
    m_pPolicy = &m_aPolicySet.aPolicies[nIndex];
    PopulateClassifications();
    PopulateCategories();
}

void SwSecurityLabelDlg::PopulateClassifications()
{
    m_xClassification->clear();
    if (!m_pPolicy)
        return;

    // Obsolete values are hidden for new labels; initFromExistingLabel re-adds one
    // when editing an existing label that uses it.
    for (const auto& rClass : m_pPolicy->aClassifications)
    {
        if (!rClass.bObsolete)
            m_xClassification->append_text(rClass.aName);
    }
    if (m_xClassification->get_count())
        m_xClassification->set_active(0);
}

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
    if (!m_pPolicy)
        return;

    auto xIter = m_xCategories->make_iterator();
    sal_Int32 nTag = 0;
    for (const auto& rTagSet : m_pPolicy->aTagSets)
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

void SwSecurityLabelDlg::initFromExistingLabel()
{
    SwDocShell* pDocShell = m_rSh.GetDoc()->GetDocShell();
    if (!pDocShell)
        return;
    uno::Reference<frame::XModel> xModel(pDocShell->GetModel());
    if (!xModel.is())
        return;

    sw::seclabel::StanagLabel aLabel;
    if (!sw::seclabel::readLabel(xModel, aLabel))
        return;

    // A label written under a policy we don't have can't be edited structurally;
    // show it read-only and offer re-labeling under an available policy.
    const sw::seclabel::SpifPolicy* pMatch = m_aPolicySet.findByLabel(aLabel);
    if (!pMatch)
    {
        enterForeignMode(aLabel);
        return;
    }

    // Make the matching policy active in the selector and rebuild the editor for it.
    const int nIndex = static_cast<int>(pMatch - m_aPolicySet.aPolicies.data());
    m_xPolicy->set_active(nIndex);
    setActivePolicy(nIndex);

    // Select the stored classification. An obsolete value is hidden for new
    // labels but must render when editing one that uses it, so append it.
    int nPos = m_xClassification->find_text(aLabel.aClassification);
    if (nPos == -1)
    {
        m_xClassification->append_text(aLabel.aClassification);
        nPos = m_xClassification->find_text(aLabel.aClassification);
    }
    if (nPos != -1)
        m_xClassification->set_active(nPos);

    PopulateCategories();

    // Check the rows whose category name appears among the label's values.
    std::set<OUString> aValues;
    for (const auto& rCategory : aLabel.aCategories)
    {
        for (const auto& rValue : rCategory.aValues)
            aValues.insert(rValue);
    }
    const int nCount = m_xCategories->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        if (aValues.count(m_xCategories->get_text(i, 0)))
            m_xCategories->set_toggle(i, TRISTATE_TRUE);
    }
}

void SwSecurityLabelDlg::enterForeignMode(const sw::seclabel::StanagLabel& rLabel)
{
    m_bForeignPolicy = true;

    // The 4774 label is self-describing; reconstruct a readable summary from it,
    // since the policy that defines its exact marking is not available here.
    OUString sSummary = rLabel.aClassification;
    for (const auto& rCategory : rLabel.aCategories)
    {
        for (const auto& rValue : rCategory.aValues)
            sSummary += u" " + rValue;
    }
    m_xPreview->set_label(sSummary);

    const OUString sPolicy = rLabel.aPolicyName.isEmpty() ? rLabel.aPolicyId : rLabel.aPolicyName;
    m_xWarning->set_label(SwResId(STR_SECLABEL_FOREIGN).replaceFirst(u"%1", sPolicy));
    m_xWarning->set_visible(true);

    // View-only: hide the (provisioned-policy) editor and Apply. Offer Re-label only
    // if there is a provisioned policy to re-label under.
    m_xEditBox->set_visible(false);
    m_xOkBtn->set_visible(false);
    m_xRelabelBtn->set_visible(!m_aPolicySet.empty());
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
    if (m_bForeignPolicy || !m_pPolicy)
        return; // the foreign/no-policy view owns the preview and notice

    const OUString sClassification = m_xClassification->get_active_text();
    const std::vector<bool> aSelected = collectSelection();

    m_xPreview->set_label(m_pPolicy->buildMarking(sClassification, aSelected));

    OUString sWarning;
    for (const auto& rViolation : m_pPolicy->validate(sClassification, aSelected))
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
        = m_pPolicy->buildLabel(rClassification, rSelected, utl::toISO8601(aNow.GetUNODateTime()),
                                utl::toISO8601(aReview.GetUNODateTime()));
    const OUString sItemProps = sw::seclabel::buildItemProps(
        makeGuid(), u"urn:nato:stanag:4778:bindinginformation:1:0"_ustr);
    sw::seclabel::storeLabelPart(xModel, aLabel.toBindingXml(), sItemProps);

    sal_Int32 nColor = 0;
    for (const auto& rClass : m_pPolicy->aClassifications)
    {
        if (rClass.aName == rClassification)
        {
            nColor = sw::seclabel::resolveColor(rClass.aColor);
            break;
        }
    }
    sw::seclabel::applyMarking(xModel, m_pPolicy->buildMarking(rClassification, rSelected), nColor,
                               getCurrentPageStyle(xModel));
}

IMPL_LINK_NOARG(SwSecurityLabelDlg, PolicyHdl, weld::ComboBox&, void)
{
    setActivePolicy(m_xPolicy->get_active());
    UpdatePreview();
}

IMPL_LINK_NOARG(SwSecurityLabelDlg, OkHdl, weld::Button&, void)
{
    if (m_bForeignPolicy || !m_pPolicy)
        return; // read-only foreign/no-policy view; nothing to apply
    const OUString sClassification = m_xClassification->get_active_text();
    const std::vector<bool> aSelected = collectSelection();
    if (!m_pPolicy->validate(sClassification, aSelected).empty())
        return; // the warning label already shows why; keep the dialog open
    applyLabel(sClassification, aSelected);
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SwSecurityLabelDlg, RelabelHdl, weld::Button&, void)
{
    // Leave the read-only foreign view and start a fresh label under the policy
    // selected in the selector: re-enable the editor, drop the notice, restore Apply.
    m_bForeignPolicy = false;
    m_xRelabelBtn->set_visible(false);
    m_xEditBox->set_visible(true);
    m_xOkBtn->set_visible(true);
    setActivePolicy(m_xPolicy->get_active());
    UpdatePreview();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
