/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlencryption_mscryptimpl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:31:29 $
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

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _XMLENCRYPTION_MSCRYPTIMPL_HXX_
#include "xmlencryption_mscryptimpl.hxx"
#endif

#ifndef _XMLDOCUMENTWRAPPER_XMLSECIMPL_HXX_
#include "xmldocumentwrapper_xmlsecimpl.hxx"
#endif

#ifndef _XMLELEMENTWRAPPER_XMLSECIMPL_HXX_
#include "xmlelementwrapper_xmlsecimpl.hxx"
#endif

#ifndef _SECURITYENVIRONMENT_MSCRYPTIMPL_HXX_
#include "securityenvironment_mscryptimpl.hxx"
#endif

#ifndef _ERRORCALLBACK_XMLSECIMPL_HXX_
#include "errorcallback.hxx"
#endif

#include "xmlsec/xmlsec.h"
#include "xmlsec/xmltree.h"
#include "xmlsec/xmlenc.h"
#include "xmlsec/crypto.h"

#ifdef UNX
#define stricmp strcasecmp
#endif

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

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
    xmlNodePtr pEncryptedKey = NULL ;
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

     setErrorRecorder( aTemplate );

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
        xmlSecEncCtxDestroy( pEncCtx ) ;
        pSecEnv->destroyKeysManager( pMngr ) ; //i39448

        //throw XMLEncryptionException() ;
        clearErrorRecorder();
        return aTemplate;
    }

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
    xmlNodePtr pContent = NULL ;

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

     setErrorRecorder( aTemplate );

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
        xmlSecEncCtxDestroy( pEncCtx ) ;
        pSecEnv->destroyKeysManager( pMngr ) ; //i39448

        //throw XMLEncryptionException() ;
        clearErrorRecorder();
        return aTemplate;
    }
    /*----------------------------------------
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
    ----------------------------------------*/

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

/* XInitialization */
void SAL_CALL XMLEncryption_MSCryptImpl :: initialize( const Sequence< Any >& aArguments ) throw( Exception, RuntimeException ) {
    // TBD
} ;

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
    seqServiceNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.xml.crypto.XMLEncryption" ) ;
    return seqServiceNames ;
}

OUString XMLEncryption_MSCryptImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString::createFromAscii( "com.sun.star.xml.security.bridge.xmlsec.XMLEncryption_MSCryptImpl" ) ;
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

