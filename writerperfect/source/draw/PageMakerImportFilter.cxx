/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* PageMakerImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libodfgen/libodfgen.hxx>

#include <libpagemaker/libpagemaker.h>

#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/supportsservice.hxx>

#include "PageMakerImportFilter.hxx"

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool PageMakerImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdgGenerator &rGenerator, utl::MediaDescriptor &)
{
    return libpagemaker::PMDocument::parse(&rInput, &rGenerator);
}

bool PageMakerImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    if (libpagemaker::PMDocument::isSupported(&rInput))
    {
        rTypeName = "draw_PageMaker_Document";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL PageMakerImportFilter::getImplementationName()
{
    return OUString("org.libreoffice.comp.Draw.PageMakerImportFilter");
}

sal_Bool SAL_CALL PageMakerImportFilter::supportsService(const OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL PageMakerImportFilter::getSupportedServiceNames()
{
    Sequence< OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ImportFilter";
    pArray[1] = "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

extern "C"
SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
org_libreoffice_comp_Draw_PageMakerImportFilter_get_implementation(
    css::uno::XComponentContext *const context,
    const css::uno::Sequence<css::uno::Any> &)
{
    return cppu::acquire(new PageMakerImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
