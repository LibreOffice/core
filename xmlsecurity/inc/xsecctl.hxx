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

#ifndef INCLUDED_XMLSECURITY_SOURCE_HELPER_XSECCTL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_HELPER_XSECCTL_HXX

#include <svl/sigstruct.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/sax/XSecuritySAXEventKeeper.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultListener.hpp>
#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/embed/XStorage.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>

#include <vector>

#include "xmlsignaturehelper2.hxx"

#define NS_XMLDSIG "http://www.w3.org/2000/09/xmldsig#"
#define NS_DC      "http://purl.org/dc/elements/1.1/"
#define NS_XD      "http://uri.etsi.org/01903/v1.3.2#"
#define NS_MDSSI   "http://schemas.openxmlformats.org/package/2006/digital-signature"
#define NS_LOEXT   "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"

#define ALGO_C14N          "http://www.w3.org/TR/2001/REC-xml-c14n-20010315"
#define ALGO_RSASHA1       "http://www.w3.org/2000/09/xmldsig#rsa-sha1"
#define ALGO_RSASHA256     "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256"
#define ALGO_RSASHA512     "http://www.w3.org/2001/04/xmldsig-more#rsa-sha512"
#define ALGO_ECDSASHA1     "http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha1"
#define ALGO_ECDSASHA256   "http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha256"
#define ALGO_ECDSASHA512   "http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha512"
#define ALGO_XMLDSIGSHA1   "http://www.w3.org/2000/09/xmldsig#sha1"
#define ALGO_XMLDSIGSHA256 "http://www.w3.org/2001/04/xmlenc#sha256"
#define ALGO_XMLDSIGSHA512 "http://www.w3.org/2001/04/xmlenc#sha512"
#define ALGO_RELATIONSHIP  "http://schemas.openxmlformats.org/package/2006/RelationshipTransform"

class XSecParser;
class XMLDocumentWrapper_XmlSecImpl;
class SAXEventKeeperImpl;
class XMLSignatureHelper;
namespace svl { namespace crypto { enum class SignatureMethodAlgorithm; } }

class InternalSignatureInformation
{
public:
    SignatureInformation signatureInfor;

    css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener > xReferenceResolvedListener;

    ::std::vector< sal_Int32 > vKeeperIds;

    InternalSignatureInformation(
        sal_Int32 nId,
        css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener > const & xListener)
        :signatureInfor(nId)
    {
        xReferenceResolvedListener = xListener;
    }

    void addReference( SignatureReferenceType type, sal_Int32 digestID, const OUString& uri, sal_Int32 keeperId )
    {
        signatureInfor.vSignatureReferenceInfors.push_back(
                SignatureReferenceInformation(type, digestID, uri));
        vKeeperIds.push_back( keeperId );
    }
};

class XSecController : public cppu::WeakImplHelper
<
    css::xml::crypto::sax::XSAXEventKeeperStatusChangeListener,
    css::xml::crypto::sax::XSignatureCreationResultListener,
    css::xml::crypto::sax::XSignatureVerifyResultListener
>
/****** XSecController.hxx/CLASS XSecController *******************************
 *
 *   NAME
 *  XSecController -- the xml security framework controller
 *
 *   FUNCTION
 *  Controls the whole xml security framework to create signatures or to
 *  verify signatures.
 *
 ******************************************************************************/
{
    friend class XSecParser;
    friend class OOXMLSecParser;

private:
    css::uno::Reference< css::uno::XComponentContext> mxCtx;

    /*
     * used to buffer SAX events
     */
    rtl::Reference<XMLDocumentWrapper_XmlSecImpl> m_xXMLDocumentWrapper;

    /*
     * the SAX events keeper
     */
    rtl::Reference<SAXEventKeeperImpl> m_xSAXEventKeeper;

    /*
     * the bridge component which creates/verifies signature
     */
    css::uno::Reference< css::xml::crypto::XXMLSignature > m_xXMLSignature;

    /*
     * the Security Context
     */
    css::uno::Reference< css::xml::crypto::XXMLSecurityContext > m_xSecurityContext;

    /*
     * the security id incrementer, in order to make any security id unique
     * to the SAXEventKeeper.
     * Because each XSecController has its own SAXEventKeeper, so this variable
     * is not necessary to be static.
     */
    sal_Int32 m_nNextSecurityId;

    /*
     * Signature information
     */
    std::vector< InternalSignatureInformation > m_vInternalSignatureInformations;

    /*
     * the previous node on the SAX chain.
     * The reason that use a Reference<XInterface> type variable
     * is that the previous components are different when exporting
     * and importing, and there is no other common interface they
     * can provided.
     */
    css::uno::Reference< css::uno::XInterface > m_xPreviousNodeOnSAXChain;
    /*
     * whether the previous node can provide an XInitialize interface,
     * use this variable in order to typecast the XInterface to the
     * correct interface type.
     */
    bool m_bIsPreviousNodeInitializable;

    /*
     * a flag representing whether the SAXEventKeeper is now on the
     * SAX chain.
     */
    bool m_bIsSAXEventKeeperConnected;

    /*
     * a flag representing whether it is collecting some element,
     * which means that the SAXEventKeeper can't be chained off the
     * SAX chain.
     */
    bool m_bIsCollectingElement;

    /*
     * a flag representing whether the SAX event stream is blocking,
     * which also means that the SAXEventKeeper can't be chained off
     * the SAX chain.
     */
    bool m_bIsBlocking;

    /*
     * a flag representing the current status of security related
     * components.
     */

    /*
     * status of security related components
     */
    enum class InitializationState { UNINITIALIZED, INITIALIZED, FAILTOINITIALIZED } m_eStatusOfSecurityComponents;

    /*
     * a flag representing whether the SAXEventKeeper need to be
     * on the SAX chain all the time.
     * This flag is used to the situation when creating signature.
     */
    bool m_bIsSAXEventKeeperSticky;

    /*
     * the XSecParser which is used to parse the signature stream
     */
    css::uno::Reference<css::xml::sax::XDocumentHandler> m_xSecParser;

    /*
     * the caller assigned signature id for the next signature in the
     * signature stream
     */
    sal_Int32 m_nReservedSignatureId;

    /*
     * representing whether to verify the current signature
     */
    bool m_bVerifyCurrentSignature;

    /*
     * An xUriBinding is provided to map Uris to XInputStream interfaces.
     */
    rtl::Reference<UriBindingHelper> m_xUriBinding;

private:

    /*
     * Common methods
     */
    void createXSecComponent( );
    int findSignatureInfor( sal_Int32 nSecurityId ) const;
    bool chainOn();
    void chainOff();
    void checkChainingStatus();
    void initializeSAXChain();

    css::uno::Reference< css::io::XInputStream > getObjectInputStream( const OUString& objectURL );

        //sal_Int32 getFastPropertyIndex(sal_Int32 nHandle) const;

    /*
     * For signature generation
     */
    static OUString createId();
    css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener > prepareSignatureToWrite(
        InternalSignatureInformation& signatureInfo,
        sal_Int32 nStorageFormat,
        bool bXAdESCompliantIfODF );

    /*
     * For signature verification
     */
    void addSignature();
    /// Sets algorithm from <SignatureMethod Algorithm="...">.
    void setSignatureMethod(svl::crypto::SignatureMethodAlgorithm eAlgorithmID);
    void switchGpgSignature();
    void addReference(
        const OUString& ouUri,
        sal_Int32 nDigestID );
    void addStreamReference(
        const OUString& ouUri,
        bool isBinary,
        sal_Int32 nDigestID );
    void setReferenceCount() const;

    void setX509IssuerName( OUString const & ouX509IssuerName );
    void setX509SerialNumber( OUString const & ouX509SerialNumber );
    void setX509Certificate( OUString const & ouX509Certificate );
    void setSignatureValue( OUString const & ouSignatureValue );
    void setDigestValue( sal_Int32 nDigestID, OUString const & ouDigestValue );
    void setGpgKeyID( OUString const & ouKeyID );
    void setGpgCertificate( OUString const & ouGpgCert );
    void setGpgOwner( OUString const & ouGpgOwner );

    void setDate( OUString const & ouDate );
    void setDescription(const OUString& rDescription);
    void setCertDigest(const OUString& rCertDigest);
    void setValidSignatureImage(const OUString& rValidSigImg);
    void setInvalidSignatureImage(const OUString& rInvalidSigImg);
    void setSignatureLineId(const OUString& rSignatureLineId);

public:
    void setSignatureBytes(const css::uno::Sequence<sal_Int8>& rBytes);

private:
    void setId( OUString const & ouId );
    void setPropertyId( OUString const & ouPropertyId );

    css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener > prepareSignatureToRead(
        sal_Int32 nSecurityId );

public:
    explicit XSecController(const css::uno::Reference<css::uno::XComponentContext>& rxCtx);
    virtual ~XSecController() override;

    sal_Int32 getNewSecurityId(  );

    void startMission(const rtl::Reference<UriBindingHelper>& xUriBinding, const css::uno::Reference<css::xml::crypto::XXMLSecurityContext>& xSecurityContext);

    void setSAXChainConnector(const css::uno::Reference< css::lang::XInitialization >& xInitialization);

    void clearSAXChainConnector();
    void endMission();

    SignatureInformation    getSignatureInformation( sal_Int32 nSecurityId ) const;
    SignatureInformations   getSignatureInformations() const;

    static void exportSignature(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xDocumentHandler,
        const SignatureInformation& signatureInfo,
        bool bXAdESCompliantIfODF );


    /*
     * For signature generation
     */
    void signAStream( sal_Int32 securityId, const OUString& uri, bool isBinary, bool bXAdESCompliantIfODF);


    /** sets data that describes the certificate.

        It is absolutely necessary that the parameter ouX509IssuerName is set. It contains
        the base64 encoded certificate, which is DER encoded. The XMLSec needs it to find
        the private key. Although issuer name and certificate should be sufficient to identify
        the certificate the implementation in XMLSec is broken, both for Windows and mozilla.
        The reason is that they use functions to find the certificate which take as parameter
        the DER encoded ASN.1 issuer name. The issuer name is a DName, where most attributes
        are of type DirectoryName, which is a choice of 5 string types. This information is
        not contained in the issuer string and while it is converted to the ASN.1 name the
        conversion function must assume a particular type, which is often wrong. For example,
        the Windows function CertStrToName will use a T.61 string if the string does not contain
        special characters. So if the certificate uses simple characters but encodes the
        issuer attributes in Utf8, then CertStrToName will use T.61. The resulting DER encoded
        ASN.1 name now contains different bytes which indicate the string type. The functions
        for finding the certificate apparently use memcmp - hence they fail to find the
        certificate.
     */
    void setX509Certificate(
        sal_Int32 nSecurityId,
        const OUString& ouX509IssuerName,
        const OUString& ouX509SerialNumber,
        const OUString& ouX509Cert,
        const OUString& ouX509CertDigest,
        svl::crypto::SignatureMethodAlgorithm eAlgorithmID);

    void addEncapsulatedX509Certificate(const OUString& rEncapsulatedX509Certificate);

    void setGpgCertificate(
        sal_Int32 nSecurityId,
        const OUString& ouCertDigest,
        const OUString& ouCert,
        const OUString& ouOwner);

    void setDate(
        sal_Int32 nSecurityId,
        const css::util::DateTime& rDateTime );
    void setDescription(sal_Int32 nSecurityId, const OUString& rDescription);
    void setSignatureLineId(sal_Int32 nSecurityId, const OUString& rSignatureLineId);
    void
    setSignatureLineValidGraphic(sal_Int32 nSecurityId,
                                 const css::uno::Reference<css::graphic::XGraphic>& xValidGraphic);
    void setSignatureLineInvalidGraphic(
        sal_Int32 nSecurityId, const css::uno::Reference<css::graphic::XGraphic>& xInvalidGraphic);

    bool WriteSignature(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xDocumentHandler,
        bool bXAdESCompliantIfODF);

    /*
     * For signature verification
     */
    void collectToVerify( const OUString& referenceId );
    void addSignature( sal_Int32 nSignatureId );
    css::uno::Reference< css::xml::sax::XDocumentHandler > const & createSignatureReader(XMLSignatureHelper& rXMLSignatureHelper, sal_Int32 nType = 0);
    void releaseSignatureReader();

public:
    /* Interface methods */

    /*
     * XSAXEventKeeperStatusChangeListener
     */
    virtual void SAL_CALL blockingStatusChanged( sal_Bool isBlocking ) override;
    virtual void SAL_CALL collectionStatusChanged(
        sal_Bool isInsideCollectedElement ) override;
    virtual void SAL_CALL bufferStatusChanged( sal_Bool isBufferEmpty ) override;

    /*
     * XSignatureCreationResultListener
     */
    virtual void SAL_CALL signatureCreated( sal_Int32 securityId, css::xml::crypto::SecurityOperationStatus nResult ) override;

    /*
     * XSignatureVerifyResultListener
     */
    virtual void SAL_CALL signatureVerified( sal_Int32 securityId, css::xml::crypto::SecurityOperationStatus nResult ) override;

    /// Writes XML elements inside a single OOXML signature's <Signature> element.
    bool WriteOOXMLSignature(const css::uno::Reference<css::embed::XStorage>& xRootStorage, const css::uno::Reference<css::xml::sax::XDocumentHandler>& xDocumentHandler);
    /// Exports an OOXML signature, called by WriteOOXMLSignature().
    void exportOOXMLSignature(const css::uno::Reference<css::embed::XStorage>& xRootStorage, const css::uno::Reference<css::xml::sax::XDocumentHandler>& xDocumentHandler, const SignatureInformation& rInformation);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
