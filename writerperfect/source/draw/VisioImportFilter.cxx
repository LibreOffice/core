/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* VisioImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libvisio/libvisio.h>
#include <libodfgen/libodfgen.hxx>

#include "VisioImportFilter.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool VisioImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdgGenerator &rGenerator, utl::MediaDescriptor &)
{
    return libvisio::VisioDocument::parse(&rInput, &rGenerator);
}

bool VisioImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    if (libvisio::VisioDocument::isSupported(&rInput))
    {
        rTypeName = "draw_Visio_Document";
        return true;
    }

    return false;
}

OUString VisioImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Draw.VisioImportFilter");
}

Sequence< OUString > SAL_CALL VisioImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL VisioImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new VisioImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL VisioImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return VisioImportFilter_getImplementationName();
}
sal_Bool SAL_CALL VisioImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL VisioImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return VisioImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
