/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fillproperties.cxx,v $
 * $Revision: 1.7.6.1 $
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

#include "oox/drawingml/fillproperties.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include "oox/core/modelobjectcontainer.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "properties.hxx"
#include "tokens.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::oox::core::ModelObjectContainer;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {

// ============================================================================

namespace {

static const sal_Int32 spnDefaultFillIds[ FillId_END ] =
{
    PROP_FillStyle,
    PROP_FillColor,
    PROP_FillTransparence,
    PROP_FillGradient,
    PROP_FillBitmap,
    PROP_FillBitmapMode,
    PROP_FillBitmapTile,
    PROP_FillBitmapStretch,
    PROP_FillBitmapLogicalSize,
    PROP_FillBitmapSizeX,
    PROP_FillBitmapSizeY,
    PROP_FillBitmapOffsetX,
    PROP_FillBitmapOffsetY,
    PROP_FillBitmapRectanglePoint
};

static const sal_Int32 spnDefaultPicIds[ FillId_END ] =
{
    PROP_FillStyle,
    PROP_FillColor,
    PROP_FillTransparence,
    PROP_FillGradient,
    PROP_Graphic,
    PROP_FillBitmapMode,
    PROP_FillBitmapTile,
    PROP_FillBitmapStretch,
    PROP_FillBitmapLogicalSize,
    PROP_FillBitmapSizeX,
    PROP_FillBitmapSizeY,
    PROP_FillBitmapOffsetX,
    PROP_FillBitmapOffsetY,
    PROP_FillBitmapRectanglePoint
};

BitmapMode lclGetBitmapMode( sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_tile:      return BitmapMode_REPEAT;
        case XML_stretch:   return BitmapMode_STRETCH;
    }
    return BitmapMode_NO_REPEAT;
}

RectanglePoint lclGetRectanglePoint( sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_tl:    return RectanglePoint_LEFT_TOP;
        case XML_t:     return RectanglePoint_MIDDLE_TOP;
        case XML_tr:    return RectanglePoint_RIGHT_TOP;
        case XML_l:     return RectanglePoint_LEFT_MIDDLE;
        case XML_ctr:   return RectanglePoint_MIDDLE_MIDDLE;
        case XML_r:     return RectanglePoint_RIGHT_MIDDLE;
        case XML_bl:    return RectanglePoint_LEFT_BOTTOM;
        case XML_b:     return RectanglePoint_MIDDLE_BOTTOM;
        case XML_br:    return RectanglePoint_RIGHT_BOTTOM;
    }
    return RectanglePoint_LEFT_TOP;
}

const awt::DeviceInfo& lclGetDeviceInfo( const XmlFilterBase& rFilter )
{
    static awt::DeviceInfo aDeviceInfo;
    if( aDeviceInfo.Width <= 0 )
    {
        try
        {
            Reference< frame::XFramesSupplier > xDesktop( rFilter.getGlobalFactory()->createInstance(
                    CREATE_OUSTRING( "com.sun.star.frame.Desktop" ) ), UNO_QUERY_THROW );
            Reference< frame::XFrame > xFrame( xDesktop->getActiveFrame() );
            Reference< awt::XWindow > xWindow( xFrame->getContainerWindow() );
            Reference< awt::XDevice > xDevice( xWindow, UNO_QUERY_THROW );
            aDeviceInfo = xDevice->getInfo();
        }
        catch( Exception& )
        {
        }
    }
    return aDeviceInfo;
}

const awt::Size lclGetOriginalSize( const XmlFilterBase& rFilter, const Reference< XGraphic >& rxGraphic )
{
    awt::Size aSize100thMM( 0, 0 );
    try
    {
        Reference< beans::XPropertySet > xGraphicPropertySet( rxGraphic, UNO_QUERY_THROW );
        if( xGraphicPropertySet->getPropertyValue( CREATE_OUSTRING( "Size100thMM" ) ) >>= aSize100thMM )
        {
            if( !aSize100thMM.Width && !aSize100thMM.Height )
            {   // MAPMODE_PIXEL USED :-(
                awt::Size aSourceSizePixel( 0, 0 );
                if( xGraphicPropertySet->getPropertyValue( CREATE_OUSTRING( "SizePixel" ) ) >>= aSourceSizePixel )
                {
                    const awt::DeviceInfo& rDeviceInfo = lclGetDeviceInfo( rFilter );
                    if( rDeviceInfo.PixelPerMeterX && rDeviceInfo.PixelPerMeterY )
                    {
                        aSize100thMM.Width = static_cast< sal_Int32 >( ( aSourceSizePixel.Width * 100000.0 ) / rDeviceInfo.PixelPerMeterX );
                        aSize100thMM.Height = static_cast< sal_Int32 >( ( aSourceSizePixel.Height * 100000.0 ) / rDeviceInfo.PixelPerMeterY );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
    }
    return aSize100thMM;
}

Reference< XGraphic > lclTransformGraphic( const Reference< XGraphic >& rxGraphic,
        const Color& rFromColor, const Color& rToColor, const XmlFilterBase& rFilter, sal_Int32 nPhClr )
{
    if( rxGraphic.is() && rFromColor.isUsed() && rToColor.isUsed() ) try
    {
        sal_Int32 nFromColor = rFromColor.getColor( rFilter, nPhClr );
        sal_Int32 nToColor = rToColor.getColor( rFilter, nPhClr );
        if ( (nFromColor != nToColor) || rToColor.hasTransparence() )
        {
            sal_Int16 nToTransparence = rToColor.getTransparence();
            sal_Int8 nToAlpha = static_cast< sal_Int8 >( (100 - nToTransparence) / 39.062 );   // ?!? correct ?!?
            Reference< XGraphicTransformer > xTransformer( rxGraphic, UNO_QUERY_THROW );
            return xTransformer->colorChange( rxGraphic, nFromColor, 9, nToColor, nToAlpha );
        }
    }
    catch( Exception& )
    {
    }
    return rxGraphic;
}

} // namespace

// ============================================================================

FillPropertyIds::FillPropertyIds( const sal_Int32* pnPropertyIds, bool bNamedFillGradient, bool bNamedFillBitmap, bool bTransformGraphic ) :
    mpnPropertyIds( pnPropertyIds ),
    mbNamedFillGradient( bNamedFillGradient ),
    mbNamedFillBitmap( bNamedFillBitmap ),
    mbTransformGraphic( bTransformGraphic )
{
    OSL_ENSURE( mpnPropertyIds != 0, "FillPropertyIds::FillPropertyIds - missing property identifiers" );
}

// ============================================================================

FillPropertyIds FillProperties::DEFAULT_IDS( spnDefaultFillIds, false, false, false );
FillPropertyIds FillProperties::DEFAULT_PICIDS( spnDefaultPicIds, false, false, true );

void FillProperties::assignUsed( const FillProperties& rSourceProps )
{
    moFillType.assignIfUsed( rSourceProps.moFillType );
    moRotateWithShape.assignIfUsed( rSourceProps.moRotateWithShape );

    maFillColor.assignIfUsed( rSourceProps.maFillColor );

    if( !rSourceProps.maGradientStops.empty() )
        maGradientStops = rSourceProps.maGradientStops;
    moGradientPath.assignIfUsed( rSourceProps.moGradientPath );
    moShadeAngle.assignIfUsed( rSourceProps.moShadeAngle );
    moShadeScaled.assignIfUsed( rSourceProps.moShadeScaled );
    moFlipModeToken.assignIfUsed( rSourceProps.moFlipModeToken );
    moFillToRect.assignIfUsed( rSourceProps.moFillToRect );
    moTileRect.assignIfUsed( rSourceProps.moTileRect );

    if( rSourceProps.mxGraphic.is() )
        mxGraphic = rSourceProps.mxGraphic;
    maColorChangeFrom.assignIfUsed( rSourceProps.maColorChangeFrom );
    maColorChangeTo.assignIfUsed( rSourceProps.maColorChangeTo );
    moBitmapMode.assignIfUsed( rSourceProps.moBitmapMode );
    moTileX.assignIfUsed( rSourceProps.moTileX );
    moTileY.assignIfUsed( rSourceProps.moTileY );
    moTileSX.assignIfUsed( rSourceProps.moTileSX );
    moTileSY.assignIfUsed( rSourceProps.moTileSY );
    moTileAlign.assignIfUsed( rSourceProps.moTileAlign );
}

Color FillProperties::getBestSolidColor() const
{
    Color aSolidColor;
    if( moFillType.has() ) switch( moFillType.get() )
    {
        case XML_solidFill:
            aSolidColor = maFillColor;
        break;
        case XML_gradFill:
            if( !maGradientStops.empty() )
                aSolidColor = maGradientStops.begin()->second;
        break;
        case XML_pattFill:
            aSolidColor = maPattBgColor.isUsed() ? maPattBgColor : maPattFgColor;
        break;
    }
    return aSolidColor;
}

void FillProperties::pushToPropMap( PropertyMap& rPropMap, const FillPropertyIds& rPropIds,
        const XmlFilterBase& rFilter, ModelObjectContainer& rObjContainer,
        sal_Int32 nShapeRotation, sal_Int32 nPhClr ) const
{
    if( moFillType.has() )
    {
        // "rotate with shape" not set, or set to false -> do not rotate
        if ( !moRotateWithShape.get( false ) )
            nShapeRotation = 0;

        FillStyle eFillStyle = FillStyle_NONE;
        switch( moFillType.get() )
        {
            case XML_noFill:
                eFillStyle = FillStyle_NONE;
            break;

            case XML_solidFill:
                if( maFillColor.isUsed() )
                {
                    rPropMap.setProperty( rPropIds[ FillColorId ], maFillColor.getColor( rFilter, nPhClr ) );
                    if( maFillColor.hasTransparence() )
                        rPropMap.setProperty( rPropIds[ FillTransparenceId ], maFillColor.getTransparence() );
                    eFillStyle = FillStyle_SOLID;
                }
            break;

            case XML_gradFill:
                // do not create gradient struct if property is not supported...
                if( rPropIds.has( FillGradientId ) )
                {
                    awt::Gradient aGradient;
                    aGradient.Angle = 900;
                    aGradient.StartIntensity = 100;
                    aGradient.EndIntensity = 100;

                    if( maGradientStops.size() > 1 )
                    {
                        aGradient.StartColor = maGradientStops.begin()->second.getColor( rFilter, nPhClr );
                        aGradient.EndColor = maGradientStops.rbegin()->second.getColor( rFilter, nPhClr );
                    }

                    if( moGradientPath.has() )
                    {
                        aGradient.Style = (moGradientPath.get() == XML_circle) ? awt::GradientStyle_ELLIPTICAL : awt::GradientStyle_RECT;
                        aGradient.Angle = static_cast< sal_Int16 >( (900 - (nShapeRotation / 6000)) % 3600 );
                        aGradient.XOffset = moFillToRect.has() ? getLimitedValue< sal_Int16, sal_Int32 >( moFillToRect.get().X1 / 1000, 30, 70 ) : 50;
                        aGradient.YOffset = moFillToRect.has() ? getLimitedValue< sal_Int16, sal_Int32 >( moFillToRect.get().Y1 / 1000, 30, 70 ) : 50;
                        ::std::swap( aGradient.StartColor, aGradient.EndColor );
                    }
                    else
                    {
                        aGradient.Style = awt::GradientStyle_LINEAR;
                        aGradient.Angle = static_cast< sal_Int16 >( (4500 - ((moShadeAngle.get( 0 ) - nShapeRotation) / 6000)) % 3600 );
                    }

                    // push gradient or named gradient to property map
                    if( rPropIds.mbNamedFillGradient )
                    {
                        OUString aGradientName = rObjContainer.insertFillGradient( aGradient );
                        if( aGradientName.getLength() > 0 )
                        {
                            rPropMap.setProperty( rPropIds[ FillGradientId ], aGradientName );
                            eFillStyle = FillStyle_GRADIENT;
                        }
                    }
                    else
                    {
                        rPropMap.setProperty( rPropIds[ FillGradientId ], aGradient );
                        eFillStyle = FillStyle_GRADIENT;
                    }
                }
            break;

            case XML_blipFill:
                // do not start complex graphic transformation if property is not supported...
                if( mxGraphic.is() && rPropIds.has( FillBitmapId ) )
                {
                    // TODO: "rotate with shape" is not possible with our current core

                    // created transformed graphic
                    Reference< XGraphic > xGraphic = rPropIds.mbTransformGraphic ?
                        lclTransformGraphic( mxGraphic, maColorChangeFrom, maColorChangeTo, rFilter, nPhClr ) :
                        mxGraphic;

                    if( xGraphic.is() )
                    {
                        // push bitmap or named bitmap to property map
                        if( rPropIds.mbNamedFillBitmap )
                        {
                            OUString aBitmapName = rObjContainer.insertFillBitmap( Reference< awt::XBitmap >( xGraphic, UNO_QUERY ) );
                            if( aBitmapName.getLength() > 0 )
                            {
                                rPropMap.setProperty( rPropIds[ FillBitmapId ], aBitmapName );
                                eFillStyle = FillStyle_BITMAP;
                            }
                        }
                        else
                        {
                            rPropMap.setProperty( rPropIds[ FillBitmapId ], xGraphic );
                            eFillStyle = FillStyle_BITMAP;
                        }
                    }

                    // set other bitmap properties, if bitmap has been inserted into the map
                    if( eFillStyle == FillStyle_BITMAP )
                    {
                        // bitmap mode (single, repeat, stretch)
                        BitmapMode eBitmapMode = lclGetBitmapMode( moBitmapMode.get( XML_TOKEN_INVALID ) );
                        rPropMap.setProperty( rPropIds[ FillBitmapModeId ], eBitmapMode );

                        // anchor position inside bitmap
                        RectanglePoint eRectPoint = lclGetRectanglePoint( moTileAlign.get( XML_tl ) );
                        rPropMap.setProperty( rPropIds[ FillBitmapRectanglePointId ], eRectPoint );

                        // additional settings for repeated bitmap
                        if( eBitmapMode == BitmapMode_REPEAT )
                        {
                            rPropMap.setProperty( rPropIds[ FillBitmapTileId ], true );
                            rPropMap.setProperty( rPropIds[ FillBitmapStretchId ], true );
                            rPropMap.setProperty( rPropIds[ FillBitmapLogicalSizeId ], true );

                            // size of one bitmap tile
                            awt::Size aOriginalSize = lclGetOriginalSize( rFilter, mxGraphic );
                            if( (aOriginalSize.Width > 0) && (aOriginalSize.Height > 0) )
                            {
                                sal_Int32 nFillBmpSizeX = static_cast< sal_Int32 >( (aOriginalSize.Width / 100000.0) * moTileSX.get( 100000 ) );
                                rPropMap.setProperty( rPropIds[ FillBitmapSizeXId ], nFillBmpSizeX );
                                sal_Int32 nFillBmpSizeY = static_cast< sal_Int32 >( (aOriginalSize.Height / 100000.0) * moTileSY.get( 100000 ) );
                                rPropMap.setProperty( rPropIds[ FillBitmapSizeYId ], nFillBmpSizeY );
                            }

                            // offset of the first bitmap tile
//                            if( moTileX.has() )
//                                rPropMap.setProperty( rPropIds[ FillBitmapOffsetXId ], static_cast< sal_Int16 >( moTileX.get() / 360 ) );
//                            if( moTileY.has() )
//                                rPropMap.setProperty( rPropIds[ FillBitmapOffsetYId ], static_cast< sal_Int16 >( moTileY.get() / 360 ) );
                        }
                    }
                }
            break;

            case XML_pattFill:
            {
                // todo
                Color aColor = getBestSolidColor();
                if( aColor.isUsed() )
                {
                    rPropMap.setProperty( rPropIds[ FillColorId ], aColor.getColor( rFilter, nPhClr ) );
                    if( aColor.hasTransparence() )
                        rPropMap.setProperty( rPropIds[ FillTransparenceId ], aColor.getTransparence() );
                    eFillStyle = FillStyle_SOLID;
                }
            }
            break;

            case XML_grpFill:
                // todo
                eFillStyle = FillStyle_NONE;
            break;
        }

        // set final fill style property
        rPropMap.setProperty( rPropIds[ FillStyleId ], eFillStyle );
    }
}

void FillProperties::pushToPropSet( PropertySet& rPropSet, const FillPropertyIds& rPropIds,
        const XmlFilterBase& rFilter, ModelObjectContainer& rObjContainer,
        sal_Int32 nShapeRotation, sal_Int32 nPhClr ) const
{
    PropertyMap aPropMap;
    pushToPropMap( aPropMap, rPropIds, rFilter, rObjContainer, nShapeRotation, nPhClr );
    rPropSet.setProperties( aPropMap );
}

// ============================================================================

} // namespace drawingml
} // namespace oox

