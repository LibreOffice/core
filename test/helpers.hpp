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
 * Common helper functions and utilities for testing. Independent implementations to avoid
 * reusing code under test.
 */

#pragma once

#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/HexUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Syscall.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <net/Uri.hpp>
#include <test/WebSocketSession.hpp>
#include <test/lokassert.hpp>
#include <test/testlog.hpp>
#include <tools/COOLWebSocket.hpp>
#include <wsd/TileDesc.hpp>

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>

#ifndef TDOC
#error TDOC must be defined (see Makefile.am)
#endif

// Sometimes we need to retry some commands as they can (due to timing or load) soft-fail.
constexpr int COMMAND_RETRY_COUNT = 5;

/// Common helper testing functions.
/// Avoid the temptation to reuse from COOL code!
/// These are supposed to be testing the latter.
namespace helpers
{

std::vector<char> genRandomData(const size_t size);

std::string genRandomString(const size_t size);

std::vector<char> readDataFromFile(const std::string& filename);

/// The content of a file of the test data directory, as a string.
std::string readFileAsString(const std::string& filename);

std::vector<char> readDataFromFile(std::unique_ptr<std::fstream>& file);

/// Make a temp copy of a file, and prepend it with a prefix.
/// Used by tests to avoid tainting the originals.
std::string getTempFileCopyPath(const std::string& srcDir, const std::string& srcFilename,
                                const std::string& dstFilenamePrefix);

/// Make a temp copy of a file.
/// Used by tests to avoid tainting the originals.
/// srcDir shouldn't end with '/' and srcFilename shouldn't contain '/'.
/// Returns the created file path.
std::string getTempFileCopyPath(const std::string& srcDir, const std::string& srcFilename);

void getDocumentPathAndURL(const std::string& docFilename, std::string& documentPath,
                           std::string& documentURL, std::string prefix);

void sendTextFrame(COOLWebSocket& socket, const std::string_view string,
                   const std::string_view testname);

void sendTextFrame(const std::shared_ptr<COOLWebSocket>& socket, const std::string_view string,
                   const std::string_view testname);

void sendTextFrame(const std::shared_ptr<http::WebSocketSession>& ws, const std::string_view string,
                   const std::string_view testname);

std::unique_ptr<Poco::Net::HTTPClientSession> createSession(const Poco::URI& uri);

/// Uses the internal http::Session to make an HTTP GET request.
std::shared_ptr<const http::Response> httpGet(const std::string& uri);

/// Uses the internal http::Session to make an HTTP GET request.
/// Optionally retries up to @retry times with @delayMs between attempts.
std::shared_ptr<const http::Response>
httpGetRetry(const std::string& uri, int retry = 3,
             std::chrono::milliseconds delayMs = std::chrono::seconds(1));

/// Builds a multipart/form-data body for http::Request.
/// Replaces Poco::Net::HTMLForm + FilePartSource + StringPartSource.
struct MultipartFormBody
{
    MultipartFormBody()
        : _boundary("----CoolFormBoundary" + Util::rng::getHexString(16))
    {
    }

    /// Add a simple name=value form field.
    void addField(const std::string& name, const std::string& value)
    {
        _parts.emplace_back(Part{ name, value, std::string(), std::string() });
    }

    /// Add a file part, reading from the given file path.
    void addFile(const std::string& name, const std::string& filePath,
                 const std::string& contentType = "application/octet-stream");

    /// Add a string part with explicit content type and filename (replaces StringPartSource).
    void addStringPart(const std::string& name, const std::string& content,
                       const std::string& contentType, const std::string& filename)
    {
        _parts.emplace_back(Part{ name, content, filename, contentType });
    }

    /// Build the multipart body and set it on the request.
    void applyTo(http::Request& request) const;

private:
    struct Part
    {
        std::string name;
        std::string value;
        std::string filename;
        std::string contentType;
    };

    std::string _boundary;
    std::vector<Part> _parts;
};

/// Returns true iff built with SSL and it is successfully initialized.
bool haveSsl();

/// Return a fully-qualified URI, with schema, to the test loopback server.
std::string const& getTestServerURI(const std::string& proto = "http");

std::vector<char>
getResponseMessage(COOLWebSocket& ws, const std::string_view prefix,
                   const std::string_view testname,
                   const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10));

std::vector<char>
getResponseMessage(const std::shared_ptr<http::WebSocketSession>& ws, const std::string_view prefix,
                   const std::string_view testname,
                   const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10));

std::shared_ptr<TileDesc>
getResponseDesc(const std::shared_ptr<http::WebSocketSession>& ws, const std::string_view prefix,
                const std::string_view testname,
                const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10));

std::string getResponseString(const std::shared_ptr<http::WebSocketSession>& ws,
                              const std::string_view prefix, const std::string_view testname,
                              const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10));

std::string
getResponseStringAny(const std::shared_ptr<http::WebSocketSession>& ws,
                     const std::vector<std::string_view>& prefixes, const std::string_view testname,
                     const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10));

std::vector<std::string>
getAllResponsesTimed(const std::shared_ptr<http::WebSocketSession>& ws,
                     const std::string_view prefix, const std::string_view testname,
                     const std::chrono::milliseconds timeoutMs = std::chrono::seconds(5));

std::string
assertResponseString(const std::shared_ptr<http::WebSocketSession>& ws,
                     const std::string_view prefix, const std::string_view testname,
                     const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10));

int countMessages(const std::shared_ptr<http::WebSocketSession>& ws, const std::string_view prefix,
                  const std::string_view testname,
                  const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10));

template <typename T>
std::string getResponseString(T& ws, const std::string_view prefix, const std::string_view testname,
                              const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10))
{
    const auto response = getResponseMessage(ws, prefix, testname, timeoutMs);
    return std::string(response.data(), response.size());
}

/// Assert that we don't get a response with the given prefix.
template <typename T>
std::string assertNotInResponse(T& ws, const std::string_view prefix,
                                const std::string_view testname)
{
    const auto res = getResponseString(ws, prefix, testname, std::chrono::milliseconds(1000));
    LOK_ASSERT_MESSAGE("Did not expect getting message [" + res + ']', res.empty());
    return res;
}

/// Parse the JSON body of a prefixed message such as "slideimport: {...}". A
/// reply that never arrived has no body to read, and says so as the exception a
/// malformed body raises, so that one catch covers both.
Poco::JSON::Object::Ptr parseJsonReply(const std::string& reply, const std::string& prefix);

/// Send a command and check the error reply it provokes.
/// Sends a command and asserts the error it is answered with. named is what the
/// answer says the command was for, such as " part=3", and is empty for an answer
/// that names nothing.
void assertErrorReply(const std::shared_ptr<http::WebSocketSession>& ws, const std::string& command,
                      const std::string& expectedCommand, const std::string& expectedKind,
                      const std::string& testname, const std::string& named = std::string());

/// Upload a file for the document of documentURL over the insertfile endpoint, with the
/// credential in the form field of the given name, and return the response. The body of an
/// accepted upload is the staged name.
std::shared_ptr<const http::Response> postToInsertFile(const std::string& documentURL,
                                                       const std::string& credentialField,
                                                       const std::string& credential,
                                                       const std::string& content);

bool getProgressWithIdValue(const std::string_view msg, const std::string_view idValue);

bool isDocumentLoaded(
    const std::shared_ptr<http::WebSocketSession>& ws, const std::string_view testname,
    bool isView = true,
    const std::chrono::milliseconds timeout = std::chrono::seconds(COMMAND_TIMEOUT_SECS * 4));

// Connecting to a Kit process is managed by document broker, that it does several
// jobs to establish the bridge connection between the Client and Kit process,
// The result, it is mostly time outs to get messages in the unit test and it could fail.
// connectLOKit ensures the websocket is connected to a kit process.
std::shared_ptr<http::WebSocketSession> connectLOKit(const std::shared_ptr<SocketPoll>& socketPoll,
                                                     const Poco::URI& uri, const std::string& url,
                                                     const std::string_view testname);

/// Load a document and get the WS Session.
/// By default, allow longer time for loading.
std::shared_ptr<http::WebSocketSession> loadDocAndGetSession(
    const std::shared_ptr<SocketPoll>& socketPoll, const Poco::URI& uri,
    const std::string& documentURL, const std::string_view testname, bool isView = true,
    bool isAssert = true, const std::string& loadParams = std::string(),
    const std::chrono::milliseconds timeout = std::chrono::seconds(COMMAND_TIMEOUT_SECS * 4));

std::shared_ptr<http::WebSocketSession>
loadDocAndGetSession(const std::shared_ptr<SocketPoll>& socketPoll, const std::string& docFilename,
                     const Poco::URI& uri, const std::string& testname, bool isView = true,
                     bool isAssert = true);

void SocketProcessor(const std::string& testname, const std::shared_ptr<http::WebSocketSession>& ws,
                     const std::function<bool(const std::string& msg)>& handler,
                     const std::chrono::milliseconds timeout = std::chrono::milliseconds(10000));

void parseDocSize(const std::string& message, const std::string& type, std::string& part,
                  int& parts, int& width, int& height, int& viewid, const std::string& testname);

std::vector<char> getTileMessage(const std::shared_ptr<http::WebSocketSession>& ws,
                                 const std::string& testname);

/// The parts' stable unique ids from the JSON payload of a status: message, in
/// document order: the part member of each entry of the parts array. These are
/// the numbers that name the parts of a presentation or drawing document in
/// part-carrying messages.
std::vector<std::string> parsePartUniqueIds(const std::string& message);

enum SpecialKey : std::uint16_t { skNone=0, skShift=0x1000, skCtrl=0x2000, skAlt=0x4000 };

int getCharChar(char ch, SpecialKey specialKeys);

int getCharKey(char ch, SpecialKey specialKeys);

void sendKeyEvent(std::shared_ptr<http::WebSocketSession>& socket, const char* type, int chr,
                  int key, const std::string& testname);

void sendKeyPress(std::shared_ptr<http::WebSocketSession>& socket, int chr, int key,
                  const std::string& testname);

void sendChar(std::shared_ptr<http::WebSocketSession>& socket, char ch, SpecialKey specialKeys,
              const std::string& testname);

void sendText(std::shared_ptr<http::WebSocketSession>& socket, const std::string& text,
              const std::string& testname);

void saveTileAs(const std::vector<char>& tileResponse, const std::string& filename,
                const std::string& testname);

template <typename T>
std::vector<char> getTileAndSave(T& socket, const std::string& req, const std::string& filename,
                                 const std::string& testname)
{
    TST_LOG("Requesting: " << req);
    sendTextFrame(socket, req, testname);

    const std::vector<char> tile = getResponseMessage(socket, "tile:", testname);
    TST_LOG(" Tile PNG size: " << tile.size());

    const std::string firstLine = COOLProtocol::getFirstLine(tile);
    std::vector<char> res(tile.begin() + firstLine.size() + 1, tile.end());
    std::stringstream streamRes;
    std::copy(res.begin(), res.end(), std::ostream_iterator<char>(streamRes));

    if (!filename.empty())
        saveTileAs(tile, filename, testname);

    return res;
}

template <typename T>
inline void getServerVersion(T& socket, int& major, int& minor, const std::string& testname)
{
    const std::string clientVersion = "coolclient 0.1";
    sendTextFrame(socket, clientVersion, testname);
    std::vector<char> loVersion = getResponseMessage(socket, "lokitversion", testname);
    std::string line = COOLProtocol::getFirstLine(loVersion.data(), loVersion.size());
    line = line.substr(strlen("lokitversion "));
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var loVersionVar = parser.parse(line);
    const Poco::SharedPtr<Poco::JSON::Object>& loVersionObject =
        loVersionVar.extract<Poco::JSON::Object::Ptr>();
    std::string loProductVersion = loVersionObject->get("ProductVersion").toString();
    std::istringstream stream(loProductVersion);
    stream >> major;
    if (stream.get() == '.')
    {
        stream >> minor;
    }
    else
    {
        minor = 0;
    }

    TST_LOG("Client [" << major << '.' << minor << "].");
}

bool svgMatch(const std::string& testname, const std::vector<char>& response,
              const char* templateFile);

/// Sends a command and waits for an event in response, with retrying.
bool sendAndWait(const std::shared_ptr<http::WebSocketSession>& ws, const std::string_view testname,
                 const std::string_view command, const std::string_view response,
                 std::chrono::milliseconds timeoutPerAttempt = std::chrono::seconds(10),
                 int repeat = COMMAND_RETRY_COUNT);

/// Drain all events.
/// Draining happens until nothing is received for @timeoutDrain.
void drain(const std::shared_ptr<http::WebSocketSession>& ws, const std::string& testname,
           std::chrono::milliseconds timeoutDrain = std::chrono::milliseconds(300));

/// Drain pending messages, then ask for the document status and return the
/// number of parts it reports.
std::size_t getPartCount(const std::shared_ptr<http::WebSocketSession>& ws,
                         const std::string& testname);

/// Sends a command and drain an event in response.
/// We expect @response within the given @timeoutResponse and
/// we drain all messages to get a steady-state.
/// Draining happens for @timeoutDrain.
bool sendAndDrain(const std::shared_ptr<http::WebSocketSession>& ws, const std::string& testname,
                  const std::string& command, const std::string& response,
                  std::chrono::milliseconds timeoutResponse = std::chrono::seconds(10),
                  std::chrono::milliseconds timeoutDrain = std::chrono::milliseconds(300));

/// Select all and wait for the text selection update.
bool selectAll(const std::shared_ptr<http::WebSocketSession>& ws, const std::string_view testname,
               std::chrono::milliseconds timeoutPerAttempt = std::chrono::seconds(10),
               int retry = COMMAND_RETRY_COUNT);

/// Delete all and wait for the text selection update.
bool deleteAll(const std::shared_ptr<http::WebSocketSession>& ws, const std::string_view testname,
               std::chrono::milliseconds timeoutPerAttempt = std::chrono::seconds(10),
               int retry = COMMAND_RETRY_COUNT);

std::string getAllText(const std::shared_ptr<http::WebSocketSession>& socket,
                       const std::string_view testname,
                       const std::string_view expected = std::string_view(),
                       int retry = COMMAND_RETRY_COUNT);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
