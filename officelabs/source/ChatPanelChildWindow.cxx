/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant - Child Window Implementation
 */

#include <officelabs/ChatPanelChildWindow.hxx>
#include <officelabs/ChatPanel.hxx>
#include <sfx2/sfxsids.hrc>

namespace officelabs {

// DISABLED: Replaced by AgenticPanel which uses the same SID
// SFX_IMPL_CHILDWINDOW_WITHID(ChatPanelChildWindow, SID_AI_CHAT);

ChatPanelChildWindow::ChatPanelChildWindow(vcl::Window* pParent, sal_uInt16 nId,
                                           SfxBindings* pBindings, SfxChildWinInfo* pInfo)
    : SfxChildWindow(pParent, nId)
{
    VclPtr<ChatPanel> pPanel = VclPtr<ChatPanel>::Create(pBindings, this, pParent);
    SetWindow(pPanel);
    SetAlignment(SfxChildAlignment::RIGHT);
    pPanel->SetSizePixel(Size(350, 0));  // Width of 350px, height fills available space
    pPanel->Initialize(pInfo);
}

ChatPanelChildWindow::~ChatPanelChildWindow()
{
}

SfxChildWinInfo ChatPanelChildWindow::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}

} // namespace officelabs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
