/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>
#include <rtl/uuid.h>
#include "xmlencryption_mscryptimpl.hxx"

#include "xmldocumentwrapper_xmlsecimpl.hxx"

#include "xmlelementwrapper_xmlsecimpl.hxx"

#include "securityenvironment_mscryptimpl.hxx"
#include "errorcallback.hxx"

#include "xmlsecurity/xmlsec-wrapper.h"

#ifdef UNX
#define stricmp strcasecmp
#endif

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
using ::com::sun::star::xml::wrapper::XXMLDocumentWrapper ;
using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLEncryption ;
using ::com::sun::star::xml::crypto::XXMLEncryptionTemplate ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;
using ::com::sun::star::xml::crypto::XMLEncryptionException ;

XMLEncryption_MSCryptImpl :: XMLEncryption_MSCryptImpl( const Reference< XMultiServiceFactory >& aFactory ) : m_xServiceManager( aFactory ) {
}

XMLEncryption_MSCryptImpl :: ~XMLEncryption_MSCryptImpl() {
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate >
SAL_CALL XMLEncryption_MSCryptImpl :: encrypt(
    const Reference< XXMLEncryptionTemplate >& aTemplate ,
    const Reference< XSecurityEnvironment >& aEnvironment
) throw( com::sun::star::xml::crypto::XMLEncryptionException,
         com::sun::star::uno::SecurityException )
{
    xmlSecKeysMngrPtr pMngr = NULL ;
    xmlSecEncCtxPtr pEncCtx = NULL ;
    xmlNodePtr pEncryptedData = NULL ;
    xmlNodePtr pContent = NULL ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aEnvironment.is() )
        throw RuntimeException() ;

    //Get Keys Manager
    Reference< XUnoTunnel > xSecTunnel( aEnvironment , UNO_QUERY ) ;
    if( !xSecTunnel.is() ) {
         throw RuntimeException() ;
    }

    SecurityEnvironment_MSCryptImpl* pSecEnv = ( SecurityEnvironment_MSCryptImpl* )xSecTunnel->getSomething( SecurityEnvironment_MSCryptImpl::getUnoTunnelId() ) ;
    if( pSecEnv == NULL )
        throw RuntimeException() ;

    //Get the encryption template
    Reference< XXMLElementWrapper > xTemplate = aTemplate->getTemplate() ;
    if( !xTemplate.is() ) {
        throw RuntimeException() ;
    }

    Reference< XUnoTunnel > xTplTunnel( xTemplate , UNO_QUERY ) ;
    if( !xTplTunnel.is() ) {
        throw RuntimeException() ;
    }

    XMLElementWrapper_XmlSecImpl* pTemplate = ( XMLElementWrapper_XmlSecImpl* )xTplTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() ) ;
    if( pTemplate == NULL ) {
        throw RuntimeException() ;
    }

    pEncryptedData = pTemplate->getNativeElement() ;

    //Find the element to be encrypted.
    //This element is wrapped in the CipherValue sub-element.
    xmlNodePtr pCipherData = pEncryptedData->children;
    while (pCipherData != NULL && stricmp((const char *)(pCipherData->name), "CipherData"))
    {
        pCipherData = pCipherData->next;
    }

    if( pCipherData == NULL ) {
        throw XMLEncryptionException() ;
    }

    xmlNodePtr pCipherValue = pCipherData->children;
    while (pCipherValue != NULL && stricmp((const char *)(pCipherValue->name), "CipherValue"))
    {
        pCipherValue = pCipherValue->next;
    }

    if( pCipherValue == NULL ) {
        throw XMLEncryptionException() ;
    }

    pContent = pCipherValue->children;

    if( pContent == NULL ) {
        throw XMLEncryptionException() ;
    }

    xmlUnlinkNode(pContent);
    xmlAddNextSibling(pEncryptedData, pContent);

    //remember the position of the element to be signed
    sal_Bool isParentRef = sal_True;
    xmlNodePtr pParent = pEncryptedData->parent;
    xmlNodePtr referenceNode;

    if (pEncryptedData == pParent->children)
    {
        referenceNode = pParent;
    }
    else
    {
        referenceNode = pEncryptedData->prev;
        isParentRef = sal_False;
    }

     setErrorRecorder( );

    pMngr = pSecEnv->createKeysManager() ; //i39448
    if( !pMngr ) {
        throw RuntimeException() ;
    }

    //Create Encryption context
    pEncCtx = xmlSecEncCtxCreate( pMngr ) ;
    if( pEncCtx == NULL )
    {
        pSecEnv->destroyKeysManager( pMngr ) ; //i39448
        //throw XMLEncryptionException() ;
        clearErrorRecorder();
        return aTemplate;
    }

    //Encrypt the template
    if( xmlSecEncCtxXmlEncrypt( pEncCtx , pEncryptedData , pContent ) < 0 ) {
        aTemplate->setStatus(::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN);
        xmlSecEncCtxDestroy( pEncCtx ) ;
        pSecEnv->destroyKeysManager( pMngr ) ; //i39448
        clearErrorRecorder();
        return aTemplate;
    }
    aTemplate->setStatus(::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED);
    xmlSecEncCtxDestroy( pEncCtx ) ;
    pSecEnv->destroyKeysManager( pMngr ) ; //i39448

    //get the new EncryptedData element
    if (isParentRef)
    {
        pTemplate->setNativeElement(referenceNode->children) ;
    }
    else
    {
        pTemplate->setNativeElement(referenceNode->next);
    }

    clearErrorRecorder();
    return aTemplate ;
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate > SAL_CALL
XMLEncryption_MSCryptImpl :: decrypt(
    const Reference< XXMLEncryptionTemplate >& aTemplate ,
    const Reference< XXMLSecurityContext >& aSecurityCtx
) throw( com::sun::star::xml::crypto::XMLEncryptionException ,
         com::sun::star::uno::SecurityException) {
    xmlSecKeysMngrPtr pMngr = NULL ;
    xmlSecEncCtxPtr pEncCtx = NULL ;
    xmlNodePtr pEncryptedData = NULL ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aSecurityCtx.is() )
        throw RuntimeException() ;

    //Get Keys Manager
    Reference< XSecurityEnvironment > xSecEnv
        = aSecurityCtx->getSecurityEnvironmentByIndex(
            aSecurityCtx->getDefaultSecurityEnvironmentIndex());
    Reference< XUnoTunnel > xSecTunnel( xSecEnv , UNO_QUERY ) ;
    if( !xSecTunnel.is() ) {
         throw RuntimeException() ;
    }

    SecurityEnvironment_MSCryptImpl* pSecEnv = ( SecurityEnvironment_MSCryptImpl* )xSecTunnel->getSomething( SecurityEnvironment_MSCryptImpl::getUnoTunnelId() ) ;
    if( pSecEnv == NULL )
        throw RuntimeException() ;

    //Get the encryption template
    Reference< XXMLElementWrapper > xTemplate = aTemplate->getTemplate() ;
    if( !xTemplate.is() ) {
        throw RuntimeException() ;
    }

    Reference< XUnoTunnel > xTplTunnel( xTemplate , UNO_QUERY ) ;
    if( !xTplTunnel.is() ) {
        throw RuntimeException() ;
    }

    XMLElementWrapper_XmlSecImpl* pTemplate = ( XMLElementWrapper_XmlSecImpl* )xTplTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() ) ;
    if( pTemplate == NULL ) {
        throw RuntimeException() ;
    }

    pEncryptedData = pTemplate->getNativeElement() ;

    //remember the position of the element to be signed
    sal_Bool isParentRef = sal_True;
    xmlNodePtr pParent = pEncryptedData->parent;
    xmlNodePtr referenceNode;

    if (pEncryptedData == pParent->children)
    {
        referenceNode = pParent;
    }
    else
    {
        referenceNode = pEncryptedData->prev;
        isParentRef = sal_False;
    }

     setErrorRecorder( );

    pMngr = pSecEnv->createKeysManager() ; //i39448
    if( !pMngr ) {
        throw RuntimeException() ;
    }

    //Create Encryption context
    pEncCtx = xmlSecEncCtxCreate( pMngr ) ;
    if( pEncCtx == NULL )
    {
        pSecEnv->destroyKeysManager( pMngr ) ; //i39448
        //throw XMLEncryptionException() ;
        clearErrorRecorder();
        return aTemplate;
    }

    //Decrypt the template
    if( xmlSecEncCtxDecrypt( pEncCtx , pEncryptedData ) < 0 || pEncCtx->result == NULL ) {
        aTemplate->setStatus(::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN);
        xmlSecEncCtxDestroy( pEncCtx ) ;
        pSecEnv->destroyKeysManager( pMngr ) ; //i39448

        //throw XMLEncryptionException() ;
        clearErrorRecorder();
        return aTemplate;
    }
    aTemplate->setStatus(::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED);
#if 0 // This code block has been commented out since 2004,
      // but let's keep it here in case it contains some useful hints
      // for future work.
    if( pEncCtx->resultReplaced != 0 ) {
        pContent = pEncryptedData ;

        Reference< XUnoTunnel > xTunnel( ret , UNO_QUERY ) ;
        if( !xTunnel.is() ) {
            xmlSecEncCtxDestroy( pEncCtx ) ;
            throw RuntimeException() ;
        }
        XMLElementWrapper_XmlSecImpl* pNode = ( XMLElementWrapper_XmlSecImpl* )xTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() ) ;
        if( pNode == NULL ) {
            xmlSecEncCtxDestroy( pEncCtx ) ;
            throw RuntimeException() ;
        }

        pNode->setNativeElement( pContent ) ;
    } else {
        xmlSecEncCtxDestroy( pEncCtx ) ;
        throw RuntimeException() ;
    }
#endif // 0

    //Destroy the encryption context
    xmlSecEncCtxDestroy( pEncCtx ) ;
    pSecEnv->destroyKeysManager( pMngr ) ; //i39448

    //get the decrypted element
    XMLElementWrapper_XmlSecImpl * ret = new XMLElementWrapper_XmlSecImpl(isParentRef?
        (referenceNode->children):(referenceNode->next));

    //return ret;
    aTemplate->setTemplate(ret);

    clearErrorRecorder();
    return aTemplate;
}

/* XServiceInfo */
OUString SAL_CALL XMLEncryption_MSCryptImpl :: getImplementationName() throw( RuntimeException ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLEncryption_MSCryptImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLEncryption_MSCryptImpl :: getSupportedServiceNames() throw( RuntimeException ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLEncryption_MSCryptImpl :: impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence< OUString > seqServiceNames( 1 ) ;
    seqServiceNames.getArray()[0] = OUString("com.sun.star.xml.crypto.XMLEncryption") ;
    return seqServiceNames ;
}

OUString XMLEncryption_MSCryptImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString("com.sun.star.xml.security.bridge.xmlsec.XMLEncryption_MSCryptImpl") ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLEncryption_MSCryptImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
    return Reference< XInterface >( *new XMLEncryption_MSCryptImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > XMLEncryption_MSCryptImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
