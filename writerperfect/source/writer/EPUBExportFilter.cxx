/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EPUBExportFilter.hxx"

#include "config_libepubgen.h"

#include <libepubgen/EPUBTextGenerator.h>
#include <libepubgen/libepubgen-decls.h>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

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

sal_Bool EPUBExportFilter::filter(const uno::Sequence<beans::PropertyValue> &rDescriptor)
{
    sal_Int32 nVersion = 30;
    sal_Int32 nSplitMethod = libepubgen::EPUB_SPLIT_METHOD_HEADING;
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
    uno::Reference<xml::sax::XDocumentHandler> xExportHandler(new exp::XMLImport(aGenerator));

    uno::Reference<lang::XInitialization> xInitialization(mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.comp.Writer.XMLOasisExporter", mxContext), uno::UNO_QUERY);
    xInitialization->initialize({uno::makeAny(xExportHandler)});
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
