/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <toolbarmodedlg.hxx>

#include <dialmgr.hxx>
#include <vcl/weld.hxx>
#include <toolbarmode.hrc>
#include <svtools/miscopt.hxx>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graphicfilter.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <vcl/scheduler.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <unotools/confignode.hxx>
#include <unotools/moduleoptions.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/EnumContext.hxx>

#include <sfx2/module.hxx>
#include <sfx2/app.hxx>
#include <strings.hrc>

#include <officecfg/Office/UI/ToolbarMode.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

static OUString GetCurrentApp()
{
    Reference<XFrame> xCurrentFrame;
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::NONE;
    const Reference<frame::XModuleManager> xModuleManager = frame::ModuleManager::create(xContext);
    eCurrentApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xCurrentFrame));
    if (eCurrentApp == vcl::EnumContext::Application::Writer)
        return "Writer";
    else if (eCurrentApp == vcl::EnumContext::Application::Calc)
        return "Calc";
    else if (eCurrentApp == vcl::EnumContext::Application::Impress)
        return "Impress";
    else if (eCurrentApp == vcl::EnumContext::Application::Draw)
        return "Draw";
    else if (eCurrentApp == vcl::EnumContext::Application::Formula)
        return "Formula";
    else if (eCurrentApp == vcl::EnumContext::Application::Base)
        return "Base";
    else
        return "None";
}

static OUString GetCurrentMode()
{
    OUString sResult;
    Reference<XFrame> xCurrentFrame;
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (pViewFrame)
    {
        xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
        const Reference<frame::XModuleManager> xModuleManager
            = frame::ModuleManager::create(xContext);

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
    Link<weld::ToggleButton&, void> aLink = LINK(this, ToolbarmodeDialog, SelectToolbarmode);

    OUString sCurrentMode = GetCurrentMode();
    for (int i = 0; i < 9; i++)
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
        m_pRadioButtons[7]->set_visible(false);
        m_pRadioButtons[8]->set_visible(false);
    }
}

ToolbarmodeDialog::~ToolbarmodeDialog() {}

static bool file_exists(const OUString& fileName)
{
    ::osl::File aFile(fileName);
    return aFile.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None;
}

int ToolbarmodeDialog::GetActiveRadioButton()
{
    for (int i = 0; i < 9; i++)
    {
        if (m_pRadioButtons[i]->get_active())
        {
            return i;
            break;
        }
    }
    return -1;
}

void ToolbarmodeDialog::UpdateImage(OUString sFileName)
{
    // load image
    OUString aURL("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/toolbarmode/");
    rtl::Bootstrap::expandMacros(aURL);
    if (sFileName.isEmpty() || !file_exists(aURL + sFileName))
        return;
    // draw image
    Graphic aGraphic;
    if (GraphicFilter::LoadGraphic(aURL + sFileName, OUString(), aGraphic) == ERRCODE_NONE)
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
    int i = GetActiveRadioButton();
    if (i > -1)
    {
        UpdateImage(std::get<2>(TOOLBARMODES_ARRAY[i]));
        m_pInfoLabel->set_label(CuiResId(std::get<0>(TOOLBARMODES_ARRAY[i])));
    }
}

IMPL_LINK(ToolbarmodeDialog, OnApplyClick, weld::Button&, rButton, void)
{
    OUString sCmd;
    int i = GetActiveRadioButton();
    if (i > -1)
        sCmd = std::get<1>(TOOLBARMODES_ARRAY[i]);

    if ((&rButton == m_pApplyAll.get()) && (!sCmd.isEmpty()))
    {
        std::shared_ptr<comphelper::ConfigurationChanges> aBatch(
            comphelper::ConfigurationChanges::create(::comphelper::getProcessComponentContext()));
        officecfg::Office::UI::ToolbarMode::ActiveWriter::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveCalc::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveImpress::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveDraw::set(sCmd, aBatch);
        aBatch->commit();

        Reference<XFrame> xCurrentFrame;
        uno::Reference<uno::XComponentContext> xContext
            = ::comphelper::getProcessComponentContext();
        vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::NONE;
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        OUString sCurrentApp = GetCurrentApp();
        if (pViewFrame)
        {
            xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
            const Reference<frame::XModuleManager> xModuleManager
                = frame::ModuleManager::create(xContext);

            const utl::OConfigurationTreeRoot aAppNode(
                xContext, "org.openoffice.Office.UI.ToolbarMode/Applications/", true);
            if (sCurrentApp != "Writer") //dispatch command does not work if entry has been set
                aAppNode.setNodeValue("Writer/Active", makeAny(sCmd));
            if (sCurrentApp != "Calc")
                aAppNode.setNodeValue("Calc/Active", makeAny(sCmd));
            if (sCurrentApp != "Impress")
                aAppNode.setNodeValue("Impress/Active", makeAny(sCmd));
            if (sCurrentApp != "Draw")
                aAppNode.setNodeValue("Draw/Active", makeAny(sCmd));
            aAppNode.commit();
        };
    }
    if (!sCmd.isEmpty())
        comphelper::dispatchCommand(".uno:ToolbarMode?Mode:string=" + sCmd, {});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */