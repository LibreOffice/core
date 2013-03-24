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



#ifndef _DAVRESOURCEACCESS_HXX_
#define _DAVRESOURCEACCESS_HXX_

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
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    std::vector< SerfUri > m_aRedirectURIs;

public:
    DAVResourceAccess() : m_xSessionFactory( 0 ) {}
    DAVResourceAccess( const com::sun::star::uno::Reference<
                           com::sun::star::lang::XMultiServiceFactory > & rSMgr,
                       rtl::Reference<
                       DAVSessionFactory > const & rSessionFactory,
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
    //

    // allprop & named
    void
    PROPFIND( const Depth nDepth,
              const std::vector< OUString > & rPropertyNames,
              std::vector< DAVResource > & rResources,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    // propnames
    void
    PROPFIND( const Depth nDepth,
              std::vector< DAVResourceInfo > & rResInfo,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    PROPPATCH( const std::vector< ProppatchValue > & rValues,
               const com::sun::star::uno::Reference<
                   com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    void
    HEAD( const std::vector< OUString > & rHeaderNames, // empty == 'all'
          DAVResource & rResource,
          const com::sun::star::uno::Reference<
              com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const com::sun::star::uno::Reference<
             com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    GET( com::sun::star::uno::Reference<
             com::sun::star::io::XOutputStream > & rStream,
         const com::sun::star::uno::Reference<
             com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const std::vector< OUString > & rHeaderNames, // empty == 'all'
         DAVResource & rResource,
         const com::sun::star::uno::Reference<
             com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( DAVRequestHeaders & rRequestHeaders,
         const std::vector< rtl::OUString > & rHeaderNames, // empty == 'all'
         DAVResource & rResource,
         const com::sun::star::uno::Reference<
             com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    GET( com::sun::star::uno::Reference<
             com::sun::star::io::XOutputStream > & rStream,
         const std::vector< OUString > & rHeaderNames, // empty == 'all'
         DAVResource & rResource,
         const com::sun::star::uno::Reference<
             com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    PUT( const com::sun::star::uno::Reference<
             com::sun::star::io::XInputStream > & rStream,
         const com::sun::star::uno::Reference<
             com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    POST( const OUString & rContentType,
          const OUString & rReferer,
          const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & rInputStream,
          const com::sun::star::uno::Reference<
          com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    void
    POST( const OUString & rContentType,
          const OUString & rReferer,
          const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & rInputStream,
          com::sun::star::uno::Reference<
              com::sun::star::io::XOutputStream > & rOutputStream,
          const com::sun::star::uno::Reference<
              com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    void
    MKCOL( const com::sun::star::uno::Reference<
               com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    COPY( const OUString & rSourcePath,
          const OUString & rDestinationURI,
          sal_Bool bOverwrite,
          const com::sun::star::uno::Reference<
              com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    MOVE( const OUString & rSourcePath,
          const OUString & rDestinationURI,
          sal_Bool bOverwrite,
          const com::sun::star::uno::Reference<
              com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    DESTROY( const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    // set new lock.
    void
    LOCK( com::sun::star::ucb::Lock & inLock,
          const com::sun::star::uno::Reference<
              com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( DAVException );

#if 0 // currently not used, but please don't remove code
    // refresh existing lock.
    sal_Int64
    LOCK( sal_Int64 nTimeout,
          const com::sun::star::uno::Reference<
              com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );
#endif

    void
    UNLOCK( const com::sun::star::uno::Reference<
                com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    abort()
        throw ( DAVException );

    // helper
    static void
    getUserRequestHeaders(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > & xEnv,
        const rtl::OUString & rURI,
        com::sun::star::ucb::WebDAVHTTPMethod eMethod,
        DAVRequestHeaders & rRequestHeaders );

private:
    const OUString & getRequestURI() const;
    sal_Bool detectRedirectCycle( const OUString& rRedirectURL )
        throw ( DAVException );
    sal_Bool handleException( DAVException & e, int errorCount )
        throw ( DAVException );
    void initialize()
        throw ( DAVException );
};

} // namespace http_dav_ucp

#endif // _DAVRESOURCEACCESS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
