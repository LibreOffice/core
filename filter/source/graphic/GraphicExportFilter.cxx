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

#include <vcl/graphicfilter.hxx>
#include <svl/outstrm.hxx>
#include <svtools/DocumentToGraphicRenderer.hxx>

GraphicExportFilter::GraphicExportFilter( const Reference<XComponentContext>& rxContext ) :
    mxContext( rxContext )
{}

GraphicExportFilter::~GraphicExportFilter()
{}

void GraphicExportFilter::gatherProperties( const Sequence<PropertyValue>& rProperties )
{
    OUString aInternalFilterName;

    for ( sal_Int32 i = 0; i < rProperties.getLength(); i++ )
    {
        PropertyValue aProperty = rProperties[i];

        if ( aProperty.Name == "FilterName" )
        {
            aProperty.Value >>= aInternalFilterName;
            aInternalFilterName = aInternalFilterName.replaceFirst( OUString( "draw_"    ), OUString() );
            aInternalFilterName = aInternalFilterName.replaceFirst( OUString( "impress_" ), OUString() );
            aInternalFilterName = aInternalFilterName.replaceFirst( OUString( "calc_"    ),  OUString() );
            aInternalFilterName = aInternalFilterName.replaceFirst( OUString( "writer_"  ),  OUString() );
        }
        else if ( aProperty.Name == "FilterData" )
        {
            aProperty.Value >>= mFilterDataSequence;
        }
        else if ( aProperty.Name == "SelectionOnly" )
        {
            aProperty.Value >>= mExportSelection;
        }
        else if ( aProperty.Name == "URL" )
        {
            if( !( aProperty.Value >>= mTargetUrl ) )
            {
                aProperty.Value >>= mTargetUrl.Complete;
            }
        }
        else if ( aProperty.Name == "OutputStream" )
        {
            aProperty.Value >>= mxOutputStream;
        }
    }

    for ( sal_Int32 i = 0; i < mFilterDataSequence.getLength(); i++ )
    {
        if ( mFilterDataSequence[i].Name == "PixelWidth" )
        {
            mFilterDataSequence[i].Value >>= mTargetWidth;
        }
        else if ( mFilterDataSequence[i].Name == "PixelHeight" )
        {
            mFilterDataSequence[i].Value >>= mTargetHeight;
        }
    }

    if ( !aInternalFilterName.isEmpty() )
    {
        GraphicFilter aGraphicFilter( sal_True );

        sal_uInt16 nFilterCount = aGraphicFilter.GetExportFormatCount();
        sal_uInt16 nFormat;

        for ( nFormat = 0; nFormat < nFilterCount; nFormat++ )
        {
            if ( aGraphicFilter.GetExportInternalFilterName( nFormat ) == aInternalFilterName )
                break;
        }
        if ( nFormat < nFilterCount )
        {
            mFilterExtension = aGraphicFilter.GetExportFormatShortName( nFormat );
        }
    }
}

sal_Bool SAL_CALL GraphicExportFilter::filter( const Sequence<PropertyValue>& rDescriptor )
    throw (RuntimeException)
{
    gatherProperties(rDescriptor);

    DocumentToGraphicRenderer aRenderer( mxDocument );
    sal_Int32 aCurrentPage = aRenderer.getCurrentPageWriter();
    Size aDocumentSizePixel = aRenderer.getDocumentSizeInPixels(aCurrentPage);

    Size aTargetSizePixel(mTargetWidth, mTargetHeight);

    Graphic aGraphic = aRenderer.renderToGraphic( aCurrentPage, aDocumentSizePixel, aTargetSizePixel );

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    Sequence< PropertyValue > aFilterData( 3 );
    aFilterData[ 0 ].Name = "Interlaced";
    aFilterData[ 0 ].Value <<= (sal_Int32) 0;
    aFilterData[ 1 ].Name = "Compression";
    aFilterData[ 1 ].Value <<= (sal_Int32) 9;
    aFilterData[ 2 ].Name = "Quality";
    aFilterData[ 2 ].Value <<= (sal_Int32) 99;

    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( mFilterExtension );

    SvMemoryStream aMemStream;
    const GraphicConversionParameters aParameters(aTargetSizePixel, true, true);

    const sal_uInt16 nResult = rFilter.ExportGraphic( aGraphic.GetBitmapEx(aParameters), OUString(), aMemStream, nFilterFormat, &aFilterData );

    if ( nResult == GRFILTER_OK )
    {
        SvOutputStream aOutputStream( mxOutputStream );
        aMemStream.Seek(0);
        aOutputStream << aMemStream;

        return true;
    }

    return false;
}

void SAL_CALL GraphicExportFilter::cancel( ) throw (RuntimeException)
{
}

void SAL_CALL GraphicExportFilter::setSourceDocument( const Reference<XComponent>& xDocument )
    throw (IllegalArgumentException, RuntimeException)
{
    mxDocument = xDocument;
}

void SAL_CALL GraphicExportFilter::initialize( const Sequence<Any>& )
    throw (Exception, RuntimeException)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
