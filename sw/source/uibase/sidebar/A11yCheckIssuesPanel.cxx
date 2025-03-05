/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <AccessibilityCheck.hxx>
#include <AccessibilityIssue.hxx>
#include <cmdid.h>
#include <doc.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>

#include <officecfg/Office/Common.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/AccessibilityIssue.hxx>
#include <sfx2/pageids.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/enumrange.hxx>
#include <comphelper/lok.hxx>

#include "A11yCheckIssuesPanel.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar
{
AccessibilityCheckEntry::AccessibilityCheckEntry(
    weld::Container* pParent, std::shared_ptr<sfx::AccessibilityIssue> const& rAccessibilityIssue)
    : m_xBuilder(Application::CreateBuilder(pParent, u"svx/ui/accessibilitycheckentry.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_container(u"accessibilityCheckEntryBox"_ustr))
    , m_xLabel(m_xBuilder->weld_label(u"accessibilityCheckEntryLabel"_ustr))
    , m_xGotoButton(m_xBuilder->weld_link_button(u"accessibilityCheckEntryLinkButton"_ustr))
    , m_xFixButton(m_xBuilder->weld_button(u"accessibilityCheckEntryFixButton"_ustr))
    , m_pAccessibilityIssue(rAccessibilityIssue)
{
    // lock in the height as including the button so all rows are the same height
    m_xContainer->set_size_request(-1, m_xContainer->get_preferred_size().Height());

    if (m_pAccessibilityIssue->canGotoIssue())
    {
        m_xGotoButton->set_label(m_pAccessibilityIssue->m_aIssueText);

        // tdf#156137 allow LinkButton label to wrap
        int nMaxWidth = m_xGotoButton->get_approximate_digit_width() * 10;
        m_xGotoButton->set_label_wrap(true);
        m_xGotoButton->set_size_request(nMaxWidth, -1);

        m_xGotoButton->connect_activate_link(
            LINK(this, AccessibilityCheckEntry, GotoButtonClicked));

        // add full path of linked graphic as tooltip,
        if (m_pAccessibilityIssue->m_eIssueID == sfx::AccessibilityIssueID::LINKED_GRAPHIC)
        {
            auto pSwIssue = std::static_pointer_cast<sw::AccessibilityIssue>(m_pAccessibilityIssue);
            auto aInfo = pSwIssue->getAdditionalInfo();
            if (aInfo.size() > 0)
            {
                m_xGotoButton->set_tooltip_text(aInfo[0]);
            }
        }

        m_xLabel->set_visible(false);
    }
    else
    {
        m_xLabel->set_label(m_pAccessibilityIssue->m_aIssueText);
        m_xGotoButton->set_visible(false);
    }

    m_xFixButton->set_visible(m_pAccessibilityIssue->canQuickFixIssue());
    m_xFixButton->connect_clicked(LINK(this, AccessibilityCheckEntry, FixButtonClicked));

    m_pAccessibilityIssue->setParent(dynamic_cast<weld::Window*>(get_widget()));
}

IMPL_LINK_NOARG(AccessibilityCheckEntry, GotoButtonClicked, weld::LinkButton&, bool)
{
    if (m_pAccessibilityIssue)
        m_pAccessibilityIssue->gotoIssue();
    return true;
}

IMPL_LINK_NOARG(AccessibilityCheckEntry, FixButtonClicked, weld::Button&, void)
{
    if (m_pAccessibilityIssue)
        m_pAccessibilityIssue->quickFixIssue();
}

AccessibilityCheckLevel::AccessibilityCheckLevel(weld::Box* pParent)
    : m_xBuilder(Application::CreateBuilder(pParent, u"svx/ui/accessibilitychecklevel.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_box(u"accessibilityCheckLevelBox"_ustr))
{
    m_xExpanders[0] = m_xBuilder->weld_expander(u"expand_document"_ustr);
    m_xExpanders[1] = m_xBuilder->weld_expander(u"expand_styles"_ustr);
    m_xExpanders[2] = m_xBuilder->weld_expander(u"expand_linked"_ustr);
    m_xExpanders[3] = m_xBuilder->weld_expander(u"expand_no_alt"_ustr);
    m_xExpanders[4] = m_xBuilder->weld_expander(u"expand_table"_ustr);
    m_xExpanders[5] = m_xBuilder->weld_expander(u"expand_formatting"_ustr);
    m_xExpanders[6] = m_xBuilder->weld_expander(u"expand_direct_formatting"_ustr);
    m_xExpanders[7] = m_xBuilder->weld_expander(u"expand_hyperlink"_ustr);
    m_xExpanders[8] = m_xBuilder->weld_expander(u"expand_fakes"_ustr);
    m_xExpanders[9] = m_xBuilder->weld_expander(u"expand_numbering"_ustr);
    m_xExpanders[10] = m_xBuilder->weld_expander(u"expand_other"_ustr);

    m_xBoxes[0] = m_xBuilder->weld_box(u"box_document"_ustr);
    m_xBoxes[1] = m_xBuilder->weld_box(u"box_styles"_ustr);
    m_xBoxes[2] = m_xBuilder->weld_box(u"box_linked"_ustr);
    m_xBoxes[3] = m_xBuilder->weld_box(u"box_no_alt"_ustr);
    m_xBoxes[4] = m_xBuilder->weld_box(u"box_table"_ustr);
    m_xBoxes[5] = m_xBuilder->weld_box(u"box_formatting"_ustr);
    m_xBoxes[6] = m_xBuilder->weld_box(u"box_direct_formatting"_ustr);
    m_xBoxes[7] = m_xBuilder->weld_box(u"box_hyperlink"_ustr);
    m_xBoxes[8] = m_xBuilder->weld_box(u"box_fakes"_ustr);
    m_xBoxes[9] = m_xBuilder->weld_box(u"box_numbering"_ustr);
    m_xBoxes[10] = m_xBuilder->weld_box(u"box_other"_ustr);
}

void AccessibilityCheckLevel::removeAllEntries()
{
    for (auto eGroup : o3tl::enumrange<AccessibilityCheckGroups>())
    {
        auto nGroupIndex = size_t(eGroup);
        for (auto const& xEntry : m_aEntries[nGroupIndex])
            m_xBoxes[nGroupIndex]->move(xEntry->get_widget(), nullptr);
    }
}

void AccessibilityCheckLevel::reset()
{
    for (auto& xExpander : m_xExpanders)
        xExpander.reset();

    for (auto& xBox : m_xBoxes)
        xBox.reset();
}

void AccessibilityCheckLevel::addEntryForGroup(
    AccessibilityCheckGroups eGroup, std::vector<sal_Int32>& rIndices,
    std::shared_ptr<sfx::AccessibilityIssue> const& pIssue)
{
    auto nGroupIndex = size_t(eGroup);
    // prevent the UI locking up forever, this is effectively an O(n^2) situation, given the way the UI additions work
    if (m_aEntries[nGroupIndex].size() > 500)
    {
        SAL_WARN("sw", "too many a11y issues, not adding to panel");
        return;
    }
    auto xEntry = std::make_unique<AccessibilityCheckEntry>(m_xBoxes[nGroupIndex].get(), pIssue);
    m_xBoxes[nGroupIndex]->reorder_child(xEntry->get_widget(), rIndices[nGroupIndex]++);
    m_aEntries[nGroupIndex].push_back(std::move(xEntry));
}

size_t AccessibilityCheckLevel::getEntrySize(AccessibilityCheckGroups eGroup) const
{
    auto nGroupIndex = size_t(eGroup);
    return m_aEntries[nGroupIndex].size();
}

void AccessibilityCheckLevel::show(size_t nGroupIndex) { m_xExpanders[nGroupIndex]->show(); }

void AccessibilityCheckLevel::hide(size_t nGroupIndex) { m_xExpanders[nGroupIndex]->hide(); }

std::unique_ptr<PanelLayout> A11yCheckIssuesPanel::Create(weld::Widget* pParent,
                                                          SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent window given to A11yCheckIssuesPanel::Create"_ustr, nullptr, 0);
    return std::make_unique<A11yCheckIssuesPanel>(pParent, pBindings);
}

A11yCheckIssuesPanel::A11yCheckIssuesPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : PanelLayout(pParent, u"A11yCheckIssuesPanel"_ustr,
                  u"modules/swriter/ui/a11ycheckissuespanel.ui"_ustr)
    , m_xOptionsButton(m_xBuilder->weld_button(u"bOptions"_ustr))
    , mxUpdateBox(m_xBuilder->weld_box(u"updateBox"_ustr))
    , mxUpdateLinkButton(m_xBuilder->weld_link_button(u"updateLinkButton"_ustr))
    , m_xListSep(m_xBuilder->weld_widget(u"sep_level"_ustr))
    , mpBindings(pBindings)
    , mpDoc(nullptr)
    , maA11yCheckController(FN_STAT_ACCESSIBILITY_CHECK, *pBindings, *this)
    , mnIssueCount(0)
    , mbAutomaticCheckEnabled(false)
{
    // errors
    m_xLevelExpanders[0] = m_xBuilder->weld_expander(u"expand_errors"_ustr);
    mxAccessibilityBox[0] = m_xBuilder->weld_box(u"accessibilityBoxErr"_ustr);
    m_aLevelEntries[0] = std::make_unique<AccessibilityCheckLevel>(mxAccessibilityBox[0].get());

    // warnings
    m_xLevelExpanders[1] = m_xBuilder->weld_expander(u"expand_warnings"_ustr);
    mxAccessibilityBox[1] = m_xBuilder->weld_box(u"accessibilityBoxWrn"_ustr);
    m_aLevelEntries[1] = std::make_unique<AccessibilityCheckLevel>(mxAccessibilityBox[1].get());

    mxUpdateLinkButton->connect_activate_link(
        LINK(this, A11yCheckIssuesPanel, UpdateLinkButtonClicked));

    SwDocShell* pDocSh = dynamic_cast<SwDocShell*>(SfxObjectShell::Current());
    if (!pDocSh)
        return;

    // Automatic a11y checking must be enabled for this panel to work properly
    mbAutomaticCheckEnabled
        = officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get();
    if (!mbAutomaticCheckEnabled)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::set(true, batch);
        batch->commit();
        pBindings->Invalidate(SID_ACCESSIBILITY_CHECK_ONLINE);
    }

    mpDoc = pDocSh->GetDoc();

    m_xOptionsButton->connect_clicked(LINK(this, A11yCheckIssuesPanel, OptionsButtonClicked));

    // If LOKit is enabled, then enable the update button and don't run the accessibility check.
    // In desktop don't show the update button and schedule to run the accessibility check async
    if (comphelper::LibreOfficeKit::isActive())
    {
        m_xLevelExpanders[0]->hide();
        m_xLevelExpanders[1]->hide();
        mxUpdateBox->show();
    }
    else
    {
        m_xLevelExpanders[0]->show();
        m_xLevelExpanders[1]->show();
        mxUpdateBox->hide();
        Application::PostUserEvent(LINK(this, A11yCheckIssuesPanel, PopulateIssuesHdl));
    }
}

IMPL_LINK_NOARG(A11yCheckIssuesPanel, OptionsButtonClicked, weld::Button&, void)
{
    SfxUInt16Item aPageID(SID_OPTIONS_PAGEID, sal_uInt16(RID_SVXPAGE_ACCESSIBILITYCONFIG));
    auto pDispatcher = GetBindings()->GetDispatcher();
    pDispatcher->ExecuteList(SID_OPTIONS_TREEDIALOG, SfxCallMode::SYNCHRON, { &aPageID });
}

IMPL_LINK_NOARG(A11yCheckIssuesPanel, UpdateLinkButtonClicked, weld::LinkButton&, bool)
{
    m_xLevelExpanders[0]->show();
    m_xLevelExpanders[1]->show();
    mxUpdateBox->hide();
    Application::PostUserEvent(LINK(this, A11yCheckIssuesPanel, PopulateIssuesHdl));
    return true;
}

IMPL_LINK_NOARG(A11yCheckIssuesPanel, PopulateIssuesHdl, void*, void) { populateIssues(); }

void A11yCheckIssuesPanel::ImplDestroy()
{
    // Restore state when this panel is no longer used
    if (!mbAutomaticCheckEnabled)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::set(false, batch);
        batch->commit();
        mpBindings->Invalidate(SID_ACCESSIBILITY_CHECK_ONLINE);
    }

    for (auto& aLevelEntry : m_aLevelEntries)
    {
        aLevelEntry->reset();
        aLevelEntry.reset();
    }
}

A11yCheckIssuesPanel::~A11yCheckIssuesPanel() { suppress_fun_call_w_exception(ImplDestroy()); }

void A11yCheckIssuesPanel::removeAllEntries()
{
    for (auto& aLevelEntry : m_aLevelEntries)
    {
        aLevelEntry->removeAllEntries();
    }
}

void A11yCheckIssuesPanel::addEntryForGroup(AccessibilityCheckGroups eGroup,
                                            std::vector<std::vector<sal_Int32>>& rIndices,
                                            std::shared_ptr<sfx::AccessibilityIssue> const& pIssue)
{
    // prevent the UI locking up forever, this is effectively an O(n^2) situation, given the way the UI additions work
    if (m_aLevelEntries[0]->getEntrySize(eGroup) + m_aLevelEntries[1]->getEntrySize(eGroup) > 500)
    {
        SAL_WARN("sw", "too many a11y issues, not adding to panel");
        return;
    }

    size_t nLevel = static_cast<size_t>(pIssue->m_eIssueLvl);
    m_aLevelEntries[nLevel]->addEntryForGroup(eGroup, rIndices[nLevel], pIssue);
}

void A11yCheckIssuesPanel::populateIssues()
{
    if (!mpDoc || mxUpdateBox->is_visible())
        return;

    SfxViewShell* pViewShell = SfxViewShell::Current();
    auto* pWindow = pViewShell ? pViewShell->GetWindow() : nullptr;

    if (pWindow)
        pWindow->SetPointer(PointerStyle::Wait);

    sw::AccessibilityCheck aCheck(mpDoc);
    aCheck.check();
    m_aIssueCollection = aCheck.getIssueCollection();

    removeAllEntries();

    std::vector<std::vector<sal_Int32>> nIndices(2, std::vector<sal_Int32>(11, 0));
    sal_Int32 nDirectFormats = 0;

    for (std::shared_ptr<sfx::AccessibilityIssue> const& pIssue : m_aIssueCollection.getIssues())
    {
        switch (pIssue->m_eIssueID)
        {
            case sfx::AccessibilityIssueID::DOCUMENT_TITLE:
            case sfx::AccessibilityIssueID::DOCUMENT_LANGUAGE:
            case sfx::AccessibilityIssueID::DOCUMENT_BACKGROUND:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::Document, nIndices, pIssue);
            }
            break;
            case sfx::AccessibilityIssueID::STYLE_LANGUAGE:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::Styles, nIndices, pIssue);
            }
            break;
            case sfx::AccessibilityIssueID::LINKED_GRAPHIC:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::Linked, nIndices, pIssue);
            }
            break;
            case sfx::AccessibilityIssueID::NO_ALT_OLE:
            case sfx::AccessibilityIssueID::NO_ALT_GRAPHIC:
            case sfx::AccessibilityIssueID::NO_ALT_SHAPE:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::NoAlt, nIndices, pIssue);
            }
            break;
            case sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::Table, nIndices, pIssue);
            }
            break;
            case sfx::AccessibilityIssueID::TEXT_NEW_LINES:
            case sfx::AccessibilityIssueID::TEXT_SPACES:
            case sfx::AccessibilityIssueID::TEXT_TABS:
            case sfx::AccessibilityIssueID::TEXT_EMPTY_NUM_PARA:
            case sfx::AccessibilityIssueID::TABLE_FORMATTING:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::Formatting, nIndices, pIssue);
            }
            break;
            case sfx::AccessibilityIssueID::DIRECT_FORMATTING:
            {
                if (!pIssue->getHidden())
                {
                    addEntryForGroup(AccessibilityCheckGroups::DirectFormatting, nIndices, pIssue);
                    nDirectFormats++;
                }
            }
            break;
            case sfx::AccessibilityIssueID::HYPERLINK_IS_TEXT:
            case sfx::AccessibilityIssueID::HYPERLINK_SHORT:
            case sfx::AccessibilityIssueID::HYPERLINK_NO_NAME:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::Hyperlink, nIndices, pIssue);
            }
            break;
            case sfx::AccessibilityIssueID::FAKE_FOOTNOTE:
            case sfx::AccessibilityIssueID::FAKE_CAPTION:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::Fakes, nIndices, pIssue);
            }
            break;
            case sfx::AccessibilityIssueID::MANUAL_NUMBERING:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::Numbering, nIndices, pIssue);
            }
            break;

            case sfx::AccessibilityIssueID::TEXT_CONTRAST:
            case sfx::AccessibilityIssueID::TEXT_BLINKING:
            case sfx::AccessibilityIssueID::HEADINGS_NOT_IN_ORDER:
            case sfx::AccessibilityIssueID::NON_INTERACTIVE_FORMS:
            case sfx::AccessibilityIssueID::FLOATING_TEXT:
            case sfx::AccessibilityIssueID::HEADING_IN_TABLE:
            case sfx::AccessibilityIssueID::HEADING_START:
            case sfx::AccessibilityIssueID::HEADING_ORDER:
            case sfx::AccessibilityIssueID::CONTENT_CONTROL:
            case sfx::AccessibilityIssueID::AVOID_FOOTNOTES:
            case sfx::AccessibilityIssueID::AVOID_ENDNOTES:
            case sfx::AccessibilityIssueID::FONTWORKS:
            {
                if (!pIssue->getHidden())
                    addEntryForGroup(AccessibilityCheckGroups::Other, nIndices, pIssue);
            }
            break;

            default:
            {
                SAL_WARN("sw.a11y", "Invalid issue ID.");
                continue;
            }
        }
    }

    // add DirectFormats (if have) as last element to Formatting AccessibilityCheckGroup on the Warning level
    if (nDirectFormats > 0)
    {
        size_t nGroupFormatIndex = size_t(AccessibilityCheckGroups::Formatting);
        // Direct Formats are on the warning level
        size_t nLevel = static_cast<size_t>(sfx::AccessibilityIssueLevel::WARNLEV);
        nIndices[nLevel][nGroupFormatIndex]++;
    }

    for (auto eLevel : o3tl::enumrange<sfx::AccessibilityIssueLevel>())
    {
        size_t nGroupIndex = 0;
        auto nLevelIndex = size_t(eLevel);
        bool bHaveIssue = false;
        for (sal_Int32 nIndex : nIndices[nLevelIndex])
        {
            if (nIndex > 0)
            {
                m_aLevelEntries[nLevelIndex]->show(nGroupIndex);
                if (!bHaveIssue)
                    bHaveIssue = true;
            }
            else
                m_aLevelEntries[nLevelIndex]->hide(nGroupIndex);
            nGroupIndex++;
        }
        if (!bHaveIssue)
            m_xLevelExpanders[nLevelIndex]->hide();
        else
            m_xLevelExpanders[nLevelIndex]->show();
    }

    if (m_xLevelExpanders[0]->is_visible() && m_xLevelExpanders[1]->is_visible())
        m_xListSep->set_visible(true);
    else
        m_xListSep->set_visible(false);

    if (pWindow)
        pWindow->SetPointer(PointerStyle::Arrow);
}

void A11yCheckIssuesPanel::NotifyItemUpdate(const sal_uInt16 nSid, const SfxItemState /* eState */,
                                            const SfxPoolItem* pState)
{
    if (!pState) //disposed
        return;

    switch (nSid)
    {
        case FN_STAT_ACCESSIBILITY_CHECK:
        {
            sal_Int32 nIssueCount = static_cast<const SfxInt32Item*>(pState)->GetValue();
            if (nIssueCount != mnIssueCount)
            {
                mnIssueCount = nIssueCount;
                populateIssues();
            }
        }
        break;
        default:
            break;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
