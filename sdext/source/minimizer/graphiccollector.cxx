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


#include "graphiccollector.hxx"
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>

#include "impoptimizer.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::presentation;

const DeviceInfo& GraphicCollector::GetDeviceInfo( const Reference< XComponentContext >& rxFact )
{
    static DeviceInfo aDeviceInfo;
    if( !aDeviceInfo.Width )
    {
        try
        {
            Reference< XDesktop2 > xDesktop = Desktop::create( rxFact );
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

static void ImpAddEntity( std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities, const GraphicSettings& rGraphicSettings, const GraphicCollector::GraphicUser& rUser )
{
    if ( !rGraphicSettings.mbEmbedLinkedGraphics )
        return;

    auto aIter = std::find_if(rGraphicEntities.begin(), rGraphicEntities.end(),
        [&rUser](const GraphicCollector::GraphicEntity& rGraphicEntity) {
            return rGraphicEntity.maUser[ 0 ].mxGraphic == rUser.mxGraphic;
        });
    if ( aIter == rGraphicEntities.end() )
    {
        GraphicCollector::GraphicEntity aEntity( rUser );
        rGraphicEntities.push_back( aEntity );
    }
    else
    {
        if ( rUser.maLogicalSize.Width > aIter->maLogicalSize.Width )
            aIter->maLogicalSize.Width = rUser.maLogicalSize.Width;
        if ( rUser.maLogicalSize.Height > aIter->maLogicalSize.Height )
            aIter->maLogicalSize.Height = rUser.maLogicalSize.Height;
        aIter->maUser.push_back( rUser );
    }
}

static void ImpAddGraphicEntity( const Reference< XComponentContext >& rxMSF, Reference< XShape > const & rxShape, const GraphicSettings& rGraphicSettings, std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities )
{
    Reference< XGraphic > xGraphic;
    Reference< XPropertySet > xShapePropertySet( rxShape, UNO_QUERY_THROW );
    if ( !(xShapePropertySet->getPropertyValue( "Graphic" ) >>= xGraphic) )
        return;

    text::GraphicCrop aGraphicCropLogic( 0, 0, 0, 0 );

    GraphicCollector::GraphicUser aUser;
    aUser.mxShape = rxShape;
    aUser.mbFillBitmap = false;
    aUser.mxGraphic = xGraphic;
    xShapePropertySet->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropLogic;
    awt::Size aLogicalSize( rxShape->getSize() );

    // calculating the logical size, as if there were no cropping
    if ( aGraphicCropLogic.Left || aGraphicCropLogic.Right || aGraphicCropLogic.Top || aGraphicCropLogic.Bottom )
    {
        awt::Size aSize100thMM( GraphicCollector::GetOriginalSize( rxMSF, xGraphic ) );
        if ( aSize100thMM.Width && aSize100thMM.Height )
        {
            awt::Size aCropSize( aSize100thMM.Width - ( aGraphicCropLogic.Left + aGraphicCropLogic.Right ),
                                 aSize100thMM.Height - ( aGraphicCropLogic.Top + aGraphicCropLogic.Bottom ));
            if ( aCropSize.Width && aCropSize.Height )
            {
                awt::Size aNewLogSize( static_cast< sal_Int32 >( static_cast< double >( aSize100thMM.Width * aLogicalSize.Width ) / aCropSize.Width ),
                    static_cast< sal_Int32 >( static_cast< double >( aSize100thMM.Height * aLogicalSize.Height ) / aCropSize.Height ) );
                aLogicalSize = aNewLogSize;
            }
        }
    }
    aUser.maGraphicCropLogic = aGraphicCropLogic;
    aUser.maLogicalSize = aLogicalSize;
    ImpAddEntity( rGraphicEntities, rGraphicSettings, aUser );
}

static void ImpAddFillBitmapEntity( const Reference< XComponentContext >& rxMSF, const Reference< XPropertySet >& rxPropertySet, const awt::Size& rLogicalSize,
    std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities, const GraphicSettings& rGraphicSettings, const Reference< XPropertySet >& rxPagePropertySet )
{
    try
    {
        FillStyle eFillStyle;
        if ( rxPropertySet->getPropertyValue( "FillStyle" ) >>= eFillStyle )
        {
            if ( eFillStyle == FillStyle_BITMAP )
            {
                Reference< XBitmap > xFillBitmap;
                if ( rxPropertySet->getPropertyValue( "FillBitmap" ) >>= xFillBitmap )
                {
                    Reference< XGraphic > xGraphic( xFillBitmap, UNO_QUERY_THROW );
                    awt::Size aLogicalSize( rLogicalSize );
                    Reference< XPropertySetInfo > axPropSetInfo( rxPropertySet->getPropertySetInfo() );
                    if ( axPropSetInfo.is() )
                    {
                        if ( axPropSetInfo->hasPropertyByName( "FillBitmapMode" ) )
                        {
                            BitmapMode eBitmapMode;
                            if ( rxPropertySet->getPropertyValue( "FillBitmapMode" ) >>= eBitmapMode )
                            {
                                if ( ( eBitmapMode == BitmapMode_REPEAT ) || ( eBitmapMode == BitmapMode_NO_REPEAT ) )
                                {
                                    bool bLogicalSize = false;
                                    awt::Size aSize( 0, 0 );
                                    if ( ( rxPropertySet->getPropertyValue( "FillBitmapLogicalSize" ) >>= bLogicalSize )
                                      && ( rxPropertySet->getPropertyValue( "FillBitmapSizeX" ) >>= aSize.Width )
                                      && ( rxPropertySet->getPropertyValue( "FillBitmapSizeY" ) >>= aSize.Height ) )
                                    {
                                        if ( bLogicalSize )
                                        {
                                            if ( !aSize.Width || !aSize.Height )
                                            {
                                                awt::Size aSize100thMM( GraphicCollector::GetOriginalSize( rxMSF, xGraphic ) );
                                                if ( aSize100thMM.Width && aSize100thMM.Height )
                                                    aLogicalSize = aSize100thMM;
                                            }
                                            else
                                                aLogicalSize = aSize;
                                        }
                                        else
                                        {
                                            aLogicalSize.Width = sal::static_int_cast< sal_Int32 >( ( static_cast< double >( aLogicalSize.Width ) * aSize.Width ) / -100.0 );
                                            aLogicalSize.Height = sal::static_int_cast< sal_Int32 >( ( static_cast< double >( aLogicalSize.Height ) * aSize.Height ) / -100.0 );
                                        }
                                    }
                                }
                            }
                        }
                    }
                    GraphicCollector::GraphicUser aUser;
                    aUser.mxPropertySet = rxPropertySet;
                    aUser.mxGraphic = xGraphic;
                    aUser.mbFillBitmap = true;
                    aUser.maLogicalSize = aLogicalSize;
                    aUser.mxPagePropertySet = rxPagePropertySet;
                    ImpAddEntity( rGraphicEntities, rGraphicSettings, aUser );
                }
            }
        }
    }
    catch( Exception& )
    {
    }
}

static void ImpCollectBackgroundGraphic( const Reference< XComponentContext >& rxMSF, const Reference< XDrawPage >& rxDrawPage, const GraphicSettings& rGraphicSettings, std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities )
{
    try
    {
        awt::Size aLogicalSize( 28000, 21000 );
        Reference< XPropertySet > xPropertySet( rxDrawPage, UNO_QUERY_THROW );
        xPropertySet->getPropertyValue( "Width" ) >>= aLogicalSize.Width;
        xPropertySet->getPropertyValue( "Height" ) >>= aLogicalSize.Height;

        Reference< XPropertySet > xBackgroundPropSet;
        if ( xPropertySet->getPropertyValue( "Background" ) >>= xBackgroundPropSet )
            ImpAddFillBitmapEntity( rxMSF, xBackgroundPropSet, aLogicalSize, rGraphicEntities, rGraphicSettings, xPropertySet );
    }
    catch( Exception& )
    {
    }
}

static void ImpCollectGraphicObjects( const Reference< XComponentContext >& rxMSF, const Reference< XShapes >& rxShapes, const GraphicSettings& rGraphicSettings, std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities )
{
    for ( sal_Int32 i = 0; i < rxShapes->getCount(); i++ )
    {
        try
        {
            Reference< XShape > xShape( rxShapes->getByIndex( i ), UNO_QUERY_THROW );
            const OUString sShapeType( xShape->getShapeType() );
            if ( sShapeType == "com.sun.star.drawing.GroupShape" )
            {
                Reference< XShapes > xShapes( xShape, UNO_QUERY_THROW );
                ImpCollectGraphicObjects( rxMSF, xShapes, rGraphicSettings, rGraphicEntities );
                continue;
            }

            if ( sShapeType == "com.sun.star.drawing.GraphicObjectShape" )
                ImpAddGraphicEntity( rxMSF, xShape, rGraphicSettings, rGraphicEntities );

            // now check for a fillstyle
            Reference< XPropertySet > xEmptyPagePropSet;
            Reference< XPropertySet > xShapePropertySet( xShape, UNO_QUERY_THROW );
            awt::Size aLogicalSize( xShape->getSize() );
            ImpAddFillBitmapEntity( rxMSF, xShapePropertySet, aLogicalSize, rGraphicEntities, rGraphicSettings, xEmptyPagePropSet );
        }
        catch( Exception& )
        {
        }
    }
}

awt::Size GraphicCollector::GetOriginalSize( const Reference< XComponentContext >& rxMSF, const Reference< XGraphic >& rxGraphic )
{
    awt::Size aSize100thMM( 0, 0 );
    Reference< XPropertySet > xGraphicPropertySet( rxGraphic, UNO_QUERY_THROW );
    if ( xGraphicPropertySet->getPropertyValue( "Size100thMM" ) >>= aSize100thMM )
    {
        if ( !aSize100thMM.Width && !aSize100thMM.Height )
        {   // MAPMODE_PIXEL USED :-(
            awt::Size aSourceSizePixel( 0, 0 );
            if ( xGraphicPropertySet->getPropertyValue( "SizePixel" ) >>= aSourceSizePixel )
            {
                const DeviceInfo& rDeviceInfo( GraphicCollector::GetDeviceInfo( rxMSF ) );
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

void GraphicCollector::CollectGraphics( const Reference< XComponentContext >& rxMSF, const Reference< XModel >& rxModel,
        const GraphicSettings& rGraphicSettings, std::vector< GraphicCollector::GraphicEntity >& rGraphicList )
{
    try
    {
        sal_Int32 i;
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        for ( i = 0; i < xDrawPages->getCount(); i++ )
        {
            Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
            ImpCollectBackgroundGraphic( rxMSF, xDrawPage, rGraphicSettings, rGraphicList );
            ImpCollectGraphicObjects( rxMSF, xDrawPage, rGraphicSettings, rGraphicList );

            Reference< XPresentationPage > xPresentationPage( xDrawPage, UNO_QUERY_THROW );
            Reference< XDrawPage > xNotesPage( xPresentationPage->getNotesPage() );
            ImpCollectBackgroundGraphic( rxMSF, xNotesPage, rGraphicSettings, rGraphicList );
            ImpCollectGraphicObjects( rxMSF, xNotesPage, rGraphicSettings, rGraphicList );
        }
        Reference< XMasterPagesSupplier > xMasterPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY_THROW );
        for ( i = 0; i < xMasterPages->getCount(); i++ )
        {
            Reference< XDrawPage > xMasterPage( xMasterPages->getByIndex( i ), UNO_QUERY_THROW );
            ImpCollectBackgroundGraphic( rxMSF, xMasterPage, rGraphicSettings, rGraphicList );
            ImpCollectGraphicObjects( rxMSF, xMasterPage, rGraphicSettings, rGraphicList );
        }

        for( auto& rGraphic : rGraphicList )
        {
            // check if it is possible to remove the crop area
            rGraphic.mbRemoveCropArea = rGraphicSettings.mbRemoveCropArea;
            if ( rGraphic.mbRemoveCropArea )
            {
                std::vector< GraphicCollector::GraphicUser >::iterator aGUIter( rGraphic.maUser.begin() );
                while( rGraphic.mbRemoveCropArea && ( aGUIter != rGraphic.maUser.end() ) )
                {
                    if ( aGUIter->maGraphicCropLogic.Left || aGUIter->maGraphicCropLogic.Top
                        || aGUIter->maGraphicCropLogic.Right || aGUIter->maGraphicCropLogic.Bottom )
                    {
                        if ( aGUIter == rGraphic.maUser.begin() )
                            rGraphic.maGraphicCropLogic = aGUIter->maGraphicCropLogic;
                        else if ( ( rGraphic.maGraphicCropLogic.Left != aGUIter->maGraphicCropLogic.Left )
                            || ( rGraphic.maGraphicCropLogic.Top != aGUIter->maGraphicCropLogic.Top )
                            || ( rGraphic.maGraphicCropLogic.Right != aGUIter->maGraphicCropLogic.Right )
                            || ( rGraphic.maGraphicCropLogic.Bottom != aGUIter->maGraphicCropLogic.Bottom ) )
                        {
                            rGraphic.mbRemoveCropArea = false;
                        }
                    }
                    else
                        rGraphic.mbRemoveCropArea = false;
                    ++aGUIter;
                }
            }
            if ( !rGraphic.mbRemoveCropArea )
                rGraphic.maGraphicCropLogic = text::GraphicCrop( 0, 0, 0, 0 );
        }
    }
    catch ( Exception& )
    {
    }
}

static void ImpCountGraphicObjects( const Reference< XComponentContext >& rxMSF, const Reference< XShapes >& rxShapes, const GraphicSettings& rGraphicSettings, sal_Int32& rnGraphics )
{
    for ( sal_Int32 i = 0; i < rxShapes->getCount(); i++ )
    {
        try
        {
            Reference< XShape > xShape( rxShapes->getByIndex( i ), UNO_QUERY_THROW );
            const OUString sShapeType( xShape->getShapeType() );
            if ( sShapeType == "com.sun.star.drawing.GroupShape" )
            {
                Reference< XShapes > xShapes( xShape, UNO_QUERY_THROW );
                ImpCountGraphicObjects( rxMSF, xShapes, rGraphicSettings, rnGraphics );
                continue;
            }

            if ( sShapeType == "com.sun.star.drawing.GraphicObjectShape" )
            {
                rnGraphics++;
            }

            // now check for a fillstyle
            Reference< XPropertySet > xShapePropertySet( xShape, UNO_QUERY_THROW );
            FillStyle eFillStyle;
            if ( xShapePropertySet->getPropertyValue( "FillStyle" ) >>= eFillStyle )
            {
                if ( eFillStyle == FillStyle_BITMAP )
                {
                    rnGraphics++;
                }
            }
        }
        catch( Exception& )
        {
        }
    }
}

static void ImpCountBackgroundGraphic(
    const Reference< XDrawPage >& rxDrawPage, sal_Int32& rnGraphics )
{
    try
    {
        awt::Size aLogicalSize( 28000, 21000 );
        Reference< XPropertySet > xPropertySet( rxDrawPage, UNO_QUERY_THROW );
        xPropertySet->getPropertyValue( "Width" ) >>= aLogicalSize.Width;
        xPropertySet->getPropertyValue( "Height" ) >>= aLogicalSize.Height;

        Reference< XPropertySet > xBackgroundPropSet;
        if ( xPropertySet->getPropertyValue( "Background" ) >>= xBackgroundPropSet )
        {
            FillStyle eFillStyle;
            if ( xBackgroundPropSet->getPropertyValue( "FillStyle" ) >>= eFillStyle )
            {
                if ( eFillStyle == FillStyle_BITMAP )
                {
                    rnGraphics++;
                }
            }
        }
    }
    catch( Exception& )
    {
    }
}

void GraphicCollector::CountGraphics( const Reference< XComponentContext >& rxMSF, const Reference< XModel >& rxModel,
        const GraphicSettings& rGraphicSettings, sal_Int32& rnGraphics )
{
    try
    {
        sal_Int32 i;
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        for ( i = 0; i < xDrawPages->getCount(); i++ )
        {
            Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
            ImpCountBackgroundGraphic( xDrawPage, rnGraphics );
            ImpCountGraphicObjects( rxMSF, xDrawPage, rGraphicSettings, rnGraphics );

            Reference< XPresentationPage > xPresentationPage( xDrawPage, UNO_QUERY_THROW );
            Reference< XDrawPage > xNotesPage( xPresentationPage->getNotesPage() );
            ImpCountBackgroundGraphic( xNotesPage, rnGraphics );
            ImpCountGraphicObjects( rxMSF, xNotesPage, rGraphicSettings, rnGraphics );
        }
        Reference< XMasterPagesSupplier > xMasterPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY_THROW );
        for ( i = 0; i < xMasterPages->getCount(); i++ )
        {
            Reference< XDrawPage > xMasterPage( xMasterPages->getByIndex( i ), UNO_QUERY_THROW );
            ImpCountBackgroundGraphic( xMasterPage, rnGraphics );
            ImpCountGraphicObjects( rxMSF, xMasterPage, rGraphicSettings, rnGraphics );
        }
    }
    catch ( Exception& )
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
