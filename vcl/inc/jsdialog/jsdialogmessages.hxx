/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <deque>

#include <rtl/ustring.hxx>
#include <vcl/idle.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <vcl/menu.hxx>
#include <vcl/window.hxx>

namespace jsdialog
{
enum MessageType
{
    FullUpdate,
    WidgetUpdate,
    Close,
    Action,
    Popup,
    PopupClose,
    Menu,
};
}

/// Class with the message description for storing in the queue
class JSDialogMessageInfo
{
public:
    jsdialog::MessageType m_eType;
    VclPtr<vcl::Window> m_pWindow;
    VclPtr<PopupMenu> m_pMenu;
    std::unique_ptr<jsdialog::ActionDataMap> m_pData;

private:
    void copy(const JSDialogMessageInfo& rInfo)
    {
        this->m_eType = rInfo.m_eType;
        this->m_pWindow = rInfo.m_pWindow;
        this->m_pMenu = rInfo.m_pMenu;
        if (rInfo.m_pData)
        {
            std::unique_ptr<jsdialog::ActionDataMap> pData(
                new jsdialog::ActionDataMap(*rInfo.m_pData));
            this->m_pData = std::move(pData);
        }
    }

public:
    JSDialogMessageInfo(jsdialog::MessageType eType, VclPtr<vcl::Window> pWindow,
                        std::unique_ptr<jsdialog::ActionDataMap> pData)
        : m_eType(eType)
        , m_pWindow(std::move(pWindow))
        , m_pData(std::move(pData))
    {
    }

    JSDialogMessageInfo(jsdialog::MessageType eType, VclPtr<PopupMenu> pMenu,
                        std::unique_ptr<jsdialog::ActionDataMap> pData)
        : m_eType(eType)
        , m_pMenu(std::move(pMenu))
        , m_pData(std::move(pData))
    {
    }

    JSDialogMessageInfo(const JSDialogMessageInfo& rInfo) { copy(rInfo); }

    JSDialogMessageInfo& operator=(JSDialogMessageInfo aInfo)
    {
        if (this == &aInfo)
            return *this;

        copy(aInfo);
        return *this;
    }
};

class JSDialogNotifyIdle final : public Idle
{
    // used to send message
    VclPtr<vcl::Window> m_aNotifierWindow;
    // used to generate JSON
    VclPtr<vcl::Window> m_aContentWindow;
    OUString m_sTypeOfJSON;
    OString m_LastNotificationMessage;
    bool m_bForce;

    std::deque<JSDialogMessageInfo> m_aMessageQueue;
    std::mutex m_aQueueMutex;

public:
    JSDialogNotifyIdle(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       const OUString& sTypeOfJSON);

    void Invoke() override;

    void clearQueue();
    void forceUpdate();
    template <class VclType>
    void sendMessage(jsdialog::MessageType eType, const VclPtr<VclType>& pTarget,
                     std::unique_ptr<jsdialog::ActionDataMap> pData = nullptr);

private:
    void send(const OString& sMsg);
    OString generateFullUpdate() const;
    OString generateWidgetUpdate(VclPtr<vcl::Window> pWindow) const;
    OString generateCloseMessage() const;
    OString generateActionMessage(VclPtr<vcl::Window> pWindow,
                                  std::unique_ptr<jsdialog::ActionDataMap> pData) const;
    OString generatePopupMessage(VclPtr<vcl::Window> pWindow, const rtl::OUString& sParentId,
                                 const rtl::OUString& sCloseId) const;
    OString generateClosePopupMessage(const rtl::OUString& sWindowId) const;
    OString generateMenuMessage(const VclPtr<PopupMenu>& pMenu) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
