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

#include <unotools/pathoptions.hxx>
#include <vcl/graphicfilter.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/dialmgr.hxx>
#include <svx/graphichelper.hxx>
#include <svx/strings.hrc>

#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

using namespace css::uno;
using namespace css::lang;
using namespace css::graphic;
using namespace css::ucb;
using namespace css::beans;
using namespace css::io;
using namespace css::document;
using namespace css::ui::dialogs;

using namespace sfx2;

namespace drawing = com::sun::star::drawing;

void GraphicHelper::GetPreferredExtension( OUString& rExtension, const Graphic& rGraphic )
{
    OUString aExtension = "png";
    switch( rGraphic.GetLink().GetType() )
    {
        case GfxLinkType::NativeGif:
            aExtension = "gif";
            break;
        case GfxLinkType::NativeTif:
            aExtension = "tif";
            break;
        case GfxLinkType::NativeWmf:
            aExtension = "wmf";
            break;
        case GfxLinkType::NativeMet:
            aExtension = "met";
            break;
        case GfxLinkType::NativePct:
            aExtension = "pct";
            break;
        case GfxLinkType::NativeJpg:
            aExtension = "jpg";
            break;
        case GfxLinkType::NativeBmp:
            aExtension = "bmp";
            break;
        case GfxLinkType::NativeSvg:
            aExtension = "svg";
            break;
        case GfxLinkType::NativePdf:
            aExtension = "pdf";
            break;
        default:
            break;
    }
    rExtension = aExtension;
}

OUString GraphicHelper::ExportGraphic( const Graphic& rGraphic, const OUString& rGraphicName )
{
    SvtPathOptions aPathOpt;
    OUString sGraphicsPath( aPathOpt.GetGraphicPath() );

    FileDialogHelper aDialogHelper( TemplateDescription::FILESAVE_AUTOEXTENSION );
    Reference < XFilePicker2 > xFilePicker = aDialogHelper.GetFilePicker();

    INetURLObject aPath;
    aPath.SetSmartURL( sGraphicsPath );

    // fish out the graphic's name

    aDialogHelper.SetTitle( SvxResId(RID_SVXSTR_EXPORT_GRAPHIC_TITLE));
    aDialogHelper.SetDisplayDirectory( aPath.GetMainURL(INetURLObject::DecodeMechanism::ToIUri) );
    INetURLObject aURL;
    aURL.SetSmartURL( rGraphicName );
    aDialogHelper.SetFileName( aURL.GetName() );

    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    const sal_uInt16 nCount = rGraphicFilter.GetExportFormatCount();

    OUString aExtension( aURL.GetExtension() );
    if( aExtension.isEmpty() )
    {
        GetPreferredExtension( aExtension, rGraphic );
    }

    aExtension = aExtension.toAsciiLowerCase();
    sal_uInt16 nDefaultFilter = USHRT_MAX;

    Reference<XFilterManager> xFilterManager(xFilePicker, UNO_QUERY);

    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        xFilterManager->appendFilter( rGraphicFilter.GetExportFormatName( i ), rGraphicFilter.GetExportWildcard( i ) );
        OUString aFormatShortName = rGraphicFilter.GetExportFormatShortName( i );
        if ( aFormatShortName.equalsIgnoreAsciiCase( aExtension ) )
        {
            nDefaultFilter = i;
        }
    }
    if ( USHRT_MAX == nDefaultFilter )
    {
        // "wrong" extension?
        GetPreferredExtension( aExtension, rGraphic );
        for ( sal_uInt16 i = 0; i < nCount; ++i )
            if ( aExtension == rGraphicFilter.GetExportFormatShortName( i ).toAsciiLowerCase() )
            {
                nDefaultFilter =  i;
                break;
            }
    }

    if( USHRT_MAX != nDefaultFilter )
    {
        xFilterManager->setCurrentFilter( rGraphicFilter.GetExportFormatName( nDefaultFilter ) ) ;

        if( aDialogHelper.Execute() == ERRCODE_NONE )
        {
            OUString sPath( xFilePicker->getFiles().getConstArray()[0] );
            // remember used path - please don't optimize away!
            aPath.SetSmartURL( sPath);
            sGraphicsPath = aPath.GetPath();

            if( !rGraphicName.isEmpty() &&
                nDefaultFilter == rGraphicFilter.GetExportFormatNumber( xFilterManager->getCurrentFilter()))
            {
                // try to save the original graphic
                SfxMedium aIn( rGraphicName, StreamMode::READ | StreamMode::NOCREATE );
                if( aIn.GetInStream() && !aIn.GetInStream()->GetError() )
                {
                    SfxMedium aOut( sPath, StreamMode::WRITE | StreamMode::SHARE_DENYNONE);
                    if( aOut.GetOutStream() && !aOut.GetOutStream()->GetError())
                    {
                        aOut.GetOutStream()->WriteStream( *aIn.GetInStream() );
                        if ( ERRCODE_NONE == aIn.GetError() )
                        {
                            aOut.Close();
                            aOut.Commit();
                            if ( ERRCODE_NONE == aOut.GetError() )
                                return sPath;
                        }
                    }
                }
            }

            sal_uInt16 nFilter;
            if ( !xFilterManager->getCurrentFilter().isEmpty() && rGraphicFilter.GetExportFormatCount() )
            {
                nFilter = rGraphicFilter.GetExportFormatNumber( xFilterManager->getCurrentFilter() );
            }
            else
            {
                nFilter = GRFILTER_FORMAT_DONTKNOW;
            }
            OUString aFilter( rGraphicFilter.GetExportFormatShortName( nFilter ) );

            XOutBitmap::WriteGraphic( rGraphic, sPath, aFilter,
                                        XOutFlags::DontExpandFilename |
                                        XOutFlags::DontAddExtension |
                                        XOutFlags::UseNativeIfPossible );
            return sPath;
        }
    }
    return OUString();
}

void GraphicHelper::SaveShapeAsGraphic( const Reference< drawing::XShape >& xShape )
{
    try
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        Reference< XPropertySet > xShapeSet( xShape, UNO_QUERY_THROW );

        SvtPathOptions aPathOpt;
        OUString sGraphicPath( aPathOpt.GetGraphicPath() );

        FileDialogHelper aDialogHelper( TemplateDescription::FILESAVE_AUTOEXTENSION );
        Reference < XFilePicker2 > xFilePicker = aDialogHelper.GetFilePicker();

        aDialogHelper.SetTitle( SvxResId(RID_SVXSTR_SAVEAS_IMAGE) );

        INetURLObject aPath;
        aPath.SetSmartURL( sGraphicPath );
        xFilePicker->setDisplayDirectory( aPath.GetMainURL(INetURLObject::DecodeMechanism::ToIUri) );

        // populate filter dialog filter list and select default filter to match graphic mime type

        GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
        Reference<XFilterManager> xFilterManager( xFilePicker, UNO_QUERY );
        const OUString aDefaultMimeType("image/png");
        OUString aDefaultFormatName;
        sal_uInt16 nCount = rGraphicFilter.GetExportFormatCount();

        std::map< OUString, OUString > aMimeTypeMap;

        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            const OUString aExportFormatName( rGraphicFilter.GetExportFormatName( i ) );
            const OUString aFilterMimeType( rGraphicFilter.GetExportFormatMediaType( i ) );
            xFilterManager->appendFilter( aExportFormatName, rGraphicFilter.GetExportWildcard( i ) );
            aMimeTypeMap[ aExportFormatName ] = aFilterMimeType;
            if( aDefaultMimeType == aFilterMimeType )
                aDefaultFormatName = aExportFormatName;
        }

        if( !aDefaultFormatName.isEmpty() )
            xFilterManager->setCurrentFilter( aDefaultFormatName );

        // execute dialog

        if( aDialogHelper.Execute() == ERRCODE_NONE )
        {
            OUString sPath( xFilePicker->getFiles().getConstArray()[0] );
            OUString aExportMimeType( aMimeTypeMap[xFilterManager->getCurrentFilter()] );

            Reference< XInputStream > xGraphStream;

            if( xGraphStream.is() )
            {
                Reference<XSimpleFileAccess3> xFileAccess = SimpleFileAccess::create( xContext );
                xFileAccess->writeFile( sPath, xGraphStream );
            }
            else
            {
                Reference<css::drawing::XGraphicExportFilter> xGraphicExporter = css::drawing::GraphicExportFilter::create( xContext );

                Sequence<PropertyValue> aDescriptor( 2 );
                aDescriptor[0].Name = "MediaType";
                aDescriptor[0].Value <<= aExportMimeType;
                aDescriptor[1].Name = "URL";
                aDescriptor[1].Value <<= sPath;

                Reference< XComponent > xSourceDocument( xShape, UNO_QUERY_THROW );
                if ( xSourceDocument.is() )
                {
                    xGraphicExporter->setSourceDocument( xSourceDocument );
                    xGraphicExporter->filter( aDescriptor );
                }
            }
        }
    }
    catch( Exception& )
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
