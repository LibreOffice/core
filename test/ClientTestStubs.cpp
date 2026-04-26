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

// Stubs for symbols not provided by the wsd_sources used in clienttest.

#include <config.h>

#include <memory>
#include <string>

#include <kit/BgSaveWatchDog.hpp>
#include <wsd/COOLWSD.hpp>
#include <common/Clipboard.hpp>
#include <wsd/COOLWSDServer.hpp>
#include <wsd/RequestDetails.hpp>

// ----- Kit stubs -----
#include <kit/ChildSession.hpp>
bool ChildSession::NoCapsForKit = true;



// ----- COOLWSD stubs -----
// coverity[global_init_order] - test stub, never populated, destructor is a no-op
std::unique_ptr<ClipboardCache> COOLWSD::SavedClipboards;
int createForkit(const std::string&, const StringVector&) { return -1; }

// ----- COOLWSDServer stubs -----
// coverity[global_init_order] - test stub, never populated, destructor is a no-op
std::unique_ptr<COOLWSDServer> COOLWSDServer::Instance;
std::shared_ptr<TerminatingPoll> COOLWSDServer::WebServerPoll;

// ----- COOLWSD::FileRequestHandler (needs full FileServerRequestHandler type) -----
#include <wsd/FileServer.hpp>
std::unique_ptr<FileServerRequestHandler> COOLWSD::FileRequestHandler;

// ----- RequestDetails stubs -----
std::string RequestDetails::getDocKey(const Poco::URI&) { return {}; }

/* vim:set shiftwidth=4 expandtab: */
