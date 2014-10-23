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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/supportsservice.hxx>

#include "PageMakerImportFilter.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
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

OUString PageMakerImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("org.libreoffice.comp.Draw.PageMakerImportFilter");
}

Sequence< OUString > SAL_CALL PageMakerImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence< OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ImportFilter";
    pArray[1] = "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL PageMakerImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return (cppu::OWeakObject *) new PageMakerImportFilter(rContext);
}

// XServiceInfo
OUString SAL_CALL PageMakerImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return PageMakerImportFilter_getImplementationName();
}
sal_Bool SAL_CALL PageMakerImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL PageMakerImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return PageMakerImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
