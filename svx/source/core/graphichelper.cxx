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

#include <vcl/graphicfilter.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/dialmgr.hxx>
#include <svx/graphichelper.hxx>
#include <svx/strings.hrc>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/ShapeCollection.hpp>

#include <map>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

#include <unotools/streamwrap.hxx>

using namespace css;

namespace
{

const auto constGfxTypeToExtension = frozen::make_unordered_map<GfxLinkType, OUString>(
{
    { GfxLinkType::NativePng, u"png"_ustr },
    { GfxLinkType::NativeGif, u"gif"_ustr },
    { GfxLinkType::NativeTif, u"tif"_ustr },
    { GfxLinkType::NativeWmf, u"wmf"_ustr },
    { GfxLinkType::NativeMet, u"met"_ustr },
    { GfxLinkType::NativePct, u"pct"_ustr },
    { GfxLinkType::NativeJpg, u"jpg"_ustr },
    { GfxLinkType::NativeBmp, u"bmp"_ustr },
    { GfxLinkType::NativeSvg, u"svg"_ustr },
    { GfxLinkType::NativePdf, u"pdf"_ustr },
    { GfxLinkType::NativeWebp, u"webp"_ustr },
});

const auto constVectorGraphicTypeToExtension = frozen::make_unordered_map<VectorGraphicDataType, OUString>(
{
    { VectorGraphicDataType::Wmf, u"wmf"_ustr },
    { VectorGraphicDataType::Emf, u"emf"_ustr },
    { VectorGraphicDataType::Svg, u"svg"_ustr },
});

const auto constGfxTypeToString = frozen::make_unordered_map<GfxLinkType, TranslateId>(
{
    { GfxLinkType::NativePng, STR_IMAGE_PNG },
    { GfxLinkType::NativeGif, STR_IMAGE_GIF },
    { GfxLinkType::NativeTif, STR_IMAGE_TIFF },
    { GfxLinkType::NativeWmf, STR_IMAGE_WMF },
    { GfxLinkType::NativeMet, STR_IMAGE_MET },
    { GfxLinkType::NativePct, STR_IMAGE_PCT },
    { GfxLinkType::NativeJpg, STR_IMAGE_JPEG },
    { GfxLinkType::NativeBmp, STR_IMAGE_BMP },
    { GfxLinkType::NativeSvg, STR_IMAGE_SVG },
    { GfxLinkType::NativePdf, STR_IMAGE_PNG },
    { GfxLinkType::NativeWebp, STR_IMAGE_WEBP },
});

} // end anonymous ns

void GraphicHelper::GetPreferredExtension( OUString& rExtension, const Graphic& rGraphic )
{
    auto const & rVectorGraphicDataPtr(rGraphic.getVectorGraphicData());

    if (rVectorGraphicDataPtr && !rVectorGraphicDataPtr->getBinaryDataContainer().isEmpty())
    {
        auto eType = rVectorGraphicDataPtr->getType();
        const auto iter = constVectorGraphicTypeToExtension.find(eType);
        if (iter != constVectorGraphicTypeToExtension.end())
            rExtension = iter->second;
        else
            rExtension = u"svg"_ustr; // not sure this makes sense but it is like this for a long time
    }
    else
    {
        auto eType = rGraphic.GetGfxLink().GetType();
        const auto iter = constGfxTypeToExtension.find(eType);
        if (iter != constGfxTypeToExtension.end())
            rExtension = iter->second;
        else
            rExtension = u"png"_ustr; // not sure this makes sense but it is like this for a long time
    }
}

OUString GraphicHelper::GetImageType(const Graphic& rGraphic)
{
    auto pGfxLink = rGraphic.GetSharedGfxLink();
    if (pGfxLink)
    {
        auto iter = constGfxTypeToString.find(pGfxLink->GetType());
        if (iter != constGfxTypeToString.end())
            return SvxResId(iter->second);
    }
    return SvxResId(STR_IMAGE_UNKNOWN);
}

namespace {


bool lcl_ExecuteFilterDialog(const uno::Sequence<beans::PropertyValue>& rPropsForDialog,
                             uno::Sequence<beans::PropertyValue>& rFilterData)
{
    bool bStatus = false;
    try
    {
        uno::Reference<ui::dialogs::XExecutableDialog> xFilterDialog(
                comphelper::getProcessServiceFactory()->createInstance( u"com.sun.star.svtools.SvFilterOptionsDialog"_ustr ), uno::UNO_QUERY);
        uno::Reference<beans::XPropertyAccess> xFilterProperties( xFilterDialog, uno::UNO_QUERY);

        if( xFilterDialog.is() && xFilterProperties.is() )
        {
            xFilterProperties->setPropertyValues( rPropsForDialog );
            if( xFilterDialog->execute() )
            {
                bStatus = true;
                const uno::Sequence<beans::PropertyValue> aPropsFromDialog = xFilterProperties->getPropertyValues();
                for ( const auto& rProp : aPropsFromDialog )
                {
                    if (rProp.Name == "FilterData")
                    {
                        rProp.Value >>= rFilterData;
                    }
                }
            }
        }
    }
    catch (container::NoSuchElementException const& exception)
    {
        // the filter name is unknown
        throw task::ErrorCodeIOException(
            ("lcl_ExecuteFilterDialog: NoSuchElementException"
             " \"" + exception.Message + "\": ERRCODE_IO_ABORT"),
            uno::Reference<uno::XInterface>(), sal_uInt32(ERRCODE_IO_INVALIDPARAMETER));
    }
    catch (const task::ErrorCodeIOException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("sfx.doc", "ignoring");
    }

    return bStatus;
}
} // anonymous ns

OUString GraphicHelper::ExportGraphic(weld::Window* pParent, const Graphic& rGraphic, const OUString& rGraphicName)
{
    sfx2::FileDialogHelper aDialogHelper(ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION, FileDialogFlags::NONE, pParent);
    uno::Reference<ui::dialogs::XFilePicker3> xFilePicker = aDialogHelper.GetFilePicker();

    // fish out the graphic's name
    aDialogHelper.SetContext(sfx2::FileDialogHelper::ExportImage);
    aDialogHelper.SetTitle( SvxResId(RID_SVXSTR_EXPORT_GRAPHIC_TITLE));
    INetURLObject aURL;
    aURL.SetSmartURL( rGraphicName );
    aDialogHelper.SetFileName(aURL.GetLastName());

    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    const sal_uInt16 nCount = rGraphicFilter.GetExportFormatCount();

    OUString aExtension(aURL.GetFileExtension());
    if( aExtension.isEmpty() )
    {
        GetPreferredExtension( aExtension, rGraphic );
    }

    aExtension = aExtension.toAsciiLowerCase();
    sal_uInt16 nDefaultFilter = USHRT_MAX;

    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        xFilePicker->appendFilter( rGraphicFilter.GetExportFormatName( i ), rGraphicFilter.GetExportWildcard( i ) );
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
        xFilePicker->setCurrentFilter( rGraphicFilter.GetExportFormatName( nDefaultFilter ) ) ;

        if( aDialogHelper.Execute() == ERRCODE_NONE )
        {
            OUString sPath( xFilePicker->getFiles().getConstArray()[0] );
            if( !rGraphicName.isEmpty() &&
                nDefaultFilter == rGraphicFilter.GetExportFormatNumber( xFilePicker->getCurrentFilter()))
            {
                // try to save the original graphic
                SfxMedium aIn( rGraphicName, StreamMode::READ | StreamMode::NOCREATE );
                if( aIn.GetInStream() && !aIn.GetInStream()->GetError() )
                {
                    SfxMedium aOut( sPath, StreamMode::WRITE | StreamMode::SHARE_DENYNONE);
                    if( aOut.GetOutStream() && !aOut.GetOutStream()->GetError())
                    {
                        aOut.GetOutStream()->WriteStream( *aIn.GetInStream() );
                        if ( ERRCODE_NONE == aIn.GetErrorIgnoreWarning() )
                        {
                            aOut.Close();
                            aOut.Commit();
                            if ( ERRCODE_NONE == aOut.GetErrorIgnoreWarning() )
                                return sPath;
                        }
                    }
                }
            }

            sal_uInt16 nFilter;
            if ( !xFilePicker->getCurrentFilter().isEmpty() && rGraphicFilter.GetExportFormatCount() )
            {
                nFilter = rGraphicFilter.GetExportFormatNumber( xFilePicker->getCurrentFilter() );
            }
            else
            {
                nFilter = GRFILTER_FORMAT_DONTKNOW;
            }
            OUString aFilter( rGraphicFilter.GetExportFormatShortName( nFilter ) );

            if ( rGraphic.GetType() == GraphicType::Bitmap )
            {
                Graphic aGraphic = rGraphic;
                uno::Reference<graphic::XGraphic> xGraphic = aGraphic.GetXGraphic();

                OUString aExportFilter = rGraphicFilter.GetExportInternalFilterName(nFilter);

                uno::Sequence<beans::PropertyValue> aPropsForDialog
                {
                    comphelper::makePropertyValue(u"Graphic"_ustr, xGraphic),
                    comphelper::makePropertyValue(u"FilterName"_ustr, aExportFilter)
                };

                uno::Sequence<beans::PropertyValue> aFilterData;
                bool bStatus = lcl_ExecuteFilterDialog(aPropsForDialog, aFilterData);
                if (bStatus)
                {
                    sal_Int32 nWidth = 0;
                    sal_Int32 nHeight = 0;

                    for (const auto& rProp : aFilterData)
                    {
                        if (rProp.Name == "PixelWidth")
                        {
                            rProp.Value >>= nWidth;
                        }
                        else if (rProp.Name == "PixelHeight")
                        {
                            rProp.Value >>= nHeight;
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
            else
            {
                XOutBitmap::WriteGraphic( rGraphic, sPath, aFilter,
                                            XOutFlags::DontExpandFilename |
                                            XOutFlags::DontAddExtension |
                                            XOutFlags::UseNativeIfPossible );
            }
        }
    }
    return OUString();
}

void GraphicHelper::SaveShapeAsGraphicToPath(
    const uno::Reference<lang::XComponent>& xComponent,
    const uno::Reference<drawing::XShape>& xShape, const OUString& aExportMimeType,
    const OUString& sPath)
{
    uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());
    uno::Reference<io::XInputStream> xGraphStream;

    if (xGraphStream.is())
    {
        uno::Reference<ucb::XSimpleFileAccess3> xFileAccess = ucb::SimpleFileAccess::create(xContext);
        xFileAccess->writeFile(sPath, xGraphStream);
    }
    else if (xComponent.is() && aExportMimeType == "application/pdf")
    {
        uno::Reference<lang::XMultiServiceFactory> xMSF(xContext->getServiceManager(), uno::UNO_QUERY);
        uno::Reference<document::XExporter> xExporter(
            xMSF->createInstance(u"com.sun.star.comp.PDF.PDFFilter"_ustr), uno::UNO_QUERY);
        xExporter->setSourceDocument(xComponent);

        uno::Reference<drawing::XShapes> xShapes
            = drawing::ShapeCollection::create(comphelper::getProcessComponentContext());
        xShapes->add(xShape);
        uno::Sequence<beans::PropertyValue> aFilterData{
            comphelper::makePropertyValue(u"Selection"_ustr, xShapes),
        };
        SvFileStream aStream(sPath, StreamMode::READWRITE | StreamMode::TRUNC);
        uno::Reference<io::XOutputStream> xStream(new utl::OStreamWrapper(aStream));
        uno::Sequence<beans::PropertyValue> aDescriptor
        {
            comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
            comphelper::makePropertyValue(u"OutputStream"_ustr, xStream)
        };
        uno::Reference<document::XFilter> xFilter(xExporter, uno::UNO_QUERY);
        xFilter->filter(aDescriptor);
    }
    else
    {
        uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter = drawing::GraphicExportFilter::create(xContext);

        uno::Sequence<beans::PropertyValue> aDescriptor{ comphelper::makePropertyValue(u"MediaType"_ustr,
                                                                           aExportMimeType),
                                             comphelper::makePropertyValue(u"URL"_ustr, sPath) };

        uno::Reference<lang::XComponent> xSourceDocument(xShape, uno::UNO_QUERY_THROW);
        xGraphicExporter->setSourceDocument(xSourceDocument);
        xGraphicExporter->filter(aDescriptor);
    }
}

void GraphicHelper::SaveShapeAsGraphic(weld::Window* pParent,
                                       const uno::Reference<lang::XComponent>& xComponent,
                                       const uno::Reference<drawing::XShape>& xShape)
{
    try
    {
        uno::Reference<beans::XPropertySet> xShapeSet(xShape, uno::UNO_QUERY_THROW);

        sfx2::FileDialogHelper aDialogHelper(ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION, FileDialogFlags::NONE, pParent);
        uno::Reference<ui::dialogs::XFilePicker3> xFilePicker = aDialogHelper.GetFilePicker();
        aDialogHelper.SetContext(sfx2::FileDialogHelper::ExportImage);
        aDialogHelper.SetTitle( SvxResId(RID_SVXSTR_SAVEAS_IMAGE) );

        // populate filter dialog filter list and select default filter to match graphic mime type

        GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
        static constexpr OUStringLiteral aDefaultMimeType(u"image/png");
        OUString aDefaultFormatName;
        sal_uInt16 nCount = rGraphicFilter.GetExportFormatCount();

        std::map< OUString, OUString > aMimeTypeMap;

        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            const OUString aExportFormatName( rGraphicFilter.GetExportFormatName( i ) );
            const OUString aFilterMimeType( rGraphicFilter.GetExportFormatMediaType( i ) );
            xFilePicker->appendFilter( aExportFormatName, rGraphicFilter.GetExportWildcard( i ) );
            aMimeTypeMap[ aExportFormatName ] = aFilterMimeType;
            if( aDefaultMimeType == aFilterMimeType )
                aDefaultFormatName = aExportFormatName;
        }

        if( !aDefaultFormatName.isEmpty() )
            xFilePicker->setCurrentFilter( aDefaultFormatName );

        // execute dialog

        if( aDialogHelper.Execute() == ERRCODE_NONE )
        {
            OUString sPath( xFilePicker->getFiles().getConstArray()[0] );
            OUString aExportMimeType( aMimeTypeMap[xFilePicker->getCurrentFilter()] );

            GraphicHelper::SaveShapeAsGraphicToPath(xComponent, xShape, aExportMimeType, sPath);
        }
    }
    catch (uno::Exception&)
    {
    }
}

short GraphicHelper::HasToSaveTransformedImage(weld::Widget* pWin)
{
    OUString aMsg(SvxResId(RID_SVXSTR_SAVE_MODIFIED_IMAGE));
    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin,
                                              VclMessageType::Question, VclButtonsType::YesNo, aMsg));
    return xBox->run();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
