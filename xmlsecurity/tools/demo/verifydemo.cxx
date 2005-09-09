/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: verifydemo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:39:29 $
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
#include <cppuhelper/servicefactory.hxx>

#include <xmlsecurity/xmlsignaturehelper.hxx>

using namespace ::com::sun::star;

long startVerifyHandler( void *, void * )
{
    return QueryVerifySignature();
}

int SAL_CALL main( int argc, char **argv )
{
    if( argc < 2 )
    {
        fprintf( stderr, "Usage: %s <signature file> [<cryptoken>]\n" , argv[0] ) ;
        return -1 ;
    }

    rtl::OUString aSIGFileName = rtl::OUString::createFromAscii(argv[1]);
    rtl::OUString aCryptoToken;
    if ( argc >= 3 )
        aCryptoToken = rtl::OUString::createFromAscii(argv[2]);

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
        fprintf( stderr, "Error initializing security context!" );
        return -1;
    }

    /*
     * configures the start-verify handler
     */
    aSignatureHelper.SetStartVerifySignatureHdl( Link( NULL, startVerifyHandler ) );

    aSignatureHelper.StartMission();

    /*
     * verifies the signature
     */
    uno::Reference< io::XInputStream > xInputStream = OpenInputStream( aSIGFileName );
    bool bDone = aSignatureHelper.ReadAndVerifySignature( xInputStream );

    /*
     * closes the signature stream
     */
    xInputStream->closeInput();

    if ( !bDone )
    {
        fprintf( stderr, "\nSTATUS: Error verifying Signature!\n" );
    }
    else
    {
        fprintf( stdout, "\nSTATUS: All choosen Signatures veryfied successfully!\n" );
    }

    aSignatureHelper.EndMission();

    QueryPrintSignatureDetails( aSignatureHelper.GetSignatureInformations(), aSignatureHelper.GetSecurityEnvironment() );

    return 0;
}

