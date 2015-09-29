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

#include <xmlsecurity/sigstruct.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/sax/XSecurityController.hpp>
#include <com/sun/star/xml/crypto/sax/XElementStackKeeper.hpp>
#include <com/sun/star/xml/crypto/sax/XSecuritySAXEventKeeper.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultListener.hpp>
#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <rtl/ustrbuf.hxx>

#include <cppuhelper/implbase.hxx>

#include <vector>

/*
 * all error information
 */
#define ERROR_CANNOTCREATEXMLSECURITYCOMPONENT    "Can't create XML security components."
#define ERROR_SAXEXCEPTIONDURINGCREATION          "A SAX exception is throwed during signature creation."
#define ERROR_IOEXCEPTIONDURINGCREATION           "An IO exception is throwed during signature creation."
#define ERROR_EXCEPTIONDURINGCREATION             "An exception is throwed during signature creation."

/*
 * all stringS in signature element
 */
#define TAG_SIGNATURE           "Signature"
#define TAG_SIGNEDINFO          "SignedInfo"
#define TAG_CANONICALIZATIONMETHOD  "CanonicalizationMethod"
#define TAG_SIGNATUREMETHOD     "SignatureMethod"
#define TAG_REFERENCE           "Reference"
#define TAG_TRANSFORMS          "Transforms"
#define TAG_TRANSFORM           "Transform"
#define TAG_DIGESTMETHOD        "DigestMethod"
#define TAG_DIGESTVALUE         "DigestValue"
#define TAG_SIGNATUREVALUE      "SignatureValue"
#define TAG_KEYINFO         "KeyInfo"
#define TAG_X509DATA            "X509Data"
#define TAG_X509ISSUERSERIAL        "X509IssuerSerial"
#define TAG_X509ISSUERNAME      "X509IssuerName"
#define TAG_X509SERIALNUMBER        "X509SerialNumber"
#define TAG_X509CERTIFICATE     "X509Certificate"
#define TAG_OBJECT          "Object"
#define TAG_SIGNATUREPROPERTIES     "SignatureProperties"
#define TAG_SIGNATUREPROPERTY       "SignatureProperty"
#define TAG_TIMESTAMP           "timestamp"
#define TAG_DATE            "date"
//#define TAG_TIME          "time"

#define ATTR_XMLNS          "xmlns"
#define ATTR_ALGORITHM          "Algorithm"
#define ATTR_URI            "URI"
#define ATTR_ID             "Id"
#define ATTR_TARGET         "Target"

#define NSTAG_DC            "dc"

#define NS_XMLDSIG          "http://www.w3.org/2000/09/xmldsig#"
//#define NS_DATETIME           "http://www.ietf.org/rfcXXXX.txt"
#define NS_DC               "http://purl.org/dc/elements/1.1/"

#define ALGO_C14N           "http://www.w3.org/TR/2001/REC-xml-c14n-20010315"
#define ALGO_RSASHA1            "http://www.w3.org/2000/09/xmldsig#rsa-sha1"
#define ALGO_XMLDSIGSHA1        "http://www.w3.org/2000/09/xmldsig#sha1"

#define CHAR_FRAGMENT           "#"
#define CHAR_BLANK          " "


/*
 * status of security related components
 */
#define UNINITIALIZED     0
#define INITIALIZED       1
#define FAILTOINITIALIZED 2

class XSecParser;

class InternalSignatureInformation
{
public:
    SignatureInformation signatureInfor;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XReferenceResolvedListener >
        xReferenceResolvedListener;

    ::std::vector< sal_Int32 > vKeeperIds;

    InternalSignatureInformation(
        sal_Int32 nId,
        com::sun::star::uno::Reference< com::sun::star::xml::crypto::sax::XReferenceResolvedListener >
            xListener)
        :signatureInfor(nId)
    {
        xReferenceResolvedListener = xListener;
    }

    void addReference( sal_Int32 type, const OUString& uri, sal_Int32 keeperId )
    {
        signatureInfor.vSignatureReferenceInfors.push_back(
                SignatureReferenceInformation(type, uri));
        vKeeperIds.push_back( keeperId );
    }
};

class XSecController : public cppu::WeakImplHelper
<
    com::sun::star::xml::crypto::sax::XSecurityController,
    //com::sun::star::beans::XFastPropertySet,
    com::sun::star::xml::crypto::sax::XSAXEventKeeperStatusChangeListener,
    com::sun::star::xml::crypto::sax::XSignatureCreationResultListener,
    com::sun::star::xml::crypto::sax::XSignatureVerifyResultListener
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
 *   NOTES
 *  The XFastPropertySet interface is used to transfer common values to
 *  classes in other module, for instance, the signature id for all
 *  sessions is transferred to xmloff module through this interface.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    friend class XSecParser;

private:
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext> mxCtx;

    /*
     * used to buffer SAX events
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLDocumentWrapper > m_xXMLDocumentWrapper;

    /*
     * the SAX events keeper
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XSecuritySAXEventKeeper > m_xSAXEventKeeper;

    /*
     * the bridge component which creates/verifies signature
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSignature > m_xXMLSignature;

    /*
     * the Security Context
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSecurityContext > m_xSecurityContext;

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
    com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > m_xPreviousNodeOnSAXChain;
    /*
     * whether the previous node can provide an XInitiazlize interface,
     * use this variable in order to typecast the XInterface to the
     * correct interface type.
     */
    bool m_bIsPreviousNodeInitializable;

    /*
     * the next node on the SAX chain.
     * it can always provide an XDocumentHandler interface.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler > m_xNextNodeOnSAXChain;

    /*
     * the ElementStackKeeper is used to reserve the key SAX events.
     * when the SAXEventKeeper is chained on the SAX chain, it need
     * first get all missed key SAX events in order to make sure the
     * DOM tree it buffering has the same structure with the original
     * document.
     *
     * For a given section of a SAX event stream, the key SAX events
     * are the minimal SAX event subset of that section, which,
     * combining with SAX events outside of this section, has the same
     * structure with the original document.
     *
     * For example, sees the following dom fragment:
     *     <A>
     *      <B/>
     *      <C>
     *       <D>
     *        <E/>
     *       </D>
     *      </C>
     *     </A>
     *
     * If we consider the SAX event section from startElement(<A>) to
     * startElement(<D>), then the key SAX events are:
     *
     *    startElement(<A>), startElement(<C>), startElement(<D>)
     *
     * The startElement(<B>) and endElement(<B>) is ignored, because
     * they are unimportant for the tree structure in this section.
     *
     * If we consider the SAX event section from startElement(<D>) to
     * endElement(<A>), the key SAX events are:
     *
     *    startElement(<D>), endElement(<D>), endElement(<C>),
     *    endElement(<A>).
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XElementStackKeeper > m_xElementStackKeeper;

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
    sal_Int32 m_nStatusOfSecurityComponents;

    /*
     * a flag representing whether the SAXEventKeeper need to be
     * on the SAX chain all the time.
     * This flag is used to the situation when creating signature.
     */
    bool m_bIsSAXEventKeeperSticky;

    /*
     * fast property vector
     */
    std::vector< sal_Int32 > m_vFastPropertyIndexs;
    std::vector< com::sun::star::uno::Any > m_vFastPropertyValues;

    /*
      * error message pointer
      */
    const char *m_pErrorMessage;

    /*
     * the XSecParser which is used to parse the signature stream
     */
    XSecParser *m_pXSecParser;

    /*
     * the caller assigned signature id for the next signature in the
     * signature stream
     */
    sal_Int32 m_nReservedSignatureId;

    /*
     * representing whether to verify the current signature
     */
    bool m_bVerifyCurrentSignature;
public:
    /*
     * An xUriBinding is provided to map Uris to XInputStream interfaces.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XUriBinding > m_xUriBinding;

private:

    /*
     * Common methods
     */
    void createXSecComponent( );
    int findSignatureInfor( sal_Int32 nSecurityId ) const;
    bool chainOn( bool bRetrievingLastEvent );
    void chainOff();
    void checkChainingStatus();
    void initializeSAXChain();

    com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream > getObjectInputStream( const OUString& objectURL );

        //sal_Int32 getFastPropertyIndex(sal_Int32 nHandle) const;

    /*
     * For signature generation
     */
    static OUString createId();
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XReferenceResolvedListener > prepareSignatureToWrite(
        InternalSignatureInformation& signatureInfo );

    /*
     * For signature verification
     */
    void addSignature();
    void addReference( const OUString& ouUri);
    void addStreamReference(
        const OUString& ouUri,
        bool isBinary );
    void setReferenceCount() const;

    void setX509IssuerName( OUString& ouX509IssuerName );
    void setX509SerialNumber( OUString& ouX509SerialNumber );
    void setX509Certificate( OUString& ouX509Certificate );
    void setSignatureValue( OUString& ouSignatureValue );
    void setDigestValue( OUString& ouDigestValue );

    void setDate( OUString& ouDate );

    void setId( OUString& ouId );
    void setPropertyId( OUString& ouPropertyId );

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XReferenceResolvedListener > prepareSignatureToRead(
        sal_Int32 nSecurityId );

public:
    explicit XSecController(const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& rxCtx);
    virtual ~XSecController();

    sal_Int32 getNewSecurityId(  );

    void startMission( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XUriBinding >& xUriBinding,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::XXMLSecurityContext >& xSecurityContext );

    void setSAXChainConnector(
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XInitialization >& xInitialization,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XElementStackKeeper >& xElementStackKeeper);

    void clearSAXChainConnector();
    void endMission();

    SignatureInformation    getSignatureInformation( sal_Int32 nSecurityId ) const;
    SignatureInformations   getSignatureInformations() const;

    static void exportSignature(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler,
        const SignatureInformation& signatureInfo );


    /*
     * For signature generation
     */
    void signAStream( sal_Int32 securityId, const OUString& uri, const OUString& objectURL, bool isBinary);


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
        const OUString& ouX509Cert);
    // see the other setX509Certifcate function
    void setX509Certificate(
        sal_Int32 nSecurityId,
        const sal_Int32 nSecurityEnvironmentIndex,
        const OUString& ouX509IssuerName,
        const OUString& ouX509SerialNumber,
        const OUString& ouX509Cert);

    void setDate(
        sal_Int32 nSecurityId,
        const ::com::sun::star::util::DateTime& rDateTime );


    bool WriteSignature(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler );

    /*
     * For signature verification
     */
    void collectToVerify( const OUString& referenceId );
    void addSignature( sal_Int32 nSignatureId );
    com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > createSignatureReader();
    void releaseSignatureReader();

public:
    /* Interface methods */

    /*
     * XSecurityController
     *
     * no method in XSecurityController interface
     */

    /*
     * XFastPropertySet
     */

    /*
     * XSAXEventKeeperStatusChangeListener
     */
    virtual void SAL_CALL blockingStatusChanged( sal_Bool isBlocking )
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL collectionStatusChanged(
        sal_Bool isInsideCollectedElement )
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL bufferStatusChanged( sal_Bool isBufferEmpty )
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /*
     * XSignatureCreationResultListener
     */
    virtual void SAL_CALL signatureCreated( sal_Int32 securityId, com::sun::star::xml::crypto::SecurityOperationStatus nResult )
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /*
     * XSignatureVerifyResultListener
     */
    virtual void SAL_CALL signatureVerified( sal_Int32 securityId, com::sun::star::xml::crypto::SecurityOperationStatus nResult )
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
