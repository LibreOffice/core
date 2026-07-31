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

#include <tools/Replay.hpp>

#include <sysexits.h>

#include <Poco/Util/Application.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>

int ClientPortNumber = DEFAULT_CLIENT_PORT_NUMBER;

namespace {

/// Stress testing and performance/scalability benchmarking tool.
class Stress: public Poco::Util::Application
{
public:
    Stress() {}
protected:
    void defineOptions(Poco::Util::OptionSet& options) override;
    static void printHelp();
    void handleOption(const std::string& name, const std::string& value) override;
    int  main(const std::vector<std::string>& args) override;
};

}

void Stress::defineOptions(Poco::Util::OptionSet& optionSet)
{
    Application::defineOptions(optionSet);

    optionSet.addOption(Poco::Util::Option("help", "", "Display help information on command line arguments.")
                        .required(false).repeatable(false));
}

void Stress::handleOption(const std::string& optionName,
                          const std::string& value)
{
    Application::handleOption(optionName, value);

    if (optionName == "help")
    {
        printHelp();
        Util::forcedExit(EX_OK);
    }
    else
    {
        std::cout << "Unknown option: " << optionName << std::endl;
        Util::forcedExit(EX_SOFTWARE);
    }
}

void Stress::printHelp()
{
    std::cerr << "Usage: coolstress wss://localhost:9980 <test-document-path> <trace-path> " << std::endl;
    std::cerr << "       Trace files may be plain text or gzipped (with .gz extension)." << std::endl;
    std::cerr << "       --help for full arguments list." << std::endl;
}

// coverity[root_function] : don't warn about uncaught exceptions
int Stress::main(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        printHelp();
        return EX_NOINPUT;
    }

    std::shared_ptr<TerminatingPoll> poll(std::make_shared<TerminatingPoll>("stress replay"));

    if (!UnitWSD::init(UnitWSD::UnitType::Tool, ""))
        throw std::runtime_error("Failed to init unit test pieces.");

#if ENABLE_SSL
    ssl::Manager::initializeClientContext("", "", "", "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH",
                                          ssl::CertificateVerification::Disabled);
    if (!ssl::Manager::isClientContextInitialized())
    {
        std::cerr << "Failed to initialize Client SSL.\n";
        return -1;
    }
#endif

    const std::string& server = args[0];

    if (!strncmp(server.c_str(), "http", 4))
    {
        std::cerr << "Server should be wss:// or ws:// URL, not " << server << '\n';
        return -1;
    }

    auto stats = std::make_shared<Stats>();

    std::cerr << "Connect to " << server << "\n";
    for (size_t i = 1; i < args.size() - 1; i += 2)
        StressSocketHandler::addPollFor(*poll, server, args[i], args[i+1], stats);

    do {
        poll->poll(TerminatingPoll::DefaultPollTimeoutMicroS);
    } while (poll->continuePolling() && poll->getSocketCount() > 0);

    stats->dump(std::cerr);

    return EX_OK;
}

// coverity[root_function] : don't warn about uncaught exceptions
POCO_APP_MAIN(Stress)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
