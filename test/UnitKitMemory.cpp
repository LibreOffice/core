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
 * The admin model knows the memory of an open document when its kit runs without capabilities,
 * and so without a chroot jail.
 */

#include <config.h>

#include <common/JsonUtil.hpp>
#include <common/Unit.hpp>
#include <wsd/Admin.hpp>

#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <thread>

namespace
{
/// The "documents" answer of the admin model, read on the thread that owns the model.
std::string queryDocuments()
{
    auto promise = std::make_shared<std::promise<std::string>>();
    std::future<std::string> future = promise->get_future();
    Admin::instance().addCallback(
        [promise] { promise->set_value(Admin::instance().getModel().query("documents")); });
    if (future.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return std::string();
    return future.get();
}

/// The memory in kilobytes of the first document in a "documents" answer, or 0.
int firstDocumentMemory(const std::string& documents)
{
    Poco::JSON::Object::Ptr object;
    if (!JsonUtil::parseJSON(documents, object))
        return 0;
    Poco::JSON::Array::Ptr list = object->getArray("documents");
    if (!list || list->size() == 0)
        return 0;
    return list->getObject(0)->optValue<int>("memory", 0);
}
}

class UnitKitMemory : public UnitWSD
{
public:
    UnitKitMemory()
        : UnitWSD("UnitKitMemory")
    {
        setTimeout(std::chrono::minutes(2));
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        // Without capabilities the kit sets up no chroot jail.
        config.setBool("security.capabilities", false);
    }

    void invokeWSDTest() override
    {
        std::string documentPath;
        std::string documentURL;
        helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("KitMemoryPoll");
        socketPoll->startThread();
        std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
            socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        // The admin model reads the memory of the kits every few seconds.
        int memoryKb = 0;
        for (int attempt = 0; attempt < 30 && memoryKb <= 0; ++attempt)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            memoryKb = firstDocumentMemory(queryDocuments());
        }
        TST_LOG("The open document uses " << memoryKb << " KB");
        LOK_ASSERT_MESSAGE("The admin model reports no memory for the open document",
                           memoryKb > 0);

        socket->asyncShutdown();
        socketPoll->joinThread();
        exitTest(TestResult::Ok);
    }
};

UnitBase* unit_create_wsd(void) { return new UnitKitMemory(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
