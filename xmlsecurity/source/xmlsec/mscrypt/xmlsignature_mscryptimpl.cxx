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
#include <sal/log.hxx>
#include <rtl/uuid.h>

#include <xmlsec/xmldsig.h>
#include <xmlsec/xmltree.h> // this MUST precede the mscng/x509.h for some reason
#include <xmlsec/mscng/x509.h>
#include <xmlsec/mscng/certkeys.h>

#include <com/sun/star/xml/crypto/SecurityOperationStatus.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>

#include "securityenvironment_mscryptimpl.hxx"
#include "x509certificate_mscryptimpl.hxx"

#include <xmlsec/xmldocumentwrapper_xmlsecimpl.hxx>
#include <SignatureTemplateInternal.hxx>
#include <xmlelementwrapper_xmlsecimpl.hxx>
#include <xmlsec/xmlstreamio.hxx>
#include <xmlsec/errorcallback.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLSignature ;
using ::com::sun::star::xml::crypto::XXMLSignatureTemplate ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;
using ::com::sun::star::xml::crypto::XUriBinding ;

namespace {

CERT_CONTEXT const* GetMsCert(
        css::uno::Reference<css::security::XCertificate> const& xCertificate)
{
    X509Certificate_MSCryptImpl *const pCert{dynamic_cast<X509Certificate_MSCryptImpl*>(xCertificate.get())};
    return pCert ? pCert->getMswcryCert() : nullptr;
}

/// convert to xmlsec key - the returned value owns the certificate
xmlSecKeyPtr CreateKeyFromCertificate(css::uno::Reference<css::security::XCertificate> const& xCertificate)
{
    PCCERT_CONTEXT pCertificate{GetMsCert(xCertificate)};
    // in contrast to the NSS one, the MSCNG backend wants one certificate
    // for the key value, and one for they key's X509 data...
    pCertificate = CertDuplicateCertificateContext(pCertificate);
    assert(pCertificate);

    // The key value owns the certificate context that it is created from.
    PCCERT_CONTEXT pValueCertificate = CertDuplicateCertificateContext(pCertificate);
    xmlSecKeyDataPtr const pValue{xmlSecMSCngCertAdopt(pValueCertificate, xmlSecKeyDataTypePublic)};
    if (!pValue)
    {
        CertFreeCertificateContext(pValueCertificate);
    }
    xmlSecKeyPtr const pKey{pValue ? xmlSecKeyCreate() : nullptr};
    if (!pKey || xmlSecKeySetValue(pKey, pValue) < 0)
    {
        if (pValue)
        {
            xmlSecKeyDataDestroy(pValue);
        }
        if (pKey)
        {
            xmlSecKeyDestroy(pKey);
        }
        CertFreeCertificateContext(pCertificate);
        return nullptr;
    }

    // The key owns the X.509 data, and the X.509 data owns the certificate once it is adopted.
    xmlSecKeyDataPtr pX509Data = xmlSecKeyEnsureData(pKey, xmlSecMSCngKeyDataX509GetKlass());
    if (!pX509Data || xmlSecMSCngKeyDataX509AdoptKeyCert(pX509Data, pCertificate) < 0)
    {
        CertFreeCertificateContext(pCertificate);
        xmlSecKeyDestroy(pKey);
        return nullptr;
    }
    return pKey;
}

css::uno::Reference<css::security::XCertificate> GetKeyCertificate(xmlSecKeyPtr pKey,
    css::uno::Reference<css::security::XCertificate> const& xSigningCertificate)
{
    if (!pKey)
        return {};
    xmlSecKeyDataPtr const pX509Data{xmlSecKeyGetData(pKey, xmlSecMSCngKeyDataX509GetKlass())};
    if (!pX509Data)
        return {};
    PCCERT_CONTEXT const pCertificate{xmlSecMSCngKeyDataX509GetKeyCert(pX509Data)};
    if (!pCertificate)
        return {};
    PCCERT_CONTEXT const pSigningCertificate{GetMsCert(xSigningCertificate)};
    if (pSigningCertificate == pCertificate)
    {
        return xSigningCertificate;
    }
    else
    {
        rtl::Reference<X509Certificate_MSCryptImpl> const pNew{new X509Certificate_MSCryptImpl()};
        pNew->setMswcryCert(pCertificate);
        return pNew;
    }
}

class XMLSignature_MSCryptImpl : public ::cppu::WeakImplHelper<
    css::xml::crypto::XXMLSignature ,
    css::lang::XServiceInfo >
{
    public:
        explicit XMLSignature_MSCryptImpl();

        //Methods from XXMLSignature
        virtual css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate > SAL_CALL generate(
            const css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate >& aTemplate ,
            const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& aEnvironment
        ) override;

        virtual css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate > SAL_CALL validate(
            const css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate >& aTemplate ,
            const css::uno::Reference< css::xml::crypto::XXMLSecurityContext >& aContext
        ) override;

        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
} ;

}

XMLSignature_MSCryptImpl::XMLSignature_MSCryptImpl() {
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
SAL_CALL XMLSignature_MSCryptImpl::generate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XSecurityEnvironment >& aEnvironment
)
{
    xmlSecKeysMngrPtr pMngr = nullptr ;
    xmlSecDSigCtxPtr pDsigCtx = nullptr ;
    xmlNodePtr pNode = nullptr ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aEnvironment.is() )
        throw RuntimeException() ;

    //Get Keys Manager
    SecurityEnvironment_MSCryptImpl* pSecEnv = dynamic_cast<SecurityEnvironment_MSCryptImpl*>(aEnvironment.get());
    if( pSecEnv == nullptr )
        throw RuntimeException() ;

    //Get the xml node
    Reference< XXMLElementWrapper > xElement = aTemplate->getTemplate() ;
    if( !xElement.is() ) {
        throw RuntimeException() ;
    }

    XMLElementWrapper_XmlSecImpl* pElement = dynamic_cast<XMLElementWrapper_XmlSecImpl*>(xElement.get());
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

    setErrorRecorder( );

    pMngr = pSecEnv->createKeysManager();
    if( !pMngr ) {
        throw RuntimeException() ;
    }

    //Create Signature context
    pDsigCtx = xmlSecDSigCtxCreate( pMngr ) ;
    if( pDsigCtx == nullptr )
    {
        //throw XMLSignatureException() ;
        SecurityEnvironment_MSCryptImpl::destroyKeysManager( pMngr );
        clearErrorRecorder();
        return aTemplate;
    }

    //Sign the template
    pDsigCtx->keyInfoReadCtx.flags |= XMLSEC_KEYINFO_FLAGS_X509DATA_DONT_VERIFY_CERTS;
    if( xmlSecDSigCtxSign( pDsigCtx , pNode ) == 0 )
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


    xmlSecDSigCtxDestroy( pDsigCtx ) ;
    SecurityEnvironment_MSCryptImpl::destroyKeysManager( pMngr );

    //Unregistered the stream/URI binding
    if( xUriBinding.is() )
        xmlUnregisterStreamInputCallbacks() ;

    clearErrorRecorder();
    return aTemplate ;
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
SAL_CALL XMLSignature_MSCryptImpl::validate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XXMLSecurityContext >& aSecurityCtx
) {
    xmlSecKeysMngrPtr pMngr = nullptr ;
    xmlSecDSigCtxPtr pDsigCtx = nullptr ;
    xmlNodePtr pNode = nullptr ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aSecurityCtx.is() )
        throw RuntimeException() ;

    //Get Keys Manager
    Reference< XSecurityEnvironment > xSecEnv
        = aSecurityCtx->getSecurityEnvironmentByIndex(
            aSecurityCtx->getDefaultSecurityEnvironmentIndex());
    SecurityEnvironment_MSCryptImpl* pSecEnv = dynamic_cast<SecurityEnvironment_MSCryptImpl*>(xSecEnv.get());
    if( pSecEnv == nullptr )
        throw RuntimeException() ;

    //Get the xml node
    Reference< XXMLElementWrapper > xElement = aTemplate->getTemplate() ;
    if( !xElement.is() )
        throw RuntimeException() ;

    XMLElementWrapper_XmlSecImpl* pElement = dynamic_cast<XMLElementWrapper_XmlSecImpl*>(xElement.get());
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

    setErrorRecorder( );

    pMngr = pSecEnv->createKeysManager();
    if( !pMngr ) {
        throw RuntimeException() ;
    }

    //Create Signature context
    pDsigCtx = xmlSecDSigCtxCreate( pMngr ) ;
    if( pDsigCtx == nullptr )
    {
        SecurityEnvironment_MSCryptImpl::destroyKeysManager( pMngr );
        clearErrorRecorder();
        return aTemplate;
    }

    // We do certificate verification ourselves.
    pDsigCtx->keyInfoReadCtx.flags |= XMLSEC_KEYINFO_FLAGS_X509DATA_DONT_VERIFY_CERTS;

    // limit possible key data to valid X509 certificates only, no KeyValues
    if (xmlSecPtrListAdd(&(pDsigCtx->keyInfoReadCtx.enabledKeyData), BAD_CAST xmlSecMSCngKeyDataX509GetKlass()) < 0)
        throw RuntimeException("failed to limit allowed key data");

    auto const pInternal{dynamic_cast<xmlsecurity::ISignatureTemplateInternal*>(aTemplate.get())};
    css::uno::Reference<css::security::XCertificate> const xSigningCertificate{
        pInternal ? pInternal->GetSigningCertificate() : nullptr};

    // xmlsec reads ds:KeyInfo only when the context has no key yet, so an
    // expected certificate is the only source of the key.
    if (xSigningCertificate.is())
    {
        pDsigCtx->signKey = CreateKeyFromCertificate(xSigningCertificate);
        if (!pDsigCtx->signKey)
        {
            SAL_WARN("xmlsecurity.xmlsec", "cannot create key from the expected certificate");
            aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_UNKNOWN);
            xmlSecDSigCtxDestroy(pDsigCtx);
            SecurityEnvironment_MSCryptImpl::destroyKeysManager(pMngr);
            if (xUriBinding.is())
                xmlUnregisterStreamInputCallbacks();
            clearErrorRecorder();
            return aTemplate;
        }
    }

    //Verify signature
    //The documentation says that the signature is only valid if the return value is 0 (that is, not < 0)
    //AND pDsigCtx->status == xmlSecDSigStatusSucceeded. That is, we must not make any assumptions, if
    //the return value is < 0. Then we must regard the signature as INVALID. We cannot use the
    //error recorder feature to get the ONE error that made the verification fail, because there is no
    //documentation/specification as to how to interpret the number of recorded errors and what is the initial
    //error.
    int rs = xmlSecDSigCtxVerify(pDsigCtx , pNode);

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

    if (rs == 0 && nReferenceCount == nReferenceGood)
    {
        if (pDsigCtx->status == xmlSecDSigStatusSucceeded)
        {
            aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED);
            if (pInternal)
            {
                pInternal->SetVerifiedCertificate(GetKeyCertificate(pDsigCtx->signKey, xSigningCertificate));
            }
        }
        else
            aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_UNKNOWN);
    }
    else
    {
        aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_UNKNOWN);
    }

    xmlSecDSigCtxDestroy( pDsigCtx ) ;
    SecurityEnvironment_MSCryptImpl::destroyKeysManager( pMngr );

    //Unregistered the stream/URI binding
    if( xUriBinding.is() )
        xmlUnregisterStreamInputCallbacks() ;


    clearErrorRecorder();
    return aTemplate;
}

/* XServiceInfo */
OUString SAL_CALL XMLSignature_MSCryptImpl::getImplementationName() {
    return "com.sun.star.xml.crypto.XMLSignature";
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSignature_MSCryptImpl::supportsService( const OUString& serviceName) {
return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSignature_MSCryptImpl::getSupportedServiceNames() {
    return { "com.sun.star.xml.crypto.XMLSignature" };
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_XMLSignature_get_implementation(uno::XComponentContext* /*pCtx*/,
                                                        uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new XMLSignature_MSCryptImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
