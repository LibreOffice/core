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

#include <stdio.h>
#include "helper.hxx"

#include "libxml/tree.h"
#include "libxml/parser.h"
#ifndef XMLSEC_NO_XSLT
#include "libxslt/xslt.h"
#endif

#include "securityenvironment_nssimpl.hxx"

#include <xmlsecurity/biginteger.hxx>


#include "nspr.h"
#include "prtypes.h"

#include "pk11func.h"
#include "cert.h"
#include "cryptohi.h"
#include "certdb.h"
#include "nss.h"

#include "xmlsec/strings.h"
#include "xmlsec/xmltree.h"

#include <rtl/ustring.hxx>

using namespace ::rtl ;
using namespace ::cppu ;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::io ;
using namespace ::com::sun::star::ucb ;
using namespace ::com::sun::star::beans ;
using namespace ::com::sun::star::document ;
using namespace ::com::sun::star::lang ;
using namespace ::com::sun::star::security ;
using namespace ::com::sun::star::xml::wrapper ;
using namespace ::com::sun::star::xml::crypto ;

int SAL_CALL main( int argc, char **argv )
{
    CERTCertDBHandle*   certHandle ;
    PK11SlotInfo*       slot ;

    if( argc != 3 ) {
        fprintf( stderr, "Usage: %s < CertDir > <rdb file>\n\n" , argv[0] ) ;
        return 1 ;
    }

    for( ; getchar() != 'q' ; ) {
        slot = NULL ;

    //Initialize NSPR and NSS
    PR_Init( PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1 ) ;
    PK11_SetPasswordFunc( PriPK11PasswordFunc ) ;
    if( NSS_Init( argv[1] ) != SECSuccess ) {
        fprintf( stderr , "### cannot intialize NSS!\n" ) ;
        goto done ;
    }

    certHandle = CERT_GetDefaultCertDB() ;
    slot = PK11_GetInternalKeySlot() ;

    if( PK11_NeedLogin( slot ) ) {
        SECStatus nRet = PK11_Authenticate( slot, PR_TRUE, NULL );
        if( nRet != SECSuccess ) {
            fprintf( stderr , "### cannot authehticate the crypto token!\n" ) ;
            goto done ;
        }
    }


    try {
        Reference< XMultiComponentFactory > xManager = NULL ;
        Reference< XComponentContext > xContext = NULL ;

        xManager = serviceManager( xContext , OUString::createFromAscii( "local" ), OUString::createFromAscii( argv[2] ) ) ;
        OSL_ENSURE( xManager.is() ,
            "ServicesManager - "
            "Cannot get service manager" ) ;

        //Create security environment
        //Build Security Environment
        Reference< XInterface > xsecenv =
            xManager->createInstanceWithContext( OUString::createFromAscii("com.sun.star.xml.security.bridge.xmlsec.SecurityEnvironment_NssImpl"), xContext ) ;
        OSL_ENSURE( xsecenv.is() ,
            "Signer - "
            "Cannot get service instance of \"xsec.SecurityEnvironment\"" ) ;

        Reference< XSecurityEnvironment > xSecEnv( xsecenv , UNO_QUERY ) ;
        OSL_ENSURE( xSecEnv.is() ,
            "Signer - "
            "Cannot get interface of \"XSecurityEnvironment\" from service \"xsec.SecurityEnvironment\"" ) ;

        //Setup key slot and certDb
        Reference< XUnoTunnel > xEnvTunnel( xsecenv , UNO_QUERY ) ;
        OSL_ENSURE( xEnvTunnel.is() ,
            "Signer - "
            "Cannot get interface of \"XUnoTunnel\" from service \"xsec.SecurityEnvironment\"" ) ;

        SecurityEnvironment_NssImpl* pSecEnv = ( SecurityEnvironment_NssImpl* )xEnvTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() ) ;
        OSL_ENSURE( pSecEnv != NULL ,
            "Signer - "
            "Cannot get implementation of \"xsec.SecurityEnvironment\"" ) ;

        pSecEnv->setCryptoSlot( slot ) ;
        pSecEnv->setCertDb( certHandle ) ;

        //Get personal certificate
        Sequence < Reference< XCertificate > > xPersonalCerts = pSecEnv->getPersonalCertificates() ;
        Sequence < Reference< XCertificate > > xCertPath ;
        for( int i = 0; i < xPersonalCerts.getLength(); i ++ ) {
            //Print the certificate infomation.
            fprintf( stdout, "\nPersonal Certificate Info\n" ) ;
            fprintf( stdout, "\tCertificate Issuer[%s]\n", OUStringToOString( xPersonalCerts[i]->getIssuerName(), RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
            fprintf( stdout, "\tCertificate Serial Number[%s]\n", OUStringToOString( bigIntegerToNumericString( xPersonalCerts[i]->getSerialNumber() ), RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
            fprintf( stdout, "\tCertificate Subject[%s]\n", OUStringToOString( xPersonalCerts[i]->getSubjectName(), RTL_TEXTENCODING_ASCII_US ).getStr() ) ;

            //build the certificate path
            xCertPath = pSecEnv->buildCertificatePath( xPersonalCerts[i] ) ;
            //Print the certificate path.
            fprintf( stdout, "\tCertificate Path\n" ) ;
            for( int j = 0; j < xCertPath.getLength(); j ++ ) {
                fprintf( stdout, "\t\tCertificate Authority Subject[%s]\n", OUStringToOString( xCertPath[j]->getSubjectName(), RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
            }

            //Get the certificate
            Sequence < sal_Int8 > serial = xPersonalCerts[i]->getSerialNumber() ;
            Reference< XCertificate > xcert = pSecEnv->getCertificate( xPersonalCerts[i]->getIssuerName(), xPersonalCerts[i]->getSerialNumber() ) ;
            if( !xcert.is() ) {
                fprintf( stdout, "The personal certificate is not in the certificate database\n" ) ;
            }

            //Get the certificate characters
            sal_Int32 chars = pSecEnv->getCertificateCharacters( xPersonalCerts[i] ) ;
            fprintf( stdout, "The certificate characters are %d\n", chars ) ;

            //Get the certificate status
            sal_Int32 validity = pSecEnv->verifyCertificate( xPersonalCerts[i] ) ;
            fprintf( stdout, "The certificate validities are %d\n", validity ) ;
        }
    } catch( Exception& e ) {
        fprintf( stderr , "Error Message: %s\n" , OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
        goto done ;
    }

done:
    if( slot != NULL )
        PK11_FreeSlot( slot ) ;

    PK11_LogoutAll() ;
    NSS_Shutdown() ;

    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
