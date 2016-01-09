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

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVSESSION_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVSESSION_HXX

#include <config_lgpl.h>
#include <memory>
#include <rtl/ustring.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
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
    inline void acquire()
    {
        osl_atomic_increment( &m_nRefCount );
    }

    void release()
    {
        if ( osl_atomic_decrement( &m_nRefCount ) == 0 )
        {
            m_xFactory->releaseElement( this );
            delete this;
        }
    }

    virtual bool CanUse( const OUString & inPath,
                             const css::uno::Sequence< css::beans::NamedValue >& rFlags ) = 0;

    virtual bool UsesProxy() = 0;

    // DAV methods

    virtual void OPTIONS( const OUString & inPath,
                          DAVOptions& rOptions,
                          const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    // allprop & named
    virtual void PROPFIND( const OUString & inPath,
                           const Depth inDepth,
                           const std::vector< OUString > & inPropertyNames,
                           std::vector< DAVResource > & ioResources,
                           const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    // propnames
    virtual void PROPFIND( const OUString & inPath,
                           const Depth inDepth,
                           std::vector< DAVResourceInfo > & ioResInfo,
                           const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    virtual void PROPPATCH( const OUString & inPath,
                            const std::vector< ProppatchValue > & inValues,
                            const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    virtual void HEAD( const OUString &  inPath,
                       const std::vector< OUString > & inHeaderNames,
                       DAVResource & ioResource,
                       const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    virtual css::uno::Reference< css::io::XInputStream >
    GET( const OUString & inPath,
         const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    virtual void GET( const OUString & inPath,
                      css::uno::Reference< css::io::XOutputStream >& o,
                      const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    virtual css::uno::Reference< css::io::XInputStream >
    GET( const OUString & inPath,
         const std::vector< OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    virtual void
    GET( const OUString & inPath,
         css::uno::Reference< css::io::XOutputStream >& o,
         const std::vector< OUString > & inHeaderNames,
         DAVResource & ioResource,
         const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    virtual void PUT( const OUString & inPath,
                      const css::uno::Reference< css::io::XInputStream >& s,
                      const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    virtual css::uno::Reference< css::io::XInputStream >
    POST( const OUString & inPath,
          const OUString & rContentType,
          const OUString & rReferer,
          const css::uno::Reference< css::io::XInputStream > & inInputStream,
          const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) = 0;

    virtual void POST( const OUString & inPath,
                       const OUString & rContentType,
                       const OUString & rReferer,
                       const css::uno::Reference< css::io::XInputStream > & inInputStream,
                       css::uno::Reference< css::io::XOutputStream > & oOutputStream,
                       const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) = 0;

    virtual void MKCOL( const OUString & inPath,
                        const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    virtual void COPY( const OUString & inSource,
                       const OUString & inDestination,
                       const DAVRequestEnvironment & rEnv,
                       bool inOverwrite )
        throw( std::exception ) = 0;

    virtual void MOVE( const OUString & inSource,
                       const OUString & inDestination,
                       const DAVRequestEnvironment & rEnv,
                       bool inOverwrite )
        throw( std::exception ) = 0;

    virtual void DESTROY( const OUString & inPath,
                          const DAVRequestEnvironment & rEnv )
        throw( std::exception ) = 0;

    // set new lock.
    virtual void LOCK( const OUString & inPath,
                       css::ucb::Lock & inLock,
                       const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) = 0;

    virtual void UNLOCK( const OUString & inPath,
                         const DAVRequestEnvironment & rEnv )
        throw ( std::exception ) = 0;

    virtual void abort()
        throw( std::exception ) = 0;

protected:
    rtl::Reference< DAVSessionFactory > m_xFactory;

    explicit DAVSession( rtl::Reference< DAVSessionFactory > const & rFactory )
    : m_xFactory( rFactory ), m_nRefCount( 0 ) {}

    virtual ~DAVSession() {}

private:
    DAVSessionFactory::Map::iterator m_aContainerIt;
    oslInterlockedCount m_nRefCount;

    friend class DAVSessionFactory;
    friend struct std::default_delete< DAVSession >;
};

} // namespace webdav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVSESSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
