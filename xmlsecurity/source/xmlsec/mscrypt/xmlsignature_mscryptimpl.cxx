/*************************************************************************
 *
 *  $RCSfile: xmlsignature_mscryptimpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-23 03:12:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _XMLSIGNATURE_MSCRYPTIMPL_HXX_
#include "xmlsignature_mscryptimpl.hxx"
#endif

#ifndef _XMLDOCUMENTWRAPPER_XMLSECIMPL_HXX_
#include "xmldocumentwrapper_xmlsecimpl.hxx"
#endif

#ifndef _XMLELEMENTWRAPPER_XMLSECIMPL_HXX_
#include "xmlelementwrapper_xmlsecimpl.hxx"
#endif

#ifndef _XMLSECURITYCONTEXT_MSCRYPTIMPL_HXX_
#include "xmlsecuritycontext_mscryptimpl.hxx"
#endif

#ifndef _XMLSTREAMIO_XMLSECIMPL_HXX_
#include "xmlstreamio.hxx"
#endif

#include "xmlsec/xmlsec.h"
#include "xmlsec/xmldsig.h"
#include "xmlsec/crypto.h"

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
using ::com::sun::star::xml::wrapper::XXMLDocumentWrapper ;
using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLSignature ;
using ::com::sun::star::xml::crypto::XXMLSignatureTemplate ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;
using ::com::sun::star::xml::crypto::XUriBinding ;
using ::com::sun::star::xml::crypto::XMLSignatureException ;

XMLSignature_MSCryptImpl :: XMLSignature_MSCryptImpl( const Reference< XMultiServiceFactory >& aFactory ) : m_xServiceManager( aFactory ) {
}

XMLSignature_MSCryptImpl :: ~XMLSignature_MSCryptImpl() {
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
SAL_CALL XMLSignature_MSCryptImpl :: generate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XXMLSecurityContext >& aSecurityCtx
) throw( com::sun::star::xml::crypto::XMLSignatureException,
         com::sun::star::uno::SecurityException )
{
    xmlSecKeysMngrPtr pMngr = NULL ;
    xmlSecDSigCtxPtr pDsigCtx = NULL ;
    xmlNodePtr pNode = NULL ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aSecurityCtx.is() )
        throw RuntimeException() ;

    //Get the xml node
    Reference< XXMLElementWrapper > xElement = aTemplate->getTemplate() ;
    if( !xElement.is() ) {
        throw RuntimeException() ;
    }

    Reference< XUnoTunnel > xNodTunnel( xElement , UNO_QUERY ) ;
    if( !xNodTunnel.is() ) {
        throw RuntimeException() ;
    }

    XMLElementWrapper_XmlSecImpl* pElement = ( XMLElementWrapper_XmlSecImpl* )xNodTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() ) ;
    if( pElement == NULL ) {
        throw RuntimeException() ;
    }

    pNode = pElement->getNativeElement() ;

    //Get the stream/URI binding
    Reference< XUriBinding > xUriBinding = aTemplate->getBinding() ;
    if( xUriBinding.is() ) {
        //Register the stream input callbacks into libxml2
        if( xmlRegisterStreamInputCallbacks( xUriBinding ) < 0 )
            throw RuntimeException() ;
    }

    //Get Keys Manager
    Reference< XUnoTunnel > xSecTunnel( aSecurityCtx , UNO_QUERY ) ;
    if( !xSecTunnel.is() ) {
         throw RuntimeException() ;
    }

    XMLSecurityContext_MSCryptImpl* pSecCtxt = ( XMLSecurityContext_MSCryptImpl* )xSecTunnel->getSomething( XMLSecurityContext_MSCryptImpl::getUnoTunnelId() ) ;
    if( pSecCtxt == NULL )
        throw RuntimeException() ;

    pMngr = pSecCtxt->keysManager() ;

    //Create Signature context
    pDsigCtx = xmlSecDSigCtxCreate( pMngr ) ;
    if( pDsigCtx == NULL )
        throw XMLSignatureException() ;

    //Sign the template
    if( xmlSecDSigCtxSign( pDsigCtx , pNode ) < 0 ) {
        xmlSecDSigCtxDestroy( pDsigCtx ) ;

        //Unregistered the stream/URI binding
        if( xUriBinding.is() )
            xmlUnregisterStreamInputCallbacks() ;

        throw XMLSignatureException() ;
    }

    xmlSecDSigCtxDestroy( pDsigCtx ) ;

    //Unregistered the stream/URI binding
    if( xUriBinding.is() )
        xmlUnregisterStreamInputCallbacks() ;

    return aTemplate ;
}

/* XXMLSignature */
sal_Bool SAL_CALL
XMLSignature_MSCryptImpl :: validate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XXMLSecurityContext >& aSecurityCtx
) throw( com::sun::star::uno::RuntimeException,
         com::sun::star::uno::SecurityException,
         com::sun::star::xml::crypto::XMLSignatureException ) {
    xmlSecKeysMngrPtr pMngr = NULL ;
    xmlSecDSigCtxPtr pDsigCtx = NULL ;
    xmlNodePtr pNode = NULL ;
    sal_Bool valid ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aSecurityCtx.is() )
        throw RuntimeException() ;

    //Get the xml node
    Reference< XXMLElementWrapper > xElement = aTemplate->getTemplate() ;
    if( !xElement.is() )
        throw RuntimeException() ;

    Reference< XUnoTunnel > xNodTunnel( xElement , UNO_QUERY ) ;
    if( !xNodTunnel.is() ) {
        throw RuntimeException() ;
    }

    XMLElementWrapper_XmlSecImpl* pElement = ( XMLElementWrapper_XmlSecImpl* )xNodTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() ) ;
    if( pElement == NULL )
        throw RuntimeException() ;

    pNode = pElement->getNativeElement() ;

    //Get the stream/URI binding
    Reference< XUriBinding > xUriBinding = aTemplate->getBinding() ;
    if( xUriBinding.is() ) {
        //Register the stream input callbacks into libxml2
        if( xmlRegisterStreamInputCallbacks( xUriBinding ) < 0 )
            throw RuntimeException() ;
    }

    //added for test: save the result
    /*
    {
        FILE *dstFile = fopen( "c:\\1.txt", "w" ) ;
        xmlDocDump( dstFile, pNode->doc) ;
        fclose( dstFile ) ;
    }
    */

    //Get Keys Manager
    Reference< XUnoTunnel > xSecTunnel( aSecurityCtx , UNO_QUERY ) ;
    if( !xSecTunnel.is() ) {
         throw RuntimeException() ;
    }

    XMLSecurityContext_MSCryptImpl* pSecCtxt = ( XMLSecurityContext_MSCryptImpl* )xSecTunnel->getSomething( XMLSecurityContext_MSCryptImpl::getUnoTunnelId() ) ;
    if( pSecCtxt == NULL )
        throw RuntimeException() ;

    pMngr = pSecCtxt->keysManager() ;

    //Create Signature context
    pDsigCtx = xmlSecDSigCtxCreate( pMngr ) ;
    if( pDsigCtx == NULL )
        throw XMLSignatureException() ;


    //Verify signature
    if( xmlSecDSigCtxVerify( pDsigCtx , pNode ) < 0 ) {
        xmlSecDSigCtxDestroy( pDsigCtx ) ;

        //Unregistered the stream/URI binding
        if( xUriBinding.is() )
            xmlUnregisterStreamInputCallbacks() ;

        throw XMLSignatureException() ;
    }

    valid = ( pDsigCtx->status == xmlSecDSigStatusSucceeded ) ;

    xmlSecDSigCtxDestroy( pDsigCtx ) ;

    //Unregistered the stream/URI binding
    if( xUriBinding.is() )
        xmlUnregisterStreamInputCallbacks() ;

    return valid ;
}

/* XInitialization */
void SAL_CALL XMLSignature_MSCryptImpl :: initialize( const Sequence< Any >& aArguments ) throw( Exception, RuntimeException ) {
    // TBD
} ;

/* XServiceInfo */
OUString SAL_CALL XMLSignature_MSCryptImpl :: getImplementationName() throw( RuntimeException ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSignature_MSCryptImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSignature_MSCryptImpl :: getSupportedServiceNames() throw( RuntimeException ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSignature_MSCryptImpl :: impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence< OUString > seqServiceNames( 1 ) ;
    seqServiceNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.xml.crypto.XMLSignature" ) ;
    return seqServiceNames ;
}

OUString XMLSignature_MSCryptImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString::createFromAscii( "com.sun.star.xml.security.bridge.xmlsec.XMLSignature_MSCryptImpl" ) ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSignature_MSCryptImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
    return Reference< XInterface >( *new XMLSignature_MSCryptImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > XMLSignature_MSCryptImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

