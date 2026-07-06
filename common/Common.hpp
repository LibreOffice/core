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

#pragma once

#include <vector>
#include <memory>

// Default values and other shared data between processes.

// One kit process hosts several open documents at once, each identified by its
// mobileAppDocId. True for iOS and the CODA desktop apps.
// The plain server and Android instead run one document per process.
#if defined(IOS) || defined(QTAPP) || defined(MACOS) || defined(_WIN32)
#define DOCS_SHARE_PROCESS 1
#else
#define DOCS_SHARE_PROCESS 0
#endif

constexpr int DEFAULT_CLIENT_PORT_NUMBER = 9980;

// define to wrap strace around the forkit
#define STRACE_COOLFORKIT 0
// define to wrap valgrind around the forkit
#define VALGRIND_COOLFORKIT 0

#if VALGRIND_COOLFORKIT
constexpr int TRACE_MULTIPLIER = 20;
#elif CODE_COVERAGE
constexpr int TRACE_MULTIPLIER = 5;
#else
constexpr int TRACE_MULTIPLIER = 1;
#endif

constexpr int COMMAND_TIMEOUT_SECS = 5 * TRACE_MULTIPLIER;
constexpr int COMMAND_TIMEOUT_MS = COMMAND_TIMEOUT_SECS * 1000;
constexpr int CHILD_TIMEOUT_MS = COMMAND_TIMEOUT_MS;
constexpr int CHILD_REBALANCE_INTERVAL_MS = CHILD_TIMEOUT_MS / 10;
constexpr int CHILD_SPAWN_TIMEOUT_MS = CHILD_TIMEOUT_MS * 4;
constexpr int POLL_TIMEOUT_MICRO_S = (COMMAND_TIMEOUT_MS / 5) * 1000;
constexpr int POLL_FORKIT_TIMEOUT_SECS = 5;
constexpr int WS_SEND_TIMEOUT_MS = 1000 * TRACE_MULTIPLIER;
constexpr int CLIPBOARD_EXPIRY_MINUTES = 10;

constexpr int TILE_ROUNDTRIP_TIMEOUT_MS = 10 * TRACE_MULTIPLIER * 1000;

/// Pipe and Socket read buffer size.
/// Should be large enough for ethernet packets
/// which can be 1500 bytes long.
constexpr long READ_BUFFER_SIZE = 64 * 1024;

/// Message larger than this will be dropped as invalid
/// or as intentionally flooding the server.
constexpr int MAX_MESSAGE_SIZE = 2 * 1024 * READ_BUFFER_SIZE;

/// Limits number of HTTP redirections to prevent endless redirection loops.
static constexpr int HTTP_REDIRECTION_LIMIT = 21;

constexpr const char JAILED_DOCUMENT_ROOT[] = "/tmp/user/docs/";
constexpr const char JAILED_CONFIG_ROOT[] = "/tmp/user/user/";
constexpr const char CHILD_URI[] = "/coolws/child?";
constexpr const char NEW_CHILD_URI[] = "/coolws/newchild";
constexpr const char FORKIT_URI[] = "/coolws/forkit";

constexpr const char CAPABILITIES_END_POINT[] = "/hosting/capabilities";
constexpr const char SETTING_IFRAME_END_POINT[] = "adminIntegratorSettings.html";

/// The file suffix used to mark the file slated for uploading.
constexpr const char TO_UPLOAD_SUFFIX[] = ".upload";
/// The file suffix used to mark the file being uploaded.
constexpr const char UPLOADING_SUFFIX[] = "ing";

/// A shared threadname suffix in both the WSD and Kit processes
/// is highly helpful for filtering the logs for the same document
/// by simply grepping for this shared suffix+ID. e.g. 'grep "broker_123" coolwsd.log'
/// Unfortunately grepping for only "_123" would include more noise than desirable.
/// This also makes the threadname symmetric and the entries aligned.
/// The choice of "broker" as the suffix is historic: it implies the controller
/// of which there are two: one in WSD called DocumentBroker and one in Kit
/// called Document, which wasn't called DocumentBroker to avoid confusing it
/// with the one in WSD. No such confusion should be expected in the logs, since
/// the prefix is "doc" and "kit" respectively, and each log entry has the process
/// name prefixed. And of course these threads are unrelated to the classes in
/// the code: they are logical execution unit names.
#define SHARED_DOC_THREADNAME_SUFFIX "broker_"

/// The client port number, both coolwsd and the kits have this.
extern int ClientPortNumber;
class UnxSocketPath;
extern UnxSocketPath MasterLocation;

/// Controls whether experimental features/behavior is enabled or not.
extern bool EnableExperimental;

/// More efficient use of vectors
using BlobData = std::vector<char>;
using Blob = std::shared_ptr<BlobData>;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
