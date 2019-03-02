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

#include <sal/config.h>

#include "unogaltheme.hxx"
#include <svx/gallery1.hxx>
#include <rtl/ref.hxx>
#include <vcl/svapp.hxx>
#include <unotools/pathoptions.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/gallery/XGalleryTheme.hpp>
#include <com/sun/star/gallery/XGalleryThemeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;

namespace {

class GalleryThemeProvider : public ::cppu::WeakImplHelper< css::lang::XInitialization,
                                                             css::gallery::XGalleryThemeProvider,
                                                             css::lang::XServiceInfo >
{
public:

    GalleryThemeProvider();

protected:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XGalleryThemeProvider
    virtual css::uno::Reference< css::gallery::XGalleryTheme > SAL_CALL insertNewByName( const OUString& ThemeName ) override;
    virtual void SAL_CALL removeByName( const OUString& ThemeName ) override;

private:

    Gallery*    mpGallery;
    bool    mbHiddenThemes;
};

GalleryThemeProvider::GalleryThemeProvider() :
    mbHiddenThemes( false )
{
    mpGallery = ::Gallery::GetGalleryInstance();
}

OUString SAL_CALL GalleryThemeProvider::getImplementationName()
{
    return OUString( "com.sun.star.comp.gallery.GalleryThemeProvider" );
}

sal_Bool SAL_CALL GalleryThemeProvider::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL GalleryThemeProvider::getSupportedServiceNames()
{
    uno::Sequence<OUString> aSeq { "com.sun.star.gallery.GalleryThemeProvider" };
    return aSeq;
}

uno::Sequence< uno::Type > SAL_CALL GalleryThemeProvider::getTypes()
{
    static const uno::Sequence aTypes {
        cppu::UnoType<lang::XServiceInfo>::get(),
        cppu::UnoType<lang::XTypeProvider>::get(),
        cppu::UnoType<lang::XInitialization>::get(),
        cppu::UnoType<container::XElementAccess>::get(),
        cppu::UnoType<container::XNameAccess>::get(),
        cppu::UnoType<gallery::XGalleryThemeProvider>::get() };

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL GalleryThemeProvider::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL GalleryThemeProvider::initialize( const uno::Sequence< uno::Any >& rArguments )
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
{
    return cppu::UnoType<gallery::XGalleryTheme>::get();
}


sal_Bool SAL_CALL GalleryThemeProvider::hasElements()
{
    const SolarMutexGuard aGuard;

    return( ( mpGallery != nullptr ) && ( mpGallery->GetThemeCount() > 0 ) );
}


uno::Any SAL_CALL GalleryThemeProvider::getByName( const OUString& rName )
{
    const SolarMutexGuard aGuard;
    uno::Any            aRet;

    if( !mpGallery || !mpGallery->HasTheme( rName ) )
    {
        throw container::NoSuchElementException();
    }

    aRet <<= uno::Reference< gallery::XGalleryTheme >( new ::unogallery::GalleryTheme( rName ) );

    return aRet;
}


uno::Sequence< OUString > SAL_CALL GalleryThemeProvider::getElementNames()
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
{
    const SolarMutexGuard aGuard;

    bool bRet = false;

    if( mpGallery && mpGallery->HasTheme( rName ) )
        bRet = ( mbHiddenThemes || !mpGallery->GetThemeInfo( rName )->IsHidden() );

    return bRet;
}


uno::Reference< gallery::XGalleryTheme > SAL_CALL GalleryThemeProvider::insertNewByName( const OUString& rThemeName )
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
{
    const SolarMutexGuard aGuard;

    if( !mpGallery ||
        !mpGallery->HasTheme( rName ) ||
        ( !mbHiddenThemes && mpGallery->GetThemeInfo( rName )->IsHidden() ) )
    {
        throw container::NoSuchElementException();
    }

    mpGallery->RemoveTheme( rName );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_gallery_GalleryThemeProvider_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new GalleryThemeProvider);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
