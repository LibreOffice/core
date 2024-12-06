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

#include <awt/vclxregion.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/unohelp.hxx>


VCLXRegion::VCLXRegion()
{
}

VCLXRegion::~VCLXRegion()
{
}

css::awt::Rectangle VCLXRegion::getBounds()
{
    std::scoped_lock aGuard( maMutex );

    return vcl::unohelper::ConvertToAWTRect(maRegion.GetBoundRect());
}

void VCLXRegion::clear()
{
    std::scoped_lock aGuard( maMutex );

    maRegion.SetEmpty();
}

void VCLXRegion::move( sal_Int32 nHorzMove, sal_Int32 nVertMove )
{
    std::scoped_lock aGuard( maMutex );

    maRegion.Move( nHorzMove, nVertMove );
}

void VCLXRegion::unionRectangle( const css::awt::Rectangle& rRect )
{
    std::scoped_lock aGuard( maMutex );

    maRegion.Union(vcl::unohelper::ConvertToVCLRect(rRect));
}

void VCLXRegion::intersectRectangle( const css::awt::Rectangle& rRect )
{
    std::scoped_lock aGuard( maMutex );

    maRegion.Intersect(vcl::unohelper::ConvertToVCLRect(rRect));
}

void VCLXRegion::excludeRectangle( const css::awt::Rectangle& rRect )
{
    std::scoped_lock aGuard( maMutex );

    maRegion.Exclude(vcl::unohelper::ConvertToVCLRect(rRect));
}

void VCLXRegion::xOrRectangle( const css::awt::Rectangle& rRect )
{
    std::scoped_lock aGuard( maMutex );

    maRegion.XOr(vcl::unohelper::ConvertToVCLRect(rRect));
}

void VCLXRegion::unionRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion )
{
    std::scoped_lock aGuard( maMutex );

    if ( rxRegion.is() )
        maRegion.Union( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::intersectRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion )
{
    std::scoped_lock aGuard( maMutex );

    if ( rxRegion.is() )
        maRegion.Intersect( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::excludeRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion )
{
    std::scoped_lock aGuard( maMutex );

    if ( rxRegion.is() )
        maRegion.Exclude( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::xOrRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion )
{
    std::scoped_lock aGuard( maMutex );

    if ( rxRegion.is() )
        maRegion.XOr( VCLUnoHelper::GetRegion( rxRegion ) );
}

css::uno::Sequence< css::awt::Rectangle > VCLXRegion::getRectangles()
{
    std::scoped_lock aGuard( maMutex );

    RectangleVector aRectangles;
    maRegion.GetRegionRectangles(aRectangles);

//    sal_uLong nRects = maRegion.GetRectCount();
    css::uno::Sequence< css::awt::Rectangle > aRects(aRectangles.size());
    sal_uInt32 a(0);

    for(const auto& rRect : aRectangles)
    {
        aRects.getArray()[a++] = vcl::unohelper::ConvertToAWTRect(rRect);
    }

    //Rectangle aRect;
    //sal_uInt32 nR = 0;
    //RegionHandle h = maRegion.BeginEnumRects();
    //while ( maRegion.GetEnumRects( h, aRect ) )
    //  aRects.getArray()[nR++] = vcl::unohelper::ConvertToAWTRect( aRect );
    //maRegion.EndEnumRects( h );

    return aRects;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
