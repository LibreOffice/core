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
 * The document of a conversion is quarantined when it is opened, and kept only when the
 * conversion goes wrong.
 */

#include <config.h>

#include <Unit.hpp>
#include <common/FileUtil.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <testlog.hpp>

#include <Poco/File.h>
#include <Poco/Net/FilePartSource.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/StreamCopier.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

class UnitQuarantineConvertTo : public UnitWSD
{
    std::string _quarantinePath;
    bool _workerStarted;
    std::thread _worker;
    /// The number of document brokers that are gone.
    std::atomic<int> _destroyedBrokers;
    /// The number of documents that loaded, and of those that were quarantined by then.
    std::atomic<int> _loadedDocuments;
    std::atomic<int> _loadedQuarantined;

    /// Sends the given test document to the given request, a conversion to the given format by
    /// default, and returns the HTTP status, or -1.
    int convertTo(const std::string& filename, const std::string& format = "png",
                  const std::string& request = "convert-to")
    {
        std::unique_ptr<Poco::Net::HTTPClientSession> session(
            helpers::createSession(Poco::URI(helpers::getTestServerURI())));
        session->setTimeout(Poco::Timespan(30, 0));

        const std::string uri =
            "/cool/" + request + (request == "convert-to" ? '/' + format : std::string());
        Poco::Net::HTTPRequest httpRequest(Poco::Net::HTTPRequest::HTTP_POST, uri);
        Poco::Net::HTMLForm form;
        form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);
        form.set("format", format);
        form.addPart("data", new Poco::Net::FilePartSource(Poco::Path(TDOC, filename).toString()));
        form.prepareSubmit(httpRequest);
        form.write(session->sendRequest(httpRequest));

        try
        {
            Poco::Net::HTTPResponse response;
            std::istream& rs = session->receiveResponse(response);
            std::ostringstream body;
            Poco::StreamCopier::copyStream(rs, body);
            return static_cast<int>(response.getStatus());
        }
        catch (const std::exception& ex)
        {
            TST_LOG("Exception reading the response: " << ex.what());
            return -1;
        }
    }

    /// Waits until count document brokers are gone.
    bool waitForDestroyedBrokers(int count)
    {
        for (int i = 0; i < 300 && _destroyedBrokers < count; ++i)
            std::this_thread::sleep_for(100ms);
        return _destroyedBrokers >= count;
    }

    /// The files under the quarantine, as docKey/filename.
    std::vector<std::string> getQuarantinedFiles()
    {
        std::vector<std::string> files;
        std::vector<std::string> docKeys;
        Poco::File(_quarantinePath).list(docKeys);
        for (const std::string& docKey : docKeys)
        {
            const std::string docKeyPath = _quarantinePath + '/' + docKey;
            if (!Poco::File(docKeyPath).isDirectory())
                continue;

            std::vector<std::string> names;
            Poco::File(docKeyPath).list(names);
            for (const std::string& name : names)
            {
                // The state file of a document is no version.
                if (name == "state")
                    continue;

                TST_LOG("Found quarantined file [" << docKey << '/' << name << ']');
                files.emplace_back(docKey + '/' + name);
            }
        }

        return files;
    }

    std::string readFile(const std::string& path)
    {
        const std::unique_ptr<std::vector<char>> contents = FileUtil::readFile(path);
        return contents ? std::string(contents->begin(), contents->end()) : std::string();
    }

    bool runTest()
    {
        // A conversion that goes well leaves nothing behind, although its document was quarantined
        // while it ran.
        const int goodStatus = convertTo("hello.odt");
        LOK_ASSERT_EQUAL_MESSAGE("Expected the conversion to succeed", 200, goodStatus);
        LOK_ASSERT_MESSAGE("Expected the conversion's broker to go", waitForDestroyedBrokers(1));
        LOK_ASSERT_EQUAL_MESSAGE("Expected the document loaded", 1, _loadedDocuments.load());
        LOK_ASSERT_EQUAL_MESSAGE("Expected the document quarantined while the conversion ran", 1,
                                 _loadedQuarantined.load());
        LOK_ASSERT_EQUAL_MESSAGE("Expected no quarantined file after a good conversion",
                                 static_cast<std::size_t>(0), getQuarantinedFiles().size());

        // An extraction that goes well leaves nothing behind either.
        const int extractStatus = convertTo("hello.odt", std::string(), "extract-link-targets");
        LOK_ASSERT_EQUAL_MESSAGE("Expected the extraction to succeed", 200, extractStatus);
        LOK_ASSERT_MESSAGE("Expected the extraction's broker to go", waitForDestroyedBrokers(2));
        LOK_ASSERT_EQUAL_MESSAGE("Expected the document quarantined while the extraction ran", 2,
                                 _loadedQuarantined.load());
        LOK_ASSERT_EQUAL_MESSAGE("Expected no quarantined file after a good extraction",
                                 static_cast<std::size_t>(0), getQuarantinedFiles().size());

        // A document that fails to load is kept, as it was sent.
        const int badStatus = convertTo("corrupted.odt");
        LOK_ASSERT_MESSAGE("Expected the conversion to fail", badStatus != 200);
        LOK_ASSERT_MESSAGE("Expected the conversion's broker to go", waitForDestroyedBrokers(3));
        const std::vector<std::string> files = getQuarantinedFiles();
        LOK_ASSERT_EQUAL_MESSAGE("Expected the document of the failed conversion quarantined",
                                 static_cast<std::size_t>(1), files.size());
        LOK_ASSERT_EQUAL_MESSAGE("Expected the document as it was sent",
                                 readFile(Poco::Path(TDOC, "corrupted.odt").toString()),
                                 readFile(_quarantinePath + '/' + files[0]));

        // A document that loads, but can not be converted to the format asked for, is kept too.
        const int exportStatus = convertTo("hello.odt", "nosuchformat");
        LOK_ASSERT_MESSAGE("Expected the conversion to fail", exportStatus != 200);
        LOK_ASSERT_MESSAGE("Expected the conversion's broker to go", waitForDestroyedBrokers(4));
        LOK_ASSERT_EQUAL_MESSAGE("Expected the document of the failed export quarantined too",
                                 static_cast<std::size_t>(2), getQuarantinedFiles().size());

        return true;
    }

public:
    UnitQuarantineConvertTo()
        : UnitWSD("UnitQuarantineConvertTo")
        , _workerStarted(false)
        , _destroyedBrokers(0)
        , _loadedDocuments(0)
        , _loadedQuarantined(0)
    {
        setHasKitHooks();
        setTimeout(std::chrono::minutes(2));
    }

    ~UnitQuarantineConvertTo()
    {
        LOG_INF("Joining test worker thread\n");
        if (_worker.joinable())
            _worker.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setBool("ssl.enable", true);
        config.setInt("per_document.limit_load_secs", 30);
        config.setBool("storage.filesystem[@allow]", false);

        config.setBool("quarantine_files[@enable]", true);
        Poco::Path rootPath(config.getString("child_root_path", ""));
        rootPath.popDirectory().pushDirectory("quarantine");
        _quarantinePath = FileUtil::createRandomTmpDir(rootPath.toString());
        TST_LOG("Quarantine path set to [" << _quarantinePath << ']');
        config.setString("quarantine_files.path", _quarantinePath);
    }

    void onDocBrokerViewLoaded(const std::string& docKey,
                               const std::shared_ptr<ClientSession>&) override
    {
        std::vector<std::string> names;
        const std::string docKeyPath = _quarantinePath + '/' + docKey;
        if (Poco::File(docKeyPath).exists())
            Poco::File(docKeyPath).list(names);

        TST_LOG("DocBroker [" << docKey << "] loaded with " << names.size()
                              << " quarantined files");
        ++_loadedDocuments;
        if (!names.empty())
            ++_loadedQuarantined;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("DocBroker [" << docKey << "] is gone");
        ++_destroyedBrokers;
    }

    void invokeWSDTest() override
    {
        if (_workerStarted)
            return;
        _workerStarted = true;

        _worker = std::thread(
            [this]
            {
                try
                {
                    if (runTest())
                        exitTest(TestResult::Ok);
                }
                catch (const std::exception& ex)
                {
                    TST_LOG("Test failed: " << ex.what());
                    exitTest(TestResult::Failed);
                }
            });
    }
};

class UnitKitQuarantineConvertTo : public UnitKit
{
public:
    UnitKitQuarantineConvertTo()
        : UnitKit("UnitKitQuarantineConvertTo")
    {
        setTimeout(1h);
    }
};

UnitBase* unit_create_wsd(void) { return new UnitQuarantineConvertTo(); }

UnitBase* unit_create_kit(void) { return new UnitKitQuarantineConvertTo(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
