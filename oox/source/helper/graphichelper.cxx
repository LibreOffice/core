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
#include <oox/helper/graphichelper.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <comphelper/seqstream.hxx>
#include <vcl/wmf.hxx>
#include <vcl/wmfexternal.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <tools/gen.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

namespace oox {

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace {

sal_Int32 lclConvertScreenPixelToHmm( double fPixel, double fPixelPerHmm )
{
    return static_cast< sal_Int32 >( (fPixelPerHmm > 0.0) ? (fPixel / fPixelPerHmm + 0.5) : 0.0 );
}

} // namespace

GraphicHelper::GraphicHelper( const Reference< XComponentContext >& rxContext, const Reference< XFrame >& rxTargetFrame, const StorageRef& rxStorage ) :
    mxContext( rxContext ),
    mxStorage( rxStorage )
{
    OSL_ENSURE( mxContext.is(), "GraphicHelper::GraphicHelper - missing component context" );
    if( mxContext.is() )
        mxGraphicProvider.set( graphic::GraphicProvider::create( mxContext ), uno::UNO_QUERY );

    //! TODO: get colors from system
    maSystemPalette[ XML_3dDkShadow ]               = Color(0x716F64);
    maSystemPalette[ XML_3dLight ]                  = Color(0xF1EFE2);
    maSystemPalette[ XML_activeBorder ]             = Color(0xD4D0C8);
    maSystemPalette[ XML_activeCaption ]            = Color(0x0054E3);
    maSystemPalette[ XML_appWorkspace ]             = Color(0x808080);
    maSystemPalette[ XML_background ]               = Color(0x004E98);
    maSystemPalette[ XML_btnFace ]                  = Color(0xECE9D8);
    maSystemPalette[ XML_btnHighlight ]             = Color(0xFFFFFF);
    maSystemPalette[ XML_btnShadow ]                = Color(0xACA899);
    maSystemPalette[ XML_btnText ]                  = Color(0x000000);
    maSystemPalette[ XML_captionText ]              = Color(0xFFFFFF);
    maSystemPalette[ XML_gradientActiveCaption ]    = Color(0x3D95FF);
    maSystemPalette[ XML_gradientInactiveCaption ]  = Color(0xD8E4F8);
    maSystemPalette[ XML_grayText ]                 = Color(0xACA899);
    maSystemPalette[ XML_highlight ]                = Color(0x316AC5);
    maSystemPalette[ XML_highlightText ]            = Color(0xFFFFFF);
    maSystemPalette[ XML_hotLight ]                 = Color(0x000080);
    maSystemPalette[ XML_inactiveBorder ]           = Color(0xD4D0C8);
    maSystemPalette[ XML_inactiveCaption ]          = Color(0x7A96DF);
    maSystemPalette[ XML_inactiveCaptionText ]      = Color(0xD8E4F8);
    maSystemPalette[ XML_infoBk ]                   = Color(0xFFFFE1);
    maSystemPalette[ XML_infoText ]                 = Color(0x000000);
    maSystemPalette[ XML_menu ]                     = Color(0xFFFFFF);
    maSystemPalette[ XML_menuBar ]                  = Color(0xECE9D8);
    maSystemPalette[ XML_menuHighlight ]            = Color(0x316AC5);
    maSystemPalette[ XML_menuText ]                 = Color(0x000000);
    maSystemPalette[ XML_scrollBar ]                = Color(0xD4D0C8);
    maSystemPalette[ XML_window ]                   = Color(0xFFFFFF);
    maSystemPalette[ XML_windowFrame ]              = Color(0x000000);
    maSystemPalette[ XML_windowText ]               = Color(0x000000);

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
    Size aDefault = Application::GetDefaultDevice()->LogicToPixel(Size(100000, 100000), MapMode(MapUnit::Map100thMM));
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

::Color GraphicHelper::getSystemColor( sal_Int32 nToken, ::Color nDefaultRgb ) const
{
    return ContainerHelper::getMapElement( maSystemPalette, nToken, nDefaultRgb );
}

::Color GraphicHelper::getSchemeColor( sal_Int32 /*nToken*/ ) const
{
    OSL_FAIL( "GraphicHelper::getSchemeColor - scheme colors not implemented" );
    return API_RGB_TRANSPARENT;
}

::Color GraphicHelper::getPaletteColor( sal_Int32 /*nPaletteIdx*/ ) const
{
    OSL_FAIL( "GraphicHelper::getPaletteColor - palette colors not implemented" );
    return API_RGB_TRANSPARENT;
}

sal_Int32 GraphicHelper::getDefaultChartAreaFillStyle() const
{
    return XML_solidFill;
}

sal_Int32 GraphicHelper::getDefaultChartAreaLineStyle() const
{
    return XML_solidFill;
}

sal_Int16 GraphicHelper::getDefaultChartAreaLineWidth() const
{
    // this value is what MSO 2016 writes fixing incomplete MSO 2010 documents (0.75 pt in emu)
    return 9525;
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
        const WmfExternal* pExtHeader ) const
{
    Reference< XGraphic > xGraphic;
    if( rxInStrm.is() && mxGraphicProvider.is() ) try
    {
        Sequence< PropertyValue > aArgs( 2 );
        aArgs[ 0 ].Name = "InputStream";
        aArgs[ 0 ].Value <<= rxInStrm;
        aArgs[ 1 ].Name = "LazyRead";
        bool bLazyRead = !pExtHeader;
        aArgs[ 1 ].Value <<= bLazyRead;

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
            aArgs[ 2 ].Name = "FilterData";
            aArgs[ 2 ].Value <<= aFilterData;
        }

        xGraphic = mxGraphicProvider->queryGraphic( aArgs );
    }
    catch( Exception& )
    {
    }
    return xGraphic;
}

std::vector< uno::Reference<graphic::XGraphic> > GraphicHelper::importGraphics(const std::vector< uno::Reference<io::XInputStream> >& rStreams) const
{
    std::vector< uno::Sequence<beans::PropertyValue> > aArgsVec;

    for (const auto& rStream : rStreams)
    {
        uno::Sequence<beans::PropertyValue > aArgs = comphelper::InitPropertySequence(
        {
            {"InputStream", uno::makeAny(rStream)}
        });
        aArgsVec.push_back(aArgs);
    }

    std::vector< uno::Reference<graphic::XGraphic> > aRet;

    if (mxGraphicProvider.is())
        aRet = comphelper::sequenceToContainer< std::vector< uno::Reference<graphic::XGraphic> > >(mxGraphicProvider->queryGraphics(comphelper::containerToSequence(aArgsVec)));

    return aRet;
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

void GraphicHelper::importEmbeddedGraphics(const std::vector<OUString>& rStreamNames) const
{
    // Don't actually return anything, just fill maEmbeddedGraphics.

    // Stream names and streams to be imported.
    std::vector<OUString> aMissingStreamNames;
    std::vector< uno::Reference<io::XInputStream> > aMissingStreams;

    for (const auto& rStreamName : rStreamNames)
    {
        if(rStreamName.isEmpty())
        {
            SAL_WARN("oox", "GraphicHelper::importEmbeddedGraphics - empty stream name");
            continue;
        }

        EmbeddedGraphicMap::const_iterator aIt = maEmbeddedGraphics.find(rStreamName);
        if (aIt == maEmbeddedGraphics.end())
        {
            aMissingStreamNames.push_back(rStreamName);
            aMissingStreams.push_back(mxStorage->openInputStream(rStreamName));
        }
    }

    std::vector< uno::Reference<graphic::XGraphic> > aGraphics = importGraphics(aMissingStreams);

    assert(aGraphics.size() == aMissingStreamNames.size());
    for (size_t i = 0; i < aGraphics.size(); ++i)
    {
        if (aGraphics[i].is())
            maEmbeddedGraphics[aMissingStreamNames[i]] = aGraphics[i];
    }
}

Reference< XGraphic > GraphicHelper::importEmbeddedGraphic( const OUString& rStreamName, const WmfExternal* pExtHeader ) const
{
    Reference< XGraphic > xGraphic;
    OSL_ENSURE( !rStreamName.isEmpty(), "GraphicHelper::importEmbeddedGraphic - empty stream name" );
    if( !rStreamName.isEmpty() )
    {
        EmbeddedGraphicMap::const_iterator aIt = maEmbeddedGraphics.find( rStreamName );
        if( aIt == maEmbeddedGraphics.end() )
        {
            // Lazy-loading doesn't work with TIFF at the moment.
            WmfExternal aHeader;
            if (rStreamName.endsWith(".tiff") && !pExtHeader)
                pExtHeader = &aHeader;

            xGraphic = importGraphic(mxStorage->openInputStream(rStreamName), pExtHeader);
            if( xGraphic.is() )
                maEmbeddedGraphics[ rStreamName ] = xGraphic;
        }
        else
            xGraphic = aIt->second;
    }
    return xGraphic;
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
