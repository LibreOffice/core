/*************************************************************************
 *
 *  $RCSfile: signdemo.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-16 05:18:39 $
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

#include <tools/date.hxx>
#include <tools/time.hxx>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssi = com::sun::star::io;

int SAL_CALL main( int argc, char **argv )
{
    if( argc != 6 )
    {
        fprintf( stderr, "Usage: %s <rdb file> <signature file> <xml stream file> <binary stream file> <cryptoken>\n" , argv[0] ) ;
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

    /*
     * creates a signature helper
     */
    XMLSignatureHelper aSignatureHelper( xManager );

    /*
     * creates a security context.
     */
    bool bInit = aSignatureHelper.Init( rtl::OUString::createFromAscii(argv[5]) );
    if ( !bInit )
    {
        fprintf( stderr, "Error initializing security context!\n" );
        return -1;
    }

    aSignatureHelper.StartMission();

    /*
     * select a private key certificate
     */
    cssu::Reference< cssxc::XSecurityEnvironment > xSecurityEnvironment = aSignatureHelper.GetSecurityEnvironment();
    cssu::Sequence< cssu::Reference< ::com::sun::star::security::XCertificate > > xPersonalCerts
        = xSecurityEnvironment->getPersonalCertificates() ;
    int length = xPersonalCerts.getLength();
    int i;

    fprintf( stdout, "\nSelect a private key certificate for the new signature\n" ) ;
    fprintf( stdout, "================================================================================\n" ) ;
    for( i = 0; i < length; i ++ )
    {
        fprintf( stdout, "%d:issuer=[%s] subject=[%s]\n",
            i+1,
            rtl::OUStringToOString( xPersonalCerts[i]->getIssuerName(), RTL_TEXTENCODING_ASCII_US ).getStr(),
            rtl::OUStringToOString( xPersonalCerts[i]->getSubjectName(), RTL_TEXTENCODING_ASCII_US ).getStr());
    }

    fprintf( stdout, "================================================================================\n" ) ;
    bool bInvalid = false;
    int sel = 0;

    do
    {
        if (bInvalid)
        {
            fprintf( stdout, "Invalid value! " );
        }

        fprintf( stdout, "Select <1-%d>:", length ) ;
        fscanf( stdin, "%d", &sel ) ;
        bInvalid = true;
    }while(sel<1 || sel>length);

    sel--;

    /*
     * creates a new signature id
     */
    sal_Int32 nSecurityId = aSignatureHelper.GetNewSecurityId();

    /*
     * configures the X509 certificate
     */
    aSignatureHelper.SetX509Certificate(
        nSecurityId,
        xPersonalCerts[sel]->getIssuerName(),
        bigIntegerToNumericString( xPersonalCerts[sel]->getSerialNumber()));

    /*
     * configures date/time
     */
    aSignatureHelper.SetDateTime( nSecurityId, Date(), Time());

    /*
     * signs the xml stream
     */
    rtl::OUString aXMLFileName = rtl::OUString::createFromAscii(argv[3]);
    aSignatureHelper.AddForSigning( nSecurityId, aXMLFileName, aXMLFileName, sal_False );

    /*
     * signs the binary stream
     */
    rtl::OUString aBINFileName = rtl::OUString::createFromAscii(argv[4]);
    aSignatureHelper.AddForSigning( nSecurityId, aBINFileName, aBINFileName, sal_True );

    /*
     * creates the output stream
     */
    rtl::OUString aSIGFileName = rtl::OUString::createFromAscii(argv[2]);
    SvFileStream* pStream = new SvFileStream( aSIGFileName, STREAM_WRITE );
    SvLockBytesRef xLockBytes = new SvLockBytes( pStream, TRUE );
    cssu::Reference< cssi::XOutputStream > xOutputStream = new utl::OOutputStreamHelper( xLockBytes );

    /*
     * creates signature
     */
    bool bDone = aSignatureHelper.CreateAndWriteSignature( xOutputStream );

    if ( !bDone )
    {
        fprintf( stderr, "Error creating Signature!\n" );
    }
    else
    {
        fprintf( stdout, "Signature successfully created!\n" );
    }

    aSignatureHelper.EndMission();

    fprintf( stdout, "------------- Signature details -------------\n" );
    fprintf( stdout, "%s",
        rtl::OUStringToOString(
            getSignatureInformations(aSignatureHelper.GetSignatureInformations(), aSignatureHelper.GetSecurityEnvironment()),
            RTL_TEXTENCODING_UTF8).getStr());

    return 0;
}

