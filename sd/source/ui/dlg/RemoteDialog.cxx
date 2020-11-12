/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "RemoteDialog.hxx"
#include <RemoteServer.hxx>

using namespace ::sd;
using namespace ::std;

RemoteDialog::RemoteDialog(weld::Window* pWindow)
    : GenericDialogController(pWindow, "modules/simpress/ui/remotedialog.ui", "RemoteDialog")
    , m_xButtonConnect(m_xBuilder->weld_button("ok"))
    , m_xClientBox(new sd::ClientBox(m_xBuilder->weld_scrolled_window("scroll"),
                                     m_xBuilder->weld_container("tree")))
{
    m_xButtonConnect->connect_clicked(LINK(this, RemoteDialog, HandleConnectButton));
}

RemoteDialog::~RemoteDialog() {}

IMPL_LINK_NOARG(RemoteDialog, HandleConnectButton, weld::Button&, void)
{
    weld::WaitObject(m_xDialog.get());
#if defined(ENABLE_SDREMOTE) && defined(ENABLE_SDREMOTE_BLUETOOTH)
    auto xEntry = m_xClientBox->GetActiveEntry();
    if (!xEntry)
        return;
    OUString aPin = xEntry->m_xPinBox->get_text();
    if (RemoteServer::connectClient(xEntry->m_xClientInfo, aPin))
        m_xDialog->response(RET_OK);
#endif
}

short RemoteDialog::run()
{
    short nRet = weld::GenericDialogController::run();
#ifdef ENABLE_SDREMOTE
    RemoteServer::restoreDiscoverable();
#endif
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
