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

#include <rtl/ustring.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>

// Get the demo.rdb servcie manager...
::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > CreateDemoServiceFactory();

// Ask user to show more signature details...
void QueryPrintSignatureDetails( const SignatureInformations& SignatureInformations, ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > );

// Query value from user.
int QuerySelectNumber( int nMin, int nMax );

// Ask to verify the signature
long QueryVerifySignature();

// Open In/Output Stream
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > OpenInputStream( const ::rtl::OUString& rStreamName );
::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > OpenOutputStream( const ::rtl::OUString& rStreamName );

::rtl::OUString getSignatureInformations( const SignatureInformations& SignatureInformations, ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecurityEnvironment );
::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > getCertificateFromEnvironment( ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecurityEnvironment, sal_Bool nType);
