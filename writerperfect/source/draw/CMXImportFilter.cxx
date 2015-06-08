/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* CMXImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libcdr/libcdr.h>
#include <libodfgen/libodfgen.hxx>

#include "CMXImportFilter.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool CMXImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdgGenerator &rGenerator, utl::MediaDescriptor &)
{
    return libcdr::CMXDocument::parse(&rInput, &rGenerator);
}

bool CMXImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    if (libcdr::CMXDocument::isSupported(&rInput))
    {
        rTypeName = "draw_Corel_Presentation_Exchange";
        return true;
    }

    return false;
}

OUString CMXImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Draw.CMXImportFilter");
}

Sequence< OUString > SAL_CALL CMXImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ImportFilter";
    pArray[1] = "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL CMXImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new CMXImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL CMXImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return CMXImportFilter_getImplementationName();
}
sal_Bool SAL_CALL CMXImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL CMXImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return CMXImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
