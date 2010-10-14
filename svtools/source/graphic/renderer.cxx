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
#include "precompiled_svtools.hxx"

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <rtl/uuid.h>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <svl/itemprop.hxx>
#include <svtools/grfmgr.hxx>
#include "graphic.hxx"
#include "renderer.hxx"

#define UNOGRAPHIC_DEVICE           1
#define UNOGRAPHIC_DESTINATIONRECT  2
#define UNOGRAPHIC_RENDERDATA       3

using namespace ::com::sun::star;

namespace unographic {

// ---------------------
// - GraphicRendererVCL -
// ---------------------

uno::Reference< uno::XInterface > SAL_CALL GraphicRendererVCL_CreateInstance( const uno::Reference< lang::XMultiServiceFactory >& )
{
    return SAL_STATIC_CAST( ::cppu::OWeakObject*, new GraphicRendererVCL );
}


GraphicRendererVCL::GraphicRendererVCL() :
    ::comphelper::PropertySetHelper( createPropertySetInfo() ),
    mpOutDev( NULL )
{
}

// ------------------------------------------------------------------------------

GraphicRendererVCL::~GraphicRendererVCL()
    throw()
{
}

// ------------------------------------------------------------------------------

::rtl::OUString GraphicRendererVCL::getImplementationName_Static()
    throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.graphic.GraphicRendererVCL" ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > GraphicRendererVCL::getSupportedServiceNames_Static()
    throw(  )
{
    uno::Sequence< ::rtl::OUString > aSeq( 1 );

    aSeq.getArray()[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.graphic.GraphicRendererVCL" ) );

    return aSeq;
}

// ------------------------------------------------------------------------------

uno::Any SAL_CALL GraphicRendererVCL::queryAggregation( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    uno::Any aAny;

    if( rType == ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0) )
        aAny <<= uno::Reference< lang::XServiceInfo >(this);
    else if( rType == ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0) )
        aAny <<= uno::Reference< lang::XTypeProvider >(this);
    else if( rType == ::getCppuType((const uno::Reference< beans::XPropertySet >*)0) )
        aAny <<= uno::Reference< beans::XPropertySet >(this);
    else if( rType == ::getCppuType((const uno::Reference< beans::XPropertyState >*)0) )
        aAny <<= uno::Reference< beans::XPropertyState >(this);
    else if( rType == ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0) )
        aAny <<= uno::Reference< beans::XMultiPropertySet >(this);
    else if( rType == ::getCppuType((const uno::Reference< graphic::XGraphicRenderer >*)0) )
        aAny <<= uno::Reference< graphic::XGraphicRenderer >(this);
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

    return aAny;
}

// ------------------------------------------------------------------------------

uno::Any SAL_CALL GraphicRendererVCL::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    return OWeakAggObject::queryInterface( rType );
}

// ------------------------------------------------------------------------------

void SAL_CALL GraphicRendererVCL::acquire()
    throw()
{
    OWeakAggObject::acquire();
}

// ------------------------------------------------------------------------------

void SAL_CALL GraphicRendererVCL::release()
    throw()
{
    OWeakAggObject::release();
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL GraphicRendererVCL::getImplementationName()
    throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GraphicRendererVCL::supportsService( const rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString >    aSNL( getSupportedServiceNames() );
    const ::rtl::OUString*              pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

// ------------------------------------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL GraphicRendererVCL::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

// ------------------------------------------------------------------------------

uno::Sequence< uno::Type > SAL_CALL GraphicRendererVCL::getTypes()
    throw( uno::RuntimeException )
{
    uno::Sequence< uno::Type >  aTypes( 7 );
    uno::Type*                  pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< uno::XAggregation>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< graphic::XGraphicRenderer>*)0);

    return aTypes;
}

// ------------------------------------------------------------------------------

uno::Sequence< sal_Int8 > SAL_CALL GraphicRendererVCL::getImplementationId()
    throw( uno::RuntimeException )
{
    vos::OGuard                         aGuard( Application::GetSolarMutex() );
    static uno::Sequence< sal_Int8 >    aId;

    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( aId.getArray() ), 0, sal_True );
    }

    return aId;
}

// ------------------------------------------------------------------------------

::comphelper::PropertySetInfo* GraphicRendererVCL::createPropertySetInfo()
{
    vos::OGuard                     aGuard( Application::GetSolarMutex() );
    ::comphelper::PropertySetInfo*  pRet = new ::comphelper::PropertySetInfo();

    static ::comphelper::PropertyMapEntry aEntries[] =
    {
        { MAP_CHAR_LEN( "Device" ), UNOGRAPHIC_DEVICE, &::getCppuType( (const uno::Any*)(0)), 0, 0 },
        { MAP_CHAR_LEN( "DestinationRect" ), UNOGRAPHIC_DESTINATIONRECT, &::getCppuType( (const awt::Rectangle*)(0)), 0, 0 },
        { MAP_CHAR_LEN( "RenderData" ), UNOGRAPHIC_RENDERDATA,  &::getCppuType( (const uno::Any*)(0)), 0, 0 },

        { 0,0,0,0,0,0 }
    };

    pRet->acquire();
    pRet->add( aEntries );

    return pRet;
}

// ------------------------------------------------------------------------------

void GraphicRendererVCL::_setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const uno::Any* pValues )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
              lang::WrappedTargetException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    while( *ppEntries )
    {
        switch( (*ppEntries)->mnHandle )
        {
            case( UNOGRAPHIC_DEVICE ):
            {
                uno::Reference< awt::XDevice > xDevice;

                if( ( *pValues >>= xDevice ) && xDevice.is() )
                {
                    mxDevice = xDevice;
                    mpOutDev = VCLUnoHelper::GetOutputDevice( xDevice );
                }
                else
                {
                    mxDevice.clear();
                    mpOutDev = NULL;
                }
            }
            break;

            case( UNOGRAPHIC_DESTINATIONRECT ):
            {
                awt::Rectangle aAWTRect;

                if( *pValues >>= aAWTRect )
                {
                    maDestRect = Rectangle( Point( aAWTRect.X, aAWTRect.Y ),
                                            Size( aAWTRect.Width, aAWTRect.Height ) );
                }
            }
            break;

            case( UNOGRAPHIC_RENDERDATA ):
            {
                *pValues >>= maRenderData;
            }
            break;
        }

        ++ppEntries;
        ++pValues;
    }
}

// ------------------------------------------------------------------------------

void GraphicRendererVCL::_getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, uno::Any* pValues )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    while( *ppEntries )
    {
        switch( (*ppEntries)->mnHandle )
        {
            case( UNOGRAPHIC_DEVICE ):
            {
                if( mxDevice.is() )
                    *pValues <<= mxDevice;
            }
            break;

            case( UNOGRAPHIC_DESTINATIONRECT ):
            {
                const awt::Rectangle aAWTRect( maDestRect.Left(), maDestRect.Top(),
                                               maDestRect.GetWidth(), maDestRect.GetHeight() );

                *pValues <<= aAWTRect;
            }
            break;

            case( UNOGRAPHIC_RENDERDATA ):
            {
                *pValues <<= maRenderData;
            }
            break;
        }

        ++ppEntries;
        ++pValues;
    }
}

// ------------------------------------------------------------------------------

void SAL_CALL GraphicRendererVCL::render( const uno::Reference< graphic::XGraphic >& rxGraphic )
    throw (uno::RuntimeException)
{
    if( mpOutDev && mxDevice.is() && rxGraphic.is() )
    {
        const uno::Reference< XInterface >  xIFace( rxGraphic, uno::UNO_QUERY );
        const ::Graphic*                    pGraphic = ::unographic::Graphic::getImplementation( xIFace );

        if( pGraphic )
        {
            GraphicObject aGraphicObject( *pGraphic );
            aGraphicObject.Draw( mpOutDev, maDestRect.TopLeft(), maDestRect.GetSize() );
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
