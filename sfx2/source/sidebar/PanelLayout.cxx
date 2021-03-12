/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/log.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/TabBar.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <vcl/event.hxx>

using namespace sfx2::sidebar;

PanelLayout::PanelLayout(weld::Widget* pParent, const OString& rID, const OUString& rUIXMLDescription)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , m_xContainer(m_xBuilder->weld_container(rID))
    , m_pPanel(nullptr)
{
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
    m_pPanel = pPanel;
}

weld::Window* PanelLayout::GetFrameWeld() const
{
    if (!m_pPanel)
    {
        SAL_WARN("sfx.sidebar", "Expected a toplevel Panel to exist");
        return nullptr;
    }
    return m_pPanel->GetFrameWeld();
}

PanelLayout::~PanelLayout()
{
    ::Application::RemoveEventListener(LINK(this, PanelLayout, DataChangedEventListener));

    m_xContainer.reset();
    m_xBuilder.reset();
}

void PanelLayout::queue_resize()
{
    if (!m_xContainer)
        return;
    m_xContainer->queue_resize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
