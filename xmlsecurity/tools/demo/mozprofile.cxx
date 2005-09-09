/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mozprofile.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:37:43 $
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

#ifndef _COM_SUN_STAR_MOZILLA_XMOZILLABOOTSTRAP_HPP_
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#endif

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

