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

#include "descriptor.hxx"

#include <osl/mutex.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <svl/itemprop.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>

#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <memory>

#define UNOGRAPHIC_GRAPHICTYPE  1
#define UNOGRAPHIC_MIMETYPE     2
#define UNOGRAPHIC_SIZEPIXEL    3
#define UNOGRAPHIC_SIZE100THMM  4
#define UNOGRAPHIC_BITSPERPIXEL 5
#define UNOGRAPHIC_TRANSPARENT  6
#define UNOGRAPHIC_ALPHA        7
#define UNOGRAPHIC_ANIMATED     8

using namespace ::com::sun::star;

namespace unographic {


// - GraphicDescriptor -


GraphicDescriptor::GraphicDescriptor() :
    ::comphelper::PropertySetHelper( createPropertySetInfo(), SAL_NO_ACQUIRE ),
    mpGraphic( nullptr ),
    meType( GRAPHIC_NONE ),
    mnBitsPerPixel ( 0 ),
    mbTransparent ( false ),
    mbAlpha( false ),
    mbAnimated( false )
{
}

GraphicDescriptor::~GraphicDescriptor()
    throw()
{
}

void GraphicDescriptor::init( const ::Graphic& rGraphic )
{
    mpGraphic = &rGraphic;
}

void GraphicDescriptor::init( const OUString& rURL )
{
    std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( rURL, StreamMode::READ ));

    if( pIStm )
        implCreate( *pIStm, &rURL );
}

void GraphicDescriptor::init( const uno::Reference< io::XInputStream >& rxIStm, const OUString& rURL )
{
    std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( rxIStm ));

    if( pIStm )
        implCreate( *pIStm, &rURL );
}

void GraphicDescriptor::implCreate( SvStream& rIStm, const OUString* pURL )
{
    OUString aURL;
    if( pURL )
        aURL = *pURL;
    ::GraphicDescriptor aDescriptor( rIStm, &aURL );

    mpGraphic = nullptr;
    maMimeType.clear();
    meType = GRAPHIC_NONE;
    mnBitsPerPixel = 0;
    mbTransparent = false;

    if( aDescriptor.Detect( true ) && aDescriptor.GetFileFormat() != GraphicFileFormat::NOT )
    {
        const char*             pMimeType = nullptr;
        sal_uInt8               cType = graphic::GraphicType::EMPTY;

        switch( aDescriptor.GetFileFormat() )
        {
            case( GraphicFileFormat::BMP ): pMimeType = MIMETYPE_BMP; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::GIF ): pMimeType = MIMETYPE_GIF; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::JPG ): pMimeType = MIMETYPE_JPG; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::PCD ): pMimeType = MIMETYPE_PCD; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::PCX ): pMimeType = MIMETYPE_PCX; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::PNG ): pMimeType = MIMETYPE_PNG; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::TIF ): pMimeType = MIMETYPE_TIF; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::XBM ): pMimeType = MIMETYPE_XBM; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::XPM ): pMimeType = MIMETYPE_XPM; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::PBM ): pMimeType = MIMETYPE_PBM; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::PGM ): pMimeType = MIMETYPE_PGM; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::PPM ): pMimeType = MIMETYPE_PPM; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::RAS ): pMimeType = MIMETYPE_RAS; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::TGA ): pMimeType = MIMETYPE_TGA; cType = graphic::GraphicType::PIXEL; break;
            case( GraphicFileFormat::PSD ): pMimeType = MIMETYPE_PSD; cType = graphic::GraphicType::PIXEL; break;

            case( GraphicFileFormat::EPS ): pMimeType = MIMETYPE_EPS; cType = graphic::GraphicType::VECTOR; break;
            case( GraphicFileFormat::DXF ): pMimeType = MIMETYPE_DXF; cType = graphic::GraphicType::VECTOR; break;
            case( GraphicFileFormat::MET ): pMimeType = MIMETYPE_MET; cType = graphic::GraphicType::VECTOR; break;
            case( GraphicFileFormat::PCT ): pMimeType = MIMETYPE_PCT; cType = graphic::GraphicType::VECTOR; break;
            case( GraphicFileFormat::SGF ): pMimeType = MIMETYPE_SGF; cType = graphic::GraphicType::VECTOR; break;
            case( GraphicFileFormat::SVM ): pMimeType = MIMETYPE_SVM; cType = graphic::GraphicType::VECTOR; break;
            case( GraphicFileFormat::WMF ): pMimeType = MIMETYPE_WMF; cType = graphic::GraphicType::VECTOR; break;
            case( GraphicFileFormat::SGV ): pMimeType = MIMETYPE_SGV; cType = graphic::GraphicType::VECTOR; break;
            case( GraphicFileFormat::EMF ): pMimeType = MIMETYPE_EMF; cType = graphic::GraphicType::VECTOR; break;
            case( GraphicFileFormat::SVG ): pMimeType = MIMETYPE_SVG; cType = graphic::GraphicType::VECTOR; break;

            default:
            break;
        }

        if( graphic::GraphicType::EMPTY != cType )
        {
            meType = ( ( graphic::GraphicType::PIXEL == cType ) ? GRAPHIC_BITMAP : GRAPHIC_GDIMETAFILE );
            maMimeType = OUString( pMimeType, strlen(pMimeType), RTL_TEXTENCODING_ASCII_US );
            maSizePixel = aDescriptor.GetSizePixel();
            maSize100thMM = aDescriptor.GetSize_100TH_MM();
            mnBitsPerPixel = aDescriptor.GetBitsPerPixel();
            mbTransparent = ( graphic::GraphicType::VECTOR == cType );
            mbAlpha = mbAnimated = false;
        }
    }
}



OUString GraphicDescriptor::getImplementationName_Static()
    throw()
{
    return OUString( "com.sun.star.comp.graphic.GraphicDescriptor"  );
}



uno::Sequence< OUString > GraphicDescriptor::getSupportedServiceNames_Static()
    throw(  )
{
    uno::Sequence< OUString > aSeq { "com.sun.star.graphic.GraphicDescriptor" };
    return aSeq;
}



uno::Any SAL_CALL GraphicDescriptor::queryAggregation( const uno::Type & rType )
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
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

    return aAny;
}



uno::Any SAL_CALL GraphicDescriptor::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException, std::exception )
{
    return OWeakAggObject::queryInterface( rType );
}



void SAL_CALL GraphicDescriptor::acquire()
    throw()
{
    OWeakAggObject::acquire();
}



void SAL_CALL GraphicDescriptor::release()
    throw()
{
    OWeakAggObject::release();
}



OUString SAL_CALL GraphicDescriptor::getImplementationName()
    throw( uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL GraphicDescriptor::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}



uno::Sequence< OUString > SAL_CALL GraphicDescriptor::getSupportedServiceNames()
    throw( uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}



uno::Sequence< uno::Type > SAL_CALL GraphicDescriptor::getTypes()
    throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< uno::Type >  aTypes( 6 );
    uno::Type*                  pTypes = aTypes.getArray();

    *pTypes++ = cppu::UnoType<uno::XAggregation>::get();
    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
    *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
    *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
    *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL GraphicDescriptor::getImplementationId()
    throw( uno::RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}



::comphelper::PropertySetInfo* GraphicDescriptor::createPropertySetInfo()
{
    SolarMutexGuard aGuard;
    ::comphelper::PropertySetInfo*  pRet = new ::comphelper::PropertySetInfo();

    static ::comphelper::PropertyMapEntry const aEntries[] =
    {
        { OUString("GraphicType"), UNOGRAPHIC_GRAPHICTYPE, cppu::UnoType<sal_Int8>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("MimeType"), UNOGRAPHIC_MIMETYPE, cppu::UnoType<OUString>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("SizePixel"), UNOGRAPHIC_SIZEPIXEL, cppu::UnoType<awt::Size>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("Size100thMM"), UNOGRAPHIC_SIZE100THMM, cppu::UnoType<awt::Size>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("BitsPerPixel"), UNOGRAPHIC_BITSPERPIXEL, cppu::UnoType<sal_uInt8>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("Transparent"), UNOGRAPHIC_TRANSPARENT, cppu::UnoType<sal_Bool>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("Alpha"), UNOGRAPHIC_ALPHA, cppu::UnoType<sal_Bool>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("Animated"), UNOGRAPHIC_ANIMATED, cppu::UnoType<sal_Bool>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    pRet->acquire();
    pRet->add( aEntries );

    return pRet;
}



void GraphicDescriptor::_setPropertyValues( const comphelper::PropertyMapEntry** /*ppEntries*/, const uno::Any* /*pValues*/ )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
              lang::WrappedTargetException )
{
    // we only have readonly attributes
}



void GraphicDescriptor::_getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, uno::Any* pValues )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    while( *ppEntries )
    {
        switch( (*ppEntries)->mnHandle )
        {
            case( UNOGRAPHIC_GRAPHICTYPE ):
            {
                const GraphicType eType( mpGraphic ? mpGraphic->GetType() : meType );

                *pValues <<= ( ( eType == GRAPHIC_BITMAP ? graphic::GraphicType::PIXEL :
                                ( eType == GRAPHIC_GDIMETAFILE ? graphic::GraphicType::VECTOR :
                                graphic::GraphicType::EMPTY ) ) );
            }
            break;

            case( UNOGRAPHIC_MIMETYPE ):
            {
                OUString aMimeType;

                if( mpGraphic )
                {
                    if( mpGraphic->IsLink() )
                    {
                        const char* pMimeType;

                        switch( mpGraphic->GetLink().GetType() )
                        {
                            case( GFX_LINK_TYPE_NATIVE_GIF ): pMimeType = MIMETYPE_GIF; break;

                            // #i15508# added BMP type for better exports (checked, works)
                            case( GFX_LINK_TYPE_NATIVE_BMP ): pMimeType = MIMETYPE_BMP; break;

                            case( GFX_LINK_TYPE_NATIVE_JPG ): pMimeType = MIMETYPE_JPG; break;
                            case( GFX_LINK_TYPE_NATIVE_PNG ): pMimeType = MIMETYPE_PNG; break;
                            case( GFX_LINK_TYPE_NATIVE_WMF ): pMimeType = MIMETYPE_WMF; break;
                            case( GFX_LINK_TYPE_NATIVE_MET ): pMimeType = MIMETYPE_MET; break;
                            case( GFX_LINK_TYPE_NATIVE_PCT ): pMimeType = MIMETYPE_PCT; break;

                            // added Svg mimetype support
                            case( GFX_LINK_TYPE_NATIVE_SVG ): pMimeType = MIMETYPE_SVG; break;

                            default:
                                pMimeType = nullptr;
                            break;
                        }

                        if( pMimeType )
                            aMimeType = OUString::createFromAscii( pMimeType );
                    }

                    if( aMimeType.isEmpty() && ( mpGraphic->GetType() != GRAPHIC_NONE ) )
                        aMimeType = MIMETYPE_VCLGRAPHIC;
                }
                else
                    aMimeType = maMimeType;

                 *pValues <<= aMimeType;
            }
            break;

            case( UNOGRAPHIC_SIZEPIXEL ):
            {
                awt::Size aAWTSize( 0, 0 );

                if( mpGraphic )
                {
                    if( mpGraphic->GetType() == GRAPHIC_BITMAP )
                    {
                        const Size aSizePix( mpGraphic->GetBitmapEx().GetSizePixel() );
                        aAWTSize = awt::Size( aSizePix.Width(), aSizePix.Height() );
                    }
                }
                else
                    aAWTSize = awt::Size( maSizePixel.Width(), maSizePixel.Height() );

                *pValues <<= aAWTSize;
            }
            break;

            case( UNOGRAPHIC_SIZE100THMM ):
            {
                awt::Size aAWTSize( 0, 0 );

                if( mpGraphic )
                {
                    if( mpGraphic->GetPrefMapMode().GetMapUnit() != MAP_PIXEL )
                    {
                        const Size aSizeLog( OutputDevice::LogicToLogic( mpGraphic->GetPrefSize(), mpGraphic->GetPrefMapMode(), MAP_100TH_MM ) );
                        aAWTSize = awt::Size( aSizeLog.Width(), aSizeLog.Height() );
                    }
                }
                else
                    aAWTSize = awt::Size( maSize100thMM.Width(), maSize100thMM.Height() );

                *pValues <<= aAWTSize;
            }
            break;

            case( UNOGRAPHIC_BITSPERPIXEL ):
            {
                sal_uInt16 nBitsPerPixel = 0;

                if( mpGraphic )
                {
                    if( mpGraphic->GetType() == GRAPHIC_BITMAP )
                        nBitsPerPixel = mpGraphic->GetBitmapEx().GetBitmap().GetBitCount();
                }
                else
                    nBitsPerPixel = mnBitsPerPixel;

                *pValues <<= sal::static_int_cast< sal_Int8 >(nBitsPerPixel);
            }
            break;

            case( UNOGRAPHIC_TRANSPARENT ):
            {
                *pValues <<= mpGraphic ? mpGraphic->IsTransparent() : mbTransparent;
            }
            break;

            case( UNOGRAPHIC_ALPHA ):
            {
                *pValues <<= mpGraphic ? mpGraphic->IsAlpha() : mbAlpha;
            }
            break;

            case( UNOGRAPHIC_ANIMATED ):
            {
                *pValues <<= mpGraphic ? mpGraphic->IsAnimated() : mbAnimated;
            }
            break;
        }

        ++ppEntries;
        ++pValues;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
