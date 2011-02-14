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

#include <rtl/uuid.h>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/metaact.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/imagerepository.hxx>
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <tools/resmgr.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/filter.hxx>
#include <svl/solar.hrc>
#include <vcl/salbtype.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

#include "descriptor.hxx"
#include "graphic.hxx"
#include <svtools/grfmgr.hxx>
#include "provider.hxx"

using namespace com::sun::star;

namespace unographic {

#define UNO_NAME_GRAPHOBJ_URLPREFIX                             "vnd.sun.star.GraphicObject:"

// -------------------
// - GraphicProvider -
// -------------------

uno::Reference< uno::XInterface > SAL_CALL GraphicProvider_CreateInstance( const uno::Reference< lang::XMultiServiceFactory >& )
{
    return SAL_STATIC_CAST( ::cppu::OWeakObject*, new GraphicProvider );
}

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

uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadGraphicObject( const ::rtl::OUString& rResourceURL ) const
{
    uno::Reference< ::graphic::XGraphic >   xRet;
    if( rResourceURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPREFIX ) ) == 0 )
    {
        // graphic manager url
        String aTmpStr( rResourceURL.copy( sizeof( UNO_NAME_GRAPHOBJ_URLPREFIX ) - 1 ) );
        ByteString aUniqueID( aTmpStr, RTL_TEXTENCODING_UTF8 );
        GraphicObject aGrafObj( aUniqueID );
        // I don't call aGrafObj.GetXGraphic because it will call us back
        // into implLoadMemory ( with "private:memorygraphic" test )
        ::unographic::Graphic* pUnoGraphic = new ::unographic::Graphic;
        pUnoGraphic->init( aGrafObj.GetGraphic() );
        xRet = pUnoGraphic;
    }
    return xRet;
}

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

uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadRepositoryImage( const ::rtl::OUString& rResourceURL ) const
{
    uno::Reference< ::graphic::XGraphic >   xRet;
    sal_Int32                               nIndex = 0;

    if( ( 0 == rResourceURL.getToken( 0, '/', nIndex ).compareToAscii( "private:graphicrepository" ) ) )
    {
        String sPathName( rResourceURL.copy( nIndex ) );
        BitmapEx aBitmap;
        if ( ::vcl::ImageRepository::loadImage( sPathName, aBitmap, false ) )
        {
            Image aImage( aBitmap );
            xRet = aImage.GetXGraphic();
        }
    }
    return xRet;
}


// ------------------------------------------------------------------------------

uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadStandardImage( const ::rtl::OUString& rResourceURL ) const
{
    uno::Reference< ::graphic::XGraphic >   xRet;
    sal_Int32                               nIndex = 0;

    if( ( 0 == rResourceURL.getToken( 0, '/', nIndex ).compareToAscii( "private:standardimage" ) ) )
    {
        rtl::OUString sImageName( rResourceURL.copy( nIndex ) );
        if ( sImageName.equalsAscii( "info" ) )
        {
            xRet = InfoBox::GetStandardImage().GetXGraphic();
        }
        else if ( sImageName.equalsAscii( "warning" ) )
        {
            xRet = WarningBox::GetStandardImage().GetXGraphic();
        }
        else if ( sImageName.equalsAscii( "error" ) )
        {
            xRet = ErrorBox::GetStandardImage().GetXGraphic();
        }
        else if ( sImageName.equalsAscii( "query" ) )
        {
            xRet = QueryBox::GetStandardImage().GetXGraphic();
        }
    }
    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadBitmap( const uno::Reference< awt::XBitmap >& xBtm ) const
{
    uno::Reference< ::graphic::XGraphic > xRet;
    uno::Sequence< sal_Int8 > aBmpSeq( xBtm->getDIB() );
    uno::Sequence< sal_Int8 > aMaskSeq( xBtm->getMaskDIB() );
    SvMemoryStream aBmpStream( aBmpSeq.getArray(), aBmpSeq.getLength(), STREAM_READ );
    Bitmap aBmp;
    aBmpStream >> aBmp;

    BitmapEx aBmpEx;

    if( aMaskSeq.getLength() )
    {
        SvMemoryStream aMaskStream( aMaskSeq.getArray(), aMaskSeq.getLength(), STREAM_READ );
        Bitmap aMask;
        aMaskStream >> aMask;
        aBmpEx = BitmapEx( aBmp, aMask );
    }
    else
        aBmpEx = BitmapEx( aBmp );

    if( !aBmpEx.IsEmpty() )
    {
        ::unographic::Graphic* pUnoGraphic = new ::unographic::Graphic;

        pUnoGraphic->init( aBmpEx );
        xRet = pUnoGraphic;
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

        ResMgr* pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

        if( pResMgr )
        {
            const ::rtl::OUString   aResourceType( rResourceURL.getToken( 0, '/', nIndex ) );
            const ResId             aResId( rResourceURL.getToken( 0, '/', nIndex ).toInt32(), *pResMgr );

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
                            const Image aImage( aImageList.GetImage( sal::static_int_cast< sal_uInt16 >(nImageId) ) );
                            aBmpEx = aImage.GetBitmapEx();
                        }
                        else
                        {
                            aBmpEx = aImageList.GetAsHorizontalStrip();
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

    ::rtl::OUString aURL;
    uno::Reference< io::XInputStream > xIStm;
    uno::Reference< awt::XBitmap >xBtm;

    for( sal_Int32 i = 0; ( i < rMediaProperties.getLength() ) && !xRet.is(); ++i )
    {
        const ::rtl::OUString   aName( rMediaProperties[ i ].Name );
        const uno::Any          aValue( rMediaProperties[ i ].Value );

        if( COMPARE_EQUAL == aName.compareToAscii( "URL" ) )
        {
            aValue >>= aURL;
        }
        else if( COMPARE_EQUAL == aName.compareToAscii( "InputStream" ) )
        {
            aValue >>= xIStm;
        }
        else if( COMPARE_EQUAL == aName.compareToAscii( "Bitmap" ) )
        {
            aValue >>= xBtm;
        }
    }

    if( xIStm.is() )
    {
        GraphicDescriptor* pDescriptor = new GraphicDescriptor;
        pDescriptor->init( xIStm, aURL );
        xRet = pDescriptor;
    }
    else if( aURL.getLength() )
    {
        uno::Reference< ::graphic::XGraphic > xGraphic( implLoadMemory( aURL ) );
        if( !xGraphic.is() )
            xGraphic = implLoadResource( aURL );
        if( !xGraphic.is() )
            xGraphic = implLoadGraphicObject( aURL );

        if ( !xGraphic.is() )
            xGraphic = implLoadRepositoryImage( aURL );

        if ( !xGraphic.is() )
            xGraphic = implLoadStandardImage( aURL );

        if( xGraphic.is() )
        {
            xRet = uno::Reference< beans::XPropertySet >( xGraphic, uno::UNO_QUERY );
        }
        else
        {
            GraphicDescriptor* pDescriptor = new GraphicDescriptor;
            pDescriptor->init( aURL );
            xRet = pDescriptor;
        }
    }
    else if( xBtm.is() )
    {
        uno::Reference< ::graphic::XGraphic > xGraphic( implLoadBitmap( xBtm ) );
        if( xGraphic.is() )
            xRet = uno::Reference< beans::XPropertySet >( xGraphic, uno::UNO_QUERY );
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

    uno::Reference< io::XInputStream > xIStm;
    uno::Reference< awt::XBitmap >xBtm;

    for( sal_Int32 i = 0; ( i < rMediaProperties.getLength() ) && !pIStm && !xRet.is(); ++i )
    {
        const ::rtl::OUString   aName( rMediaProperties[ i ].Name );
        const uno::Any          aValue( rMediaProperties[ i ].Value );

        if( COMPARE_EQUAL == aName.compareToAscii( "URL" ) )
        {
            ::rtl::OUString aURL;
            aValue >>= aURL;
            aPath = aURL;
        }
        else if( COMPARE_EQUAL == aName.compareToAscii( "InputStream" ) )
        {
            aValue >>= xIStm;
        }
        else if( COMPARE_EQUAL == aName.compareToAscii( "Bitmap" ) )
        {
            aValue >>= xBtm;
        }
    }

    if( xIStm.is() )
    {
        pIStm = ::utl::UcbStreamHelper::CreateStream( xIStm );
    }
    else if( aPath.Len() )
    {
        xRet = implLoadMemory( aPath );

        if( !xRet.is() )
            xRet = implLoadGraphicObject( aPath );

        if( !xRet.is() )
            xRet = implLoadResource( aPath );

        if ( !xRet.is() )
            xRet = implLoadRepositoryImage( aPath );

        if ( !xRet.is() )
            xRet = implLoadStandardImage( aPath );

        if( !xRet.is() )
            pIStm = ::utl::UcbStreamHelper::CreateStream( aPath, STREAM_READ );
    }
    else if( xBtm.is() )
    {
        xRet = implLoadBitmap( xBtm );
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

void ImplCalculateCropRect( ::Graphic& rGraphic, const text::GraphicCrop& rGraphicCropLogic, Rectangle& rGraphicCropPixel )
{
    if ( rGraphicCropLogic.Left || rGraphicCropLogic.Top || rGraphicCropLogic.Right || rGraphicCropLogic.Bottom )
    {
        Size aSourceSizePixel( rGraphic.GetSizePixel() );
        if ( aSourceSizePixel.Width() && aSourceSizePixel.Height() )
        {
            if ( rGraphicCropLogic.Left || rGraphicCropLogic.Top || rGraphicCropLogic.Right || rGraphicCropLogic.Bottom )
            {
                Size aSize100thMM( 0, 0 );
                if( rGraphic.GetPrefMapMode().GetMapUnit() != MAP_PIXEL )
                {
                    aSize100thMM = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(), rGraphic.GetPrefMapMode(), MAP_100TH_MM );
                }
                else
                {
                    aSize100thMM = Application::GetDefaultDevice()->PixelToLogic( rGraphic.GetPrefSize(), MAP_100TH_MM );
                }
                if ( aSize100thMM.Width() && aSize100thMM.Height() )
                {
                    double fSourceSizePixelWidth = static_cast<double>(aSourceSizePixel.Width());
                    double fSourceSizePixelHeight= static_cast<double>(aSourceSizePixel.Height());
                    rGraphicCropPixel.Left() = static_cast< sal_Int32 >((fSourceSizePixelWidth * rGraphicCropLogic.Left ) / aSize100thMM.Width());
                    rGraphicCropPixel.Top() = static_cast< sal_Int32 >((fSourceSizePixelHeight * rGraphicCropLogic.Top ) / aSize100thMM.Height());
                    rGraphicCropPixel.Right() = static_cast< sal_Int32 >(( fSourceSizePixelWidth * ( aSize100thMM.Width() - rGraphicCropLogic.Right ) ) / aSize100thMM.Width() );
                    rGraphicCropPixel.Bottom() = static_cast< sal_Int32 >(( fSourceSizePixelHeight * ( aSize100thMM.Height() - rGraphicCropLogic.Bottom ) ) / aSize100thMM.Height() );
                }
            }
        }
    }
}

void ImplApplyBitmapScaling( ::Graphic& rGraphic, sal_Int32 nPixelWidth, sal_Int32 nPixelHeight )
{
    if ( nPixelWidth && nPixelHeight )
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );
        MapMode aPrefMapMode( aBmpEx.GetPrefMapMode() );
        Size    aPrefSize( aBmpEx.GetPrefSize() );
        aBmpEx.Scale( Size( nPixelWidth, nPixelHeight ) );
        aBmpEx.SetPrefMapMode( aPrefMapMode );
        aBmpEx.SetPrefSize( aPrefSize );
        rGraphic = aBmpEx;
    }
}

void ImplApplyBitmapResolution( ::Graphic& rGraphic, sal_Int32 nImageResolution, const Size& rVisiblePixelSize, const awt::Size& rLogicalSize )
{
    if ( nImageResolution && rLogicalSize.Width && rLogicalSize.Height )
    {
        const double fImageResolution = static_cast<double>( nImageResolution );
        const double fSourceDPIX = ( static_cast<double>(rVisiblePixelSize.Width()) * 2540.0 ) / static_cast<double>(rLogicalSize.Width);
        const double fSourceDPIY = ( static_cast<double>(rVisiblePixelSize.Height()) * 2540.0 ) / static_cast<double>(rLogicalSize.Height);
        const sal_Int32 nSourcePixelWidth( rGraphic.GetSizePixel().Width() );
        const sal_Int32 nSourcePixelHeight( rGraphic.GetSizePixel().Height() );
        const double fSourcePixelWidth = static_cast<double>( nSourcePixelWidth );
        const double fSourcePixelHeight= static_cast<double>( nSourcePixelHeight );

        sal_Int32 nDestPixelWidth = nSourcePixelWidth;
        sal_Int32 nDestPixelHeight = nSourcePixelHeight;

        // check, if the bitmap DPI exceeds the maximum DPI
        if( fSourceDPIX > fImageResolution )
        {
            nDestPixelWidth = static_cast<sal_Int32>(( fSourcePixelWidth * fImageResolution ) / fSourceDPIX);
            if ( !nDestPixelWidth || ( nDestPixelWidth > nSourcePixelWidth ) )
                nDestPixelWidth = nSourcePixelWidth;
        }
        if ( fSourceDPIY > fImageResolution )
        {
            nDestPixelHeight= static_cast<sal_Int32>(( fSourcePixelHeight* fImageResolution ) / fSourceDPIY);
            if ( !nDestPixelHeight || ( nDestPixelHeight > nSourcePixelHeight ) )
                nDestPixelHeight = nSourcePixelHeight;
        }
        if ( ( nDestPixelWidth != nSourcePixelWidth ) || ( nDestPixelHeight != nSourcePixelHeight ) )
            ImplApplyBitmapScaling( rGraphic, nDestPixelWidth, nDestPixelHeight );
    }
}

void ImplApplyFilterData( ::Graphic& rGraphic, uno::Sequence< beans::PropertyValue >& rFilterData )
{
    /* this method applies following attributes to the graphic, in the first step the
       cropping area (logical size in 100thmm) is applied, in the second step the resolution
       is applied, in the third step the graphic is scaled to the corresponding pixelsize.
       if a parameter value is zero or not available the corresponding step will be skipped */

    sal_Int32 nPixelWidth = 0;
    sal_Int32 nPixelHeight= 0;
    sal_Int32 nImageResolution = 0;
    awt::Size aLogicalSize( 0, 0 );
    text::GraphicCrop aCropLogic( 0, 0, 0, 0 );
    sal_Bool bRemoveCropArea = sal_True;

    for( sal_Int32 i = 0; i < rFilterData.getLength(); ++i )
    {
        const ::rtl::OUString   aName(  rFilterData[ i ].Name );
        const uno::Any          aValue( rFilterData[ i ].Value );

        if( COMPARE_EQUAL == aName.compareToAscii( "PixelWidth" ) )
            aValue >>= nPixelWidth;
        else if( COMPARE_EQUAL == aName.compareToAscii( "PixelHeight" ) )
            aValue >>= nPixelHeight;
        else if( COMPARE_EQUAL == aName.compareToAscii( "LogicalSize" ) )
            aValue >>= aLogicalSize;
        else if (COMPARE_EQUAL == aName.compareToAscii( "GraphicCropLogic" ) )
            aValue >>= aCropLogic;
        else if (COMPARE_EQUAL == aName.compareToAscii( "RemoveCropArea" ) )
            aValue >>= bRemoveCropArea;
        else if (COMPARE_EQUAL == aName.compareToAscii( "ImageResolution" ) )
            aValue >>= nImageResolution;
    }
    if ( rGraphic.GetType() == GRAPHIC_BITMAP )
    {
        Rectangle aCropPixel( Point( 0, 0 ), rGraphic.GetSizePixel() );
        ImplCalculateCropRect( rGraphic, aCropLogic, aCropPixel );
        if ( bRemoveCropArea )
        {
            BitmapEx aBmpEx( rGraphic.GetBitmapEx() );
            aBmpEx.Crop( aCropPixel );
            rGraphic = aBmpEx;
        }
        Size aVisiblePixelSize( bRemoveCropArea ? rGraphic.GetSizePixel() : aCropPixel.GetSize() );
        ImplApplyBitmapResolution( rGraphic, nImageResolution, aVisiblePixelSize, aLogicalSize );
        ImplApplyBitmapScaling( rGraphic, nPixelWidth, nPixelHeight );
    }
    else if ( ( rGraphic.GetType() == GRAPHIC_GDIMETAFILE ) && nImageResolution )
    {
        VirtualDevice aDummyVDev;
        GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );
        Size aMtfSize( aDummyVDev.LogicToLogic( aMtf.GetPrefSize(), aMtf.GetPrefMapMode(), MAP_100TH_MM ) );
        if ( aMtfSize.Width() && aMtfSize.Height() )
        {
            MapMode aNewMapMode( MAP_100TH_MM );
            aNewMapMode.SetScaleX( static_cast< double >( aLogicalSize.Width ) / static_cast< double >( aMtfSize.Width() ) );
            aNewMapMode.SetScaleY( static_cast< double >( aLogicalSize.Height ) / static_cast< double >( aMtfSize.Height() ) );
            aDummyVDev.EnableOutput( sal_False );
            aDummyVDev.SetMapMode( aNewMapMode );

            for( sal_uInt32 i = 0, nObjCount = aMtf.GetActionCount(); i < nObjCount; i++ )
            {
                MetaAction* pAction = aMtf.GetAction( i );
                switch( pAction->GetType() )
                {
                    // only optimizing common bitmap actions:
                    case( META_MAPMODE_ACTION ):
                    {
                        const_cast< MetaAction* >( pAction )->Execute( &aDummyVDev );
                        break;
                    }
                    case( META_PUSH_ACTION ):
                    {
                        const MetaPushAction* pA = (const MetaPushAction*)pAction;
                        aDummyVDev.Push( pA->GetFlags() );
                        break;
                    }
                    case( META_POP_ACTION ):
                    {
                        aDummyVDev.Pop();
                        break;
                    }
                    case( META_BMPSCALE_ACTION ):
                    case( META_BMPEXSCALE_ACTION ):
                    {
                        BitmapEx aBmpEx;
                        Point aPos;
                        Size aSize;
                        if ( pAction->GetType() == META_BMPSCALE_ACTION )
                        {
                            MetaBmpScaleAction* pScaleAction = dynamic_cast< MetaBmpScaleAction* >( pAction );
                            aBmpEx = pScaleAction->GetBitmap();
                            aPos = pScaleAction->GetPoint();
                            aSize = pScaleAction->GetSize();
                        }
                        else
                        {
                            MetaBmpExScaleAction* pScaleAction = dynamic_cast< MetaBmpExScaleAction* >( pAction );
                            aBmpEx = pScaleAction->GetBitmapEx();
                            aPos = pScaleAction->GetPoint();
                            aSize = pScaleAction->GetSize();
                        }
                        ::Graphic aGraphic( aBmpEx );
                        const Size aSize100thmm( aDummyVDev.LogicToPixel( aSize ) );
                        Size aSize100thmm2( aDummyVDev.PixelToLogic( aSize100thmm, MAP_100TH_MM ) );

                        ImplApplyBitmapResolution( aGraphic, nImageResolution,
                            aGraphic.GetSizePixel(), awt::Size( aSize100thmm2.Width(), aSize100thmm2.Height() ) );

                        MetaAction* pNewAction;
                        if ( pAction->GetType() == META_BMPSCALE_ACTION )
                            pNewAction = new MetaBmpScaleAction ( aPos, aSize, aGraphic.GetBitmap() );
                        else
                            pNewAction = new MetaBmpExScaleAction( aPos, aSize, aGraphic.GetBitmapEx() );

                        aMtf.ReplaceAction( pNewAction, i );
                        pAction->Delete();
                        break;
                    }
                    default:
                    case( META_BMP_ACTION ):
                    case( META_BMPSCALEPART_ACTION ):
                    case( META_BMPEX_ACTION ):
                    case( META_BMPEXSCALEPART_ACTION ):
                    case( META_MASK_ACTION ):
                    case( META_MASKSCALE_ACTION ):
                    break;
                }
            }
            rGraphic = aMtf;
        }
    }
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
                    ::Graphic aGraphic( *pGraphic );
                    ImplApplyFilterData( aGraphic, aFilterDataSeq );

                    /* sj: using a temporary memory stream, because some graphic filters are seeking behind
                       stream end (which leads to an invalid argument exception then). */
                    SvMemoryStream aMemStrm;
                    aMemStrm.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
                    if( 0 == strcmp( pFilterShortName, MIMETYPE_VCLGRAPHIC ) )
                        aMemStrm << aGraphic;
                    else
                    {
                        pFilter->ExportGraphic( aGraphic, aPath, aMemStrm,
                                                pFilter->GetExportFormatNumberForShortName( ::rtl::OUString::createFromAscii( pFilterShortName ) ),
                                                    ( aFilterDataSeq.getLength() ? &aFilterDataSeq : NULL ) );
                    }
                    aMemStrm.Seek( STREAM_SEEK_TO_END );
                    pOStm->Write( aMemStrm.GetData(), aMemStrm.Tell() );
                }
            }
        }
        delete pOStm;
    }
}

}
