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

#include <graphic/UnoGraphic.hxx>

#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <vcl/dibtools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <string.h>

using namespace com::sun::star;

namespace unographic {

Graphic::Graphic() :
    maGraphic()
{
}

Graphic::~Graphic() throw()
{
}

void Graphic::init( const ::Graphic& rGraphic )
    throw()
{
    maGraphic = rGraphic;
    unographic::GraphicDescriptor::init(maGraphic);
}

uno::Any SAL_CALL Graphic::queryAggregation( const uno::Type& rType )
{
    uno::Any aAny;
    if( rType == cppu::UnoType<graphic::XGraphic>::get())
        aAny <<= uno::Reference< graphic::XGraphic >( this );
    else if( rType == cppu::UnoType<awt::XBitmap>::get())
        aAny <<= uno::Reference< awt::XBitmap >( this );
    else if( rType == cppu::UnoType<lang::XUnoTunnel>::get())
        aAny <<= uno::Reference< lang::XUnoTunnel >(this);
    else
        aAny = ::unographic::GraphicDescriptor::queryAggregation( rType );

    return aAny;
}

uno::Any SAL_CALL Graphic::queryInterface( const uno::Type & rType )
{
    css::uno::Any aReturn = ::unographic::GraphicDescriptor::queryInterface( rType );
    if ( !aReturn.hasValue() )
        aReturn = ::cppu::queryInterface ( rType, static_cast< graphic::XGraphicTransformer*>( this ) );
    return aReturn;
}

void SAL_CALL Graphic::acquire()
    throw()
{
    unographic::GraphicDescriptor::acquire();
}

void SAL_CALL Graphic::release() throw()
{
    unographic::GraphicDescriptor::release();
}

OUString SAL_CALL Graphic::getImplementationName()
{
    return OUString( "com.sun.star.comp.graphic.Graphic" );
}

sal_Bool SAL_CALL Graphic::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

uno::Sequence< OUString > SAL_CALL Graphic::getSupportedServiceNames()
{
    uno::Sequence< OUString >    aRet( ::unographic::GraphicDescriptor::getSupportedServiceNames() );
    uno::Sequence< OUString >    aNew { "com.sun.star.graphic.Graphic" };
    sal_Int32                           nOldCount = aRet.getLength();

    aRet.realloc( nOldCount + aNew.getLength() );

    for( sal_Int32 i = 0; i < aNew.getLength(); ++i )
        aRet[ nOldCount++ ] = aNew[ i ];

    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL Graphic::getTypes()
{
    return cppu::OTypeCollection(
            cppu::UnoType<graphic::XGraphic>::get(),
            cppu::UnoType<awt::XBitmap>::get(),
            ::unographic::GraphicDescriptor::getTypes()
        ).getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL Graphic::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

sal_Int8 SAL_CALL Graphic::getType()
{
    sal_Int8 cRet = graphic::GraphicType::EMPTY;

    if (!!maGraphic)
    {
        ::GraphicType eType = maGraphic.GetType();

        if (eType != ::GraphicType::NONE)
        {
            cRet = (eType == ::GraphicType::Bitmap) ? graphic::GraphicType::PIXEL
                                                  : graphic::GraphicType::VECTOR;
        }
    }

    return cRet;
}

// XBitmap

awt::Size SAL_CALL Graphic::getSize()
{
    SolarMutexGuard aGuard;

    Size aVclSize;
    if (!!maGraphic && maGraphic.GetType() != ::GraphicType::NONE)
    {
        aVclSize = maGraphic.GetSizePixel();
    }
    return awt::Size(aVclSize.Width(), aVclSize.Height());
}

uno::Sequence<sal_Int8> SAL_CALL Graphic::getDIB()
{
    SolarMutexGuard aGuard;

    if (!!maGraphic && maGraphic.GetType() != ::GraphicType::NONE)
    {
        SvMemoryStream aMemoryStream;

        WriteDIB(maGraphic.GetBitmapEx().GetBitmap(), aMemoryStream, false, true);
        return css::uno::Sequence<sal_Int8>(static_cast<sal_Int8 const *>(aMemoryStream.GetData()), aMemoryStream.Tell());
    }
    else
    {
        return uno::Sequence<sal_Int8>();
    }
}

uno::Sequence<sal_Int8> SAL_CALL Graphic::getMaskDIB()
{
    SolarMutexGuard aGuard;

    if (!!maGraphic && maGraphic.GetType() != ::GraphicType::NONE)
    {
        SvMemoryStream aMemoryStream;

        WriteDIB(maGraphic.GetBitmapEx().GetMask(), aMemoryStream, false, true);
        return css::uno::Sequence<sal_Int8>( static_cast<sal_Int8 const *>(aMemoryStream.GetData()), aMemoryStream.Tell() );
    }
    else
    {
        return uno::Sequence<sal_Int8>();
    }
}

const ::Graphic* Graphic::getImplementation( const uno::Reference< uno::XInterface >& rxIFace )
    throw()
{
    uno::Reference< lang::XUnoTunnel > xTunnel( rxIFace, uno::UNO_QUERY );
    return( xTunnel.is() ? reinterpret_cast< ::Graphic* >( xTunnel->getSomething( ::Graphic::getUnoTunnelId() ) ) : nullptr );
}

sal_Int64 SAL_CALL Graphic::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    return( ( rId.getLength() == 16 && 0 == memcmp( ::Graphic::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) ?
            reinterpret_cast<sal_Int64>(&maGraphic) : 0 );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
