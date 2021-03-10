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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/macros.hxx>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "pkgprovider.hxx"
#include "pkgcontent.hxx"
#include "pkguri.hxx"
#include <unordered_map>

using namespace com::sun::star;

namespace package_ucp
{



namespace {

class Package : public cppu::OWeakObject,
                public container::XHierarchicalNameAccess
{
    friend ContentProvider;

    OUString                                             m_aName;
    uno::Reference< container::XHierarchicalNameAccess > m_xNA;
    ContentProvider*                                     m_pOwner;

public:
    Package( const OUString& rName,
             const uno::Reference< container::XHierarchicalNameAccess > & xNA,
             ContentProvider* pOwner )
    : m_aName( rName ), m_xNA( xNA ), m_pOwner( pOwner ) {}
    virtual ~Package() override { m_pOwner->removePackage( m_aName ); }

    // XInterface
    virtual uno::Any SAL_CALL
    queryInterface( const uno::Type& aType ) override
    { return m_xNA->queryInterface( aType ); }
    virtual void SAL_CALL
    acquire() throw() override
    { OWeakObject::acquire(); }
    virtual void SAL_CALL
    release() throw() override
    { OWeakObject::release(); }

    // XHierarchicalNameAccess
    virtual uno::Any SAL_CALL
    getByHierarchicalName( const OUString& aName ) override
    { return m_xNA->getByHierarchicalName( aName ); }
    virtual sal_Bool SAL_CALL
    hasByHierarchicalName( const OUString& aName ) override
    { return m_xNA->hasByHierarchicalName( aName ); }
};

}

class Packages : public std::unordered_map<OUString, Package*> {};

}

using namespace package_ucp;


// ContentProvider Implementation.
ContentProvider::ContentProvider(
            const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext )
{
}


// virtual
ContentProvider::~ContentProvider()
{
}

// XInterface methods.
void SAL_CALL ContentProvider::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ContentProvider::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ContentProvider::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< lang::XTypeProvider* >(this),
                                               static_cast< lang::XServiceInfo* >(this),
                                               static_cast< ucb::XContentProvider* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider );


// XServiceInfo methods.

OUString
ContentProvider::getImplementationName()
{
    return "com.sun.star.comp.ucb.PackageContentProvider";
}

sal_Bool
ContentProvider::supportsService(const OUString& s)
{
    return cppu::supportsService(this, s);
}

css::uno::Sequence< OUString >
ContentProvider::getSupportedServiceNames()
{
    return { "com.sun.star.ucb.PackageContentProvider" };
}


// XContentProvider methods.


// virtual
uno::Reference< ucb::XContent > SAL_CALL ContentProvider::queryContent(
            const uno::Reference< ucb::XContentIdentifier >& Identifier )
{
    if ( !Identifier.is() )
        return uno::Reference< ucb::XContent >();

    PackageUri aUri( Identifier->getContentIdentifier() );
    if ( !aUri.isValid() )
        throw ucb::IllegalIdentifierException();

    // Create a new identifier for the normalized URL returned by
    // PackageUri::getUri().
    uno::Reference< ucb::XContentIdentifier > xId = new ::ucbhelper::ContentIdentifier( aUri.getUri() );

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xId );
    if ( xContent.is() )
        return xContent;

    // Create a new content.

    xContent = Content::create( m_xContext, this, Identifier ); // not xId!!!
    registerNewContent( xContent );

    if ( xContent.is() && !xContent->getIdentifier().is() )
        throw ucb::IllegalIdentifierException();

    return xContent;
}


// Other methods.


uno::Reference< container::XHierarchicalNameAccess >
ContentProvider::createPackage( const PackageUri & rURI )
{
    osl::MutexGuard aGuard( m_aMutex );

    OUString rURL = rURI.getPackage() + rURI.getParam();

    if ( m_pPackages )
    {
        Packages::const_iterator it = m_pPackages->find( rURL );
        if ( it != m_pPackages->end() )
        {
            // Already instantiated. Return package.
            return (*it).second->m_xNA;
        }
    }
    else
        m_pPackages.reset( new Packages );

    // Create new package...
    uno::Sequence< uno::Any > aArguments( 1 );
    aArguments[ 0 ] <<= rURL;
    uno::Reference< container::XHierarchicalNameAccess > xNameAccess;
    try
    {
        xNameAccess.set(
            m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                "com.sun.star.packages.comp.ZipPackage",
                aArguments, m_xContext ),
            css::uno::UNO_QUERY_THROW );
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            e.Message, e.Context, anyEx);
    }

    rtl::Reference< Package> xPackage = new Package( rURL, xNameAccess, this );
    (*m_pPackages)[ rURL ] = xPackage.get();
    return xPackage;
}


void ContentProvider::removePackage( const OUString & rName )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pPackages )
    {
        Packages::iterator it = m_pPackages->find( rName );
        if ( it != m_pPackages->end() )
        {
            m_pPackages->erase( it );
            return;
        }
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_package_ContentProvider_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ContentProvider(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
