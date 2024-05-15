/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EPUBExportFilter.hxx"

#include <libepubgen/EPUBTextGenerator.h>
#include <libepubgen/libepubgen-decls.h>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svtools/DocumentToGraphicRenderer.hxx>
#include <vcl/filter/SvmWriter.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/stream.hxx>

#include "exp/xmlimp.hxx"
#include "EPUBPackage.hxx"

using namespace com::sun::star;

namespace writerperfect
{
EPUBExportFilter::EPUBExportFilter(uno::Reference<uno::XComponentContext> xContext)
    : mxContext(std::move(xContext))
{
}

sal_Int32 EPUBExportFilter::GetDefaultVersion() { return 30; }

sal_Int32 EPUBExportFilter::GetDefaultSplitMethod()
{
    return libepubgen::EPUB_SPLIT_METHOD_HEADING;
}

sal_Int32 EPUBExportFilter::GetDefaultLayoutMethod()
{
    return libepubgen::EPUB_LAYOUT_METHOD_REFLOWABLE;
}

sal_Bool EPUBExportFilter::filter(const uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    sal_Int32 nVersion = EPUBExportFilter::GetDefaultVersion();
    sal_Int32 nSplitMethod = EPUBExportFilter::GetDefaultSplitMethod();
    sal_Int32 nLayoutMethod = EPUBExportFilter::GetDefaultLayoutMethod();
    uno::Sequence<beans::PropertyValue> aFilterData;
    OUString aFilterOptions;
    for (const auto& rProp : rDescriptor)
    {
        if (rProp.Name == "FilterData")
            rProp.Value >>= aFilterData;
        else if (rProp.Name == "FilterOptions")
            rProp.Value >>= aFilterOptions;
    }

    if (aFilterOptions == "layout=fixed")
        nLayoutMethod = libepubgen::EPUB_LAYOUT_METHOD_FIXED;

    for (const auto& rProp : aFilterData)
    {
        if (rProp.Name == "EPUBVersion")
            rProp.Value >>= nVersion;
        else if (rProp.Name == "EPUBSplitMethod")
            rProp.Value >>= nSplitMethod;
        else if (rProp.Name == "EPUBLayoutMethod")
            rProp.Value >>= nLayoutMethod;
    }

    // Build the export filter chain: the package has direct access to the ZIP
    // file, the flat ODF filter has access to the doc model, everything else
    // is in-between.
    EPUBPackage aPackage(mxContext, rDescriptor);
    libepubgen::EPUBTextGenerator aGenerator(&aPackage, nVersion);
    aGenerator.setOption(libepubgen::EPUB_GENERATOR_OPTION_SPLIT, nSplitMethod);
    aGenerator.setOption(libepubgen::EPUB_GENERATOR_OPTION_LAYOUT, nLayoutMethod);
    OUString aSourceURL;
    uno::Reference<frame::XModel> xSourceModel(mxSourceDocument, uno::UNO_QUERY);
    if (xSourceModel.is())
        aSourceURL = xSourceModel->getURL();

    std::vector<exp::FixedLayoutPage> aPageMetafiles;
    if (nLayoutMethod == libepubgen::EPUB_LAYOUT_METHOD_FIXED)
        CreateMetafiles(aPageMetafiles);

    uno::Reference<xml::sax::XDocumentHandler> xExportHandler(
        new exp::XMLImport(mxContext, aGenerator, aSourceURL, rDescriptor, aPageMetafiles));

    uno::Reference<lang::XInitialization> xInitialization(
        mxContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.comp.Writer.XMLOasisExporter"_ustr, mxContext),
        uno::UNO_QUERY);

    // A subset of parameters are passed in as a property set.
    static comphelper::PropertyMapEntry const aInfoMap[]
        = { { u"BaseURI"_ustr, 0, cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 } };
    uno::Reference<beans::XPropertySet> xInfoSet(
        comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(aInfoMap)));
    xInfoSet->setPropertyValue(u"BaseURI"_ustr, uno::Any(aSourceURL));

    xInitialization->initialize({ uno::Any(xExportHandler), uno::Any(xInfoSet) });
    uno::Reference<document::XExporter> xExporter(xInitialization, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxSourceDocument);
    uno::Reference<document::XFilter> xFilter(xInitialization, uno::UNO_QUERY);

    return xFilter->filter(rDescriptor);
}

void EPUBExportFilter::CreateMetafiles(std::vector<exp::FixedLayoutPage>& rPageMetafiles)
{
    DocumentToGraphicRenderer aRenderer(mxSourceDocument, /*bSelectionOnly=*/false);
    uno::Reference<frame::XModel> xModel(mxSourceDocument, uno::UNO_QUERY);
    if (!xModel.is())
        return;

    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    if (!xTextViewCursorSupplier.is())
        return;

    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    if (!xCursor.is())
        return;

    xCursor->jumpToLastPage();
    sal_Int16 nPages = xCursor->getPage();
    for (sal_Int16 nPage = 1; nPage <= nPages; ++nPage)
    {
        Size aDocumentSizePixel = aRenderer.getDocumentSizeInPixels(nPage);
        Size aLogic = aRenderer.getDocumentSizeIn100mm(nPage);
        // Get the CSS pixel size of the page (mm100 -> pixel using 96 DPI, independent from system DPI).
        Size aCss(static_cast<double>(aLogic.getWidth()) / 26.4583,
                  static_cast<double>(aLogic.getHeight()) / 26.4583);
        Graphic aGraphic = aRenderer.renderToGraphic(nPage, aDocumentSizePixel, aCss, COL_WHITE,
                                                     /*bExtOutDevData=*/true);
        auto& rGDIMetaFile = const_cast<GDIMetaFile&>(aGraphic.GetGDIMetaFile());

        // Set preferred map unit and size on the metafile, so the SVG size
        // will be correct in MM.
        MapMode aMapMode;
        aMapMode.SetMapUnit(MapUnit::Map100thMM);
        rGDIMetaFile.SetPrefMapMode(aMapMode);
        rGDIMetaFile.SetPrefSize(aLogic);

        SvMemoryStream aMemoryStream;
        SvmWriter aWriter(aMemoryStream);
        aWriter.Write(rGDIMetaFile);
        exp::FixedLayoutPage aPage;
        aPage.aMetafile = uno::Sequence<sal_Int8>(
            static_cast<const sal_Int8*>(aMemoryStream.GetData()), aMemoryStream.Tell());
        aPage.aCssPixels = aCss;
        aPage.aChapterNames = aRenderer.getChapterNames();
        rPageMetafiles.push_back(aPage);
    }
}

void EPUBExportFilter::cancel() {}

void EPUBExportFilter::setSourceDocument(const uno::Reference<lang::XComponent>& xDocument)
{
    mxSourceDocument = xDocument;
}

OUString EPUBExportFilter::getImplementationName()
{
    return u"com.sun.star.comp.Writer.EPUBExportFilter"_ustr;
}

sal_Bool EPUBExportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> EPUBExportFilter::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet = { u"com.sun.star.document.ExportFilter"_ustr };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Writer_EPUBExportFilter_get_implementation(
    uno::XComponentContext* pContext, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new EPUBExportFilter(pContext));
}

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
