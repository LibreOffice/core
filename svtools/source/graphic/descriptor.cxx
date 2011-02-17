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

#include "descriptor.hxx"

#include <rtl/uuid.h>
#include <vos/mutex.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/filter.hxx>
#include <svl/itemprop.hxx>

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

void GraphicDescriptor::init( const ::rtl::OUString& rURL )
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

void GraphicDescriptor::init( const uno::Reference< io::XInputStream >& rxIStm, const ::rtl::OUString& rURL )
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

bool GraphicDescriptor::isValid() const
{
    return( mpGraphic ? ( mpGraphic->GetType() != GRAPHIC_NONE ) : ( meType != GRAPHIC_NONE ) );
}

// ------------------------------------------------------------------------------

void GraphicDescriptor::implCreate( SvStream& rIStm, const ::rtl::OUString* pURL )
{
    String aURL;
    if( pURL )
        aURL = *pURL;
    ::GraphicDescriptor aDescriptor( rIStm, &aURL );

    mpGraphic = NULL;
    maMimeType = ::rtl::OUString();
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
            maMimeType = String( pMimeType, RTL_TEXTENCODING_ASCII_US );
            maSizePixel = aDescriptor.GetSizePixel();
            maSize100thMM = aDescriptor.GetSize_100TH_MM();
            mnBitsPerPixel = aDescriptor.GetBitsPerPixel();
            mbTransparent = ( graphic::GraphicType::VECTOR == cType );
            mbAlpha = mbAnimated = false;
        }
    }
}

// ------------------------------------------------------------------------------

::rtl::OUString GraphicDescriptor::getImplementationName_Static()
    throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.graphic.GraphicDescriptor" ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > GraphicDescriptor::getSupportedServiceNames_Static()
    throw(  )
{
    uno::Sequence< ::rtl::OUString > aSeq( 1 );

    aSeq.getArray()[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.graphic.GraphicDescriptor" ) );

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

::rtl::OUString SAL_CALL GraphicDescriptor::getImplementationName()
    throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GraphicDescriptor::supportsService( const rtl::OUString& ServiceName )
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

uno::Sequence< rtl::OUString > SAL_CALL GraphicDescriptor::getSupportedServiceNames()
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

// ------------------------------------------------------------------------------

uno::Sequence< sal_Int8 > SAL_CALL GraphicDescriptor::getImplementationId()
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

::comphelper::PropertySetInfo* GraphicDescriptor::createPropertySetInfo()
{
    vos::OGuard                     aGuard( Application::GetSolarMutex() );
    ::comphelper::PropertySetInfo*  pRet = new ::comphelper::PropertySetInfo();

    static ::comphelper::PropertyMapEntry aEntries[] =
    {
        { MAP_CHAR_LEN( "GraphicType" ), UNOGRAPHIC_GRAPHICTYPE, &::getCppuType( (const sal_Int8*)(0)), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN( "MimeType" ), UNOGRAPHIC_MIMETYPE, &::getCppuType( (const ::rtl::OUString*)(0)), beans::PropertyAttribute::READONLY, 0 },
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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
                ::rtl::OUString aMimeType;

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
                            case( GFX_LINK_TYPE_NATIVE_PCT ): pMimeType = MIMETYPE_PCT ; break;

                            default:
                                pMimeType = NULL;
                            break;
                        }

                        if( pMimeType )
                            aMimeType = ::rtl::OUString::createFromAscii( pMimeType );
                    }

                    if( !aMimeType.getLength() && ( mpGraphic->GetType() != GRAPHIC_NONE ) )
                        aMimeType = ::rtl::OUString::createFromAscii( MIMETYPE_VCLGRAPHIC );
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
