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

RemoteDialog::RemoteDialog(weld::Window* pWindow)
    : GenericDialogController(pWindow, u"modules/simpress/ui/remotedialog.ui"_ustr,
                              u"RemoteDialog"_ustr)
    , m_xButtonConnect(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xClientBox(new sd::ClientBox(m_xBuilder->weld_scrolled_window(u"scroll"_ustr),
                                     m_xBuilder->weld_container(u"tree"_ustr)))
{
    m_xButtonConnect->connect_clicked(LINK(this, RemoteDialog, HandleConnectButton));
}

RemoteDialog::~RemoteDialog() {}

IMPL_LINK_NOARG(RemoteDialog, HandleConnectButton, weld::Button&, void)
{
    weld::WaitObject(m_xDialog.get());
#if defined(ENABLE_SDREMOTE)
    auto xEntry = m_xClientBox->GetActiveEntry();
    if (!xEntry)
        return;
    OUString aPin = xEntry->m_xPinBox->get_text();
    if (IPRemoteServer::connectClient(xEntry->m_xClientInfo, aPin))
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
