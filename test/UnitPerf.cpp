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
 * Unit test for performance measurements.
 */

#include <config.h>
#include <config_version.h>

#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/StringVector.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <test/WebSocketSession.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>
#include <tools/Replay.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/DocumentBroker.hpp>

#include <Poco/Util/LayeredConfiguration.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class UnitPerf : public UnitWSD
{
    Util::SysStopwatch _timer;
    std::shared_ptr<Stats> _stats;
    std::unique_ptr<std::thread> _thread;
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _brokerDestroyed;
    std::atomic<bool> _done;

    void testPerf(const std::string& testType, const std::string& fileType,
                  const std::string& traceStr);

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        config.setString("logging.level", "critical");
        config.setString("logging.level_startup", "critical");
        config.setBool("admin_console.enable", false);

        //        pfm_initialize();
        UnitWSD::configure(config);
    }

    void invokeWSDTest() override;
    void onPerfDocumentLoading() override;
    void onPerfDocumentLoaded() override;
    void onDocBrokerDestroy(const std::string& docKey) override;
    void workerThread();

public:
    UnitPerf();
    ~UnitPerf() override;
};

void UnitPerf::testPerf(const std::string& testType, const std::string& fileType,
                        const std::string& traceStr)
{
    std::shared_ptr<Stats> stats = std::make_shared<Stats>();
    stats->setTypeOfTest(testType);

    {
        std::unique_lock<std::mutex> lock(_mutex);
        _stats = stats;
        _brokerDestroyed = false;
    }

    std::shared_ptr<TerminatingPoll> poll = std::make_shared<TerminatingPoll>("performance test");
    poll->runOnClientThread();

    const std::string docName = "empty." + fileType;

    std::string filePath, dummy;

    helpers::getDocumentPathAndURL(docName, filePath, dummy, "testPerf");

    const std::string tracePath = TDOC + traceStr;
#if ENABLE_RUNTIME_OPTIMIZATIONS
    constexpr float latencyFactor = 0.1; // ~10x faster replay than recorded.
#else
    constexpr float latencyFactor = 0.5; // ~2x faster replay than recorded.
#endif
    StressSocketHandler::addPollFor(*poll, helpers::getTestServerURI("ws"), filePath, tracePath,
                                    stats, latencyFactor);

    do {
        poll->poll(TerminatingPoll::DefaultPollTimeoutMicroS);
    } while (poll->continuePolling() && poll->getSocketCount() > 0);

    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait_for(lock, std::chrono::seconds(10), [this] { return _brokerDestroyed; });

    TST_LOG("Stats: " << [&](std::ostream& os) { stats->dump(os); });
}

UnitPerf::UnitPerf()
    : UnitWSD("UnitPerf")
    , _brokerDestroyed(false)
    , _done(false)
{
    // Double of the default.
#if ENABLE_RUNTIME_OPTIMIZATIONS
    constexpr std::chrono::minutes timeout_minutes(1);
#else
    constexpr std::chrono::minutes timeout_minutes(2);
#endif
    setTimeout(timeout_minutes);
}

UnitPerf::~UnitPerf()
{
    if (_thread && _thread->joinable())
    {
        _thread->join();
    }
}

void UnitPerf::workerThread()
{
    TST_LOG("startup: " << _timer.elapsedTime());
    _timer.restart();

    testPerf("writer", "odt", "/../traces/perf-writer.txt");

    testPerf("calc", "ods", "/../traces/perf-calc.txt");

    testPerf("impress", "odp", "/../traces/perf-impress.txt");

    testPerf("draw", "odg", "/../traces/perf-draw.txt");

    const auto cpuTime = _timer.elapsedTime();

    TST_LOG("test: " << cpuTime);

    _done = true;
}

void UnitPerf::invokeWSDTest()
{
    if (!_thread)
    {
        _thread = std::make_unique<std::thread>(&UnitPerf::workerThread, this);
    }
    else if (_done)
    {
        _thread->join();
        exitTest(TestResult::Ok);
    }
}

//Called when document loading process starts e.g. setup finishes
void UnitPerf::onPerfDocumentLoading()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (_stats)
    {
        _stats->endPhase(Log::Phase::Setup);
    }
}

//called when document has been loaded into core
void UnitPerf::onPerfDocumentLoaded()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (_stats)
    {
        _stats->endPhase(Log::Phase::Load);
    }
}

void UnitPerf::onDocBrokerDestroy(const std::string& docKey)
{
    TST_LOG("Dockey [" << docKey << "] destroyed");

    UnitWSD::onDocBrokerDestroy(docKey);

    std::unique_lock<std::mutex> lock(_mutex);
    _brokerDestroyed = true;
    _cv.notify_one();
}

UnitBase* unit_create_wsd(void) { return new UnitPerf(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
