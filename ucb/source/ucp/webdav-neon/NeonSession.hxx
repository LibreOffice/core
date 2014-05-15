/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_NEONSESSION_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_NEONSESSION_HXX

#include <config_lgpl.h>
#include <vector>
#include <osl/mutex.hxx>
#include "DAVSession.hxx"
#include "NeonTypes.hxx"
#include "NeonLockStore.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace ucbhelper { class ProxyDecider; }

namespace webdav_ucp
{

// A DAVSession implementation using the neon/expat library
class NeonSession : public DAVSession
{
private:
    osl::Mutex        m_aMutex;
    OUString     m_aScheme;
    OUString     m_aHostName;
    OUString     m_aProxyName;
    sal_Int32         m_nPort;
    sal_Int32         m_nProxyPort;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > m_aFlags;
    HttpSession *     m_pHttpSession;
    void *            m_pRequestData;
    const ucbhelper::InternetProxyDecider & m_rProxyDecider;

    // @@@ This should really be per-request data. But Neon currently
    // (0.23.5) has no interface for passing per-request user data.
    // Theoretically, a NeonSession instance could handle multiple requests
    // at a time --currently it doesn't. Thus this is not an issue at the
    // moment.
    DAVRequestEnvironment m_aEnv;

    static bool          m_bGlobalsInited;
    static NeonLockStore m_aNeonLockStore;

protected:
    virtual ~NeonSession();

public:
    NeonSession( const rtl::Reference< DAVSessionFactory > & rSessionFactory,
                 const OUString& inUri,
                 const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rFlags,
                 const ucbhelper::InternetProxyDecider & rProxyDecider )
        throw ( std::exception );

    // DAVSession methods
    virtual bool CanUse( const OUString & inPath,
                         const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rFlags ) SAL_OVERRIDE;

    virtual bool UsesProxy() SAL_OVERRIDE;

    const DAVRequestEnvironment & getRequestEnvironment() const
    { return m_aEnv; }

    virtual void
    OPTIONS( const OUString &  inPath,
             DAVCapabilities & outCapabilities,
             const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    // allprop & named
    virtual void
    PROPFIND( const OUString & inPath,
              const Depth inDepth,
              const std::vector< OUString > & inPropNames,
              std::vector< DAVResource > & ioResources,
              const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    // propnames
    virtual void
    PROPFIND( const OUString & inPath,
              const Depth inDepth,
              std::vector< DAVResourceInfo >& ioResInfo,
              const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void
    PROPPATCH( const OUString & inPath,
               const std::vector< ProppatchValue > & inValues,
               const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void
    HEAD( const OUString &  inPath,
          const std::vector< OUString > & inHeaderNames,
          DAVResource & ioResource,
          const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const OUString & inPath,
         const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void
    GET( const OUString & inPath,
         com::sun::star::uno::Reference<
             com::sun::star::io::XOutputStream > &  ioOutputStream,
         const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const OUString & inPath,
         const std::vector< OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void
    GET( const OUString & inPath,
         com::sun::star::uno::Reference<
             com::sun::star::io::XOutputStream > & ioOutputStream,
         const std::vector< OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void
    PUT( const OUString & inPath,
         const com::sun::star::uno::Reference<
             com::sun::star::io::XInputStream > & inInputStream,
         const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    POST( const OUString & inPath,
          const OUString & rContentType,
          const OUString & rReferer,
          const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & inInputStream,
          const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void
    POST( const OUString & inPath,
          const OUString & rContentType,
          const OUString & rReferer,
          const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & inInputStream,
          com::sun::star::uno::Reference<
              com::sun::star::io::XOutputStream > & oOutputStream,
          const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void
    MKCOL( const OUString & inPath,
           const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void
    COPY( const OUString & inSourceURL,
          const OUString & inDestinationURL,
          const DAVRequestEnvironment & rEnv,
          sal_Bool inOverWrite )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void
    MOVE( const OUString & inSourceURL,
          const OUString & inDestinationURL,
          const DAVRequestEnvironment & rEnv,
          sal_Bool inOverWrite )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void DESTROY( const OUString & inPath,
                          const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    // set new lock.
    virtual void LOCK( const OUString & inURL,
                       com::sun::star::ucb::Lock & inLock,
                       const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    // refresh existing lock.
    virtual sal_Int64 LOCK( const OUString & inURL,
                            sal_Int64 nTimeout,
                            const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    virtual void UNLOCK( const OUString & inURL,
                         const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) SAL_OVERRIDE;

    // helpers
    virtual void abort()
        throw ( std::exception ) SAL_OVERRIDE;

    const OUString & getHostName() const { return m_aHostName; }

    const ::uno::Reference< ::uno::XComponentContext > getComponentContext()
    { return m_xFactory->getComponentContext(); }

    const void * getRequestData() const { return m_pRequestData; }

    bool isDomainMatch( const OUString& certHostName );

private:
    friend class NeonLockStore;

    void Init( void )
        throw ( std::exception );

    void Init( const DAVRequestEnvironment & rEnv )
        throw ( std::exception );

    // ret: true => retry request.
    void HandleError( int nError,
                      const OUString & inPath,
                      const DAVRequestEnvironment & rEnv )
        throw ( std::exception );

    const ucbhelper::InternetProxyServer & getProxySettings() const;

    bool removeExpiredLocktoken( const OUString & inURL,
                                 const DAVRequestEnvironment & rEnv );

    // refresh lock, called by NeonLockStore::refreshLocks
    bool LOCK( NeonLock * pLock,
               sal_Int32 & rlastChanceToSendRefreshRequest );

    // unlock, called by NeonLockStore::~NeonLockStore
    bool UNLOCK( NeonLock * pLock );

    // low level GET implementation, used by public GET implementations
    static int GET( ne_session * sess,
                    const char * uri,
                    ne_block_reader reader,
                    bool getheaders,
                    void * userdata );

    // Buffer-based PUT implementation. Neon only has file descriptor-
    // based API.
    static int PUT( ne_session * sess,
                    const char * uri,
                    const char * buffer,
                    size_t size );

    // Buffer-based POST implementation. Neon only has file descriptor-
    // based API.
    int POST( ne_session * sess,
              const char * uri,
              const char * buffer,
              ne_block_reader reader,
              void * userdata,
              const OUString & rContentType,
              const OUString & rReferer );

    // Helper: XInputStream -> Sequence< sal_Int8 >
    static bool getDataFromInputStream(
        const com::sun::star::uno::Reference<
            com::sun::star::io::XInputStream > & xStream,
        com::sun::star::uno::Sequence< sal_Int8 > & rData,
        bool bAppendTrailingZeroByte );

    OUString makeAbsoluteURL( OUString const & rURL ) const;
};

} // namespace webdav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_NEONSESSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
