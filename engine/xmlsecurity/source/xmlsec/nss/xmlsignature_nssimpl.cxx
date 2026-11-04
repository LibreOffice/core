/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <xmlsec/nss/x509.h>

#include <xmlelementwrapper_xmlsecimpl.hxx>
#include <SignatureTemplateInternal.hxx>
#include <xmlsec/xmlstreamio.hxx>
#include <xmlsec/errorcallback.hxx>

#include "securityenvironment_nssimpl.hxx"
#include "x509certificate_nssimpl.hxx"
#include "xmlsignature_nssimpl.hxx"
#ifdef MACOSX
#include <apple/xmlsignature_appleimpl.hxx>
#endif

#include <xmlsec/xmldsig.h>
#include <sal/log.hxx>

#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <memory>

namespace cpo::uno { class XComponentContext; }

using namespace ::com::sun::star;
using namespace ::cpo::uno ;
using namespace ::com::sun::star::lang ;
using namespace ::cpo::uno;

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

namespace
{

CERTCertificate const* GetNssCert(
        cpo::uno::Reference<css::security::XCertificate> const& xCertificate)
{
    X509Certificate_NssImpl *const pCert{dynamic_cast<X509Certificate_NssImpl*>(xCertificate.get())};
    return pCert ? pCert->getNssCert() : nullptr;
}

/// convert to xmlsec key - the returned value owns the certificate
xmlSecKeyPtr CreateKeyFromCertificate(cpo::uno::Reference<css::security::XCertificate> const& xCertificate)
{
    CERTCertificate const* pCertificate{GetNssCert(xCertificate)};
    assert(pCertificate);
    // xmlsec will take ownership of "signKey"
    pCertificate = CERT_DupCertificate(const_cast<CERTCertificate *>(pCertificate));
    // NSS keeps the reference count of a certificate in its NSSCertificate, so a certificate that
    // still has none after CERT_DupCertificate has no reference of its own to hand to xmlsec:
    if (!pCertificate->nssCertificate)
    {
        return nullptr;
    }

    xmlSecKeyDataPtr const pValue{xmlSecNssX509CertGetKey(const_cast<CERTCertificate *>(pCertificate))};
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
        CERT_DestroyCertificate(const_cast<CERTCertificate *>(pCertificate));
        return nullptr;
    }

    // The key owns the X.509 data, and the X.509 data owns the certificate once it is adopted.
    xmlSecKeyDataPtr const pX509Data{xmlSecKeyEnsureData(pKey, xmlSecNssKeyDataX509GetKlass())};
    if (!pX509Data || xmlSecNssKeyDataX509AdoptKeyCert(pX509Data, const_cast<CERTCertificate *>(pCertificate)) < 0)
    {
        CERT_DestroyCertificate(const_cast<CERTCertificate *>(pCertificate));
        xmlSecKeyDestroy(pKey);
        return nullptr;
    }
    return pKey;
}

cpo::uno::Reference<css::security::XCertificate> GetKeyCertificate(xmlSecKeyPtr pKey,
    cpo::uno::Reference<css::security::XCertificate> const& xSigningCertificate)
{
    if (!pKey)
    {
        return {};
    }
    xmlSecKeyDataPtr const pX509Data{xmlSecKeyGetData(pKey, xmlSecNssKeyDataX509GetKlass())};
    if (!pX509Data)
    {
        return {};
    }
    CERTCertificate *const pCertificate{xmlSecNssKeyDataX509GetKeyCert(pX509Data)};
    if (!pCertificate)
    {
        return {};
    }
    CERTCertificate const*const pSigningCertificate{GetNssCert(xSigningCertificate)};
    if (pSigningCertificate == pCertificate)
    {
        return xSigningCertificate;
    }
    else
    {
        rtl::Reference<X509Certificate_NssImpl> const pNew{new X509Certificate_NssImpl()};
        pNew->setCert(pCertificate);
        return pNew;
    }
}

} // namespace

XMLSignature_NssImpl::XMLSignature_NssImpl() {
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
XMLSignature_NssImpl::generate(
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
    pDsigCtx->keyInfoReadCtx.flags |= XMLSEC_KEYINFO_FLAGS_X509DATA_DONT_VERIFY_CERTS;

    // Allow ODF/OOXML references to resolve.
    pDsigCtx->enabledReferenceUris = xmlSecTransformUriTypeAny;

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
XMLSignature_NssImpl::validate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XXMLSecurityContext >& aSecurityCtx
) {
    xmlNodePtr pNode = nullptr ;
    //bool valid ;

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

    auto const pInternal{dynamic_cast<xmlsecurity::ISignatureTemplateInternal*>(aTemplate.get())};
    cpo::uno::Reference<css::security::XCertificate> const xSigningCertificate{
        pInternal ? pInternal->GetSigningCertificate() : nullptr};

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

        // Allow ODF/OOXML references to resolve.
        pDsigCtx->enabledReferenceUris = xmlSecTransformUriTypeAny;

        // limit possible key data to valid X509 certificates only, no KeyValues
        if (xmlSecPtrListAdd(&(pDsigCtx->keyInfoReadCtx.enabledKeyData), BAD_CAST xmlSecNssKeyDataX509GetKlass()) < 0)
            throw RuntimeException(u"failed to limit allowed key data"_ustr);

        // xmlsec reads ds:KeyInfo only when the context has no key yet, so an
        // expected certificate is the only source of the key.
        if (xSigningCertificate.is())
        {
            pDsigCtx->signKey = CreateKeyFromCertificate(xSigningCertificate);
            if (!pDsigCtx->signKey)
            {
                SAL_WARN("xmlsecurity.xmlsec", "cannot create key from the expected certificate");
                aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_UNKNOWN);
                continue;
            }
        }

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
            if (pInternal)
            {
                pInternal->SetVerifiedCertificate(GetKeyCertificate(pDsigCtx->signKey, xSigningCertificate));
            }
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
OUString XMLSignature_NssImpl::getImplementationName()
{
    return u"com.sun.star.xml.crypto.XMLSignature"_ustr;
}

/* XServiceInfo */
bool XMLSignature_NssImpl::supportsService(const OUString& rServiceName)
{
    const cpo::uno::Sequence<OUString> aServiceNames = getSupportedServiceNames();
    for (OUString const & rCurrentServiceName : aServiceNames)
    {
        if (rCurrentServiceName == rServiceName)
            return true;
    }
    return false;
}

/* XServiceInfo */
Sequence<OUString> XMLSignature_NssImpl::getSupportedServiceNames()
{
    return { u"com.sun.star.xml.crypto.XMLSignature"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
com_sun_star_xml_crypto_XMLSignature_get_implementation(cpo::uno::XComponentContext* /*pCtx*/,
                                                        cpo::uno::Sequence<cpo::uno::Any> const& /*rSeq*/)
{
#ifdef MACOSX
    // The macOS implementation extends the NSS one with Keychain-backed signing.
    return cppu::acquire(new XMLSignature_AppleImpl);
#else
    return cppu::acquire(new XMLSignature_NssImpl);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
