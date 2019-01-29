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

#include "GraphicExportFilter.hxx"

#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vcl/graphicfilter.hxx>
#include <svl/outstrm.hxx>
#include <svtools/DocumentToGraphicRenderer.hxx>

using namespace css;

GraphicExportFilter::GraphicExportFilter( const uno::Reference< uno::XComponentContext > & rxContext  )
    : mxContext(rxContext)
    , mnTargetWidth(0)
    , mnTargetHeight(0)
    , mbSelectionOnly(false)
{}

GraphicExportFilter::~GraphicExportFilter()
{}

void GraphicExportFilter::gatherProperties( const uno::Sequence< beans::PropertyValue > & rProperties )
{
    OUString aInternalFilterName;

    for ( sal_Int32 i = 0; i < rProperties.getLength(); i++ )
    {
        const beans::PropertyValue& rProperty = rProperties[i];

        if ( rProperty.Name == "FilterName" )
        {
            rProperty.Value >>= aInternalFilterName;
            const sal_Int32 nLen = aInternalFilterName.getLength();
            aInternalFilterName = aInternalFilterName.replaceFirst("calc_", "");
            if (aInternalFilterName.getLength() == nLen)
                aInternalFilterName = aInternalFilterName.replaceFirst("writer_", "");
            if (aInternalFilterName.getLength() == nLen)
                aInternalFilterName = aInternalFilterName.replaceFirst("web_", "");
            if (aInternalFilterName.getLength() == nLen)
                aInternalFilterName = aInternalFilterName.replaceFirst("draw_", "");
            if (aInternalFilterName.getLength() == nLen)
                aInternalFilterName = aInternalFilterName.replaceFirst("impress_", "");
        }
        else if ( rProperty.Name == "FilterData" )
        {
            rProperty.Value >>= maFilterDataSequence;
        }
        else if ( rProperty.Name == "OutputStream" )
        {
            rProperty.Value >>= mxOutputStream;
        }
        else if ( rProperty.Name == "SelectionOnly" )
        {
            rProperty.Value >>= mbSelectionOnly;
        }
    }

    for ( sal_Int32 i = 0; i < maFilterDataSequence.getLength(); i++ )
    {
        if ( maFilterDataSequence[i].Name == "PixelWidth" )
        {
            maFilterDataSequence[i].Value >>= mnTargetWidth;
        }
        else if ( maFilterDataSequence[i].Name == "PixelHeight" )
        {
            maFilterDataSequence[i].Value >>= mnTargetHeight;
        }
    }

    if ( !aInternalFilterName.isEmpty() )
    {
        GraphicFilter aGraphicFilter( true );

        sal_uInt16 nFilterCount = aGraphicFilter.GetExportFormatCount();
        sal_uInt16 nFormat;

        for ( nFormat = 0; nFormat < nFilterCount; nFormat++ )
        {
            if ( aGraphicFilter.GetExportInternalFilterName( nFormat ) == aInternalFilterName )
                break;
        }
        if ( nFormat < nFilterCount )
        {
            maFilterExtension = aGraphicFilter.GetExportFormatShortName( nFormat );
        }
    }
}

sal_Bool SAL_CALL GraphicExportFilter::filter( const uno::Sequence< beans::PropertyValue > & rDescriptor )
{
    gatherProperties(rDescriptor);

    if (mbSelectionOnly && mxDocument.is())
    {
        uno::Reference< frame::XModel > xModel( mxDocument, uno::UNO_QUERY);
        if (xModel.is())
        {
            uno::Reference< frame::XController > xController( xModel->getCurrentController());
            if (xController.is())
            {
                uno::Reference< drawing::XShapes > xShapes;
                uno::Reference< drawing::XShape > xShape;
                if (DocumentToGraphicRenderer::isShapeSelected( xShapes, xShape, xController))
                    return filterExportShape( rDescriptor, xShapes, xShape);
            }
        }
    }

    return filterRenderDocument();
}

bool GraphicExportFilter::filterRenderDocument() const
{
    DocumentToGraphicRenderer aRenderer( mxDocument, mbSelectionOnly );
    sal_Int32 nCurrentPage = aRenderer.getCurrentPage();
    Size aDocumentSizePixel = aRenderer.getDocumentSizeInPixels(nCurrentPage);

    Size aTargetSizePixel(mnTargetWidth, mnTargetHeight);

    if (mnTargetWidth == 0 || mnTargetHeight == 0)
        aTargetSizePixel = aDocumentSizePixel;

    Graphic aGraphic = aRenderer.renderToGraphic(nCurrentPage, aDocumentSizePixel, aTargetSizePixel, COL_WHITE, /*bExtOutDevData=*/false);

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( maFilterExtension );

    SvMemoryStream aMemStream;
    const GraphicConversionParameters aParameters(aTargetSizePixel, true, true);

    const ErrCode nResult = rFilter.ExportGraphic( aGraphic.GetBitmapEx(aParameters), OUString(), aMemStream,
            nFilterFormat, &maFilterDataSequence );

    if ( nResult == ERRCODE_NONE )
    {
        SvOutputStream aOutputStream( mxOutputStream );
        aMemStream.Seek(0);
        aOutputStream.WriteStream( aMemStream );

        return true;
    }

    return false;
}

bool GraphicExportFilter::filterExportShape(
        const css::uno::Sequence< css::beans::PropertyValue > & rDescriptor,
        const css::uno::Reference< css::drawing::XShapes > & rxShapes,
        const css::uno::Reference< css::drawing::XShape > & rxShape ) const
{
    uno::Reference< lang::XComponent > xSourceDoc;
    if (rxShapes.is())
        xSourceDoc.set( rxShapes, uno::UNO_QUERY_THROW );
    else if (rxShape.is())
        xSourceDoc.set( rxShape, uno::UNO_QUERY_THROW );
    if (!xSourceDoc.is())
        return false;

    uno::Reference< drawing::XGraphicExportFilter > xGraphicExporter =
        drawing::GraphicExportFilter::create( mxContext );
    if (!xGraphicExporter.is())
        return false;

    // Need to replace the internal filter name with the short name
    // (extension).
    uno::Sequence< beans::PropertyValue > aDescriptor( rDescriptor);
    for (sal_Int32 i = 0; i < aDescriptor.getLength(); ++i)
    {
        if (aDescriptor[i].Name == "FilterName")
        {
            aDescriptor[i].Value <<= maFilterExtension;
            break;
        }
    }

    xGraphicExporter->setSourceDocument( xSourceDoc );
    return xGraphicExporter->filter( aDescriptor );
}

void SAL_CALL GraphicExportFilter::cancel( )
{
}

void SAL_CALL GraphicExportFilter::setSourceDocument( const uno::Reference< lang::XComponent > & xDocument )
{
    mxDocument = xDocument;
}

void SAL_CALL GraphicExportFilter::initialize( const uno::Sequence< uno::Any > & )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
