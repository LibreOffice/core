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
#include "xmlelementwrapper_xmlsecimpl.hxx"

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
#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/xml/crypto/XXMLEncryption.hpp>
#include <com/sun/star/xml/crypto/XXMLEncryptionTemplate.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>

using namespace ::rtl ;
using namespace ::cppu ;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::io ;
using namespace ::com::sun::star::ucb ;
using namespace ::com::sun::star::beans ;
using namespace ::com::sun::star::document ;
using namespace ::com::sun::star::lang ;
using namespace ::com::sun::star::registry ;
using namespace ::com::sun::star::xml::wrapper ;
using namespace ::com::sun::star::xml::crypto ;

int SAL_CALL main( int argc, char **argv )
{
    CERTCertDBHandle*   certHandle ;
    PK11SlotInfo*       slot = NULL ;
    PK11SymKey*         symKey = NULL ;
    xmlDocPtr           doc = NULL ;
    xmlNodePtr          tplNode ;
    xmlNodePtr          tarNode ;
    FILE*               dstFile = NULL ;

    if( argc != 7 ) {
        fprintf( stderr, "Usage: %s < CertDir > <file_url of template> <file_url of result> <target element name> <target element namespace> <rdb file>\n\n" , argv[0] ) ;
        return 1 ;
    }

    //Init libxml and libxslt libraries
    xmlInitParser();
    LIBXML_TEST_VERSION
    xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;
    xmlSubstituteEntitiesDefault(1);

    #ifndef XMLSEC_NO_XSLT
    xmlIndentTreeOutput = 1;
    #endif // XMLSEC_NO_XSLT

    //Initialize NSPR and NSS
    PR_Init( PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1 ) ;
    PK11_SetPasswordFunc( PriPK11PasswordFunc ) ;
    if( NSS_Init( argv[1] ) != SECSuccess ) {
        fprintf( stderr , "### cannot intialize NSS!\n" ) ;
        goto done ;
    }

    certHandle = CERT_GetDefaultCertDB() ;
    slot = PK11_GetInternalKeySlot() ;

    symKey = PK11_KeyGen( slot , CKM_DES3_CBC, NULL, 128, NULL ) ;
    if( symKey == NULL ) {
        fprintf( stderr , "### cannot create symmetric key!\n" ) ;
        goto done ;
    }

    //Load XML document
    doc = xmlParseFile( argv[2] ) ;
    if( doc == NULL || xmlDocGetRootElement( doc ) == NULL ) {
        fprintf( stderr , "### Cannot load template xml document!\n" ) ;
        goto done ;
    }

    //Find the encryption template
    tplNode = xmlSecFindNode( xmlDocGetRootElement( doc ), xmlSecNodeEncryptedData, xmlSecEncNs ) ;
    if( tplNode == NULL ) {
        fprintf( stderr , "### Cannot find the encryption template!\n" ) ;
        goto done ;
    }

    //Find the encryption template
    tarNode = xmlSecFindNode( xmlDocGetRootElement( doc ), ( const unsigned char*)argv[4], ( const unsigned char*)argv[5] ) ;
    if( tarNode == NULL ) {
        fprintf( stderr , "### Cannot find the encryption target!\n" ) ;
        goto done ;
    }

    try {
        Reference< XMultiComponentFactory > xManager = NULL ;
        Reference< XComponentContext > xContext = NULL ;

        xManager = serviceManager( xContext , OUString(RTL_CONSTASCII_USTRINGPARAM("local")), OUString::createFromAscii( argv[6] ) ) ;

        //Create encryption template
        Reference< XInterface > tplElement =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.bridge.xmlsec.XMLElementWrapper_XmlSecImpl")) , xContext ) ;
        OSL_ENSURE( tplElement.is() ,
            "Encryptor - "
            "Cannot get service instance of \"xsec.XMLElementWrapper\"" ) ;

        Reference< XXMLElementWrapper > xTplElement( tplElement , UNO_QUERY ) ;
        OSL_ENSURE( xTplElement.is() ,
            "Encryptor - "
            "Cannot get interface of \"XXMLElementWrapper\" from service \"xsec.XMLElementWrapper\"" ) ;

        Reference< XUnoTunnel > xTplEleTunnel( xTplElement , UNO_QUERY ) ;
        OSL_ENSURE( xTplEleTunnel.is() ,
            "Encryptor - "
            "Cannot get interface of \"XUnoTunnel\" from service \"xsec.XMLElementWrapper\"" ) ;

        XMLElementWrapper_XmlSecImpl* pTplElement = ( XMLElementWrapper_XmlSecImpl* )xTplEleTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() ) ;
        OSL_ENSURE( pTplElement != NULL ,
            "Encryptor - "
            "Cannot get implementation of \"xsec.XMLElementWrapper\"" ) ;

        pTplElement->setNativeElement( tplNode ) ;

        //Create encryption target element
        Reference< XInterface > tarElement =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.bridge.xmlsec.XMLElementWrapper_XmlSecImpl")) , xContext ) ;
        OSL_ENSURE( tarElement.is() ,
            "Encryptor - "
            "Cannot get service instance of \"xsec.XMLElementWrapper\"" ) ;

        Reference< XXMLElementWrapper > xTarElement( tarElement , UNO_QUERY ) ;
        OSL_ENSURE( xTarElement.is() ,
            "Encryptor - "
            "Cannot get interface of \"XXMLElementWrapper\" from service \"xsec.XMLElementWrapper\"" ) ;

        Reference< XUnoTunnel > xTarEleTunnel( xTarElement , UNO_QUERY ) ;
        OSL_ENSURE( xTarEleTunnel.is() ,
            "Encryptor - "
            "Cannot get interface of \"XUnoTunnel\" from service \"xsec.XMLElementWrapper\"" ) ;

        XMLElementWrapper_XmlSecImpl* pTarElement = ( XMLElementWrapper_XmlSecImpl* )xTarEleTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() ) ;
        OSL_ENSURE( pTarElement != NULL ,
            "Encryptor - "
            "Cannot get implementation of \"xsec.XMLElementWrapper\"" ) ;

        pTarElement->setNativeElement( tarNode ) ;


        //Build XML Encryption template
        Reference< XInterface > enctpl =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.crypto.XMLEncryptionTemplate")), xContext ) ;
        OSL_ENSURE( enctpl.is() ,
            "Encryptor - "
            "Cannot get service instance of \"xsec.XMLEncryptionTemplate\"" ) ;

        Reference< XXMLEncryptionTemplate > xTemplate( enctpl , UNO_QUERY ) ;
        OSL_ENSURE( xTemplate.is() ,
            "Encryptor - "
            "Cannot get interface of \"XXMLEncryptionTemplate\" from service \"xsec.XMLEncryptionTemplate\"" ) ;

        //Import the encryption template
        xTemplate->setTemplate( xTplElement ) ;
        xTemplate->setTarget( xTarElement ) ;

        //Create security environment
        //Build Security Environment
        Reference< XInterface > xsecenv =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.bridge.xmlsec.SecurityEnvironment_NssImpl")), xContext ) ;
        OSL_ENSURE( xsecenv.is() ,
            "Encryptor - "
            "Cannot get service instance of \"xsec.SecurityEnvironment\"" ) ;

        Reference< XSecurityEnvironment > xSecEnv( xsecenv , UNO_QUERY ) ;
        OSL_ENSURE( xSecEnv.is() ,
            "Encryptor - "
            "Cannot get interface of \"XSecurityEnvironment\" from service \"xsec.SecurityEnvironment\"" ) ;

        //Setup key slot and certDb
        Reference< XUnoTunnel > xEnvTunnel( xsecenv , UNO_QUERY ) ;
        OSL_ENSURE( xEnvTunnel.is() ,
            "Encryptor - "
            "Cannot get interface of \"XUnoTunnel\" from service \"xsec.SecurityEnvironment\"" ) ;

        SecurityEnvironment_NssImpl* pSecEnv = ( SecurityEnvironment_NssImpl* )xEnvTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() ) ;
        OSL_ENSURE( pSecEnv != NULL ,
            "Encryptor - "
            "Cannot get implementation of \"xsec.SecurityEnvironment\"" ) ;

        pSecEnv->setCryptoSlot( slot ) ;
        pSecEnv->setCertDb( certHandle ) ;
        pSecEnv->adoptSymKey( symKey ) ;


        //Build XML Security Context
        Reference< XInterface > xmlsecctx =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.bridge.xmlsec.XMLSecurityContext_NssImpl")), xContext ) ;
        OSL_ENSURE( xmlsecctx.is() ,
            "Encryptor - "
            "Cannot get service instance of \"xsec.XMLSecurityContext\"" ) ;

        Reference< XXMLSecurityContext > xSecCtx( xmlsecctx , UNO_QUERY ) ;
        OSL_ENSURE( xSecCtx.is() ,
            "Encryptor - "
            "Cannot get interface of \"XXMLSecurityContext\" from service \"xsec.XMLSecurityContext\"" ) ;

        xSecCtx->setSecurityEnvironment( xSecEnv ) ;

        //Get encrypter
        Reference< XInterface > xmlencrypter =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.bridge.xmlsec.XMLEncryption_NssImpl")), xContext ) ;
        OSL_ENSURE( xmlencrypter.is() ,
            "Encryptor - "
            "Cannot get service instance of \"xsec.XMLEncryption\"" ) ;

        Reference< XXMLEncryption > xEncrypter( xmlencrypter , UNO_QUERY ) ;
        OSL_ENSURE( xEncrypter.is() ,
            "Encryptor - "
            "Cannot get interface of \"XXMLEncryption\" from service \"xsec.XMLEncryption\"" ) ;

        //perform encryption
        xTemplate = xEncrypter->encrypt( xTemplate , xSecCtx ) ;
        OSL_ENSURE( xTemplate.is() ,
            "Encryptor - "
            "Cannot encrypt the xml document" ) ;
    } catch( Exception& e ) {
        fprintf( stderr , "Error Message: %s\n" , OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
        goto done ;
    }

    dstFile = fopen( argv[3], "w" ) ;
    if( dstFile == NULL ) {
        fprintf( stderr , "### Can not open file %s\n", argv[3] ) ;
        goto done ;
    }

    //Save result
    xmlDocDump( dstFile, doc ) ;

done:
    if( dstFile != NULL )
        fclose( dstFile ) ;

    if( symKey != NULL ) {
        PK11_FreeSymKey( symKey ) ;
    }

    if( slot != NULL )
        PK11_FreeSlot( slot ) ;

    PK11_LogoutAll() ;
    NSS_Shutdown() ;

    /* Shutdown libxslt/libxml */
    #ifndef XMLSEC_NO_XSLT
    xsltCleanupGlobals();
    #endif /* XMLSEC_NO_XSLT */
    xmlCleanupParser();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
