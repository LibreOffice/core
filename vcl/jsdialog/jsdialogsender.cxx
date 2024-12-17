/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jsdialog/jsdialogsender.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <tools/json_writer.hxx>
#include <vcl/dockwin.hxx>

JSDialogNotifyIdle::JSDialogNotifyIdle(VclPtr<vcl::Window> aNotifierWindow,
                                       VclPtr<vcl::Window> aContentWindow,
                                       const OUString& sTypeOfJSON)
    : Idle("JSDialog notify")
    , m_aNotifierWindow(std::move(aNotifierWindow))
    , m_aContentWindow(std::move(aContentWindow))
    , m_sTypeOfJSON(sTypeOfJSON)
    , m_bForce(false)
{
    SetPriority(TaskPriority::POST_PAINT);
}

void JSDialogNotifyIdle::forceUpdate() { m_bForce = true; }

void JSDialogNotifyIdle::send(const OString& sMsg)
{
    if (!m_aNotifierWindow)
    {
        return;
    }

    const vcl::ILibreOfficeKitNotifier* pNotifier = m_aNotifierWindow->GetLOKNotifier();
    if (pNotifier)
    {
        if (m_bForce || sMsg != m_LastNotificationMessage)
        {
            m_bForce = false;
            m_LastNotificationMessage = sMsg;
            pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, m_LastNotificationMessage);
        }
    }
}

template <class VclType>
void JSDialogNotifyIdle::sendMessage(const jsdialog::MessageType eType,
                                     const VclPtr<VclType>& pTarget,
                                     std::unique_ptr<jsdialog::ActionDataMap> pData)
{
    std::scoped_lock aGuard(m_aQueueMutex);

    // we want only the latest update of same type
    // TODO: also if we met full update - previous updates are not valid
    auto it = m_aMessageQueue.begin();
    const VclReferenceBase* pRawTarget = static_cast<VclReferenceBase*>(pTarget);

    while (it != m_aMessageQueue.end())
    {
        const VclReferenceBase* pRawWindow = static_cast<VclReferenceBase*>(it->m_pWindow.get());
        const VclReferenceBase* pRawMenu = it->m_pMenu.get();

        if (it->m_eType == eType && (pRawWindow == pRawTarget || pRawMenu == pRawTarget))
        {
            // actions should be always sent, eg. rendering of custom entries in combobox
            if (eType == jsdialog::MessageType::Action)
            {
                it++;
                continue;
            }
            it = m_aMessageQueue.erase(it);
        }
        else
            it++;
    }

    JSDialogMessageInfo aMessage(eType, pTarget, std::move(pData));
    m_aMessageQueue.push_back(aMessage);
}

OString JSDialogNotifyIdle::generateFullUpdate() const
{
    if (!m_aContentWindow || !m_aNotifierWindow)
        return OString();

    tools::JsonWriter aJsonWriter;

    m_aContentWindow->DumpAsPropertyTree(aJsonWriter);
    if (m_aNotifierWindow)
        aJsonWriter.put("id", m_aNotifierWindow->GetLOKWindowId());
    aJsonWriter.put("jsontype", m_sTypeOfJSON);

    return aJsonWriter.finishAndGetAsOString();
}

OString JSDialogNotifyIdle::generateWidgetUpdate(const VclPtr<vcl::Window>& pWindow) const
{
    if (!pWindow || !m_aNotifierWindow)
        return OString();

    tools::JsonWriter aJsonWriter;

    aJsonWriter.put("jsontype", m_sTypeOfJSON);
    aJsonWriter.put("action", "update");
    if (m_aNotifierWindow)
        aJsonWriter.put("id", m_aNotifierWindow->GetLOKWindowId());
    {
        auto aEntries = aJsonWriter.startNode("control");
        pWindow->DumpAsPropertyTree(aJsonWriter);
    }

    return aJsonWriter.finishAndGetAsOString();
}

OString JSDialogNotifyIdle::generateCloseMessage() const
{
    tools::JsonWriter aJsonWriter;
    if (m_aNotifierWindow)
        aJsonWriter.put("id", m_aNotifierWindow->GetLOKWindowId());
    aJsonWriter.put("jsontype", m_sTypeOfJSON);
    aJsonWriter.put("action", "close");

    return aJsonWriter.finishAndGetAsOString();
}

OString
JSDialogNotifyIdle::generateActionMessage(const VclPtr<vcl::Window>& pWindow,
                                          std::unique_ptr<jsdialog::ActionDataMap> pData) const
{
    tools::JsonWriter aJsonWriter;

    aJsonWriter.put("jsontype", m_sTypeOfJSON);
    aJsonWriter.put("action", "action");
    if (m_aNotifierWindow)
        aJsonWriter.put("id", m_aNotifierWindow->GetLOKWindowId());

    {
        auto aDataNode = aJsonWriter.startNode("data");
        aJsonWriter.put("control_id", pWindow->get_id());

        for (auto it = pData->begin(); it != pData->end(); it++)
            aJsonWriter.put(it->first, it->second);
    }

    return aJsonWriter.finishAndGetAsOString();
}

OString JSDialogNotifyIdle::generatePopupMessage(const VclPtr<vcl::Window>& pWindow,
                                                 const OUString& sParentId,
                                                 const OUString& sCloseId) const
{
    if (!pWindow || !m_aNotifierWindow)
        return OString();

    if (!pWindow->GetParentWithLOKNotifier())
        return OString();

    tools::JsonWriter aJsonWriter;

    {
        auto aChildren = aJsonWriter.startArray("children");
        {
            auto aStruct = aJsonWriter.startStruct();
            pWindow->DumpAsPropertyTree(aJsonWriter);
        }
    }

    // try to get the position eg. for the autofilter
    {
        vcl::Window* pVclWindow = pWindow.get();
        DockingWindow* pDockingWindow = dynamic_cast<DockingWindow*>(pVclWindow);
        while (pVclWindow && !pDockingWindow)
        {
            pVclWindow = pVclWindow->GetParent();
            pDockingWindow = dynamic_cast<DockingWindow*>(pVclWindow);
        }

        if (pDockingWindow)
        {
            Point aPos = pDockingWindow->GetFloatingPos();
            aJsonWriter.put("posx", aPos.getX());
            aJsonWriter.put("posy", aPos.getY());
            if (!pDockingWindow->IsVisible())
                aJsonWriter.put("visible", "false");
        }
    }

    aJsonWriter.put("jsontype", "dialog");
    aJsonWriter.put("type", "modalpopup");
    aJsonWriter.put("cancellable", true);
    aJsonWriter.put("popupParent", sParentId);
    aJsonWriter.put("clickToClose", sCloseId);
    aJsonWriter.put("id", pWindow->GetParentWithLOKNotifier()->GetLOKWindowId());

    return aJsonWriter.finishAndGetAsOString();
}

OString JSDialogNotifyIdle::generateClosePopupMessage(const OUString& sWindowId) const
{
    if (!m_aNotifierWindow)
        return OString();

    tools::JsonWriter aJsonWriter;

    aJsonWriter.put("jsontype", "dialog");
    aJsonWriter.put("type", "modalpopup");
    aJsonWriter.put("action", "close");
    aJsonWriter.put("id", sWindowId);

    return aJsonWriter.finishAndGetAsOString();
}

OString JSDialogNotifyIdle::generateMenuMessage(const VclPtr<PopupMenu>& pMenu,
                                                const OUString& sParentId,
                                                const OUString& sCloseId) const
{
    if (!pMenu || !m_aNotifierWindow)
        return OString();

    tools::JsonWriter aJsonWriter;

    {
        auto aChildren = aJsonWriter.startArray("children");
        {
            auto aStruct = aJsonWriter.startStruct();
            pMenu->DumpAsPropertyTree(aJsonWriter);
        }
    }

    aJsonWriter.put("jsontype", "dialog");
    aJsonWriter.put("type", "dropdown");
    aJsonWriter.put("cancellable", true);
    aJsonWriter.put("popupParent", sParentId);
    aJsonWriter.put("clickToClose", sCloseId);
    aJsonWriter.put("id", m_aNotifierWindow->GetLOKWindowId());

    return aJsonWriter.finishAndGetAsOString();
}

void JSDialogNotifyIdle::Invoke()
{
    std::deque<JSDialogMessageInfo> aMessageQueue;
    {
        std::scoped_lock aGuard(m_aQueueMutex);

        std::swap(aMessageQueue, m_aMessageQueue);
    }

    for (auto& rMessage : aMessageQueue)
    {
        jsdialog::MessageType eType = rMessage.m_eType;

        if (m_sTypeOfJSON == "formulabar" && eType != jsdialog::MessageType::Action)
            continue;

        switch (eType)
        {
            case jsdialog::MessageType::FullUpdate:
                send(generateFullUpdate());
                break;

            case jsdialog::MessageType::WidgetUpdate:
                send(generateWidgetUpdate(rMessage.m_pWindow));
                break;

            case jsdialog::MessageType::Close:
                send(generateCloseMessage());
                break;

            case jsdialog::MessageType::Action:
                send(generateActionMessage(rMessage.m_pWindow, std::move(rMessage.m_pData)));
                break;

            case jsdialog::MessageType::Popup:
                send(generatePopupMessage(rMessage.m_pWindow,
                                          (*rMessage.m_pData)[PARENT_ID ""_ostr],
                                          (*rMessage.m_pData)[CLOSE_ID ""_ostr]));
                break;

            case jsdialog::MessageType::PopupClose:
                send(generateClosePopupMessage((*rMessage.m_pData)[WINDOW_ID ""_ostr]));
                break;

            case jsdialog::MessageType::Menu:
            {
                send(generateMenuMessage(rMessage.m_pMenu, (*rMessage.m_pData)[PARENT_ID ""_ostr],
                                         (*rMessage.m_pData)[CLOSE_ID ""_ostr]));
                break;
            }
        }
    }
}

void JSDialogNotifyIdle::clearQueue() { m_aMessageQueue.clear(); }

JSDialogSender::~JSDialogSender() COVERITY_NOEXCEPT_FALSE
{
    sendClose();

    if (mpIdleNotify)
        mpIdleNotify->Stop();
}

void JSDialogSender::sendFullUpdate(bool bForce)
{
    if (!mpIdleNotify)
    {
        assert(false);
        return;
    }

    if (bForce)
        mpIdleNotify->forceUpdate();

    mpIdleNotify->sendMessage(jsdialog::MessageType::FullUpdate, VclPtr<vcl::Window>(nullptr));
    mpIdleNotify->Start();
}

void JSDialogSender::sendClose()
{
    if (!mpIdleNotify || !m_bCanClose)
        return;

    mpIdleNotify->clearQueue();
    mpIdleNotify->sendMessage(jsdialog::MessageType::Close, VclPtr<vcl::Window>(nullptr));
    flush();
}

void JSDialogSender::sendUpdate(const VclPtr<vcl::Window>& pWindow, bool bForce)
{
    if (!mpIdleNotify)
        return;

    if (bForce)
        mpIdleNotify->forceUpdate();

    mpIdleNotify->sendMessage(jsdialog::MessageType::WidgetUpdate, pWindow);
    mpIdleNotify->Start();
}

void JSDialogSender::sendAction(const VclPtr<vcl::Window>& pWindow,
                                std::unique_ptr<jsdialog::ActionDataMap> pData)
{
    if (!mpIdleNotify)
        return;

    mpIdleNotify->sendMessage(jsdialog::MessageType::Action, pWindow, std::move(pData));
    mpIdleNotify->Start();
}

void JSDialogSender::sendPopup(const VclPtr<vcl::Window>& pWindow, const OUString& sParentId,
                               const OUString& sCloseId)
{
    if (!mpIdleNotify)
        return;

    std::unique_ptr<jsdialog::ActionDataMap> pData = std::make_unique<jsdialog::ActionDataMap>();
    (*pData)[PARENT_ID ""_ostr] = sParentId;
    (*pData)[CLOSE_ID ""_ostr] = sCloseId;
    mpIdleNotify->sendMessage(jsdialog::MessageType::Popup, pWindow, std::move(pData));
    mpIdleNotify->Start();
}

void JSDialogSender::sendClosePopup(vcl::LOKWindowId nWindowId)
{
    if (!mpIdleNotify)
        return;

    std::unique_ptr<jsdialog::ActionDataMap> pData = std::make_unique<jsdialog::ActionDataMap>();
    (*pData)[WINDOW_ID ""_ostr] = OUString::number(nWindowId);
    mpIdleNotify->sendMessage(jsdialog::MessageType::PopupClose, VclPtr<vcl::Window>(nullptr),
                              std::move(pData));
    flush();
}

void JSDialogSender::sendMenu(const VclPtr<PopupMenu>& pMenu, const OUString& sParentId,
                              const OUString& sCloseId)
{
    if (!mpIdleNotify)
        return;

    std::unique_ptr<jsdialog::ActionDataMap> pData = std::make_unique<jsdialog::ActionDataMap>();
    (*pData)[PARENT_ID ""_ostr] = sParentId;
    (*pData)[CLOSE_ID ""_ostr] = sCloseId;

    mpIdleNotify->sendMessage(jsdialog::MessageType::Menu, pMenu, std::move(pData));
    mpIdleNotify->Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
