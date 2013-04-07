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


#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <svl/itemprop.hxx>
#include <svtools/grfmgr.hxx>
#include <comphelper/servicehelper.hxx>
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
    return (static_cast< ::cppu::OWeakObject* >(new GraphicRendererVCL ));
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

OUString GraphicRendererVCL::getImplementationName_Static()
    throw()
{
    return OUString( "com.sun.star.comp.graphic.GraphicRendererVCL" );
}

// ------------------------------------------------------------------------------

uno::Sequence< OUString > GraphicRendererVCL::getSupportedServiceNames_Static()
    throw(  )
{
    uno::Sequence< OUString > aSeq( 1 );

    aSeq.getArray()[ 0 ] = OUString( "com.sun.star.graphic.GraphicRendererVCL" );

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

OUString SAL_CALL GraphicRendererVCL::getImplementationName()
    throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GraphicRendererVCL::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString >    aSNL( getSupportedServiceNames() );
    const OUString*              pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

// ------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL GraphicRendererVCL::getSupportedServiceNames()
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

namespace
{
    class theGraphicRendererVCLUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theGraphicRendererVCLUnoTunnelId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL GraphicRendererVCL::getImplementationId()
    throw( uno::RuntimeException )
{
    return theGraphicRendererVCLUnoTunnelId::get().getSeq();
}

// ------------------------------------------------------------------------------

::comphelper::PropertySetInfo* GraphicRendererVCL::createPropertySetInfo()
{
    SolarMutexGuard aGuard;
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
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;

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
