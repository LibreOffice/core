/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillproperties.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:20:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <comphelper/processfactory.hxx>
#include "oox/drawingml/fillproperties.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/GradientStyle.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::frame;

namespace oox { namespace drawingml {


const awt::DeviceInfo& GetDeviceInfo( const Reference< lang::XMultiServiceFactory >& rxFact )
{
    static awt::DeviceInfo aDeviceInfo;
    if( !aDeviceInfo.Width )
    {
        try
        {
            Reference< XFramesSupplier > xDesktop( rxFact->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ), UNO_QUERY_THROW );
            Reference< XFrame > xFrame( xDesktop->getActiveFrame() );
            Reference< XWindow > xWindow( xFrame->getContainerWindow() );
            Reference< XDevice > xDevice( xWindow, UNO_QUERY_THROW );
            aDeviceInfo = xDevice->getInfo();
        }
        catch( Exception& )
        {
        }
    }
    return aDeviceInfo;
}

// ---------------------------------------------------------------------

const awt::Size GetOriginalSize( const Reference< lang::XMultiServiceFactory >& rxFact, const Reference< XGraphic >& rxGraphic )
{
    awt::Size aSize100thMM( 0, 0 );
    Reference< XPropertySet > xGraphicPropertySet( rxGraphic, UNO_QUERY_THROW );
    static const OUString sSize100thMM = CREATE_OUSTRING( "Size100thMM" );
    if ( xGraphicPropertySet->getPropertyValue( sSize100thMM ) >>= aSize100thMM )
    {
        if ( !aSize100thMM.Width && !aSize100thMM.Height )
        {   // MAPMODE_PIXEL USED :-(
            awt::Size aSourceSizePixel( 0, 0 );
            static const OUString sSizePixel = CREATE_OUSTRING( "SizePixel" );
            if ( xGraphicPropertySet->getPropertyValue( sSizePixel ) >>= aSourceSizePixel )
            {
                const DeviceInfo& rDeviceInfo( GetDeviceInfo( rxFact ) );
                if ( rDeviceInfo.PixelPerMeterX && rDeviceInfo.PixelPerMeterY )
                {
                    aSize100thMM.Width = static_cast< sal_Int32 >( ( aSourceSizePixel.Width * 100000.0 ) / rDeviceInfo.PixelPerMeterX );
                    aSize100thMM.Height = static_cast< sal_Int32 >( ( aSourceSizePixel.Height * 100000.0 ) / rDeviceInfo.PixelPerMeterY );
                }
            }
        }
    }
    return aSize100thMM;
}

// ---------------------------------------------------------------------

FillProperties::FillProperties( sal_Int32 nContext )
: mnContext( nContext )
, maFillColor( new Color() )
, maColorChangeFrom( new Color() )
, maColorChangeTo( new Color() )
{
}
FillProperties::~FillProperties()
{
}

void FillProperties::apply( const FillPropertiesPtr& rSourceFillProperties )
{
    if ( rSourceFillProperties->moFillStyle )
        moFillStyle = rSourceFillProperties->moFillStyle;
    if ( rSourceFillProperties->maFillColor->isUsed() )
        maFillColor = rSourceFillProperties->maFillColor;
    if ( rSourceFillProperties->moBitmapMode )
        moBitmapMode = rSourceFillProperties->moBitmapMode;
    if ( rSourceFillProperties->moRotateWithShape )
        moRotateWithShape = rSourceFillProperties->moRotateWithShape;
    if ( rSourceFillProperties->moShadeScaled )
        moShadeScaled = rSourceFillProperties->moShadeScaled;
    if ( rSourceFillProperties->moShadeAngle )
        moShadeAngle = rSourceFillProperties->moShadeAngle;
    if ( rSourceFillProperties->moShadeTypeToken )
        moShadeTypeToken = rSourceFillProperties->moShadeTypeToken;
    if ( rSourceFillProperties->moFlipModeToken )
        moFlipModeToken = rSourceFillProperties->moFlipModeToken;
    if ( rSourceFillProperties->moFillToRect )
        moFillToRect = rSourceFillProperties->moFillToRect;
    if ( rSourceFillProperties->moTileRect )
        moTileRect = rSourceFillProperties->moTileRect;
    if ( rSourceFillProperties->moTileAlign )
        moTileAlign = rSourceFillProperties->moTileAlign;
    if ( rSourceFillProperties->moTileX )
        moTileX = rSourceFillProperties->moTileX;
    if ( rSourceFillProperties->moTileY )
        moTileY = rSourceFillProperties->moTileY;
    if ( rSourceFillProperties->moTileSX )
        moTileSX = rSourceFillProperties->moTileSX;
    if ( rSourceFillProperties->moTileSY )
        moTileSY = rSourceFillProperties->moTileSY;
    if ( rSourceFillProperties->mvGradientStops.size() )
        mvGradientStops = rSourceFillProperties->mvGradientStops;
    if ( rSourceFillProperties->mxGraphic.is() )
        mxGraphic = rSourceFillProperties->mxGraphic;
    if ( rSourceFillProperties->maColorChangeFrom->isUsed() )
        maColorChangeFrom = rSourceFillProperties->maColorChangeFrom;
    if ( rSourceFillProperties->maColorChangeTo->isUsed() )
        maColorChangeTo = rSourceFillProperties->maColorChangeTo;
}

void FillProperties::pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
    const Reference < XPropertySet >& xPropSet, sal_Int32 nShapeRotation ) const
{
    try
    {
        if ( !moRotateWithShape || !(*moRotateWithShape) )
            nShapeRotation = 0;

        ::com::sun::star::drawing::FillStyle eFillStyle( moFillStyle ? *moFillStyle : drawing::FillStyle_NONE );
        const rtl::OUString sFillStyle( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) );
        xPropSet->setPropertyValue( sFillStyle, Any( eFillStyle ) );
        if ( maFillColor->isUsed() )
        {
            const rtl::OUString sFillColor( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) );
            xPropSet->setPropertyValue( sFillColor, Any( maFillColor->getColor( rFilterBase ) ) );
        }
        if ( eFillStyle == drawing::FillStyle_SOLID )
        {
            if ( maFillColor->isUsed() && maFillColor->hasAlpha() )
            {
                const rtl::OUString sFillTransparence( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "FillTransparence" ) ) );
                xPropSet->setPropertyValue( sFillTransparence, Any( static_cast< sal_Int16 >( ( 100000 - maFillColor->getAlpha() ) / 1000 ) ) );
            }
        }
        else if ( eFillStyle == drawing::FillStyle_GRADIENT )
        {
            awt::Gradient aGradient;
            aGradient.Angle = 900;
            aGradient.StartIntensity = 100;
            aGradient.EndIntensity = 100;
            if ( !moShadeTypeToken )
            {
                aGradient.Style = awt::GradientStyle_LINEAR;
                aGradient.Angle = static_cast< sal_Int16 >( moShadeAngle ? 4500 - ( ( *moShadeAngle - nShapeRotation ) / 6000 ) : 900 ) % 3600;
                if ( mvGradientStops.size() > 1 )
                {
                    aGradient.StartColor = mvGradientStops[ 0 ].maColor->getColor( rFilterBase );
                    aGradient.EndColor = mvGradientStops[ mvGradientStops.size() - 1 ].maColor->getColor( rFilterBase );
                }
            }
            else
            {
                aGradient.Style = awt::GradientStyle_RECT;
                aGradient.Angle = static_cast< sal_Int16 >( 900 - ( nShapeRotation / 6000 ) ) % 3600;
                if ( mvGradientStops.size() > 1 )
                {
                    aGradient.StartColor = mvGradientStops[ mvGradientStops.size() - 1 ].maColor->getColor( rFilterBase );
                    aGradient.EndColor = mvGradientStops[ 0 ].maColor->getColor( rFilterBase );
                }
                aGradient.XOffset = 50;
                aGradient.YOffset = 50;
                if ( moFillToRect )
                {
                    sal_Int32 nX = moFillToRect->X1 / 1000;
                    sal_Int32 nY = moFillToRect->Y1 / 1000;
                    if ( nX <= 30 )
                        nX = 30;
                    if ( nY < 30 )
                        nY = 30;
                    if ( nX > 70 )
                        nX = 70;
                    if ( nY > 70 )
                        nY = 70;
                    aGradient.XOffset = static_cast< sal_Int16 >( nX );
                    aGradient.YOffset = static_cast< sal_Int16 >( nY );
                }
            }
            static const OUString sFillGradient = CREATE_OUSTRING( "FillGradient" );
            xPropSet->setPropertyValue( sFillGradient, Any( aGradient ) );
        }
        else if ( eFillStyle == drawing::FillStyle_BITMAP )
        {
            // TODO: RotateWithShape is not possible with our current core

            drawing::BitmapMode eBitmapMode( moBitmapMode ? *moBitmapMode : drawing::BitmapMode_NO_REPEAT );
            static const OUString sFillBitmapMode = CREATE_OUSTRING( "FillBitmapMode" );
            xPropSet->setPropertyValue( sFillBitmapMode, Any( eBitmapMode ) );

            if ( eBitmapMode == drawing::BitmapMode_REPEAT )
            {
                static const OUString sFillBitmapTile = CREATE_OUSTRING( "FillBitmapTile" );
                xPropSet->setPropertyValue( sFillBitmapTile, Any( sal_True ) );
                static const OUString sFillBitmapStretch = CREATE_OUSTRING( "FillBitmapStretch" );
                xPropSet->setPropertyValue( sFillBitmapStretch, Any( sal_True ) );
                static const OUString sFillBitmapLogicalSize = CREATE_OUSTRING( "FillBitmapLogicalSize" );
                xPropSet->setPropertyValue( sFillBitmapLogicalSize, Any( sal_True ) );

                sal_Int32 nSX = moTileSX ? *moTileSX : 100000;
                sal_Int32 nSY = moTileSY ? *moTileSY : 100000;

                Reference< lang::XMultiServiceFactory > xMSFT( rFilterBase.getServiceFactory(), UNO_QUERY_THROW );
                awt::Size aOriginalSize = GetOriginalSize( xMSFT, mxGraphic );

                sal_Int32 nSizeX = static_cast< sal_Int32 >( ( aOriginalSize.Width / 100000.0 ) * nSX );
                sal_Int32 nSizeY = static_cast< sal_Int32 >( ( aOriginalSize.Height / 100000.0 ) * nSY );
                static const OUString sFillBitmapSizeX = CREATE_OUSTRING( "FillBitmapSizeX" );
                xPropSet->setPropertyValue( sFillBitmapSizeX, Any( nSizeX ) );
                static const OUString sFillBitmapSizeY = CREATE_OUSTRING( "FillBitmapSizeY" );
                xPropSet->setPropertyValue( sFillBitmapSizeY, Any( nSizeY ) );
/*
                if ( moTileX )
                {
                    static const OUString sFillBitmapOffsetX = CREATE_OUSTRING( "FillBitmapOffsetX" );
                    xPropSet->setPropertyValue( sFillBitmapOffsetX, Any( *moTileX / 360 ) );
                }
                if ( moTileY )
                {
                    static const OUString sFillBitmapOffsetY = CREATE_OUSTRING( "FillBitmapOffsetY" );
                    xPropSet->setPropertyValue( sFillBitmapOffsetY, Any( *moTileY / 360 ) );
                }
*/
            }
            if ( moTileAlign )
            {
                drawing::RectanglePoint eRectPoint = drawing::RectanglePoint_LEFT_TOP;
                switch ( *moTileAlign )
                {
                    case XML_tl :   eRectPoint = drawing::RectanglePoint_LEFT_TOP; break;
                    case XML_t :    eRectPoint = drawing::RectanglePoint_MIDDLE_TOP; break;
                    case XML_tr :   eRectPoint = drawing::RectanglePoint_RIGHT_TOP; break;
                    case XML_l :    eRectPoint = drawing::RectanglePoint_LEFT_MIDDLE; break;
                    case XML_ctr :  eRectPoint = drawing::RectanglePoint_MIDDLE_MIDDLE; break;
                    case XML_r :    eRectPoint = drawing::RectanglePoint_RIGHT_MIDDLE; break;
                    case XML_bl :   eRectPoint = drawing::RectanglePoint_LEFT_BOTTOM; break;
                    case XML_b :    eRectPoint = drawing::RectanglePoint_MIDDLE_BOTTOM; break;
                    case XML_br :   eRectPoint = drawing::RectanglePoint_RIGHT_BOTTOM; break;
                }
                static const OUString sFillBitmapRectanglePoint = CREATE_OUSTRING( "FillBitmapRectanglePoint" );
                xPropSet->setPropertyValue( sFillBitmapRectanglePoint, Any( eRectPoint ) );
            }
        }
    }
    catch( Exception& )
    {
//....
    }
    createTransformedGraphic( rFilterBase, xPropSet );
}

void FillProperties::createTransformedGraphic( const oox::core::XmlFilterBase& rFilterBase, const Reference < XPropertySet >& xPropSet ) const
{
    if( mxGraphic.is() )
    {
        if ( mnContext == XML_pic )
        {
            Reference< XGraphic > xGraphic( mxGraphic );
            if ( maColorChangeFrom.get() && maColorChangeTo.get() )
            {
                sal_Int32 nClrChangeFrom = maColorChangeFrom->getColor( rFilterBase );
                sal_Int32 nClrChangeTo = maColorChangeTo->getColor( rFilterBase );
                sal_Int32 nAlphaTo = maColorChangeTo->getAlpha();
                if ( ( nClrChangeFrom != nClrChangeTo ) || ( maColorChangeTo->hasAlpha() && ( nAlphaTo != 1000000 ) ) )
                {
                    Reference< XGraphicTransformer > xTransformer( xGraphic, UNO_QUERY );
                    if ( xTransformer.is() )
                        xGraphic = xTransformer->colorChange( xGraphic, nClrChangeFrom, 9, nClrChangeTo, static_cast< sal_Int8 >( ( nAlphaTo / 39062 ) ) );
                }
            }
            static const OUString sGraphic( CREATE_OUSTRING( "Graphic" ) );
            xPropSet->setPropertyValue( sGraphic, Any( xGraphic ) );
        }
        else if ( mnContext == XML_spPr )
        {
            static const OUString sFillBitmap( CREATE_OUSTRING( "FillBitmap" ) );
            xPropSet->setPropertyValue( sFillBitmap, Any( mxGraphic ) );
        }
    }
}

} }
