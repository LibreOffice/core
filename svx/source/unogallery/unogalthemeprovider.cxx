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


#include "unogalthemeprovider.hxx"
#include "unogaltheme.hxx"
#include "svx/gallery1.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <unotools/pathoptions.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/gallery/XGalleryTheme.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace ::com::sun::star;

namespace unogallery {

// --------------------
// - Helper functions -
// --------------------

uno::Reference< uno::XInterface > SAL_CALL GalleryThemeProvider_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & )
    throw( uno::Exception )
{
    return *( new GalleryThemeProvider() );
}

// -----------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL GalleryThemeProvider_getSupportedServiceNames()
    throw()
{
    return GalleryThemeProvider::getSupportedServiceNames_Static();
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL GalleryThemeProvider_getImplementationName()
    throw()
{
    return GalleryThemeProvider::getImplementationName_Static();
}

// -----------------
// - GalleryThemeProvider -
// -----------------

GalleryThemeProvider::GalleryThemeProvider() :
    mbHiddenThemes( sal_False )
{
    mpGallery = ::Gallery::GetGalleryInstance();
}

// ------------------------------------------------------------------------------

GalleryThemeProvider::~GalleryThemeProvider()
{
}

// ------------------------------------------------------------------------------

SVX_DLLPUBLIC ::rtl::OUString GalleryThemeProvider::getImplementationName_Static()
    throw()
{
    return ::rtl::OUString( "com.sun.star.comp.gallery.GalleryThemeProvider" );
}

// ------------------------------------------------------------------------------

SVX_DLLPUBLIC uno::Sequence< ::rtl::OUString > GalleryThemeProvider::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence< ::rtl::OUString > aSeq( 1 );

    aSeq.getArray()[ 0 ] = ::rtl::OUString( "com.sun.star.gallery.GalleryThemeProvider" );

    return aSeq;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL GalleryThemeProvider::getImplementationName()
    throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GalleryThemeProvider::supportsService( const ::rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString >    aSNL( getSupportedServiceNames() );
    const ::rtl::OUString*              pArray = aSNL.getConstArray();

    for( int i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL GalleryThemeProvider::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

// ------------------------------------------------------------------------------

uno::Sequence< uno::Type > SAL_CALL GalleryThemeProvider::getTypes()
    throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type >  aTypes( 6 );
    uno::Type*                  pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XInitialization>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< container::XElementAccess>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< container::XNameAccess>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< gallery::XGalleryThemeProvider>*)0);

    return aTypes;
}

namespace
{
    class theGalleryThemeProviderImplementationId : public rtl::Static< UnoTunnelIdInit, theGalleryThemeProviderImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL GalleryThemeProvider::getImplementationId()
    throw(uno::RuntimeException)
{
    return theGalleryThemeProviderImplementationId::get().getSeq();
}

// ------------------------------------------------------------------------------

void SAL_CALL GalleryThemeProvider::initialize( const uno::Sequence< uno::Any >& rArguments )
    throw ( uno::Exception, uno::RuntimeException )
{
    uno::Sequence< beans::PropertyValue >   aParams;
    sal_Int32                               i;

    for( i = 0; i < rArguments.getLength(); ++i )
    {
        if( rArguments[ i ] >>= aParams )
            break;
    }

    for( i = 0; i < aParams.getLength(); ++i )
    {
        const beans::PropertyValue& rProp = aParams[ i ];

        if ( rProp.Name == "ProvideHiddenThemes" )
            rProp.Value >>= mbHiddenThemes;
    }
}

// ------------------------------------------------------------------------------

uno::Type SAL_CALL GalleryThemeProvider::getElementType()
    throw (uno::RuntimeException)
{
    return ::getCppuType( (const uno::Reference< gallery::XGalleryTheme >*) 0);
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GalleryThemeProvider::hasElements()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aGuard;

    return( ( mpGallery != NULL ) && ( mpGallery->GetThemeCount() > 0 ) );
}

// ------------------------------------------------------------------------------

uno::Any SAL_CALL GalleryThemeProvider::getByName( const ::rtl::OUString& rName )
    throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    const SolarMutexGuard aGuard;
    uno::Any            aRet;

    if( !mpGallery || !mpGallery->HasTheme( rName ) )
    {
        throw container::NoSuchElementException();
    }
    else
    {
        aRet = uno::makeAny( uno::Reference< gallery::XGalleryTheme >( new ::unogallery::GalleryTheme( rName ) ) );
    }

    return aRet;
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL GalleryThemeProvider::getElementNames()
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aGuard;
    sal_uInt32                          i = 0, nCount = ( mpGallery ? mpGallery->GetThemeCount() : 0 ), nRealCount = 0;
    uno::Sequence< ::rtl::OUString >    aSeq( nCount );

    for( ; i < nCount; ++i )
    {
        const GalleryThemeEntry* pEntry = mpGallery->GetThemeInfo( i );

        if( mbHiddenThemes || !pEntry->IsHidden() )
            aSeq[ nRealCount++ ] = pEntry->GetThemeName();
    }

    aSeq.realloc( nRealCount );

    return aSeq;
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GalleryThemeProvider::hasByName( const ::rtl::OUString& rName )
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aGuard;

    sal_Bool bRet = sal_False;

    if( mpGallery && mpGallery->HasTheme( rName ) )
        bRet = ( mbHiddenThemes || !mpGallery->GetThemeInfo( rName )->IsHidden() );

    return( bRet );
}

// ------------------------------------------------------------------------------

uno::Reference< gallery::XGalleryTheme > SAL_CALL GalleryThemeProvider::insertNewByName( const ::rtl::OUString& rThemeName )
    throw (container::ElementExistException, uno::RuntimeException)
{
    const SolarMutexGuard aGuard;
    uno::Reference< gallery::XGalleryTheme >    xRet;

    if( mpGallery )
    {
        if( mpGallery->HasTheme( rThemeName ) )
        {
            throw container::ElementExistException();
        }
        else if( mpGallery->CreateTheme( rThemeName ) )
        {
            xRet = new ::unogallery::GalleryTheme( rThemeName );
        }
    }

    return xRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL GalleryThemeProvider::removeByName( const ::rtl::OUString& rName )
    throw (container::NoSuchElementException, uno::RuntimeException)
{
    const SolarMutexGuard aGuard;

    if( !mpGallery ||
        !mpGallery->HasTheme( rName ) ||
        ( !mbHiddenThemes && mpGallery->GetThemeInfo( rName )->IsHidden() ) )
    {
        throw container::NoSuchElementException();
    }
    else
    {
        mpGallery->RemoveTheme( rName );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
