/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSWorksImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/supportsservice.hxx>

#include <libwps/libwps.h>

#include "MSWorksImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

bool MSWorksImportFilter::doImportDocument( WPXInputStream &rInput, const rtl::OUString &, WPXDocumentInterface &rGenerator )
{
    return WPS_OK == WPSDocument::parse(&rInput, &rGenerator);
}

bool MSWorksImportFilter::doDetectFormat( WPXInputStream &rInput, OUString &rTypeName )
{
    const WPSConfidence confidence = WPSDocument::isFileFormatSupported(&rInput);

    if ((confidence == WPS_CONFIDENCE_EXCELLENT) || (confidence == WPS_CONFIDENCE_GOOD))
    {
        rTypeName = "writer_MS_Works_Document";
        return true;
    }

    return false;
}

OUString MSWorksImportFilter_getImplementationName ()
throw (RuntimeException)
{
    return OUString (  "com.sun.star.comp.Writer.MSWorksImportFilter"  );
}

Sequence< OUString > SAL_CALL MSWorksImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}
#undef SERVICE_NAME2
#undef SERVICE_NAME1

Reference< XInterface > SAL_CALL MSWorksImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    return (cppu::OWeakObject *) new MSWorksImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL MSWorksImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    return MSWorksImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MSWorksImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}
Sequence< OUString > SAL_CALL MSWorksImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    return MSWorksImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
