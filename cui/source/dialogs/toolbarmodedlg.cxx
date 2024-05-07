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
#include <comphelper/types.hxx>
#include <dialmgr.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <sfx2/viewfrm.hxx>
#include <strings.hrc>
#include <unotools/confignode.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

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
    if (SfxViewFrame::Current())
    {
        const auto xContext = comphelper::getProcessComponentContext();
        const utl::OConfigurationTreeRoot aAppNode(
            xContext, "org.openoffice.Office.UI.ToolbarMode/Applications/" + GetCurrentApp(), true);
        if (aAppNode.isValid())
            sResult = comphelper::getString(aAppNode.getNodeValue("Active"));
    };
    return sResult;
}

ToolbarmodeDialog::ToolbarmodeDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"cui/ui/toolbarmodedialog.ui"_ustr,
                              u"ToolbarmodeDialog"_ustr)
    , m_pImage(m_xBuilder->weld_image(u"imImage"_ustr))
    , m_pApply(m_xBuilder->weld_button(u"btnApply"_ustr))
    , m_pApplyAll(m_xBuilder->weld_button(u"btnApplyAll"_ustr))
    , m_pRadioButtons{ (m_xBuilder->weld_radio_button(u"rbButton1"_ustr)),
                       (m_xBuilder->weld_radio_button(u"rbButton2"_ustr)),
                       (m_xBuilder->weld_radio_button(u"rbButton3"_ustr)),
                       (m_xBuilder->weld_radio_button(u"rbButton4"_ustr)),
                       (m_xBuilder->weld_radio_button(u"rbButton5"_ustr)),
                       (m_xBuilder->weld_radio_button(u"rbButton6"_ustr)),
                       (m_xBuilder->weld_radio_button(u"rbButton7"_ustr)),
                       (m_xBuilder->weld_radio_button(u"rbButton8"_ustr)),
                       (m_xBuilder->weld_radio_button(u"rbButton9"_ustr)) }
    , m_pInfoLabel(m_xBuilder->weld_label(u"lbInfo"_ustr))
{
    static_assert(SAL_N_ELEMENTS(m_pRadioButtons) == std::size(TOOLBARMODES_ARRAY));

    Link<weld::Toggleable&, void> aLink = LINK(this, ToolbarmodeDialog, SelectToolbarmode);

    const OUString sCurrentMode = GetCurrentMode();
    for (std::size_t i = 0; i < std::size(m_pRadioButtons); ++i)
    {
        m_pRadioButtons[i]->connect_toggled(aLink);
        if (sCurrentMode == std::get<1>(TOOLBARMODES_ARRAY[i]))
        {
            m_pRadioButtons[i]->set_active(true);
            UpdateImage(std::get<2>(TOOLBARMODES_ARRAY[i]));
            m_pInfoLabel->set_label(CuiResId(std::get<0>(TOOLBARMODES_ARRAY[i])));
        }
    }

    m_pApply->set_label(CuiResId(RID_CUISTR_UI_APPLYALL).replaceFirst("%MODULE", GetCurrentApp()));
    m_pApply->connect_clicked(LINK(this, ToolbarmodeDialog, OnApplyClick));
    m_pApplyAll->connect_clicked(LINK(this, ToolbarmodeDialog, OnApplyClick));

    if (!officecfg::Office::Common::Misc::ExperimentalMode::get())
    {
        m_pRadioButtons[nGroupedbarFull]->set_visible(false);
        m_pRadioButtons[nContextualGroups]->set_visible(false);
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
    for (std::size_t i = 0; i < std::size(m_pRadioButtons); ++i)
    {
        if (m_pRadioButtons[i]->get_active())
            return i;
    }
    return -1;
}

void ToolbarmodeDialog::UpdateImage(std::u16string_view sFileName)
{
    // load image
    OUString aURL(u"$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/toolbarmode/"_ustr);
    rtl::Bootstrap::expandMacros(aURL);
    aURL += sFileName;
    if (sFileName.empty() || !file_exists(aURL))
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

IMPL_LINK_NOARG(ToolbarmodeDialog, SelectToolbarmode, weld::Toggleable&, void)
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
            comphelper::ConfigurationChanges::create());
        officecfg::Office::UI::ToolbarMode::ActiveWriter::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveCalc::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveImpress::set(sCmd, aBatch);
        officecfg::Office::UI::ToolbarMode::ActiveDraw::set(sCmd, aBatch);
        aBatch->commit();

        OUString sCurrentApp = GetCurrentApp();
        if (SfxViewFrame::Current())
        {
            const auto xContext = comphelper::getProcessComponentContext();
            const utl::OConfigurationTreeRoot aAppNode(
                xContext, u"org.openoffice.Office.UI.ToolbarMode/Applications/"_ustr, true);
            if (sCurrentApp != "Writer")
                aAppNode.setNodeValue("Writer/Active", css::uno::Any(sCmd));
            if (sCurrentApp != "Calc")
                aAppNode.setNodeValue("Calc/Active", css::uno::Any(sCmd));
            if (sCurrentApp != "Impress")
                aAppNode.setNodeValue("Impress/Active", css::uno::Any(sCmd));
            if (sCurrentApp != "Draw")
                aAppNode.setNodeValue("Draw/Active", css::uno::Any(sCmd));
            aAppNode.commit();
        };
    }
    //apply to current module
    comphelper::dispatchCommand(".uno:ToolbarMode?Mode:string=" + sCmd, {});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
