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
#include "xmlsignature_gpgimpl.hxx"

#include <gpgme.h>
#include <context.h>
#include <key.h>
#include <data.h>
#include <signingresult.h>

#include "xmlsec/xmldocumentwrapper_xmlsecimpl.hxx"
#include "xmlsec/xmlelementwrapper_xmlsecimpl.hxx"
#include "xmlsec/xmlstreamio.hxx"
#include "xmlsec/errorcallback.hxx"

#include "SecurityEnvironment.hxx"
#include "xmlsec-wrapper.h"

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

#if 0
    //Get Keys Manager
    SecurityEnvironmentGpg* pSecEnv =
        dynamic_cast<SecurityEnvironmentGpg*>(aEnvironment.get());
    if( pSecEnv == nullptr )
        throw RuntimeException() ;
#endif

    // TODO pSecEnv is still from nss, roll our own impl there
    // TODO figure out key from pSecEnv!
    // unclear how/where that is transported in nss impl...
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

    //Sign the template via gpgme
    GpgME::initializeLibrary();
    if( GpgME::checkEngine(GpgME::OpenPGP) )
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    GpgME::Context* ctx = GpgME::Context::createForProtocol(GpgME::OpenPGP);
    if( ctx == nullptr )
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    ctx->setKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    GpgME::Error err;
    if( ctx->addSigningKey(ctx->key("0x909BE2575CEDBEA3", err, true)) )
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    // good, ctx is setup now, let's sign the lot
    GpgME::Data data_in(
        reinterpret_cast<char*>(xmlSecBufferGetData(pDsigCtx->transformCtx.result)),
        xmlSecBufferGetSize(pDsigCtx->transformCtx.result), false);
    GpgME::Data data_out;

    SAL_INFO("xmlsecurity.xmlsec.gpg", "Generating signature for: " << xmlSecBufferGetData(pDsigCtx->transformCtx.result));

    GpgME::SigningResult sign_res=ctx->sign(data_in, data_out,
                                            GpgME::Detached);
    // TODO: needs some error handling
    data_out.seek(0,SEEK_SET);
    int len=0, curr=0; char buf;
    while( (curr=data_out.read(&buf, 1)) )
        len += curr;

    // write signed data to xml
    std::vector<unsigned char> buf2(len);
    data_out.seek(0,SEEK_SET);
    if( data_out.read(&buf2[0], len) != len )
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    // walk xml tree to sign value node - go to children, first is
    // SignedInfo, 2nd is signaturevalue
    cur = xmlSecGetNextElementNode(pNode->children);
    cur = xmlSecGetNextElementNode(cur->next);

    // TODO some assert would be good...
    xmlNodeSetContentLen(cur, &buf2[0], len);

    aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED);

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

#if 0
        //Get Keys Manager
        SecurityEnvironmentGpg* pSecEnv =
            dynamic_cast<SecurityEnvironmentGpg*>(aEnvironment.get());
        if( pSecEnv == nullptr )
            throw RuntimeException() ;
#endif

        // TODO pSecEnv is still from nss, roll our own impl there
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
        GpgME::initializeLibrary();
        if( GpgME::checkEngine(GpgME::OpenPGP) )
            throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

        GpgME::Context* ctx = GpgME::Context::createForProtocol(GpgME::OpenPGP);
        if( ctx == nullptr )
            throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

        // good, ctx is setup now, let's validate the lot
        GpgME::Data data_text(
            reinterpret_cast<char*>(xmlSecBufferGetData(pDsigCtx->transformCtx.result)),
            xmlSecBufferGetSize(pDsigCtx->transformCtx.result), false);

        SAL_INFO("xmlsecurity.xmlsec.gpg", "Validating SignatureInfo: " << xmlSecBufferGetData(pDsigCtx->transformCtx.result));

        // walk xml tree to sign value node - go to children, first is
        // SignedInfo, 2nd is signaturevalue
        cur = xmlSecGetNextElementNode(pNode->children);
        cur = xmlSecGetNextElementNode(cur->next);

        // TODO some assert would be good that cur is actually SignatureValue
        xmlChar* pSignatureValue=xmlNodeGetContent(cur);
        GpgME::Data data_signature(
            reinterpret_cast<char*>(pSignatureValue),
            xmlStrlen(pSignatureValue), false);

        GpgME::VerificationResult verify_res=ctx->verifyDetachedSignature(
            data_signature, data_text);

        xmlFree(pSignatureValue);

        // TODO: needs some more error handling, needs checking _all_ signatures
        if( verify_res.isNull() ||
            verify_res.numSignatures() == 0 ||
            verify_res.signature(0).validity() < GpgME::Signature::Full )
        {
            clearErrorRecorder();
            return aTemplate;
        }

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
        aTemplate->setStatus(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED);

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
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return true ;
    }
    return false ;
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
Reference< XInterface > SAL_CALL XMLSignature_GpgImpl::impl_createInstance( const Reference< XMultiServiceFactory >& ) {
    return Reference< XInterface >( *new XMLSignature_GpgImpl ) ;
}

Reference< XSingleServiceFactory > XMLSignature_GpgImpl::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
