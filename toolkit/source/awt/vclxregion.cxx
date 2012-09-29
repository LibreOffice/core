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

#include <toolkit/awt/vclxregion.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>
#include <vcl/svapp.hxx>

//  ----------------------------------------------------
//  class VCLXRegion
//  ----------------------------------------------------
VCLXRegion::VCLXRegion()
{
}

VCLXRegion::~VCLXRegion()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXRegion::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XRegion* >(this)),
                                        (static_cast< ::com::sun::star::lang::XUnoTunnel* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXRegion )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXRegion )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion>* ) NULL )
IMPL_XTYPEPROVIDER_END



::com::sun::star::awt::Rectangle VCLXRegion::getBounds() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return AWTRectangle( maRegion.GetBoundRect() );
}

void VCLXRegion::clear() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.SetEmpty();
}

void VCLXRegion::move( sal_Int32 nHorzMove, sal_Int32 nVertMove ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Move( nHorzMove, nVertMove );
}

void VCLXRegion::unionRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Union( VCLRectangle( rRect ) );
}

void VCLXRegion::intersectRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Intersect( VCLRectangle( rRect ) );
}

void VCLXRegion::excludeRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Exclude( VCLRectangle( rRect ) );
}

void VCLXRegion::xOrRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.XOr( VCLRectangle( rRect ) );
}

void VCLXRegion::unionRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.Union( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::intersectRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.Intersect( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::excludeRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.Exclude( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::xOrRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.XOr( VCLUnoHelper::GetRegion( rxRegion ) );
}

::com::sun::star::uno::Sequence< ::com::sun::star::awt::Rectangle > VCLXRegion::getRectangles() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uLong nRects = maRegion.GetRectCount();
    ::com::sun::star::uno::Sequence< ::com::sun::star::awt::Rectangle > aRects( nRects );

    Rectangle aRect;
    sal_uInt32 nR = 0;
    RegionHandle h = maRegion.BeginEnumRects();
    while ( maRegion.GetNextEnumRect( h, aRect ) )
        aRects.getArray()[nR++] = AWTRectangle( aRect );
    maRegion.EndEnumRects( h );

    return aRects;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
