/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant - Agentic Panel Child Window Implementation
 */

#include <officelabs/AgenticPanelChildWindow.hxx>
#include <officelabs/AgenticPanel.hxx>
#include <sfx2/sfxsids.hrc>

namespace officelabs {

// Replace the old ChatPanel with AgenticPanel
// Use the same SID_AI_CHAT so the menu item opens our new panel
SFX_IMPL_CHILDWINDOW_WITHID(AgenticPanelChildWindow, SID_AI_CHAT);

AgenticPanelChildWindow::AgenticPanelChildWindow(vcl::Window* pParent, sal_uInt16 nId,
                                                 SfxBindings* pBindings, SfxChildWinInfo* pInfo)
    : SfxChildWindow(pParent, nId)
{
    VclPtr<AgenticPanel> pPanel = VclPtr<AgenticPanel>::Create(pBindings, this, pParent);
    SetWindow(pPanel);
    SetAlignment(SfxChildAlignment::RIGHT);
    pPanel->SetSizePixel(Size(420, 0));  // Width of 420px, height fills available space
    pPanel->Initialize(pInfo);
}

AgenticPanelChildWindow::~AgenticPanelChildWindow()
{
}

SfxChildWinInfo AgenticPanelChildWindow::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}

} // namespace officelabs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
