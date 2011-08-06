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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
