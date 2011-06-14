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
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
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
    CERTCertDBHandle*   certHandle = NULL ;
    PK11SlotInfo*       slot = NULL ;
    xmlDocPtr           doc = NULL ;
    xmlNodePtr          tplNode ;
    xmlNodePtr          tarNode ;
    xmlAttrPtr          idAttr ;
    xmlChar*            idValue ;
    xmlAttrPtr          uriAttr ;
    xmlChar*            uriValue ;
    OUString*           uri = NULL ;
    Reference< XUriBinding >    xUriBinding ;

    if( argc != 4 ) {
        fprintf( stderr, "Usage: %s < CertDir > <file_url> <rdb file>\n" , argv[0] ) ;
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
        return 1 ;
    }

    certHandle = CERT_GetDefaultCertDB() ;
    slot = PK11_GetInternalKeySlot() ;

    //Load XML document
    doc = xmlParseFile( argv[2] ) ;
    if( doc == NULL || xmlDocGetRootElement( doc ) == NULL ) {
        fprintf( stderr , "### Cannot load template xml document!\n" ) ;
        goto done ;
    }

    //Find the signature template
    tplNode = xmlSecFindNode( xmlDocGetRootElement( doc ), xmlSecNodeSignature, xmlSecDSigNs ) ;
    if( tplNode == NULL ) {
        fprintf( stderr , "### Cannot find the signature template!\n" ) ;
        goto done ;
    }

    //Find the element with ID attribute
    tarNode = xmlSecFindNode( xmlDocGetRootElement( doc ), ( xmlChar* )"document", ( xmlChar* )"http://openoffice.org/2000/office" ) ;
    if( tarNode == NULL ) {
        tarNode = xmlSecFindNode( xmlDocGetRootElement( doc ), ( xmlChar* )"document", NULL ) ;
    }

    //Find the "id" attrbute in the element
    if( tarNode != NULL ) {
        if( ( idAttr = xmlHasProp( tarNode, ( xmlChar* )"id" ) ) != NULL ) {
            //NULL
        } else if( ( idAttr = xmlHasProp( tarNode, ( xmlChar* )"Id" ) ) != NULL ) {
            //NULL
        } else {
            idAttr = NULL ;
        }
    }

    //Add ID to DOM
    if( idAttr != NULL ) {
        idValue = xmlNodeListGetString( tarNode->doc, idAttr->children, 1 ) ;
        if( idValue == NULL ) {
            fprintf( stderr , "### the ID value is NULL!\n" ) ;
            goto done ;
        }

        if( xmlAddID( NULL, doc, idValue, idAttr ) == NULL ) {
            fprintf( stderr , "### Can not add the ID value!\n" ) ;
            goto done ;
        }
    }

    //Reference handler
    //Find the signature reference
    tarNode = xmlSecFindNode( tplNode, xmlSecNodeReference, xmlSecDSigNs ) ;
    if( tarNode == NULL ) {
        fprintf( stderr , "### Cannot find the signature reference!\n" ) ;
        goto done ;
    }

    //Find the "URI" attrbute in the reference
    uriAttr = xmlHasProp( tarNode, ( xmlChar* )"URI" ) ;
    if( tarNode == NULL ) {
        fprintf( stderr , "### Cannot find URI of the reference!\n" ) ;
        goto done ;
    }

    //Get the "URI" attrbute value
    uriValue = xmlNodeListGetString( tarNode->doc, uriAttr->children, 1 ) ;
    if( uriValue == NULL ) {
        fprintf( stderr , "### the URI value is NULL!\n" ) ;
        goto done ;
    }

    if( strchr( ( const char* )uriValue, '/' ) != NULL && strchr( ( const char* )uriValue, '#' ) == NULL ) {
        fprintf( stdout , "### Find a stream URI [%s]\n", uriValue ) ;
    //  uri = new ::rtl::OUString( ( const sal_Unicode* )uriValue ) ;
        uri = new ::rtl::OUString( ( const sal_Char* )uriValue, xmlStrlen( uriValue ), RTL_TEXTENCODING_ASCII_US ) ;
    }

    if( uri != NULL ) {
        fprintf( stdout , "### Find the URI [%s]\n", OUStringToOString( *uri , RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
        Reference< XInputStream > xStream = createStreamFromFile( *uri ) ;
        if( !xStream.is() ) {
            fprintf( stderr , "### Can not get the URI stream!\n" ) ;
            goto done ;
        }

        xUriBinding = new OUriBinding( *uri, xStream ) ;
    }


    try {
        Reference< XMultiComponentFactory > xManager = NULL ;
        Reference< XComponentContext > xContext = NULL ;

        xManager = serviceManager( xContext , OUString(RTL_CONSTASCII_USTRINGPARAM("local")),  OUString::createFromAscii( argv[3] ) ) ;

        //Create signature template
        Reference< XInterface > element =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.bridge.xmlsec.XMLElementWrapper_XmlSecImpl")) , xContext ) ;
        OSL_ENSURE( element.is() ,
            "Verifier - "
            "Cannot get service instance of \"xsec.XMLElementWrapper\"" ) ;

        Reference< XXMLElementWrapper > xElement( element , UNO_QUERY ) ;
        OSL_ENSURE( xElement.is() ,
            "Verifier - "
            "Cannot get interface of \"XXMLElementWrapper\" from service \"xsec.XMLElementWrapper\"" ) ;

        Reference< XUnoTunnel > xEleTunnel( xElement , UNO_QUERY ) ;
        OSL_ENSURE( xEleTunnel.is() ,
            "Verifier - "
            "Cannot get interface of \"XUnoTunnel\" from service \"xsec.XMLElement\"" ) ;

        XMLElementWrapper_XmlSecImpl* pElement = ( XMLElementWrapper_XmlSecImpl* )xEleTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() ) ;
        OSL_ENSURE( pElement != NULL ,
            "Verifier - "
            "Cannot get implementation of \"xsec.XMLElementWrapper\"" ) ;

        //Set signature template
        pElement->setNativeElement( tplNode ) ;

        //Build XML Signature template
        Reference< XInterface > signtpl =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.crypto.XMLSignatureTemplate")), xContext ) ;
        OSL_ENSURE( signtpl.is() ,
            "Verifier - "
            "Cannot get service instance of \"xsec.XMLSignatureTemplate\"" ) ;

        Reference< XXMLSignatureTemplate > xTemplate( signtpl , UNO_QUERY ) ;
        OSL_ENSURE( xTemplate.is() ,
            "Verifier - "
            "Cannot get interface of \"XXMLSignatureTemplate\" from service \"xsec.XMLSignatureTemplate\"" ) ;

        //Import the signature template
        xTemplate->setTemplate( xElement ) ;

        //Import the URI/Stream binding
        if( xUriBinding.is() )
            xTemplate->setBinding( xUriBinding ) ;

        //Create security environment
        //Build Security Environment
        Reference< XInterface > xsecenv =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.bridge.xmlsec.SecurityEnvironment_NssImpl")), xContext ) ;
        OSL_ENSURE( xsecenv.is() ,
            "Verifier - "
            "Cannot get service instance of \"xsec.SecurityEnvironment\"" ) ;

        Reference< XSecurityEnvironment > xSecEnv( xsecenv , UNO_QUERY ) ;
        OSL_ENSURE( xSecEnv.is() ,
            "Verifier - "
            "Cannot get interface of \"XSecurityEnvironment\" from service \"xsec.SecurityEnvironment\"" ) ;

        //Setup key slot and certDb
        Reference< XUnoTunnel > xEnvTunnel( xsecenv , UNO_QUERY ) ;
        OSL_ENSURE( xElement.is() ,
            "Verifier - "
            "Cannot get interface of \"XUnoTunnel\" from service \"xsec.SecurityEnvironment\"" ) ;

        SecurityEnvironment_NssImpl* pSecEnv = ( SecurityEnvironment_NssImpl* )xEnvTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() ) ;
        OSL_ENSURE( pSecEnv != NULL ,
            "Verifier - "
            "Cannot get implementation of \"xsec.SecurityEnvironment\"" ) ;

        pSecEnv->setCryptoSlot( slot ) ;
        pSecEnv->setCertDb( certHandle ) ;

        //Build XML Security Context
        Reference< XInterface > xmlsecctx =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.bridge.xmlsec.XMLSecurityContext_NssImpl")), xContext ) ;
        OSL_ENSURE( xsecenv.is() ,
            "Verifier - "
            "Cannot get service instance of \"xsec.XMLSecurityContext\"" ) ;

        Reference< XXMLSecurityContext > xSecCtx( xmlsecctx , UNO_QUERY ) ;
        OSL_ENSURE( xSecCtx.is() ,
            "Verifier - "
            "Cannot get interface of \"XXMLSecurityContext\" from service \"xsec.XMLSecurityContext\"" ) ;

        xSecCtx->setSecurityEnvironment( xSecEnv ) ;

        //Generate XML signature
        Reference< XInterface > xmlsigner =
            xManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.bridge.xmlsec.XMLSignature_NssImpl")), xContext ) ;
        OSL_ENSURE( xmlsigner.is() ,
            "Verifier - "
            "Cannot get service instance of \"xsec.XMLSignature\"" ) ;

        Reference< XXMLSignature > xSigner( xmlsigner , UNO_QUERY ) ;
        OSL_ENSURE( xSigner.is() ,
            "Verifier - "
            "Cannot get interface of \"XXMLSignature\" from service \"xsec.XMLSignature\"" ) ;


        //perform validation
        sal_Bool valid = xSigner->validate( xTemplate , xSecCtx ) ;
        if( !valid ) {
            printf( "Signature is INVALID!\n" ) ;
        } else {
            printf( "Signature is VALID!\n" ) ;
        }
    } catch( Exception& e ) {
        fprintf( stderr , "Error Message: %s\n" , OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
        goto done ;
    }

done :
    if( doc != NULL )
        xmlFreeDoc( doc ) ;

    if( slot != NULL )
        PK11_FreeSlot( slot ) ;

    PK11_LogoutAll() ;
    NSS_Shutdown() ;

    /* Shutdown libxslt/libxml */
    #ifndef XMLSEC_NO_XSLT
    xsltCleanupGlobals();
    #endif /* XMLSEC_NO_XSLT */
    xmlCleanupParser();

    return 0 ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
