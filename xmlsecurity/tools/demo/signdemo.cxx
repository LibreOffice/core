/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: signdemo.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:38:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "util.hxx"

#include <stdio.h>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <cppuhelper/servicefactory.hxx>

#include <xmlsecurity/biginteger.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>

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
    aSignatureHelper.SetX509Certificate( nSecurityId, nEnvIndex, xPersonalCert->getIssuerName(), bigIntegerToNumericString( xPersonalCert->getSerialNumber()));

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

