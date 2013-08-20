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

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>

#include "vcl/graph.hxx"
#include "vcl/svapp.hxx"

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

// ---------------------
// - GraphicDescriptor -
// ---------------------

GraphicDescriptor::GraphicDescriptor() :
    ::comphelper::PropertySetHelper( createPropertySetInfo(), SAL_NO_ACQUIRE ),
    mpGraphic( NULL ),
    meType( GRAPHIC_NONE ),
    mnBitsPerPixel ( 0 ),
    mbTransparent ( false ),
    mbAlpha( false ),
    mbAnimated( false )
{
}

// ------------------------------------------------------------------------------

GraphicDescriptor::~GraphicDescriptor()
    throw()
{
}

// ------------------------------------------------------------------------------

void GraphicDescriptor::init( const ::Graphic& rGraphic )
    throw()
{
    mpGraphic = &rGraphic;
}

// ------------------------------------------------------------------------------

void GraphicDescriptor::init( const OUString& rURL )
    throw()
{
    SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( rURL, STREAM_READ );

    if( pIStm )
    {
        implCreate( *pIStm, &rURL );
        delete pIStm;
    }
}

// ------------------------------------------------------------------------------

void GraphicDescriptor::init( const uno::Reference< io::XInputStream >& rxIStm, const OUString& rURL )
    throw()
{
    SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( rxIStm );

    if( pIStm )
    {
        implCreate( *pIStm, &rURL );
        delete pIStm;
    }
}

// ------------------------------------------------------------------------------

void GraphicDescriptor::implCreate( SvStream& rIStm, const OUString* pURL )
{
    OUString aURL;
    if( pURL )
        aURL = *pURL;
    ::GraphicDescriptor aDescriptor( rIStm, &aURL );

    mpGraphic = NULL;
    maMimeType = OUString();
    meType = GRAPHIC_NONE;
    mnBitsPerPixel = 0;
    mbTransparent = false;

    if( aDescriptor.Detect( true ) && aDescriptor.GetFileFormat() != GFF_NOT )
    {
        const char*             pMimeType = NULL;
        sal_uInt8               cType = graphic::GraphicType::EMPTY;

        switch( aDescriptor.GetFileFormat() )
        {
            case( GFF_BMP ): pMimeType = MIMETYPE_BMP; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_GIF ): pMimeType = MIMETYPE_GIF; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_JPG ): pMimeType = MIMETYPE_JPG; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_PCD ): pMimeType = MIMETYPE_PCD; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_PCX ): pMimeType = MIMETYPE_PCX; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_PNG ): pMimeType = MIMETYPE_PNG; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_TIF ): pMimeType = MIMETYPE_TIF; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_XBM ): pMimeType = MIMETYPE_XBM; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_XPM ): pMimeType = MIMETYPE_XPM; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_PBM ): pMimeType = MIMETYPE_PBM; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_PGM ): pMimeType = MIMETYPE_PGM; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_PPM ): pMimeType = MIMETYPE_PPM; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_RAS ): pMimeType = MIMETYPE_RAS; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_TGA ): pMimeType = MIMETYPE_TGA; cType = graphic::GraphicType::PIXEL; break;
            case( GFF_PSD ): pMimeType = MIMETYPE_PSD; cType = graphic::GraphicType::PIXEL; break;

            case( GFF_EPS ): pMimeType = MIMETYPE_EPS; cType = graphic::GraphicType::VECTOR; break;
            case( GFF_DXF ): pMimeType = MIMETYPE_DXF; cType = graphic::GraphicType::VECTOR; break;
            case( GFF_MET ): pMimeType = MIMETYPE_MET; cType = graphic::GraphicType::VECTOR; break;
            case( GFF_PCT ): pMimeType = MIMETYPE_PCT; cType = graphic::GraphicType::VECTOR; break;
            case( GFF_SGF ): pMimeType = MIMETYPE_SGF; cType = graphic::GraphicType::VECTOR; break;
            case( GFF_SVM ): pMimeType = MIMETYPE_SVM; cType = graphic::GraphicType::VECTOR; break;
            case( GFF_WMF ): pMimeType = MIMETYPE_WMF; cType = graphic::GraphicType::VECTOR; break;
            case( GFF_SGV ): pMimeType = MIMETYPE_SGV; cType = graphic::GraphicType::VECTOR; break;
            case( GFF_EMF ): pMimeType = MIMETYPE_EMF; cType = graphic::GraphicType::VECTOR; break;
            case( GFF_SVG ): pMimeType = MIMETYPE_SVG; cType = graphic::GraphicType::VECTOR; break;

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

// ------------------------------------------------------------------------------

OUString GraphicDescriptor::getImplementationName_Static()
    throw()
{
    return OUString( "com.sun.star.comp.graphic.GraphicDescriptor"  );
}

// ------------------------------------------------------------------------------

uno::Sequence< OUString > GraphicDescriptor::getSupportedServiceNames_Static()
    throw(  )
{
    uno::Sequence< OUString > aSeq( 1 );

    aSeq.getArray()[ 0 ] = OUString( "com.sun.star.graphic.GraphicDescriptor"  );

    return aSeq;
}

// ------------------------------------------------------------------------------

uno::Any SAL_CALL GraphicDescriptor::queryAggregation( const uno::Type & rType )
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
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

    return aAny;
}

// ------------------------------------------------------------------------------

uno::Any SAL_CALL GraphicDescriptor::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    return OWeakAggObject::queryInterface( rType );
}

// ------------------------------------------------------------------------------

void SAL_CALL GraphicDescriptor::acquire()
    throw()
{
    OWeakAggObject::acquire();
}

// ------------------------------------------------------------------------------

void SAL_CALL GraphicDescriptor::release()
    throw()
{
    OWeakAggObject::release();
}

// ------------------------------------------------------------------------------

OUString SAL_CALL GraphicDescriptor::getImplementationName()
    throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GraphicDescriptor::supportsService( const OUString& ServiceName )
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

uno::Sequence< OUString > SAL_CALL GraphicDescriptor::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

// ------------------------------------------------------------------------------

uno::Sequence< uno::Type > SAL_CALL GraphicDescriptor::getTypes()
    throw( uno::RuntimeException )
{
    uno::Sequence< uno::Type >  aTypes( 6 );
    uno::Type*                  pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< uno::XAggregation>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet>*)0);

    return aTypes;
}

namespace
{
    class theGraphicDescriptorUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theGraphicDescriptorUnoTunnelId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL GraphicDescriptor::getImplementationId()
    throw( uno::RuntimeException )
{
    return theGraphicDescriptorUnoTunnelId::get().getSeq();
}

// ------------------------------------------------------------------------------

::comphelper::PropertySetInfo* GraphicDescriptor::createPropertySetInfo()
{
    SolarMutexGuard aGuard;
    ::comphelper::PropertySetInfo*  pRet = new ::comphelper::PropertySetInfo();

    static ::comphelper::PropertyMapEntry aEntries[] =
    {
        { MAP_CHAR_LEN( "GraphicType" ), UNOGRAPHIC_GRAPHICTYPE, &::getCppuType( (const sal_Int8*)(0)), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN( "MimeType" ), UNOGRAPHIC_MIMETYPE, &::getCppuType( (const OUString*)(0)), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN( "SizePixel" ), UNOGRAPHIC_SIZEPIXEL, &::getCppuType( (const awt::Size*)(0)), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN( "Size100thMM" ), UNOGRAPHIC_SIZE100THMM,    &::getCppuType( (const awt::Size*)(0)), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN( "BitsPerPixel" ), UNOGRAPHIC_BITSPERPIXEL, &::getCppuType( (const sal_uInt8*)(0)), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN( "Transparent" ), UNOGRAPHIC_TRANSPARENT, &::getCppuType( (const sal_Bool*)(0)), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN( "Alpha" ), UNOGRAPHIC_ALPHA, &::getCppuType( (const sal_Bool*)(0)), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN( "Animated" ), UNOGRAPHIC_ANIMATED, &::getCppuType( (const sal_Bool*)(0)), beans::PropertyAttribute::READONLY, 0 },

        { 0,0,0,0,0,0 }
    };

    pRet->acquire();
    pRet->add( aEntries );

    return pRet;
}

// ------------------------------------------------------------------------------

void GraphicDescriptor::_setPropertyValues( const comphelper::PropertyMapEntry** /*ppEntries*/, const uno::Any* /*pValues*/ )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
              lang::WrappedTargetException )
{
    // we only have readonly attributes
}

// ------------------------------------------------------------------------------

void GraphicDescriptor::_getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, uno::Any* pValues )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException )
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

                        switch( const_cast< Graphic* >( mpGraphic )->GetLink().GetType() )
                        {
                            case( GFX_LINK_TYPE_NATIVE_GIF ): pMimeType = MIMETYPE_GIF; break;
                            case( GFX_LINK_TYPE_NATIVE_JPG ): pMimeType = MIMETYPE_JPG; break;
                            case( GFX_LINK_TYPE_NATIVE_PNG ): pMimeType = MIMETYPE_PNG; break;
                            case( GFX_LINK_TYPE_NATIVE_WMF ): pMimeType = MIMETYPE_WMF; break;
                            case( GFX_LINK_TYPE_NATIVE_MET ): pMimeType = MIMETYPE_MET; break;
                            case( GFX_LINK_TYPE_NATIVE_PCT ): pMimeType = MIMETYPE_PCT; break;

                            // added Svg mimetype support
                            case( GFX_LINK_TYPE_NATIVE_SVG ): pMimeType = MIMETYPE_SVG; break;

                            default:
                                pMimeType = NULL;
                            break;
                        }

                        if( pMimeType )
                            aMimeType = OUString::createFromAscii( pMimeType );
                    }

                    if( aMimeType.isEmpty() && ( mpGraphic->GetType() != GRAPHIC_NONE ) )
                        aMimeType = OUString(MIMETYPE_VCLGRAPHIC );
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
                *pValues <<= static_cast< sal_Bool >( mpGraphic ? mpGraphic->IsTransparent() : mbTransparent );
            }
            break;

            case( UNOGRAPHIC_ALPHA ):
            {
                *pValues <<= static_cast< sal_Bool >( mpGraphic ? mpGraphic->IsAlpha() : mbAlpha );
            }
            break;

            case( UNOGRAPHIC_ANIMATED ):
            {
                *pValues <<= static_cast< sal_Bool >( mpGraphic ? mpGraphic->IsAnimated() : mbAnimated );
            }
            break;
        }

        ++ppEntries;
        ++pValues;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
