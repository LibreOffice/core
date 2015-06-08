/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSPUBImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libmspub/libmspub.h>
#include <libodfgen/libodfgen.hxx>

#include "MSPUBImportFilter.hxx"

#include <iostream>

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool MSPUBImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdgGenerator &rGenerator, utl::MediaDescriptor &)
{
    return libmspub::MSPUBDocument::parse(&rInput, &rGenerator);
}

bool MSPUBImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    if (libmspub::MSPUBDocument::isSupported(&rInput))
    {
        rTypeName = "draw_Publisher_Document";
        return true;
    }

    return false;
}

OUString MSPUBImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Draw.MSPUBImportFilter");
}

Sequence< OUString > SAL_CALL MSPUBImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL MSPUBImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new MSPUBImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL MSPUBImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return MSPUBImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MSPUBImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL MSPUBImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return MSPUBImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
