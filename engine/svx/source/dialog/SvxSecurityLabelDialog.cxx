/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SvxSecurityLabelDialog.hxx>

#include <svx/dialmgr.hxx>
#include <svx/seclabel/SecLabelStore.hxx>
#include <svx/seclabel/SecurityLabelDialog.hxx>
#include <svx/seclabel/StanagLabel.hxx>
#include <svx/strings.hrc>

#include <com/sun/star/frame/XModel.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/uuid.h>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <vcl/abstdlgimpl.hxx>

#include <set>

using namespace css;

namespace
{
OUString formatViolation(const svx::seclabel::SpifViolation& rViolation)
{
    using T = svx::seclabel::SpifViolationType;
    switch (rViolation.eType)
    {
        case T::MinSelection:
            return SvxResId(RID_SVXSTR_SECLABEL_MIN)
                .replaceFirst(u"%1", OUString::number(rViolation.nMinSelection))
                .replaceFirst(u"%2", rViolation.aName);
        case T::MaxSelection:
            return SvxResId(RID_SVXSTR_SECLABEL_MAX)
                .replaceFirst(u"%1", OUString::number(rViolation.nMaxSelection))
                .replaceFirst(u"%2", rViolation.aName);
        case T::ExcludedCategory:
            return SvxResId(RID_SVXSTR_SECLABEL_EXCLUDED).replaceFirst(u"%1", rViolation.aName);
        case T::RequiredCategory:
            return SvxResId(RID_SVXSTR_SECLABEL_REQUIRED).replaceFirst(u"%1", rViolation.aName);
    }
    return OUString();
}

// A short note on how many categories the tag admits (empty when unconstrained),
// derived from singleSelection (a max of one) and min/maxSelection (-1 = unbounded).
// nSelectable is how many categories the group actually offers.
OUString selectionHint(const svx::seclabel::SpifCategoryTag& rTag, sal_Int32 nSelectable)
{
    const sal_Int32 nMin = rTag.nMinSelection < 0 ? 0 : rTag.nMinSelection;
    sal_Int32 nMax = rTag.bSingleSelection ? 1 : rTag.nMaxSelection;

    // A max that admits every category on offer rules nothing out.
    if (nMax >= nSelectable)
    {
        nMax = -1;
    }

    if (nMax < 0)
    {
        if (nMin <= 0)
            return OUString(); // unbounded both ways: no note
        return SvxResId(RID_SVXSTR_SECLABEL_HINT_ATLEAST).replaceFirst(u"%1", OUString::number(nMin));
    }
    if (nMin <= 0)
        return SvxResId(RID_SVXSTR_SECLABEL_HINT_ATMOST).replaceFirst(u"%1", OUString::number(nMax));
    if (nMin >= nMax)
        return SvxResId(RID_SVXSTR_SECLABEL_HINT_EXACTLY).replaceFirst(u"%1", OUString::number(nMax));
    return SvxResId(RID_SVXSTR_SECLABEL_HINT_RANGE)
        .replaceFirst(u"%1", OUString::number(nMin))
        .replaceFirst(u"%2", OUString::number(nMax));
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
}

SvxSecurityLabelDialog::SvxSecurityLabelDialog(
    weld::Window* pParent, std::unique_ptr<svx::seclabel::SecurityLabelTarget> pTarget)
    : GenericDialogController(pParent, u"svx/ui/seclabeldialog.ui"_ustr,
                              u"SecurityLabelDialog"_ustr)
    , m_pTarget(std::move(pTarget))
    , m_xEditBox(m_xBuilder->weld_widget(u"editbox"_ustr))
    , m_xPolicy(m_xBuilder->weld_combo_box(u"policy"_ustr))
    , m_xClassification(m_xBuilder->weld_combo_box(u"classification"_ustr))
    , m_xCategoriesWin(m_xBuilder->weld_scrolled_window(u"categorieswin"_ustr))
    , m_xPreview(m_xBuilder->weld_label(u"preview"_ustr))
    , m_xWarning(m_xBuilder->weld_label(u"seclabelwarning"_ustr))
    , m_xOkBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xRelabelBtn(m_xBuilder->weld_button(u"relabel"_ustr))
    , m_xRemoveBtn(m_xBuilder->weld_button(u"remove"_ustr))
{
    // Cap the category area's height so it (and only it) scrolls. This bounds the
    // desktop weld rendering; in online the dialog is laid out by the browser, where
    // the cap comes from jsdialogs.css (#SecurityLabelDialog #categorieswin) instead.
    m_xCategoriesWin->set_size_request(-1, m_xCategoriesWin->get_text_height() * 16);

    // Weld the static category pool (group boxes, header labels, checkboxes) and hide
    // it all; PopulateCategories shows and labels what the active policy needs.
    for (int i = 0; i < MAX_GROUPS; ++i)
    {
        m_xGroupBoxes.push_back(m_xBuilder->weld_widget("group" + OUString::number(i)));
        m_xGroupLabels.push_back(m_xBuilder->weld_label("group" + OUString::number(i) + "label"));
        m_xGroupHints.push_back(m_xBuilder->weld_label("group" + OUString::number(i) + "hint"));
        m_xGroupBoxes[i]->set_visible(false);

        std::vector<std::unique_ptr<weld::CheckButton>> aChecks;
        for (int j = 0; j < MAX_CATS; ++j)
        {
            auto xCheck = m_xBuilder->weld_check_button("g" + OUString::number(i) + "c"
                                                        + OUString::number(j));
            xCheck->set_visible(false);
            xCheck->connect_toggled(LINK(this, SvxSecurityLabelDialog, CategoryToggleHdl));
            aChecks.push_back(std::move(xCheck));
        }
        m_xChecks.push_back(std::move(aChecks));
    }

    // The provisioned policies (WOPI-preset spif/ dir, with the dev stopgap).
    m_aPolicySet.loadProvisioned();

    PopulatePolicies();
    initFromExistingLabel();

    // Removal is offered whenever the document already carries a label.
    m_xRemoveBtn->set_visible(m_bHasLabel);

    m_xPolicy->connect_changed(LINK(this, SvxSecurityLabelDialog, PolicyHdl));
    m_xClassification->connect_changed(LINK(this, SvxSecurityLabelDialog, ClassificationHdl));
    m_xOkBtn->connect_clicked(LINK(this, SvxSecurityLabelDialog, OkHdl));
    m_xRelabelBtn->connect_clicked(LINK(this, SvxSecurityLabelDialog, RelabelHdl));
    m_xRemoveBtn->connect_clicked(LINK(this, SvxSecurityLabelDialog, RemoveHdl));

    m_xWarning->set_label_type(weld::LabelType::Warning);

    // The marking preview reads as a banner, like the one the document gets, so it
    // stands out from the form below it. Online styles it from jsdialogs.css instead
    // (#SecurityLabelDialog #preview); neither the background nor the label type
    // reaches the browser.
    m_xPreview->set_title_background();
    m_xPreview->set_label_type(weld::LabelType::Title);

    UpdatePreview();
}

SvxSecurityLabelDialog::~SvxSecurityLabelDialog() {}

void SvxSecurityLabelDialog::PopulatePolicies()
{
    m_xPolicy->clear();
    for (const auto& rPolicy : m_aPolicySet.aPolicies)
        m_xPolicy->append_text(rPolicy.aName);

    if (m_aPolicySet.empty())
    {
        // Nothing to label with: hide the editor and Apply, leaving only the notice.
        m_pPolicy = nullptr;
        m_xWarning->set_label(SvxResId(RID_SVXSTR_SECLABEL_NOPOLICY));
        m_xWarning->set_visible(true);
        m_xEditBox->set_visible(false);
        m_xOkBtn->set_visible(false);
        return;
    }

    m_xPolicy->set_active(0);
    setActivePolicy(0);
}

void SvxSecurityLabelDialog::setActivePolicy(int nIndex)
{
    if (nIndex < 0 || nIndex >= static_cast<int>(m_aPolicySet.aPolicies.size()))
    {
        m_pPolicy = nullptr;
        return;
    }
    m_pPolicy = &m_aPolicySet.aPolicies[nIndex];
    PopulateClassifications();

    // Returning to the label's own policy restores its stored classification and
    // selections (the cross-policy check-preservation in PopulateCategories cannot,
    // since the other policy's categories never matched).
    if (m_bHasLabel && !m_bForeignPolicy && m_pPolicy->matchesLabel(m_aLabel))
    {
        restoreFromLabel();
        return;
    }
    PopulateCategories();
}

void SvxSecurityLabelDialog::PopulateClassifications()
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

void SvxSecurityLabelDialog::PopulateCategories()
{
    // Preserve the current checks across a rebuild (classification change), keyed by
    // group label (tag name) + checkbox label (category name).
    std::set<std::pair<OUString, OUString>> aChecked;
    for (size_t g = 0; g < m_aGroups.size(); ++g)
    {
        for (int c = 0; c < m_aGroups[g].nCats; ++c)
        {
            if (m_xChecks[g][c]->get_active())
                aChecked.emplace(m_xGroupLabels[g]->get_label(), m_xChecks[g][c]->get_label());
        }
    }

    // Reset the pool: hide every group and checkbox, drop the used-slot bookkeeping.
    for (int g = 0; g < MAX_GROUPS; ++g)
    {
        m_xGroupBoxes[g]->set_visible(false);
        for (int c = 0; c < MAX_CATS; ++c)
            m_xChecks[g][c]->set_visible(false);
    }
    m_aGroups.clear();
    m_aCheckGroup.clear();
    if (!m_pPolicy)
        return;

    const OUString sClassification = m_xClassification->get_active_text();
    int g = 0;
    for (const auto& rTagSet : m_pPolicy->aTagSets)
    {
        for (const auto& rTag : rTagSet.aTags)
        {
            // A group per securityCategoryTag that has at least one category selectable
            // under the current classification.
            std::vector<const svx::seclabel::SpifTagCategory*> aSelectable;
            for (const auto& rCategory : rTag.aCategories)
            {
                if (rCategory.isSelectable(sClassification))
                    aSelectable.push_back(&rCategory);
            }
            if (aSelectable.empty())
                continue;

            if (g >= MAX_GROUPS)
            {
                SAL_WARN("svx.seclabel", "category groups exceed pool of "
                                             << MAX_GROUPS << "; tag '" << rTag.aName
                                             << "' truncated");
                continue;
            }

            m_xGroupLabels[g]->set_label(rTag.aName);
            const OUString sHint = selectionHint(rTag, static_cast<sal_Int32>(aSelectable.size()));
            m_xGroupHints[g]->set_label(sHint);
            m_xGroupHints[g]->set_visible(!sHint.isEmpty());
            m_xGroupBoxes[g]->set_visible(true);

            int c = 0;
            for (const auto* pCategory : aSelectable)
            {
                if (c >= MAX_CATS)
                {
                    SAL_WARN("svx.seclabel", "categories of tag '"
                                                 << rTag.aName << "' exceed pool of " << MAX_CATS
                                                 << "; remainder truncated");
                    break;
                }
                const bool bChecked = aChecked.count({ rTag.aName, pCategory->aName }) != 0;
                m_xChecks[g][c]->set_label(pCategory->aName);
                m_xChecks[g][c]->set_active(bChecked);
                m_xChecks[g][c]->set_visible(true);
                m_aCheckGroup[m_xChecks[g][c].get()] = m_aGroups.size();
                ++c;
            }
            m_aGroups.push_back(
                { rTag.bSingleSelection, c, static_cast<int>(aSelectable.size()) });
            ++g;
        }
    }
}

void SvxSecurityLabelDialog::initFromExistingLabel()
{
    uno::Reference<frame::XModel> xModel = m_pTarget->getModel();
    if (!xModel.is())
        return;

    if (!svx::seclabel::readLabel(xModel, m_aLabel))
        return;

    m_bHasLabel = true;

    // A label written under a policy we don't have can't be edited structurally;
    // show it read-only and offer re-labeling under an available policy.
    const svx::seclabel::SpifPolicy* pMatch = m_aPolicySet.findByLabel(m_aLabel);
    if (!pMatch)
    {
        enterForeignMode(m_aLabel);
        return;
    }

    // Make the matching policy active in the selector; setActivePolicy restores the
    // stored classification and selections (now that m_bHasLabel/m_aLabel are set).
    const int nIndex = static_cast<int>(pMatch - m_aPolicySet.aPolicies.data());
    m_xPolicy->set_active(nIndex);
    setActivePolicy(nIndex);
}

void SvxSecurityLabelDialog::restoreFromLabel()
{
    // Select the stored classification. An obsolete value is hidden for new labels but
    // must render when editing one that uses it, so append it.
    int nPos = m_xClassification->find_text(m_aLabel.aClassification);
    if (nPos == -1)
    {
        m_xClassification->append_text(m_aLabel.aClassification);
        nPos = m_xClassification->find_text(m_aLabel.aClassification);
    }
    if (nPos != -1)
        m_xClassification->set_active(nPos);

    PopulateCategories();

    // Check the boxes whose category name (label) is among the label's values.
    std::set<OUString> aValues;
    for (const auto& rCategory : m_aLabel.aCategories)
    {
        for (const auto& rValue : rCategory.aValues)
            aValues.insert(rValue);
    }
    for (size_t nGroup = 0; nGroup < m_aGroups.size(); ++nGroup)
    {
        for (int c = 0; c < m_aGroups[nGroup].nCats; ++c)
        {
            if (aValues.count(m_xChecks[nGroup][c]->get_label()))
                m_xChecks[nGroup][c]->set_active(true);
        }
    }
}

void SvxSecurityLabelDialog::enterForeignMode(const svx::seclabel::StanagLabel& rLabel)
{
    m_bForeignPolicy = true;

    // The 4774 label is self-describing; use its summary, since the policy that
    // defines its exact marking is not available here.
    setPreview(rLabel.summary());

    const OUString sPolicy = rLabel.aPolicyName.isEmpty() ? rLabel.aPolicyId : rLabel.aPolicyName;
    m_xWarning->set_label(SvxResId(RID_SVXSTR_SECLABEL_FOREIGN).replaceFirst(u"%1", sPolicy));
    m_xWarning->set_visible(true);

    // View-only: hide the (provisioned-policy) editor and Apply. Offer Re-label only
    // if there is a provisioned policy to re-label under.
    m_xEditBox->set_visible(false);
    m_xOkBtn->set_visible(false);
    m_xRelabelBtn->set_visible(!m_aPolicySet.empty());
}

std::vector<bool> SvxSecurityLabelDialog::collectSelection() const
{
    // One bit per selectable category, in tag-set/tag/category order: exactly how
    // buildLabel and validate index. A tag that outgrew the pool has no checkbox for
    // the remainder, so pad those false -- otherwise every following group is read at
    // a shifted index and the label gets categories nobody checked.
    std::vector<bool> aSelected;
    for (size_t g = 0; g < m_aGroups.size(); ++g)
    {
        for (int c = 0; c < m_aGroups[g].nCats; ++c)
        {
            aSelected.push_back(m_xChecks[g][c]->get_active());
        }

        for (int c = m_aGroups[g].nCats; c < m_aGroups[g].nSelectable; ++c)
        {
            aSelected.push_back(false);
        }
    }

    return aSelected;
}

void SvxSecurityLabelDialog::UpdatePreview()
{
    if (m_bForeignPolicy || !m_pPolicy)
        return; // the foreign/no-policy view owns the preview and notice

    const OUString sClassification = m_xClassification->get_active_text();
    const std::vector<bool> aSelected = collectSelection();

    // The marking is derived from a label (the authoritative form); timestamps are
    // irrelevant to the visual marking, so a preview label with empty ones is fine.
    setPreview(m_pPolicy->deriveMarking(
        m_pPolicy->buildLabel(sClassification, aSelected, OUString(), OUString())));

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

void SvxSecurityLabelDialog::setPreview(const OUString& rMarking)
{
    m_xPreview->set_label(rMarking);
    m_xPreview->set_visible(!rMarking.isEmpty());
}

IMPL_LINK_NOARG(SvxSecurityLabelDialog, ClassificationHdl, weld::ComboBox&, void)
{
    PopulateCategories();
    UpdatePreview();
}

IMPL_LINK(SvxSecurityLabelDialog, CategoryToggleHdl, weld::Toggleable&, rToggle, void)
{
    // Single-selection group: when a box is checked, clear the others of its group so
    // it behaves like a radio set.
    const auto it = m_aCheckGroup.find(&rToggle);
    if (it != m_aCheckGroup.end() && m_aGroups[it->second].bSingle && rToggle.get_active())
    {
        for (auto& xCheck : m_xChecks[it->second])
        {
            if (xCheck.get() != &rToggle)
                xCheck->set_active(false);
        }
    }

    UpdatePreview();
}

void SvxSecurityLabelDialog::applyLabel(const OUString& rClassification,
                                        const std::vector<bool>& rSelected)
{
    uno::Reference<frame::XModel> xModel = m_pTarget->getModel();
    if (!xModel.is())
        return;

    const DateTime aNow(DateTime::SYSTEM);
    DateTime aReview(aNow);
    aReview.AddYears(1);

    svx::seclabel::StanagLabel aLabel
        = m_pPolicy->buildLabel(rClassification, rSelected, utl::toISO8601(aNow.GetUNODateTime()),
                                utl::toISO8601(aReview.GetUNODateTime()));
    // Cache the derived marking in the 4778 binding so the banner is correct even when
    // this policy is not provisioned on a later open (the label stays authoritative).
    const OUString sMarking = m_pPolicy->deriveMarking(aLabel);
    aLabel.aMarking = sMarking;
    const OUString sItemProps
        = svx::seclabel::buildItemProps(makeGuid(), svx::seclabel::STANAG_BINDING_SCHEMA);
    svx::seclabel::storeLabelPart(xModel, aLabel.toBindingXml(), sItemProps);

    // The target renders the marking + placements. The marking is the one just cached.
    svx::seclabel::LabelPlacement aPlacement;
    aPlacement.aMarking = sMarking;
    for (const auto& rClass : m_pPolicy->aClassifications)
    {
        if (rClass.aName == rClassification)
        {
            aPlacement.nColor = svx::seclabel::resolveColor(rClass.aColor);
            break;
        }
    }
    aPlacement.bCoverStart = m_pPolicy->wantsDocumentStart(rClassification, rSelected);
    aPlacement.bCoverEnd = m_pPolicy->wantsDocumentEnd(rClassification, rSelected);
    aPlacement.bPortion = m_pPolicy->wantsPortionMarking(rClassification, rSelected);
    aPlacement.bWatermark = m_pPolicy->wantsWatermark(rClassification, rSelected);
    m_pTarget->applyMarking(aPlacement);

    // Push the derived marking (what the banner shows on load), not the label's
    // generic summary, so the just-applied banner matches the dialog preview.
    m_pTarget->notify(aPlacement.aMarking);
}

IMPL_LINK_NOARG(SvxSecurityLabelDialog, PolicyHdl, weld::ComboBox&, void)
{
    setActivePolicy(m_xPolicy->get_active());
    UpdatePreview();
}

IMPL_LINK_NOARG(SvxSecurityLabelDialog, OkHdl, weld::Button&, void)
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

IMPL_LINK_NOARG(SvxSecurityLabelDialog, RelabelHdl, weld::Button&, void)
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

IMPL_LINK_NOARG(SvxSecurityLabelDialog, RemoveHdl, weld::Button&, void)
{
    uno::Reference<frame::XModel> xModel = m_pTarget->getModel();
    if (!xModel.is())
        return;
    svx::seclabel::removeLabelPart(xModel); // the customXml part
    m_pTarget->clearMarkings(); // header/footer, body, watermark
    m_pTarget->notify(OUString()); // empty => banner hides
    m_xDialog->response(RET_OK);
}

namespace svx::seclabel
{
VclPtr<VclAbstractDialog>
CreateSecurityLabelDialog(weld::Window* pParent, std::unique_ptr<SecurityLabelTarget> pTarget)
{
    using AbstractImpl = vcl::AbstractDialogImpl_Async<VclAbstractDialog, SvxSecurityLabelDialog>;
    return VclPtr<AbstractImpl>::Create(pParent, std::move(pTarget));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
