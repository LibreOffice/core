/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: multisigdemo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:37:57 $
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

#include <stdio.h>
#include "util.hxx"

#include <rtl/ustring.hxx>
#include <cppuhelper/servicefactory.hxx>

#include <xmlsecurity/biginteger.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>

#include <tools/date.hxx>
#include <tools/time.hxx>

using namespace ::com::sun::star;

long denyVerifyHandler( void *, void * )
{
    return  0;
}

long startVerifyHandler( void *, void * )
{
    return QueryVerifySignature();
}

int SAL_CALL main( int argc, char **argv )
{
    if( argc < 5 )
    {
        fprintf( stderr, "Usage: %s <signature file 1> <signature file 2> <xml stream file> <binary stream file> [<cryptoken>]\n" , argv[0] ) ;
        return -1 ;
    }

    uno::Reference< lang::XMultiServiceFactory > xMSF = CreateDemoServiceFactory();

    rtl::OUString aSIGFileName = rtl::OUString::createFromAscii(argv[1]);
    rtl::OUString aSIGFileName2 = rtl::OUString::createFromAscii(argv[2]);
    rtl::OUString aXMLFileName = rtl::OUString::createFromAscii(argv[3]);
    rtl::OUString aBINFileName = rtl::OUString::createFromAscii(argv[4]);
    rtl::OUString aCryptoToken;
    if ( argc >= 7 )
        aCryptoToken = rtl::OUString::createFromAscii(argv[6]);

    sal_Int32 nSecurityId;
    uno::Reference< io::XOutputStream > xOutputStream;
    uno::Reference< io::XInputStream > xInputStream;
    bool bDone;
    SignatureInformations signatureInformations;
    uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler> xDocumentHandler;

    // -------- START -------

    XMLSignatureHelper aSignatureHelper( xMSF );

    bool bInit = aSignatureHelper.Init( aCryptoToken );
    if ( !bInit )
    {
        fprintf( stderr, "Error initializing security context!\n" );
        return -1;
    }

    fprintf( stdout, "\n\nTEST MISSION 1: Create the first signature file\n");

    aSignatureHelper.StartMission();

    /*
     * select a private key certificate
     */
    uno::Reference< xml::crypto::XSecurityEnvironment > xSecurityEnvironment = aSignatureHelper.GetSecurityEnvironment();
    uno::Sequence< uno::Reference< ::com::sun::star::security::XCertificate > > xPersonalCerts = xSecurityEnvironment->getPersonalCertificates() ;

    fprintf( stdout, "\nPlease select two certificates:\n" );

    for ( int nSig = 0; nSig < 2; nSig++ )
    {
        // New security ID for signature...
        nSecurityId = aSignatureHelper.GetNewSecurityId();

        // Select certificate...
        uno::Reference< ::com::sun::star::security::XCertificate > xPersonalCert = getCertificateFromEnvironment( xSecurityEnvironment, true );
        aSignatureHelper.SetX509Certificate( nSecurityId, xPersonalCert->getIssuerName(), bigIntegerToNumericString( xPersonalCert->getSerialNumber()));
        aSignatureHelper.AddForSigning( nSecurityId, aXMLFileName, aXMLFileName, sal_False );
        aSignatureHelper.AddForSigning( nSecurityId, aBINFileName, aBINFileName, sal_True );
        aSignatureHelper.SetDateTime( nSecurityId, Date(), Time() );
    }
    /*
     * creates signature
     */
    xOutputStream = OpenOutputStream( aSIGFileName );
    bDone = aSignatureHelper.CreateAndWriteSignature( xOutputStream );
    if ( !bDone )
        fprintf( stderr, "\nSTATUS MISSION 1: Error creating Signature!\n" );
    else
        fprintf( stdout, "\nSTATUS MISSION 1: Signature successfully created!\n" );

    aSignatureHelper.EndMission();


    fprintf( stdout, "\n\nTEST MISSION 2: Transfer the second signature to a new signature file\n");

    /*
     * You can use an uninitialized SignatureHelper to perform this mission.
     */

    /*
     * configures the start-verify handler. Don't need to verify for transfering...
     */
    aSignatureHelper.SetStartVerifySignatureHdl( Link( NULL, denyVerifyHandler ) );
    aSignatureHelper.StartMission();

    xInputStream = OpenInputStream( aSIGFileName );
    bDone = aSignatureHelper.ReadAndVerifySignature( xInputStream );
    xInputStream->closeInput();

    if ( !bDone )
        fprintf( stderr, "\nSTATUS MISSION 2: Error in reading Signature!\n" );
    else
        fprintf( stdout, "\nSTATUS MISSION 2: Signature successfully transfered!\n" );

    /*
     * get all signature information
     */
    signatureInformations = aSignatureHelper.GetSignatureInformations();

    /*
     * write the first signature into the second signature file.
     */

    xOutputStream = OpenOutputStream( aSIGFileName2 );
    xDocumentHandler = aSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream);
    aSignatureHelper.ExportSignature( xDocumentHandler, signatureInformations[1]);
    aSignatureHelper.CloseDocumentHandler( xDocumentHandler);
    aSignatureHelper.EndMission();

    fprintf( stdout, "\n\nTEST MISSION 3: Insert a new signature to the first signature file\n");

    aSignatureHelper.StartMission();

    nSecurityId = aSignatureHelper.GetNewSecurityId();

    // Select certificate...
    uno::Reference< ::com::sun::star::security::XCertificate > xPersonalCert = getCertificateFromEnvironment( xSecurityEnvironment, true );
    aSignatureHelper.SetX509Certificate( nSecurityId, xPersonalCert->getIssuerName(), bigIntegerToNumericString( xPersonalCert->getSerialNumber()));
    aSignatureHelper.AddForSigning( nSecurityId, aXMLFileName, aXMLFileName, sal_False );
    aSignatureHelper.AddForSigning( nSecurityId, aBINFileName, aBINFileName, sal_True );
    aSignatureHelper.SetDateTime( nSecurityId, Date(), Time() );


    xOutputStream = OpenOutputStream( aSIGFileName );
    xDocumentHandler = aSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream);

    aSignatureHelper.ExportSignature( xDocumentHandler, signatureInformations[0]);
    bDone = aSignatureHelper.CreateAndWriteSignature( xDocumentHandler );
    aSignatureHelper.ExportSignature( xDocumentHandler, signatureInformations[1]);
    aSignatureHelper.CloseDocumentHandler( xDocumentHandler);

    if ( !bDone )
        fprintf( stderr, "\nSTATUS MISSION 3: Error creating Signature!\n" );
    else
        fprintf( stdout, "\nSTATUS MISSION 3: Signature successfully created!\n" );

    aSignatureHelper.EndMission();

    fprintf( stdout, "\n\nTEST MISSION 4 : Verify the first signature file\n");

    aSignatureHelper.SetStartVerifySignatureHdl( Link( NULL, startVerifyHandler ) );

    aSignatureHelper.StartMission();

    xInputStream = OpenInputStream( aSIGFileName );
    bDone = aSignatureHelper.ReadAndVerifySignature( xInputStream );
    xInputStream->closeInput();

    if ( !bDone )
        fprintf( stderr, "\nSTATUS MISSION 4: Error verifying Signatures!\n" );
    else
        fprintf( stdout, "\nSTATUS MISSION 4: All choosen Signatures veryfied successfully!\n" );

    aSignatureHelper.EndMission();

    QueryPrintSignatureDetails( aSignatureHelper.GetSignatureInformations(), aSignatureHelper.GetSecurityEnvironment() );

    fprintf( stdout, "\n\nTEST MISSION 5: Verify the second signature file\n");

    aSignatureHelper.StartMission();

    xInputStream = OpenInputStream( aSIGFileName2 );
    bDone = aSignatureHelper.ReadAndVerifySignature( xInputStream );
    xInputStream->closeInput();

    if ( !bDone )
        fprintf( stderr, "\nSTATUS MISSION 5: Error verifying Signatures!\n" );
    else
        fprintf( stdout, "\nSTATUS MISSION 5: All choosen Signatures veryfied successfully!\n" );

    aSignatureHelper.EndMission();

    QueryPrintSignatureDetails( aSignatureHelper.GetSignatureInformations(), aSignatureHelper.GetSecurityEnvironment() );

    return 0;
}
