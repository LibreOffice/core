/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CurlSession.hxx"

namespace http_dav_ucp
{
CurlSession::CurlSession(::rtl::Reference<DAVSessionFactory> const& rpFactory, OUString const& rURI,
                         ::ucbhelper::InternetProxyDecider const& rProxyDecider)
    : DAVSession(rpFactory)
    , m_URI(rURI)
    , m_rProxyDecider(rProxyDecider)
{
}

CurlSession::~CurlSession() {}

auto CurlSession::CanUse(OUString const& rURI) -> bool {}

auto CurlSession::UsesProxy() -> bool {}

auto CurlSession::abort() -> void {}

// DAV methods
auto CurlSession::PROPFIND(OUString const& rURIReference, Depth const depth,
                           ::std::vector<OUString> const& rPropertyNames,
                           ::std::vector<DAVResource>& o_rResources,
                           DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::PROPFIND(OUString const& rURIReference, Depth const depth,
                           ::std::vector<DAVResourceInfo>& o_rResourceInfos,
                           DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::PROPPATCH(OUString const& rURIReference,
                            ::std::vector<ProppatchValue> const& rValues,
                            DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::HEAD(OUString const& rURIReference, ::std::vector<OUString> const& rHeaderNames,
                       DAVResource& io_rResource, DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::GET(OUString const& rURIReference, DAVRequestEnvironment const& rEnv)
    -> uno::Reference<io::XInputStream>
{
}

auto CurlSession::GET(OUString const& rURIReference, uno::Reference<io::XOutputStream>& rxOutStream,
                      DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::GET(OUString const& rURIReference, ::std::vector<OUString> const& rHeaderNames,
                      DAVResource& io_rResource, DAVRequestEnvironment const& rEnv)
    -> uno::Reference<io::XInputStream>
{
}

auto CurlSession::GET(OUString const& rURIReference, uno::Reference<io::XOutputStream>& rxOutStream,
                      ::std::vector<OUString> const& rHeaderNames, DAVResource& io_rResource,
                      DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::PUT(OUString const& rURIReference,
                      uno::Reference<io::XInputStream> const& rxInStream,
                      DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::POST(OUString const& rURIReference, OUString const& rContentType,
                       OUString const& rReferer, uno::Reference<io::XInputStream> const& rxInStream,
                       DAVRequestEnvironment const& rEnv) -> uno::Reference<io::XInputStream>
{
}

auto CurlSession::POST(OUString const& rURIReference, OUString const& rContentType,
                       OUString const& rReferer, uno::Reference<io::XInputStream> const& rxInStream,
                       uno::Reference<io::XOutputStream>& rxOutStream,
                       DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::MKCOL(OUString const& rURIReference, DAVRequestEnvironment const& rEnv) -> void {}

auto CurlSession::COPY(OUString const& rSourceURIReference, OUString const& rDestinationURI,
                       DAVRequestEnvironment const& rEnv, bool const isOverwrite) -> void
{
}

auto CurlSession::MOVE(OUString const& rSourceURIReference, OUString const& rDestinationURI,
                       DAVRequestEnvironment const& rEnv, bool const isOverwrite) -> void
{
}

auto CurlSession::DESTROY(OUString const& rURIReference, DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::LOCK(OUString const& rURIReference, ucb::Lock /*const*/& rLock,
                       DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::LOCK(OUString const& rURIReference, sal_Int64 const nTimeout,
                       DAVRequestEnvironment const& rEnv) -> sal_Int64
{
}

auto CurlSession::UNLOCK(OUString const& rURIReference, DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::NonInteractive_LOCK(::std::u16string_view const rURIReference,
                                      sal_Int32& o_rLastChanceToSendRefreshRequest) -> bool
{
    (void)this;
}

auto CurlSession::NonInteractive_UNLOCK(::std::u16string_view const rURIReference) -> void
{
    (void)this;
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
