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
#include <xmlsec-wrapper.h>

#include <xmlsec/nss/x509.h>

#include <xmlelementwrapper_xmlsecimpl.hxx>
#include <xmlsec/xmlstreamio.hxx>
#include <xmlsec/errorcallback.hxx>

#include "securityenvironment_nssimpl.hxx"

#include <xmlsec/xmldsig.h>
#include <sal/log.hxx>

#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <memory>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
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

namespace {

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

}

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

    XMLElementWrapper_XmlSecImpl* pElement
        = dynamic_cast<XMLElementWrapper_XmlSecImpl*>(xElement.get());
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

    // the key manager should be retrieved from SecurityEnvironment, instead of SecurityContext
    SecurityEnvironment_NssImpl* pSecEnv
        = dynamic_cast<SecurityEnvironment_NssImpl*>(aEnvironment.get());
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

    XMLElementWrapper_XmlSecImpl* pElement
        = dynamic_cast<XMLElementWrapper_XmlSecImpl*>(xElement.get());
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
        SecurityEnvironment_NssImpl* pSecEnv
            = dynamic_cast<SecurityEnvironment_NssImpl*>(aEnvironment.get());
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

        // limit possible key data to valid X509 certificates only, no KeyValues
        if (xmlSecPtrListAdd(&(pDsigCtx->keyInfoReadCtx.enabledKeyData), BAD_CAST xmlSecNssKeyDataX509GetKlass()) < 0)
            throw RuntimeException("failed to limit allowed key data");

        xmlBufferPtr pBuf = xmlBufferCreate();
        xmlNodeDump(pBuf, nullptr, pNode, 0, 0);
        SAL_INFO("xmlsecurity.xmlsec", "xmlSecDSigCtxVerify input XML node is '"
                                           << reinterpret_cast<const char*>(xmlBufferContent(pBuf))
                                           << "'");
        xmlBufferFree(pBuf);

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
        SAL_INFO("xmlsecurity.xmlsec", "xmlSecDSigCtxVerify status " << pDsigCtx->status << ", references good " << nReferenceGood << " of " << nReferenceCount);

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
    return "com.sun.star.xml.crypto.XMLSignature";
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSignature_NssImpl::supportsService(const OUString& rServiceName)
{
    const css::uno::Sequence<OUString> aServiceNames = getSupportedServiceNames();
    for (OUString const & rCurrentServiceName : aServiceNames)
    {
        if (rCurrentServiceName == rServiceName)
            return true;
    }
    return false;
}

/* XServiceInfo */
Sequence<OUString> SAL_CALL XMLSignature_NssImpl::getSupportedServiceNames()
{
    return { "com.sun.star.xml.crypto.XMLSignature" };
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_XMLSignature_get_implementation(uno::XComponentContext* /*pCtx*/,
                                                        uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new XMLSignature_NssImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
