/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSWorksCalcImportFilter: Sets up the filter, and calls DocumentCollector
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

#include "MSWorksCalcImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

bool MSWorksCalcImportFilter::doImportDocument( librevenge::RVNGInputStream &rInput, librevenge::RVNGSpreadsheetInterface &rGenerator )
{
    return libwps::WPS_OK == libwps::WPSDocument::parse(&rInput, &rGenerator);
}

bool MSWorksCalcImportFilter::doDetectFormat( librevenge::RVNGInputStream &rInput, OUString &rTypeName )
{
    libwps::WPSKind kind = libwps::WPS_TEXT;
    const libwps::WPSConfidence confidence = libwps::WPSDocument::isFileFormatSupported(&rInput, kind);

    if ((kind == libwps::WPS_SPREADSHEET || kind == libwps::WPS_DATABASE) && (confidence == libwps::WPS_CONFIDENCE_EXCELLENT))
    {
        rTypeName = "calc_MS_Works_Document";
        return true;
    }

    return false;
}

void MSWorksCalcImportFilter::doRegisterHandlers( OdsGenerator & )
{
}

OUString MSWorksCalcImportFilter_getImplementationName ()
throw (RuntimeException)
{
    return OUString (  "com.sun.star.comp.Calc.MSWorksCalcImportFilter"  );
}

Sequence< OUString > SAL_CALL MSWorksCalcImportFilter_getSupportedServiceNames(  )
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

Reference< XInterface > SAL_CALL MSWorksCalcImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    return (cppu::OWeakObject *) new MSWorksCalcImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL MSWorksCalcImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    return MSWorksCalcImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MSWorksCalcImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}
Sequence< OUString > SAL_CALL MSWorksCalcImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    return MSWorksCalcImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
