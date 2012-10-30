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

#ifndef _DAVRESOURCEACCESS_HXX_
#define _DAVRESOURCEACCESS_HXX_

#include <vector>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include "DAVAuthListener.hxx"
#include "DAVException.hxx"
#include "DAVSession.hxx"
#include "DAVResource.hxx"
#include "DAVTypes.hxx"
#include "NeonUri.hxx"

namespace webdav_ucp
{

class DAVSessionFactory;

class DAVResourceAccess
{
    osl::Mutex    m_aMutex;
    rtl::OUString m_aURL;
    rtl::OUString m_aPath;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > m_aFlags;
    rtl::Reference< DAVSession > m_xSession;
    rtl::Reference< DAVSessionFactory > m_xSessionFactory;
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext > m_xContext;
    std::vector< NeonUri > m_aRedirectURIs;

public:
    DAVResourceAccess() : m_xSessionFactory( 0 ) {}
    DAVResourceAccess( const com::sun::star::uno::Reference<
                           com::sun::star::uno::XComponentContext > & rxContext,
                       rtl::Reference<
                       DAVSessionFactory > const & rSessionFactory,
                       const rtl::OUString & rURL );
    DAVResourceAccess( const DAVResourceAccess & rOther );

    DAVResourceAccess & operator=( const DAVResourceAccess & rOther );

    void setFlags( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rFlags )
        throw ( DAVException );

    void setURL( const rtl::OUString & rNewURL )
        throw ( DAVException );

    void resetUri();

    const rtl::OUString & getURL() const { return m_aURL; }

    rtl::Reference< DAVSessionFactory > getSessionFactory() const
    { return m_xSessionFactory; }

    // DAV methods
    //

#if 0 // currently not used, but please don't remove code
    void
    OPTIONS(  DAVCapabilities & rCapabilities,
              const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );
#endif

    // allprop & named
    void
    PROPFIND( const Depth nDepth,
              const std::vector< rtl::OUString > & rPropertyNames,
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
    HEAD( const std::vector< rtl::OUString > & rHeaderNames, // empty == 'all'
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
    GET( const std::vector< rtl::OUString > & rHeaderNames, // empty == 'all'
         DAVResource & rResource,
         const com::sun::star::uno::Reference<
             com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    GET( com::sun::star::uno::Reference<
             com::sun::star::io::XOutputStream > & rStream,
         const std::vector< rtl::OUString > & rHeaderNames, // empty == 'all'
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
    POST( const rtl::OUString & rContentType,
          const rtl::OUString & rReferer,
          const com::sun::star::uno::Reference<
              com::sun::star::io::XInputStream > & rInputStream,
          const com::sun::star::uno::Reference<
          com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( DAVException );

    void
    POST( const rtl::OUString & rContentType,
          const rtl::OUString & rReferer,
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
    COPY( const ::rtl::OUString & rSourcePath,
          const ::rtl::OUString & rDestinationURI,
          sal_Bool bOverwrite,
          const com::sun::star::uno::Reference<
              com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( DAVException );

    void
    MOVE( const ::rtl::OUString & rSourcePath,
          const ::rtl::OUString & rDestinationURI,
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
        const rtl::OUString & rMethod,
        DAVRequestHeaders & rRequestHeaders );

private:
    const rtl::OUString & getRequestURI() const;
    sal_Bool detectRedirectCycle( const rtl::OUString& rRedirectURL )
        throw ( DAVException );
    sal_Bool handleException( const DAVException & e, int errorCount )
        throw ( DAVException );
    void initialize()
        throw ( DAVException );
};

} // namespace webdav_ucp

#endif // _DAVRESOURCEACCESS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
