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

