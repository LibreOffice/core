/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <jsdialog/jsdialogregister.hxx>
#include <jsdialog/jsdialogmessages.hxx>

#include <utility>
#include <vcl/weld.hxx>
#include <vcl/virdev.hxx>
#include <salvtables.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/fmtfield.hxx>
#include <vcl/toolkit/prgsbar.hxx>

#include <comphelper/compbase.hxx>

#include <list>
#include <mutex>

#define ACTION_TYPE "action_type"
#define PARENT_ID "parent_id"
#define WINDOW_ID "id"
#define CLOSE_ID "close_id"
#define MENU_PTR "menu_ptr"

namespace vcl
{
class ILibreOfficeKitNotifier;
}

class JSDialogSender
{
    std::unique_ptr<JSDialogNotifyIdle> mpIdleNotify;

protected:
    bool m_bCanClose; // specifies if can send a close message

public:
    JSDialogSender()
        : m_bCanClose(true)
    {
    }
    JSDialogSender(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                   const OUString& sTypeOfJSON)
        : m_bCanClose(true)
    {
        initializeSender(aNotifierWindow, aContentWindow, sTypeOfJSON);
    }

    virtual ~JSDialogSender() COVERITY_NOEXCEPT_FALSE;

    virtual void sendFullUpdate(bool bForce = false);
    void sendClose();
    void sendUpdate(VclPtr<vcl::Window> pWindow, bool bForce = false);
    virtual void sendAction(VclPtr<vcl::Window> pWindow,
                            std::unique_ptr<jsdialog::ActionDataMap> pData);
    virtual void sendPopup(VclPtr<vcl::Window> pWindow, OUString sParentId, OUString sCloseId);
    virtual void sendClosePopup(vcl::LOKWindowId nWindowId);
    void flush() { mpIdleNotify->Invoke(); }

protected:
    void initializeSender(const VclPtr<vcl::Window>& rNotifierWindow,
                          const VclPtr<vcl::Window>& rContentWindow, const OUString& rTypeOfJSON)
    {
        mpIdleNotify.reset(new JSDialogNotifyIdle(rNotifierWindow, rContentWindow, rTypeOfJSON));
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
