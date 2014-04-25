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

#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/supportsservice.hxx>

#include <libabw/libabw.h>

#include "AbiWordImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool AbiWordImportFilter::doImportDocument( WPXInputStream &rInput, const rtl::OUString &, WPXDocumentInterface &rGenerator )
{
    return libabw::AbiDocument::parse(&rInput, &rGenerator);
}

bool AbiWordImportFilter::doDetectFormat( WPXInputStream &rInput, OUString &rTypeName )
{
    if (libabw::AbiDocument::isFileFormatSupported(&rInput))
    {
        rTypeName = "writer_AbiWord_Document";
        return true;
    }

    return false;
}

OUString AbiWordImportFilter_getImplementationName ()
throw (RuntimeException)
{
    return OUString (  "com.sun.star.comp.Writer.AbiWordImportFilter"  );
}

Sequence< OUString > SAL_CALL AbiWordImportFilter_getSupportedServiceNames(  )
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

Reference< XInterface > SAL_CALL AbiWordImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    return (cppu::OWeakObject *) new AbiWordImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL AbiWordImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    return AbiWordImportFilter_getImplementationName();
}
sal_Bool SAL_CALL AbiWordImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}
Sequence< OUString > SAL_CALL AbiWordImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    return AbiWordImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
