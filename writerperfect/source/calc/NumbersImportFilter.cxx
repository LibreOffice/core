/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* NumbersImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libetonyek/libetonyek.h>

#include <com/sun/star/uno/Reference.h>

#include <cppuhelper/supportsservice.hxx>

#include "NumbersImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

using libetonyek::EtonyekDocument;

bool NumbersImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdsGenerator &rGenerator, utl::MediaDescriptor &)
{
    return EtonyekDocument::parse(&rInput, &rGenerator);
}

bool NumbersImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    EtonyekDocument::Type type = EtonyekDocument::TYPE_UNKNOWN;
    const EtonyekDocument::Confidence confidence = EtonyekDocument::isSupported(&rInput, &type);
    if ((confidence == EtonyekDocument::CONFIDENCE_EXCELLENT) && (type == EtonyekDocument::TYPE_NUMBERS))
    {
        rTypeName = "calc_AppleNumbers";
        return true;
    }

    return false;
}

void NumbersImportFilter::doRegisterHandlers(OdsGenerator &)
{
}

OUString NumbersImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("org.libreoffice.comp.Calc.NumbersImportFilter");
}

Sequence< OUString > SAL_CALL NumbersImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL NumbersImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new NumbersImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL NumbersImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return NumbersImportFilter_getImplementationName();
}
sal_Bool SAL_CALL NumbersImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL NumbersImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return NumbersImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
