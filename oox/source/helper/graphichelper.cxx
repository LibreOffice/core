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

#include <com/sun/star/container/XNameContainer.hpp>
#include "oox/helper/graphichelper.hxx"

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <osl/diagnose.h>
#include <comphelper/seqstream.hxx>
#include <vcl/wmf.hxx>
#include <vcl/svapp.hxx>
#include <tools/gen.hxx>
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/properties.hxx"
#include "oox/token/tokens.hxx"

namespace oox {

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace {

inline sal_Int32 lclConvertScreenPixelToHmm( double fPixel, double fPixelPerHmm )
{
    return static_cast< sal_Int32 >( (fPixelPerHmm > 0.0) ? (fPixel / fPixelPerHmm + 0.5) : 0.0 );
}

} // namespace

GraphicHelper::GraphicHelper( const Reference< XComponentContext >& rxContext, const Reference< XFrame >& rxTargetFrame, const StorageRef& rxStorage ) :
    mxContext( rxContext ),
    mxStorage( rxStorage ),
    maGraphicObjScheme( "vnd.sun.star.GraphicObject:" )
{
    OSL_ENSURE( mxContext.is(), "GraphicHelper::GraphicHelper - missing component context" );
    if( mxContext.is() )
        mxGraphicProvider.set( graphic::GraphicProvider::create( mxContext ) );

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
    if( !xFrame.is() && mxContext.is() ) try
    {
        Reference< XDesktop2 > xFramesSupp = Desktop::create( mxContext );
        xFrame = xFramesSupp->getActiveFrame();
    }
    catch( Exception& )
    {
    }

    // get the metric of the output device
    OSL_ENSURE( xFrame.is(), "GraphicHelper::GraphicHelper - cannot get target frame" );
    // some default just in case, 100 000 is 1 meter in MM100
    Size aDefault = Application::GetDefaultDevice()->LogicToPixel(Size(100000, 100000), MapMode(MAP_100TH_MM));
    maDeviceInfo.PixelPerMeterX = aDefault.Width();
    maDeviceInfo.PixelPerMeterY = aDefault.Height();
    if( xFrame.is() ) try
    {
        Reference< awt::XDevice > xDevice( xFrame->getContainerWindow(), UNO_QUERY_THROW );
        mxUnitConversion.set( xDevice, UNO_QUERY );
        OSL_ENSURE( mxUnitConversion.is(), "GraphicHelper::GraphicHelper - cannot get unit converter" );
        maDeviceInfo = xDevice->getInfo();
    }
    catch( Exception& )
    {
        OSL_FAIL( "GraphicHelper::GraphicHelper - cannot get output device info" );
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
    OSL_FAIL( "GraphicHelper::getSchemeColor - scheme colors not implemented" );
    return API_RGB_TRANSPARENT;
}

sal_Int32 GraphicHelper::getPaletteColor( sal_Int32 /*nPaletteIdx*/ ) const
{
    OSL_FAIL( "GraphicHelper::getPaletteColor - palette colors not implemented" );
    return API_RGB_TRANSPARENT;
}

sal_Int32 GraphicHelper::getDefaultChartAreaFillStyle() const
{
    return XML_solidFill;
}

// Device info and device dependent unit conversion ---------------------------

sal_Int32 GraphicHelper::convertScreenPixelXToHmm( double fPixelX ) const
{
    return lclConvertScreenPixelToHmm( fPixelX, mfPixelPerHmmX );
}

sal_Int32 GraphicHelper::convertScreenPixelYToHmm( double fPixelY ) const
{
    return lclConvertScreenPixelToHmm( fPixelY, mfPixelPerHmmY );
}

awt::Size GraphicHelper::convertScreenPixelToHmm( const awt::Size& rPixel ) const
{
    return awt::Size( convertScreenPixelXToHmm( rPixel.Width ), convertScreenPixelYToHmm( rPixel.Height ) );
}

double GraphicHelper::convertHmmToScreenPixelX( sal_Int32 nHmmX ) const
{
    return nHmmX * mfPixelPerHmmX;
}

double GraphicHelper::convertHmmToScreenPixelY( sal_Int32 nHmmY ) const
{
    return nHmmY * mfPixelPerHmmY;
}

awt::Point GraphicHelper::convertHmmToScreenPixel( const awt::Point& rHmm ) const
{
    return awt::Point(
        static_cast< sal_Int32 >( convertHmmToScreenPixelX( rHmm.X ) + 0.5 ),
        static_cast< sal_Int32 >( convertHmmToScreenPixelY( rHmm.Y ) + 0.5 ) );
}

awt::Size GraphicHelper::convertHmmToScreenPixel( const awt::Size& rHmm ) const
{
    return awt::Size(
        static_cast< sal_Int32 >( convertHmmToScreenPixelX( rHmm.Width ) + 0.5 ),
        static_cast< sal_Int32 >( convertHmmToScreenPixelY( rHmm.Height ) + 0.5 ) );
}

awt::Point GraphicHelper::convertHmmToAppFont( const awt::Point& rHmm ) const
{
    if( mxUnitConversion.is() ) try
    {
        awt::Point aPixel = convertHmmToScreenPixel( rHmm );
        return mxUnitConversion->convertPointToLogic( aPixel, css::util::MeasureUnit::APPFONT );
    }
    catch( Exception& )
    {
    }
    return awt::Point( 0, 0 );
}

awt::Size GraphicHelper::convertHmmToAppFont( const awt::Size& rHmm ) const
{
    if( mxUnitConversion.is() ) try
    {
        awt::Size aPixel = convertHmmToScreenPixel( rHmm );
        return mxUnitConversion->convertSizeToLogic( aPixel, css::util::MeasureUnit::APPFONT );
    }
    catch( Exception& )
    {
    }
    return awt::Size( 0, 0 );
}

// Graphics and graphic objects  ----------------------------------------------

Reference< XGraphic > GraphicHelper::importGraphic( const Reference< XInputStream >& rxInStrm,
        const WMF_EXTERNALHEADER* pExtHeader ) const
{
    Reference< XGraphic > xGraphic;
    if( rxInStrm.is() && mxGraphicProvider.is() ) try
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[ 0 ].Name = "InputStream";
        aArgs[ 0 ].Value <<= rxInStrm;

        if ( pExtHeader && pExtHeader->mapMode > 0 )
        {
            aArgs.realloc( aArgs.getLength() + 1 );
            Sequence< PropertyValue > aFilterData( 3 );
            aFilterData[ 0 ].Name = "ExternalWidth";
            aFilterData[ 0 ].Value <<= pExtHeader->xExt;
            aFilterData[ 1 ].Name = "ExternalHeight";
            aFilterData[ 1 ].Value <<= pExtHeader->yExt;
            aFilterData[ 2 ].Name = "ExternalMapMode";
            aFilterData[ 2 ].Value <<= pExtHeader->mapMode;
            aArgs[ 1 ].Name = "FilterData";
            aArgs[ 1 ].Value <<= aFilterData;
        }

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

Reference< XGraphic > GraphicHelper::importEmbeddedGraphic( const OUString& rStreamName, const WMF_EXTERNALHEADER* pExtHeader ) const
{
    Reference< XGraphic > xGraphic;
    OSL_ENSURE( !rStreamName.isEmpty(), "GraphicHelper::importEmbeddedGraphic - empty stream name" );
    if( !rStreamName.isEmpty() )
    {
        EmbeddedGraphicMap::const_iterator aIt = maEmbeddedGraphics.find( rStreamName );
        if( aIt == maEmbeddedGraphics.end() )
        {
            xGraphic = importGraphic(mxStorage->openInputStream(rStreamName), pExtHeader);
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
    if( mxContext.is() && rxGraphic.is() ) try
    {
        Reference< XGraphicObject > xGraphicObj( graphic::GraphicObject::create( mxContext ), UNO_SET_THROW );
        xGraphicObj->setGraphic( rxGraphic );
        maGraphicObjects.push_back( xGraphicObj );
        aGraphicObjUrl = maGraphicObjScheme + xGraphicObj->getUniqueID();
    }
    catch( Exception& )
    {
    }
    return aGraphicObjUrl;
}

OUString GraphicHelper::importGraphicObject( const Reference< XInputStream >& rxInStrm,
        const WMF_EXTERNALHEADER* pExtHeader ) const
{
    return createGraphicObject( importGraphic( rxInStrm, pExtHeader ) );
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

awt::Size GraphicHelper::getOriginalSize( const Reference< XGraphic >& xGraphic ) const
{
    awt::Size aSizeHmm;
    PropertySet aPropSet( xGraphic );
    if( aPropSet.getProperty( aSizeHmm, PROP_Size100thMM ) && (aSizeHmm.Width == 0) && (aSizeHmm.Height == 0) )     // MAPMODE_PIXEL used?
    {
        awt::Size aSizePixel( 0, 0 );
        if( aPropSet.getProperty( aSizePixel, PROP_SizePixel ) )
            aSizeHmm = convertScreenPixelToHmm( aSizePixel );
    }
    return aSizeHmm;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
