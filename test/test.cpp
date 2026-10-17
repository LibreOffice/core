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
 * Main test runner for client tests.
 */

#include <config.h>

#define TST_LOG_REDIRECT
#include <test.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

#include <Poco/DirectoryIterator.h>
#include <Poco/FileStream.h>
#include <regex>
#include <Poco/StreamCopier.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <Unit.hpp>
#if ENABLE_SSL
#include <Ssl.hpp>
#include <SslSocket.hpp>
#endif
#include <common/Log.hpp>
#include <common/ConfigUtil.hpp>

bool filterTests(CPPUNIT_NS::TestRunner& runner, CPPUNIT_NS::Test* testRegistry, const std::string& testName)
{
    std::regex re(testName, std::regex_constants::icase);

    bool haveTests = false;
    for (int i = 0; i < testRegistry->getChildTestCount(); ++i)
    {
        CPPUNIT_NS::Test* testSuite = testRegistry->getChildTestAt(i);
        for (int j = 0; j < testSuite->getChildTestCount(); ++j)
        {
            CPPUNIT_NS::Test* testCase = testSuite->getChildTestAt(j);
            try
            {
                if (std::regex_search(testCase->getName(), re))
                {
                    runner.addTest(testCase);
                    haveTests = true;
                }
            }
            catch (const std::exception& exc)
            {
                // Nothing to do; skip.
            }
        }
    }

    return haveTests;
}

#ifdef STANDALONE_CPPUNIT

// coverity[root_function] : don't warn about uncaught exceptions
int main(int argc, char** argv)
{
    bool verbose = false;
    std::string cert_path = "/etc/coolwsd/";
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg(argv[i]);
        if (arg == "--verbose")
        {
            verbose = true;
        }
        else if (arg == "--cert-path" && ++i < argc)
        {
            cert_path = argv[i];
        }
    }

    const char* loglevel = verbose ? "trace" : "warning";
    const bool withColor = isatty(fileno(stderr));
    Log::initialize("tst", loglevel, withColor, false, {}, false, {});

    Poco::AutoPtr<Poco::Util::LayeredConfiguration> defConfig(new Poco::Util::LayeredConfiguration);
    ConfigUtil::initialize(defConfig.get());

#if ENABLE_SSL
    try
    {
        // The most likely place. If not found, SSL will be disabled in the tests.
        const std::string ssl_cert_file_path = cert_path + "/cert.pem";
        const std::string ssl_key_file_path = cert_path + "/key.pem";
        const std::string ssl_ca_file_path = cert_path + "/ca-chain.cert.pem";
        const std::string ssl_cipher_list = DEFAULT_CIPHER_SET;

        // Initialize the non-blocking socket SSL.
        ssl::Manager::initializeServerContext(ssl_cert_file_path, ssl_key_file_path,
                                              ssl_ca_file_path, ssl_cipher_list,
                                              ssl::CertificateVerification::Disabled);

        ssl::Manager::initializeClientContext(ssl_cert_file_path, ssl_key_file_path,
                                              ssl_ca_file_path, ssl_cipher_list,
                                              ssl::CertificateVerification::Required);
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Exception while initializing SslContext: " << ex.what());
    }

    if (!ssl::Manager::isServerContextInitialized())
        LOG_ERR("Failed to initialize Server SSL. Set the path to the certificates via "
                "--cert-path. HTTPS tests will be disabled in unit-tests.");
    else
        LOG_INF("Initialized Server SSL.");

    if (!ssl::Manager::isClientContextInitialized())
        LOG_ERR("Failed to initialize Client SSL.");
    else
        LOG_INF("Initialized Client SSL.");
#else
    LOG_INF("SSL is unsupported in this build.");
#endif

    return runClientTests(argv[0], true, verbose) ? 0 : 1;
}
#endif

static bool IsStandalone = false;

bool isStandalone()
{
    return IsStandalone;
}

static std::mutex ErrorMutex;
static std::ostringstream ErrorsStream;

class TestProgressListener : public CppUnit::TestListener
{
    TestProgressListener(const TestProgressListener& copy) = delete;
    void operator=(const TestProgressListener& copy) = delete;

public:
    TestProgressListener() {}
    virtual ~TestProgressListener() {}

    void startTest(CppUnit::Test* test)
    {
        LOG_TST("=============== START " << test->getName());
        if (UnitBase::isUnitTesting()) // Only if we are in UnitClient.
            UnitBase::get().setTestname(test->getName());
        _startTime = std::chrono::steady_clock::now();
    }

    void addFailure(const CppUnit::TestFailure& failure)
    {
        if (failure.isError())
            LOG_TST(">>>>>>>> ERROR " << failure.failedTestName() << " <<<<<<<<<");
        else
            LOG_TST(">>>>>>>> FAILED " << failure.failedTestName() << " <<<<<<<<<");

        const auto ex = failure.thrownException();
        if (ex != nullptr)
        {
            LOG_TST("Exception: " << ex->message().shortDescription() << '\n'
                                  << ex->message().details() << "\tat "
                                  << ex->sourceLine().fileName() << ':'
                                  << std::to_string(ex->sourceLine().lineNumber()));
        }
        else
        {
            LOG_TST("\tat " << failure.sourceLine().fileName() << ':'
                            << std::to_string(failure.sourceLine().lineNumber()));
        }
    }

    void endTest(CppUnit::Test* test)
    {
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - _startTime);
        LOG_TST("=============== END " << test->getName() << " (" << std::to_string(ms.count())
                                       << "ms) ===============");
    }

private:
    std::chrono::steady_clock::time_point _startTime;
};

// returns true on success
bool runClientTests(const char* cmd, bool standalone, bool verbose)
{
    IsStandalone = standalone;

    CPPUNIT_NS::TestResult controller;
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);
    TestProgressListener listener;
    controller.addListener(&listener);

    CPPUNIT_NS::Test* testRegistry = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

    CPPUNIT_NS::TestRunner runner;
    const char* envar = std::getenv("CPPUNIT_TEST_NAME");
    std::string testName;
    if (envar)
    {
        testName = std::string(envar);
    }

    if (testName.empty())
    {
        // Add all tests.
        runner.addTest(testRegistry);
    }
    else
    {
        const bool testsAdded = filterTests(runner, testRegistry, testName);
        if (!testsAdded)
        {
            std::cerr << "Failed to match [" << testName << "] to any names in the external test-suite. "
                      << "No external tests will be executed" << std::endl;
        }
    }

    if (!verbose)
    {
        runner.run(controller);

        // output the ErrorsStream we got during the testing
        if (!result.wasSuccessful())
        {
            std::lock_guard<std::mutex> lock(ErrorMutex);
            LOG_TST(ErrorsStream.str() + '\n');
        }
    }
    else
    {
        runner.run(controller);
    }

    CPPUNIT_NS::CompilerOutputter outputter(&result, std::cerr);
    outputter.setNoWrap();
    outputter.write();

    const std::deque<CPPUNIT_NS::TestFailure *> &failures = result.failures();
    if (!envar && failures.size() > 0)
    {
        std::cerr << "\nTo reproduce the first test failure use:\n\n";
#ifdef STANDALONE_CPPUNIT // unittest
        std::cerr << "To debug:\n\n";
        std::cerr << "  (cd test; CPPUNIT_TEST_NAME=\"" << (*failures.begin())->failedTestName() << "\" gdb --args " << cmd << ")\n\n";
#else
        (void)cmd;
        std::string lib = UnitBase::get().getUnitLibPath();
        std::size_t lastSlash = lib.rfind('/');
        if (lastSlash != std::string::npos)
            lib = lib.substr(lastSlash + 1, lib.length() - lastSlash - 4) + ".la";
        std::cerr << "(cd test; CPPUNIT_TEST_NAME=\"" << (*failures.begin())->failedTestName() <<
            "\" ./run_unit.sh --test-name " << lib << ")\n\n";
#endif
    }

    return result.wasSuccessful();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
