/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <toolbarmodedlg.hxx>
#include <toolbarmode.hrc>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/types.hxx>
#include <dialmgr.hxx>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <sfx2/viewfrm.hxx>
#include <strings.hrc>
#include <svtools/miscopt.hxx>
#include <unotools/confignode.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/weld.hxx>

static OUString GetCurrentApp()
{
    OUString sResult;
    if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
    {
        const auto xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
        const auto xContext = comphelper::getProcessComponentContext();
        const auto xModuleManager = css::frame::ModuleManager::create(xContext);
        switch (vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xCurrentFrame)))
        {
            case vcl::EnumContext::Application::Writer:
                sResult = "Writer";
                break;
            case vcl::EnumContext::Application::Calc:
                sResult = "Calc";
                break;
            case vcl::EnumContext::Application::Impress:
                sResult = "Impress";
                break;
            case vcl::EnumContext::Application::Draw:
                sResult = "Draw";
                break;
            case vcl::EnumContext::Application::Formula:
                sResult = "Formula";
                break;
            case vcl::EnumContext::Application::Base:
                sResult = "Base";
                break;
            default:
                sResult = "Unsupported";
        }
    }
    return sResult;
}

static OUString GetCurrentMode()
{
    OUString sResult;
    if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
    {
        const auto xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
        const auto xContext = comphelper::getProcessComponentContext();
        const auto xModuleManager = css::frame::ModuleManager::create(xContext);
        const utl::OConfigurationTreeRoot aAppNode(
            xContext, "org.openoffice.Office.UI.ToolbarMode/Applications/" + GetCurrentApp(), true);
        if (aAppNode.isValid())
            sResult = comphelper::getString(aAppNode.getNodeValue("Active"));
    };
    return sResult;
}

ToolbarmodeDialog::ToolbarmodeDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/toolbarmodedialog.ui", "ToolbarmodeDialog")
    , m_pImage(m_xBuilder->weld_image("imImage"))
    , m_pApply(m_xBuilder->weld_button("btnApply"))
    , m_pApplyAll(m_xBuilder->weld_button("btnApplyAll"))
    , m_pRadioButtons{ (m_xBuilder->weld_radio_button("rbButton1")),
                       (m_xBuilder->weld_radio_button("rbButton2")),
                       (m_xBuilder->weld_radio_button("rbButton3")),
                       (m_xBuilder->weld_radio_button("rbButton4")),
                       (m_xBuilder->weld_radio_button("rbButton5")),
                       (m_xBuilder->weld_radio_button("rbButton6")),
                       (m_xBuilder->weld_radio_button("rbButton7")),
                       (m_xBuilder->weld_radio_button("rbButton8")),
                       (m_xBuilder->weld_radio_button("rbButton9")) }
    , m_pInfoLabel(m_xBuilder->weld_label("lbInfo"))
{
    static_assert(SAL_N_ELEMENTS(m_pRadioButtons) == SAL_N_ELEMENTS(TOOLBARMODES_ARRAY));

    Link<weld::ToggleButton&, void> aLink = LINK(this, ToolbarmodeDialog, SelectToolbarmode);

    const OUString sCurrentMode = GetCurrentMode();
    for (unsigned long i = 0; i < SAL_N_ELEMENTS(m_pRadioButtons); i++)
    {
        m_pRadioButtons[i]->connect_toggled(aLink);
        if (sCurrentMode == std::get<1>(TOOLBARMODES_ARRAY[i]))
        {
            m_pRadioButtons[i]->set_active(true);
            UpdateImage(std::get<2>(TOOLBARMODES_ARRAY[i]));
            m_pInfoLabel->set_label(CuiResId(std::get<0>(TOOLBARMODES_ARRAY[i])));
        }
    }

    m_pApply->set_label(CuiResId(RID_SVXSTR_UI_APPLYALL).replaceFirst("%MODULE", GetCurrentApp()));
    m_pApply->connect_clicked(LINK(this, ToolbarmodeDialog, OnApplyClick));
    m_pApplyAll->connect_clicked(LINK(this, ToolbarmodeDialog, OnApplyClick));

    if (!SvtMiscOptions().IsExperimentalMode())
    {
        m_pRadioButtons[nContextSingle]->set_visible(false);
        m_pRadioButtons[nContextGroup]->set_visible(false);
    }
}

ToolbarmodeDialog::~ToolbarmodeDialog() = default;

static bool file_exists(const OUString& fileName)
{
    osl::File aFile(fileName);
    return aFile.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None;
}

int ToolbarmodeDialog::GetActiveRadioButton()
{
    for (unsigned long i = 0; i < SAL_N_ELEMENTS(m_pRadioButtons); i++)
    {
        if (m_pRadioButtons[i]->get_active())
            return i;
    }
    return -1;
}

void ToolbarmodeDialog::UpdateImage(OUString sFileName)
{
    // load image
    OUString aURL("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/toolbarmode/");
    rtl::Bootstrap::expandMacros(aURL);
    aURL += sFileName;
    if (sFileName.isEmpty() || !file_exists(aURL))
        return;
    // draw image
    Graphic aGraphic;
    if (GraphicFilter::LoadGraphic(aURL, OUString(), aGraphic) == ERRCODE_NONE)
    {
        ScopedVclPtr<VirtualDevice> m_pVirDev = m_pImage->create_virtual_device();
        m_pVirDev->SetOutputSizePixel(aGraphic.GetSizePixel());
        m_pVirDev->DrawBitmapEx(Point(0, 0), aGraphic.GetBitmapEx());
        m_pImage->set_image(m_pVirDev.get());
        m_pVirDev.disposeAndClear();
    }
}

IMPL_LINK_NOARG(ToolbarmodeDialog, SelectToolbarmode, weld::ToggleButton&, void)
{
    const int i = GetActiveRadioButton();
    if (i > -1)
    {
        UpdateImage(std::get<2>(TOOLBARMODES_ARRAY[i]));
        m_pInfoLabel->set_label(CuiResId(std::get<0>(TOOLBARMODES_ARRAY[i])));
    }
}

IMPL_LINK(ToolbarmodeDialog, OnApplyClick, weld::Button&, rButton, void)
{
    const int i = GetActiveRadioButton();
    if (i == -1)
        return;
    const OUString sCmd = std::get<1>(TOOLBARMODES_ARRAY[i]);
    //apply to all except current module
    if (&rButton == m_pApplyAll.get())
    {
        std::shared_ptr<comphelper::ConfigurationChanges> aBatch(
            comphelper::ConfigurationChanges::create(::comphelper::getProcessComponentContext()));
        officecfg::Office::UI::ToolbarMode::ActiveWriter::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveCalc::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveImpress::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveDraw::set(sCmd, aBatch);
        aBatch->commit();

        OUString sCurrentApp = GetCurrentApp();
        if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
        {
            const auto xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
            const auto xContext = comphelper::getProcessComponentContext();
            const auto xModuleManager = css::frame::ModuleManager::create(xContext);
            const utl::OConfigurationTreeRoot aAppNode(
                xContext, "org.openoffice.Office.UI.ToolbarMode/Applications/", true);
            if (sCurrentApp != "Writer")
                aAppNode.setNodeValue("Writer/Active", css::uno::makeAny(sCmd));
            if (sCurrentApp != "Calc")
                aAppNode.setNodeValue("Calc/Active", css::uno::makeAny(sCmd));
            if (sCurrentApp != "Impress")
                aAppNode.setNodeValue("Impress/Active", css::uno::makeAny(sCmd));
            if (sCurrentApp != "Draw")
                aAppNode.setNodeValue("Draw/Active", css::uno::makeAny(sCmd));
            aAppNode.commit();
        };
    }
    //apply to current module
    comphelper::dispatchCommand(".uno:ToolbarMode?Mode:string=" + sCmd, {});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
