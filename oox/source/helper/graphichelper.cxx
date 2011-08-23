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

#include "oox/helper/graphichelper.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <comphelper/componentcontext.hxx>
#include <comphelper/seqstream.hxx>
#include "tokens.hxx"
#include "oox/helper/containerhelper.hxx"

using ::rtl::OUString;
using ::com::sun::star::awt::DeviceInfo;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Size;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::awt::XUnitConversion;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XFramesSupplier;
using ::com::sun::star::graphic::GraphicObject;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::graphic::XGraphicObject;
using ::com::sun::star::graphic::XGraphicProvider;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;

namespace oox {

// ============================================================================

namespace {

inline sal_Int32 lclConvertScreenPixelToHmm( double fPixel, double fPixelPerHmm )
{
    return static_cast< sal_Int32 >( (fPixelPerHmm > 0.0) ? (fPixel / fPixelPerHmm + 0.5) : 0.0 );
}

} // namespace

// ============================================================================

GraphicHelper::GraphicHelper( const Reference< XMultiServiceFactory >& rxGlobalFactory, const Reference< XFrame >& rxTargetFrame, const StorageRef& rxStorage ) :
    mxGraphicProvider( rxGlobalFactory->createInstance( CREATE_OUSTRING( "com.sun.star.graphic.GraphicProvider" ) ), UNO_QUERY ),
    mxStorage( rxStorage ),
    maGraphicObjScheme( CREATE_OUSTRING( "vnd.sun.star.GraphicObject:" ) )
{
    ::comphelper::ComponentContext aContext( rxGlobalFactory );
    mxCompContext = aContext.getUNOContext();

    //! TODO: get colors from system
    maSystemPalette[ XML_3dDkShadow ]               = 0x716F64;
    maSystemPalette[ XML_3dLight ]                  = 0xF1EFE2;
    maSystemPalette[ XML_activeBorder ]             = 0xD4D0C8;
    maSystemPalette[ XML_activeCaption ]            = 0x0054E3;
    maSystemPalette[ XML_appWorkspace ]             = 0x808080;
    maSystemPalette[ XML_background ]               = 0x004E98;
    maSystemPalette[ XML_btnFace ]                  = 0xECE9D8;
    maSystemPalette[ XML_btnHighlight ]             = 0xFFFFFF;
    maSystemPalette[ XML_btnShadow ]                = 0xACA899;
    maSystemPalette[ XML_btnText ]                  = 0x000000;
    maSystemPalette[ XML_captionText ]              = 0xFFFFFF;
    maSystemPalette[ XML_gradientActiveCaption ]    = 0x3D95FF;
    maSystemPalette[ XML_gradientInactiveCaption ]  = 0xD8E4F8;
    maSystemPalette[ XML_grayText ]                 = 0xACA899;
    maSystemPalette[ XML_highlight ]                = 0x316AC5;
    maSystemPalette[ XML_highlightText ]            = 0xFFFFFF;
    maSystemPalette[ XML_hotLight ]                 = 0x000080;
    maSystemPalette[ XML_inactiveBorder ]           = 0xD4D0C8;
    maSystemPalette[ XML_inactiveCaption ]          = 0x7A96DF;
    maSystemPalette[ XML_inactiveCaptionText ]      = 0xD8E4F8;
    maSystemPalette[ XML_infoBk ]                   = 0xFFFFE1;
    maSystemPalette[ XML_infoText ]                 = 0x000000;
    maSystemPalette[ XML_menu ]                     = 0xFFFFFF;
    maSystemPalette[ XML_menuBar ]                  = 0xECE9D8;
    maSystemPalette[ XML_menuHighlight ]            = 0x316AC5;
    maSystemPalette[ XML_menuText ]                 = 0x000000;
    maSystemPalette[ XML_scrollBar ]                = 0xD4D0C8;
    maSystemPalette[ XML_window ]                   = 0xFFFFFF;
    maSystemPalette[ XML_windowFrame ]              = 0x000000;
    maSystemPalette[ XML_windowText ]               = 0x000000;

    // if no target frame has been passed (e.g. OLE objects), try to fallback to the active frame
    // TODO: we need some mechanism to keep and pass the parent frame
    Reference< XFrame > xFrame = rxTargetFrame;
    if( !xFrame.is() && rxGlobalFactory.is() ) try
    {
        Reference< XFramesSupplier > xFramesSupp( rxGlobalFactory->createInstance( CREATE_OUSTRING( "com.sun.star.frame.Desktop" ) ), UNO_QUERY_THROW );
        xFrame = xFramesSupp->getActiveFrame();
    }
    catch( Exception& )
    {
    }

    // get the metric of the output device
    OSL_ENSURE( xFrame.is(), "GraphicHelper::GraphicHelper - cannot get target frame" );
    maDeviceInfo.PixelPerMeterX = maDeviceInfo.PixelPerMeterY = 3500.0; // some default just in case
    if( xFrame.is() ) try
    {
        Reference< XDevice > xDevice( xFrame->getContainerWindow(), UNO_QUERY_THROW );
        mxUnitConversion.set( xDevice, UNO_QUERY );
        OSL_ENSURE( mxUnitConversion.is(), "GraphicHelper::GraphicHelper - cannot get unit converter" );
        maDeviceInfo = xDevice->getInfo();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "GraphicHelper::GraphicHelper - cannot get output device info" );
    }
    mfPixelPerHmmX = maDeviceInfo.PixelPerMeterX / 100000.0;
    mfPixelPerHmmY = maDeviceInfo.PixelPerMeterY / 100000.0;
}

GraphicHelper::~GraphicHelper()
{
}

// System colors and predefined colors ----------------------------------------

sal_Int32 GraphicHelper::getSystemColor( sal_Int32 nToken, sal_Int32 nDefaultRgb ) const
{
    return ContainerHelper::getMapElement( maSystemPalette, nToken, nDefaultRgb );
}

sal_Int32 GraphicHelper::getSchemeColor( sal_Int32 /*nToken*/ ) const
{
    OSL_ENSURE( false, "GraphicHelper::getSchemeColor - scheme colors not implemented" );
    return API_RGB_TRANSPARENT;
}

sal_Int32 GraphicHelper::getPaletteColor( sal_Int32 /*nPaletteIdx*/ ) const
{
    OSL_ENSURE( false, "GraphicHelper::getPaletteColor - palette colors not implemented" );
    return API_RGB_TRANSPARENT;
}

// Device info and device dependent unit conversion ---------------------------

const DeviceInfo& GraphicHelper::getDeviceInfo() const
{
    return maDeviceInfo;
}

sal_Int32 GraphicHelper::convertScreenPixelXToHmm( double fPixelX ) const
{
    return lclConvertScreenPixelToHmm( fPixelX, mfPixelPerHmmX );
}

sal_Int32 GraphicHelper::convertScreenPixelYToHmm( double fPixelY ) const
{
    return lclConvertScreenPixelToHmm( fPixelY, mfPixelPerHmmY );
}

Point GraphicHelper::convertScreenPixelToHmm( const Point& rPixel ) const
{
    return Point( convertScreenPixelXToHmm( rPixel.X ), convertScreenPixelYToHmm( rPixel.Y ) );
}

Size GraphicHelper::convertScreenPixelToHmm( const Size& rPixel ) const
{
    return Size( convertScreenPixelXToHmm( rPixel.Width ), convertScreenPixelYToHmm( rPixel.Height ) );
}

double GraphicHelper::convertHmmToScreenPixelX( sal_Int32 nHmmX ) const
{
    return nHmmX * mfPixelPerHmmX;
}

double GraphicHelper::convertHmmToScreenPixelY( sal_Int32 nHmmY ) const
{
    return nHmmY * mfPixelPerHmmY;
}

Point GraphicHelper::convertHmmToScreenPixel( const Point& rHmm ) const
{
    return Point(
        static_cast< sal_Int32 >( convertHmmToScreenPixelX( rHmm.X ) + 0.5 ),
        static_cast< sal_Int32 >( convertHmmToScreenPixelY( rHmm.Y ) + 0.5 ) );
}

Size GraphicHelper::convertHmmToScreenPixel( const Size& rHmm ) const
{
    return Size(
        static_cast< sal_Int32 >( convertHmmToScreenPixelX( rHmm.Width ) + 0.5 ),
        static_cast< sal_Int32 >( convertHmmToScreenPixelY( rHmm.Height ) + 0.5 ) );
}

Point GraphicHelper::convertAppFontToHmm( const Point& rAppFont ) const
{
    if( mxUnitConversion.is() ) try
    {
        Point aPixel = mxUnitConversion->convertPointToPixel( rAppFont, ::com::sun::star::util::MeasureUnit::APPFONT );
        return convertScreenPixelToHmm( aPixel );
    }
    catch( Exception& )
    {
    }
    return Point( 0, 0 );
}

Size GraphicHelper::convertAppFontToHmm( const Size& rAppFont ) const
{
    if( mxUnitConversion.is() ) try
    {
        Size aPixel = mxUnitConversion->convertSizeToPixel( rAppFont, ::com::sun::star::util::MeasureUnit::APPFONT );
        return convertScreenPixelToHmm( aPixel );
    }
    catch( Exception& )
    {
    }
    return Size( 0, 0 );
}

Point GraphicHelper::convertHmmToAppFont( const Point& rHmm ) const
{
    if( mxUnitConversion.is() ) try
    {
        Point aPixel = convertHmmToScreenPixel( rHmm );
        return mxUnitConversion->convertPointToLogic( aPixel, ::com::sun::star::util::MeasureUnit::APPFONT );
    }
    catch( Exception& )
    {
    }
    return Point( 0, 0 );
}

Size GraphicHelper::convertHmmToAppFont( const Size& rHmm ) const
{
    if( mxUnitConversion.is() ) try
    {
        Size aPixel = convertHmmToScreenPixel( rHmm );
        return mxUnitConversion->convertSizeToLogic( aPixel, ::com::sun::star::util::MeasureUnit::APPFONT );
    }
    catch( Exception& )
    {
    }
    return Size( 0, 0 );
}

// Graphics and graphic objects  ----------------------------------------------

Reference< XGraphic > GraphicHelper::importGraphic( const Reference< XInputStream >& rxInStrm ) const
{
    Reference< XGraphic > xGraphic;
    if( rxInStrm.is() && mxGraphicProvider.is() ) try
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[ 0 ].Name = CREATE_OUSTRING( "InputStream" );
        aArgs[ 0 ].Value <<= rxInStrm;
        xGraphic = mxGraphicProvider->queryGraphic( aArgs );
    }
    catch( Exception& )
    {
    }
    return xGraphic;
}

Reference< XGraphic > GraphicHelper::importGraphic( const StreamDataSequence& rGraphicData ) const
{
    Reference< XGraphic > xGraphic;
    if( rGraphicData.hasElements() )
    {
        Reference< XInputStream > xInStrm( new ::comphelper::SequenceInputStream( rGraphicData ) );
        xGraphic = importGraphic( xInStrm );
    }
    return xGraphic;
}

Reference< XGraphic > GraphicHelper::importEmbeddedGraphic( const OUString& rStreamName ) const
{
    Reference< XGraphic > xGraphic;
    OSL_ENSURE( rStreamName.getLength() > 0, "GraphicHelper::importEmbeddedGraphic - empty stream name" );
    if( rStreamName.getLength() > 0 )
    {
        EmbeddedGraphicMap::const_iterator aIt = maEmbeddedGraphics.find( rStreamName );
        if( aIt == maEmbeddedGraphics.end() )
        {
            xGraphic = importGraphic( mxStorage->openInputStream( rStreamName ) );
            if( xGraphic.is() )
                maEmbeddedGraphics[ rStreamName ] = xGraphic;
        }
        else
            xGraphic = aIt->second;
    }
    return xGraphic;
}

OUString GraphicHelper::createGraphicObject( const Reference< XGraphic >& rxGraphic ) const
{
    OUString aGraphicObjUrl;
    if( mxCompContext.is() && rxGraphic.is() ) try
    {
        Reference< XGraphicObject > xGraphicObj( GraphicObject::create( mxCompContext ), UNO_SET_THROW );
        xGraphicObj->setGraphic( rxGraphic );
        maGraphicObjects.push_back( xGraphicObj );
        aGraphicObjUrl = maGraphicObjScheme + xGraphicObj->getUniqueID();
    }
    catch( Exception& )
    {
    }
    return aGraphicObjUrl;
}

OUString GraphicHelper::importGraphicObject( const Reference< XInputStream >& rxInStrm ) const
{
    return createGraphicObject( importGraphic( rxInStrm ) );
}

OUString GraphicHelper::importGraphicObject( const StreamDataSequence& rGraphicData ) const
{
    return createGraphicObject( importGraphic( rGraphicData ) );
}

OUString GraphicHelper::importEmbeddedGraphicObject( const OUString& rStreamName ) const
{
    Reference< XGraphic > xGraphic = importEmbeddedGraphic( rStreamName );
    return xGraphic.is() ? createGraphicObject( xGraphic ) : OUString();
}

// ============================================================================

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
