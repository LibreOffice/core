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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <svl/itemprop.hxx>
#include <svtools/grfmgr.hxx>
#include <comphelper/servicehelper.hxx>
#include "graphic.hxx"
#include "renderer.hxx"

#define UNOGRAPHIC_DEVICE           1
#define UNOGRAPHIC_DESTINATIONRECT  2
#define UNOGRAPHIC_RENDERDATA       3

using namespace ::com::sun::star;

namespace {

GraphicRendererVCL::GraphicRendererVCL() :
    ::comphelper::PropertySetHelper( createPropertySetInfo() ),
    mpOutDev( nullptr )
{
}



GraphicRendererVCL::~GraphicRendererVCL()
    throw()
{
}



uno::Any SAL_CALL GraphicRendererVCL::queryAggregation( const uno::Type & rType )
    throw( uno::RuntimeException, std::exception )
{
    uno::Any aAny;

    if( rType == cppu::UnoType<lang::XServiceInfo>::get())
        aAny <<= uno::Reference< lang::XServiceInfo >(this);
    else if( rType == cppu::UnoType<lang::XTypeProvider>::get())
        aAny <<= uno::Reference< lang::XTypeProvider >(this);
    else if( rType == cppu::UnoType<beans::XPropertySet>::get())
        aAny <<= uno::Reference< beans::XPropertySet >(this);
    else if( rType == cppu::UnoType<beans::XPropertyState>::get())
        aAny <<= uno::Reference< beans::XPropertyState >(this);
    else if( rType == cppu::UnoType<beans::XMultiPropertySet>::get())
        aAny <<= uno::Reference< beans::XMultiPropertySet >(this);
    else if( rType == cppu::UnoType<graphic::XGraphicRenderer>::get())
        aAny <<= uno::Reference< graphic::XGraphicRenderer >(this);
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

    return aAny;
}



uno::Any SAL_CALL GraphicRendererVCL::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException, std::exception )
{
    return OWeakAggObject::queryInterface( rType );
}



void SAL_CALL GraphicRendererVCL::acquire()
    throw()
{
    OWeakAggObject::acquire();
}



void SAL_CALL GraphicRendererVCL::release()
    throw()
{
    OWeakAggObject::release();
}



OUString SAL_CALL GraphicRendererVCL::getImplementationName()
    throw( uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.graphic.GraphicRendererVCL" );
}

sal_Bool SAL_CALL GraphicRendererVCL::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}



uno::Sequence< OUString > SAL_CALL GraphicRendererVCL::getSupportedServiceNames()
    throw( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aSeq { "com.sun.star.graphic.GraphicRendererVCL" };
    return aSeq;
}



uno::Sequence< uno::Type > SAL_CALL GraphicRendererVCL::getTypes()
    throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< uno::Type >  aTypes( 7 );
    uno::Type*                  pTypes = aTypes.getArray();

    *pTypes++ = cppu::UnoType<uno::XAggregation>::get();
    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
    *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
    *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
    *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
    *pTypes++ = cppu::UnoType<graphic::XGraphicRenderer>::get();

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL GraphicRendererVCL::getImplementationId()
    throw( uno::RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}



::comphelper::PropertySetInfo* GraphicRendererVCL::createPropertySetInfo()
{
    SolarMutexGuard aGuard;
    ::comphelper::PropertySetInfo*  pRet = new ::comphelper::PropertySetInfo();

    static ::comphelper::PropertyMapEntry const aEntries[] =
    {
        { OUString("Device"), UNOGRAPHIC_DEVICE, cppu::UnoType<uno::Any>::get(), 0, 0 },
        { OUString("DestinationRect"), UNOGRAPHIC_DESTINATIONRECT, cppu::UnoType<awt::Rectangle>::get(), 0, 0 },
        { OUString("RenderData"), UNOGRAPHIC_RENDERDATA, cppu::UnoType<uno::Any>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    pRet->acquire();
    pRet->add( aEntries );

    return pRet;
}



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
                    mpOutDev = nullptr;
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



void SAL_CALL GraphicRendererVCL::render( const uno::Reference< graphic::XGraphic >& rxGraphic )
    throw (uno::RuntimeException, std::exception)
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_graphic_GraphicRendererVCL_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new GraphicRendererVCL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
