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
 * Common fuzzing initialization and utilities.
 * Functions: DoInitialization() - Sets up fuzzing environment
 */

#include <config.h>

#include <fuzzer/Common.hpp>

#include <common/Clipboard.hpp>
#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <net/Ssl.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/FileServer.hpp>

#include <Poco/AutoPtr.h>

#include <cstdlib>
#include <map>
#include <memory>
#include <string>

namespace fuzzer
{
bool DoInitialization()
{
    ConfigUtil::initializeFromFile("coolwsd.xml");

    COOLWSD::ChildRoot = FileUtil::getSysTempDirectoryPath();
    COOLWSD::SavedClipboards = std::make_unique<ClipboardCache>();
    COOLWSD::FileRequestHandler =
        std::make_unique<FileServerRequestHandler>(COOLWSD::FileServerRoot);

    const char* level = std::getenv("LOG_LEVEL");
    const std::string logLevel(level ? level : "fatal");
    bool withColor = false;
    bool logToFile = false;
    std::map<std::string, std::string> logProperties;
    Log::initialize("wsd", logLevel, withColor, logToFile, logProperties, false, {});
    ssl::Manager::initializeClientContext(
            /*certificateFile=*/"", /*privateKeyFile=*/"", /*caLocation=*/"",
            /*cipherList=*/"", ssl::CertificateVerification::Required);
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
