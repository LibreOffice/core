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



#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVSESSION_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVSESSION_HXX

#include <memory>
#include <rtl/ustring.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include "DAVException.hxx"
#include "DAVProperties.hxx"
#include "DAVResource.hxx"
#include "DAVSessionFactory.hxx"
#include "DAVTypes.hxx"
#include "DAVRequestEnvironment.hxx"

namespace com { namespace sun { namespace star { namespace ucb {
    struct Lock;
} } } }

namespace http_dav_ucp
{

class DAVAuthListener;

class DAVSession
{
public:
    inline void acquire() SAL_THROW(())
    {
        osl_incrementInterlockedCount( &m_nRefCount );
    }

    void release() SAL_THROW(())
    {
        if ( osl_decrementInterlockedCount( &m_nRefCount ) == 0 )
        {
            m_xFactory->releaseElement( this );
            delete this;
        }
    }

    virtual sal_Bool CanUse( const OUString & inPath ) = 0;

    virtual sal_Bool UsesProxy() = 0;

    // DAV methods


    // NOT USED
    /*
    virtual void OPTIONS( const OUString & inPath,
                          DAVCapabilities & outCapabilities,
                          const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;
    */

    // allprop & named
    virtual void PROPFIND( const OUString & inPath,
                           const Depth inDepth,
                           const std::vector< OUString > & inPropertyNames,
                           std::vector< DAVResource > & ioResources,
                           const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    // propnames
    virtual void PROPFIND( const OUString & inPath,
                           const Depth inDepth,
                           std::vector< DAVResourceInfo > & ioResInfo,
                           const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void PROPPATCH( const OUString & inPath,
                            const std::vector< ProppatchValue > & inValues,
                            const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void HEAD( const OUString &  inPath,
                       const std::vector< OUString > & inHeaderNames,
                       DAVResource & ioResource,
                       const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const OUString & inPath,
         const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void GET( const OUString & inPath,
                      com::sun::star::uno::Reference<
                          com::sun::star::io::XOutputStream >& o,
                      const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const OUString & inPath,
         const std::vector< OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void
    GET( const OUString & inPath,
         com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& o,
         const std::vector< OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void PUT( const OUString & inPath,
                      const com::sun::star::uno::Reference<
                          com::sun::star::io::XInputStream >& s,
                      const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    POST( const OUString & inPath,
          const OUString & rContentType,
          const OUString & rReferer,
          const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & inInputStream,
          const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    virtual void POST( const OUString & inPath,
                       const OUString & rContentType,
                       const OUString & rReferer,
                       const com::sun::star::uno::Reference<
                           com::sun::star::io::XInputStream > & inInputStream,
                       com::sun::star::uno::Reference<
                           com::sun::star::io::XOutputStream > & oOutputStream,
                       const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    virtual void MKCOL( const OUString & inPath,
                        const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void COPY( const OUString & inSource,
                       const OUString & inDestination,
                       const DAVRequestEnvironment & rEnv,
                       sal_Bool inOverwrite = false )
        throw( DAVException ) = 0;

    virtual void MOVE( const OUString & inSource,
                       const OUString & inDestination,
                       const DAVRequestEnvironment & rEnv,
                       sal_Bool inOverwrite = false )
        throw( DAVException ) = 0;

    virtual void DESTROY( const OUString & inPath,
                          const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    // set new lock.
    virtual void LOCK( const OUString & inPath,
                       com::sun::star::ucb::Lock & inLock,
                       const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    // refresh existing lock.
    virtual sal_Int64 LOCK( const OUString & inPath,
                            sal_Int64 nTimeout,
                            const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    virtual void UNLOCK( const OUString & inPath,
                         const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    virtual void abort()
        throw( DAVException ) = 0;

protected:
    rtl::Reference< DAVSessionFactory > m_xFactory;

    DAVSession( rtl::Reference< DAVSessionFactory > const & rFactory )
    : m_xFactory( rFactory ), m_nRefCount( 0 ) {}

    virtual ~DAVSession() {}

private:
    DAVSessionFactory::Map::iterator m_aContainerIt;
    oslInterlockedCount m_nRefCount;

    friend class DAVSessionFactory;
    friend class std::auto_ptr< DAVSession >;
};

} // namespace http_dav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVSESSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
