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

namespace com { namespace sun { namespace star { namespace beans {
    struct NamedValue;
} } } }

namespace com { namespace sun { namespace star { namespace ucb {
    struct Lock;
} } } }

namespace webdav_ucp
{

class DAVSession
{
public:
    inline void acquire() SAL_THROW(())
    {
        osl_atomic_increment( &m_nRefCount );
    }

    void release() SAL_THROW(())
    {
        if ( osl_atomic_decrement( &m_nRefCount ) == 0 )
        {
            m_xFactory->releaseElement( this );
            delete this;
        }
    }

    virtual sal_Bool CanUse( const OUString & inPath,
                             const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rFlags ) = 0;

    virtual sal_Bool UsesProxy() = 0;

    // DAV methods
    //

    virtual void OPTIONS( const OUString & inPath,
                          DAVCapabilities & outCapabilities,
                          const DAVRequestEnvironment & rEnv )
        throw( DAVException ) = 0;

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
#if defined WNT && _MSC_VER < 1310
    friend struct std::auto_ptr< DAVSession >;
    // work around compiler bug...
#else // WNT
    friend class std::auto_ptr< DAVSession >;
#endif // WNT
};

} // namespace webdav_ucp

#endif // _DAVSESSION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
