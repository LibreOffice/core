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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <boost/unordered_map.hpp>
#include <osl/diagnose.h>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include "pkgprovider.hxx"
#include "pkgcontent.hxx"
#include "pkguri.hxx"

using namespace com::sun::star;

namespace package_ucp
{

//=========================================================================
//
// class Package.
//
//=========================================================================

class Package : public cppu::OWeakObject,
                public container::XHierarchicalNameAccess
{
    friend class ContentProvider;

    rtl::OUString                                        m_aName;
    uno::Reference< container::XHierarchicalNameAccess > m_xNA;
    ContentProvider*                                     m_pOwner;

public:
    Package( const rtl::OUString& rName,
             const uno::Reference< container::XHierarchicalNameAccess > & xNA,
             ContentProvider* pOwner )
    : m_aName( rName ), m_xNA( xNA ), m_pOwner( pOwner ) {}
    virtual ~Package() { m_pOwner->removePackage( m_aName ); }

    // XInterface
    virtual uno::Any SAL_CALL
    queryInterface( const uno::Type& aType )
        throw( uno::RuntimeException )
    { return m_xNA->queryInterface( aType ); }
    virtual void SAL_CALL
    acquire() throw()
    { OWeakObject::acquire(); }
    virtual void SAL_CALL
    release() throw()
    { OWeakObject::release(); }

    // XHierarchicalNameAccess
    virtual uno::Any SAL_CALL
    getByHierarchicalName( const rtl::OUString& aName )
        throw( container::NoSuchElementException, uno::RuntimeException )
    { return m_xNA->getByHierarchicalName( aName ); }
    virtual sal_Bool SAL_CALL
    hasByHierarchicalName( const rtl::OUString& aName )
        throw( uno::RuntimeException )
    { return m_xNA->hasByHierarchicalName( aName ); }
};

//=========================================================================
//
// Packages.
//
//=========================================================================

struct equalString
{
    bool operator()(
        const rtl::OUString& rKey1, const rtl::OUString& rKey2 ) const
    {
        return !!( rKey1 == rKey2 );
    }
};

struct hashString
{
    size_t operator()( const rtl::OUString & rName ) const
    {
        return rName.hashCode();
    }
};

typedef boost::unordered_map
<
    rtl::OUString,
    Package*,
    hashString,
    equalString
>
PackageMap;

class Packages : public PackageMap {};

}

using namespace package_ucp;

//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================

ContentProvider::ContentProvider(
            const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
: ::ucbhelper::ContentProviderImplHelper( rSMgr ),
  m_pPackages( 0 )
{
}

//=========================================================================
// virtual
ContentProvider::~ContentProvider()
{
    delete m_pPackages;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   ucb::XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( ContentProvider,
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.comp.ucb.PackageContentProvider" )),
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        PACKAGE_CONTENT_PROVIDER_SERVICE_NAME )) );

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
uno::Reference< ucb::XContent > SAL_CALL ContentProvider::queryContent(
            const uno::Reference< ucb::XContentIdentifier >& Identifier )
    throw( ucb::IllegalIdentifierException, uno::RuntimeException )
{
    if ( !Identifier.is() )
        return uno::Reference< ucb::XContent >();

    PackageUri aUri( Identifier->getContentIdentifier() );
    if ( !aUri.isValid() )
        throw ucb::IllegalIdentifierException();

    // Create a new identifier for the mormalized URL returned by
    // PackageUri::getUri().
    uno::Reference< ucb::XContentIdentifier > xId
                = new ::ucbhelper::ContentIdentifier( m_xSMgr, aUri.getUri() );

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xId ).get();
    if ( xContent.is() )
        return xContent;

    // Create a new content.

    xContent = Content::create( m_xSMgr, this, Identifier ); // not xId!!!
    registerNewContent( xContent );

    if ( xContent.is() && !xContent->getIdentifier().is() )
        throw ucb::IllegalIdentifierException();

    return xContent;
}

//=========================================================================
//
// Other methods.
//
//=========================================================================

uno::Reference< container::XHierarchicalNameAccess >
ContentProvider::createPackage( const rtl::OUString & rName, const rtl::OUString & rParam )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !rName.getLength() )
    {
        OSL_ENSURE( sal_False,
                    "ContentProvider::createPackage - Invalid URL!" );
        return uno::Reference< container::XHierarchicalNameAccess >();
    }

    rtl::OUString rURL = rName + rParam;

    if ( m_pPackages )
    {
        Packages::const_iterator it = m_pPackages->find( rURL );
        if ( it != m_pPackages->end() )
        {
            // Already instanciated. Return package.
            return (*it).second->m_xNA;
        }
    }
    else
        m_pPackages = new Packages;

    // Create new package...
    try
    {
        uno::Sequence< uno::Any > aArguments( 1 );
        aArguments[ 0 ] <<= rURL;

        uno::Reference< uno::XInterface > xIfc
            = m_xSMgr->createInstanceWithArguments(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.packages.comp.ZipPackage" )),
                aArguments );

        if ( xIfc.is() )
        {
            uno::Reference<
                container::XHierarchicalNameAccess > xNameAccess(
                                                        xIfc, uno::UNO_QUERY );

            OSL_ENSURE( xNameAccess.is(),
                        "ContentProvider::createPackage - "
                        "Got no hierarchical name access!" );

            rtl::Reference< Package> xPackage
                = new Package( rURL, xNameAccess, this );

            (*m_pPackages)[ rURL ] = xPackage.get();

            return xPackage.get();
        }
    }
    catch ( uno::RuntimeException const & )
    {
        // createInstanceWithArguemts
    }
    catch ( uno::Exception const & )
    {
        // createInstanceWithArguemts
    }

    return uno::Reference< container::XHierarchicalNameAccess >();
}

//=========================================================================
sal_Bool ContentProvider::removePackage( const rtl::OUString & rName )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pPackages )
    {
        Packages::iterator it = m_pPackages->find( rName );
        if ( it != m_pPackages->end() )
        {
            m_pPackages->erase( it );
            return sal_True;
        }
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
