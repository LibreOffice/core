/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* AbiWordImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/supportsservice.hxx>

#include <libabw/libabw.h>

#include "AbiWordImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool AbiWordImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdtGenerator &rGenerator, utl::MediaDescriptor &)
{
    return libabw::AbiDocument::parse(&rInput, &rGenerator);
}

bool AbiWordImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    if (libabw::AbiDocument::isFileFormatSupported(&rInput))
    {
        rTypeName = "writer_AbiWord_Document";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL AbiWordImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.Writer.AbiWordImportFilter");
}

sal_Bool SAL_CALL AbiWordImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL AbiWordImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

extern "C"
SAL_DLLPUBLIC_EXPORT css::uno::XInterface *SAL_CALL
com_sun_star_comp_Writer_AbiWordImportFilter_get_implementation(
    css::uno::XComponentContext *const context,
    const css::uno::Sequence<css::uno::Any> &)
{
    return cppu::acquire(new AbiWordImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
