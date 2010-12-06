 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "pdfinteract.hxx"
#include "impdialog.hxx"

#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/task/PDFExportException.hpp"

// -------------
// - PDFInteractionHandler -
// -------------

PDFInteractionHandler::PDFInteractionHandler( const Reference< XMultiServiceFactory > &rxMSF ) :
    mxMSF( rxMSF )
{
}

// -----------------------------------------------------------------------------

PDFInteractionHandler::~PDFInteractionHandler()
{
}


void SAL_CALL PDFInteractionHandler::handle( const Reference< task::XInteractionRequest >& i_xRequest )
    throw (RuntimeException)
{
    handleInteractionRequest( i_xRequest );
}

sal_Bool SAL_CALL PDFInteractionHandler::handleInteractionRequest( const Reference< task::XInteractionRequest >& i_xRequest )
    throw (RuntimeException)
{
    sal_Bool bHandled = sal_False;

    Any aRequest( i_xRequest->getRequest() );
    task::PDFExportException aExc;
    if( aRequest >>= aExc )
    {
        std::set< vcl::PDFWriter::ErrorCode > aCodes;
        sal_Int32 nCodes = aExc.ErrorCodes.getLength();
        for( sal_Int32 i = 0; i < nCodes; i++ )
            aCodes.insert( (vcl::PDFWriter::ErrorCode)aExc.ErrorCodes.getConstArray()[i] );
        ImplErrorDialog aDlg( aCodes );
        aDlg.Execute();
        bHandled = sal_True;
    }
    return bHandled;
}

// -----------------------------------------------------------------------------

OUString PDFInteractionHandler_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.PDF.PDFExportInteractionHandler" ) );
}

// -----------------------------------------------------------------------------

#define SERVICE_NAME "com.sun.star.filter.pdfexport.PDFExportInteractionHandler"

sal_Bool SAL_CALL PDFInteractionHandler_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL PDFInteractionHandler_getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}

#undef SERVICE_NAME

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL PDFInteractionHandler_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw( Exception )
{
    return (cppu::OWeakObject*) new PDFInteractionHandler( rSMgr );
}

// -----------------------------------------------------------------------------

OUString SAL_CALL PDFInteractionHandler::getImplementationName()
    throw (RuntimeException)
{
    return PDFInteractionHandler_getImplementationName();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL PDFInteractionHandler::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return PDFInteractionHandler_supportsService( rServiceName );
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< OUString > SAL_CALL PDFInteractionHandler::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return PDFInteractionHandler_getSupportedServiceNames();
}
