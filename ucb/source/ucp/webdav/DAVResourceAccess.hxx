/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */



#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVRESOURCEACCESS_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVRESOURCEACCESS_HXX

#include <vector>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/WebDAVHTTPMethod.hpp>
#include "DAVAuthListener.hxx"
#include "DAVException.hxx"
#include "DAVSession.hxx"
#include "DAVResource.hxx"
#include "DAVTypes.hxx"
#include "SerfUri.hxx"

namespace http_dav_ucp
{

class DAVSessionFactory;

class DAVResourceAccess
{
    osl::Mutex    m_aMutex;
    OUString m_aURL;
    OUString m_aPath;
    rtl::Reference< DAVSession > m_xSession;
    rtl::Reference< DAVSessionFactory > m_xSessionFactory;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    std::vector< SerfUri > m_aRedirectURIs;

public:
    DAVResourceAccess() : m_xSessionFactory( 0 ) {}
    DAVResourceAccess( const css::uno::Reference< css::uno::XComponentContext > & rContext,
                       rtl::Reference< DAVSessionFactory > const & rSessionFactory,
                       const OUString & rURL );
    DAVResourceAccess( const DAVResourceAccess & rOther );

    DAVResourceAccess & operator=( const DAVResourceAccess & rOther );

    void setURL( const OUString & rNewURL )
        throw ( DAVException );

    void resetUri();

    const OUString & getURL() const { return m_aURL; }

    rtl::Reference< DAVSessionFactory > getSessionFactory() const
    { return m_xSessionFactory; }

    // DAV methods


    // allprop & named
    void
    PROPFIND( const Depth nDepth,
              const std::vector< OUString > & rPropertyNames,
              std::vector< DAVResource > & rResources,
              const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    // propnames
    void
    PROPFIND( const Depth nDepth,
              std::vector< DAVResourceInfo > & rResInfo,
              const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    PROPPATCH( const std::vector< ProppatchValue > & rValues,
               const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    void
    HEAD( const std::vector< OUString > & rHeaderNames, // empty == 'all'
          DAVResource & rResource,
          const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    css::uno::Reference< css::io::XInputStream >
    GET( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    GET( css::uno::Reference< css::io::XOutputStream > & rStream,
         const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    css::uno::Reference< css::io::XInputStream >
    GET( const std::vector< OUString > & rHeaderNames, // empty == 'all'
         DAVResource & rResource,
         const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    css::uno::Reference< css::io::XInputStream >
    GET( DAVRequestHeaders & rRequestHeaders,
         const std::vector< rtl::OUString > & rHeaderNames, // empty == 'all'
         DAVResource & rResource,
         const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    GET( css::uno::Reference< css::io::XOutputStream > & rStream,
         const std::vector< OUString > & rHeaderNames, // empty == 'all'
         DAVResource & rResource,
         const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    PUT( const css::uno::Reference< css::io::XInputStream > & rStream,
         const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    css::uno::Reference< css::io::XInputStream >
    POST( const OUString & rContentType,
          const OUString & rReferer,
          const css::uno::Reference< css::io::XInputStream > & rInputStream,
          const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    void
    POST( const OUString & rContentType,
          const OUString & rReferer,
          const css::uno::Reference< css::io::XInputStream > & rInputStream,
          css::uno::Reference< css::io::XOutputStream > & rOutputStream,
          const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    void
    MKCOL( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    COPY( const OUString & rSourcePath,
          const OUString & rDestinationURI,
          bool bOverwrite,
          const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    MOVE( const OUString & rSourcePath,
          const OUString & rDestinationURI,
          bool bOverwrite,
          const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    DESTROY( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    // set new lock.
    void
    LOCK( css::ucb::Lock & inLock,
          const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

#if 0 // currently not used, but please don't remove code
    // refresh existing lock.
    sal_Int64
    LOCK( sal_Int64 nTimeout,
          const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );
#endif

    void
    UNLOCK( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    static abort()
        throw ( DAVException );

    // helper
    static void
    getUserRequestHeaders(
        const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv,
        const rtl::OUString & rURI,
        css::ucb::WebDAVHTTPMethod eMethod,
        DAVRequestHeaders & rRequestHeaders );

private:
    const OUString & getRequestURI() const;
    bool detectRedirectCycle( const OUString& rRedirectURL )
        throw ( DAVException );
    bool handleException( DAVException & e, int errorCount )
        throw ( DAVException );
    void initialize()
        throw ( DAVException );
};

} // namespace http_dav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVRESOURCEACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
