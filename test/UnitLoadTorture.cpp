/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Unit test for stress testing document loading under heavy load.
 */

#include <config.h>

#include <Unit.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>
#include <WebSocketSession.hpp>
#include <test/lokassert.hpp>

#include <memory>
#include <string>

using namespace std::literals;

/// Load torture testcase.
class UnitLoadTorture : public UnitWSD
{
    void loadTorture(const std::string& name, const std::string& docName,
                    const size_t thread_count, const size_t max_jitter_ms = 100);
    TestResult testLoadTortureODT();
    TestResult testLoadTortureODS();
    TestResult testLoadTortureODP();
    TestResult testLoadTortureODG();
    TestResult testLoadTorture();

public:
    UnitLoadTorture();
    void invokeWSDTest() override;
};

void UnitLoadTorture::loadTorture(const std::string& name, const std::string& docName,
                                 const size_t thread_count, const size_t max_jitter_ms)
{
    // Load same document from many threads together.
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL(docName, documentPath, documentURL, name);

    TST_LOG("Starting test on " << documentURL << ' ' << documentPath);

    std::mutex stateLock;
    std::vector<int> view_ids;

    std::atomic<int> sum_view_ids;
    sum_view_ids = 0;
    std::atomic<int> num_of_views(0);
    std::atomic<int> num_to_load(thread_count);

    std::shared_ptr<SocketPoll> poll = std::make_shared<SocketPoll>("WebSocketPoll");
    poll->startThread();

    std::vector<std::thread> threads;
    for (size_t i = 0; i < thread_count; ++i)
    {
        threads.emplace_back([&] {
            const std::string id = std::to_string(ProcUtil::getThreadId());

            TST_LOG(": #" << id << ", views: " << num_of_views << ", to load: " << num_to_load);
            try
            {
                // Load a document and wait for the status.
                auto wsSession = http::WebSocketSession::create(helpers::getTestServerURI());
                http::Request req(documentURL);
                wsSession->asyncRequest(req, poll);

                wsSession->sendMessage("load url=" + documentURL);

                // 20s is double of the default.
                std::vector<char> message =
                    wsSession->waitForMessage("status:", 20s, name + id + ' ');
                const std::string status = COOLProtocol::getFirstLine(message);

                int viewid = -1;
                Poco::JSON::Parser parser;
                Poco::Dynamic::Var statusJsonVar = parser.parse(status.substr(7));
                const Poco::SharedPtr<Poco::JSON::Object>& statusJsonObject = statusJsonVar.extract<Poco::JSON::Object::Ptr>();
                if (statusJsonObject->has("viewid"))
                    viewid = std::atoi(statusJsonObject->get("viewid").toString().c_str());

                LOK_ASSERT("Failed to create view in time " && viewid >= 0);

                {
                    std::lock_guard<std::mutex> guard(stateLock);
                    LOK_ASSERT("Duplicate view-id generated " && std::find(view_ids.begin(), view_ids.end(), viewid) == view_ids.end());
                    view_ids.push_back(viewid);
                }

                ++num_of_views;
                --num_to_load;

                TST_LOG(": #" << id << ", new viewId: " << viewid << ", loaded views: "
                              << num_of_views << ", to load: " << num_to_load);

                // Get all the views loaded - lean on test timeout to interrupt.
                while (num_to_load > 0)
                    std::this_thread::sleep_for(20ms);

                // delay randomly to close in random order:
                const auto ms =
                    (max_jitter_ms > 0
                         ? std::chrono::milliseconds(Util::rng::getNext() % max_jitter_ms)
                         : 0ms);
                std::this_thread::sleep_for(ms);

                --num_of_views;

                TST_LOG(": #" << id << ", view: " << num_of_views << " unloading");
            }
            catch (const std::exception& exc)
            {
                TST_LOG(": #" << id << ", Exception: " << exc.what());
                --num_to_load;
            }
        });
    }

    for (auto& thread : threads)
    {
        try
        {
            thread.join();
        }
        catch (const std::exception& exc)
        {
            TST_LOG(": Exception: " << exc.what());
        }
    }
}

UnitBase::TestResult UnitLoadTorture::testLoadTortureODT()
{
    const int thread_count = 6;

    loadTorture(testname, "empty.odt", thread_count);

    return TestResult::Ok;
}

UnitBase::TestResult UnitLoadTorture::testLoadTortureODS()
{
    const int thread_count = 6;

    loadTorture(testname, "empty.ods", thread_count);

    return TestResult::Ok;
}

UnitBase::TestResult UnitLoadTorture::testLoadTortureODP()
{
    const int thread_count = 6;

    loadTorture(testname, "empty.odp", thread_count);

    return TestResult::Ok;
}

UnitBase::TestResult UnitLoadTorture::testLoadTortureODG()
{
    const int thread_count = 6;

    loadTorture(testname, "empty.odg", thread_count);

    return TestResult::Ok;
}

UnitBase::TestResult UnitLoadTorture::testLoadTorture()
{
    const int thread_count = 3;
    const int max_jitter_ms = 75;

    std::vector<std::string> docNames = { "setclientpart.ods", "hello.odt", "viewcursor.odp" };

    std::vector<std::thread> threads;
    threads.reserve(docNames.size());
    for (const auto& docName : docNames)
    {
        threads.emplace_back([&] {
            const auto name = "loadTorture_" + docName + ' ';
            loadTorture(name, docName, thread_count, max_jitter_ms);
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
    return TestResult::Ok;
}

UnitLoadTorture::UnitLoadTorture()
    : UnitWSD("UnitLoadTorture")
{
    // Double of the default.
    constexpr std::chrono::minutes timeout_minutes(1);
    setTimeout(timeout_minutes);
}

void UnitLoadTorture::invokeWSDTest()
{
    UnitBase::TestResult result = testLoadTortureODT();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testLoadTortureODS();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testLoadTortureODP();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testLoadTortureODG();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testLoadTorture();
    exitTest(result);
}

UnitBase* unit_create_wsd(void) { return new UnitLoadTorture(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
