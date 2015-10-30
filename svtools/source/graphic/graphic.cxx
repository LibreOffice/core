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

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <vcl/dibtools.hxx>
#include <vcl/graph.hxx>
#include "graphic.hxx"
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <string.h>

using namespace com::sun::star;

namespace unographic {

Graphic::Graphic() :
    mpGraphic( NULL )
{
}



Graphic::~Graphic()
    throw()
{
    delete mpGraphic;
}



void Graphic::init( const ::Graphic& rGraphic )
    throw()
{
    delete mpGraphic;
    mpGraphic = new ::Graphic( rGraphic );
    ::unographic::GraphicDescriptor::init( *mpGraphic );
}



uno::Any SAL_CALL Graphic::queryAggregation( const uno::Type& rType )
    throw( uno::RuntimeException, std::exception )
{
    uno::Any aAny;
    if( rType == cppu::UnoType<graphic::XGraphic>::get())
        aAny <<= uno::Reference< graphic::XGraphic >( this );
    else if( rType == cppu::UnoType<awt::XBitmap>::get())
        aAny <<= uno::Reference< awt::XBitmap >( this );
    else if( rType == cppu::UnoType<lang::XUnoTunnel>::get())
        aAny <<= uno::Reference< lang::XUnoTunnel >(this);
    else
        aAny <<= ::unographic::GraphicDescriptor::queryAggregation( rType );

    return aAny ;
}



uno::Any SAL_CALL Graphic::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException, std::exception )
{
    css::uno::Any aReturn = ::unographic::GraphicDescriptor::queryInterface( rType );
    if ( !aReturn.hasValue() )
        aReturn = ::cppu::queryInterface ( rType, static_cast< graphic::XGraphicTransformer*>( this ) );
    return aReturn;
}



void SAL_CALL Graphic::acquire()
    throw()
{
    ::unographic::GraphicDescriptor::acquire();
}



void SAL_CALL Graphic::release() throw()
{
    ::unographic::GraphicDescriptor::release();
}

OUString Graphic::getImplementationName_Static()
    throw()
{
    return OUString( "com.sun.star.comp.graphic.Graphic" );
}



uno::Sequence< OUString > Graphic::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence< OUString > aSeq( 1 );

    aSeq.getArray()[ 0 ] = "com.sun.star.graphic.Graphic";

    return aSeq;
}

OUString SAL_CALL Graphic::getImplementationName()
    throw( uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL Graphic::supportsService( const OUString& rServiceName )
    throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService( this, rServiceName );
}

uno::Sequence< OUString > SAL_CALL Graphic::getSupportedServiceNames()
    throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString >    aRet( ::unographic::GraphicDescriptor::getSupportedServiceNames() );
    uno::Sequence< OUString >    aNew( getSupportedServiceNames_Static() );
    sal_Int32                           nOldCount = aRet.getLength();

    aRet.realloc( nOldCount + aNew.getLength() );

    for( sal_Int32 i = 0; i < aNew.getLength(); ++i )
        aRet[ nOldCount++ ] = aNew[ i ];

    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL Graphic::getTypes()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type >  aRet( ::unographic::GraphicDescriptor::getTypes() );
    sal_Int32                   nOldCount = aRet.getLength();

    aRet.realloc( nOldCount + 2 );
    aRet[ nOldCount ] = cppu::UnoType<graphic::XGraphic>::get();
    aRet[ nOldCount+1 ] = cppu::UnoType<awt::XBitmap>::get();

    return aRet;
}



uno::Sequence< sal_Int8 > SAL_CALL Graphic::getImplementationId()
    throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}



::sal_Int8 SAL_CALL Graphic::getType()
     throw (uno::RuntimeException, std::exception)
{
    ::sal_Int8 cRet = graphic::GraphicType::EMPTY;

    if( mpGraphic && ( mpGraphic->GetType() != GRAPHIC_NONE ) )
        cRet = ( ( mpGraphic->GetType() == GRAPHIC_BITMAP ) ? graphic::GraphicType::PIXEL : graphic::GraphicType::VECTOR );

    return cRet;
}


// XBitmap


awt::Size SAL_CALL Graphic::getSize(  ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::Size aVclSize;
    if( mpGraphic && ( mpGraphic->GetType() != GRAPHIC_NONE ) )
        aVclSize = mpGraphic->GetSizePixel();

    return awt::Size( aVclSize.Width(), aVclSize.Height() );
}



uno::Sequence< ::sal_Int8 > SAL_CALL Graphic::getDIB(  ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mpGraphic && ( mpGraphic->GetType() != GRAPHIC_NONE ) )
    {
        SvMemoryStream aMem;

        WriteDIB(mpGraphic->GetBitmapEx().GetBitmap(), aMem, false, true);
        return css::uno::Sequence<sal_Int8>( static_cast<sal_Int8 const *>(aMem.GetData()), aMem.Tell() );
    }
    else
    {
        return uno::Sequence<sal_Int8>();
    }
}



uno::Sequence< ::sal_Int8 > SAL_CALL Graphic::getMaskDIB(  ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mpGraphic && ( mpGraphic->GetType() != GRAPHIC_NONE ) )
    {
        SvMemoryStream aMem;

        WriteDIB(mpGraphic->GetBitmapEx().GetMask(), aMem, false, true);
        return css::uno::Sequence<sal_Int8>( static_cast<sal_Int8 const *>(aMem.GetData()), aMem.Tell() );
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
    return( xTunnel.is() ? reinterpret_cast< ::Graphic* >( xTunnel->getSomething( ::Graphic::getUnoTunnelId() ) ) : NULL );
}


sal_Int64 SAL_CALL Graphic::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw( uno::RuntimeException, std::exception )
{
    return( ( rId.getLength() == 16 && 0 == memcmp( ::Graphic::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) ?
            reinterpret_cast< sal_Int64 >( mpGraphic ) :
            0 );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
