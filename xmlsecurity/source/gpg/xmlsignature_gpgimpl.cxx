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

#include <config_gpgme.h>

#include <sal/config.h>
#include <sal/log.hxx>
#include <rtl/uuid.h>
#include <cppuhelper/supportsservice.hxx>
#include <gpg/xmlsignature_gpgimpl.hxx>

#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#endif
#include <gpgme.h>
#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic pop
#endif
#include <context.h>
#include <key.h>
#include <data.h>
#include <signingresult.h>
#include <importresult.h>

#include <xmlsec/xmldocumentwrapper_xmlsecimpl.hxx>
#include <xmlelementwrapper_xmlsecimpl.hxx>
#include <xmlsec/xmlstreamio.hxx>
#include <xmlsec/errorcallback.hxx>

#include "SecurityEnvironment.hxx"
#include <xmlsec-wrapper.h>

using namespace css::uno;
using namespace css::lang;
using namespace css::xml::wrapper;
using namespace css::xml::crypto;

XMLSignature_GpgImpl::XMLSignature_GpgImpl() {
}

XMLSignature_GpgImpl::~XMLSignature_GpgImpl() {
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
SAL_CALL XMLSignature_GpgImpl::generate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XSecurityEnvironment >& aEnvironment
)
{
    xmlSecDSigCtxPtr pDsigCtx = nullptr ;
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

    XMLElementWrapper_XmlSecImpl* pElement =
        dynamic_cast<XMLElementWrapper_XmlSecImpl*>(xElement.get());
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
    SecurityEnvironmentGpg* pSecEnv =
        dynamic_cast<SecurityEnvironmentGpg*>(aEnvironment.get());
    if( pSecEnv == nullptr )
        throw RuntimeException() ;

    setErrorRecorder();

    //Create Signature context
    pDsigCtx = xmlSecDSigCtxCreate( nullptr ) ;
    if( pDsigCtx == nullptr )
    {
        clearErrorRecorder();
        return aTemplate;
    }

    // set intended operation to sign - several asserts inside libxmlsec
    // wanting that for digest / transforms
    pDsigCtx->operation = xmlSecTransformOperationSign;

    // we default to SHA512 for all digests - nss crypto does not have it...
    //pDsigCtx->defDigestMethodId = xmlSecTransformSha512Id;

    // Calculate digest for all references
    xmlNodePtr cur = xmlSecGetNextElementNode(pNode->children);
    if( cur != nullptr )
        cur = xmlSecGetNextElementNode(cur->children);
    while( cur != nullptr )
    {
        // some of those children I suppose should be reference elements
        if( xmlSecCheckNodeName(cur, xmlSecNodeReference, xmlSecDSigNs) )
        {
            xmlSecDSigReferenceCtxPtr pDsigRefCtx =
                xmlSecDSigReferenceCtxCreate(pDsigCtx,
                                             xmlSecDSigReferenceOriginSignedInfo);
            if(pDsigRefCtx == nullptr)
                throw RuntimeException();

            // add this one to the list
            if( xmlSecPtrListAdd(&(pDsigCtx->signedInfoReferences),
                                 pDsigRefCtx) < 0 )
            {
                // TODO resource handling
                xmlSecDSigReferenceCtxDestroy(pDsigRefCtx);
                throw RuntimeException();
            }

            if( xmlSecDSigReferenceCtxProcessNode(pDsigRefCtx, cur) < 0 )
                throw RuntimeException();

            // final check - all good?
            if(pDsigRefCtx->status != xmlSecDSigStatusSucceeded)
            {
                pDsigCtx->status = xmlSecDSigStatusInvalid;
                return aTemplate; // TODO - harder error?
            }
        }

        cur = xmlSecGetNextElementNode(cur->next);
    }

    // get me a digestible buffer from the signature template!
    // -------------------------------------------------------

    // run the transformations over SignedInfo element (first child of
    // pNode)
    xmlSecNodeSetPtr nodeset = nullptr;
    cur = xmlSecGetNextElementNode(pNode->children);
    // TODO assert that...
    nodeset = xmlSecNodeSetGetChildren(pNode->doc, cur, 1, 0);
    if(nodeset == nullptr)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    if( xmlSecTransformCtxXmlExecute(&(pDsigCtx->transformCtx), nodeset) < 0 )
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    // now extract the keyid from PGPData
    // walk xml tree to PGPData node - go to children, first is
    // SignedInfo, 2nd is signaturevalue, 3rd is KeyInfo
    // 1st child is PGPData, 1st grandchild is PGPKeyID
    cur = xmlSecGetNextElementNode(pNode->children);
    // TODO error handling
    cur = xmlSecGetNextElementNode(cur->next);
    cur = xmlSecGetNextElementNode(cur->next);
    cur = xmlSecGetNextElementNode(cur->children);
    // check that this is now PGPData
    if(!xmlSecCheckNodeName(cur, xmlSecNodePGPData, xmlSecDSigNs))
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");
    // check that this is now PGPKeyID
    cur = xmlSecGetNextElementNode(cur->children);
    static const xmlChar xmlSecNodePGPKeyID[] = "PGPKeyID";
    if(!xmlSecCheckNodeName(cur, xmlSecNodePGPKeyID, xmlSecDSigNs))
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    GpgME::Context& rCtx=pSecEnv->getGpgContext();
    rCtx.setKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    GpgME::Error err;
    xmlChar* pKey=xmlNodeGetContent(cur);
    if(xmlSecBase64Decode(pKey, reinterpret_cast<xmlSecByte*>(pKey), xmlStrlen(pKey)) < 0)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");
    if( rCtx.addSigningKey(
            rCtx.key(
                reinterpret_cast<char*>(pKey), err, true)) )
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    xmlFree(pKey);

    // good, ctx is setup now, let's sign the lot
    GpgME::Data data_in(
        reinterpret_cast<char*>(xmlSecBufferGetData(pDsigCtx->transformCtx.result)),
        xmlSecBufferGetSize(pDsigCtx->transformCtx.result), false);
    GpgME::Data data_out;

    SAL_INFO("xmlsecurity.xmlsec.gpg", "Generating signature for: " << xmlSecBufferGetData(pDsigCtx->transformCtx.result));

    // we base64-encode anyway
    rCtx.setArmor(false);
    GpgME::SigningResult sign_res=rCtx.sign(data_in, data_out,
                                            GpgME::Detached);
    off_t result = data_out.seek(0,SEEK_SET);
    (void) result;
    assert(result == 0);
    int len=0, curr=0; char buf;
    while( (curr=data_out.read(&buf, 1)) )
        len += curr;

    if(sign_res.error() || !len)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    // write signed data to xml
    xmlChar* signature = static_cast<xmlChar*>(xmlMalloc(len + 1));
    if(signature == nullptr)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");
    result = data_out.seek(0,SEEK_SET);
    assert(result == 0);
    if( data_out.read(signature, len) != len )
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    // conversion to base64
    xmlChar* signatureEncoded=nullptr;
    if( !(signatureEncoded=xmlSecBase64Encode(reinterpret_cast<xmlSecByte*>(signature), len, 79)) )
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");
    xmlFree(signature);

    // walk xml tree to sign value node - go to children, first is
    // SignedInfo, 2nd is signaturevalue
    cur = xmlSecGetNextElementNode(pNode->children);
    cur = xmlSecGetNextElementNode(cur->next);

    // TODO some assert would be good...
    xmlNodeSetContentLen(cur, signatureEncoded, xmlStrlen(signatureEncoded));
    xmlFree(signatureEncoded);

    aTemplate->setStatus(SecurityOperationStatus_OPERATION_SUCCEEDED);

    // done
    xmlSecDSigCtxDestroy( pDsigCtx ) ;

    //Unregistered the stream/URI binding
    if( xUriBinding.is() )
        xmlUnregisterStreamInputCallbacks() ;

    clearErrorRecorder();
    return aTemplate ;
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
SAL_CALL XMLSignature_GpgImpl::validate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XXMLSecurityContext >& aSecurityCtx
) {
    xmlSecDSigCtxPtr pDsigCtx = nullptr ;
    xmlNodePtr pNode = nullptr ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aSecurityCtx.is() )
        throw RuntimeException() ;

    //Get the xml node
    Reference< XXMLElementWrapper > xElement = aTemplate->getTemplate() ;
    if( !xElement.is() )
        throw RuntimeException() ;

    XMLElementWrapper_XmlSecImpl* pElement =
        dynamic_cast<XMLElementWrapper_XmlSecImpl*>(xElement.get());
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

        SecurityEnvironmentGpg* pSecEnv =
            dynamic_cast<SecurityEnvironmentGpg*>(aEnvironment.get());
        if( pSecEnv == nullptr )
            throw RuntimeException() ;

        // TODO figure out key from pSecEnv!
        // unclear how/where that is transported in nss impl...

        //Create Signature context
        pDsigCtx = xmlSecDSigCtxCreate( nullptr ) ;
        if( pDsigCtx == nullptr )
        {
            clearErrorRecorder();
            return aTemplate;
        }

        // set intended operation to verify - several asserts inside libxmlsec
        // wanting that for digest / transforms
        pDsigCtx->operation = xmlSecTransformOperationVerify;

        // reset status - to be set later
        pDsigCtx->status = xmlSecDSigStatusUnknown;

        // get me a digestible buffer from the SignatureInfo node!
        // -------------------------------------------------------

        // run the transformations - first child node is required to
        // be SignatureInfo
        xmlSecNodeSetPtr nodeset = nullptr;
        xmlNodePtr cur = xmlSecGetNextElementNode(pNode->children);
        // TODO assert that...
        nodeset = xmlSecNodeSetGetChildren(pNode->doc, cur, 1, 0);
        if(nodeset == nullptr)
            throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

        // TODO assert we really have the SignatureInfo here?
        if( xmlSecTransformCtxXmlExecute(&(pDsigCtx->transformCtx), nodeset) < 0 )
            throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

        // Validate the template via gpgme
        GpgME::Context& rCtx=pSecEnv->getGpgContext();

        GpgME::Data data_text(
            reinterpret_cast<char*>(xmlSecBufferGetData(pDsigCtx->transformCtx.result)),
            xmlSecBufferGetSize(pDsigCtx->transformCtx.result), false);

        SAL_INFO("xmlsecurity.xmlsec.gpg", "Validating SignatureInfo: " << xmlSecBufferGetData(pDsigCtx->transformCtx.result));

        // walk xml tree to sign value node - go to children, first is
        // SignedInfo, 2nd is signaturevalue
        cur = xmlSecGetNextElementNode(pNode->children);
        cur = xmlSecGetNextElementNode(cur->next);

        if(!xmlSecCheckNodeName(cur, xmlSecNodeSignatureValue, xmlSecDSigNs))
            throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");
        xmlChar* pSignatureValue=xmlNodeGetContent(cur);
        int nSigSize = xmlSecBase64Decode(pSignatureValue, reinterpret_cast<xmlSecByte*>(pSignatureValue), xmlStrlen(pSignatureValue));
        if( nSigSize < 0)
            throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

        GpgME::Data data_signature(
            reinterpret_cast<char*>(pSignatureValue),
            nSigSize, false);

        GpgME::VerificationResult verify_res=rCtx.verifyDetachedSignature(
            data_signature, data_text);

        // TODO: needs some more error handling, needs checking _all_ signatures
        if( verify_res.isNull() || verify_res.numSignatures() == 0
            // there is at least 1 signature and it is anything else than fully valid
            || ( (verify_res.numSignatures() > 0)
                  && verify_res.signature(0).status().encodedError() > 0 ) )
        {
            // let's try again, but this time import the public key
            // payload (avoiding that in a first cut for being a bit
            // speedier. also prevents all too easy poisoning/sha1
            // fingerprint collision attacks)

            // walk xml tree to PGPData node - go to children, first is
            // SignedInfo, 2nd is signaturevalue, 3rd is KeyInfo
            // 1st child is PGPData, 1st or 2nd grandchild is PGPKeyPacket
            cur = xmlSecGetNextElementNode(pNode->children);
            // TODO error handling
            cur = xmlSecGetNextElementNode(cur->next);
            cur = xmlSecGetNextElementNode(cur->next);
            cur = xmlSecGetNextElementNode(cur->children);
            // check that this is now PGPData
            if(!xmlSecCheckNodeName(cur, xmlSecNodePGPData, xmlSecDSigNs))
                throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");
            // check that this is now PGPKeyPacket
            cur = xmlSecGetNextElementNode(cur->children);
            static const xmlChar xmlSecNodePGPKeyPacket[] = "PGPKeyPacket";
            if(!xmlSecCheckNodeName(cur, xmlSecNodePGPKeyPacket, xmlSecDSigNs))
            {
                // not this one, maybe the next?
                cur = xmlSecGetNextElementNode(cur->next);
                if(!xmlSecCheckNodeName(cur, xmlSecNodePGPKeyPacket, xmlSecDSigNs))
                {
                    // ok, giving up
                    clearErrorRecorder();
                    xmlFree(pSignatureValue);

                    return aTemplate;
                }
            }

            // got a key packet, import & re-validate
            xmlChar* pKeyPacket=xmlNodeGetContent(cur);
            int nKeyLen = xmlSecBase64Decode(pKeyPacket, reinterpret_cast<xmlSecByte*>(pKeyPacket), xmlStrlen(pKeyPacket));
            if( nKeyLen < 0)
                throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

            GpgME::Data data_key(
                reinterpret_cast<char*>(pKeyPacket),
                nKeyLen, false);

            GpgME::ImportResult import_res=rCtx.importKeys(data_key);
            xmlFree(pKeyPacket);

            // and re-run (rewind text and signature streams to position 0)
            (void)data_text.seek(0,SEEK_SET);
            (void)data_signature.seek(0,SEEK_SET);
            verify_res=rCtx.verifyDetachedSignature(data_signature, data_text);

            // TODO: needs some more error handling, needs checking _all_ signatures
            if( verify_res.isNull() || verify_res.numSignatures() == 0
                // there is at least 1 signature and it is anything else than valid
                || ( (verify_res.numSignatures() > 0)
                      && verify_res.signature(0).status().encodedError() > 0 ) )
            {
                clearErrorRecorder();
                xmlFree(pSignatureValue);

                return aTemplate;
            }
        }

        xmlFree(pSignatureValue);

        // now verify digest for all references
        cur = xmlSecGetNextElementNode(pNode->children);
        if( cur != nullptr )
            cur = xmlSecGetNextElementNode(cur->children);
        while( cur != nullptr )
        {
            // some of those children I suppose should be reference elements
            if( xmlSecCheckNodeName(cur, xmlSecNodeReference, xmlSecDSigNs) )
            {
                xmlSecDSigReferenceCtxPtr pDsigRefCtx =
                    xmlSecDSigReferenceCtxCreate(pDsigCtx,
                                                 xmlSecDSigReferenceOriginSignedInfo);
                if(pDsigRefCtx == nullptr)
                    throw RuntimeException();

                // add this one to the list
                if( xmlSecPtrListAdd(&(pDsigCtx->signedInfoReferences),
                                     pDsigRefCtx) < 0 )
                {
                    // TODO resource handling
                    xmlSecDSigReferenceCtxDestroy(pDsigRefCtx);
                    throw RuntimeException();
                }

                if( xmlSecDSigReferenceCtxProcessNode(pDsigRefCtx, cur) < 0 )
                    throw RuntimeException();

                // final check - all good?
                if(pDsigRefCtx->status != xmlSecDSigStatusSucceeded)
                {
                    pDsigCtx->status = xmlSecDSigStatusInvalid;
                    return aTemplate; // TODO - harder error?
                }
            }

            cur = xmlSecGetNextElementNode(cur->next);
        }

        // TODO - also verify manifest (only relevant for ooxml)?
        aTemplate->setStatus(SecurityOperationStatus_OPERATION_SUCCEEDED);

        // done
        xmlSecDSigCtxDestroy( pDsigCtx ) ;
    }

    //Unregistered the stream/URI binding
    if( xUriBinding.is() )
        xmlUnregisterStreamInputCallbacks() ;

    clearErrorRecorder();
    return aTemplate ;
}

/* XServiceInfo */
OUString SAL_CALL XMLSignature_GpgImpl::getImplementationName() {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSignature_GpgImpl::supportsService( const OUString& serviceName) {
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSignature_GpgImpl::getSupportedServiceNames() {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSignature_GpgImpl::impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence<OUString> seqServiceNames { "com.sun.star.xml.crypto.XMLSignature" };
    return seqServiceNames ;
}

OUString XMLSignature_GpgImpl::impl_getImplementationName() {
    return OUString("com.sun.star.xml.security.bridge.xmlsec.XMLSignature_GpgImpl") ;
}

//Helper for registry
Reference< XInterface > XMLSignature_GpgImpl::impl_createInstance( const Reference< XMultiServiceFactory >& ) {
    return Reference< XInterface >( *new XMLSignature_GpgImpl ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
