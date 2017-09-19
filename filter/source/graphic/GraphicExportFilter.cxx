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

GraphicExportFilter::GraphicExportFilter( const Reference<XComponentContext>&  )
    : mTargetWidth(0)
    , mTargetHeight(0)
    , mbSelectionOnly(false)
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
            aInternalFilterName = aInternalFilterName.replaceFirst("draw_", "");
            aInternalFilterName = aInternalFilterName.replaceFirst("impress_", "");
            aInternalFilterName = aInternalFilterName.replaceFirst("calc_", "");
            aInternalFilterName = aInternalFilterName.replaceFirst("writer_", "");
            aInternalFilterName = aInternalFilterName.replaceFirst("web_", "");
        }
        else if ( aProperty.Name == "FilterData" )
        {
            aProperty.Value >>= mFilterDataSequence;
        }
        else if ( aProperty.Name == "OutputStream" )
        {
            aProperty.Value >>= mxOutputStream;
        }
        else if ( aProperty.Name == "SelectionOnly" )
        {
            aProperty.Value >>= mbSelectionOnly;
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
        else if ( mFilterDataSequence[i].Name == "Compression" )
        {
            maCompression = mFilterDataSequence[i].Value;
        }
        else if ( mFilterDataSequence[i].Name == "Interlaced" )
        {
            maInterlaced = mFilterDataSequence[i].Value;
        }
        else if ( mFilterDataSequence[i].Name == "Translucent" )
        {
            maTranslucent = mFilterDataSequence[i].Value;
        }
        else if ( mFilterDataSequence[i].Name == "Quality" )
        {
            maQuality = mFilterDataSequence[i].Value;
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
            mFilterExtension = aGraphicFilter.GetExportFormatShortName( nFormat );
        }
    }
}

sal_Bool SAL_CALL GraphicExportFilter::filter( const Sequence<PropertyValue>& rDescriptor )
{
    gatherProperties(rDescriptor);

    DocumentToGraphicRenderer aRenderer( mxDocument, mbSelectionOnly );
    sal_Int32 nCurrentPage = aRenderer.getCurrentPage();
    Size aDocumentSizePixel = aRenderer.getDocumentSizeInPixels(nCurrentPage);

    Size aTargetSizePixel(mTargetWidth, mTargetHeight);

    if (mTargetWidth == 0 || mTargetHeight == 0)
        aTargetSizePixel = aDocumentSizePixel;

    Graphic aGraphic = aRenderer.renderToGraphic(nCurrentPage, aDocumentSizePixel, aTargetSizePixel, COL_WHITE);

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    Sequence< PropertyValue > aFilterData( mFilterDataSequence );
    sal_Int32 nAdd = 0;
    if (!maCompression.hasValue())
        ++nAdd;
    if (!maInterlaced.hasValue())
        ++nAdd;
    if (!maTranslucent.hasValue())
        ++nAdd;
    if (!maQuality.hasValue())
        ++nAdd;
    if (nAdd)
    {
        sal_Int32 nLen = aFilterData.getLength();
        aFilterData.realloc( nLen + nAdd);
        if (!maCompression.hasValue())
        {   // PNG,JPG
            aFilterData[ nLen ].Name = "Compression";
            aFilterData[ nLen ].Value <<= (sal_Int32) 9;
            ++nLen;
        }
        if (!maInterlaced.hasValue())
        {   // PNG,JPG
            aFilterData[ nLen ].Name = "Interlaced";
            aFilterData[ nLen ].Value <<= (sal_Int32) 0;
            ++nLen;
        }
        if (!maTranslucent.hasValue())
        {   // PNG
            aFilterData[ nLen ].Name = "Translucent";
            aFilterData[ nLen ].Value <<= (sal_Int32) 0;
            ++nLen;
        }
        if (!maQuality.hasValue())
        {   // JPG
            aFilterData[ nLen ].Name = "Quality";
            aFilterData[ nLen ].Value <<= (sal_Int32) 99;
            ++nLen;
        }
        assert( nLen == aFilterData.getLength());
    }

    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( mFilterExtension );

    SvMemoryStream aMemStream;
    const GraphicConversionParameters aParameters(aTargetSizePixel, true, true);

    const ErrCode nResult = rFilter.ExportGraphic( aGraphic.GetBitmapEx(aParameters), OUString(), aMemStream, nFilterFormat, &aFilterData );

    if ( nResult == ERRCODE_NONE )
    {
        SvOutputStream aOutputStream( mxOutputStream );
        aMemStream.Seek(0);
        aOutputStream.WriteStream( aMemStream );

        return true;
    }

    return false;
}

void SAL_CALL GraphicExportFilter::cancel( )
{
}

void SAL_CALL GraphicExportFilter::setSourceDocument( const Reference<XComponent>& xDocument )
{
    mxDocument = xDocument;
}

void SAL_CALL GraphicExportFilter::initialize( const Sequence<Any>& )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
