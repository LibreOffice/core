/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* EBookImportFilter: Sets up the filter, and calls DocumentCollector
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

#include <libe-book/libe-book.h>

#include "EBookImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

bool EBookImportFilter::doImportDocument( WPXInputStream &rInput, const rtl::OUString &rFilterName, WPXDocumentInterface &rGenerator )
{
    if (rFilterName == "FictionBook 2")
        return libebook::FB2Document::parse(&rInput, &rGenerator);
    else if (rFilterName == "PalmDoc")
        return libebook::PDBDocument::parse(&rInput, &rGenerator);
    else if (rFilterName == "Plucker eBook")
        return libebook::PLKRDocument::parse(&rInput, &rGenerator);
    else if (rFilterName == "eReader eBook")
        return libebook::PMLDocument::parse(&rInput, &rGenerator);
    else if (rFilterName == "TealDoc")
        return libebook::TDDocument::parse(&rInput, &rGenerator);
    else if (rFilterName == "zTXT")
        return libebook::ZTXTDocument::parse(&rInput, &rGenerator);

    return false;
}

bool EBookImportFilter::doDetectFormat( WPXInputStream &rInput, OUString &rTypeName )
{
    rTypeName = "";

    if (libebook::FB2Document::isSupported(&rInput))
        rTypeName = "writer_FictionBook_2";
    else if (libebook::PDBDocument::isSupported(&rInput))
        rTypeName = "writer_PalmDoc";
    else if (libebook::PLKRDocument::isSupported(&rInput))
        rTypeName = "writer_Plucker_eBook";
    else if (libebook::PMLDocument::isSupported(&rInput))
        rTypeName = "writer_eReader_eBook";
    else if (libebook::TDDocument::isSupported(&rInput))
        rTypeName = "writer_TealDoc";
    else if (libebook::ZTXTDocument::isSupported(&rInput))
        rTypeName = "writer_zTXT";

    return !rTypeName.isEmpty();
}

OUString EBookImportFilter_getImplementationName ()
throw (RuntimeException)
{
    return OUString (  "org.libreoffice.comp.Writer.EBookImportFilter"  );
}

Sequence< OUString > SAL_CALL EBookImportFilter_getSupportedServiceNames(  )
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

Reference< XInterface > SAL_CALL EBookImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    return (cppu::OWeakObject *) new EBookImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL EBookImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    return EBookImportFilter_getImplementationName();
}
sal_Bool SAL_CALL EBookImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}
Sequence< OUString > SAL_CALL EBookImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    return EBookImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
