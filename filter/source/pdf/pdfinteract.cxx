/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
    return ServiceName == SERVICE_NAME;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
