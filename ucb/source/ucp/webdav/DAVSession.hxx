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



#ifndef _DAVSESSION_HXX_
#define _DAVSESSION_HXX_

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

    virtual sal_Bool CanUse( const ::rtl::OUString & inPath ) = 0;

    virtual sal_Bool UsesProxy() = 0;

    // DAV methods
    //

    // NOT USED
    /*
    virtual void OPTIONS( const ::rtl::OUString & inPath,
                          DAVCapabilities & outCapabilities,
                          const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;
    */

    // allprop & named
    virtual void PROPFIND( const ::rtl::OUString & inPath,
                           const Depth inDepth,
                           const std::vector< ::rtl::OUString > & inPropertyNames,
                           std::vector< DAVResource > & ioResources,
                           const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    // propnames
    virtual void PROPFIND( const ::rtl::OUString & inPath,
                           const Depth inDepth,
                           std::vector< DAVResourceInfo > & ioResInfo,
                           const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void PROPPATCH( const ::rtl::OUString & inPath,
                            const std::vector< ProppatchValue > & inValues,
                            const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void HEAD( const ::rtl::OUString &  inPath,
                       const std::vector< ::rtl::OUString > & inHeaderNames,
                       DAVResource & ioResource,
                       const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const ::rtl::OUString & inPath,
         const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void GET( const ::rtl::OUString & inPath,
                      com::sun::star::uno::Reference<
                          com::sun::star::io::XOutputStream >& o,
                      const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    GET( const ::rtl::OUString & inPath,
         const std::vector< ::rtl::OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void
    GET( const ::rtl::OUString & inPath,
         com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& o,
         const std::vector< ::rtl::OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void PUT( const ::rtl::OUString & inPath,
                      const com::sun::star::uno::Reference<
                          com::sun::star::io::XInputStream >& s,
                      const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    POST( const rtl::OUString & inPath,
          const rtl::OUString & rContentType,
          const rtl::OUString & rReferer,
          const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & inInputStream,
          const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    virtual void POST( const rtl::OUString & inPath,
                       const rtl::OUString & rContentType,
                       const rtl::OUString & rReferer,
                       const com::sun::star::uno::Reference<
                           com::sun::star::io::XInputStream > & inInputStream,
                       com::sun::star::uno::Reference<
                           com::sun::star::io::XOutputStream > & oOutputStream,
                       const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    virtual void MKCOL( const ::rtl::OUString & inPath,
                        const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    virtual void COPY( const ::rtl::OUString & inSource,
                       const ::rtl::OUString & inDestination,
                       const DAVRequestEnvironment & rEnv,
                       sal_Bool inOverwrite = false )
        throw( DAVException ) = 0;

    virtual void MOVE( const ::rtl::OUString & inSource,
                       const ::rtl::OUString & inDestination,
                       const DAVRequestEnvironment & rEnv,
                       sal_Bool inOverwrite = false )
        throw( DAVException ) = 0;

    virtual void DESTROY( const ::rtl::OUString & inPath,
                          const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

    // set new lock.
    virtual void LOCK( const ::rtl::OUString & inPath,
                       com::sun::star::ucb::Lock & inLock,
                       const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    // refresh existing lock.
    virtual sal_Int64 LOCK( const ::rtl::OUString & inPath,
                            sal_Int64 nTimeout,
                            const DAVRequestEnvironment & rEnv )
        throw ( DAVException ) = 0;

    virtual void UNLOCK( const ::rtl::OUString & inPath,
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
#if defined WNT && _MSC_VER < 1310
    friend struct std::auto_ptr< DAVSession >;
    // work around compiler bug...
#else // WNT
    friend class std::auto_ptr< DAVSession >;
#endif // WNT
};

} // namespace http_dav_ucp

#endif // _DAVSESSION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
