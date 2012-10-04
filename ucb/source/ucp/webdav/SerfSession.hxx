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



#ifndef INCLUDED_SERFSESSION_HXX
#define INCLUDED_SERFSESSION_HXX

#include <vector>
#include <boost/shared_ptr.hpp>
#include <osl/mutex.hxx>
#include <DAVSession.hxx>
#include <SerfTypes.hxx>
//#include "SerfLockStore.hxx"
#include <SerfUri.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace ucbhelper { class ProxyDecider; }

namespace http_dav_ucp
{

class SerfRequestProcessor;

// -------------------------------------------------------------------
// SerfSession
// A DAVSession implementation using the neon/expat library
// -------------------------------------------------------------------

class SerfSession : public DAVSession
{
private:
    osl::Mutex              m_aMutex;

    SerfUri                 m_aUri;

    rtl::OUString           m_aProxyName;
    sal_Int32               m_nProxyPort;

    SerfConnection*         m_pSerfConnection;
    serf_context_t*         m_pSerfContext;
    serf_bucket_alloc_t*    m_pSerfBucket_Alloc;
    bool                    m_bIsHeadRequestInProgress;
    bool                    m_bUseChunkedEncoding;
    sal_Int16               m_bNoOfTransferEncodingSwitches;

    const ucbhelper::InternetProxyDecider & m_rProxyDecider;

    DAVRequestEnvironment m_aEnv;

//    static SerfLockStore m_aSerfLockStore;

    char* getHostinfo();
    bool isSSLNeeded();

    SerfRequestProcessor* createReqProc( const rtl::OUString & inPath );

protected:
    virtual ~SerfSession();

public:
    SerfSession( const rtl::Reference< DAVSessionFactory > & rSessionFactory,
                 const rtl::OUString& inUri,
                 const ucbhelper::InternetProxyDecider & rProxyDecider )
        throw ( DAVException );

    // Serf library callbacks
    apr_status_t setupSerfConnection( apr_socket_t * inAprSocket,
                                      serf_bucket_t **outSerfInputBucket,
                                      serf_bucket_t **outSerfOutputBucket,
                                      apr_pool_t* inAprPool );

    apr_status_t provideSerfCredentials( bool bGiveProvidedCredentialsASecondTry,
                                         char ** outUsername,
                                         char ** outPassword,
                                         serf_request_t * inRequest,
                                         int inCode,
                                         const char *inAuthProtocol,
                                         const char *inRealm,
                                         apr_pool_t *inAprPool );

    apr_status_t verifySerfCertificateChain (
        int nFailures,
        const char** pCertificateChainBase64Encoded,
        int nCertificateChainLength);

    serf_bucket_t* acceptSerfResponse( serf_request_t * inSerfRequest,
                                       serf_bucket_t * inSerfStreamBucket,
                                       apr_pool_t* inAprPool );

    // Serf-related data structures
    apr_pool_t* getAprPool();
    serf_bucket_alloc_t* getSerfBktAlloc();
    serf_context_t* getSerfContext();
    SerfConnection* getSerfConnection();

    // DAVSession methods
    virtual sal_Bool CanUse( const ::rtl::OUString & inUri );

    virtual sal_Bool UsesProxy();

    const DAVRequestEnvironment & getRequestEnvironment() const
    { return m_aEnv; }

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

    bool isHeadRequestInProgress();

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

    const rtl::OUString & getHostName() const { return m_aUri.GetHost(); }
    int getPort() const { return m_aUri.GetPort(); }

    const ::uno::Reference< ::lang::XMultiServiceFactory > getMSF()
    { return m_xFactory->getServiceFactory(); }

    sal_Bool isDomainMatch( rtl::OUString certHostName );

private:
    friend class SerfLockStore;

    void Init( void )
        throw ( DAVException );

    void Init( const DAVRequestEnvironment & rEnv )
        throw ( DAVException );

    void HandleError( boost::shared_ptr<SerfRequestProcessor> rReqProc )
        throw ( DAVException );

    const ucbhelper::InternetProxyServer & getProxySettings() const;

    bool removeExpiredLocktoken( const rtl::OUString & inURL,
                                 const DAVRequestEnvironment & rEnv );

    // refresh lock, called by SerfLockStore::refreshLocks
    bool LOCK( SerfLock * pLock,
               sal_Int32 & rlastChanceToSendRefreshRequest );

    // unlock, called by SerfLockStore::~SerfLockStore
    bool UNLOCK( SerfLock * pLock );

    /*
    // low level GET implementation, used by public GET implementations
    static int GET( SerfConnection * sess,
                    const char * uri,
                    //ne_block_reader reader,
                    bool getheaders,
                    void * userdata );

    // Buffer-based PUT implementation. Serf only has file descriptor-
    // based API.
    static int PUT( SerfConnection * sess,
                    const char * uri,
                    const char * buffer,
                    size_t size );

    // Buffer-based POST implementation. Serf only has file descriptor-
    // based API.
    int POST( SerfConnection * sess,
              const char * uri,
              const char * buffer,
              //ne_block_reader reader,
              void * userdata,
              const rtl::OUString & rContentType,
              const rtl::OUString & rReferer );
    */

    // Helper: XInputStream -> Sequence< sal_Int8 >
    static bool getDataFromInputStream(
        const com::sun::star::uno::Reference<
            com::sun::star::io::XInputStream > & xStream,
        com::sun::star::uno::Sequence< sal_Int8 > & rData,
        bool bAppendTrailingZeroByte );

    /*
    rtl::OUString makeAbsoluteURL( rtl::OUString const & rURL ) const;
    */
};

} // namespace http_dav_ucp

#endif // INCLUDED_SERFSESSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
