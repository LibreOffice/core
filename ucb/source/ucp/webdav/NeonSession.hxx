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

#ifndef _NEONSESSION_HXX_
#define _NEONSESSION_HXX_

#include <vector>
#include <osl/mutex.hxx>
#include "DAVSession.hxx"
#include "NeonTypes.hxx"
#include "NeonLockStore.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace ucbhelper { class ProxyDecider; }

namespace webdav_ucp
{

// -------------------------------------------------------------------
// NeonSession
// A DAVSession implementation using the neon/expat library
// -------------------------------------------------------------------

class NeonSession : public DAVSession
{
private:
    osl::Mutex        m_aMutex;
    rtl::OUString     m_aScheme;
    rtl::OUString     m_aHostName;
    rtl::OUString     m_aProxyName;
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
                 const rtl::OUString& inUri,
                 const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rFlags,
                 const ucbhelper::InternetProxyDecider & rProxyDecider )
        throw ( DAVException );

    // DAVSession methods
    virtual sal_Bool CanUse( const ::rtl::OUString & inPath,
                             const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rFlags );

    virtual sal_Bool UsesProxy();

    const DAVRequestEnvironment & getRequestEnvironment() const
    { return m_aEnv; }

    virtual void
    OPTIONS( const ::rtl::OUString &  inPath,
             DAVCapabilities & outCapabilities,
             const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    // allprop & named
    virtual void
    PROPFIND( const ::rtl::OUString & inPath,
              const Depth inDepth,
              const std::vector< ::rtl::OUString > & inPropNames,
              std::vector< DAVResource > & ioResources,
              const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    // propnames
    virtual void
    PROPFIND( const ::rtl::OUString & inPath,
              const Depth inDepth,
              std::vector< DAVResourceInfo >& ioResInfo,
              const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual void
    PROPPATCH( const ::rtl::OUString & inPath,
               const std::vector< ProppatchValue > & inValues,
               const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual void
    HEAD( const ::rtl::OUString &  inPath,
          const std::vector< ::rtl::OUString > & inHeaderNames,
          DAVResource & ioResource,
          const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const ::rtl::OUString & inPath,
         const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual void
    GET( const ::rtl::OUString & inPath,
         com::sun::star::uno::Reference<
             com::sun::star::io::XOutputStream > &  ioOutputStream,
         const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const ::rtl::OUString & inPath,
         const std::vector< ::rtl::OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual void
    GET( const ::rtl::OUString & inPath,
         com::sun::star::uno::Reference<
             com::sun::star::io::XOutputStream > & ioOutputStream,
         const std::vector< ::rtl::OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual void
    PUT( const ::rtl::OUString & inPath,
         const com::sun::star::uno::Reference<
             com::sun::star::io::XInputStream > & inInputStream,
         const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    POST( const rtl::OUString & inPath,
          const rtl::OUString & rContentType,
          const rtl::OUString & rReferer,
          const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & inInputStream,
          const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual void
    POST( const rtl::OUString & inPath,
          const rtl::OUString & rContentType,
          const rtl::OUString & rReferer,
          const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & inInputStream,
          com::sun::star::uno::Reference<
              com::sun::star::io::XOutputStream > & oOutputStream,
          const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual void
    MKCOL( const ::rtl::OUString & inPath,
           const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual void
    COPY( const ::rtl::OUString & inSourceURL,
          const ::rtl::OUString & inDestinationURL,
          const DAVRequestEnvironment & rEnv,
          sal_Bool inOverWrite )
        throw ( DAVException );

    virtual void
    MOVE( const ::rtl::OUString & inSourceURL,
          const ::rtl::OUString & inDestinationURL,
          const DAVRequestEnvironment & rEnv,
          sal_Bool inOverWrite )
        throw ( DAVException );

    virtual void DESTROY( const ::rtl::OUString & inPath,
                          const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    // set new lock.
    virtual void LOCK( const ::rtl::OUString & inURL,
                       com::sun::star::ucb::Lock & inLock,
                       const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    // refresh existing lock.
    virtual sal_Int64 LOCK( const ::rtl::OUString & inURL,
                            sal_Int64 nTimeout,
                            const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    virtual void UNLOCK( const ::rtl::OUString & inURL,
                         const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    // helpers
    virtual void abort()
        throw ( DAVException );

    const rtl::OUString & getHostName() const { return m_aHostName; }

    const ::uno::Reference< ::lang::XMultiServiceFactory > getMSF()
    { return m_xFactory->getServiceFactory(); }

    const void * getRequestData() const { return m_pRequestData; }

    sal_Bool isDomainMatch( rtl::OUString certHostName );

private:
    friend class NeonLockStore;

    void Init( void )
        throw ( DAVException );

    void Init( const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    // ret: true => retry request.
    void HandleError( int nError,
                      const rtl::OUString & inPath,
                      const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    const ucbhelper::InternetProxyServer & getProxySettings() const;

    bool removeExpiredLocktoken( const rtl::OUString & inURL,
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
              const rtl::OUString & rContentType,
              const rtl::OUString & rReferer );

    // Helper: XInputStream -> Sequence< sal_Int8 >
    static bool getDataFromInputStream(
        const com::sun::star::uno::Reference<
            com::sun::star::io::XInputStream > & xStream,
        com::sun::star::uno::Sequence< sal_Int8 > & rData,
        bool bAppendTrailingZeroByte );

    rtl::OUString makeAbsoluteURL( rtl::OUString const & rURL ) const;
};

} // namespace webdav_ucp

#endif // _NEONSESSION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
