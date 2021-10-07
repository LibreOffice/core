/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "DAVSession.hxx"
#include "CurlUri.hxx"

#include <curl/curl.h>

#include <mutex>

namespace http_dav_ucp
{
/// implementation of libcurl HTTP/DAV back-end
class CurlSession : public DAVSession
{
private:
    /// mutex required to access all other members
    ::std::mutex m_Mutex;
    css::uno::Reference<css::uno::XComponentContext> const m_xContext;
    /// flags may be passed to constructor, e.g. "KeepAlive"
    css::uno::Sequence<css::beans::NamedValue> const m_Flags;
    CurlUri const m_URI;
    /// buffer for libcurl detailed error messages
    char m_ErrorBuffer[CURL_ERROR_SIZE];
    /// proxy is used if aName is non-empty
    ::ucbhelper::InternetProxyServer const m_Proxy;
    /// once authentication was successful, rely on m_pCurl's data
    bool m_isAuthenticated = false;
    bool m_isAuthenticatedProxy = false;

    /// libcurl easy handle
    ::std::unique_ptr<CURL, deleter_from_fn<curl_easy_cleanup>> m_pCurl;

    // this class exists just to hide the implementation details in cxx file
    friend struct CurlProcessor;

public:
    explicit CurlSession(css::uno::Reference<css::uno::XComponentContext> const& xContext,
                         ::rtl::Reference<DAVSessionFactory> const& rpFactory, OUString const& rURI,
                         css::uno::Sequence<css::beans::NamedValue> const& rFlags,
                         ::ucbhelper::InternetProxyDecider const& rProxyDecider);
    virtual ~CurlSession() override;

    virtual auto CanUse(OUString const& rURI,
                        css::uno::Sequence<css::beans::NamedValue> const& rFlags) -> bool override;

    virtual auto UsesProxy() -> bool override;

    // DAV methods
    virtual auto PROPFIND(OUString const& rURIReference, Depth depth,
                          ::std::vector<OUString> const& rPropertyNames,
                          ::std::vector<DAVResource>& o_rResources,
                          DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto PROPFIND(OUString const& rURIReference, Depth depth,
                          ::std::vector<DAVResourceInfo>& o_rResourceInfos,
                          DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto PROPPATCH(OUString const& rURIReference,
                           ::std::vector<ProppatchValue> const& rValues,
                           DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto HEAD(OUString const& rURIReference, ::std::vector<OUString> const& rHeaderNames,
                      DAVResource& io_rResource, DAVRequestEnvironment const& rEnv)
        -> void override;

    virtual auto GET(OUString const& rURIReference, DAVRequestEnvironment const& rEnv)
        -> css::uno::Reference<css::io::XInputStream> override;

    virtual auto GET(OUString const& rURIReference,
                     css::uno::Reference<css::io::XOutputStream>& rxOutStream,
                     DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto GET(OUString const& rURIReference, ::std::vector<OUString> const& rHeaderNames,
                     DAVResource& io_rResource, DAVRequestEnvironment const& rEnv)
        -> css::uno::Reference<css::io::XInputStream> override;

    virtual auto GET(OUString const& rURIReference,
                     css::uno::Reference<css::io::XOutputStream>& rxOutStream,
                     ::std::vector<OUString> const& rHeaderNames, DAVResource& io_rResource,
                     DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto PUT(OUString const& rURIReference,
                     css::uno::Reference<css::io::XInputStream> const& rxInStream,
                     DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto POST(OUString const& rURIReference, OUString const& rContentType,
                      OUString const& rReferer,
                      css::uno::Reference<css::io::XInputStream> const& rxInStream,
                      DAVRequestEnvironment const& rEnv)
        -> css::uno::Reference<css::io::XInputStream> override;

    virtual auto POST(OUString const& rURIReference, OUString const& rContentType,
                      OUString const& rReferer,
                      css::uno::Reference<css::io::XInputStream> const& rxInStream,
                      css::uno::Reference<css::io::XOutputStream>& rxOutStream,
                      DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto MKCOL(OUString const& rURIReference, DAVRequestEnvironment const& rEnv)
        -> void override;

    virtual auto COPY(OUString const& rSourceURIReference, OUString const& rDestinationURI,
                      DAVRequestEnvironment const& rEnv, bool isOverwrite = false) -> void override;

    virtual auto MOVE(OUString const& rSourceURIReference, OUString const& rDestinationURI,
                      DAVRequestEnvironment const& rEnv, bool isOverwrite = false) -> void override;

    virtual auto DESTROY(OUString const& rURIReference, DAVRequestEnvironment const& rEnv)
        -> void override;

    virtual auto LOCK(OUString const& rURIReference, css::ucb::Lock /*const*/& rLock,
                      DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto UNLOCK(OUString const& rURIReference, DAVRequestEnvironment const& rEnv)
        -> void override;

    virtual auto abort() -> void override;

    auto NonInteractive_LOCK(OUString const& rURIReference,
                             sal_Int32& o_rLastChanceToSendRefreshRequest) -> bool;
    auto NonInteractive_UNLOCK(OUString const& rURIReference) -> void;
};

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
