/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/TabBar.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <vcl/event.hxx>

using namespace sfx2::sidebar;

PanelLayout::PanelLayout(weld::Widget* pParent, const OString& rID, const OUString& rUIXMLDescription,
                         const css::uno::Reference<css::frame::XFrame> &rFrame)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , m_xContainer(m_xBuilder->weld_container(rID))
    , m_pInitialFocusWidget(nullptr)
    , mxFrame(rFrame)
{
//TODO    SetStyle(GetStyle() | WB_DIALOGCONTROL);
    m_xContainer->set_background(Theme::GetColor(Theme::Color_PanelBackground));
    m_xContainer->connect_get_property_tree(LINK(this, PanelLayout, DumpAsPropertyTreeHdl));
    ::Application::AddEventListener(LINK(this, PanelLayout, DataChangedEventListener));
}

IMPL_LINK(PanelLayout, DumpAsPropertyTreeHdl, tools::JsonWriter&, rJsonWriter, void)
{
    DumpAsPropertyTree(rJsonWriter);
}

void PanelLayout::DumpAsPropertyTree(tools::JsonWriter&)
{
}

IMPL_LINK(PanelLayout, DataChangedEventListener, VclSimpleEvent&, rEvent, void)
{
    if (rEvent.GetId() != VclEventId::ApplicationDataChanged)
        return;

    DataChangedEvent* pData = static_cast<DataChangedEvent*>(static_cast<VclWindowEvent&>(rEvent).GetData());
    DataChanged(*pData);
}

void PanelLayout::DataChanged(const DataChangedEvent& rEvent)
{
    if (rEvent.GetType() != DataChangedEventType::SETTINGS)
        return;
    if (rEvent.GetFlags() & AllSettingsFlags::STYLE)
        m_xContainer->set_background(Theme::GetColor(Theme::Color_PanelBackground));
}

void PanelLayout::SetPanel(sfx2::sidebar::Panel* pPanel)
{
    m_xPanel = pPanel;
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
    ::Application::RemoveEventListener(LINK(this, PanelLayout, DataChangedEventListener));

    m_pInitialFocusWidget = nullptr;
    m_xContainer.reset();
    m_xBuilder.reset();
}

void PanelLayout::queue_resize()
{
    if (!m_xContainer)
        return;
    m_xContainer->queue_resize();
}

#if 0
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
