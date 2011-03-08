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
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>

using namespace ::com::sun::star;

int SAL_CALL main( int argc, char **argv )
{
    fprintf( stdout, "\nTesting Mozilla Profile Detection...\n\nOpenOffice.org will use the first detected profile.\nResults might be different when started in OOo program folder!\n" ) ;

    uno::Reference< lang::XMultiServiceFactory > xMSF = CreateDemoServiceFactory();
    if ( !xMSF.is() )
    {
        fprintf( stdout, "\n\nERROR: Can't create Service Factory\n" );
        exit (-1);
    }

    uno::Reference<mozilla::XMozillaBootstrap> xMozillaBootstrap( xMSF->createInstance(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.MozillaBootstrap" ) ) ), uno::UNO_QUERY );
    if ( !xMozillaBootstrap.is() )
    {
        fprintf( stdout, "\n\nERROR: Can't create Mozilla Bootstrap Service\n" );
        exit (-1);
    }

    int nProducts = 4;
    mozilla::MozillaProductType productTypes[4] = { mozilla::MozillaProductType_Thunderbird, mozilla::MozillaProductType_Mozilla, mozilla::MozillaProductType_Firefox, mozilla::MozillaProductType_Default };
    for ( int i = 0; i < nProducts; i++)
    {
        if ( i == 0 )
            fprintf( stdout, "\nThunderbird: " );
        else if ( i == 1 )
            fprintf( stdout, "\nMozilla:     " );
        else if ( i == 2 )
            fprintf( stdout, "\nFireFox:     " );
        else
            fprintf( stdout, "\nDefault:     " );

        ::rtl::OUString profile = xMozillaBootstrap->getDefaultProfile(productTypes[i]);
        if ( profile.getLength() )
        {
            ::rtl::OUString profilepath = xMozillaBootstrap->getProfilePath(productTypes[i],profile);
            fprintf( stdout, "Name=%s, Path=%s", rtl::OUStringToOString( profile , RTL_TEXTENCODING_ASCII_US ).getStr(), rtl::OUStringToOString( profilepath , RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
        else
        {
            fprintf( stdout, "NOT FOUND" );
        }
    }

    /*
     * creates a signature helper
     */
    XMLSignatureHelper aSignatureHelper( xMSF );

    /*
     * creates a security context.
     */
    rtl::OUString aCryptoToken;
    bool bInit = aSignatureHelper.Init( aCryptoToken );
    if ( !bInit )
    {
        fprintf( stdout, "\n\nERROR: Unable to initialize security environment.\n\n" );
    }
    else
    {
        fprintf( stdout, "\n\nSecurity environment can be initialized successfully.\n\n" );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
