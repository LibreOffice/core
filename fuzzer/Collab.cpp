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

#include <config.h>

#include "CollabBroker.hpp"
#include "CollabSocketHandler.hpp"

#include <COOLWSD.hpp>
#include <common/Anonymizer.hpp>
#include <common/Log.hpp>
#include <common/SigUtil.hpp>
#include <common/Unit.hpp>
#include <fuzzer/Common.hpp>
#include <wsd/Admin.hpp>

#include <cstdlib>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

extern "C" int LLVMFuzzerInitialize(int* /*argc*/, char*** /*argv*/)
{
    COOLWSD::ChildRoot = "/tmp/collab-fuzzer/child-root";
    UnitBase::init(UnitBase::UnitType::Wsd, std::string());

    fuzzer::DoInitialization();
    Admin::initialize();

    std::atexit(
        []
        {
            SigUtil::setTerminationFlag();
            Admin::uninitialize();
            Log::shutdown();
        });

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Create a handler for the incoming messages.
    const std::string wopiSrc = "http://localhost/wopi/files/fuzz";
    const std::string docKey = "/fuzz/fuzz.odt";
    auto broker = std::make_shared<CollabBroker>(docKey, wopiSrc);
    auto handler = std::make_shared<CollabSocketHandler>(wopiSrc, docKey, broker);

    // Feed the messages into the handler, one message per line.
    std::string input(reinterpret_cast<const char*>(data), size);
    std::stringstream ss(input);
    std::string line;
    while (std::getline(ss, line, '\n'))
        handler->handleAuthenticatedMessage(line);

    // Clean up state to avoid OOM when this function is called many times.
    handler.reset();
    broker.reset();
    {
        std::lock_guard<std::mutex> lock(CollabFetchRequestsMutex);
        CollabFetchRequests.clear();
    }
    {
        std::lock_guard<std::mutex> lock(CollabUploadRequestsMutex);
        CollabUploadRequests.clear();
    }
    Admin::instance().poll(std::chrono::microseconds(0));
    Anonymizer::clear();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
