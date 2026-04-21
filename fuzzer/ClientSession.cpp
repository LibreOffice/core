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
 * Fuzzer for ClientSession message handling.
 * Functions: LLVMFuzzerTestOneInput() - Tests ClientSession::_handleInput()
 */

#include <config.h>

#include "ClientSession.hpp"

#include <common/Anonymizer.hpp>
#include <fuzzer/Common.hpp>

bool DoInitialization()
{
    COOLWSD::ChildRoot = "/tmp/clientsession-fuzzer/child-root";
    UnitBase::init(UnitBase::UnitType::Wsd, std::string());

    fuzzer::DoInitialization();
    return true;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    static bool initialized = DoInitialization();
    (void)initialized;

    std::string uri;
    Poco::URI uriPublic;
    std::string docKey = "/fuzz/fuzz.odt";
    auto docBroker = std::make_shared<DocumentBroker>(DocumentBroker::ChildType::Interactive, uri,
                                                      uriPublic, docKey, "", 0);

    std::shared_ptr<ProtocolHandlerInterface> ws;
    std::string id;
    bool isReadOnly = false;
    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri,
                                   Poco::Net::HTTPMessage::HTTP_1_1);
    request.setHost("localhost:9980");
    const RequestDetails requestDetails(request, "");
    auto session
        = std::make_shared<ClientSession>(ws, id, docBroker, uriPublic, isReadOnly, requestDetails);

    std::string input(reinterpret_cast<const char*>(data), size);
    std::stringstream ss(input);
    std::string line;
    while (std::getline(ss, line, '\n'))
    {
        std::vector<char> lineVector(line.data(), line.data() + line.size());
        session->handleMessage(lineVector);
    }

    // The DocumentBroker dtor grows SocketPoll::_newCallbacks.
    docBroker.reset();

    // Make sure SocketPoll::_newCallbacks does not grow forever, leading to OOM.
    Admin::instance().poll(std::chrono::microseconds(0));

    // Make sure the anon map does not grow forever, leading to OOM.
    Anonymizer::clear();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
