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
#include <cmdid.h>
#include <doc.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>

#include <officecfg/Office/Common.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/AccessibilityIssue.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/svapp.hxx>

#include "A11yCheckIssuesPanel.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar
{
AccessibilityCheckEntry::AccessibilityCheckEntry(
    weld::Container* pParent, std::shared_ptr<sfx::AccessibilityIssue> const& rAccessibilityIssue)
    : m_xBuilder(Application::CreateBuilder(pParent, "svx/ui/accessibilitycheckentry.ui"))
    , m_xContainer(m_xBuilder->weld_container("accessibilityCheckEntryBox"))
    , m_xLabel(m_xBuilder->weld_label("accessibilityCheckEntryLabel"))
    , m_xGotoButton(m_xBuilder->weld_link_button("accessibilityCheckEntryLinkButton"))
    , m_xFixButton(m_xBuilder->weld_button("accessibilityCheckEntryFixButton"))
    , m_pAccessibilityIssue(rAccessibilityIssue)
{
    // lock in the height as including the button so all rows are the same height
    m_xContainer->set_size_request(-1, m_xContainer->get_preferred_size().Height());

    if (m_pAccessibilityIssue->canGotoIssue())
    {
        m_xGotoButton->set_label(m_pAccessibilityIssue->m_aIssueText);

        // int nPrefWidth(m_xGotoButton->get_preferred_size().Width());
        int nMaxWidth = m_xGotoButton->get_approximate_digit_width() * 10;
        // if (nPrefWidth > nMaxWidth)
        {
            // tdf#156137 allow LinkButton label to wrap
            m_xGotoButton->set_label_wrap(true);
            m_xGotoButton->set_size_request(nMaxWidth, -1);
        }

        m_xGotoButton->connect_activate_link(
            LINK(this, AccessibilityCheckEntry, GotoButtonClicked));
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
    m_pAccessibilityIssue->gotoIssue();
    return true;
}

IMPL_LINK_NOARG(AccessibilityCheckEntry, FixButtonClicked, weld::Button&, void)
{
    m_pAccessibilityIssue->quickFixIssue();
}

std::unique_ptr<PanelLayout> A11yCheckIssuesPanel::Create(weld::Widget* pParent,
                                                          SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw ::com::sun::star::lang::IllegalArgumentException(
            "no parent window given to A11yCheckIssuesPanel::Create", nullptr, 0);
    return std::make_unique<A11yCheckIssuesPanel>(pParent, pBindings);
}

A11yCheckIssuesPanel::A11yCheckIssuesPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : PanelLayout(pParent, "A11yCheckIssuesPanel", "modules/swriter/ui/a11ycheckissuespanel.ui")
    , m_xExpanderDocument(m_xBuilder->weld_expander("expand_document"))
    , m_xExpanderStyles(m_xBuilder->weld_expander("expand_styles"))
    , m_xExpanderNoAlt(m_xBuilder->weld_expander("expand_no_alt"))
    , m_xExpanderTable(m_xBuilder->weld_expander("expand_table"))
    , m_xExpanderFormatting(m_xBuilder->weld_expander("expand_formatting"))
    , m_xExpanderHyperlink(m_xBuilder->weld_expander("expand_hyperlink"))
    , m_xExpanderFakes(m_xBuilder->weld_expander("expand_fakes"))
    , m_xExpanderNumbering(m_xBuilder->weld_expander("expand_numbering"))
    , m_xExpanderOther(m_xBuilder->weld_expander("expand_other"))
    , m_xBoxDocument(m_xBuilder->weld_box("box_document"))
    , m_xBoxStyles(m_xBuilder->weld_box("box_styles"))
    , m_xBoxNoAlt(m_xBuilder->weld_box("box_no_alt"))
    , m_xBoxTable(m_xBuilder->weld_box("box_table"))
    , m_xBoxFormatting(m_xBuilder->weld_box("box_formatting"))
    , m_xBoxHyperlink(m_xBuilder->weld_box("box_hyperlink"))
    , m_xBoxFakes(m_xBuilder->weld_box("box_fakes"))
    , m_xBoxNumbering(m_xBuilder->weld_box("box_numbering"))
    , m_xBoxOther(m_xBuilder->weld_box("box_other"))
    , mpBindings(pBindings)
    , mpDoc(nullptr)
    , maA11yCheckController(FN_STAT_ACCESSIBILITY_CHECK, *pBindings, *this)
    , mnIssueCount(0)
    , mbAutomaticCheckEnabled(false)
{
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

    populateIssues();
}

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
    m_xExpanderDocument.reset();
    m_xExpanderStyles.reset();
    m_xExpanderNoAlt.reset();
    m_xExpanderTable.reset();
    m_xExpanderFormatting.reset();
    m_xExpanderHyperlink.reset();
    m_xExpanderFakes.reset();
    m_xExpanderNumbering.reset();
    m_xExpanderOther.reset();
    m_xBoxDocument.reset();
    m_xBoxStyles.reset();
    m_xBoxNoAlt.reset();
    m_xBoxTable.reset();
    m_xBoxFormatting.reset();
    m_xBoxHyperlink.reset();
    m_xBoxFakes.reset();
    m_xBoxNumbering.reset();
    m_xBoxOther.reset();
}

A11yCheckIssuesPanel::~A11yCheckIssuesPanel() { suppress_fun_call_w_exception(ImplDestroy()); }

void A11yCheckIssuesPanel::removeOldWidgets()
{
    for (auto const& xEntry : m_aDocumentEntries)
        m_xBoxDocument->move(xEntry->get_widget(), nullptr);
    m_xExpanderDocument->set_visible(false);

    for (auto const& xEntry : m_aStylesEntries)
        m_xBoxStyles->move(xEntry->get_widget(), nullptr);
    m_xExpanderStyles->set_visible(false);

    for (auto const& xEntry : m_aNoAltEntries)
        m_xBoxNoAlt->move(xEntry->get_widget(), nullptr);
    m_xExpanderNoAlt->set_visible(false);

    for (auto const& xEntry : m_aTableEntries)
        m_xBoxTable->move(xEntry->get_widget(), nullptr);
    m_xExpanderTable->set_visible(false);

    for (auto const& xEntry : m_aFormattingEntries)
        m_xBoxFormatting->move(xEntry->get_widget(), nullptr);
    m_xExpanderFormatting->set_visible(false);

    for (auto const& xEntry : m_aHyperlinkEntries)
        m_xBoxHyperlink->move(xEntry->get_widget(), nullptr);
    m_xExpanderHyperlink->set_visible(false);

    for (auto const& xEntry : m_aFakesEntries)
        m_xBoxFakes->move(xEntry->get_widget(), nullptr);
    m_xExpanderFakes->set_visible(false);

    for (auto const& xEntry : m_aNumberingEntries)
        m_xBoxNumbering->move(xEntry->get_widget(), nullptr);
    m_xExpanderNumbering->set_visible(false);

    for (auto const& xEntry : m_aOtherEntries)
        m_xBoxOther->move(xEntry->get_widget(), nullptr);
    m_xExpanderOther->set_visible(false);
}

void A11yCheckIssuesPanel::populateIssues()
{
    if (!mpDoc)
        return;
    sw::AccessibilityCheck aCheck(mpDoc);
    aCheck.check();
    m_aIssueCollection = aCheck.getIssueCollection();

    removeOldWidgets();

    sal_Int32 iDocument = 0;
    sal_Int32 iStyles = 0;
    sal_Int32 iNoAlt = 0;
    sal_Int32 iTable = 0;
    sal_Int32 iFormatting = 0;
    sal_Int32 iHyperlink = 0;
    sal_Int32 iFakes = 0;
    sal_Int32 iNumbering = 0;
    sal_Int32 iOther = 0;
    for (std::shared_ptr<sfx::AccessibilityIssue> const& pIssue : m_aIssueCollection.getIssues())
    {
        switch (pIssue->m_eIssueID)
        {
            case sfx::AccessibilityIssueID::DOCUMENT_TITLE:
            case sfx::AccessibilityIssueID::DOCUMENT_LANGUAGE:
            case sfx::AccessibilityIssueID::DOCUMENT_BACKGROUND:
            {
                auto xEntry
                    = std::make_unique<AccessibilityCheckEntry>(m_xBoxDocument.get(), pIssue);
                m_xBoxDocument->reorder_child(xEntry->get_widget(), iDocument++);
                m_xExpanderDocument->set_visible(true);
                m_aDocumentEntries.push_back(std::move(xEntry));
            }
            break;
            case sfx::AccessibilityIssueID::STYLE_LANGUAGE:
            {
                auto xEntry = std::make_unique<AccessibilityCheckEntry>(m_xBoxStyles.get(), pIssue);
                m_xBoxStyles->reorder_child(xEntry->get_widget(), iStyles++);
                m_xExpanderStyles->set_visible(true);
                m_aStylesEntries.push_back(std::move(xEntry));
            }
            break;
            case sfx::AccessibilityIssueID::NO_ALT_OLE:
            case sfx::AccessibilityIssueID::NO_ALT_GRAPHIC:
            case sfx::AccessibilityIssueID::NO_ALT_SHAPE:
            {
                auto xEntry = std::make_unique<AccessibilityCheckEntry>(m_xBoxNoAlt.get(), pIssue);
                m_xBoxNoAlt->reorder_child(xEntry->get_widget(), iNoAlt++);
                m_xExpanderNoAlt->set_visible(true);
                m_aNoAltEntries.push_back(std::move(xEntry));
            }
            break;
            case sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT:
            {
                auto xEntry = std::make_unique<AccessibilityCheckEntry>(m_xBoxTable.get(), pIssue);
                m_xBoxTable->reorder_child(xEntry->get_widget(), iTable++);
                m_xExpanderTable->set_visible(true);
                m_aTableEntries.push_back(std::move(xEntry));
            }
            break;
            case sfx::AccessibilityIssueID::TEXT_FORMATTING:
            case sfx::AccessibilityIssueID::TABLE_FORMATTING:
            {
                auto xEntry
                    = std::make_unique<AccessibilityCheckEntry>(m_xBoxFormatting.get(), pIssue);
                m_xBoxFormatting->reorder_child(xEntry->get_widget(), iFormatting++);
                m_xExpanderFormatting->set_visible(true);
                m_aFormattingEntries.push_back(std::move(xEntry));
            }
            break;
            case sfx::AccessibilityIssueID::HYPERLINK_IS_TEXT:
            case sfx::AccessibilityIssueID::HYPERLINK_SHORT:
            {
                auto xEntry
                    = std::make_unique<AccessibilityCheckEntry>(m_xBoxHyperlink.get(), pIssue);
                m_xBoxHyperlink->reorder_child(xEntry->get_widget(), iHyperlink++);
                m_xExpanderHyperlink->set_visible(true);
                m_aHyperlinkEntries.push_back(std::move(xEntry));
            }
            break;
            case sfx::AccessibilityIssueID::FAKE_FOOTNOTE:
            case sfx::AccessibilityIssueID::FAKE_CAPTION:
            {
                auto xEntry = std::make_unique<AccessibilityCheckEntry>(m_xBoxFakes.get(), pIssue);
                m_xBoxFakes->reorder_child(xEntry->get_widget(), iFakes++);
                m_xExpanderFakes->set_visible(true);
                m_aFakesEntries.push_back(std::move(xEntry));
            }
            break;
            case sfx::AccessibilityIssueID::MANUAL_NUMBERING:
            {
                auto xEntry
                    = std::make_unique<AccessibilityCheckEntry>(m_xBoxNumbering.get(), pIssue);
                m_xBoxNumbering->reorder_child(xEntry->get_widget(), iNumbering++);
                m_xExpanderNumbering->set_visible(true);
                m_aNumberingEntries.push_back(std::move(xEntry));
            }
            break;
            case sfx::AccessibilityIssueID::UNSPECIFIED:
            {
                auto xEntry = std::make_unique<AccessibilityCheckEntry>(m_xBoxOther.get(), pIssue);
                m_xBoxOther->reorder_child(xEntry->get_widget(), iOther++);
                m_xExpanderOther->set_visible(true);
                m_aOtherEntries.push_back(std::move(xEntry));
            }
            break;
            default:
            {
                SAL_WARN("sw.a11y", "Invalid issue ID.");
                continue;
            }
            break;
        };
    }
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
