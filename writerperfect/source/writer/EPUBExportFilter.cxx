/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EPUBExportFilter.hxx"

#include <config_libepubgen.h>

#include <libepubgen/EPUBTextGenerator.h>
#include <libepubgen/libepubgen-decls.h>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "exp/xmlimp.hxx"
#include "EPUBPackage.hxx"

using namespace com::sun::star;

namespace writerperfect
{

EPUBExportFilter::EPUBExportFilter(const uno::Reference<uno::XComponentContext> &xContext)
    : mxContext(xContext)
{
}

sal_Int32 EPUBExportFilter::GetDefaultVersion()
{
    return 30;
}

sal_Int32 EPUBExportFilter::GetDefaultSplitMethod()
{
    return libepubgen::EPUB_SPLIT_METHOD_HEADING;
}

sal_Int32 EPUBExportFilter::GetDefaultLayoutMethod()
{
#if LIBEPUBGEN_VERSION_SUPPORT
    return libepubgen::EPUB_LAYOUT_METHOD_REFLOWABLE;
#else
    return 0;
#endif
}

sal_Bool EPUBExportFilter::filter(const uno::Sequence<beans::PropertyValue> &rDescriptor)
{
    sal_Int32 nVersion = EPUBExportFilter::GetDefaultVersion();
    sal_Int32 nSplitMethod = EPUBExportFilter::GetDefaultSplitMethod();
    sal_Int32 nLayoutMethod = EPUBExportFilter::GetDefaultLayoutMethod();
    uno::Sequence<beans::PropertyValue> aFilterData;
    for (sal_Int32 i = 0; i < rDescriptor.getLength(); ++i)
    {
        if (rDescriptor[i].Name == "FilterData")
        {
            rDescriptor[i].Value >>= aFilterData;
            break;
        }
    }

    for (sal_Int32 i = 0; i < aFilterData.getLength(); ++i)
    {
        if (aFilterData[i].Name == "EPUBVersion")
            aFilterData[i].Value >>= nVersion;
        else if (aFilterData[i].Name == "EPUBSplitMethod")
            aFilterData[i].Value >>= nSplitMethod;
        else if (aFilterData[i].Name == "EPUBLayoutMethod")
            aFilterData[i].Value >>= nLayoutMethod;
    }

    // Build the export filter chain: the package has direct access to the ZIP
    // file, the flat ODF filter has access to the doc model, everything else
    // is in-between.
    EPUBPackage aPackage(mxContext, rDescriptor);
    libepubgen::EPUBTextGenerator aGenerator(&aPackage, static_cast<libepubgen::EPUBSplitMethod>(nSplitMethod)
#if LIBEPUBGEN_VERSION_SUPPORT
                                             , nVersion
#endif
                                            );
#if LIBEPUBGEN_VERSION_SUPPORT
    aGenerator.setLayoutMethod(static_cast<libepubgen::EPUBLayoutMethod>(nLayoutMethod));
#endif
    OUString aSourceURL;
    uno::Reference<frame::XModel> xSourceModel(mxSourceDocument, uno::UNO_QUERY);
    if (xSourceModel.is())
        aSourceURL = xSourceModel->getURL();
    uno::Reference<xml::sax::XDocumentHandler> xExportHandler(new exp::XMLImport(mxContext, aGenerator, aSourceURL, rDescriptor));

    uno::Reference<lang::XInitialization> xInitialization(mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.comp.Writer.XMLOasisExporter", mxContext), uno::UNO_QUERY);

    // A subset of parameters are passed in as a property set.
    comphelper::PropertyMapEntry const aInfoMap[] =
    {
        {OUString("BaseURI"), 0, cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0},
        {OUString(), 0, css::uno::Type(), 0, 0}
    };
    uno::Reference<beans::XPropertySet> xInfoSet(comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(aInfoMap)));
    xInfoSet->setPropertyValue("BaseURI", uno::makeAny(aSourceURL));

    xInitialization->initialize({uno::makeAny(xExportHandler), uno::makeAny(xInfoSet)});
    uno::Reference<document::XExporter> xExporter(xInitialization, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxSourceDocument);
    uno::Reference<document::XFilter> xFilter(xInitialization, uno::UNO_QUERY);
    return xFilter->filter(rDescriptor);
}

void EPUBExportFilter::cancel()
{
}

void EPUBExportFilter::setSourceDocument(const uno::Reference<lang::XComponent> &xDocument)
{
    mxSourceDocument = xDocument;
}

OUString EPUBExportFilter::getImplementationName()
{
    return OUString("com.sun.star.comp.Writer.EPUBExportFilter");
}

sal_Bool EPUBExportFilter::supportsService(const OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> EPUBExportFilter::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet =
    {
        OUString("com.sun.star.document.ExportFilter")
    };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface *SAL_CALL com_sun_star_comp_Writer_EPUBExportFilter_get_implementation(uno::XComponentContext *pContext, uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new EPUBExportFilter(pContext));
}

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
