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

#include <oox/helper/graphichelper.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/GraphicMapper.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/seqstream.hxx>
#include <utility>
#include <vcl/wmfexternal.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <tools/gen.hxx>
#include <comphelper/diagnose_ex.hxx>
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
using namespace ::com::sun::star::uno;

namespace {

sal_Int32 lclConvertScreenPixelToHmm( double fPixel, double fPixelPerHmm )
{
    return static_cast< sal_Int32 >( (fPixelPerHmm > 0.0) ? (fPixel / fPixelPerHmm + 0.5) : 0.0 );
}

} // namespace

GraphicHelper::GraphicHelper( const Reference< XComponentContext >& rxContext, const Reference< XFrame >& /*rxTargetFrame*/, StorageRef xStorage ) :
    mxContext( rxContext ),
    mxStorage(std::move( xStorage ))
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

    // Note that we cannot try to get DeviceInfo from the current frame here,
    // because there might not be a current frame yet
    mxDefaultOutputDevice = Application::GetDefaultDevice();
    maDeviceInfo = mxDefaultOutputDevice->GetDeviceInfo();
    // 100 000 is 1 meter in MM100.
    // various unit tests rely on these values being exactly this and not the "true" values
    Size aDefault = mxDefaultOutputDevice->LogicToPixel(Size(100000, 100000), MapMode(MapUnit::Map100thMM));
    maDeviceInfo.PixelPerMeterX = aDefault.Width();
    maDeviceInfo.PixelPerMeterY = aDefault.Height();
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

void GraphicHelper::getSchemeColorToken(sal_Int32& /*nToken*/) const {}

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

sal_Int32 GraphicHelper::getDefaultChartAreaLineStyle()
{
    return XML_solidFill;
}

sal_Int16 GraphicHelper::getDefaultChartAreaLineWidth()
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
    try
    {
        awt::Point aPixel = convertHmmToScreenPixel( rHmm );
        MapMode aMode(MapUnit::MapAppFont);
        ::Point aVCLPoint(aPixel.X, aPixel.Y);
        ::Point aDevPoint = mxDefaultOutputDevice->PixelToLogic(aVCLPoint, aMode );
        return awt::Point(aDevPoint.X(), aDevPoint.Y());
    }
    catch( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }
    return awt::Point( 0, 0 );
}

awt::Size GraphicHelper::convertHmmToAppFont( const awt::Size& rHmm ) const
{
    try
    {
        awt::Size aPixel = convertHmmToScreenPixel( rHmm );
        MapMode aMode(MapUnit::MapAppFont);
        ::Size aVCLSize(aPixel.Width, aPixel.Height);
        ::Size aDevSz = mxDefaultOutputDevice->PixelToLogic(aVCLSize, aMode );
        return awt::Size(aDevSz.Width(), aDevSz.Height());
    }
    catch( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }
    return awt::Size( 0, 0 );
}


// Graphics and graphic objects  ----------------------------------------------

Reference< XGraphic > GraphicHelper::importGraphic( const Reference< XInputStream >& rxInStrm,
        const WmfExternal* pExtHeader, const bool bLazyLoad ) const
{
    Reference< XGraphic > xGraphic;
    if( rxInStrm.is() && mxGraphicProvider.is() ) try
    {
        Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(u"InputStream"_ustr, rxInStrm),
                                         comphelper::makePropertyValue(u"LazyRead"_ustr, bLazyLoad) };

        if ( pExtHeader && pExtHeader->mapMode > 0 )
        {
            aArgs.realloc( aArgs.getLength() + 1 );
            auto pArgs = aArgs.getArray();
            Sequence< PropertyValue > aFilterData{
                comphelper::makePropertyValue(u"ExternalWidth"_ustr, pExtHeader->xExt),
                comphelper::makePropertyValue(u"ExternalHeight"_ustr, pExtHeader->yExt),
                comphelper::makePropertyValue(u"ExternalMapMode"_ustr, pExtHeader->mapMode)
            };
            pArgs[ 2 ].Name = "FilterData";
            pArgs[ 2 ].Value <<= aFilterData;
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

Reference< XGraphic > GraphicHelper::importEmbeddedGraphic( const OUString& rStreamName, const WmfExternal* pExtHeader ) const
{
    Reference< XGraphic > xGraphic;
    OSL_ENSURE( !rStreamName.isEmpty(), "GraphicHelper::importEmbeddedGraphic - empty stream name" );

    if( !rStreamName.isEmpty() )
    {
        initializeGraphicMapperIfNeeded();

        SAL_WARN_IF(!mxGraphicMapper.is(), "oox", "GraphicHelper::importEmbeddedGraphic - graphic mapper not available");

        xGraphic = mxGraphicMapper->findGraphic(rStreamName);
        if (!xGraphic.is())
        {
            // Lazy-loading doesn't work with cropped TIFF images, because in case of Lazy-load TIFF images
            // we are using MapUnit::MapPixel, but in case of cropped images we are using MapUnit::Map100thMM
            // and the crop values are relative to original bitmap size.
            auto xStream = mxStorage->openInputStream(rStreamName);
            xGraphic = importGraphic(xStream, pExtHeader, !rStreamName.endsWith(".tiff"));
            if (xGraphic.is())
                mxGraphicMapper->putGraphic(rStreamName, xGraphic);
        }
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

void GraphicHelper::setGraphicMapper(css::uno::Reference<css::graphic::XGraphicMapper> const & rGraphicMapper)
{
    mxGraphicMapper = rGraphicMapper;
}

void GraphicHelper::initializeGraphicMapperIfNeeded() const
{
    if (!mxGraphicMapper.is())
    {
        auto* pNonConstThis = const_cast<GraphicHelper*>(this);
        pNonConstThis->mxGraphicMapper = graphic::GraphicMapper::create(mxContext);
    }
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
