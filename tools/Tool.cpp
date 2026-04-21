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
 * General utility tool for document conversion and inspection.
 * Functions: Document conversion, property inspection
 */

#include <config.h>

#include <common/Common.hpp>
#include <common/NumUtil.hpp>
#include <common/Protocol.hpp>
#include <common/Util.hpp>

#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/FilePartSource.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/OptionSet.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sysexits.h>
#include <thread>
#include <unistd.h>

/// Simple command-line tool for file format conversion.
class Tool: public Poco::Util::Application
{
    // Display help information on the console
    void displayHelp();

public:
    Tool();

    const std::string& getServerURI() const { return _serverURI; }
    const std::string& getDestinationFormat() const { return _destinationFormat; }
    const std::string& getDestinationDir() const { return _destinationDir; }

private:
    unsigned    _numWorkers;
    std::string _serverURI;
    std::string _destinationFormat;
    std::string _destinationDir;

protected:
    void defineOptions(Poco::Util::OptionSet& options) override;
    void handleOption(const std::string& name, const std::string& value) override;
    int  main(const std::vector<std::string>& args) override;
};


using namespace COOLProtocol;

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Runnable;
using Poco::URI;
using Poco::Util::Application;
using Poco::Util::OptionSet;

/// Thread class which performs the conversion.
class Worker: public Runnable
{
    Tool& _app;
    std::vector< std::string > _files;
public:
    Worker(Tool& app, const std::vector< std::string > & files) :
        _app(app), _files(files)
    {
    }

    void run() override
    {
        for (const auto& i : _files)
            convertFile(i);
    }

    void convertFile(const std::string& document)
    {
        Poco::URI uri(_app.getServerURI());

        Poco::Net::HTTPClientSession *session;
        if (_app.getServerURI().compare(0, 5, "https") == 0)
            session = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
        else
            session = new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/cool/convert-to");

        try {
            Poco::Net::HTMLForm form;
            form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);
            form.set("format", _app.getDestinationFormat());
            form.addPart("data", new Poco::Net::FilePartSource(document));
            form.prepareSubmit(request);

            // If this results in a Poco::Net::ConnectionRefusedException, coolwsd is not running.
            form.write(session->sendRequest(request));
        }
        catch (const Poco::Exception &e)
        {
            std::cerr << "Failed to write data: " << e.name() <<
                  ' ' << e.message() << '\n';
            return;
        }

        Poco::Net::HTTPResponse response;

        try {
            // receiveResponse() resulted in a Poco::Net::NoMessageException.
            std::istream& responseStream = session->receiveResponse(response);

            Poco::Path path(document);
            std::string outPath = _app.getDestinationDir() + '/' + path.getBaseName() + '.' + _app.getDestinationFormat();
            std::ofstream fileStream(outPath);

            Poco::StreamCopier::copyStream(responseStream, fileStream);
        }
        catch (const Poco::Exception &e)
        {
            std::cerr << "Exception converting: " << e.name() <<
                  ' ' << e.message() << '\n';
            return;
        }

        delete session;
    }
};

Tool::Tool() :
    _numWorkers(4),
#if ENABLE_SSL
    _serverURI("https://127.0.0.1:" + std::to_string(DEFAULT_CLIENT_PORT_NUMBER)),
#else
    _serverURI("http://127.0.0.1:" + std::to_string(DEFAULT_CLIENT_PORT_NUMBER)),
#endif
    _destinationFormat("txt")
{
}

void Tool::displayHelp()
{
    std::cout << "Collabora Online document converter tool.\n"
              << "Usage: " << commandName() << " [options] file...\n"
              << "Options are:\n"
              << "  --help                      Show this text\n"
              << "  --extension=format          File format to convert to\n"
              << "  --outdir=directory          Output directory for converted files\n"
              << "  --parallelism=threads       Number of simultaneous threads to use\n"
              << "  --server=uri                URI of COOL server\n"
              << "  --no-check-certificate      Disable checking of SSL certificate\n"
              << "In addition, the options taken by the soffice command for its --convert-to\n"
              << "functionality can be used (but are ignored if irrelevant to this command)." << std::endl;
}

void Tool::defineOptions(OptionSet&)
{
    stopOptionsProcessing();
}

void Tool::handleOption(const std::string& optionName,
                        const std::string& value)
{
    if (optionName == "help")
    {
        displayHelp();
        std::exit(EX_OK);
    }
    else if (optionName == "extension"
             || optionName == "convert-to")
        _destinationFormat = value;
    else if (optionName == "outdir")
        _destinationDir = value;
    else if (optionName == "parallelism")
        _numWorkers = std::max(NumUtil::stoi(value), 1);
    else if (optionName == "server")
        _serverURI = value;
    else if (optionName == "no-check-certificate")
    {
        Poco::SharedPtr<Poco::Net::PrivateKeyPassphraseHandler> consoleClientHandler = new Poco::Net::KeyConsoleHandler(false);
        Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> invalidClientCertHandler = new Poco::Net::AcceptCertificateHandler(false);
        Poco::Net::Context::Ptr sslClientContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "");
        Poco::Net::SSLManager::instance().initializeClient(std::move(consoleClientHandler),
                                                           std::move(invalidClientCertHandler),
                                                           std::move(sslClientContext));
    }
}

int Tool::main(const std::vector<std::string>& origArgs)
{
    std::vector<std::string> args = origArgs;

    for (unsigned i = 0; i < origArgs.size(); ++i)
    {
        if (origArgs[i].length() > 0 && origArgs[i][0] != '-')
            break;

        // It's an option. Erase it from the file name vector.
        args.erase(args.begin());

        std::string optionName, value;

        // Accept either one or two dashes, like CollaboraOffice.
        if (origArgs[i].length() > 1 && origArgs[i][1] != '-')
            optionName = origArgs[i].substr(1);
        else if (origArgs[i].length() > 1 && origArgs[i][1] == '-')
            optionName = origArgs[i].substr(2);
        else
            break;

        std::string::size_type equals = optionName.find('=');

        // Handle CollaboraOffice-compatible options that don't have their value separated with an equals,
        // but as the next argument.
        if (equals == std::string::npos
            && (optionName == "convert-to"
                || optionName == "outdir")
            && i < origArgs.size()-1)
        {
            value = origArgs[i+1];
            args.erase(args.begin());
            ++i;
        }
        else if (equals != std::string::npos)
        {
            value = optionName.substr(equals+1);
            optionName = optionName.substr(0, equals);
        }
        handleOption(optionName, value);
    }

    if (args.empty())
    {
        std::cerr << "Nothing to do." << std::endl;
        displayHelp();
        return EX_NOINPUT;
    }

    std::vector<std::thread> clients;
    clients.reserve(_numWorkers);

    size_t chunk = (args.size() + _numWorkers - 1) / _numWorkers;
    size_t offset = 0;
    for (unsigned i = 0; i < _numWorkers; i++)
    {
        size_t toCopy = std::min(args.size() - offset, chunk);
        if (toCopy > 0)
        {
            std::vector< std::string > files( toCopy );
            std::copy( args.begin() + offset, args.begin() + offset + toCopy, files.begin() );
            offset += toCopy;
            clients.emplace_back([this, files=std::move(files)]{Worker(*this, files).run();});
        }
    }

    for (auto& client: clients)
    {
        client.join();
    }

    return EX_OK;
}

// coverity[root_function] : don't warn about uncaught exceptions
POCO_APP_MAIN(Tool)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
