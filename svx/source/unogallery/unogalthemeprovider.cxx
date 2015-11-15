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
#include <rtl/ref.hxx>
#include <vcl/svapp.hxx>
#include <unotools/pathoptions.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/gallery/XGalleryTheme.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;

namespace {

GalleryThemeProvider::GalleryThemeProvider() :
    mbHiddenThemes( false )
{
    mpGallery = ::Gallery::GetGalleryInstance();
}

GalleryThemeProvider::~GalleryThemeProvider()
{
}

OUString SAL_CALL GalleryThemeProvider::getImplementationName()
    throw( uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.gallery.GalleryThemeProvider" );
}

sal_Bool SAL_CALL GalleryThemeProvider::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL GalleryThemeProvider::getSupportedServiceNames()
    throw( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aSeq { "com.sun.star.gallery.GalleryThemeProvider" };
    return aSeq;
}

uno::Sequence< uno::Type > SAL_CALL GalleryThemeProvider::getTypes()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type >  aTypes( 6 );
    uno::Type*                  pTypes = aTypes.getArray();

    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
    *pTypes++ = cppu::UnoType<lang::XInitialization>::get();
    *pTypes++ = cppu::UnoType<container::XElementAccess>::get();
    *pTypes++ = cppu::UnoType<container::XNameAccess>::get();
    *pTypes++ = cppu::UnoType<gallery::XGalleryThemeProvider>::get();

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL GalleryThemeProvider::getImplementationId()
    throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL GalleryThemeProvider::initialize( const uno::Sequence< uno::Any >& rArguments )
    throw ( uno::Exception, uno::RuntimeException, std::exception )
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



uno::Type SAL_CALL GalleryThemeProvider::getElementType()
    throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<gallery::XGalleryTheme>::get();
}



sal_Bool SAL_CALL GalleryThemeProvider::hasElements()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aGuard;

    return( ( mpGallery != nullptr ) && ( mpGallery->GetThemeCount() > 0 ) );
}



uno::Any SAL_CALL GalleryThemeProvider::getByName( const OUString& rName )
    throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
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



uno::Sequence< OUString > SAL_CALL GalleryThemeProvider::getElementNames()
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aGuard;
    sal_uInt32                          i = 0, nCount = ( mpGallery ? mpGallery->GetThemeCount() : 0 ), nRealCount = 0;
    uno::Sequence< OUString >    aSeq( nCount );

    for( ; i < nCount; ++i )
    {
        const GalleryThemeEntry* pEntry = mpGallery->GetThemeInfo( i );

        if( mbHiddenThemes || !pEntry->IsHidden() )
            aSeq[ nRealCount++ ] = pEntry->GetThemeName();
    }

    aSeq.realloc( nRealCount );

    return aSeq;
}



sal_Bool SAL_CALL GalleryThemeProvider::hasByName( const OUString& rName )
    throw (uno::RuntimeException, std::exception)
{
    const SolarMutexGuard aGuard;

    bool bRet = false;

    if( mpGallery && mpGallery->HasTheme( rName ) )
        bRet = ( mbHiddenThemes || !mpGallery->GetThemeInfo( rName )->IsHidden() );

    return bRet;
}



uno::Reference< gallery::XGalleryTheme > SAL_CALL GalleryThemeProvider::insertNewByName( const OUString& rThemeName )
    throw (container::ElementExistException, uno::RuntimeException, std::exception)
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



void SAL_CALL GalleryThemeProvider::removeByName( const OUString& rName )
    throw (container::NoSuchElementException, uno::RuntimeException, std::exception)
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_gallery_GalleryThemeProvider_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new GalleryThemeProvider);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
