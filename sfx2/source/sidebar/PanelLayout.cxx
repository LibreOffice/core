/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/TabBar.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/accel.hxx>

using namespace sfx2::sidebar;

PanelLayout::PanelLayout(weld::Widget* pParent, const OString& rID, const OUString& rUIXMLDescription,
                         const css::uno::Reference<css::frame::XFrame> &rFrame)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , m_xContainer(m_xBuilder->weld_container(rID))
    , m_pInitialFocusWidget(nullptr)
    , mxFrame(rFrame)
{
//TODO    SetStyle(GetStyle() | WB_DIALOGCONTROL);
}

#if 0
void PanelLayout::GetFocus()
{
    Control::GetFocus();
    if (m_pInitialFocusWidget)
        m_pInitialFocusWidget->grab_focus();
}
#endif

PanelLayout::~PanelLayout()
{
//TODO    disposeOnce();
}

#if 0
void PanelLayout::dispose()
{
    m_bInClose = true;
    m_pInitialFocusWidget = nullptr;
    m_aPanelLayoutIdle.Stop();
    m_xContainer.reset();
    m_xBuilder.reset();
    m_xVclContentArea.disposeAndClear();
    Control::dispose();
}

Size PanelLayout::GetOptimalSize() const
{
    Size aSize = m_xContainer->get_preferred_size();

    if (mxFrame)
    {
        SidebarController* pController
            = SidebarController::GetSidebarControllerForFrame(mxFrame);
        if (pController)
            aSize.setWidth(std::min<tools::Long>(
                aSize.Width(), (pController->getMaximumWidth() - TabBar::GetDefaultWidth())
                                   * GetDPIScaleFactor()));
    }

    return aSize;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
