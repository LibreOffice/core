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

#include <graphic/UnoGraphicDescriptor.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <svl/itemprop.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>

#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <memory>

enum class UnoGraphicProperty
{
      GraphicType = 1
    , MimeType = 2
    , SizePixel = 3
    , Size100thMM = 4
    , BitsPerPixel = 5
    , Transparent = 6
    , Alpha = 7
    , Animated = 8
    , Linked = 9
    , OriginURL = 10
};


using namespace ::com::sun::star;

namespace unographic {


GraphicDescriptor::GraphicDescriptor() :
    ::comphelper::PropertySetHelper( createPropertySetInfo() ),
    mpGraphic( nullptr ),
    meType( GraphicType::NONE ),
    mnBitsPerPixel ( 0 ),
    mbTransparent ( false )
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
    meType = GraphicType::NONE;
    mnBitsPerPixel = 0;
    mbTransparent = false;

    if( !(aDescriptor.Detect( true ) && aDescriptor.GetFileFormat() != GraphicFileFormat::NOT) )
        return;

    const char*             pMimeType = nullptr;
    sal_uInt8               cType = graphic::GraphicType::EMPTY;

    switch( aDescriptor.GetFileFormat() )
    {
        case GraphicFileFormat::BMP: pMimeType = MIMETYPE_BMP; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::GIF: pMimeType = MIMETYPE_GIF; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::JPG: pMimeType = MIMETYPE_JPG; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::PCD: pMimeType = MIMETYPE_PCD; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::PCX: pMimeType = MIMETYPE_PCX; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::PNG: pMimeType = MIMETYPE_PNG; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::TIF: pMimeType = MIMETYPE_TIF; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::XBM: pMimeType = MIMETYPE_XBM; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::XPM: pMimeType = MIMETYPE_XPM; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::PBM: pMimeType = MIMETYPE_PBM; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::PGM: pMimeType = MIMETYPE_PGM; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::PPM: pMimeType = MIMETYPE_PPM; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::RAS: pMimeType = MIMETYPE_RAS; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::TGA: pMimeType = MIMETYPE_TGA; cType = graphic::GraphicType::PIXEL; break;
        case GraphicFileFormat::PSD: pMimeType = MIMETYPE_PSD; cType = graphic::GraphicType::PIXEL; break;

        case GraphicFileFormat::EPS: pMimeType = MIMETYPE_EPS; cType = graphic::GraphicType::VECTOR; break;
        case GraphicFileFormat::DXF: pMimeType = MIMETYPE_DXF; cType = graphic::GraphicType::VECTOR; break;
        case GraphicFileFormat::MET: pMimeType = MIMETYPE_MET; cType = graphic::GraphicType::VECTOR; break;
        case GraphicFileFormat::PCT: pMimeType = MIMETYPE_PCT; cType = graphic::GraphicType::VECTOR; break;
        case GraphicFileFormat::SVM: pMimeType = MIMETYPE_SVM; cType = graphic::GraphicType::VECTOR; break;
        case GraphicFileFormat::WMF: pMimeType = MIMETYPE_WMF; cType = graphic::GraphicType::VECTOR; break;
        case GraphicFileFormat::EMF: pMimeType = MIMETYPE_EMF; cType = graphic::GraphicType::VECTOR; break;
        case GraphicFileFormat::SVG: pMimeType = MIMETYPE_SVG; cType = graphic::GraphicType::VECTOR; break;

        default:
        break;
    }

    if( graphic::GraphicType::EMPTY != cType )
    {
        meType = ( ( graphic::GraphicType::PIXEL == cType ) ? GraphicType::Bitmap : GraphicType::GdiMetafile );
        maMimeType = OUString( pMimeType, strlen(pMimeType), RTL_TEXTENCODING_ASCII_US );
        maSizePixel = aDescriptor.GetSizePixel();
        maSize100thMM = aDescriptor.GetSize_100TH_MM();
        mnBitsPerPixel = aDescriptor.GetBitsPerPixel();
        mbTransparent = ( graphic::GraphicType::VECTOR == cType );
    }
}


uno::Any SAL_CALL GraphicDescriptor::queryAggregation( const uno::Type & rType )
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
        aAny = OWeakAggObject::queryAggregation( rType );

    return aAny;
}


uno::Any SAL_CALL GraphicDescriptor::queryInterface( const uno::Type & rType )
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
{
    return OUString( "com.sun.star.comp.graphic.GraphicDescriptor"  );
}

sal_Bool SAL_CALL GraphicDescriptor::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


uno::Sequence< OUString > SAL_CALL GraphicDescriptor::getSupportedServiceNames()
{
    return { "com.sun.star.graphic.GraphicDescriptor" };
}


uno::Sequence< uno::Type > SAL_CALL GraphicDescriptor::getTypes()
{
    static const uno::Sequence< uno::Type > aTypes {
        cppu::UnoType<uno::XAggregation>::get(),
        cppu::UnoType<lang::XServiceInfo>::get(),
        cppu::UnoType<lang::XTypeProvider>::get(),
        cppu::UnoType<beans::XPropertySet>::get(),
        cppu::UnoType<beans::XPropertyState>::get(),
        cppu::UnoType<beans::XMultiPropertySet>::get() };
    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL GraphicDescriptor::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


rtl::Reference<::comphelper::PropertySetInfo> GraphicDescriptor::createPropertySetInfo()
{
    static ::comphelper::PropertyMapEntry const aEntries[] =
    {
        { OUString( "GraphicType" ), static_cast< sal_Int32 >( UnoGraphicProperty::GraphicType ), cppu::UnoType< sal_Int8 >::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString( "MimeType" ), static_cast< sal_Int32 >( UnoGraphicProperty::MimeType ), cppu::UnoType< OUString >::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString( "SizePixel" ), static_cast< sal_Int32 >( UnoGraphicProperty::SizePixel ), cppu::UnoType< awt::Size >::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString( "Size100thMM" ), static_cast< sal_Int32 >( UnoGraphicProperty::Size100thMM ), cppu::UnoType< awt::Size >::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString( "BitsPerPixel" ), static_cast< sal_Int32 >( UnoGraphicProperty::BitsPerPixel ), cppu::UnoType< sal_uInt8 >::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString( "Transparent" ), static_cast< sal_Int32 >( UnoGraphicProperty::Transparent ), cppu::UnoType< sal_Bool >::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString( "Alpha" ), static_cast< sal_Int32 >( UnoGraphicProperty::Alpha ), cppu::UnoType< sal_Bool >::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString( "Animated" ), static_cast< sal_Int32 >( UnoGraphicProperty::Animated ), cppu::UnoType< sal_Bool >::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("Linked"), sal_Int32(UnoGraphicProperty::Linked), cppu::UnoType<sal_Bool>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("OriginURL"), sal_Int32(UnoGraphicProperty::OriginURL), cppu::UnoType<OUString>::get(), beans::PropertyAttribute::READONLY, 0 },

        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return rtl::Reference<::comphelper::PropertySetInfo>( new ::comphelper::PropertySetInfo(aEntries) );
}


void GraphicDescriptor::_setPropertyValues( const comphelper::PropertyMapEntry** /*ppEntries*/, const uno::Any* /*pValues*/ )
{
    // we only have readonly attributes
}


void GraphicDescriptor::_getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, uno::Any* pValues )
{
    SolarMutexGuard aGuard;

    while( *ppEntries )
    {
        UnoGraphicProperty theProperty = static_cast< UnoGraphicProperty >( (*ppEntries)->mnHandle );
        switch( theProperty )
        {
            case UnoGraphicProperty::GraphicType:
            {
                const GraphicType eType( mpGraphic ? mpGraphic->GetType() : meType );

                *pValues <<= ( eType == GraphicType::Bitmap ? graphic::GraphicType::PIXEL :
                                ( eType == GraphicType::GdiMetafile ? graphic::GraphicType::VECTOR :
                                graphic::GraphicType::EMPTY ) );
            }
            break;

            case UnoGraphicProperty::MimeType:
            {
                OUString aMimeType;

                if( mpGraphic )
                {
                    if( mpGraphic->IsGfxLink() )
                    {
                        const char* pMimeType;

                        switch( mpGraphic->GetGfxLink().GetType() )
                        {
                            case GfxLinkType::NativeGif: pMimeType = MIMETYPE_GIF; break;

                            // #i15508# added BMP type for better exports (checked, works)
                            case GfxLinkType::NativeBmp: pMimeType = MIMETYPE_BMP; break;

                            case GfxLinkType::NativeJpg: pMimeType = MIMETYPE_JPG; break;
                            case GfxLinkType::NativePng: pMimeType = MIMETYPE_PNG; break;
                            case GfxLinkType::NativeWmf: pMimeType = MIMETYPE_WMF; break;
                            case GfxLinkType::NativeMet: pMimeType = MIMETYPE_MET; break;
                            case GfxLinkType::NativePct: pMimeType = MIMETYPE_PCT; break;

                            // added Svg mimetype support
                            case GfxLinkType::NativeSvg: pMimeType = MIMETYPE_SVG; break;
                            case GfxLinkType::NativePdf: pMimeType = MIMETYPE_PDF; break;

                            default:
                                pMimeType = nullptr;
                            break;
                        }

                        if( pMimeType )
                            aMimeType = OUString::createFromAscii( pMimeType );
                    }

                    if( aMimeType.isEmpty() && ( mpGraphic->GetType() != GraphicType::NONE ) )
                        aMimeType = MIMETYPE_VCLGRAPHIC;
                }
                else
                    aMimeType = maMimeType;

                *pValues <<= aMimeType;
            }
            break;

            case UnoGraphicProperty::SizePixel:
            {
                awt::Size aAWTSize( 0, 0 );

                if( mpGraphic )
                {
                    if( mpGraphic->GetType() == GraphicType::Bitmap )
                    {
                        const Size aSizePix( mpGraphic->GetSizePixel() );
                        aAWTSize = awt::Size( aSizePix.Width(), aSizePix.Height() );
                    }
                }
                else
                    aAWTSize = awt::Size( maSizePixel.Width(), maSizePixel.Height() );

                *pValues <<= aAWTSize;
            }
            break;

            case UnoGraphicProperty::Size100thMM:
            {
                awt::Size aAWTSize( 0, 0 );

                if( mpGraphic )
                {
                    if( mpGraphic->GetPrefMapMode().GetMapUnit() != MapUnit::MapPixel )
                    {
                        const Size aSizeLog( OutputDevice::LogicToLogic(
                            mpGraphic->GetPrefSize(),
                            mpGraphic->GetPrefMapMode(),
                            MapMode(MapUnit::Map100thMM)) );
                        aAWTSize = awt::Size( aSizeLog.Width(), aSizeLog.Height() );
                    }
                }
                else
                    aAWTSize = awt::Size( maSize100thMM.Width(), maSize100thMM.Height() );

                *pValues <<= aAWTSize;
            }
            break;

            case UnoGraphicProperty::BitsPerPixel:
            {
                sal_uInt16 nBitsPerPixel = 0;

                if( mpGraphic )
                {
                    if( mpGraphic->GetType() == GraphicType::Bitmap )
                        nBitsPerPixel = mpGraphic->GetBitmapEx().GetBitmap().GetBitCount();
                }
                else
                    nBitsPerPixel = mnBitsPerPixel;

                *pValues <<= sal::static_int_cast< sal_Int8 >(nBitsPerPixel);
            }
            break;

            case UnoGraphicProperty::Transparent:
            {
                *pValues <<= mpGraphic ? mpGraphic->IsTransparent() : mbTransparent;
            }
            break;

            case UnoGraphicProperty::Alpha:
            {
                *pValues <<= mpGraphic && mpGraphic->IsAlpha();
            }
            break;

            case UnoGraphicProperty::Animated:
            {
                *pValues <<= mpGraphic && mpGraphic->IsAnimated();
            }
            break;

            case UnoGraphicProperty::Linked:
            {
                *pValues <<= mpGraphic && !mpGraphic->getOriginURL().isEmpty();
            }
            break;

            case UnoGraphicProperty::OriginURL:
            {
                OUString aOriginURL;
                if (mpGraphic)
                    aOriginURL = mpGraphic->getOriginURL();

                *pValues <<= aOriginURL;
            }
            break;
        }

        ++ppEntries;
        ++pValues;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
