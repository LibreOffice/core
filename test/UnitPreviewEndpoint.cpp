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
 * Unit tests for the slide preview endpoints: GET /cool/preview answers with the index of
 * the slides of a presentation, and GET /cool/preview/<tag>/content with the picture of one
 * slide.
 */

#include <config.h>

#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>

#include <WopiTestServer.hpp>

#include <net/HttpRequest.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/PreviewSession.hpp>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <atomic>
#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

namespace
{
constexpr auto GoodToken = "previewsecret";
constexpr auto RefusedToken = "refusedsecret";
constexpr auto MissingToken = "missingsecret";

constexpr auto PngSignature = "\x89PNG\r\n\x1a\n";

std::shared_ptr<const http::Response> httpGet(const std::string& path)
{
    http::Request request(path, http::Request::VERB_GET);
    auto session = http::Session::create(helpers::getTestServerURI());
    session->setTimeout(std::chrono::seconds(30));
    return session->syncRequest(request);
}

unsigned statusOf(const std::shared_ptr<const http::Response>& response)
{
    return response ? static_cast<unsigned>(response->statusLine().statusCode()) : 0;
}
} // namespace

class PreviewEndpointTest : public WopiTestServer
{
    std::atomic<bool> _started{ false };
    std::thread _worker;
    /// The name of the document, which is the name of the fixture that is its content.
    const std::string _fixture;

protected:
    std::atomic<unsigned> _readOnlyViewsAdded{ 0 };
    std::atomic<unsigned> _readOnlyViewsRemoved{ 0 };
    /// The number of documents opened and closed.
    std::atomic<unsigned> _brokersCreated{ 0 };
    std::atomic<unsigned> _brokersDestroyed{ 0 };

    PreviewEndpointTest(const std::string& name, const std::string& filename)
        : WopiTestServer(name, filename)
        , _fixture(filename)
    {
    }

    ~PreviewEndpointTest() override
    {
        if (_worker.joinable())
            _worker.join();
    }

    std::string documentWopiSrc() const
    {
        return getWopiHostURI() + "/wopi/files/" + _fixture + "?testname=" + getTestname();
    }

    std::string indexPath(const std::string& token) const
    {
        return "/cool/preview?WOPISrc=" + Uri::encode(documentWopiSrc()) +
               "&access_token=" + token;
    }

    Poco::JSON::Object::Ptr getIndex(const std::string& token, unsigned& status)
    {
        const auto response = httpGet(indexPath(token));
        status = statusOf(response);
        Poco::JSON::Object::Ptr index;
        if (status == static_cast<unsigned>(http::StatusCode::OK))
            JsonUtil::parseJSON(response->getBody(), index);
        return index;
    }

    /// What the worker thread does; it ends the test.
    virtual void run() = 0;

public:
    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string uri = request.getURI();
        std::unique_ptr<http::Response> response;
        if (uri.find(std::string("access_token=") + RefusedToken) != std::string::npos)
            response = std::make_unique<http::Response>(http::StatusCode::Unauthorized);
        else if (uri.find(std::string("access_token=") + MissingToken) != std::string::npos)
            response = std::make_unique<http::Response>(http::StatusCode::NotFound);

        // A refusal is a complete answer, with a body length, so it is read as one.
        if (response)
            response->setBody(std::string(), "text/plain");
        return response;
    }

    void onDocBrokerAddSession(const std::string& docKey,
                               const std::shared_ptr<ClientSession>& session) override
    {
        TST_LOG("onDocBrokerAddSession: [" << docKey << "], read-only: "
                                           << session->isReadOnly());
        if (session->isReadOnly())
            ++_readOnlyViewsAdded;
    }

    void onDocBrokerRemoveSession(const std::string& docKey,
                                  const std::shared_ptr<ClientSession>& session) override
    {
        TST_LOG("onDocBrokerRemoveSession: [" << docKey << "], read-only: "
                                              << session->isReadOnly());
        if (session->isReadOnly())
            ++_readOnlyViewsRemoved;
    }

    void onDocBrokerCreate(const std::string& docKey) override
    {
        TST_LOG("onDocBrokerCreate: [" << docKey << ']');
        ++_brokersCreated;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("onDocBrokerDestroy: [" << docKey << ']');
        ++_brokersDestroyed;
    }

    /// Waits up to the given time for the given counter to reach the given value.
    static bool waitFor(const std::atomic<unsigned>& counter, unsigned value,
                        std::chrono::seconds limit)
    {
        const auto deadline = std::chrono::steady_clock::now() + limit;
        while (counter.load() < value && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return counter.load() >= value;
    }

    void invokeWSDTest() override
    {
        if (_started.exchange(true))
            return;

        _worker = std::thread([this] { run(); });
    }
};

/// The index lists every slide of the presentation with a content URL, the content URLs
/// answer with pictures, a second index call for the same token reuses the view, and the
/// requests that name nothing valid are refused.
class UnitPreviewEndpoint : public PreviewEndpointTest
{
public:
    UnitPreviewEndpoint()
        : PreviewEndpointTest("UnitPreviewEndpoint", "slide-link-source.odp")
    {
    }

    void run() override
    {
        // A request that names no document, or no token, is refused before the storage is
        // asked.
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::BadRequest),
                         statusOf(httpGet("/cool/preview?access_token=" + std::string(GoodToken))));
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::BadRequest),
                         statusOf(httpGet("/cool/preview?WOPISrc=" +
                                      Uri::encode(documentWopiSrc()))));

        // A token the storage refuses is answered as unauthorized, and so is a document the
        // storage does not have: the storage check files a missing document under the same
        // state as a refused token.
        unsigned status = 0;
        getIndex(RefusedToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::Unauthorized), status);
        getIndex(MissingToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::Unauthorized), status);
        LOK_ASSERT_EQUAL(0U, _readOnlyViewsAdded.load());

        // The index of a token the storage accepts.
        const Poco::JSON::Object::Ptr index = getIndex(GoodToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), status);
        LOK_ASSERT_MESSAGE("The index must be JSON", index);
        LOK_ASSERT_EQUAL(1, index->optValue<int>("version", 0));

        const std::string tag = index->optValue<std::string>("tag", std::string());
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(64), tag.size());

        const Poco::JSON::Object::Ptr file = index->getObject("file");
        LOK_ASSERT_MESSAGE("The index must describe the file", file);
        LOK_ASSERT_EQUAL(std::string("slide-link-source.odp"),
                         file->optValue<std::string>("name", std::string()));
        LOK_ASSERT_EQUAL(10, file->optValue<int>("slideCount", 0));
        LOK_ASSERT_MESSAGE("The index must carry the document width",
                           file->optValue<int>("docWidth", 0) > 0);

        const Poco::JSON::Array::Ptr slides = index->getArray("slides");
        LOK_ASSERT_MESSAGE("The index must list the slides", slides);
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(10), slides->size());

        std::string firstUrl;
        std::string secondUrl;
        for (std::size_t i = 0; i < slides->size(); ++i)
        {
            const Poco::JSON::Object::Ptr slide = slides->getObject(i);
            LOK_ASSERT_MESSAGE("Each slide is an object", slide);
            LOK_ASSERT_EQUAL(static_cast<int>(i), slide->optValue<int>("index", -1));
            LOK_ASSERT_MESSAGE("Each slide names its part",
                               !slide->optValue<std::string>("part", std::string()).empty());

            const Poco::JSON::Object::Ptr preview = slide->getObject("preview");
            LOK_ASSERT_MESSAGE("Each slide has a preview", preview);
            LOK_ASSERT_EQUAL(std::string("image/png"),
                             preview->optValue<std::string>("mime", std::string()));
            const std::string url = preview->optValue<std::string>("url", std::string());
            LOK_ASSERT_MESSAGE("The content URL names this server",
                               url.find("ServerId=") != std::string::npos);
            LOK_ASSERT_MESSAGE("The content URL carries the tag in its path",
                               url.starts_with("/cool/preview/" + tag + "/content?"));
            LOK_ASSERT_MESSAGE("The content URL carries no token",
                               url.find("access_token") == std::string::npos);
            if (i == 0)
                firstUrl = url;
            else if (i == 1)
                secondUrl = url;
        }

        // The pictures of two slides.
        const auto first = httpGet(firstUrl);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), statusOf(first));
        LOK_ASSERT_EQUAL(std::string("image/png"), first->get("Content-Type"));
        LOK_ASSERT_MESSAGE("The content is a PNG",
                           first->getBody().starts_with(PngSignature));

        const auto second = httpGet(secondUrl);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), statusOf(second));
        LOK_ASSERT_MESSAGE("The content is a PNG",
                           second->getBody().starts_with(PngSignature));
        LOK_ASSERT_MESSAGE("Two slides have two pictures", first->getBody() != second->getBody());

        // A slide asked for again is answered from the tile cache, with the same picture.
        const auto cached = httpGet(firstUrl);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), statusOf(cached));
        LOK_ASSERT_EQUAL(std::string(first->getBody()), std::string(cached->getBody()));

        // The same token answers again without a second view: through the live view, or from
        // the cache once the view stored every picture there.
        const Poco::JSON::Object::Ptr again = getIndex(GoodToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), status);
        LOK_ASSERT_MESSAGE("The index must be JSON", again);
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(64),
                         again->optValue<std::string>("tag", std::string()).size());
        LOK_ASSERT_EQUAL(1U, _readOnlyViewsAdded.load());

        // A content request that names a slide the document has not.
        const std::string base = firstUrl.substr(0, firstUrl.find("&slide="));
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::NotFound),
                         statusOf(httpGet(base + "&slide=" +
                                      Uri::encode("{00000000-0000-0000-0000-000000000000}"))));

        // A content request without a slide, with a tag of the wrong shape, with a tag that
        // was never handed out, or for another server.
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::BadRequest),
                         statusOf(httpGet(base)));
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::BadRequest),
                         statusOf(httpGet(Util::replace(firstUrl, "/" + tag + "/", "/short/"))));
        const std::string unknownTag(64, 'a');
        LOK_ASSERT_EQUAL(
            static_cast<unsigned>(http::StatusCode::Gone),
            statusOf(httpGet(Util::replace(firstUrl, "/" + tag + "/", "/" + unknownTag + "/"))));
        LOK_ASSERT_EQUAL(
            static_cast<unsigned>(http::StatusCode::BadRequest),
            statusOf(httpGet(Util::replace(firstUrl, "ServerId=", "ServerId=other"))));

        passTest("The preview endpoints answer the index and the slide pictures");
    }
};

/// A tag expires after its time without use, its content requests are then
/// answered as gone, and the view behind it leaves the document. The time is counted from
/// the last answer, so the load of the document does not eat into it.
class UnitPreviewEndpointExpiry : public PreviewEndpointTest
{
public:
    UnitPreviewEndpointExpiry()
        : PreviewEndpointTest("UnitPreviewEndpointExpiry", "slide-link-source.odp")
    {
        // The test outlives the tag, with room for the load before it.
        setTimeout(PreviewSession::TagTtl * 4);
    }

    void run() override
    {
        unsigned status = 0;
        const Poco::JSON::Object::Ptr index = getIndex(GoodToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), status);
        LOK_ASSERT_MESSAGE("The index must be JSON", index);
        LOK_ASSERT_EQUAL(1U, _readOnlyViewsAdded.load());

        const Poco::JSON::Array::Ptr slides = index->getArray("slides");
        LOK_ASSERT_MESSAGE("The index must list the slides", slides && slides->size() > 0);
        const std::string url =
            slides->getObject(0)->getObject("preview")->optValue<std::string>("url",
                                                                              std::string());

        // The picture is served while the tag lives.
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), statusOf(httpGet(url)));

        // The view leaves once the tag has gone unused past its time.
        const auto deadline = std::chrono::steady_clock::now() + PreviewSession::TagTtl * 2;
        while (_readOnlyViewsRemoved.load() == 0 && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        LOK_ASSERT_EQUAL(1U, _readOnlyViewsRemoved.load());

        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::Gone), statusOf(httpGet(url)));

        passTest("An unused preview tag expires with its view");
    }
};

/// A document that is not a presentation has no slides to preview.
class UnitPreviewEndpointWriter : public PreviewEndpointTest
{
public:
    UnitPreviewEndpointWriter()
        : PreviewEndpointTest("UnitPreviewEndpointWriter", "hello.odt")
    {
    }

    void run() override
    {
        unsigned status = 0;
        getIndex(GoodToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::UnsupportedMediaType), status);

        passTest("A text document is refused by the preview index");
    }
};

/// The previews outlive the document: once its last view left and it unloaded, the index
/// and the pictures of an unchanged file are answered from the cache without loading it, a
/// changed file is loaded again, a token the storage refuses is refused whatever the cache
/// holds, and an entry not used for the time limit is loaded again.
class UnitPreviewEndpointCache : public PreviewEndpointTest
{
public:
    UnitPreviewEndpointCache()
        : PreviewEndpointTest("UnitPreviewEndpointCache", "slide-link-source.odp")
    {
        // The test outlives the tags of two indexes, with room for the loads before them.
        setTimeout(PreviewSession::TagTtl * 6);
    }

    void run() override
    {
        // The first index loads the document, and its view fills the cache.
        unsigned status = 0;
        const Poco::JSON::Object::Ptr first = getIndex(GoodToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), status);
        LOK_ASSERT_MESSAGE("The index must be JSON", first);
        LOK_ASSERT_EQUAL(1U, _brokersCreated.load());

        // The view leaves once the cache has every picture and its tag expired, and the
        // document then unloads.
        LOK_ASSERT_MESSAGE("The document must unload",
                           waitFor(_brokersDestroyed, 1, PreviewSession::TagTtl * 2));

        // The unchanged file is answered from the cache: nothing is downloaded or opened.
        resetCountGetFile();
        const Poco::JSON::Object::Ptr cached = getIndex(GoodToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), status);
        LOK_ASSERT_MESSAGE("The index must be JSON", cached);
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), getCountGetFile());
        LOK_ASSERT_EQUAL(1U, _brokersCreated.load());

        const std::string tag = cached->optValue<std::string>("tag", std::string());
        LOK_ASSERT_MESSAGE("A cache hit has a tag of its own",
                           tag != first->optValue<std::string>("tag", std::string()));
        LOK_ASSERT_EQUAL(std::string("slide-link-source.odp"),
                         cached->getObject("file")->optValue<std::string>("name", ""));

        const Poco::JSON::Array::Ptr slides = cached->getArray("slides");
        LOK_ASSERT_MESSAGE("The cached index lists the slides", slides && slides->size() == 10);
        for (std::size_t i = 0; i < slides->size(); ++i)
        {
            const std::string url =
                slides->getObject(i)->getObject("preview")->optValue<std::string>("url", "");
            LOK_ASSERT_MESSAGE("The cached content URL carries the new tag",
                               url.starts_with("/cool/preview/" + tag + "/content?"));
            const auto picture = httpGet(url);
            LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), statusOf(picture));
            LOK_ASSERT_MESSAGE("The cached content is a PNG",
                               picture->getBody().starts_with(PngSignature));
        }
        LOK_ASSERT_EQUAL(1U, _brokersCreated.load());

        // A slide the cached document does not have.
        const std::string firstUrl =
            slides->getObject(0)->getObject("preview")->optValue<std::string>("url", "");
        const std::string base = firstUrl.substr(0, firstUrl.find("&slide="));
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::NotFound),
                         statusOf(httpGet(base + "&slide=" +
                                      Uri::encode("{00000000-0000-0000-0000-000000000000}"))));

        // A token the storage refuses is refused, though the cache has the file.
        getIndex(RefusedToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::Unauthorized), status);

        // A changed file is loaded again.
        setFileContent(getFileContent());
        resetCountGetFile();
        getIndex(GoodToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), status);
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), getCountGetFile());
        LOK_ASSERT_EQUAL(2U, _brokersCreated.load());

        LOK_ASSERT_MESSAGE("The document must unload again",
                           waitFor(_brokersDestroyed, 2, PreviewSession::TagTtl * 2));

        // An entry not used for the time limit is loaded again: the stamps of the entries in
        // the cache, which outlives a test run, are set back further than any limit.
        const std::string previews =
            Poco::Path(ConfigUtil::getConfigValue<std::string>("cache_files.path", ""),
                       "previews")
                .toString();
        const std::vector<std::string> entries = FileUtil::getDirEntries(previews);
        LOK_ASSERT_MESSAGE("The cache holds the entry of the file", !entries.empty());
        const std::string aged = Util::getIso8601FracformatTime(
            std::chrono::system_clock::now() - std::chrono::hours(24 * 400));
        for (const std::string& entry : entries)
        {
            std::ofstream stamp(Poco::Path(previews, entry).append("lastused").toString(),
                                std::ios::trunc);
            stamp << aged << '\n';
        }
        resetCountGetFile();
        getIndex(GoodToken, status);
        LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK), status);
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), getCountGetFile());
        LOK_ASSERT_EQUAL(3U, _brokersCreated.load());

        passTest("The previews outlive their document and follow its changes");
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[5]{ new UnitPreviewEndpoint(), new UnitPreviewEndpointExpiry(),
                             new UnitPreviewEndpointWriter(), new UnitPreviewEndpointCache(),
                             nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
