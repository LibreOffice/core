/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

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

