/*************************************************************************
 *
 *  $RCSfile: provider.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-30 10:22:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX_
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_IMAGE_HXX_
#include <vcl/image.hxx>
#endif
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <tools/resmgr.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/filter.hxx>
#include <svtools/solar.hrc>

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif

#include "descriptor.hxx"
#include "graphic.hxx"
#include "provider.hxx"

using namespace com::sun::star;

namespace unographic {

// -------------------
// - GraphicProvider -
// -------------------

GraphicProvider::GraphicProvider()
{
}

// ------------------------------------------------------------------------------

GraphicProvider::~GraphicProvider()
{
}

// ------------------------------------------------------------------------------

::rtl::OUString GraphicProvider::getImplementationName_Static()
    throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.graphic.GraphicProvider" ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > GraphicProvider::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence< ::rtl::OUString > aSeq( 1 );

    aSeq.getArray()[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.graphic.GraphicProvider" ) );

    return aSeq;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL GraphicProvider::getImplementationName()
    throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL GraphicProvider::supportsService( const ::rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString >    aSNL( getSupportedServiceNames() );
    const ::rtl::OUString*              pArray = aSNL.getConstArray();

    for( int i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL GraphicProvider::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

// ------------------------------------------------------------------------------

uno::Sequence< uno::Type > SAL_CALL GraphicProvider::getTypes()
    throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type >  aTypes( 3 );
    uno::Type*                  pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< graphic::XGraphicProvider>*)0);

    return aTypes;
}

// ------------------------------------------------------------------------------

uno::Sequence< sal_Int8 > SAL_CALL GraphicProvider::getImplementationId()
    throw(uno::RuntimeException)
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

uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadMemory( const ::rtl::OUString& rResourceURL ) const
{
    uno::Reference< ::graphic::XGraphic >   xRet;
    sal_Int32                               nIndex = 0;

    if( ( 0 == rResourceURL.getToken( 0, '/', nIndex ).compareToAscii( "private:memorygraphic" ) ) )
    {
        sal_Int64 nGraphicAddress = rResourceURL.getToken( 0, '/', nIndex ).toInt64();

        if( nGraphicAddress )
        {
            ::unographic::Graphic* pUnoGraphic = new ::unographic::Graphic;

            pUnoGraphic->init( *reinterpret_cast< ::Graphic* >( nGraphicAddress ) );
            xRet = pUnoGraphic;
        }
    }

    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadResource( const ::rtl::OUString& rResourceURL ) const
{
    uno::Reference< ::graphic::XGraphic >   xRet;
    sal_Int32                               nIndex = 0;

    if( ( 0 == rResourceURL.getToken( 0, '/', nIndex ).compareToAscii( "private:resource" ) ) )
    {
        ByteString aResMgrName( String( rResourceURL.getToken( 0, '/', nIndex ) ), RTL_TEXTENCODING_ASCII_US );

        aResMgrName += ByteString::CreateFromInt32( SOLARUPD );
        ResMgr* pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

        if( pResMgr )
        {
            const ::rtl::OUString   aResourceType( rResourceURL.getToken( 0, '/', nIndex ) );
            const ResId             aResId( rResourceURL.getToken( 0, '/', nIndex ).toInt32(), pResMgr );

            if( aResourceType.getLength() )
            {
                BitmapEx aBmpEx;

                if( ( 0 == aResourceType.compareToAscii( "bitmap" ) ) ||
                    ( 0 == aResourceType.compareToAscii( "bitmapex" ) ) )
                {
                    aResId.SetRT( RSC_BITMAP );

                    if( pResMgr->IsAvailable( aResId ) )
                    {
                        aBmpEx = BitmapEx( aResId );
                    }
                }
                else if( 0 == aResourceType.compareToAscii( "image" ) )
                {
                    aResId.SetRT( RSC_IMAGE );

                    if( pResMgr->IsAvailable( aResId ) )
                    {
                        const Image aImage( aResId );
                        aBmpEx = aImage.GetBitmapEx();
                    }
                }
                else if( 0 == aResourceType.compareToAscii( "imagelist" ) )
                {
                    aResId.SetRT( RSC_IMAGELIST );

                    if( pResMgr->IsAvailable( aResId ) )
                    {
                        const ImageList aImageList( aResId );
                        sal_Int32       nImageId = ( nIndex > -1 ) ? rResourceURL.getToken( 0, '/', nIndex ).toInt32() : 0;

                        if( 0 < nImageId )
                        {
                            const Image aImage( aImageList.GetImage( nImageId ) );
                            aBmpEx = aImage.GetBitmapEx();
                        }
                        else
                        {
                            aBmpEx = aImageList.GetBitmapEx();
                        }
                    }
                }

                if( !aBmpEx.IsEmpty() )
                {
                    ::unographic::Graphic* pUnoGraphic = new ::unographic::Graphic;

                    pUnoGraphic->init( aBmpEx );
                    xRet = pUnoGraphic;
                }
            }

            delete pResMgr;
        }
    }

    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< beans::XPropertySet > SAL_CALL GraphicProvider::queryGraphicDescriptor( const uno::Sequence< beans::PropertyValue >& rMediaProperties )
    throw ( io::IOException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xRet;

    for( sal_Int32 i = 0; ( i < rMediaProperties.getLength() ) && !xRet.is(); ++i )
    {
        const ::rtl::OUString   aName( rMediaProperties[ i ].Name );
        const uno::Any          aValue( rMediaProperties[ i ].Value );

        if( COMPARE_EQUAL == aName.compareToAscii( "URL" ) )
        {
            ::rtl::OUString aURL;

            if( ( aValue >>= aURL ) && aURL.getLength() )
            {
                uno::Reference< ::graphic::XGraphic > xGraphic( implLoadMemory( aURL ) );

                if( !xGraphic.is() )
                    xGraphic = implLoadResource( aURL );

                if( xGraphic.is() )
                    xRet = uno::Reference< beans::XPropertySet >( xGraphic, uno::UNO_QUERY );
                else
                {
                    GraphicDescriptor* pDescriptor = new GraphicDescriptor;
                    pDescriptor->init( aURL );
                    xRet = pDescriptor;
                }
            }
        }
        else if( COMPARE_EQUAL == aName.compareToAscii( "InputStream" ) )
        {
            uno::Reference< io::XInputStream > xIStm;

            if( ( aValue >>= xIStm ) && xIStm.is() )
            {
                GraphicDescriptor* pDescriptor = new GraphicDescriptor;
                pDescriptor->init( xIStm );
                xRet = pDescriptor;
            }
        }
    }

    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< ::graphic::XGraphic > SAL_CALL GraphicProvider::queryGraphic( const uno::Sequence< ::beans::PropertyValue >& rMediaProperties )
    throw ( io::IOException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference< ::graphic::XGraphic >   xRet;
    String                                  aPath;
    SvStream*                               pIStm = NULL;

    for( sal_Int32 i = 0; ( i < rMediaProperties.getLength() ) && !pIStm && !xRet.is(); ++i )
    {
        const ::rtl::OUString   aName( rMediaProperties[ i ].Name );
        const uno::Any          aValue( rMediaProperties[ i ].Value );

        if( COMPARE_EQUAL == aName.compareToAscii( "URL" ) )
        {
            ::rtl::OUString aURL;

            if( ( aValue >>= aURL ) && aURL.getLength() )
            {
                xRet = implLoadMemory( aURL );

                if( !xRet.is() )
                    xRet = implLoadResource( aURL );

                if( !xRet.is() )
                {
                    pIStm = ::utl::UcbStreamHelper::CreateStream( aURL, STREAM_READ );

                    if( pIStm )
                        aPath = aURL;
                }
            }
        }
        else if( COMPARE_EQUAL == aName.compareToAscii( "InputStream" ) )
        {
            uno::Reference< io::XInputStream > xIStm;

            if( ( aValue >>= xIStm ) && xIStm.is() )
                pIStm = ::utl::UcbStreamHelper::CreateStream( xIStm );
        }
    }

    if( pIStm )
    {
        ::GraphicFilter* pFilter = ::GraphicFilter::GetGraphicFilter();

        if( pFilter )
        {
            ::Graphic aVCLGraphic;

            if( ( pFilter->ImportGraphic( aVCLGraphic, aPath, *pIStm ) == GRFILTER_OK ) &&
                ( aVCLGraphic.GetType() != GRAPHIC_NONE ) )
            {
                ::unographic::Graphic* pUnoGraphic = new ::unographic::Graphic;

                pUnoGraphic->init( aVCLGraphic );
                xRet = pUnoGraphic;
            }
        }

        delete pIStm;
    }

    return xRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL GraphicProvider::storeGraphic( const uno::Reference< ::graphic::XGraphic >& rxGraphic, const uno::Sequence< beans::PropertyValue >& rMediaProperties )
    throw ( io::IOException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    SvStream*   pOStm = NULL;
    String      aPath;
    sal_Int32   i;

    for( i = 0; ( i < rMediaProperties.getLength() ) && !pOStm; ++i )
    {
        const ::rtl::OUString   aName( rMediaProperties[ i ].Name );
        const uno::Any          aValue( rMediaProperties[ i ].Value );

        if( COMPARE_EQUAL == aName.compareToAscii( "URL" ) )
        {
            ::rtl::OUString aURL;

            aValue >>= aURL;
            pOStm = ::utl::UcbStreamHelper::CreateStream( aURL, STREAM_WRITE | STREAM_TRUNC );
            aPath = aURL;
        }
        else if( COMPARE_EQUAL == aName.compareToAscii( "OutputStream" ) )
        {
            uno::Reference< io::XStream > xOStm;

            aValue >>= xOStm;

            if( xOStm.is() )
                pOStm = ::utl::UcbStreamHelper::CreateStream( xOStm );
        }
    }

    if( pOStm )
    {
        uno::Sequence< beans::PropertyValue >   aFilterDataSeq;
        const char*                             pFilterShortName = NULL;

        for( i = 0; i < rMediaProperties.getLength(); ++i )
        {
            const ::rtl::OUString   aName( rMediaProperties[ i ].Name );
            const uno::Any          aValue( rMediaProperties[ i ].Value );

            if( COMPARE_EQUAL == aName.compareToAscii( "FilterData" ) )
            {
                aValue >>= aFilterDataSeq;
            }
            else if( COMPARE_EQUAL == aName.compareToAscii( "MimeType" ) )
            {
                ::rtl::OUString aMimeType;

                aValue >>= aMimeType;

                if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_BMP ) )
                    pFilterShortName = "bmp";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_EPS ) )
                    pFilterShortName = "eps";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_GIF ) )
                    pFilterShortName = "gif";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_JPG ) )
                    pFilterShortName = "jpg";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_MET ) )
                    pFilterShortName = "met";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_PNG ) )
                    pFilterShortName = "png";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_PCT ) )
                    pFilterShortName = "pct";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_PBM ) )
                    pFilterShortName = "pbm";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_PGM ) )
                    pFilterShortName = "pgm";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_PPM ) )
                    pFilterShortName = "ppm";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_RAS ) )
                    pFilterShortName = "ras";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_SVM ) )
                    pFilterShortName = "svm";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_TIF ) )
                    pFilterShortName = "tif";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_EMF ) )
                    pFilterShortName = "emf";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_WMF ) )
                    pFilterShortName = "wmf";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_XPM ) )
                    pFilterShortName = "xpm";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_SVG ) )
                    pFilterShortName = "svg";
                else if( COMPARE_EQUAL == aMimeType.compareToAscii( MIMETYPE_VCLGRAPHIC ) )
                    pFilterShortName = MIMETYPE_VCLGRAPHIC;
            }
        }

        if( pFilterShortName )
        {
            ::GraphicFilter* pFilter = ::GraphicFilter::GetGraphicFilter();

            if( pFilter )
            {
                const uno::Reference< XInterface >  xIFace( rxGraphic, uno::UNO_QUERY );
                const ::Graphic*                    pGraphic = ::unographic::Graphic::getImplementation( xIFace );

                if( pGraphic && ( pGraphic->GetType() != GRAPHIC_NONE ) )
                {
                    if( 0 == strcmp( pFilterShortName, MIMETYPE_VCLGRAPHIC ) )
                        (*pOStm) << *pGraphic;
                    else
                    {
                        pFilter->ExportGraphic( *pGraphic, aPath, *pOStm,
                                                pFilter->GetExportFormatNumberForShortName( ::rtl::OUString::createFromAscii( pFilterShortName ) ),
                                                true, ( aFilterDataSeq.getLength() ? &aFilterDataSeq : NULL ) );
                    }
                }
            }
        }

        delete pOStm;
    }
}

}
