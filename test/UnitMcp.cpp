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
 * End to end test for the Model Context Protocol endpoint: the handshake, the tool listing, running
 * each document tool over a document passed as base64 bytes and as an uploaded file, and the
 * failure and authentication paths.
 */

#include <config.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <typeinfo>

#include <unistd.h>

#include <Common.hpp>
#include <Protocol.hpp>
#include <Unit.hpp>
#include <common/JailUtil.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>
#include <wsd/McpTools.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Path.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/LayeredConfiguration.h>

using namespace std::literals;

namespace
{
const std::string ApiKey = "secret123";

bool contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

std::string readWholeFile(const std::string& path)
{
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

/// The number of files named filename anywhere under the directories that hold the incoming
/// documents of this process. The child root of a process is a directory under childRootBase whose
/// name starts with the process id and a dash.
std::size_t countIncomingFiles(const std::string& childRootBase, const std::string& filename)
{
    const std::string prefix = std::to_string(getpid()) + '-';
    std::error_code error;
    std::size_t count = 0;
    for (std::filesystem::directory_iterator root(childRootBase, error), rootEnd;
         !error && root != rootEnd; root.increment(error))
    {
        if (!root->path().filename().string().starts_with(prefix))
            continue;

        std::error_code innerError;
        for (std::filesystem::recursive_directory_iterator
                 it(root->path().string() + JailUtil::CHILDROOT_TMP_INCOMING_PATH, innerError),
             end;
             !innerError && it != end; it.increment(innerError))
        {
            if (it->path().filename() == filename)
                ++count;
        }
    }
    return count;
}

/// The name of a document whose kit exits when it is asked to save it, as a crash would end it.
const std::string KitCrashName = "kitcrash";

/// The largest document the endpoint accepts in this test.
constexpr std::int64_t MaxFileSize = 32 * 1024 * 1024;

/// The largest request body the server holds in memory while it arrives. A larger upload is written
/// to disk as it arrives.
constexpr std::uintmax_t MaxInMemoryBody = 16 * 1024;
}

/// Inside the WSD process.
class UnitMcp : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

public:
    UnitMcp()
        : UnitWSD("UnitMcp")
        , _workerStarted(false)
    {
        setHasKitHooks();
        setTimeout(1h);
    }

    ~UnitMcp()
    {
        if (_worker.joinable())
            _worker.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setBool("ssl.enable", true);
        config.setInt("per_document.limit_load_secs", 30);
        config.setBool("storage.filesystem[@allow]", false);

        // Switch the endpoint on and give it a key to require.
        config.setBool("ai.mcp.enable", true);
        config.setString("ai.mcp.api_key", ApiKey);
        config.setInt64("ai.mcp.max_file_size", MaxFileSize);
    }

    /// Send jsonBody to the message address path, with bearer as the Authorization header, or with
    /// no Authorization header when bearer is empty.
    std::shared_ptr<const http::Response>
    mcpCall(const std::shared_ptr<http::Session>& session, const std::string& jsonBody,
            const std::string& bearer = "Bearer " + ApiKey, const std::string& path = "/cool/mcp")
    {
        http::Request request(path, http::Request::VERB_POST);
        if (!bearer.empty())
            request.set("Authorization", bearer);
        request.setBody(jsonBody, "application/json");
        return session->syncRequest(request);
    }

    /// A POST to path with the key whose header states a body of length bytes. No byte of the body
    /// is ever sent.
    static http::Request headerOnlyPost(const std::string& path, std::int64_t length)
    {
        http::Request request(path, http::Request::VERB_POST);
        request.set("Authorization", "Bearer " + ApiKey);
        request.setBodySource([](char*, std::int64_t) { return 0; }, length);
        return request;
    }

    /// The answer to a tools/call of the named tool with the given arguments, as JSON text.
    std::shared_ptr<const http::Response> toolCall(const std::shared_ptr<http::Session>& session,
                                                   int id, const std::string& name,
                                                   const std::string& arguments)
    {
        return mcpCall(session, R"({"jsonrpc":"2.0","id":)" + std::to_string(id) +
                                    R"(,"method":"tools/call","params":{"name":")" + name +
                                    R"(","arguments":)" + arguments + "}}");
    }

    std::shared_ptr<const http::Response>
    mcpUpload(const std::shared_ptr<http::Session>& session, const std::string& path,
              const std::string& query, const std::string& bearer = "Bearer " + ApiKey)
    {
        http::Request request("/cool/mcp/upload" + query, http::Request::VERB_POST);
        request.set("Authorization", bearer);
        request.setBodyFile(path);
        return session->syncRequest(request);
    }

    std::shared_ptr<const http::Response> mcpGet(const std::shared_ptr<http::Session>& session,
                                                 const std::string& path,
                                                 const std::string& bearer = "Bearer " + ApiKey)
    {
        http::Request request(path, http::Request::VERB_GET);
        request.set("Authorization", bearer);
        return session->syncRequest(request);
    }

    static std::string fileIdFrom(const std::string& body)
    {
        try
        {
            Poco::JSON::Parser parser;
            auto object = parser.parse(body).extract<Poco::JSON::Object::Ptr>();
            return object->optValue<std::string>("file_id", std::string());
        }
        catch (const std::exception&)
        {
            return std::string();
        }
    }

    /// The content block at the given position in a tool result, as an object, or null.
    static Poco::JSON::Object::Ptr contentBlock(const std::string& body, std::size_t index)
    {
        try
        {
            Poco::JSON::Parser parser;
            auto root = parser.parse(body).extract<Poco::JSON::Object::Ptr>();
            auto result = root->getObject("result");
            if (!result)
                return nullptr;
            auto content = result->getArray("content");
            if (!content || content->size() <= index)
                return nullptr;
            return content->getObject(index);
        }
        catch (const std::exception&)
        {
            return nullptr;
        }
    }

    /// The first content block of a tool result, as an object, or null.
    static Poco::JSON::Object::Ptr firstContentBlock(const std::string& body)
    {
        return contentBlock(body, 0);
    }

    /// The base64 blob of a document returned inline as an embedded resource.
    static std::string resourceBlobFrom(const std::string& body)
    {
        auto block = firstContentBlock(body);
        if (!block)
            return std::string();
        auto resource = block->getObject("resource");
        if (!resource)
            return std::string();
        return resource->optValue<std::string>("blob", std::string());
    }

    /// The text of a tool result carried in a text content block.
    static std::string textResultFrom(const std::string& body)
    {
        auto block = firstContentBlock(body);
        if (!block)
            return std::string();
        return block->optValue<std::string>("text", std::string());
    }

    static std::string stringValueFrom(const std::string& json, const std::string& key)
    {
        try
        {
            Poco::JSON::Parser parser;
            auto object = parser.parse(json).extract<Poco::JSON::Object::Ptr>();
            return object->optValue<std::string>(key, std::string());
        }
        catch (const std::exception&)
        {
            return std::string();
        }
    }

    void runTest(const std::shared_ptr<http::Session>& session)
    {
        // The configured child root, under which this process has a child root of its own.
        const std::string childRootBase =
            Poco::Path(Poco::Util::Application::instance().config().getString("child_root_path"))
                .makeAbsolute()
                .toString();

        const std::string helloPath = Poco::Path(TDOC, "hello.odt").toString();
        const std::string helloData = Util::base64Encode(readWholeFile(helloPath));

        // The handshake negotiates a supported protocol version and returns the numeric request id
        // unchanged.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":1,"method":"initialize",)"
                         R"("params":{"protocolVersion":"2025-06-18"}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("protocolVersion":"2025-06-18")"));
            LOK_ASSERT(contains(response->getBody(), R"("id":1)"));
        }

        // The tool listing names both document tools.
        {
            auto response =
                mcpCall(session, R"({"jsonrpc":"2.0","id":2,"method":"tools/list"})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("name":"extract_document_structure")"));
            LOK_ASSERT(contains(response->getBody(), R"("name":"extract_link_targets")"));
            LOK_ASSERT(contains(response->getBody(), R"("name":"convert_document")"));
            LOK_ASSERT(contains(response->getBody(), R"("name":"transform_document_structure")"));
        }

        // Extracting the text of a document passed as base64 bytes returns its body, and is not
        // marked as an error.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":3,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + helloData + R"(","filename":"hello.odt",)"
                         R"("filter":"text"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));
            LOK_ASSERT(contains(response->getBody(), "Hello world"));
        }

        // The same document uploaded once and then named by its file_id gives the same result.
        {
            auto uploadResponse = mcpUpload(session, helloPath, "?filename=hello.odt");
            LOK_ASSERT(uploadResponse->statusCode() == http::StatusCode::OK);
            const std::string fileId = fileIdFrom(uploadResponse->getBody());
            LOK_ASSERT(!fileId.empty());
            LOK_ASSERT(contains(uploadResponse->getBody(), R"("expires_in":900)"));

            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":4,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"file_id":")" + fileId + R"(","filter":"text"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));
            LOK_ASSERT(contains(response->getBody(), "Hello world"));
        }

        // A document too large to be held in memory while it arrives is uploaded and named by its
        // file_id in the same way.
        {
            // A body of up to 16 KB is held in memory, and a larger one is written to disk.
            const std::string largePath = Poco::Path(TDOC, "docStructure.docx").toString();
            LOK_ASSERT(std::filesystem::file_size(largePath) > MaxInMemoryBody);
            auto uploadResponse = mcpUpload(session, largePath, "?filename=docStructure.docx");
            LOK_ASSERT(uploadResponse->statusCode() == http::StatusCode::OK);
            const std::string fileId = fileIdFrom(uploadResponse->getBody());
            LOK_ASSERT(!fileId.empty());

            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":23,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"file_id":")" + fileId + R"(","filter":"text"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));
        }

        // Listing link targets of a document passed as base64 bytes returns a categorised result.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":5,"method":"tools/call","params":)"
                         R"({"name":"extract_link_targets","arguments":)"
                         R"({"data":")" + helloData + R"(","filename":"hello.odt"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));
            LOK_ASSERT(contains(response->getBody(), "Headings"));
        }

        // An unknown file_id is a tool failure the model can read, not a protocol error.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":6,"method":"tools/call","params":)"
                         R"({"name":"extract_link_targets","arguments":)"
                         R"({"file_id":"does-not-exist"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(!contains(response->getBody(), R"("error")"));
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()),
                                "The file_id is unknown or has expired."));
        }

        // A document given both inline and by file_id, given neither way, or given as text that is
        // not base64, is a tool failure that says how to give it.
        {
            auto response = toolCall(session, 26, "extract_link_targets",
                                     R"({"data":")" + helloData + R"(","file_id":"abc"})");
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()),
                                "Provide the document as only one of the data or file_id"));

            response = toolCall(session, 27, "extract_link_targets", R"({"filename":"a.odt"})");
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()),
                                "Provide the document as the data or the file_id argument."));

            response = toolCall(session, 28, "extract_link_targets", R"({"data":"abc"})");
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()),
                                "The data argument is not valid base64"));
        }

        // A deck asked for in a design that no template has is a tool failure that points to the
        // list of designs.
        {
            const std::string slide = R"({"part":"body","intent":"bullets","title":"Slide",)"
                                      R"("blocks":[{"kind":"bullets","items":["one"]}]})";
            auto response =
                toolCall(session, 29, "create_presentation",
                         R"({"design":"No Such Design","spec":{"slides":[)" + slide + "]}}");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            const std::string text = textResultFrom(response->getBody());
            LOK_ASSERT(contains(text, "There is no design template named \"No Such Design\"."));
            LOK_ASSERT(contains(text, "Call list_design_templates"));
        }

        // Converting a small text document to PDF inline returns the produced PDF as an embedded
        // resource whose bytes begin with the PDF marker.
        {
            const std::string textData = Util::base64Encode(std::string("hello world"));
            auto response =
                mcpCall(session, R"({"jsonrpc":"2.0","id":7,"method":"tools/call","params":)"
                                 R"({"name":"convert_document","arguments":)"
                                 R"({"data":")" +
                                     textData +
                                     R"(","filename":"hello.txt","format":"pdf"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));

            const std::string blob = resourceBlobFrom(response->getBody());
            LOK_ASSERT(!blob.empty());
            const std::string pdf = Util::base64Decode(blob);
            LOK_ASSERT(pdf.rfind("%PDF-", 0) == 0);
        }

        // The same conversion asked for as a download link returns a JSON text result naming a
        // download path. Fetching it with the bearer token gives the PDF with the right content
        // type; a second fetch finds nothing because the link serves the document only once.
        {
            const std::string textData = Util::base64Encode(std::string("hello world"));
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":8,"method":"tools/call","params":)"
                         R"({"name":"convert_document","arguments":)"
                         R"({"data":")" + textData + R"(","filename":"hello.txt",)"
                         R"("format":"pdf","output":"link"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));

            // The link is a full address of this server, ending in the download path.
            const std::string linkJson = textResultFrom(response->getBody());
            const std::string downloadUrl = stringValueFrom(linkJson, "download_url");
            LOK_ASSERT(downloadUrl.rfind("http", 0) == 0);
            const std::size_t pathStart = downloadUrl.find("/cool/mcp/download/");
            LOK_ASSERT(pathStart != std::string::npos);
            const std::string downloadPath = downloadUrl.substr(pathStart);
            LOK_ASSERT(contains(linkJson, R"("mimeType":"application/pdf")"));

            // The produced document waits in the store until it is fetched, and is gone from disk
            // once it has been served.
            LOK_ASSERT_EQUAL(static_cast<std::size_t>(1),
                             countIncomingFiles(childRootBase, "hello.pdf"));

            auto download = mcpGet(session, downloadPath);
            LOK_ASSERT(download->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(std::string(download->getBody()).rfind("%PDF-", 0) == 0);
            LOK_ASSERT(download->get("Content-Type") == "application/pdf");
            LOK_ASSERT_EQUAL_STR(R"(attachment; filename="hello.pdf"; filename*=UTF-8''hello.pdf)",
                                 download->get("Content-Disposition"));
            LOK_ASSERT_EQUAL(static_cast<std::size_t>(0),
                             countIncomingFiles(childRootBase, "hello.pdf"));

            // The link is single use, and a second fetch is told so.
            auto again = mcpGet(session, downloadPath);
            LOK_ASSERT(again->statusCode() == http::StatusCode::Gone);

            // A link that was never issued is not found.
            auto unknown = mcpGet(session, "/cool/mcp/download/0123456789abcdef");
            LOK_ASSERT(unknown->statusCode() == http::StatusCode::NotFound);
        }

        // Fetching a download link without the right bearer token is refused before the link is
        // looked up, and the reply names the credential it lacks.
        {
            auto response =
                mcpGet(session, "/cool/mcp/download/does-not-exist", "Bearer wrong-key");
            LOK_ASSERT(response->statusCode() == http::StatusCode::Unauthorized);
            LOK_ASSERT(contains(response->getBody(), "bearer token"));
        }

        // Transforming a document with content controls returns the changed document, an OOXML
        // package whose first content control holds the new value.
        {
            const std::string structurePath = Poco::Path(TDOC, "docStructure.docx").toString();
            const std::string structureData = Util::base64Encode(readWholeFile(structurePath));
            const std::string transform =
                R"({\"Transforms\":{\"ContentControls.ByIndex.0\":{\"content\":\"Short text\"}}})";
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":9,"method":"tools/call","params":)"
                         R"({"name":"transform_document_structure","arguments":)"
                         R"({"data":")" + structureData + R"(","filename":"docStructure.docx",)"
                         R"("transform":")" + transform + R"(","format":"docx"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));

            const std::string blob = resourceBlobFrom(response->getBody());
            LOK_ASSERT(!blob.empty());
            const std::string docx = Util::base64Decode(blob);
            // An OOXML package is a zip archive, so it begins with the zip local file header
            // marker.
            LOK_ASSERT(docx.rfind("PK", 0) == 0);

            auto controls = mcpCall(
                session, R"({"jsonrpc":"2.0","id":25,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + blob + R"(","filename":"docStructure.docx",)"
                         R"("filter":"contentcontrol"}}})");
            LOK_ASSERT(contains(controls->getBody(), R"("isError":false)"));
            const std::string structure = textResultFrom(controls->getBody());
            LOK_ASSERT(contains(structure, "ContentControls.ByIndex.0"));
            LOK_ASSERT(contains(structure, "Short text"));
            LOK_ASSERT(!contains(structure, "plain text value"));
        }

        // A transform that names a key the engine does not know returns the document with the rest
        // applied, and a note that names the key.
        {
            const std::string structurePath = Poco::Path(TDOC, "docStructure.docx").toString();
            const std::string structureData = Util::base64Encode(readWholeFile(structurePath));
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":12,"method":"tools/call","params":)"
                         R"({"name":"transform_document_structure","arguments":)"
                         R"({"data":")" + structureData + R"(","filename":"docStructure.docx",)"
                         R"("transform":"{\"Transforms\":{\"NoSuchKey\":1}}"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));
            LOK_ASSERT(Util::base64Decode(resourceBlobFrom(response->getBody())).rfind("PK", 0) ==
                       0);
            auto note = contentBlock(response->getBody(), 1);
            LOK_ASSERT(note);
            LOK_ASSERT(contains(note->optValue<std::string>("text", std::string()), "NoSuchKey"));
        }

        // Replacing text in a text document returns the document with the text replaced.
        {
            const std::string sourcePath = Poco::Path(TDOC, "hello.odt").toString();
            const std::string sourceData = Util::base64Encode(readWholeFile(sourcePath));
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":18,"method":"tools/call","params":)"
                         R"({"name":"transform_document_structure","arguments":)"
                         R"({"data":")" + sourceData + R"(","filename":"hello.odt",)"
                         R"("transform":"{\"Transforms\":{\"ReplaceText\":)"
                         R"({\"search\":\"world\",\"replace\":\"there\"}}}"}}})");
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));

            const std::string replaced = resourceBlobFrom(response->getBody());
            auto text = mcpCall(
                session, R"({"jsonrpc":"2.0","id":19,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + replaced + R"(","filename":"hello.odt",)"
                         R"("filter":"text"}}})");
            LOK_ASSERT(contains(textResultFrom(text->getBody()), "there"));
            LOK_ASSERT(!contains(textResultFrom(text->getBody()), "world"));
        }

        // A spreadsheet has no transforms, and a transform on one says so.
        {
            const std::string sheetPath = Poco::Path(TDOC, "convert-to.xlsx").toString();
            const std::string sheetData = Util::base64Encode(readWholeFile(sheetPath));
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":13,"method":"tools/call","params":)"
                         R"({"name":"transform_document_structure","arguments":)"
                         R"({"data":")" + sheetData + R"(","filename":"convert-to.xlsx",)"
                         R"("transform":"{\"Transforms\":{}}"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()), "spreadsheet"));

            // Listing the link targets of a spreadsheet answers with its targets.
            auto targets = mcpCall(
                session, R"({"jsonrpc":"2.0","id":17,"method":"tools/call","params":)"
                         R"({"name":"extract_link_targets","arguments":)"
                         R"({"data":")" + sheetData + R"(","filename":"convert-to.xlsx"}}})");
            LOK_ASSERT(contains(targets->getBody(), R"("isError":false)"));
            LOK_ASSERT(contains(textResultFrom(targets->getBody()), "Targets"));

            // Extracting from a spreadsheet with no filter answers with a JSON-RPC response whose
            // result is the structure of the spreadsheet as a JSON object.
            auto extract = mcpCall(
                session, R"({"jsonrpc":"2.0","id":14,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + sheetData + R"(","filename":"convert-to.xlsx"}}})");
            LOK_ASSERT(extract->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(extract->get("Content-Type") == "application/json");
            LOK_ASSERT(contains(extract->getBody(), R"("id":14)"));
            LOK_ASSERT(contains(extract->getBody(), R"("isError":false)"));
            Poco::JSON::Parser parser;
            LOK_ASSERT(parser.parse(textResultFrom(extract->getBody()))
                           .type() == typeid(Poco::JSON::Object::Ptr));
        }

        // A filter that is not one of the views, and a range that is not a cell range, are readable
        // failures that name the argument.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":20,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + helloData + R"(","filename":"hello.odt",)"
                         R"("filter":"text&x=1"}}})");
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()), "filter argument"));

            response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":21,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + helloData + R"(","filename":"hello.odt",)"
                         R"("range":"A1:B5 "}}})");
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()), "range argument"));
        }

        // Converting to a format the engine does not know is a readable tool failure, not a
        // protocol error.
        {
            const std::string textData = Util::base64Encode(std::string("hello world"));
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":10,"method":"tools/call","params":)"
                         R"({"name":"convert_document","arguments":)"
                         R"({"data":")" + textData + R"(","filename":"hello.txt",)"
                         R"("format":"nosuchformat"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(!contains(response->getBody(), R"("error")"));
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
        }

        // Uploading without the right bearer token is refused, and the reply names the credential
        // it lacks.
        {
            auto response =
                mcpUpload(session, helloPath, "?filename=hello.odt", "Bearer wrong-key");
            LOK_ASSERT(response->statusCode() == http::StatusCode::Unauthorized);
            LOK_ASSERT(contains(response->getBody(), "bearer token"));
        }

        // An upload is accepted when the scheme of the authorization header is written in another
        // case.
        {
            auto response =
                mcpUpload(session, helloPath, "?filename=hello.odt", "bearer " + ApiKey);
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
        }

        const std::string initialize = R"({"jsonrpc":"2.0","id":30,"method":"initialize"})";

        // A request with the key from a web page, which carries an Origin header, is forbidden.
        {
            http::Request request("/cool/mcp", http::Request::VERB_POST);
            request.set("Authorization", "Bearer " + ApiKey);
            request.set("Origin", "https://example.com");
            request.setBody(initialize, "application/json");
            auto response = session->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::Forbidden, response->statusCode());
        }

        // A message without the key is refused and told to send a bearer token: with no
        // Authorization header, with the scheme and no key, with the key one character short or one
        // character long, and with the key under another scheme.
        const std::string shortKey = ApiKey.substr(0, ApiKey.size() - 1);
        for (const std::string& authorization :
             { std::string(), std::string("Bearer"), "Bearer " + shortKey, "Bearer " + ApiKey + 'x',
               "Basic " + ApiKey })
        {
            TST_LOG("Sending a message with authorization [" << authorization << ']');
            auto response = mcpCall(session, initialize, authorization);
            LOK_ASSERT_EQUAL(http::StatusCode::Unauthorized, response->statusCode());
            LOK_ASSERT_EQUAL_STR("Bearer", response->get("WWW-Authenticate"));
        }

        // The address written in hex is the same endpoint: it is refused without the key and
        // answered with it.
        {
            auto refused = mcpCall(session, initialize, std::string(), "/cool/0x6d6370");
            LOK_ASSERT_EQUAL(http::StatusCode::Unauthorized, refused->statusCode());
            LOK_ASSERT_EQUAL_STR("Bearer", refused->get("WWW-Authenticate"));

            auto answered = mcpCall(session, initialize, "Bearer " + ApiKey, "/cool/0x6d6370");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, answered->statusCode());
            LOK_ASSERT(contains(answered->getBody(), R"("serverInfo")"));
        }

        // A POST must state the length of its body. One sent in chunks, or with no Content-Length,
        // is refused.
        {
            http::Request chunked("/cool/mcp", http::Request::VERB_POST);
            chunked.set("Authorization", "Bearer " + ApiKey);
            chunked.set("Transfer-Encoding", "chunked");
            auto response = session->syncRequest(chunked);
            LOK_ASSERT_EQUAL(http::StatusCode::LengthRequired, response->statusCode());

            http::Request unstated("/cool/mcp", http::Request::VERB_POST);
            unstated.set("Authorization", "Bearer " + ApiKey);
            response = session->syncRequest(unstated);
            LOK_ASSERT_EQUAL(http::StatusCode::LengthRequired, response->statusCode());
        }

        // A message over the most a message may hold, and an upload over the largest document the
        // server accepts, are refused on their header alone. No byte of either body is sent, so the
        // answer shows the server decided from the header.
        {
            auto response = session->syncRequest(headerOnlyPost(
                "/cool/mcp", static_cast<std::int64_t>(McpTools::MaxMessageBytes) + 1));
            LOK_ASSERT_EQUAL(http::StatusCode::PayloadTooLarge, response->statusCode());
            LOK_ASSERT(contains(response->getBody(), "Upload a large document"));

            response = session->syncRequest(
                headerOnlyPost("/cool/mcp/upload?filename=big.odt", MaxFileSize + 1));
            LOK_ASSERT_EQUAL(http::StatusCode::PayloadTooLarge, response->statusCode());
            LOK_ASSERT(contains(response->getBody(), "larger than the server accepts"));
        }

        // The message address takes only POST, and a download link only GET.
        {
            auto response = mcpGet(session, "/cool/mcp");
            LOK_ASSERT_EQUAL(http::StatusCode::MethodNotAllowed, response->statusCode());
            LOK_ASSERT_EQUAL_STR("POST", response->get("Allow"));

            response = mcpCall(session, "{}", "Bearer " + ApiKey,
                               "/cool/mcp/download/0123456789abcdef");
            LOK_ASSERT_EQUAL(http::StatusCode::MethodNotAllowed, response->statusCode());
            LOK_ASSERT_EQUAL_STR("GET", response->get("Allow"));
        }

        // An upload with an empty body is a bad request.
        {
            http::Request request("/cool/mcp/upload?filename=empty.odt", http::Request::VERB_POST);
            request.set("Authorization", "Bearer " + ApiKey);
            request.setBody(std::string(), "application/octet-stream");
            auto response = session->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::BadRequest, response->statusCode());
        }

        // A tool call whose kit ends before the result is ready is answered with a failure that
        // says so.
        {
            const std::string textData = Util::base64Encode(std::string("hello world"));
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":24,"method":"tools/call","params":)"
                         R"({"name":"convert_document","arguments":)"
                         R"({"data":")" + textData + R"(","filename":")" + KitCrashName +
                             R"(.txt","format":"pdf"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("id":24)"));
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()),
                                "stopped before the result was ready"));
        }
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
                    auto session = http::Session::create(helpers::getTestServerURI());
                    session->setTimeout(std::chrono::seconds(30));
                    runTest(session);
                    exitTest(TestResult::Ok);
                }
                catch (const std::exception& exc)
                {
                    LOG_TST("UnitMcp failed: " << exc.what());
                    exitTest(TestResult::Failed);
                }
            });
    }
};

/// Inside the forkit and kit processes.
class UnitKitMcp : public UnitKit
{
public:
    UnitKitMcp()
        : UnitKit("UnitKitMcp")
    {
        setTimeout(1h);
    }

    bool filterKitMessage(WebSocketHandler*, std::string& message) override
    {
        if (message.find(" saveas ") != std::string::npos &&
            message.find(KitCrashName) != std::string::npos)
        {
            TST_LOG("Ending the kit on [" << message << ']');
            _exit(0);
        }

        return false;
    }
};

UnitBase* unit_create_wsd(void) { return new UnitMcp(); }

UnitBase* unit_create_kit(void) { return new UnitKitMcp(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
