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
 * Dispatches incoming client HTTP/WebSocket requests to appropriate handlers.
 * Classes: ClientRequestDispatcher
 */

#pragma once

#include <net/Socket.hpp>
#include <wsd/RequestDetails.hpp>
#include <wsd/RequestVettingStation.hpp>
#if !MOBILEAPP
#include <wopi/WopiProxy.hpp>
#endif // !MOBILEAPP

#include <cstdint>
#include <memory>
#include <string>

/// Handles incoming connections and dispatches to the appropriate handler.
class ClientRequestDispatcher final : public SimpleSocketHandler
{
public:
    static void InitStaticFileContentCache()
    {
        StaticFileContentCache["discovery.xml"] = getDiscoveryXML();
    }

    using AsyncFn = std::function<void(bool)>;

    /// Uninitialize static data.
    static void uninitialize()
    {
        StaticFileContentCache.clear();

        RequestVettingStations.clear();
    }

private:
    // NB: these names are part of the published API, and should not be renamed or altered but can be expanded
    STATE_ENUM(CheckStatus, Ok, NotHttpSuccess, HostNotFound, WopiHostNotAllowed, UnspecifiedError,
               ConnectionAborted, CertificateValidation, SelfSignedCertificate, ExpiredCertificate,
               SslHandshakeFail, MissingSsl, NotHttps, NoScheme, Timeout, );

    /// Set the socket associated with this ResponseClient.
    void onConnect(const std::shared_ptr<StreamSocket>& socket) override;

    /// Called after successful socket reads.
    void handleIncomingMessage(SocketDisposition& disposition) override;

    /// Read the HTTP Header and create an HTTP Request.
    ssize_t readHeader(const std::shared_ptr<StreamSocket>& socket, Poco::Net::HTTPRequest& request,
                       std::chrono::duration<float, std::milli> delayMs);

    int getPollEvents(std::chrono::steady_clock::time_point /* now */,
                      int64_t& /* timeoutMaxMs */) override
    {
        return POLLIN;
    }

    void performWrites(std::size_t /*capacity*/) override {}

#if !MOBILEAPP
    /// Does this address feature in the allowed hosts list.
    static bool allowPostFrom(const std::string& address);

    static bool allowConvertTo(const std::string& address, const Poco::Net::HTTPRequest& request,
                               bool capabilityQuery, AsyncFn asyncCb);

    /// @return true if request has been handled synchronously and response sent, otherwise false
    bool handleRootRequest(const RequestDetails& requestDetails,
                           const std::shared_ptr<StreamSocket>& socket);

    /// @return true if request has been handled synchronously and response sent, otherwise false
    static bool handleFaviconRequest(const RequestDetails& requestDetails,
                                     const std::shared_ptr<StreamSocket>& socket);

    /// @return true if request has been handled synchronously and response sent, otherwise false
    bool handleWopiDiscoveryRequest(const RequestDetails& requestDetails,
                                    const std::shared_ptr<StreamSocket>& socket);

    /// @return true if request has been handled synchronously and response sent, otherwise false
    bool handleCapabilitiesRequest(const Poco::Net::HTTPRequest& request,
                                   const std::shared_ptr<StreamSocket>& socket);

    bool handleWopiAccessCheckRequest(const Poco::Net::HTTPRequest& request,
                                      const std::string& text,
                                      const std::shared_ptr<StreamSocket>& socket);

    /// @return true if request has been handled synchronously and response sent, otherwise false
    static bool handleClipboardRequest(const Poco::Net::HTTPRequest& request,
                                       std::istream& message,
                                       SocketDisposition& disposition,
                                       const std::shared_ptr<StreamSocket>& socket);

    /// @return true if request has been handled synchronously and response sent, otherwise false
    static bool handleSignatureRequest(const Poco::Net::HTTPRequest& request,
                                       const std::shared_ptr<StreamSocket>& socket);

    /// @return true if request has been handled synchronously and response sent, otherwise false
    static bool handleRobotsTxtRequest(const Poco::Net::HTTPRequest& request,
                                       const std::shared_ptr<StreamSocket>& socket);

    /// @return true if request has been handled synchronously and response sent, otherwise false
    static bool handleMediaRequest(const Poco::Net::HTTPRequest& request,
                                   SocketDisposition& /*disposition*/,
                                   const std::shared_ptr<StreamSocket>& socket,
                                   bool bVTT);

    /// @return true if request has been handled synchronously and response sent, otherwise false
    bool handlePostRequest(const RequestDetails& requestDetails,
                           const Poco::Net::HTTPRequest& request, std::istream& message,
                           SocketDisposition& disposition,
                           const std::shared_ptr<StreamSocket>& socket);

    bool handleClientProxyRequest(const Poco::Net::HTTPRequest& request,
                                  const RequestDetails& requestDetails,
                                  std::istream& message, SocketDisposition& disposition);

    static void sendResult(const std::shared_ptr<StreamSocket>& socket, CheckStatus result);

    enum class MessageResult : std::uint8_t
    {
        ServedAsync,
        ServedSync,
        Ignore
    };

    MessageResult handleMessage(Poco::Net::HTTPRequest& request,
                                std::istream& message,
                                SocketDisposition& disposition,
                                const std::shared_ptr<StreamSocket>& socket,
                                ssize_t headerSize);

    void finishedMessage(const Poco::Net::HTTPRequest& request,
                         const std::shared_ptr<StreamSocket>& socket,
                         bool servedSync, size_t preInBufferSz);

    void handleFullMessage(Poco::Net::HTTPRequest& request,
                           std::istream& message,
                           SocketDisposition& disposition,
                           const std::shared_ptr<StreamSocket>& socket,
                           ssize_t headerSize,
                           ssize_t contentSize,
                           bool eraseMessageFromSocket,
                           std::chrono::steady_clock::time_point now);
#endif // !MOBILEAPP

    /// @return true if request has been handled synchronously and response sent, otherwise false
    bool handleClientWsUpgrade(const Poco::Net::HTTPRequest& request,
                               const RequestDetails& requestDetails, SocketDisposition& disposition,
                               const std::shared_ptr<StreamSocket>& socket,
                               unsigned mobileAppDocId = 0);

    /// Lookup cached file content.
    static const std::string& getFileContent(const std::string& filename);

    /// Process the discovery.xml file and return as string.
    static std::string getDiscoveryXML();

    /// Keeps RVS instances in check.
    void CleanupRequestVettingStations();

    void onDisconnect() override
    {
        LOG_TRC("ClientRequestDispatcher " << _id << " disconnected");
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
        {
            socket->asyncShutdown(); // Flag for shutdown for housekeeping in SocketPoll.
            socket->shutdownConnection(); // Immediately disconnect.
        }
    }

private:
    // The socket that owns us (we can't own it).
    std::weak_ptr<StreamSocket> _socket;
    std::string _id;

    // Used for StreamSocket::parseHeader, net::Defaults::HTTPTimeout acting as max delay
    std::chrono::steady_clock::time_point _lastSeenHTTPHeader;

#if !MOBILEAPP
    /// WASM document request handler. Used only when WASM is enabled.
    std::unique_ptr<WopiProxy> _wopiProxy;
#endif // !MOBILEAPP

    /// The private RequestVettingStation. Held privately after the
    /// WS is created and as long as it is connected.
    std::shared_ptr<RequestVettingStation> _rvs;

    /// scratch dir that POSTs are streamed to
    std::unique_ptr<FileUtil::OwnedFile> _postFileDir;
    std::fstream _postStream;
#if !MOBILEAPP
    std::streamsize _postContentPending = 0;
#endif // !MOBILEAPP

    /// The current position while reading the header.
    std::size_t _headerPos = 0;

    /// The minimum number of RVS instances in flight to trigger cleanup.
    static constexpr std::size_t RvsLowWatermark = 1 * 1024;

    /// The absolute maximum number of RVS instances in flight.
    /// Note: exceeding this means we will not do parallel CheckFileInfo, ahead of loading.
    static constexpr std::size_t RvsHighWatermark = (10 * RvsLowWatermark) - 1;

    /// Any RVS instance, in RequestVettingStations, older than this will be purged.
    static constexpr std::chrono::seconds RvsMaxAge = std::chrono::seconds(60);

    /// The expected size of RVS to trigger the next cleanup.
    /// This is to avoid excessive cleanup attempts.
    static std::size_t NextRvsCleanupSize;

    /// External requests are first vetted before allocating DocBroker and Kit process.
    /// This is a map of the request URI to the RequestVettingStation for vetting.
    /// This is a temporary storage until we get the WS upgrade. If we don't, we purge.
    /// Note: this is accessed exclusively from websrv_poll, through
    /// handleIncomingMessage and handleClientWsUpgrade. Do *not* access in the ctor/dtor!
    static std::unordered_map<std::string, std::shared_ptr<RequestVettingStation>>
        RequestVettingStations;

    /// Cache for static files, to avoid reading and processing from disk.
    static std::map<std::string, std::string> StaticFileContentCache;

    /// The next unique connection-ID.
    static std::atomic<uint64_t> NextConnectionId;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
