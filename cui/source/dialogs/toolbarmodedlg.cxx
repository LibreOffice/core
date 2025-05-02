/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <toolbarmodedlg.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <dialmgr.hxx>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <sfx2/viewfrm.hxx>
#include <strings.hrc>
#include <unotools/confignode.hxx>

ToolbarmodeDialog::ToolbarmodeDialog(weld::Window* pParent)
    : SfxTabDialogController(pParent, u"cui/ui/toolbarmodedialog.ui"_ustr,
                             u"ToolbarModeDialog"_ustr)
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xApplyBtn(m_xBuilder->weld_button(u"apply"_ustr)) // Apply to %Module
    , m_xCancelBtn(m_xBuilder->weld_button(u"cancel"_ustr)) // Close
    , m_xHelpBtn(m_xBuilder->weld_button(u"help"_ustr))
    , m_xResetBtn(m_xBuilder->weld_button(u"reset"_ustr)) // Apply to All
{
    AddTabPage("uimode", UITabPage::Create, nullptr);

    m_xOKBtn->set_visible(false);
    m_xHelpBtn->set_visible(false);
    m_xCancelBtn->set_label(CuiResId(RID_CUISTR_HYPDLG_CLOSEBUT)); // "close"
}

void ToolbarmodeDialog::ActivatePage(const OUString& rPage)
{
    if (rPage == "uimode")
    {
        m_xApplyBtn->set_label(
            CuiResId(RID_CUISTR_UI_APPLY).replaceFirst("%MODULE", UITabPage::GetCurrentApp()));
        m_xApplyBtn->set_from_icon_name("sw/res/sc20558.png");
        m_xResetBtn->set_label(CuiResId(RID_CUISTR_UI_APPLYALL));

        m_xApplyBtn->connect_clicked(LINK(this, ToolbarmodeDialog, OnApplyClick));
        m_xResetBtn->connect_clicked(LINK(this, ToolbarmodeDialog, OnApplyClick));
    }
}

IMPL_LINK(ToolbarmodeDialog, OnApplyClick, weld::Button&, rButton, void)
{
    UITabPage* pUITabPage = static_cast<UITabPage*>(GetCurTabPage());
    OUString sCmd = pUITabPage->GetSelectedMode();
    if (sCmd.isEmpty())
    {
        SAL_WARN("cui.dialogs", "ToolbarmodeDialog: no mode selected");
        return;
    }

    //apply to all except current module
    if (&rButton == m_xResetBtn.get()) // Apply to All
    {
        std::shared_ptr<comphelper::ConfigurationChanges> aBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::UI::ToolbarMode::ActiveWriter::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveCalc::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveImpress::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveDraw::set(sCmd, aBatch);
        aBatch->commit();

        const OUString sCurrentApp = UITabPage::GetCurrentApp();
        if (SfxViewFrame::Current())
        {
            const auto& xContext = comphelper::getProcessComponentContext();
            const utl::OConfigurationTreeRoot aAppNode(
                xContext, u"org.openoffice.Office.UI.ToolbarMode/Applications/"_ustr, true);
            if (sCurrentApp != "Writer")
                aAppNode.setNodeValue(u"Writer/Active"_ustr, css::uno::Any(sCmd));
            if (sCurrentApp != "Calc")
                aAppNode.setNodeValue(u"Calc/Active"_ustr, css::uno::Any(sCmd));
            if (sCurrentApp != "Impress")
                aAppNode.setNodeValue(u"Impress/Active"_ustr, css::uno::Any(sCmd));
            if (sCurrentApp != "Draw")
                aAppNode.setNodeValue(u"Draw/Active"_ustr, css::uno::Any(sCmd));
            aAppNode.commit();
        };
    }
    //apply to current module
    comphelper::dispatchCommand(".uno:ToolbarMode?Mode:string=" + sCmd, {});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
