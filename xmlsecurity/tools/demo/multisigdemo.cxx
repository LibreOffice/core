/*************************************************************************
 *
 *  $RCSfile: multisigdemo.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-26 07:29:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>
#include "util.hxx"

#include <rtl/ustring.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <unotools/streamhelper.hxx>

#include <xmlsecurity/biginteger.hxx>


namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssi = com::sun::star::io;

long denyVerifyHandler( void *, void * )
{
    return  0;
}

long startVerifyHandler( void *, void * )
{
    char answer;
    fprintf( stdout,
        "A signature is found, whether to verify it(y/n)?[y]:" );
    fscanf( stdin, "%c", &answer);

    return  (answer == 'n')?0:1;
}

int SAL_CALL main( int argc, char **argv )
{
    if( argc != 7 )
    {
        fprintf( stderr, "Usage: %s <rdb file> <signature file 1> <xml stream file> <binary stream file> <cryptoken> <signature file 2>\n" , argv[0] ) ;
        return -1 ;
    }

    /*
     * creates a component factory from local rdb file.
     */
    cssu::Reference< cssl::XMultiServiceFactory > xManager = NULL ;
    cssu::Reference< cssu::XComponentContext > xContext = NULL ;
    try
    {
        xManager = serviceManager( xContext , rtl::OUString::createFromAscii( "local" ), rtl::OUString::createFromAscii( argv[1] ) ) ;
        OSL_ENSURE( xManager.is() ,
            "ServicesManager - "
            "Cannot get service manager" );

        fprintf( stdout , "xManager created.\n" ) ;
    }
    catch( cssu::Exception& e )
    {
        fprintf( stderr , "Error Message: %s\n" , rtl::OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
        return -1;
    }

    rtl::OUString aXMLFileName = rtl::OUString::createFromAscii(argv[3]);
    rtl::OUString aBINFileName = rtl::OUString::createFromAscii(argv[4]);
    rtl::OUString aSIGFileName = rtl::OUString::createFromAscii(argv[2]);
    rtl::OUString aSIGFileName2 = rtl::OUString::createFromAscii(argv[6]);
    sal_Int32 nSecurityId;
    SvFileStream* pStream;
    ULONG nBytes;
    SvLockBytesRef xLockBytes;
    cssu::Reference< cssi::XOutputStream > xOutputStream;
    cssu::Reference< cssi::XInputStream > xInputStream;
    bool bDone;
    SignatureInformations signatureInformations;
    cssu::Reference< ::com::sun::star::xml::sax::XDocumentHandler> xDocumentHandler;

    XMLSignatureHelper aSignatureHelper( xManager );

    bool bInit = aSignatureHelper.Init( rtl::OUString::createFromAscii(argv[5]) );
    if ( !bInit )
    {
        fprintf( stderr, "Error initializing security context!\n" );
        return -1;
    }

    fprintf( stdout, "------ Mission 1 : create the first signature file ...\n");

    aSignatureHelper.StartMission();

    /*
     * select a private key certificate
     */
    cssu::Reference< cssxc::XSecurityEnvironment > xSecurityEnvironment = aSignatureHelper.GetSecurityEnvironment();
    cssu::Sequence< cssu::Reference< ::com::sun::star::security::XCertificate > > xPersonalCerts
        = xSecurityEnvironment->getPersonalCertificates() ;

    nSecurityId = aSignatureHelper.GetNewSecurityId();

    /*
     * use no.3 certificate to configure the X509 certificate
     */
    aSignatureHelper.SetX509Certificate(
        nSecurityId,
        xPersonalCerts[2]->getIssuerName(),
        bigIntegerToNumericString( xPersonalCerts[2]->getSerialNumber()));

    aSignatureHelper.AddForSigning( nSecurityId, aXMLFileName, aXMLFileName, sal_False );
    aSignatureHelper.AddForSigning( nSecurityId, aBINFileName, aBINFileName, sal_True );

    /*
     * creates another signature on the xml stream, use no.4 certificate
     */
    nSecurityId = aSignatureHelper.GetNewSecurityId();

    aSignatureHelper.SetX509Certificate(
        nSecurityId,
        xPersonalCerts[3]->getIssuerName(),
        bigIntegerToNumericString( xPersonalCerts[3]->getSerialNumber()));
    aSignatureHelper.AddForSigning( nSecurityId, aXMLFileName, aXMLFileName, sal_False );

    /*
     * creates the output stream
     */
    pStream = new SvFileStream( aSIGFileName, STREAM_WRITE );
    xLockBytes = new SvLockBytes( pStream, TRUE );
    xOutputStream = new utl::OOutputStreamHelper( xLockBytes );

    /*
     * creates signature
     */
    bDone = aSignatureHelper.CreateAndWriteSignature( xOutputStream );
    if ( !bDone )
    {
        fprintf( stderr, "Error creating Signature!\n" );
    }
    else
    {
        fprintf( stdout, "Signature successfully created!\n" );
    }

    aSignatureHelper.EndMission();


    fprintf( stdout, "------ Mission 2 : transfer the second signature to a new signature file ...\n");

    /*
     * You can use an uninitialized SignatureHelper to perform this mission.
     */

    /*
     * configures the start-verify handler
     */
    aSignatureHelper.SetStartVerifySignatureHdl( Link( NULL, denyVerifyHandler ) );
    aSignatureHelper.StartMission();

    pStream = new SvFileStream( aSIGFileName, STREAM_READ );
    pStream->Seek( STREAM_SEEK_TO_END );
    nBytes = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    xLockBytes = new SvLockBytes( pStream, TRUE );
    xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );

    bDone = aSignatureHelper.ReadAndVerifySignature( xInputStream );
    xInputStream->closeInput();

    if ( !bDone )
    {
        fprintf( stderr, "Error in reading Signature!\n" );
    }
    else
    {
        fprintf( stdout, "Signature successfully transfered!\n" );
    }

    /*
     * get all signature information
     */
    signatureInformations = aSignatureHelper.GetSignatureInformations();

    /*
     * write the first signature into the second signature file.
     */
    pStream = new SvFileStream( aSIGFileName2, STREAM_WRITE );
    xLockBytes = new SvLockBytes( pStream, TRUE );
    xOutputStream = new utl::OOutputStreamHelper( xLockBytes );

    xDocumentHandler = aSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream);

    aSignatureHelper.ExportSignature( xDocumentHandler, signatureInformations[1]);
    aSignatureHelper.CloseDocumentHandler( xDocumentHandler);
    aSignatureHelper.EndMission();

    fprintf( stdout, "------ Mission 3 : insert a new signature to the first signature file ...\n");

    aSignatureHelper.StartMission();

    nSecurityId = aSignatureHelper.GetNewSecurityId();

    /*
     * use no.5 certificate to the new signature
     */
    aSignatureHelper.SetX509Certificate(
        nSecurityId,
        xPersonalCerts[4]->getIssuerName(),
        bigIntegerToNumericString( xPersonalCerts[4]->getSerialNumber()));

    aSignatureHelper.AddForSigning( nSecurityId, aBINFileName, aBINFileName, sal_True );

    pStream = new SvFileStream( aSIGFileName, STREAM_WRITE );
    xLockBytes = new SvLockBytes( pStream, TRUE );
    xOutputStream = new utl::OOutputStreamHelper( xLockBytes );

    xDocumentHandler = aSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream);

    aSignatureHelper.ExportSignature( xDocumentHandler, signatureInformations[0]);
    bDone = aSignatureHelper.CreateAndWriteSignature( xDocumentHandler );
    aSignatureHelper.ExportSignature( xDocumentHandler, signatureInformations[1]);
    aSignatureHelper.CloseDocumentHandler( xDocumentHandler);

    if ( !bDone )
    {
        fprintf( stderr, "Error creating Signature!\n" );
    }
    else
    {
        fprintf( stdout, "Signature successfully created!\n" );
    }

    aSignatureHelper.EndMission();

    fprintf( stdout, "------ Mission 4 : verify the first signature file ...\n");

    aSignatureHelper.SetStartVerifySignatureHdl( Link( NULL, startVerifyHandler ) );

    aSignatureHelper.StartMission();

    pStream = new SvFileStream( aSIGFileName, STREAM_READ );
    pStream->Seek( STREAM_SEEK_TO_END );
    nBytes = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    xLockBytes = new SvLockBytes( pStream, TRUE );
    xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );

    bDone = aSignatureHelper.ReadAndVerifySignature( xInputStream );
    xInputStream->closeInput();

    if ( !bDone )
    {
        fprintf( stderr, "Error in Signature!\n" );
    }
    else
    {
        fprintf( stdout, "Signatures verified without any problems!\n" );
    }

    aSignatureHelper.EndMission();

    fprintf( stdout, "------------- Signature details -------------\n" );
    fprintf( stdout, "%s",
        rtl::OUStringToOString(
            getSignatureInformations(aSignatureHelper.GetSignatureInformations(), aSignatureHelper.GetSecurityEnvironment()),
            RTL_TEXTENCODING_UTF8).getStr());

    fprintf( stdout, "------ Mission 5 : verify the second signature file ...\n");

    aSignatureHelper.StartMission();

    pStream = new SvFileStream( aSIGFileName2, STREAM_READ );
    pStream->Seek( STREAM_SEEK_TO_END );
    nBytes = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    xLockBytes = new SvLockBytes( pStream, TRUE );
    xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );

    bDone = aSignatureHelper.ReadAndVerifySignature( xInputStream );
    xInputStream->closeInput();

    if ( !bDone )
    {
        fprintf( stderr, "Error in Signature!\n" );
    }
    else
    {
        fprintf( stdout, "Signatures verified without any problems!\n" );
    }

    aSignatureHelper.EndMission();

    fprintf( stdout, "------------- Signature details -------------\n" );
    fprintf( stdout, "%s",
        rtl::OUStringToOString(
            getSignatureInformations(aSignatureHelper.GetSignatureInformations(), aSignatureHelper.GetSecurityEnvironment()),
            RTL_TEXTENCODING_UTF8).getStr());

    return 0;
}

