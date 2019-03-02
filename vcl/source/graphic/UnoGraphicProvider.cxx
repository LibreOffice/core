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

#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/metaact.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/imagerepository.hxx>
#include <unotools/resmgr.hxx>
#include <tools/fract.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/wmf.hxx>
#include <vcl/wmfexternal.hxx>
#include <svl/solar.hrc>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphicProvider2.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/fileformat.h>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <graphic/UnoGraphicDescriptor.hxx>
#include <graphic/UnoGraphic.hxx>
#include <rtl/ref.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/dibtools.hxx>
#include <comphelper/sequence.hxx>
#include <memory>
#include <svtools/ehdl.hxx>

using namespace com::sun::star;

namespace {

class GraphicProvider : public ::cppu::WeakImplHelper< css::graphic::XGraphicProvider2,
                                                        css::lang::XServiceInfo >
{
public:

    GraphicProvider();

protected:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XGraphicProvider
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL queryGraphicDescriptor( const css::uno::Sequence< css::beans::PropertyValue >& MediaProperties ) override;
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL queryGraphic( const css::uno::Sequence< css::beans::PropertyValue >& MediaProperties ) override;
    virtual void SAL_CALL storeGraphic( const css::uno::Reference< css::graphic::XGraphic >& Graphic, const css::uno::Sequence< css::beans::PropertyValue >& MediaProperties ) override;

    // XGraphicProvider2
    uno::Sequence< uno::Reference<graphic::XGraphic> > SAL_CALL queryGraphics(const uno::Sequence< uno::Sequence<beans::PropertyValue> >& MediaPropertiesSeq ) override;

private:

    static css::uno::Reference< css::graphic::XGraphic > implLoadMemory( const OUString& rResourceURL );
    static css::uno::Reference< css::graphic::XGraphic > implLoadRepositoryImage( const OUString& rResourceURL );
    static css::uno::Reference< css::graphic::XGraphic > implLoadBitmap( const css::uno::Reference< css::awt::XBitmap >& rBitmap );
    static css::uno::Reference< css::graphic::XGraphic > implLoadStandardImage( const OUString& rResourceURL );
};

GraphicProvider::GraphicProvider()
{
}

OUString SAL_CALL GraphicProvider::getImplementationName()
{
    return OUString( "com.sun.star.comp.graphic.GraphicProvider" );
}

sal_Bool SAL_CALL GraphicProvider::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL GraphicProvider::getSupportedServiceNames()
{
    uno::Sequence<OUString> aSeq { "com.sun.star.graphic.GraphicProvider" };
    return aSeq;
}

uno::Sequence< uno::Type > SAL_CALL GraphicProvider::getTypes()
{
    static const uno::Sequence< uno::Type > aTypes {
        cppu::UnoType<lang::XServiceInfo>::get(),
        cppu::UnoType<lang::XTypeProvider>::get(),
        cppu::UnoType<graphic::XGraphicProvider>::get()
    };
    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL GraphicProvider::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadMemory( const OUString& rResourceURL )
{
    uno::Reference< ::graphic::XGraphic >   xRet;
    sal_Int32                               nIndex = 0;

    if( rResourceURL.getToken( 0, '/', nIndex ) == "private:memorygraphic" )
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


uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadRepositoryImage( const OUString& rResourceURL )
{
    uno::Reference< ::graphic::XGraphic >   xRet;

    OUString sPathName;
    if( rResourceURL.startsWith("private:graphicrepository/", &sPathName) )
    {
        BitmapEx aBitmap;
        if ( vcl::ImageRepository::loadImage( sPathName, aBitmap ) )
        {
            xRet = Graphic(aBitmap).GetXGraphic();
        }
    }
    return xRet;
}


uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadStandardImage( const OUString& rResourceURL )
{
    uno::Reference< ::graphic::XGraphic >   xRet;

    OUString sImageName;
    if( rResourceURL.startsWith("private:standardimage/", &sImageName) )
    {
        if ( sImageName == "info" )
        {
            xRet = Graphic(GetStandardInfoBoxImage().GetBitmapEx()).GetXGraphic();
        }
        else if ( sImageName == "warning" )
        {
            xRet = Graphic(GetStandardWarningBoxImage().GetBitmapEx()).GetXGraphic();
        }
        else if ( sImageName == "error" )
        {
            xRet = Graphic(GetStandardErrorBoxImage().GetBitmapEx()).GetXGraphic();
        }
        else if ( sImageName == "query" )
        {
            xRet = Graphic(GetStandardQueryBoxImage().GetBitmapEx()).GetXGraphic();
        }
    }
    return xRet;
}


uno::Reference< ::graphic::XGraphic > GraphicProvider::implLoadBitmap( const uno::Reference< awt::XBitmap >& xBtm )
{
    uno::Reference< ::graphic::XGraphic > xRet;
    uno::Sequence< sal_Int8 > aBmpSeq( xBtm->getDIB() );
    uno::Sequence< sal_Int8 > aMaskSeq( xBtm->getMaskDIB() );
    SvMemoryStream aBmpStream( aBmpSeq.getArray(), aBmpSeq.getLength(), StreamMode::READ );
    Bitmap aBmp;
    BitmapEx aBmpEx;

    ReadDIB(aBmp, aBmpStream, true);

    if( aMaskSeq.getLength() )
    {
        SvMemoryStream aMaskStream( aMaskSeq.getArray(), aMaskSeq.getLength(), StreamMode::READ );
        Bitmap aMask;

        ReadDIB(aMask, aMaskStream, true);
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

uno::Reference< beans::XPropertySet > SAL_CALL GraphicProvider::queryGraphicDescriptor( const uno::Sequence< beans::PropertyValue >& rMediaProperties )
{
    uno::Reference< beans::XPropertySet > xRet;

    OUString aURL;
    uno::Reference< io::XInputStream > xIStm;
    uno::Reference< awt::XBitmap >xBtm;

    for( sal_Int32 i = 0; ( i < rMediaProperties.getLength() ) && !xRet.is(); ++i )
    {
        const OUString   aName( rMediaProperties[ i ].Name );
        const uno::Any          aValue( rMediaProperties[ i ].Value );

        if (aName == "URL")
        {
            aValue >>= aURL;
        }
        else if (aName == "InputStream")
        {
            aValue >>= xIStm;
        }
        else if (aName == "Bitmap")
        {
            aValue >>= xBtm;
        }
    }

    SolarMutexGuard g;

    if( xIStm.is() )
    {
        unographic::GraphicDescriptor* pDescriptor = new unographic::GraphicDescriptor;
        pDescriptor->init( xIStm, aURL );
        xRet = pDescriptor;
    }
    else if( !aURL.isEmpty() )
    {
        uno::Reference< ::graphic::XGraphic > xGraphic( implLoadMemory( aURL ) );

        if ( !xGraphic.is() )
            xGraphic = implLoadRepositoryImage( aURL );

        if ( !xGraphic.is() )
            xGraphic = implLoadStandardImage( aURL );

        if( xGraphic.is() )
        {
            xRet.set( xGraphic, uno::UNO_QUERY );
        }
        else
        {
            unographic::GraphicDescriptor* pDescriptor = new unographic::GraphicDescriptor;
            pDescriptor->init( aURL );
            xRet = pDescriptor;
        }
    }
    else if( xBtm.is() )
    {
        uno::Reference< ::graphic::XGraphic > xGraphic( implLoadBitmap( xBtm ) );
        if( xGraphic.is() )
            xRet.set( xGraphic, uno::UNO_QUERY );
    }

    return xRet;
}


uno::Reference< ::graphic::XGraphic > SAL_CALL GraphicProvider::queryGraphic( const uno::Sequence< ::beans::PropertyValue >& rMediaProperties )
{
    uno::Reference< ::graphic::XGraphic >   xRet;
    OUString                                aPath;

    uno::Reference< io::XInputStream > xIStm;
    uno::Reference< awt::XBitmap >xBtm;

    uno::Sequence< ::beans::PropertyValue > aFilterData;

    bool bLazyRead = false;
    bool bLoadAsLink = false;

    for (sal_Int32 i = 0; ( i < rMediaProperties.getLength() ) && !xRet.is(); ++i)
    {
        const OUString   aName( rMediaProperties[ i ].Name );
        const uno::Any          aValue( rMediaProperties[ i ].Value );

        if (aName == "URL")
        {
            OUString aURL;
            aValue >>= aURL;
            aPath = aURL;
        }
        else if (aName == "InputStream")
        {
            aValue >>= xIStm;
        }
        else if (aName == "Bitmap")
        {
            aValue >>= xBtm;
        }
        else if (aName == "FilterData")
        {
            aValue >>= aFilterData;
        }
        else if (aName == "LazyRead")
        {
            aValue >>= bLazyRead;
        }
        else if (aName == "LoadAsLink")
        {
            aValue >>= bLoadAsLink;
        }
    }

    // Check for the goal width and height if they are defined
    sal_uInt16 nExtWidth = 0;
    sal_uInt16 nExtHeight = 0;
    sal_uInt16 nExtMapMode = 0;
    for( sal_Int32 i = 0; i < aFilterData.getLength(); ++i )
    {
        const OUString   aName( aFilterData[ i ].Name );
        const uno::Any          aValue( aFilterData[ i ].Value );

        if (aName == "ExternalWidth")
        {
            aValue >>= nExtWidth;
        }
        else if (aName == "ExternalHeight")
        {
            aValue >>= nExtHeight;
        }
        else if (aName == "ExternalMapMode")
        {
            aValue >>= nExtMapMode;
        }
    }

    SolarMutexGuard g;

    std::unique_ptr<SvStream> pIStm;

    if( xIStm.is() )
    {
        pIStm = ::utl::UcbStreamHelper::CreateStream( xIStm );
    }
    else if( !aPath.isEmpty() )
    {
        xRet = implLoadMemory( aPath );

        if ( !xRet.is() )
            xRet = implLoadRepositoryImage( aPath );

        if ( !xRet.is() )
            xRet = implLoadStandardImage( aPath );

        if( !xRet.is() )
            pIStm = ::utl::UcbStreamHelper::CreateStream( aPath, StreamMode::READ );
    }
    else if( xBtm.is() )
    {
        xRet = implLoadBitmap( xBtm );
    }

    if( pIStm )
    {
        ::GraphicFilter& rFilter = ::GraphicFilter::GetGraphicFilter();

        {
            Graphic aVCLGraphic;

            // Define APM Header if goal height and width are defined
            WmfExternal aExtHeader;
            aExtHeader.xExt = nExtWidth;
            aExtHeader.yExt = nExtHeight;
            aExtHeader.mapMode = nExtMapMode;
            WmfExternal *pExtHeader = nullptr;
            if ( nExtMapMode > 0 )
            {
                pExtHeader = &aExtHeader;
                bLazyRead = false;
            }

            ErrCode error = ERRCODE_NONE;
            if (bLazyRead)
            {
                Graphic aGraphic = rFilter.ImportUnloadedGraphic(*pIStm);
                if (aGraphic)
                    aVCLGraphic = aGraphic;
            }
            if (!aVCLGraphic)
                error = rFilter.ImportGraphic(aVCLGraphic, aPath, *pIStm, GRFILTER_FORMAT_DONTKNOW,
                                              nullptr, GraphicFilterImportFlags::NONE, pExtHeader);

            if( (error == ERRCODE_NONE ) &&
                ( aVCLGraphic.GetType() != GraphicType::NONE ) )
            {
                if (!aPath.isEmpty() && bLoadAsLink)
                    aVCLGraphic.setOriginURL(aPath);

                ::unographic::Graphic* pUnoGraphic = new ::unographic::Graphic;

                pUnoGraphic->init( aVCLGraphic );
                xRet = pUnoGraphic;
            }
            else{
                SAL_WARN("svtools", "Could not create graphic: " << error);
            }
        }
    }

    return xRet;
}

uno::Sequence< uno::Reference<graphic::XGraphic> > SAL_CALL GraphicProvider::queryGraphics(const uno::Sequence< uno::Sequence<beans::PropertyValue> >& rMediaPropertiesSeq)
{
    SolarMutexGuard aGuard;

    // Turn properties into streams.
    std::vector< std::unique_ptr<SvStream> > aStreams;
    for (const auto& rMediaProperties : rMediaPropertiesSeq)
    {
        std::unique_ptr<SvStream> pStream;
        uno::Reference<io::XInputStream> xStream;

        for (sal_Int32 i = 0; rMediaProperties.getLength(); ++i)
        {
            if (rMediaProperties[i].Name == "InputStream")
            {
                rMediaProperties[i].Value >>= xStream;
                if (xStream.is())
                    pStream = utl::UcbStreamHelper::CreateStream(xStream);
                break;
            }
        }

        aStreams.push_back(std::move(pStream));
    }

    // Import: streams to graphics.
    std::vector< std::shared_ptr<Graphic> > aGraphics;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    rFilter.ImportGraphics(aGraphics, std::move(aStreams));

    // Returning: graphics to UNO objects.
    std::vector< uno::Reference<graphic::XGraphic> > aRet;
    for (const auto& pGraphic : aGraphics)
    {
        uno::Reference<graphic::XGraphic> xGraphic;

        if (pGraphic)
        {
            auto pUnoGraphic = new unographic::Graphic();
            pUnoGraphic->init(*pGraphic);
            xGraphic = pUnoGraphic;
        }

        aRet.push_back(xGraphic);
    }

    return comphelper::containerToSequence(aRet);
}

void ImplCalculateCropRect( ::Graphic const & rGraphic, const text::GraphicCrop& rGraphicCropLogic, tools::Rectangle& rGraphicCropPixel )
{
    if ( !(rGraphicCropLogic.Left || rGraphicCropLogic.Top || rGraphicCropLogic.Right || rGraphicCropLogic.Bottom) )
        return;

    Size aSourceSizePixel( rGraphic.GetSizePixel() );
    if ( !(aSourceSizePixel.Width() && aSourceSizePixel.Height()) )
        return;

    if ( !(rGraphicCropLogic.Left || rGraphicCropLogic.Top || rGraphicCropLogic.Right || rGraphicCropLogic.Bottom) )
        return;

    Size aSize100thMM( 0, 0 );
    if( rGraphic.GetPrefMapMode().GetMapUnit() != MapUnit::MapPixel )
    {
        aSize100thMM = OutputDevice::LogicToLogic(rGraphic.GetPrefSize(), rGraphic.GetPrefMapMode(), MapMode(MapUnit::Map100thMM));
    }
    else
    {
        aSize100thMM = Application::GetDefaultDevice()->PixelToLogic(rGraphic.GetPrefSize(), MapMode(MapUnit::Map100thMM));
    }
    if ( aSize100thMM.Width() && aSize100thMM.Height() )
    {
        double fSourceSizePixelWidth = static_cast<double>(aSourceSizePixel.Width());
        double fSourceSizePixelHeight= static_cast<double>(aSourceSizePixel.Height());
        rGraphicCropPixel.SetLeft( static_cast< sal_Int32 >((fSourceSizePixelWidth * rGraphicCropLogic.Left ) / aSize100thMM.Width()) );
        rGraphicCropPixel.SetTop( static_cast< sal_Int32 >((fSourceSizePixelHeight * rGraphicCropLogic.Top ) / aSize100thMM.Height()) );
        rGraphicCropPixel.SetRight( static_cast< sal_Int32 >(( fSourceSizePixelWidth * ( aSize100thMM.Width() - rGraphicCropLogic.Right ) ) / aSize100thMM.Width() ) );
        rGraphicCropPixel.SetBottom( static_cast< sal_Int32 >(( fSourceSizePixelHeight * ( aSize100thMM.Height() - rGraphicCropLogic.Bottom ) ) / aSize100thMM.Height() ) );
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
    if ( !(nImageResolution && rLogicalSize.Width && rLogicalSize.Height) )
        return;

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
    bool bRemoveCropArea = true;

    for( sal_Int32 i = 0; i < rFilterData.getLength(); ++i )
    {
        const OUString   aName(  rFilterData[ i ].Name );
        const uno::Any          aValue( rFilterData[ i ].Value );

        if (aName == "PixelWidth")
            aValue >>= nPixelWidth;
        else if (aName == "PixelHeight")
            aValue >>= nPixelHeight;
        else if (aName == "LogicalSize")
            aValue >>= aLogicalSize;
        else if (aName == "GraphicCropLogic")
            aValue >>= aCropLogic;
        else if (aName == "RemoveCropArea")
            aValue >>= bRemoveCropArea;
        else if (aName == "ImageResolution")
            aValue >>= nImageResolution;
    }
    if ( rGraphic.GetType() == GraphicType::Bitmap )
    {
        if(rGraphic.getVectorGraphicData().get())
        {
            // embedded Vector Graphic Data, no need to scale. Also no method to apply crop data currently
        }
        else
        {
            tools::Rectangle aCropPixel( Point( 0, 0 ), rGraphic.GetSizePixel() );
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
    }
    else if ( ( rGraphic.GetType() == GraphicType::GdiMetafile ) && nImageResolution )
    {
        ScopedVclPtrInstance< VirtualDevice > aDummyVDev;
        GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );
        Size aMtfSize( OutputDevice::LogicToLogic(aMtf.GetPrefSize(), aMtf.GetPrefMapMode(), MapMode(MapUnit::Map100thMM)) );
        if ( aMtfSize.Width() && aMtfSize.Height() )
        {
            MapMode aNewMapMode( MapUnit::Map100thMM );
            aNewMapMode.SetScaleX( Fraction( aLogicalSize.Width, aMtfSize.Width() ) );
            aNewMapMode.SetScaleY( Fraction( aLogicalSize.Height, aMtfSize.Height() ) );
            aDummyVDev->EnableOutput( false );
            aDummyVDev->SetMapMode( aNewMapMode );

            for( size_t i = 0, nObjCount = aMtf.GetActionSize(); i < nObjCount; i++ )
            {
                MetaAction* pAction = aMtf.GetAction( i );
                switch( pAction->GetType() )
                {
                    // only optimizing common bitmap actions:
                    case MetaActionType::MAPMODE:
                    {
                        pAction->Execute( aDummyVDev.get() );
                        break;
                    }
                    case MetaActionType::PUSH:
                    {
                        const MetaPushAction* pA = static_cast<const MetaPushAction*>(pAction);
                        aDummyVDev->Push( pA->GetFlags() );
                        break;
                    }
                    case MetaActionType::POP:
                    {
                        aDummyVDev->Pop();
                        break;
                    }
                    case MetaActionType::BMPSCALE:
                    case MetaActionType::BMPEXSCALE:
                    {
                        BitmapEx aBmpEx;
                        Point aPos;
                        Size aSize;
                        if ( pAction->GetType() == MetaActionType::BMPSCALE )
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
                        const Size aSize100thmm( aDummyVDev->LogicToPixel( aSize ) );
                        Size aSize100thmm2( aDummyVDev->PixelToLogic(aSize100thmm, MapMode(MapUnit::Map100thMM)) );

                        ImplApplyBitmapResolution( aGraphic, nImageResolution,
                            aGraphic.GetSizePixel(), awt::Size( aSize100thmm2.Width(), aSize100thmm2.Height() ) );

                        rtl::Reference<MetaAction> pNewAction = new MetaBmpExScaleAction( aPos, aSize, aGraphic.GetBitmapEx() );
                        aMtf.ReplaceAction( pNewAction, i );
                        break;
                    }
                    default:
                    case MetaActionType::BMP:
                    case MetaActionType::BMPSCALEPART:
                    case MetaActionType::BMPEX:
                    case MetaActionType::BMPEXSCALEPART:
                    case MetaActionType::MASK:
                    case MetaActionType::MASKSCALE:
                    break;
                }
            }
            rGraphic = aMtf;
        }
    }
}


void SAL_CALL GraphicProvider::storeGraphic( const uno::Reference< ::graphic::XGraphic >& rxGraphic, const uno::Sequence< beans::PropertyValue >& rMediaProperties )
{
    SolarMutexGuard g;

    std::unique_ptr<SvStream> pOStm;
    OUString    aPath;
    sal_Int32   i;

    for( i = 0; ( i < rMediaProperties.getLength() ) && !pOStm; ++i )
    {
        const OUString   aName( rMediaProperties[ i ].Name );
        const uno::Any          aValue( rMediaProperties[ i ].Value );

        if (aName == "URL")
        {
            OUString aURL;

            aValue >>= aURL;
            pOStm = ::utl::UcbStreamHelper::CreateStream( aURL, StreamMode::WRITE | StreamMode::TRUNC );
            aPath = aURL;
        }
        else if (aName == "OutputStream")
        {
            uno::Reference< io::XStream > xOStm;

            aValue >>= xOStm;

            if( xOStm.is() )
                pOStm = ::utl::UcbStreamHelper::CreateStream( xOStm );
        }
    }

    if( !pOStm )
        return;

    uno::Sequence< beans::PropertyValue >   aFilterDataSeq;
    const char*                             pFilterShortName = nullptr;

    for( i = 0; i < rMediaProperties.getLength(); ++i )
    {
        const OUString   aName( rMediaProperties[ i ].Name );
        const uno::Any          aValue( rMediaProperties[ i ].Value );

        if (aName == "FilterData")
        {
            aValue >>= aFilterDataSeq;
        }
        else if (aName == "MimeType")
        {
            OUString aMimeType;

            aValue >>= aMimeType;

            if (aMimeType == MIMETYPE_BMP)
                pFilterShortName = "bmp";
            else if (aMimeType == MIMETYPE_EPS)
                pFilterShortName = "eps";
            else if (aMimeType == MIMETYPE_GIF)
                pFilterShortName = "gif";
            else if (aMimeType == MIMETYPE_JPG)
                pFilterShortName = "jpg";
            else if (aMimeType == MIMETYPE_MET)
                pFilterShortName = "met";
            else if (aMimeType == MIMETYPE_PNG)
                pFilterShortName = "png";
            else if (aMimeType == MIMETYPE_PCT)
                pFilterShortName = "pct";
            else if (aMimeType == MIMETYPE_PBM)
                pFilterShortName = "pbm";
            else if (aMimeType == MIMETYPE_PGM)
                pFilterShortName = "pgm";
            else if (aMimeType == MIMETYPE_PPM)
                pFilterShortName = "ppm";
            else if (aMimeType == MIMETYPE_RAS)
                pFilterShortName = "ras";
            else if (aMimeType == MIMETYPE_SVM)
                pFilterShortName = "svm";
            else if (aMimeType == MIMETYPE_TIF)
                pFilterShortName = "tif";
            else if (aMimeType == MIMETYPE_EMF)
                pFilterShortName = "emf";
            else if (aMimeType == MIMETYPE_WMF)
                pFilterShortName = "wmf";
            else if (aMimeType == MIMETYPE_XPM)
                pFilterShortName = "xpm";
            else if (aMimeType == MIMETYPE_SVG)
                pFilterShortName = "svg";
            else if (aMimeType == MIMETYPE_VCLGRAPHIC)
                pFilterShortName = MIMETYPE_VCLGRAPHIC;
        }
    }

    if( !pFilterShortName )
        return;

    ::GraphicFilter& rFilter = ::GraphicFilter::GetGraphicFilter();

    {
        const uno::Reference< XInterface >  xIFace( rxGraphic, uno::UNO_QUERY );
        const ::Graphic*                    pGraphic = ::unographic::Graphic::getImplementation( xIFace );

        if( pGraphic && ( pGraphic->GetType() != GraphicType::NONE ) )
        {
            ::Graphic aGraphic( *pGraphic );
            ImplApplyFilterData( aGraphic, aFilterDataSeq );

            /* sj: using a temporary memory stream, because some graphic filters are seeking behind
               stream end (which leads to an invalid argument exception then). */
            SvMemoryStream aMemStrm;
            aMemStrm.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
            if( 0 == strcmp( pFilterShortName, MIMETYPE_VCLGRAPHIC ) )
                WriteGraphic( aMemStrm, aGraphic );
            else
            {
                rFilter.ExportGraphic( aGraphic, aPath, aMemStrm,
                                        rFilter.GetExportFormatNumberForShortName( OUString::createFromAscii( pFilterShortName ) ),
                                            ( aFilterDataSeq.getLength() ? &aFilterDataSeq : nullptr ) );
            }
            pOStm->WriteBytes( aMemStrm.GetData(), aMemStrm.TellEnd() );
        }
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_graphic_GraphicProvider_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new GraphicProvider);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
