/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant - Child Window for ChatPanel
 */

#ifndef INCLUDED_OFFICELABS_CHATPANELCHILDWINDOW_HXX
#define INCLUDED_OFFICELABS_CHATPANELCHILDWINDOW_HXX

#include <sfx2/childwin.hxx>

namespace officelabs {

class ChatPanelChildWindow final : public SfxChildWindow
{
public:
    ChatPanelChildWindow(vcl::Window* pParent, sal_uInt16 nId,
                         SfxBindings* pBindings, SfxChildWinInfo* pInfo);
    virtual ~ChatPanelChildWindow() override;

    SFX_DECL_CHILDWINDOW_WITHID(ChatPanelChildWindow);
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_CHATPANELCHILDWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
