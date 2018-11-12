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

#include <xmlsec/xmldocumentwrapper_xmlsecimpl.hxx>
#include <xmlelementwrapper_xmlsecimpl.hxx>
#include <xmlsec/xmlstreamio.hxx>
#include <xmlsec/errorcallback.hxx>

#include "securityenvironment_nssimpl.hxx"

#include <xmlsec-wrapper.h>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
using ::com::sun::star::xml::wrapper::XXMLDocumentWrapper ;
using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLSignature ;
using ::com::sun::star::xml::crypto::XXMLSignatureTemplate ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;
using ::com::sun::star::xml::crypto::XUriBinding ;

namespace std
{
template <> struct default_delete<xmlSecKeysMngr>
{
    void operator()(xmlSecKeysMngrPtr ptr) { SecurityEnvironment_NssImpl::destroyKeysManager(ptr); }
};
template <> struct default_delete<xmlSecDSigCtx>
{
    void operator()(xmlSecDSigCtxPtr ptr) { xmlSecDSigCtxDestroy(ptr); }
};
}

class XMLSignature_NssImpl
    : public ::cppu::WeakImplHelper<xml::crypto::XXMLSignature, lang::XServiceInfo>
{
public:
    explicit XMLSignature_NssImpl();

    //Methods from XXMLSignature
    virtual uno::Reference<xml::crypto::XXMLSignatureTemplate> SAL_CALL
    generate(const uno::Reference<xml::crypto::XXMLSignatureTemplate>& aTemplate,
             const uno::Reference<xml::crypto::XSecurityEnvironment>& aEnvironment) override;

    virtual uno::Reference<xml::crypto::XXMLSignatureTemplate> SAL_CALL
    validate(const uno::Reference<xml::crypto::XXMLSignatureTemplate>& aTemplate,
             const uno::Reference<xml::crypto::XXMLSecurityContext>& aContext) override;

    //Methods from XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;

    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

XMLSignature_NssImpl::XMLSignature_NssImpl() {
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
SAL_CALL XMLSignature_NssImpl::generate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XSecurityEnvironment >& aEnvironment
)
{
    xmlNodePtr pNode = nullptr ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aEnvironment.is() )
        throw RuntimeException() ;

    //Get the xml node
    Reference< XXMLElementWrapper > xElement = aTemplate->getTemplate() ;
    if( !xElement.is() ) {
        throw RuntimeException() ;
    }

    Reference< XUnoTunnel > xNodTunnel( xElement , UNO_QUERY_THROW ) ;
    XMLElementWrapper_XmlSecImpl* pElement =
        reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xNodTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() )));
    if( pElement == nullptr ) {
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
    Reference< XUnoTunnel > xSecTunnel( aEnvironment , UNO_QUERY_THROW ) ;

    // the key manager should be retrieved from SecurityEnvironment, instead of SecurityContext

    SecurityEnvironment_NssImpl* pSecEnv =
        reinterpret_cast<SecurityEnvironment_NssImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xSecTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() )));
    if( pSecEnv == nullptr )
        throw RuntimeException() ;

    setErrorRecorder();

    std::unique_ptr<xmlSecKeysMngr> pMngr(pSecEnv->createKeysManager());
    if( !pMngr ) {
        throw RuntimeException() ;
    }

    //Create Signature context
    std::unique_ptr<xmlSecDSigCtx> pDsigCtx(xmlSecDSigCtxCreate(pMngr.get()));
    if( pDsigCtx == nullptr )
    {
        //throw XMLSignatureException() ;
        clearErrorRecorder();
        return aTemplate;
    }

    //Sign the template
    if( xmlSecDSigCtxSign( pDsigCtx.get() , pNode ) == 0 )
    {
        if (pDsigCtx->status == xmlSecDSigStatusSucceeded)
            aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED);
        else
            aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_UNKNOWN);
    }
    else
    {
        aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_UNKNOWN);
    }

    //Unregistered the stream/URI binding
    if( xUriBinding.is() )
        xmlUnregisterStreamInputCallbacks() ;

    clearErrorRecorder();
    return aTemplate ;
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
SAL_CALL XMLSignature_NssImpl::validate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XXMLSecurityContext >& aSecurityCtx
) {
    xmlNodePtr pNode = nullptr ;
    //sal_Bool valid ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aSecurityCtx.is() )
        throw RuntimeException() ;

    //Get the xml node
    Reference< XXMLElementWrapper > xElement = aTemplate->getTemplate() ;
    if( !xElement.is() )
        throw RuntimeException() ;

    Reference< XUnoTunnel > xNodTunnel( xElement , UNO_QUERY_THROW ) ;
    XMLElementWrapper_XmlSecImpl* pElement =
        reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xNodTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() )));
    if( pElement == nullptr )
        throw RuntimeException() ;

    pNode = pElement->getNativeElement() ;

    //Get the stream/URI binding
    Reference< XUriBinding > xUriBinding = aTemplate->getBinding() ;
    if( xUriBinding.is() ) {
        //Register the stream input callbacks into libxml2
        if( xmlRegisterStreamInputCallbacks( xUriBinding ) < 0 )
            throw RuntimeException() ;
    }

    setErrorRecorder();

    sal_Int32 nSecurityEnvironment = aSecurityCtx->getSecurityEnvironmentNumber();
    sal_Int32 i;

    for (i=0; i<nSecurityEnvironment; ++i)
    {
        Reference< XSecurityEnvironment > aEnvironment = aSecurityCtx->getSecurityEnvironmentByIndex(i);

        //Get Keys Manager
        Reference< XUnoTunnel > xSecTunnel( aEnvironment , UNO_QUERY_THROW ) ;
        SecurityEnvironment_NssImpl* pSecEnv =
            reinterpret_cast<SecurityEnvironment_NssImpl*>(
                sal::static_int_cast<sal_uIntPtr>(
                    xSecTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() )));
        if( pSecEnv == nullptr )
            throw RuntimeException() ;

        std::unique_ptr<xmlSecKeysMngr> pMngr(pSecEnv->createKeysManager());
        if( !pMngr ) {
            throw RuntimeException() ;
        }

        //Create Signature context
        std::unique_ptr<xmlSecDSigCtx> pDsigCtx(xmlSecDSigCtxCreate(pMngr.get()));
        if( pDsigCtx == nullptr )
        {
            clearErrorRecorder();
            return aTemplate;
        }

        // We do certificate verification ourselves.
        pDsigCtx->keyInfoReadCtx.flags |= XMLSEC_KEYINFO_FLAGS_X509DATA_DONT_VERIFY_CERTS;

        //Verify signature
        int rs = xmlSecDSigCtxVerify( pDsigCtx.get() , pNode );

        // Also verify manifest: this is empty for ODF, but contains everything (except signature metadata) for OOXML.
        xmlSecSize nReferenceCount = xmlSecPtrListGetSize(&pDsigCtx->manifestReferences);
        // Require that all manifest references are also good.
        xmlSecSize nReferenceGood = 0;
        for (xmlSecSize nReference = 0; nReference < nReferenceCount; ++nReference)
        {
            xmlSecDSigReferenceCtxPtr pReference = static_cast<xmlSecDSigReferenceCtxPtr>(xmlSecPtrListGetItem(&pDsigCtx->manifestReferences, nReference));
            if (pReference)
            {
                if (pReference->status == xmlSecDSigStatusSucceeded)
                    ++nReferenceGood;
            }
        }

        if (rs == 0 && pDsigCtx->status == xmlSecDSigStatusSucceeded && nReferenceCount == nReferenceGood)
        {
            aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED);
            break;
        }
        else
        {
            aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_UNKNOWN);
        }
    }


    //Unregistered the stream/URI binding
    if( xUriBinding.is() )
        xmlUnregisterStreamInputCallbacks() ;

    //return valid ;
    clearErrorRecorder();
    return aTemplate;
}

/* XServiceInfo */
OUString SAL_CALL XMLSignature_NssImpl::getImplementationName()
{
    return OUString("com.sun.star.xml.crypto.XMLSignature");
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSignature_NssImpl::supportsService(const OUString& rServiceName)
{
    for (OUString const & rCurrentServiceName : getSupportedServiceNames())
    {
        if (rCurrentServiceName == rServiceName)
            return true;
    }
    return false;
}

/* XServiceInfo */
Sequence<OUString> SAL_CALL XMLSignature_NssImpl::getSupportedServiceNames()
{
    Sequence<OUString> seqServiceNames { "com.sun.star.xml.crypto.XMLSignature" };
    return seqServiceNames;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_XMLSignature_get_implementation(uno::XComponentContext* /*pCtx*/,
                                                        uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new XMLSignature_NssImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
