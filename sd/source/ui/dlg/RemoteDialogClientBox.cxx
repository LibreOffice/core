/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vector>

#include <svtools/controldims.hxx>

#include "RemoteDialogClientBox.hxx"
#include <RemoteServer.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

#include <sdresid.hxx>
#include <strings.hrc>

using namespace std;

using namespace ::com::sun::star;

namespace sd {

//                          struct ClientBoxEntry

ClientBoxEntry::ClientBoxEntry(ClientBox* pClientBox,
                               const std::shared_ptr<ClientInfo>& pClientInfo)
    : m_xBuilder(Application::CreateBuilder(pClientBox->GetContainer(), "modules/simpress/ui/clientboxfragment.ui"))
    , m_xContainer(m_xBuilder->weld_container("ClientboxFragment"))
    , m_xDeviceName(m_xBuilder->weld_label("name"))
    , m_xPinLabel(m_xBuilder->weld_label("pinlabel"))
    , m_xPinBox(m_xBuilder->weld_entry("pin"))
    , m_xDeauthoriseButton(m_xBuilder->weld_button("button"))
    , m_xClientInfo(pClientInfo)
    , m_pClientBox(pClientBox)
{
    m_xDeviceName->set_label(m_xClientInfo->mName);
    m_xDeauthoriseButton->connect_clicked(LINK(this, ClientBoxEntry, DeauthoriseHdl));
    m_xDeauthoriseButton->set_visible(m_xClientInfo->mbIsAlreadyAuthorised);
    m_xPinBox->set_visible(!m_xClientInfo->mbIsAlreadyAuthorised);
    m_xPinLabel->set_visible(!m_xClientInfo->mbIsAlreadyAuthorised);

    m_xDeauthoriseButton->connect_focus_in(LINK(this, ClientBoxEntry, FocusHdl));
    m_xPinBox->connect_focus_in(LINK(this, ClientBoxEntry, FocusHdl));
}

ClientBoxEntry::~ClientBoxEntry()
{
    m_pClientBox->GetContainer()->move(m_xContainer.get(), nullptr);
}

// ClientBox
ClientBox::ClientBox(std::unique_ptr<weld::ScrolledWindow> xScroll,
                     std::unique_ptr<weld::Container> xContents)
    : m_xScroll(std::move(xScroll))
    , m_xContents(std::move(xContents))
    , m_pActive(nullptr)
{
    Size aSize(m_xScroll->get_approximate_digit_width() * 40,
               m_xScroll->get_text_height() * 16);
    m_xScroll->set_size_request(aSize.Width(), aSize.Height());

    m_xContents->set_stack_background();

    populateEntries();
}

ClientBox::~ClientBox()
{
}

ClientBoxEntry* ClientBox::GetActiveEntry()
{
    return m_pActive;
}

void ClientBox::addEntry( const std::shared_ptr<ClientInfo>& pClientInfo )
{
    TClientBoxEntry xEntry(new ClientBoxEntry(this, pClientInfo));
    m_vEntries.push_back(xEntry);
}

void ClientBox::setActive(ClientBoxEntry* pClientEntry)
{
    m_pActive = pClientEntry;
}

void ClientBox::clearEntries()
{
    m_vEntries.clear();
    m_pActive = nullptr;
}

void ClientBox::populateEntries()
{
    clearEntries();

#ifdef ENABLE_SDREMOTE
    RemoteServer::ensureDiscoverable();

    vector< std::shared_ptr< ClientInfo > > aClients( RemoteServer::getClients() );

    for ( const auto& rxClient : aClients )
    {
        addEntry( rxClient );
    }
#endif

}

IMPL_LINK_NOARG(ClientBoxEntry, DeauthoriseHdl, weld::Button&, void)
{
#ifdef ENABLE_SDREMOTE
    RemoteServer::deauthoriseClient(m_xClientInfo);
#endif
    m_pClientBox->populateEntries();
}

IMPL_LINK_NOARG(ClientBoxEntry, FocusHdl, weld::Widget&, void)
{
    m_pClientBox->setActive(this);
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
