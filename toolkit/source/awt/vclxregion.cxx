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

#include <toolkit/awt/vclxregion.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <rtl/uuid.h>
#include <vcl/svapp.hxx>


//  class VCLXRegion

VCLXRegion::VCLXRegion()
{
}

VCLXRegion::~VCLXRegion()
{
}

// css::uno::XInterface
css::uno::Any VCLXRegion::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XRegion* >(this),
                                        static_cast< css::lang::XUnoTunnel* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// css::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXRegion )

IMPL_IMPLEMENTATION_ID( VCLXRegion )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXRegion::getTypes()
{
    static const css::uno::Sequence< css::uno::Type > aTypeList {
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XRegion>::get()
    };
    return aTypeList;
}


css::awt::Rectangle VCLXRegion::getBounds()
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return AWTRectangle( maRegion.GetBoundRect() );
}

void VCLXRegion::clear()
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.SetEmpty();
}

void VCLXRegion::move( sal_Int32 nHorzMove, sal_Int32 nVertMove )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Move( nHorzMove, nVertMove );
}

void VCLXRegion::unionRectangle( const css::awt::Rectangle& rRect )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Union( VCLRectangle( rRect ) );
}

void VCLXRegion::intersectRectangle( const css::awt::Rectangle& rRect )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Intersect( VCLRectangle( rRect ) );
}

void VCLXRegion::excludeRectangle( const css::awt::Rectangle& rRect )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Exclude( VCLRectangle( rRect ) );
}

void VCLXRegion::xOrRectangle( const css::awt::Rectangle& rRect )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.XOr( VCLRectangle( rRect ) );
}

void VCLXRegion::unionRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.Union( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::intersectRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.Intersect( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::excludeRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.Exclude( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::xOrRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.XOr( VCLUnoHelper::GetRegion( rxRegion ) );
}

css::uno::Sequence< css::awt::Rectangle > VCLXRegion::getRectangles()
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    RectangleVector aRectangles;
    maRegion.GetRegionRectangles(aRectangles);

//    sal_uLong nRects = maRegion.GetRectCount();
    css::uno::Sequence< css::awt::Rectangle > aRects(aRectangles.size());
    sal_uInt32 a(0);

    for(const auto& rRect : aRectangles)
    {
        aRects.getArray()[a++] = AWTRectangle(rRect);
    }

    //Rectangle aRect;
    //sal_uInt32 nR = 0;
    //RegionHandle h = maRegion.BeginEnumRects();
    //while ( maRegion.GetEnumRects( h, aRect ) )
    //  aRects.getArray()[nR++] = AWTRectangle( aRect );
    //maRegion.EndEnumRects( h );

    return aRects;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
