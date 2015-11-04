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


#include "impoptimizer.hxx"
#include "pppoptimizer.hxx"
#include "graphiccollector.hxx"
#include "pagecollector.hxx"
#include "informationdialog.hxx"

#include <vector>
#include "com/sun/star/util/URL.hpp"
#include "com/sun/star/util/XURLTransformer.hpp"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/presentation/XPresentation.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;

void ImpExtractCustomShow( const Reference< XModel >& rxModel, const OUString& rCustomShowName )
{
    vector< Reference< XDrawPage > > vNonUsedPageList;
    try
    {
        PageCollector::CollectNonCustomShowPages( rxModel, rCustomShowName, vNonUsedPageList );
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        vector< Reference< XDrawPage > >::iterator aIter( vNonUsedPageList.begin() );
        while( aIter != vNonUsedPageList.end() )
            xDrawPages->remove( *aIter++ );
    }
    catch( Exception& )
    {

    }
}

void ImpDeleteUnusedMasterPages( const Reference< XModel >& rxModel )
{
    vector< PageCollector::MasterPageEntity > aMasterPageList;
    PageCollector::CollectMasterPages( rxModel, aMasterPageList );

    // now master pages that are not marked can be deleted
    Reference< XMasterPagesSupplier > xMasterPagesSupplier( rxModel, UNO_QUERY_THROW );
    Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY_THROW );
    vector< PageCollector::MasterPageEntity >::iterator aIter( aMasterPageList.begin() );
    while( aIter != aMasterPageList.end() )
    {
        if ( !aIter->bUsed )
            xMasterPages->remove( aIter->xMasterPage );
        ++aIter;
    }
}

void ImpDeleteHiddenSlides(  const Reference< XModel >& rxModel )
{
    try
    {
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        for( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
        {
            Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
            Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY_THROW );

            bool bVisible = true;
            const OUString sVisible( "Visible"  );
            if ( xPropSet->getPropertyValue( sVisible ) >>= bVisible )
            {
                if (!bVisible )
                {
                    xDrawPages->remove( xDrawPage );
                    i--;
                }
            }
        }
    }
    catch( Exception& )
    {
    }
}

void ImpDeleteNotesPages( const Reference< XModel >& rxModel )
{
    try
    {
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        sal_Int32 i, nPages = xDrawPages->getCount();
        for( i = 0; i < nPages; i++ )
        {
            Reference< XPresentationPage > xPresentationPage( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
            Reference< XPropertySet > xPropSet( xPresentationPage->getNotesPage(), UNO_QUERY_THROW );
            Reference< XShapes > xShapes( xPropSet, UNO_QUERY_THROW );
            while( xShapes->getCount() )
                xShapes->remove( Reference< XShape >( xShapes->getByIndex( xShapes->getCount() - 1 ), UNO_QUERY_THROW ) );

            const OUString sLayout( "Layout"  );
            xPropSet->setPropertyValue( sLayout, Any( (sal_Int16)21 ) );
        }
    }
    catch( Exception& )
    {
    }
}

void ImpConvertOLE( const Reference< XModel >& rxModel, sal_Int32 nOLEOptimizationType )
{
    try
    {
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        for ( sal_Int32 i = 0; i < xDrawPages->getCount(); i++ )
        {
            Reference< XShapes > xShapes( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
            for ( sal_Int32 j = 0; j < xShapes->getCount(); j++ )
            {
                const OUString sOLE2Shape( "com.sun.star.drawing.OLE2Shape"  );
                Reference< XShape > xShape( xShapes->getByIndex( j ), UNO_QUERY_THROW );
                if ( xShape->getShapeType() == sOLE2Shape )
                {
                    Reference< XPropertySet > xPropSet( xShape, UNO_QUERY_THROW );

                    bool bConvertOLE = nOLEOptimizationType == 0;
                    if ( nOLEOptimizationType == 1 )
                    {
                        bool bIsInternal = true;
                        xPropSet->getPropertyValue( "IsInternal" ) >>= bIsInternal;
                        bConvertOLE = !bIsInternal;
                    }
                    if ( bConvertOLE )
                    {
                        Reference< XGraphic > xGraphic;
                        if ( xPropSet->getPropertyValue( "Graphic" ) >>= xGraphic )
                        {
                            const OUString sGraphicShape( "com.sun.star.drawing.GraphicObjectShape"  );
                            Reference< XMultiServiceFactory > xFact( rxModel, UNO_QUERY_THROW );
                            Reference< XShape > xShape2( xFact->createInstance( sGraphicShape ), UNO_QUERY_THROW );
                            xShapes->add( xShape2 );
                            xShape2->setPosition( xShape->getPosition() );
                            xShape2->setSize( xShape->getSize() );
                            Reference< XPropertySet > xPropSet2( xShape2, UNO_QUERY_THROW );
                            xPropSet2->setPropertyValue( "Graphic", Any( xGraphic ) );
                            xShapes->remove( xShape );
                            xPropSet2->setPropertyValue( "ZOrder", Any( j ) );
                        }
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
    }
}

void ImpCompressGraphic( Reference< XGraphicProvider >& rxGraphicProvider, const Reference< XGraphic >& rxGraphic, Reference< XOutputStream >& rxOutputStream,
    const OUString& rDestMimeType, const awt::Size& rLogicalSize, sal_Int32 nJPEGQuality, sal_Int32 nImageResolution, bool bRemoveCropping, const text::GraphicCrop& rGraphicCropLogic )
{
    try
    {
        if ( rxGraphicProvider.is() && rxOutputStream.is() )
        {
            Sequence< PropertyValue > aFilterData( 8 );
            aFilterData[ 0 ].Name = "ImageResolution";
            aFilterData[ 0 ].Value <<= nImageResolution;
            aFilterData[ 1 ].Name = "ColorMode";      // todo: jpeg color mode (0->true color, 1->greyscale)
            aFilterData[ 1 ].Value <<= (sal_Int32)0;
            aFilterData[ 2 ].Name = "Quality";        // quality that is used if we export to jpeg
            aFilterData[ 2 ].Value <<= nJPEGQuality;
            aFilterData[ 3 ].Name = "Compression";    // compression that is used if we export to png
            aFilterData[ 3 ].Value <<= (sal_Int32)6;
            aFilterData[ 4 ].Name = "Interlaced";     // interlaced is turned off if we export to png
            aFilterData[ 4 ].Value <<= (sal_Int32)0;
            aFilterData[ 5 ].Name = "LogicalSize";
            aFilterData[ 5 ].Value <<= rLogicalSize;
            aFilterData[ 6 ].Name = "RemoveCropArea";
            aFilterData[ 6 ].Value <<= bRemoveCropping;
            aFilterData[ 7 ].Name = "GraphicCropLogic";
            aFilterData[ 7 ].Value <<= rGraphicCropLogic;

            Sequence< PropertyValue > aArgs( 3 );
            aArgs[ 0 ].Name = "MimeType";             // the GraphicProvider is using "MimeType", the GraphicExporter "MediaType"...
            aArgs[ 0 ].Value <<= rDestMimeType;
            aArgs[ 1 ].Name = "OutputStream";
            aArgs[ 1 ].Value <<= rxOutputStream;
            aArgs[ 2 ].Name = "FilterData";
            aArgs[ 2 ].Value <<= aFilterData;

            rxGraphicProvider->storeGraphic( rxGraphic, aArgs );
        }
    }
    catch( Exception& )
    {
    }
}

Reference< XGraphic > ImpCompressGraphic( const Reference< XComponentContext >& rxContext,
    const Reference< XGraphic >& xGraphic, const awt::Size& aLogicalSize, const text::GraphicCrop& aGraphicCropLogic,
        const GraphicSettings& rGraphicSettings )
{
    Reference< XGraphic > xNewGraphic;
    try
    {
        OUString aSourceMimeType;
        Reference< XPropertySet > xGraphicPropertySet( xGraphic, UNO_QUERY_THROW );
        if ( xGraphicPropertySet->getPropertyValue( "MimeType" ) >>= aSourceMimeType )
        {
            sal_Int8 nGraphicType( xGraphic->getType() );
            if ( nGraphicType == css::graphic::GraphicType::PIXEL )
            {
                bool bTransparent = false;
                bool bAlpha       = false;
                bool bAnimated    = false;

                awt::Size aSourceSizePixel( 0, 0 );
                text::GraphicCrop aGraphicCropPixel( 0, 0, 0, 0 );

                if ( ( xGraphicPropertySet->getPropertyValue( "SizePixel" ) >>= aSourceSizePixel ) &&
                    ( xGraphicPropertySet->getPropertyValue( "Transparent" ) >>= bTransparent ) &&
                    ( xGraphicPropertySet->getPropertyValue( "Alpha" ) >>= bAlpha ) &&
                    ( xGraphicPropertySet->getPropertyValue( "Animated" ) >>= bAnimated ) )
                {
                    awt::Size aDestSizePixel( aSourceSizePixel );
                    if ( !bAnimated )
                    {
                        bool bNeedsOptimizing = false;
                        bool bRemoveCropArea( rGraphicSettings.mbRemoveCropArea );

                        // cropping has to be removed from SourceSizePixel
                        if ( aGraphicCropLogic.Left || aGraphicCropLogic.Top || aGraphicCropLogic.Right || aGraphicCropLogic.Bottom )
                        {
                            const awt::Size aSize100thMM( GraphicCollector::GetOriginalSize( rxContext, xGraphic ) );

                            if ( bRemoveCropArea )
                                bNeedsOptimizing = true;

                            if ( aSize100thMM.Width && aSize100thMM.Height )
                            {
                                aGraphicCropPixel.Left = static_cast< sal_Int32 >( ( (double)aSourceSizePixel.Width * aGraphicCropLogic.Left ) / aSize100thMM.Width );
                                aGraphicCropPixel.Top = static_cast< sal_Int32 >( ( (double)aSourceSizePixel.Height* aGraphicCropLogic.Top ) / aSize100thMM.Height );
                                aGraphicCropPixel.Right = static_cast< sal_Int32 >( ( (double)aSourceSizePixel.Width * ( aSize100thMM.Width - aGraphicCropLogic.Right ) ) / aSize100thMM.Width );
                                aGraphicCropPixel.Bottom = static_cast< sal_Int32 >( ( (double)aSourceSizePixel.Height* ( aSize100thMM.Height - aGraphicCropLogic.Bottom ) ) / aSize100thMM.Height );

                                // first calculating new SourceSizePixel by removing the cropped area
                                aSourceSizePixel.Width = aGraphicCropPixel.Right - aGraphicCropPixel.Left;
                                aSourceSizePixel.Height= aGraphicCropPixel.Bottom - aGraphicCropPixel.Top;
                            }
                            else
                            {
                                bRemoveCropArea = false;
                            }
                        }
                        if ( ( aSourceSizePixel.Width > 0 ) && ( aSourceSizePixel.Height > 0 ) )
                        {
                            OUString aDestMimeType( "image/png"  );
                            if ( rGraphicSettings.mbJPEGCompression && !bTransparent && !bAlpha && !bAnimated )
                            {
                                aDestMimeType = "image/jpeg";
//                                      if( aSourceMimeType != aDestMimeType )
                                bNeedsOptimizing = true;
                            }
                            if ( bRemoveCropArea )
                                aDestSizePixel = aSourceSizePixel;
                            if ( rGraphicSettings.mnImageResolution && aLogicalSize.Width && aLogicalSize.Height )
                            {
                                const double fSourceDPIX = ((double)aSourceSizePixel.Width / ((double)aLogicalSize.Width / 2540.0 ));
                                const double fSourceDPIY = ((double)aSourceSizePixel.Height/ ((double)aLogicalSize.Height/ 2540.0 ));

                                // check, if the bitmap DPI exceeds the maximum DPI
                                if( ( fSourceDPIX > rGraphicSettings.mnImageResolution ) || ( fSourceDPIY > rGraphicSettings.mnImageResolution ) )
                                {
                                    const double fNewSizePixelX = ((double)aDestSizePixel.Width * rGraphicSettings.mnImageResolution ) / fSourceDPIX;
                                    const double fNewSizePixelY = ((double)aDestSizePixel.Height* rGraphicSettings.mnImageResolution ) / fSourceDPIY;

                                    aDestSizePixel = awt::Size( (sal_Int32)fNewSizePixelX, (sal_Int32)fNewSizePixelY );
                                    bNeedsOptimizing = true;
                                }
                            }
                            if ( bNeedsOptimizing && aDestSizePixel.Width && aDestSizePixel.Height )
                            {
                                Reference< XStream > xTempFile( io::TempFile::create(rxContext), UNO_QUERY_THROW );
                                Reference< XOutputStream > xOutputStream( xTempFile->getOutputStream() );
                                Reference< XGraphicProvider > xGraphicProvider( GraphicProvider::create( rxContext ) );

                                ImpCompressGraphic( xGraphicProvider, xGraphic, xOutputStream, aDestMimeType, aLogicalSize, rGraphicSettings.mnJPEGQuality, rGraphicSettings.mnImageResolution, bRemoveCropArea, aGraphicCropLogic );
                                Reference< XInputStream > xInputStream( xTempFile->getInputStream() );
                                Reference< XSeekable > xSeekable( xInputStream, UNO_QUERY_THROW );
                                xSeekable->seek( 0 );
                                Sequence< PropertyValue > aArgs( 1 );
                                aArgs[ 0 ].Name = "InputStream";
                                aArgs[ 0 ].Value <<= xInputStream;
                                xNewGraphic = xGraphicProvider->queryGraphic( aArgs );
                            }
                        }
                    }
                }
            }
            else // this is a metafile
            {
                OUString aDestMimeType( aSourceMimeType );
                Reference< XStream > xTempFile( io::TempFile::create(rxContext), UNO_QUERY_THROW );
                Reference< XOutputStream > xOutputStream( xTempFile->getOutputStream() );
                Reference< XGraphicProvider > xGraphicProvider( GraphicProvider::create( rxContext ) );
                ImpCompressGraphic( xGraphicProvider, xGraphic, xOutputStream, aDestMimeType, aLogicalSize, rGraphicSettings.mnJPEGQuality, rGraphicSettings.mnImageResolution, false, aGraphicCropLogic );
                Reference< XInputStream > xInputStream( xTempFile->getInputStream() );
                Reference< XSeekable > xSeekable( xInputStream, UNO_QUERY_THROW );
                xSeekable->seek( 0 );
                Sequence< PropertyValue > aArgs( 1 );
                aArgs[ 0 ].Name = "InputStream";
                aArgs[ 0 ].Value <<= xInputStream;
                xNewGraphic = xGraphicProvider->queryGraphic( aArgs );
            }
        }
    }
    catch( Exception& )
    {
    }
    return xNewGraphic;
}

void CompressGraphics( ImpOptimizer& rOptimizer, const Reference< XComponentContext >& rxContext, const GraphicSettings& rGraphicSettings,
    std::vector< GraphicCollector::GraphicEntity >& rGraphicList )
{
    try
    {
        std::vector< GraphicCollector::GraphicEntity >::iterator aGraphicIter( rGraphicList.begin() );
        std::vector< GraphicCollector::GraphicEntity >::iterator aGraphicIEnd( rGraphicList.end() );
        double i = 0;
        while( aGraphicIter != aGraphicIEnd )
        {
            i++;
            sal_Int32 nProgress = static_cast< sal_Int32 >( 40.0 * ( i / static_cast< double >( rGraphicList.size() ) ) ) + 50;
            rOptimizer.SetStatusValue( TK_Progress, Any( static_cast< sal_Int32 >( nProgress ) ) );
            rOptimizer.DispatchStatus();

            if ( aGraphicIter->maUser.size() )
            {
                GraphicSettings aGraphicSettings( rGraphicSettings );
                aGraphicSettings.mbRemoveCropArea = aGraphicIter->mbRemoveCropArea;

                Reference< XGraphic > xGraphic;
                if ( aGraphicIter->maUser[ 0 ].mbFillBitmap && aGraphicIter->maUser[ 0 ].mxPropertySet.is() )
                {
                    Reference< XBitmap > xFillBitmap;
                    if ( aGraphicIter->maUser[ 0 ].mxPropertySet->getPropertyValue( "FillBitmap" ) >>= xFillBitmap )
                        xGraphic.set( xFillBitmap, UNO_QUERY_THROW );
                }
                else if ( aGraphicIter->maUser[ 0 ].mxShape.is() )
                {
                    Reference< XPropertySet > xShapePropertySet( aGraphicIter->maUser[ 0 ].mxShape, UNO_QUERY_THROW );
                    xShapePropertySet->getPropertyValue( "Graphic" ) >>= xGraphic;
                }
                if ( xGraphic.is() )
                {
                    Reference< XPropertySet > xNewGraphicPropertySet( xGraphic, UNO_QUERY_THROW );
                    awt::Size aSize100thMM( GraphicCollector::GetOriginalSize( rxContext, xGraphic ) );
                    Reference< XGraphic > xNewGraphic( ImpCompressGraphic( rxContext, xGraphic, aGraphicIter->maLogicalSize, aGraphicIter->maGraphicCropLogic, aGraphicSettings ) );
                    if ( xNewGraphic.is() )
                    {
                        // applying graphic to each user
                        std::vector< GraphicCollector::GraphicUser >::iterator aGraphicUserIter( aGraphicIter->maUser.begin() );
                        while( aGraphicUserIter != aGraphicIter->maUser.end() )
                        {
                            if ( aGraphicUserIter->mxShape.is() )
                            {
                                OUString sEmptyGraphicURL;
                                Reference< XPropertySet > xShapePropertySet( aGraphicUserIter->mxShape, UNO_QUERY_THROW );
                                xShapePropertySet->setPropertyValue( "GraphicURL", Any( sEmptyGraphicURL ) );
                                xShapePropertySet->setPropertyValue( "Graphic", Any( xNewGraphic ) );

                                if ( aGraphicUserIter->maGraphicCropLogic.Left || aGraphicUserIter->maGraphicCropLogic.Top
                                || aGraphicUserIter->maGraphicCropLogic.Right || aGraphicUserIter->maGraphicCropLogic.Bottom )
                                {   // removing crop area was not possible or should't been applied
                                    text::GraphicCrop aGraphicCropLogic( 0, 0, 0, 0 );
                                    if ( !aGraphicSettings.mbRemoveCropArea )
                                    {
                                        awt::Size aNewSize( GraphicCollector::GetOriginalSize( rxContext, xNewGraphic ) );
                                        aGraphicCropLogic.Left = (sal_Int32)((double)aGraphicUserIter->maGraphicCropLogic.Left * ((double)aNewSize.Width / (double)aSize100thMM.Width));
                                        aGraphicCropLogic.Top = (sal_Int32)((double)aGraphicUserIter->maGraphicCropLogic.Top * ((double)aNewSize.Height / (double)aSize100thMM.Height));
                                        aGraphicCropLogic.Right = (sal_Int32)((double)aGraphicUserIter->maGraphicCropLogic.Right * ((double)aNewSize.Width / (double)aSize100thMM.Width));
                                        aGraphicCropLogic.Bottom = (sal_Int32)((double)aGraphicUserIter->maGraphicCropLogic.Bottom * ((double)aNewSize.Height / (double)aSize100thMM.Height));
                                    }
                                    xShapePropertySet->setPropertyValue( "GraphicCrop", Any( aGraphicCropLogic ) );
                                }
                            }
                            else if ( aGraphicUserIter->mxPropertySet.is() )
                            {
                                Reference< XBitmap > xFillBitmap( xNewGraphic, UNO_QUERY );
                                if ( xFillBitmap.is() )
                                {
                                    awt::Size aSize;
                                    bool bLogicalSize;

                                    Reference< XPropertySet >& rxPropertySet( aGraphicUserIter->mxPropertySet );
                                    rxPropertySet->setPropertyValue( "FillBitmap", Any( xFillBitmap ) );
                                    if ( ( rxPropertySet->getPropertyValue( "FillBitmapLogicalSize" ) >>= bLogicalSize )
                                        && ( rxPropertySet->getPropertyValue( "FillBitmapSizeX" ) >>= aSize.Width )
                                        && ( rxPropertySet->getPropertyValue( "FillBitmapSizeY" ) >>= aSize.Height ) )
                                    {
                                        if ( !aSize.Width || !aSize.Height )
                                        {
                                            rxPropertySet->setPropertyValue( "FillBitmapLogicalSize", Any( sal_True ) );
                                            rxPropertySet->setPropertyValue( "FillBitmapSizeX", Any( aGraphicUserIter->maLogicalSize.Width ) );
                                            rxPropertySet->setPropertyValue( "FillBitmapSizeY", Any( aGraphicUserIter->maLogicalSize.Height ) );
                                        }
                                    }
                                    if ( aGraphicUserIter->mxPagePropertySet.is() )
                                        aGraphicUserIter->mxPagePropertySet->setPropertyValue( "Background", Any( rxPropertySet ) );
                                }
                            }
                            ++aGraphicUserIter;
                        }
                    }
                }
            }
            ++aGraphicIter;
        }
    }
    catch ( Exception& )
    {
    }
}


// - ImpOptimizer -


ImpOptimizer::ImpOptimizer( const Reference< XComponentContext >& rxContext, const Reference< XModel >& rxModel ) :
    mxContext                   ( rxContext ),
    mxModel                     ( rxModel ),
    mbJPEGCompression           ( false ),
    mnJPEGQuality               ( 90 ),
    mbRemoveCropArea            ( false ),
    mnImageResolution           ( 0 ),
    mbEmbedLinkedGraphics       ( true ),
    mbOLEOptimization           ( false ),
    mnOLEOptimizationType       ( 0 ),
    mbDeleteUnusedMasterPages   ( false ),
    mbDeleteHiddenSlides        ( false ),
    mbDeleteNotesPages          ( false ),
    mbOpenNewDocument           ( true )
{
}



ImpOptimizer::~ImpOptimizer()
{
}



void ImpOptimizer::DispatchStatus()
{
    if ( mxStatusDispatcher.is() )
    {
        URL aURL;
        aURL.Protocol = "vnd.com.sun.star.comp.PresentationMinimizer:";
        aURL.Path = "statusupdate";
        mxStatusDispatcher->dispatch( aURL, GetStatusSequence() );
    }
}



bool ImpOptimizer::Optimize()
{

    if ( !maCustomShowName.isEmpty() )
        ImpExtractCustomShow( mxModel, maCustomShowName );

    if ( mbDeleteUnusedMasterPages )
    {
        SetStatusValue( TK_Progress, Any( static_cast< sal_Int32 >( 40 ) ) );
        SetStatusValue( TK_Status, Any( OUString("STR_DELETING_SLIDES") ) );
        DispatchStatus();
        ImpDeleteUnusedMasterPages( mxModel );
    }

    if ( mbDeleteHiddenSlides )
    {
        SetStatusValue( TK_Progress, Any( static_cast< sal_Int32 >( 40 ) ) );
        SetStatusValue( TK_Status, Any( OUString("STR_DELETING_SLIDES") ) );
        DispatchStatus();
        ImpDeleteHiddenSlides( mxModel );
    }

    if ( mbDeleteNotesPages )
    {
        SetStatusValue( TK_Status, Any( OUString("STR_DELETING_SLIDES") ) );
        DispatchStatus();
        ImpDeleteNotesPages( mxModel );
    }

    if ( mbOLEOptimization )
    {
        SetStatusValue( TK_Progress, Any( static_cast< sal_Int32 >( 45 ) ) );
        SetStatusValue( TK_Status, Any( OUString("STR_CREATING_OLE_REPLACEMENTS") ) );
        DispatchStatus();
        ImpConvertOLE( mxModel, mnOLEOptimizationType );
    }

    if ( mbJPEGCompression || mbRemoveCropArea || mnImageResolution )
    {
        SetStatusValue( TK_Progress, Any( static_cast< sal_Int32 >( 50 ) ) );
        SetStatusValue( TK_Status, Any( OUString("STR_OPTIMIZING_GRAPHICS") ) );
        DispatchStatus();

        std::vector< GraphicCollector::GraphicEntity > aGraphicList;
        GraphicSettings aGraphicSettings( mbJPEGCompression, mnJPEGQuality, mbRemoveCropArea, mnImageResolution, mbEmbedLinkedGraphics );
        GraphicCollector::CollectGraphics( mxContext, mxModel, aGraphicSettings, aGraphicList );
        CompressGraphics( *this, mxContext, aGraphicSettings, aGraphicList );
    }
    SetStatusValue( TK_Progress, Any( static_cast< sal_Int32 >( 100 ) ) );
    DispatchStatus();
    return true;
}

static void DispatchURL( Reference< XComponentContext > xContext, const OUString& sURL, Reference< XFrame > xFrame )
{
    try
    {
        Reference< XURLTransformer > xURLTransformer( URLTransformer::create(xContext) );
        util::URL aUrl;
        aUrl.Complete = sURL;
        xURLTransformer->parseStrict( aUrl );
        Sequence< PropertyValue > aArgs;
        Reference< XDispatchProvider > xDispatchProvider( xFrame, UNO_QUERY_THROW );
        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aUrl, OUString(), 0 );  // "_self"
        if ( xDispatch.is() )
            xDispatch->dispatch( aUrl, aArgs );
    }
    catch( Exception& )
    {
    }
}



bool ImpOptimizer::Optimize( const Sequence< PropertyValue >& rArguments )
{
    bool bRet = true;

    if ( mxModel.is() )
    {
        sal_Int64 nEstimatedFileSize = 0;
        SetStatusValue( TK_Progress, Any( static_cast< sal_Int32 >( 0 ) ) );
        DispatchStatus();

        int i, nICount;
        for ( i = 0, nICount = rArguments.getLength(); i < nICount; i++ )
        {
            switch( TKGet( rArguments[ i ].Name ) )
            {
                case TK_StatusDispatcher : rArguments[ i ].Value >>= mxStatusDispatcher; break;
                case TK_InformationDialog: rArguments[ i ].Value >>= mxInformationDialog; break;
                case TK_Settings :
                {
                    css::uno::Sequence< css::beans::PropertyValue > aSettings;
                    int j, nJCount;
                    rArguments[ i ].Value >>= aSettings;
                    for ( j = 0, nJCount = aSettings.getLength(); j < nJCount; j++ )
                    {
                        switch( TKGet( aSettings[ j ].Name ) )
                        {
                            case TK_JPEGCompression         : aSettings[ j ].Value >>= mbJPEGCompression; break;
                            case TK_JPEGQuality             : aSettings[ j ].Value >>= mnJPEGQuality; break;
                            case TK_RemoveCropArea          : aSettings[ j ].Value >>= mbRemoveCropArea; break;
                            case TK_ImageResolution         : aSettings[ j ].Value >>= mnImageResolution; break;
                            case TK_EmbedLinkedGraphics     : aSettings[ j ].Value >>= mbEmbedLinkedGraphics; break;
                            case TK_OLEOptimization         : aSettings[ j ].Value >>= mbOLEOptimization; break;
                            case TK_OLEOptimizationType     : aSettings[ j ].Value >>= mnOLEOptimizationType; break;
                            case TK_CustomShowName          : aSettings[ j ].Value >>= maCustomShowName; break;
                            case TK_DeleteUnusedMasterPages : aSettings[ j ].Value >>= mbDeleteUnusedMasterPages; break;
                            case TK_DeleteHiddenSlides      : aSettings[ j ].Value >>= mbDeleteHiddenSlides; break;
                            case TK_DeleteNotesPages        : aSettings[ j ].Value >>= mbDeleteNotesPages; break;
                            case TK_SaveAsURL               : aSettings[ j ].Value >>= maSaveAsURL; break;
                            case TK_FilterName              : aSettings[ j ].Value >>= maFilterName; break;
                            case TK_OpenNewDocument         : aSettings[ j ].Value >>= mbOpenNewDocument; break;
                            case TK_EstimatedFileSize       : aSettings[ j ].Value >>= nEstimatedFileSize; break;
                            default: break;
                        }
                    }
                }
                break;
                default: break;
            }
        }

        sal_Int64 nSourceSize = 0;
        sal_Int64 nDestSize = 0;

        Reference< XFrame > xSelf;
        if ( !maSaveAsURL.isEmpty() )
        {

            SetStatusValue( TK_Progress, Any( static_cast< sal_Int32 >( 10 ) ) );
            SetStatusValue( TK_Status, Any( OUString("STR_DUPLICATING_PRESENTATION") ) );
            DispatchStatus();

            Reference< XStorable >xStorable( mxModel, UNO_QUERY );
            if ( xStorable.is() )
            {
                if ( xStorable->hasLocation() )
                    nSourceSize = PPPOptimizer::GetFileSize( xStorable->getLocation() );

                Sequence< PropertyValue > aArguments;
                if ( !maFilterName.isEmpty() )
                {
                    int nLength = aArguments.getLength();
                    aArguments.realloc( nLength + 1 );
                    aArguments[ nLength ].Name = "FilterName";
                    aArguments[ nLength ].Value <<= maFilterName;
                }
                xStorable->storeToURL( maSaveAsURL, aArguments );
                if ( !nSourceSize )
                    nSourceSize = PPPOptimizer::GetFileSize( maSaveAsURL );

                SetStatusValue( TK_Progress, Any( static_cast< sal_Int32 >( 30 ) ) );
                SetStatusValue( TK_Status, Any( OUString("STR_DUPLICATING_PRESENTATION") ) );
                DispatchStatus();

                Reference< XDesktop2 > xDesktop = Desktop::create( mxContext );
                xSelf = xDesktop->findFrame( "_blank", FrameSearchFlag::CREATE );
                Reference< XComponentLoader > xComponentLoader( xSelf, UNO_QUERY );

                Sequence< PropertyValue > aLoadProps( 1 );
                aLoadProps[ 0 ].Name = "Hidden";
                aLoadProps[ 0 ].Value <<= true;
                mxModel.set( xComponentLoader->loadComponentFromURL(
                    maSaveAsURL, "_self", 0, aLoadProps ), UNO_QUERY );
            }
        }

        // check if the document is ReadOnly -> error
        Reference< XStorable > xStorable( mxModel, UNO_QUERY );
        if ( xStorable.is() && !xStorable->isReadonly() )
        {
            mxModel->lockControllers();
            bRet = Optimize();
            mxModel->unlockControllers();

            // clearing undo stack:
            Reference< XFrame > xFrame( xSelf.is() ? xSelf : mxInformationDialog );
            if ( xFrame.is() )
            {
                const OUString sSlot( "slot:27115"  );
                DispatchURL( mxContext, sSlot, xFrame );
            }
        }

        if ( !maSaveAsURL.isEmpty() )
        {
            if ( xStorable.is() )
            {
                xStorable->store();
                nDestSize = PPPOptimizer::GetFileSize( maSaveAsURL );
            }
        }

        if ( mxInformationDialog.is() )
        {
            InformationDialog aInformationDialog( mxContext, mxInformationDialog, maSaveAsURL, mbOpenNewDocument, nSourceSize, nDestSize, nEstimatedFileSize );
            aInformationDialog.execute();
            SetStatusValue( TK_OpenNewDocument, Any( mbOpenNewDocument ) );
            DispatchStatus();
        }

        if ( !maSaveAsURL.isEmpty() )
        {
            if ( mbOpenNewDocument && xSelf.is() )
            {
                Reference< awt::XWindow > xContainerWindow( xSelf->getContainerWindow() );
                xContainerWindow->setVisible( sal_True );
            }
            else
            {
                Reference< XComponent > xComponent( mxModel, UNO_QUERY );
                xComponent->dispose();
            }
        }
        if ( nSourceSize && nDestSize )
        {
            SetStatusValue( TK_FileSizeSource, Any( nSourceSize ) );
            SetStatusValue( TK_FileSizeDestination, Any( nDestSize ) );
            DispatchStatus();
        }
    }
    else
        bRet = false;
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
