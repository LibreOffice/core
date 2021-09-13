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

namespace http_dav_ucp
{
class CurlSession : public DAVSession
{
private:
    CurlUri m_URI;
    ::ucbhelper::InternetProxyDecider const& m_rProxyDecider;

public:
    explicit CurlSession(::rtl::Reference<DAVSessionFactory> const& rpFactory, OUString const& rURI,
                         ::ucbhelper::InternetProxyDecider const& rProxyDecider);
    virtual ~CurlSession() override;

    virtual auto CanUse(OUString const& rURI) -> bool override;

    virtual auto UsesProxy() -> bool override;

    // DAV methods
    virtual auto PROPFIND(OUString const& rInPath, Depth depth,
                          ::std::vector<OUString> const& rPropertyNames,
                          ::std::vector<DAVResource>& o_rResources,
                          DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto PROPFIND(OUString const& rInPath, Depth depth,
                          ::std::vector<DAVResourceInfo>& o_rResourceInfos,
                          DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto PROPPATCH(OUString const& rInPath, ::std::vector<ProppatchValue> const& rValues,
                           DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto HEAD(OUString const& rInPath, ::std::vector<OUString> const& rHeaderNames,
                      DAVResource& io_rResource, DAVRequestEnvironment const& rEnv)
        -> void override;

    virtual auto GET(OUString const& rInPath, DAVRequestEnvironment const& rEnv)
        -> css::uno::Reference<css::io::XInputStream> override;

    virtual auto GET(OUString const& rInPath,
                     css::uno::Reference<css::io::XOutputStream>& rxOutStream,
                     DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto GET(OUString const& rInPath, ::std::vector<OUString> const& rHeaderNames,
                     DAVResource& io_rResource, DAVRequestEnvironment const& rEnv)
        -> css::uno::Reference<css::io::XInputStream> override;

    virtual auto GET(OUString const& rInPath,
                     css::uno::Reference<css::io::XOutputStream>& rxOutStream,
                     ::std::vector<OUString> const& rHeaderNames, DAVResource& io_rResource,
                     DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto PUT(OUString const& rInPath,
                     css::uno::Reference<css::io::XInputStream> const& rxInStream,
                     DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto POST(OUString const& rInPath, OUString const& rContentType,
                      OUString const& rReferer,
                      css::uno::Reference<css::io::XInputStream> const& rxInStream,
                      DAVRequestEnvironment const& rEnv)
        -> css::uno::Reference<css::io::XInputStream> override;

    virtual auto POST(OUString const& rInPath, OUString const& rContentType,
                      OUString const& rReferer,
                      css::uno::Reference<css::io::XInputStream> const& rxInStream,
                      css::uno::Reference<css::io::XOutputStream>& rxOutStream,
                      DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto MKCOL(OUString const& rInPath, DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto COPY(OUString const& rInPathSource, OUString const& rInPathDestination,
                      DAVRequestEnvironment const& rEnv, bool isOverwrite = false) -> void override;

    virtual auto MOVE(OUString const& rInPathSource, OUString const& rInPathDestination,
                      DAVRequestEnvironment const& rEnv, bool isOverwrite = false) -> void override;

    virtual auto DESTROY(OUString const& rInPath, DAVRequestEnvironment const& rEnv)
        -> void override;

    virtual auto LOCK(OUString const& rInPath, css::ucb::Lock /*const*/& rLock,
                      DAVRequestEnvironment const& rEnv) -> void override;

    virtual auto LOCK(OUString const& rInPath, sal_Int64 nTimeout,
                      DAVRequestEnvironment const& rEnv) -> sal_Int64 override;

    virtual auto UNLOCK(OUString const& rInPath, DAVRequestEnvironment const& rEnv)
        -> void override;

    virtual auto abort() -> void override;
};

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
