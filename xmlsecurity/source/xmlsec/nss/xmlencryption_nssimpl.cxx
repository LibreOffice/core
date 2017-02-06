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

#include <com/sun/star/xml/crypto/XMLEncryptionException.hpp>
#include <rtl/uuid.h>
#include "xmlencryption_nssimpl.hxx"

#include "xmlsec/xmldocumentwrapper_xmlsecimpl.hxx"

#include "xmlelementwrapper_xmlsecimpl.hxx"

#include "securityenvironment_nssimpl.hxx"
#include "errorcallback.hxx"

#include "xmlsec-wrapper.h"

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

XMLEncryption_NssImpl::XMLEncryption_NssImpl() {
}

XMLEncryption_NssImpl::~XMLEncryption_NssImpl() {
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate >
SAL_CALL XMLEncryption_NssImpl::encrypt(
    const Reference< XXMLEncryptionTemplate >& aTemplate ,
    const Reference< XSecurityEnvironment >& aEnvironment
)
{
    xmlSecKeysMngrPtr pMngr = nullptr ;
    xmlSecEncCtxPtr pEncCtx = nullptr ;
    xmlNodePtr pEncryptedData = nullptr ;
    xmlNodePtr pContent = nullptr ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aEnvironment.is() )
        throw RuntimeException() ;

    //Get Keys Manager
    Reference< XUnoTunnel > xSecTunnel( aEnvironment , UNO_QUERY ) ;
    if( !xSecTunnel.is() ) {
         throw RuntimeException() ;
    }

    SecurityEnvironment_NssImpl* pSecEnv =
        reinterpret_cast<SecurityEnvironment_NssImpl*>(
            sal::static_int_cast<sal_uIntPtr>(xSecTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() ))) ;
    if( pSecEnv == nullptr )
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

    XMLElementWrapper_XmlSecImpl* pTemplate =
        reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xTplTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() )));
    if( pTemplate == nullptr ) {
        throw RuntimeException() ;
    }

    // Get the element to be encrypted
    Reference< XXMLElementWrapper > xTarget = aTemplate->getTarget() ;
    if( !xTarget.is() ) {
        throw XMLEncryptionException() ;
    }

    Reference< XUnoTunnel > xTgtTunnel( xTarget , UNO_QUERY ) ;
    if( !xTgtTunnel.is() ) {
        throw XMLEncryptionException() ;
    }

    XMLElementWrapper_XmlSecImpl* pTarget =
        reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xTgtTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() )));
    if( pTarget == nullptr ) {
        throw RuntimeException() ;
    }

    pContent = pTarget->getNativeElement() ;

    if( pContent == nullptr ) {
        throw XMLEncryptionException() ;
    }

    //remember the position of the element to be signed
    bool isParentRef = true;
    pEncryptedData = pTemplate->getNativeElement();

    xmlNodePtr pParent = pEncryptedData->parent;
    xmlNodePtr referenceNode;

    if (pEncryptedData == pParent->children)
    {
        referenceNode = pParent;
    }
    else
    {
        referenceNode = pEncryptedData->prev;
        isParentRef = false;
    }

     setErrorRecorder( );

    pMngr = pSecEnv->createKeysManager();
    if( !pMngr ) {
        throw RuntimeException() ;
    }

    //Create Encryption context
    pEncCtx = xmlSecEncCtxCreate( pMngr ) ;
    if( pEncCtx == nullptr )
    {
        SecurityEnvironment_NssImpl::destroyKeysManager( pMngr );
        //throw XMLEncryptionException() ;
        clearErrorRecorder();
        return aTemplate;
    }

    //Find the element to be encrypted.

    //Encrypt the template
    if( xmlSecEncCtxXmlEncrypt( pEncCtx , pEncryptedData , pContent ) < 0 )
    {
        xmlSecEncCtxDestroy( pEncCtx ) ;
        SecurityEnvironment_NssImpl::destroyKeysManager( pMngr );

        //throw XMLEncryptionException() ;
        clearErrorRecorder();
        return aTemplate;
    }

    xmlSecEncCtxDestroy( pEncCtx ) ;
    SecurityEnvironment_NssImpl::destroyKeysManager( pMngr );

    //get the new EncryptedData element
    if (isParentRef)
    {
        pTemplate->setNativeElement(referenceNode->children) ;
    }
    else
    {
        pTemplate->setNativeElement(referenceNode->next);
    }

    return aTemplate ;
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate >
SAL_CALL XMLEncryption_NssImpl::decrypt(
    const Reference< XXMLEncryptionTemplate >& aTemplate ,
    const Reference< XXMLSecurityContext >& aSecurityCtx
)
{
    xmlSecKeysMngrPtr pMngr = nullptr ;
    xmlSecEncCtxPtr pEncCtx = nullptr ;
    xmlNodePtr pEncryptedData = nullptr ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aSecurityCtx.is() )
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

    XMLElementWrapper_XmlSecImpl* pTemplate =
        reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xTplTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() )));
    if( pTemplate == nullptr ) {
        throw RuntimeException() ;
    }

    pEncryptedData = pTemplate->getNativeElement() ;

    //remember the position of the element to be signed
    bool isParentRef = true;
    xmlNodePtr pParent = pEncryptedData->parent;
    xmlNodePtr referenceNode;

    if (pEncryptedData == pParent->children)
    {
        referenceNode = pParent;
    }
    else
    {
        referenceNode = pEncryptedData->prev;
        isParentRef = false;
    }

     setErrorRecorder( );

    sal_Int32 nSecurityEnvironment = aSecurityCtx->getSecurityEnvironmentNumber();
    sal_Int32 i;

    for (i=0; i<nSecurityEnvironment; ++i)
    {
        Reference< XSecurityEnvironment > aEnvironment = aSecurityCtx->getSecurityEnvironmentByIndex(i);

        //Get Keys Manager
        Reference< XUnoTunnel > xSecTunnel( aEnvironment , UNO_QUERY ) ;
        if( !aEnvironment.is() ) {
             throw RuntimeException() ;
        }

        SecurityEnvironment_NssImpl* pSecEnv =
            reinterpret_cast<SecurityEnvironment_NssImpl*>(
                sal::static_int_cast<sal_uIntPtr>(
                    xSecTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() )));
        if( pSecEnv == nullptr )
            throw RuntimeException() ;

        pMngr = pSecEnv->createKeysManager();
        if( !pMngr ) {
            throw RuntimeException() ;
        }

        //Create Encryption context
        pEncCtx = xmlSecEncCtxCreate( pMngr ) ;
        if( pEncCtx == nullptr )
        {
            SecurityEnvironment_NssImpl::destroyKeysManager( pMngr );
            //throw XMLEncryptionException() ;
            clearErrorRecorder();
            return aTemplate;
        }

        //Decrypt the template
        if(!( xmlSecEncCtxDecrypt( pEncCtx , pEncryptedData ) < 0 || pEncCtx->result == nullptr ))
        {
            //The decryption succeeds

            //Destroy the encryption context
            xmlSecEncCtxDestroy( pEncCtx ) ;
            SecurityEnvironment_NssImpl::destroyKeysManager( pMngr );

            //get the decrypted element
            XMLElementWrapper_XmlSecImpl * ret = new XMLElementWrapper_XmlSecImpl(isParentRef?
                (referenceNode->children):(referenceNode->next));

            //return ret;
            aTemplate->setTemplate(ret);
            break;
        }
        else
        {
            //The decryption fails, continue with the next security environment
            xmlSecEncCtxDestroy( pEncCtx ) ;
            SecurityEnvironment_NssImpl::destroyKeysManager( pMngr );
        }
    }

    clearErrorRecorder();
    return aTemplate;
}

/* XServiceInfo */
OUString SAL_CALL XMLEncryption_NssImpl::getImplementationName() {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLEncryption_NssImpl::supportsService( const OUString& serviceName) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return true ;
    }
    return false ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLEncryption_NssImpl::getSupportedServiceNames() {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLEncryption_NssImpl::impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence<OUString> seqServiceNames { "com.sun.star.xml.crypto.XMLEncryption" };
    return seqServiceNames ;
}

OUString XMLEncryption_NssImpl::impl_getImplementationName() {
    return OUString("com.sun.star.xml.security.bridge.xmlsec.XMLEncryption_NssImpl") ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLEncryption_NssImpl::impl_createInstance( const Reference< XMultiServiceFactory >&  ) {
    return Reference< XInterface >( *new XMLEncryption_NssImpl ) ;
}

Reference< XSingleServiceFactory > XMLEncryption_NssImpl::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
