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

#include <vcl/layout.hxx>

#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

using namespace css::uno;
using namespace css::lang;
using namespace css::graphic;
using namespace css::ucb;
using namespace css::beans;
using namespace css::io;
using namespace css::document;
using namespace css::ui::dialogs;
using namespace css::container;
using namespace com::sun::star::task;
using namespace css::frame;

using namespace sfx2;

namespace drawing = com::sun::star::drawing;

void GraphicHelper::GetPreferredExtension( OUString& rExtension, const Graphic& rGraphic )
{
    OUString aExtension = "png";
    const VectorGraphicDataPtr& aVectorGraphicDataPtr(rGraphic.getVectorGraphicData());

    if (aVectorGraphicDataPtr.get() && aVectorGraphicDataPtr->getVectorGraphicDataArrayLength())
    {
        switch (aVectorGraphicDataPtr->getVectorGraphicDataType())
        {
        case VectorGraphicDataType::Wmf:
            aExtension = "wmf";
            break;
        case VectorGraphicDataType::Emf:
            aExtension = "emf";
            break;
        default: // case VectorGraphicDataType::Svg:
            aExtension = "svg";
            break;
        }

        rExtension = aExtension;
        return;
    }

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

namespace {


bool lcl_ExecuteFilterDialog( const Sequence< PropertyValue >& rPropsForDialog,
                              Sequence< PropertyValue >& rFilterData )
{
    bool bStatus = false;
    try
    {
        const OUString aServiceName("com.sun.star.svtools.SvFilterOptionsDialog");
        Reference< XExecutableDialog > xFilterDialog(
                comphelper::getProcessServiceFactory()->createInstance( aServiceName ), UNO_QUERY );
        Reference< XPropertyAccess > xFilterProperties( xFilterDialog, UNO_QUERY );

        if( xFilterDialog.is() && xFilterProperties.is() )
        {
            xFilterProperties->setPropertyValues( rPropsForDialog );
            if( xFilterDialog->execute() )
            {
                bStatus = true;
                Sequence< PropertyValue > aPropsFromDialog = xFilterProperties->getPropertyValues();
                const sal_Int32 nPropsLen = aPropsFromDialog.getLength();
                for ( sal_Int32 nInd = 0; nInd < nPropsLen; ++nInd )
                {
                    if (aPropsFromDialog[nInd].Name == "FilterData")
                    {
                        aPropsFromDialog[nInd].Value >>= rFilterData;
                    }
                }
            }
        }
    }
    catch( const NoSuchElementException& e )
    {
        // the filter name is unknown
        throw ErrorCodeIOException(
            ("lcl_ExecuteFilterDialog: NoSuchElementException"
             " \"" + e.Message + "\": ERRCODE_IO_ABORT"),
            Reference< XInterface >(), sal_uInt32(ERRCODE_IO_INVALIDPARAMETER));
    }
    catch( const ErrorCodeIOException& )
    {
        throw;
    }
    catch( const Exception& e )
    {
        SAL_WARN("sfx.doc", "ignoring " << e);
    }

    return bStatus;
}
} // anonymous ns

OUString GraphicHelper::ExportGraphic(const vcl::Window* pParent, const Graphic& rGraphic, const OUString& rGraphicName)
{
    SvtPathOptions aPathOpt;
    OUString sGraphicsPath( aPathOpt.GetGraphicPath() );

    FileDialogHelper aDialogHelper(TemplateDescription::FILESAVE_AUTOEXTENSION, FileDialogFlags::NONE, pParent);
    Reference < XFilePicker3 > xFilePicker = aDialogHelper.GetFilePicker();

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

            if ( rGraphic.GetType() == GraphicType::Bitmap )
            {
                Graphic aGraphic = rGraphic;
                Reference<XGraphic> xGraphic = aGraphic.GetXGraphic();

                OUString aExportFilter = rGraphicFilter.GetExportInternalFilterName(nFilter);

                Sequence< PropertyValue > aPropsForDialog(2);
                aPropsForDialog[0].Name = "Graphic";
                aPropsForDialog[0].Value <<= xGraphic;
                aPropsForDialog[1].Name = "FilterName";
                aPropsForDialog[1].Value <<= aExportFilter;

                Sequence< PropertyValue > aFilterData;
                bool bStatus = lcl_ExecuteFilterDialog(aPropsForDialog, aFilterData);
                if (bStatus)
                {
                    sal_Int32 nWidth = 0;
                    sal_Int32 nHeight = 0;

                    sal_Int32 nLen = aFilterData.getLength();
                    for (sal_Int32 i = 0; i < nLen; ++i)
                    {
                        if (aFilterData[i].Name == "PixelWidth")
                        {
                            aFilterData[i].Value >>= nWidth;
                        }
                        else if (aFilterData[i].Name == "PixelHeight")
                        {
                            aFilterData[i].Value >>= nHeight;
                        }
                    }

                    // scaling must performed here because png/jpg writer s
                    // do not take care of that.
                    Size aSizePixel( aGraphic.GetSizePixel() );
                    if( nWidth && nHeight &&
                        ( ( nWidth != aSizePixel.Width() ) ||
                          ( nHeight != aSizePixel.Height() ) ) )
                    {
                        BitmapEx aBmpEx( aGraphic.GetBitmapEx() );
                        // export: use highest quality
                        aBmpEx.Scale( Size( nWidth, nHeight ), BmpScaleFlag::Lanczos );
                        aGraphic = aBmpEx;
                    }

                    XOutBitmap::WriteGraphic( aGraphic, sPath, aFilter,
                                                XOutFlags::DontExpandFilename |
                                                XOutFlags::DontAddExtension |
                                                XOutFlags::UseNativeIfPossible,
                                                nullptr, &aFilterData );
                    return sPath;
                }
            }
        }
    }
    return OUString();
}

void GraphicHelper::SaveShapeAsGraphic(const vcl::Window* pParent,  const Reference< drawing::XShape >& xShape)
{
    try
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        Reference< XPropertySet > xShapeSet( xShape, UNO_QUERY_THROW );

        SvtPathOptions aPathOpt;
        OUString sGraphicPath( aPathOpt.GetGraphicPath() );

        FileDialogHelper aDialogHelper(TemplateDescription::FILESAVE_AUTOEXTENSION, FileDialogFlags::NONE, pParent);
        Reference < XFilePicker3 > xFilePicker = aDialogHelper.GetFilePicker();

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

short GraphicHelper::HasToSaveTransformedImage(vcl::Window* pWin)
{
    OUString aMsg(SvxResId(RID_SVXSTR_SAVE_MODIFIED_IMAGE));
    ScopedVclPtrInstance< MessageDialog > aBox(pWin, aMsg, VclMessageType::Question, VclButtonsType::YesNo);
    return aBox->Execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
