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
 * Global variables and state for WSD process.
 * Functions: getDocBrokers()
 */

#pragma once

#include <common/Clipboard.hpp>
#include <wsd/COOLWSDServer.hpp>
#include <wsd/FileServer.hpp>

#if !MOBILEAPP
#include <wsd/Admin.hpp>

std::unique_ptr<ClipboardCache> COOLWSD::SavedClipboards;

std::unique_ptr<FileServerRequestHandler> COOLWSD::FileRequestHandler;

// Defined before COOLWSDServer::Instance, so it is destroyed after it.
std::unique_ptr<Admin> Admin::Instance;
#endif

std::shared_ptr<TerminatingPoll> COOLWSDServer::WebServerPoll;

std::unique_ptr<COOLWSDServer> COOLWSDServer::Instance;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
