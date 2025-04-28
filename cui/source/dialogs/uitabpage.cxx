/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <uitabpage.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <comphelper/types.hxx>
#include <dialmgr.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/confignode.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/EnumContext.hxx>

std::unique_ptr<SfxTabPage> UITabPage::Create(weld::Container* pPage,
                                              weld::DialogController* pController,
                                              const SfxItemSet* rAttr)
{
    return std::make_unique<UITabPage>(pPage, pController, *rAttr);
}

UITabPage::UITabPage(weld::Container* pPage, weld::DialogController* pController,
                     const SfxItemSet& rAttr)
    : SfxTabPage(pPage, pController, u"cui/ui/uitabpage.ui"_ustr, u"UITabPage"_ustr, &rAttr)
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
    static_assert(SAL_N_ELEMENTS(m_pRadioButtons) == std::size(UIMODES_ARRAY));

    Link<weld::Toggleable&, void> aLink = LINK(this, UITabPage, SelectUIMode);

    const OUString sCurrentMode = GetCurrentMode();
    for (std::size_t i = 0; i < std::size(m_pRadioButtons); ++i)
    {
        m_pRadioButtons[i]->connect_toggled(aLink);
        if (sCurrentMode == std::get<1>(UIMODES_ARRAY[i]))
        {
            m_pRadioButtons[i]->set_active(true);
            UpdateImage(std::get<2>(UIMODES_ARRAY[i]));
            m_pInfoLabel->set_label(CuiResId(std::get<0>(UIMODES_ARRAY[i])));
        }
    }

    if (!officecfg::Office::Common::Misc::ExperimentalMode::get())
    {
        m_pRadioButtons[nGroupedbarFull]->set_visible(false);
        m_pRadioButtons[nContextualGroups]->set_visible(false);
    }
}

OUString UITabPage::GetCurrentMode()
{
    OUString sResult;
    if (SfxViewFrame::Current())
    {
        const auto& xContext = comphelper::getProcessComponentContext();
        const utl::OConfigurationTreeRoot aAppNode(
            xContext, "org.openoffice.Office.UI.ToolbarMode/Applications/" + GetCurrentApp(), true);
        if (aAppNode.isValid())
            sResult = comphelper::getString(aAppNode.getNodeValue(u"Active"_ustr));
    };
    return sResult;
}

OUString UITabPage::GetCurrentApp()
{
    OUString sResult;
    if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
    {
        const auto xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
        const auto& xContext = comphelper::getProcessComponentContext();
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

int UITabPage::GetActiveRadioButton()
{
    const int nSize = std::size(m_pRadioButtons);
    for (int i = 0; i < nSize; ++i)
    {
        if (m_pRadioButtons[i]->get_active())
            return i;
    }
    return -1;
}

const OUString& UITabPage::GetSelectedMode()
{
    int i = GetActiveRadioButton();
    return std::get<1>(UIMODES_ARRAY[i]);
}

static bool file_exists(const OUString& fileName)
{
    osl::File aFile(fileName);
    return aFile.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None;
}

void UITabPage::UpdateImage(std::u16string_view sFileName)
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

IMPL_LINK_NOARG(UITabPage, SelectUIMode, weld::Toggleable&, void)
{
    const int i = GetActiveRadioButton();
    if (i > -1)
    {
        UpdateImage(std::get<2>(UIMODES_ARRAY[i]));
        m_pInfoLabel->set_label(CuiResId(std::get<0>(UIMODES_ARRAY[i])));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
