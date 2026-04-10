/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Stub missing symbols required for unit tests ...
 */

#include <config.h>

#include <common/Log.hpp>
#include <wsd/AIChatSession.hpp>
#include <wsd/Admin.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>

void DocumentBroker::assertCorrectThread(LOG_CAPTURE_CALLER) const {}

void ClientSession::enqueueSendMessage(const std::shared_ptr<Message>& /*data*/) {};

#if !MOBILEAPP
AIChatSession::AIChatSession(ClientSession& session) : _session(session) {}
AIChatSession::~AIChatSession() = default;
#endif

ClientSession::~ClientSession() = default;

void ClientSession::onDisconnect() {}

bool ClientSession::hasQueuedMessages() const { return false; }

void ClientSession::writeQueuedMessages(std::size_t) {}

void ClientSession::dumpState(std::ostream& /*os*/) {}

void ClientSession::setReadOnly(bool) {}

bool ClientSession::_handleInput(const char* /*buffer*/, int /*length*/) { return false; }

Admin::~Admin() {}
std::unique_ptr<Admin> Admin::Instance;
void Admin::dumpState(std::ostream&) const {}
void Admin::pollingThread() {}
AdminModel::~AdminModel() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
