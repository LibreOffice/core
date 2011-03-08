/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_xmlsecurity.hxx"

#include "util.hxx"

#include <stdio.h>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <cppuhelper/servicefactory.hxx>

#include <xmlsecurity/biginteger.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>
#include "xmlsecurity/baseencoding.hxx"

using namespace ::com::sun::star;

int SAL_CALL main( int argc, char **argv )
{
    if( argc < 4 )
    {
        fprintf( stderr, "Usage: %s <signature file> <xml stream file> <binary stream file> [<cryptoken>]\n" , argv[0] ) ;
        return -1 ;
    }

    rtl::OUString aSIGFileName = rtl::OUString::createFromAscii(argv[1]);
    rtl::OUString aXMLFileName = rtl::OUString::createFromAscii(argv[2]);
    rtl::OUString aBINFileName = rtl::OUString::createFromAscii(argv[3]);
    rtl::OUString aCryptoToken;
    if ( argc >= 5 )
        aCryptoToken = rtl::OUString::createFromAscii(argv[4]);

    uno::Reference< lang::XMultiServiceFactory > xMSF = CreateDemoServiceFactory();

    /*
     * creates a signature helper
     */
    XMLSignatureHelper aSignatureHelper( xMSF );

    /*
     * creates a security context.
     */
    bool bInit = aSignatureHelper.Init( aCryptoToken );
    if ( !bInit )
    {
        fprintf( stderr, "Error initializing security context!\n" );
        return -1;
    }

    aSignatureHelper.StartMission();

    /*
     * select a private key certificate
     */
    sal_Int32 i;
    sal_Int32 nEnvCount = aSignatureHelper.GetSecurityEnvironmentNumber();
    if( nEnvCount == 0 )
    {
        fprintf( stdout, "\nNo SecurityEnvironment found!\n" ) ;
        return -1;
    }

    uno::Sequence< uno::Reference< xml::crypto::XSecurityEnvironment > > xSecurityEnvironments(nEnvCount) ;
    for( i=0; i < nEnvCount; i++ )
        xSecurityEnvironments[i] = aSignatureHelper.GetSecurityEnvironmentByIndex(i);

    fprintf( stdout, "\nSelect a SecurityEnvironment:\n" ) ;
    for( i = 0; i < nEnvCount; i ++ )
        fprintf( stdout, "\n[%d] %s", i+1, rtl::OUStringToOString( xSecurityEnvironments[i]->getSecurityEnvironmentInformation() ,RTL_TEXTENCODING_ASCII_US ).getStr());

    sal_Int32 nEnvIndex = QuerySelectNumber( 1, nEnvCount ) -1;

    uno::Reference< ::com::sun::star::security::XCertificate > xPersonalCert = getCertificateFromEnvironment(xSecurityEnvironments[nEnvIndex], true);

    if ( !xPersonalCert.is() )
    {
        fprintf( stdout, "No certificate choosen - exit.\n" );
        return (-2);
    }

    /*
     * creates a new signature id
     */
    sal_Int32 nSecurityId = aSignatureHelper.GetNewSecurityId();

    /*
     * configures the X509 certificate
     */
    aSignatureHelper.SetX509Certificate(
        nSecurityId, nEnvIndex,
        xPersonalCert->getIssuerName(),
        bigIntegerToNumericString( xPersonalCert->getSerialNumber()),
        baseEncode(xPersonalCert->getEncoded(), BASE64));

    /*
     * configures date/time
     */
    aSignatureHelper.SetDateTime( nSecurityId, Date(), Time());

    /*
     * signs the xml stream
     */
    aSignatureHelper.AddForSigning( nSecurityId, aXMLFileName, aXMLFileName, sal_False );

    /*
     * signs the binary stream
     */
    aSignatureHelper.AddForSigning( nSecurityId, aBINFileName, aBINFileName, sal_True );

    /*
     * creates signature
     */
    uno::Reference< io::XOutputStream > xOutputStream = OpenOutputStream( aSIGFileName );
    bool bDone = aSignatureHelper.CreateAndWriteSignature( xOutputStream );

    if ( !bDone )
    {
        fprintf( stderr, "\nSTATUS: Error creating Signature!\n" );
    }
    else
    {
        fprintf( stdout, "\nSTATUS: Signature successfully created!\n" );
    }

    aSignatureHelper.EndMission();

    QueryPrintSignatureDetails( aSignatureHelper.GetSignatureInformations(), aSignatureHelper.GetSecurityEnvironment() );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
