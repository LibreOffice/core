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


#pragma once

#include <osl/mutex.hxx>
#include <memory>
#include <vector>
#include "DAVSession.hxx"
#include "SerfUri.hxx"

#include <serf.h>

namespace ucbhelper { class ProxyDecider; }

namespace http_dav_ucp
{

class SerfRequestProcessor;


// SerfSession
// A DAVSession implementation using the neon/expat library


class SerfSession : public DAVSession
{
private:
    osl::Mutex              m_aMutex;

    SerfUri                 m_aUri;

    OUString           m_aProxyName;
    sal_Int32               m_nProxyPort;

    serf_connection_t*      m_pSerfConnection;
    serf_context_t*         m_pSerfContext;
    serf_bucket_alloc_t*    m_pSerfBucket_Alloc;
    bool                    m_bIsHeadRequestInProgress;
    bool                    m_bUseChunkedEncoding;
    sal_Int16               m_bNoOfTransferEncodingSwitches;

    const ucbhelper::InternetProxyDecider & m_rProxyDecider;

    DAVRequestEnvironment m_aEnv;

    char* getHostinfo();
    bool isSSLNeeded();

    SerfRequestProcessor* createReqProc( const OUString & inPath );

protected:
    virtual ~SerfSession() override;

public:
    /// @throws DAVException
    SerfSession( const rtl::Reference< DAVSessionFactory > & rSessionFactory,
                 const OUString& inUri,
                 const ucbhelper::InternetProxyDecider & rProxyDecider );

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
        const serf_ssl_certificate_t * const * pCertificateChainBase64Encoded,
        int nCertificateChainLength);

    serf_bucket_t* acceptSerfResponse( serf_request_t * inSerfRequest,
                                       serf_bucket_t * inSerfStreamBucket,
                                       apr_pool_t* inAprPool );

    // Serf-related data structures
    static apr_pool_t* getAprPool();
    serf_bucket_alloc_t* getSerfBktAlloc();
    serf_context_t* getSerfContext();
    serf_connection_t* getSerfConnection();

    // DAVSession methods
    virtual bool CanUse( const OUString & inUri ) override;

    virtual bool UsesProxy() override;

    const DAVRequestEnvironment & getRequestEnvironment() const
    { return m_aEnv; }

    // allprop & named
    virtual void
    PROPFIND( const OUString & inPath,
              const Depth inDepth,
              const std::vector< OUString > & inPropNames,
              std::vector< DAVResource > & ioResources,
              const DAVRequestEnvironment & rEnv ) override;

    // propnames
    virtual void
    PROPFIND( const OUString & inPath,
              const Depth inDepth,
              std::vector< DAVResourceInfo >& ioResInfo,
              const DAVRequestEnvironment & rEnv ) override;

    virtual void
    PROPPATCH( const OUString & inPath,
               const std::vector< ProppatchValue > & inValues,
               const DAVRequestEnvironment & rEnv ) override;

    virtual void
    HEAD( const OUString &  inPath,
          const std::vector< OUString > & inHeaderNames,
          DAVResource & ioResource,
          const DAVRequestEnvironment & rEnv ) override;

    bool isHeadRequestInProgress();

    virtual css::uno::Reference< css::io::XInputStream >
    GET( const OUString & inPath,
         const DAVRequestEnvironment & rEnv ) override;

    virtual void
    GET( const OUString & inPath,
         css::uno::Reference< css::io::XOutputStream > &  ioOutputStream,
         const DAVRequestEnvironment & rEnv ) override;

    virtual css::uno::Reference< css::io::XInputStream >
    GET( const OUString & inPath,
         const std::vector< OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv ) override;

    virtual void
    GET( const OUString & inPath,
         css::uno::Reference< css::io::XOutputStream > & ioOutputStream,
         const std::vector< OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv ) override;

    virtual void
    PUT( const OUString & inPath,
         const css::uno::Reference< css::io::XInputStream > & inInputStream,
         const DAVRequestEnvironment & rEnv ) override;

    virtual css::uno::Reference< css::io::XInputStream >
    POST( const OUString & inPath,
          const OUString & rContentType,
          const OUString & rReferer,
          const css::uno::Reference< css::io::XInputStream > & inInputStream,
          const DAVRequestEnvironment & rEnv ) override;

    virtual void
    POST( const OUString & inPath,
          const OUString & rContentType,
          const OUString & rReferer,
          const css::uno::Reference< css::io::XInputStream > & inInputStream,
          css::uno::Reference< css::io::XOutputStream > & oOutputStream,
          const DAVRequestEnvironment & rEnv ) override;

    virtual void
    MKCOL( const OUString & inPath,
           const DAVRequestEnvironment & rEnv ) override;

    virtual void
    COPY( const OUString & inSourceURL,
          const OUString & inDestinationURL,
          const DAVRequestEnvironment & rEnv,
          bool inOverWrite = false ) override;

    virtual void
    MOVE( const OUString & inSourceURL,
          const OUString & inDestinationURL,
          const DAVRequestEnvironment & rEnv,
          bool inOverWrite = false ) override;

    virtual void DESTROY( const OUString & inPath,
                          const DAVRequestEnvironment & rEnv ) override;

    // set new lock.
    virtual void LOCK( const OUString & inURL,
                       css::ucb::Lock & inLock,
                       const DAVRequestEnvironment & rEnv ) override;

    // refresh existing lock.
    virtual sal_Int64 LOCK( const OUString & inURL,
                            sal_Int64 nTimeout,
                            const DAVRequestEnvironment & rEnv ) override;

    virtual void UNLOCK( const OUString & inURL,
                         const DAVRequestEnvironment & rEnv ) override;

    // helpers
    virtual void abort() override;

    const OUString & getHostName() const { return m_aUri.GetHost(); }
    int getPort() const { return m_aUri.GetPort(); }

    bool isDomainMatch( const OUString & certHostName );

private:
    friend class SerfLockStore;

    /// @throws DAVException
    void Init();

    /// @throws DAVException
    void Init( const DAVRequestEnvironment & rEnv );

    /// @throws DAVException
    void HandleError( std::shared_ptr<SerfRequestProcessor> rReqProc );

    ucbhelper::InternetProxyServer getProxySettings() const;

    static bool removeExpiredLocktoken( const OUString & inURL,
                                 const DAVRequestEnvironment & rEnv );

    // refresh lock, called by SerfLockStore::refreshLocks
    bool LOCK( const OUString& rLock, sal_Int32 *plastChanceToSendRefreshRequest );

    // unlock, called by SerfLockStore::~SerfLockStore
    void UNLOCK( const OUString& rLock );

    // Helper: XInputStream -> Sequence< sal_Int8 >
    static bool getDataFromInputStream(
        const css::uno::Reference<
            css::io::XInputStream > & xStream,
        css::uno::Sequence< sal_Int8 > & rData,
        bool bAppendTrailingZeroByte );
};

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
