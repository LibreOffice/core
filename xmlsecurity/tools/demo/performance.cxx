/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include <stdio.h>
#include <string.h>

#include <rtl/ustring.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <iostream>
#include <fstream>

#include <util.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <cppuhelper/implbase4.hxx>

#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSecuritySAXEventKeeper.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/xml/csax/XMLAttribute.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/SecurityOperationStatus.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeBroadcaster.hpp>
#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>

#include <xmloff/attrlist.hxx>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <osl/time.h>


#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef INCLUDED_STACK
#include <stack>
#define INCLUDED_STACK
#endif

/* xml security framework components */
#define SIGNATURECREATOR_COMPONENT  "com.sun.star.xml.crypto.sax.SignatureCreator"
#define SIGNATUREVERIFIER_COMPONENT "com.sun.star.xml.crypto.sax.SignatureVerifier"
#define JAVAFLATFILTER_COMPONENT    "com.sun.star.xml.crypto.eval.JavaFlatFilter"
#define SAXEVENTKEEPER_COMPONENT    "com.sun.star.xml.crypto.sax.SAXEventKeeper"

/* java based bridge components */
#define SEINITIALIZER_JAVA_COMPONENT      "com.sun.star.xml.security.bridge.jxsec.SEInitializer_JxsecImpl"
#define XMLSIGNATURE_JAVA_COMPONENT       "com.sun.star.xml.security.bridge.jxsec.XMLSignature_JxsecImpl"
#define XMLDOCUMENTWRAPPER_JAVA_COMPONENT "com.sun.star.xml.security.bridge.jxsec.XMLDocumentWrapper_JxsecImpl"

/* c based bridge components */
#define SEINITIALIZER_C_COMPONENT "com.sun.star.xml.crypto.SEInitializer"
#define XMLSIGNATURE_C_COMPONENT "com.sun.star.xml.crypto.XMLSignature"
#define XMLDOCUMENT_C_COMPONENT "com.sun.star.xml.wrapper.XMLDocumentWrapper"

/* security related elements and attributes */
#define SIGNATURE_STR       "Signature"
#define REFERENCE_STR       "Reference"
#define SIGNEDINFO_STR      "SignedInfo"
#define KEYINFO_STR         "KeyInfo"
#define KEYVALUE_STR        "KeyValue"
#define KEYNAME_STR         "KeyName"
#define X509DATA_STR        "X509Data"
#define ENCRYPTEDKEY_STR    "EncryptedKey"
#define RETRIEVALMETHOD_STR "RetrievalMethod"
#define OTHER_ELEMENT_STR   "OTHER_ELEMENT_STR"
#define REFNUM_ATTR_STR     "refNum"
#define URI_ATTR_STR        "URI"


#define RTL_ASCII_USTRINGPARAM( asciiStr ) asciiStr, strlen( asciiStr ), RTL_TEXTENCODING_ASCII_US

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssb = com::sun::star::beans;
namespace cssi = com::sun::star::io;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxs = com::sun::star::xml::sax;
namespace cssxw = com::sun::star::xml::wrapper;
namespace cssxcsax = com::sun::star::xml::csax;


using namespace ::com::sun::star;


class XSecTester;

/*
 * The XSecTester class is a C++ version of SecurityFramworkController.java
 *
 */

class SecurityEntity
{
private:
    static int m_nNextSecurityId;
    rtl::OUString m_ouKeyURI;

protected:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > mxMSF;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XReferenceResolvedListener >
        m_xReferenceListener;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XSecuritySAXEventKeeper >
        m_xSAXEventKeeper;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSecurityContext >
        m_xXMLSecurityContext;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSignature >
        m_xXMLSignature;

    int m_nSecurityId;

private:
    int getNextSecurityId() const;

protected:
    SecurityEntity(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSecuritySAXEventKeeper >&
            xSAXEventKeeper,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::XXMLSecurityContext >&
            xXMLSecurityContext,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::XXMLSignature >&
            xXMLSignature,
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >&
            rsMSF);

public:
    void setKeyId(int nId);

    int getSecurityId() const;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XReferenceResolvedListener >
        getReferenceListener() const;

    bool setKey( const rtl::OUString& ouUri, bool bIsExporting );

    void setKeyURI(const rtl::OUString& ouUri);

    bool endMission();
};


class SignatureEntity : public SecurityEntity
{
private:
    std::vector< rtl::OUString > m_vReferenceIds;
    int m_nSignatureElementCollectorId;

    bool hasReference(const rtl::OUString& ouUri) const;

public:
    SignatureEntity(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSecuritySAXEventKeeper >&
            xSAXEventKeeper,
        bool bIsExporting,
        XSecTester* pListener,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::XXMLSecurityContext >&
            xXMLSecurityContext,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::XXMLSignature >&
            xXMLSignature,
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >&
            rsMSF);
    ~SignatureEntity(){};

    void setReferenceNumber() const;
    bool setReference( const rtl::OUString& ouUri, bool bIsExporting ) const;
    void addReferenceURI( const rtl::OUString& ouUri );
};

struct AncestorEvent
{
    AncestorEvent( sal_Int32 nAttrNum ):aAttributeList(nAttrNum){};

    bool bIsStartElement;
    rtl::OUString ouName;

    com::sun::star::uno::Sequence<
        com::sun::star::xml::csax::XMLAttribute >
        aAttributeList;
};

class XSecTester : public cppu::WeakImplHelper4
<
    com::sun::star::xml::crypto::sax::XSignatureCreationResultListener,
    com::sun::star::xml::crypto::sax::XSignatureVerifyResultListener,
    com::sun::star::xml::crypto::sax::XSAXEventKeeperStatusChangeListener,
    com::sun::star::xml::sax::XDocumentHandler
>
{
private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

    sal_Int32 m_nTotalSignatureNumber;
    sal_Int32 m_nSuccessfulSignatureNumber;

    com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler >
        m_xExportHandler;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XSecuritySAXEventKeeper >
        m_xSAXEventKeeper;

    com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLDocumentWrapper >
        m_xXMLDocumentWrapper;

    com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler >
        m_xOutputHandler;

    com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XParser >
        m_xSaxParser;

    std::stack< void* > m_stCurrentPath;
    std::stack< bool > m_stCurrentPathType;

    std::vector< AncestorEvent* > m_vAncestorEvents;
    std::vector< SignatureEntity* > m_vSignatureList;

    std::vector< rtl::OUString > m_vUnsolvedReferenceURIs;
    std::vector< int > m_vUnsolvedReferenceKeeperIds;
    std::vector< int > m_vUnsolvedReferenceRefNums;

    bool m_bIsExporting;
    bool m_bIsBlocking;

    bool m_bIsInsideCollectedElement;
    bool m_bIsSAXEventKeeperOnTheSAXChain;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSecurityContext >
        m_xXMLSecurityContext;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSignature >
        m_xXMLSignature;

    rtl::OUString m_ouJavaCryptokenDir;
    rtl::OUString m_ouCCryptokenDir;
    rtl::OUString m_ouXMLDocumentWrapperComponentName;

private:
    com::sun::star::uno::Reference<
        com::sun::star::io::XOutputStream >
        createOutputStream( const rtl::OUString& ouFile );

    rtl::OUString parseFile(
        const rtl::OUString& ouInputFileName,
        const rtl::OUString& ouOutputFileName,
        bool bIsExporting,
        bool bIsJavaBased);

    void changeOutput();

    bool foundSecurityRelated();

    void findKeyOrReference(SecurityEntity* pSecurityEntity, const rtl::OUString& ouUri, bool bIsFindKey);

    bool checkSecurityElement(
        const rtl::OUString& ouLocalName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList>& xAttribs);

    void checkReference(
        const rtl::OUString& ouLocalName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList>& xAttribs,
        const rtl::OUString& ouId);

    void endMission();

    void addStartAncestorEvent(
        const rtl::OUString& ouName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList>& xAttribs);

    void addEndAncestorEvent( const rtl::OUString& ouName );

    void flushAncestorEvents(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler);

    void XSecTester::sendAncestorStartElementEvent(
        const rtl::OUString& ouName,
        const com::sun::star::uno::Sequence<
            com::sun::star::xml::csax::XMLAttribute >& xAttrList,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler) const;

    void XSecTester::sendAncestorEndElementEvent(
        const rtl::OUString& ouName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler) const;

    std::vector< AncestorEvent* >::const_iterator XSecTester::checkAncestorStartElementEvent(
        const std::vector< AncestorEvent* >::const_iterator& ii,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler) const;

public:
    XSecTester(const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxMSF)
        :mxMSF( rxMSF ){};
    virtual ~XSecTester(){};

    /* XSignatureCreationResultListener */
    virtual void SAL_CALL signatureCreated(
        sal_Int32 securityId,
        com::sun::star::xml::crypto::SecurityOperationStatus creationResult )
        throw (com::sun::star::uno::RuntimeException);

    /* XSignatureVerifyResultListener */
    virtual void SAL_CALL signatureVerified(
        sal_Int32 securityId,
        com::sun::star::xml::crypto::SecurityOperationStatus verifyResult )
        throw (com::sun::star::uno::RuntimeException);

    /* XSAXEventKeeperStatusChangeListener */
    virtual void SAL_CALL blockingStatusChanged( sal_Bool isBlocking )
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL collectionStatusChanged(
        sal_Bool isInsideCollectedElement )
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL bufferStatusChanged( sal_Bool isBufferEmpty )
        throw (com::sun::star::uno::RuntimeException);

    /* XXMLSecTester */
    virtual rtl::OUString SAL_CALL transfer_without_sec(
        const rtl::OUString& inputFileName,
        const rtl::OUString& outputFileName,
        sal_Bool isBridgeInvolved)
        throw (com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL export_xml(
        const rtl::OUString& inputFileName,
        const rtl::OUString& outputFileName,
        sal_Bool isJavaBased)
        throw (com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL import_xml(
        const rtl::OUString& inputFileName,
        const rtl::OUString& outputFileName,
        sal_Bool isJavaBased)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setCryptoDir(
        const rtl::OUString & javaDirName,
        const rtl::OUString & cDirName)
        throw (com::sun::star::uno::RuntimeException);

    /* XDocumentHandler */
    virtual void SAL_CALL endDocument()
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startDocument()
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters(const class rtl::OUString&)
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction(const rtl::OUString&, const rtl::OUString&)
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace(const rtl::OUString&)
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startElement(
        const rtl::OUString&,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >&)
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endElement(const rtl::OUString&)
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XLocator >&)
        throw (com::sun::star::uno::RuntimeException);
};

rtl::OUString XSecTester::parseFile(
    const rtl::OUString& ouInputFileName,
    const rtl::OUString& ouOutputFileName,
    bool bIsExporting,
    bool bIsJavaBased)
{
    rtl::OUString ouMessage;

    cssu::Reference<cssi::XInputStream> xInputStream = OpenInputStream(ouInputFileName);

    if (xInputStream != NULL )
    {
        /* initialization */
        rtl::OUString SEInitializer_comp;
        rtl::OUString XMLSignature_comp;
        rtl::OUString tokenPath;
        cssu::Reference < cssxc::XSEInitializer > xSEInitializer;

        if (bIsJavaBased)
        {
            SEInitializer_comp = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SEINITIALIZER_JAVA_COMPONENT ));
            XMLSignature_comp = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XMLSIGNATURE_JAVA_COMPONENT));
            m_ouXMLDocumentWrapperComponentName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XMLDOCUMENTWRAPPER_JAVA_COMPONENT ));
            tokenPath = m_ouJavaCryptokenDir;
        }
        else
        {
            SEInitializer_comp = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SEINITIALIZER_C_COMPONENT ));
            XMLSignature_comp = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XMLSIGNATURE_C_COMPONENT));
            m_ouXMLDocumentWrapperComponentName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XMLDOCUMENT_C_COMPONENT ));
            tokenPath = m_ouCCryptokenDir;
        }

        xSEInitializer = cssu::Reference < cssxc::XSEInitializer > (
             mxMSF->createInstance( SEInitializer_comp ),
             cssu::UNO_QUERY );

        m_xXMLSignature = cssu::Reference<cssxc::XXMLSignature> (
            mxMSF->createInstance( XMLSignature_comp ),
            cssu::UNO_QUERY );

        if ( xSEInitializer.is() && m_xXMLSignature.is())
        {
            /* create SAX Parser */
            const rtl::OUString sSaxParser (
                RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser") );
            m_xSaxParser = cssu::Reference < cssxs::XParser > ( mxMSF->createInstance( sSaxParser ), cssu::UNO_QUERY );

            /* create SAX Writer */
            const rtl::OUString sSaxWriter (
                RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer") );
            cssu::Reference < cssi::XActiveDataSource > xSaxWriter
                ( mxMSF->createInstance( sSaxWriter ), cssu::UNO_QUERY );

            cssu::Reference< cssi::XOutputStream > xOutputStream = OpenOutputStream(ouOutputFileName);
            xSaxWriter->setOutputStream( xOutputStream );

            cssxs::InputSource aInput;
            aInput.sSystemId = ouInputFileName;
            aInput.aInputStream = xInputStream;

            cssu::Reference < cssxs::XDocumentHandler > xSaxWriterHandler( xSaxWriter, cssu::UNO_QUERY);

            m_xXMLSecurityContext =
                xSEInitializer->createSecurityContext(tokenPath);

            m_bIsExporting = bIsExporting;
            m_xExportHandler = xSaxWriterHandler;
            m_xOutputHandler = xSaxWriterHandler;

            m_xXMLDocumentWrapper = NULL;
            m_xSAXEventKeeper = NULL;
            m_bIsSAXEventKeeperOnTheSAXChain = false;

            m_bIsBlocking = false;
            m_bIsInsideCollectedElement = false;

            OSL_ASSERT(m_vSignatureList.size() == 0);
            OSL_ASSERT(m_vUnsolvedReferenceURIs.size() == 0);
            OSL_ASSERT(m_vUnsolvedReferenceKeeperIds.size() == 0);
            OSL_ASSERT(m_vUnsolvedReferenceRefNums.size() == 0);
            OSL_ASSERT(m_stCurrentPath.empty());
            OSL_ASSERT(m_stCurrentPathType.empty());
            OSL_ASSERT(m_vAncestorEvents.empty());

            changeOutput();

            /* foundSecurityRelated(); */

            /* Begin to parse */
            TimeValue startTime, endTime;
            osl_getSystemTime( &startTime );

            xSaxWriterHandler->startDocument();

            if (m_bIsExporting)
            {
                m_xSaxParser->setDocumentHandler(this);
                m_xSaxParser->parseStream(aInput);
            }
            else
            {
                m_xSaxParser->setDocumentHandler(this);
                m_xSaxParser->parseStream(aInput);
            }

            endMission();
            xSaxWriterHandler->endDocument();

            osl_getSystemTime( &endTime );

            flushAncestorEvents( NULL );

            // Bug in SAXWriter, done in endDocument()
            // xOutputStream->closeOutput();
            xInputStream->closeInput();


            /*
             * Free the security context
             */
            xSEInitializer->freeSecurityContext(m_xXMLSecurityContext);
            m_xXMLSecurityContext = NULL;

            /* Calculate the time */
            double diff = ((double)((endTime.Nanosec + endTime.Seconds*1000000000.0)
                    - (startTime.Nanosec + startTime.Seconds*1000000000.0))) /
                ((double)1000000000.0);

            char buf[32];
            sprintf(buf, "%.2f", diff);
            ouMessage += rtl::OUString(RTL_ASCII_USTRINGPARAM(buf));
        }
        else
        {
            ouMessage += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("N/A"));
        }

    }
    else
    {
        ouMessage += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
    }

    return ouMessage;
}

/* XSignatureCreationResultListener */
void SAL_CALL XSecTester::signatureCreated(
    sal_Int32 securityId,
    cssxc::SecurityOperationStatus creationResult )
    throw (cssu::RuntimeException)
{
    m_nTotalSignatureNumber++;
    if (creationResult == cssxc::SecurityOperationStatus_OPERATION_SUCCEEDED)
    {
        m_nSuccessfulSignatureNumber++;
    }
}

/* XSignatureVerifyResultListener */
void SAL_CALL XSecTester::signatureVerified(
    sal_Int32 securityId,
    cssxc::SecurityOperationStatus verifyResult )
    throw (cssu::RuntimeException)
{
    m_nTotalSignatureNumber++;
    if (verifyResult == cssxc::SecurityOperationStatus_OPERATION_SUCCEEDED)
    {
        m_nSuccessfulSignatureNumber++;
    }
}

/* XSAXEventKeeperStatusChangeListener */
void SAL_CALL XSecTester::blockingStatusChanged( sal_Bool isBlocking )
    throw (cssu::RuntimeException)
{
    this->m_bIsBlocking = isBlocking;
}

void SAL_CALL XSecTester::collectionStatusChanged( sal_Bool isInsideCollectedElement )
    throw (cssu::RuntimeException)
{
    this->m_bIsInsideCollectedElement = isInsideCollectedElement;

    if ( !m_bIsInsideCollectedElement && !m_bIsBlocking)
    {
        m_bIsSAXEventKeeperOnTheSAXChain = false;
    }
    else
    {
        m_bIsSAXEventKeeperOnTheSAXChain = true;
    }
    changeOutput();
}

void SAL_CALL XSecTester::bufferStatusChanged( sal_Bool isBufferEmpty )
    throw (cssu::RuntimeException)
{
    if (isBufferEmpty)
    {
        m_xXMLDocumentWrapper = NULL;

        m_xSAXEventKeeper = NULL;
        m_bIsSAXEventKeeperOnTheSAXChain = false;
        changeOutput();
    }
}

/* XXMLSecTester */
rtl::OUString SAL_CALL XSecTester::export_xml( const rtl::OUString& inputFileName, const rtl::OUString& outputFileName, sal_Bool isJavaBased)
    throw (cssu::RuntimeException)
{
    rtl::OUString ouMessage;

    m_nTotalSignatureNumber = 0;
    m_nSuccessfulSignatureNumber = 0;

    ouMessage += parseFile(inputFileName, outputFileName, sal_True, isJavaBased);

    rtl::OUString ouRemark = rtl::OUString::valueOf(m_nSuccessfulSignatureNumber) +
        rtl::OUString(RTL_ASCII_USTRINGPARAM( "/" ))
        + rtl::OUString::valueOf(m_nTotalSignatureNumber);
    ouMessage += rtl::OUString(RTL_ASCII_USTRINGPARAM("\t")) + ouRemark;

    return ouMessage;
}

rtl::OUString SAL_CALL XSecTester::import_xml( const rtl::OUString& inputFileName, const rtl::OUString& outputFileName, sal_Bool isJavaBased)
    throw (cssu::RuntimeException)
{
    rtl::OUString ouMessage;

    m_nTotalSignatureNumber = 0;
    m_nSuccessfulSignatureNumber = 0;

    ouMessage += parseFile(inputFileName, outputFileName, sal_False, isJavaBased);

    rtl::OUString ouRemark = rtl::OUString::valueOf(m_nSuccessfulSignatureNumber) +
        rtl::OUString(RTL_ASCII_USTRINGPARAM( "/" ))
        + rtl::OUString::valueOf(m_nTotalSignatureNumber);
    ouMessage += rtl::OUString(RTL_ASCII_USTRINGPARAM("\t")) + ouRemark;

    return ouMessage;
}

rtl::OUString SAL_CALL XSecTester::transfer_without_sec(
    const rtl::OUString& inputFileName,
    const rtl::OUString& outputFileName,
    sal_Bool isBridgeInvolved)
    throw (cssu::RuntimeException)
{
    rtl::OUString ouMessage;

    cssu::Reference< cssi::XInputStream > xInputStream = OpenInputStream(inputFileName);

    if (xInputStream != NULL )
    {
        /* create SAX Parser */
        const rtl::OUString sSaxParser (
            RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser") );
        m_xSaxParser = cssu::Reference < cssxs::XParser > ( mxMSF->createInstance( sSaxParser ), cssu::UNO_QUERY );

        /* create SAX Writer */
        const rtl::OUString sSaxWriter (
            RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer") );
        cssu::Reference < cssi::XActiveDataSource > xSaxWriter
            ( mxMSF->createInstance( sSaxWriter ), cssu::UNO_QUERY );
        cssu::Reference < cssxs::XDocumentHandler > xSaxWriterHandler(
            xSaxWriter, cssu::UNO_QUERY);

        if (!isBridgeInvolved)
        {
            /* connect the SAX Parser and the SAX Writer */
            m_xSaxParser->setDocumentHandler ( xSaxWriterHandler );
        }
        else
        {
            /* create Java Flat Filter */
            const rtl::OUString sJavaFlatFilter(
                RTL_CONSTASCII_USTRINGPARAM( JAVAFLATFILTER_COMPONENT ) );
            cssu::Reference < cssxs::XParser > xJavaFilterParser
                ( mxMSF->createInstance( sJavaFlatFilter ), cssu::UNO_QUERY );
            cssu::Reference < cssxs::XDocumentHandler > xJavaFilterHandler(
                xJavaFilterParser, cssu::UNO_QUERY );

            if ( !xJavaFilterParser.is() )
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NO JAVA"));

            /* connect the SAX Parser, the Java Flat Filter and the SAX Writer */
            xJavaFilterParser->setDocumentHandler( xSaxWriterHandler );
            m_xSaxParser->setDocumentHandler ( xJavaFilterHandler );
        }


        /* set output stream */
        cssu::Reference< cssi::XOutputStream > xOutputStream =
                OpenOutputStream(outputFileName);
        xSaxWriter->setOutputStream( xOutputStream );

        /* prepare input stream */
        cssxs::InputSource aInput;
        aInput.sSystemId = inputFileName;
        aInput.aInputStream = xInputStream;

        TimeValue startTime, endTime;
        osl_getSystemTime( &startTime );

        m_xSaxParser->parseStream ( aInput );

        // xOutputStream->closeOutput();
        xInputStream->closeInput();

        osl_getSystemTime( &endTime );

        double diff = ((double)((endTime.Nanosec + endTime.Seconds*1000000000.0)
                 - (startTime.Nanosec + startTime.Seconds*1000000000.0)))/((double)1000000000.0);
        char buf[32];
        sprintf(buf, "%.2f", diff);
        ouMessage += rtl::OUString(RTL_ASCII_USTRINGPARAM(buf));
    }

    return ouMessage;
}

void SAL_CALL XSecTester::setCryptoDir(const rtl::OUString & javaDirName, const rtl::OUString & cDirName)
    throw (cssu::RuntimeException)
{
    m_ouJavaCryptokenDir = javaDirName;
    m_ouCCryptokenDir = cDirName;
}


cssu::Reference< cssu::XInterface > SAL_CALL XSecTester_createInstance(
    const cssu::Reference< cssl::XMultiServiceFactory > & rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new XSecTester( rSMgr );
}

int SecurityEntity::m_nNextSecurityId = 1;

SecurityEntity::SecurityEntity(
    const cssu::Reference<cssxc::sax::XSecuritySAXEventKeeper>& xSAXEventKeeper,
    const cssu::Reference<cssxc::XXMLSecurityContext>& xXMLSecurityContext,
    const cssu::Reference<cssxc::XXMLSignature>& xXMLSignature,
    const cssu::Reference< cssl::XMultiServiceFactory > &rsMSF)
    :m_xSAXEventKeeper(xSAXEventKeeper),
     m_xXMLSecurityContext(xXMLSecurityContext),
     m_xXMLSignature(xXMLSignature),
     mxMSF(rsMSF),
     m_ouKeyURI(RTL_ASCII_USTRINGPARAM(""))
{
    m_nSecurityId = getNextSecurityId();
}

int SecurityEntity::getNextSecurityId() const
{
    int nId = m_nNextSecurityId++;
    return nId;
}

void SecurityEntity::setKeyId(int nId)
{
    cssu::Reference<cssxc::sax::XKeyCollector> keyCollector (m_xReferenceListener, cssu::UNO_QUERY);
    keyCollector->setKeyId(nId);
}


void SecurityEntity::setKeyURI(const rtl::OUString& ouUri)
{
    m_ouKeyURI = ouUri;
}

cssu::Reference<cssxc::sax::XReferenceResolvedListener> SecurityEntity::getReferenceListener() const
{
    return m_xReferenceListener;
}

int SecurityEntity::getSecurityId() const
{
    return m_nSecurityId;
}

bool SecurityEntity::setKey(const rtl::OUString& ouUri, bool bIsExporting)
{
    bool rc = false;

     if (m_ouKeyURI != rtl::OUString(RTL_ASCII_USTRINGPARAM("")) &&
         m_ouKeyURI == ouUri)
    {
        int nKeeperId = m_xSAXEventKeeper->addSecurityElementCollector(
            bIsExporting ?
            (cssxc::sax::ElementMarkPriority_BEFOREMODIFY):
            (cssxc::sax::ElementMarkPriority_AFTERMODIFY),
            true);

        setKeyId(nKeeperId);
        m_xSAXEventKeeper->setSecurityId(nKeeperId, m_nSecurityId);

        cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster> xReferenceResolvedBroadcaster
            (m_xSAXEventKeeper, cssu::UNO_QUERY);
        xReferenceResolvedBroadcaster->addReferenceResolvedListener(nKeeperId,
            m_xReferenceListener);

        rc = true;
    }

    return rc;
}

bool SecurityEntity::endMission()
{
    cssu::Reference<cssxc::sax::XMissionTaker> xMissionTaker
        (m_xReferenceListener, cssu::UNO_QUERY);

    return xMissionTaker->endMission();
}

SignatureEntity::SignatureEntity(
    const cssu::Reference<cssxc::sax::XSecuritySAXEventKeeper>& xSAXEventKeeper,
    bool bIsExporting,
    XSecTester* pListener,
    const cssu::Reference<cssxc::XXMLSecurityContext>& xXMLSecurityContext,
    const cssu::Reference<cssxc::XXMLSignature>& xXMLSignature,
    const cssu::Reference< cssl::XMultiServiceFactory >& rsMSF)
    :SecurityEntity(xSAXEventKeeper,
            xXMLSecurityContext,
            xXMLSignature,
            rsMSF)
{
    if (bIsExporting)
    {
        m_nSignatureElementCollectorId =
            m_xSAXEventKeeper->addSecurityElementCollector(
                cssxc::sax::ElementMarkPriority_AFTERMODIFY,
                true);

        m_xSAXEventKeeper->setSecurityId(m_nSignatureElementCollectorId, m_nSecurityId);

        m_xReferenceListener = cssu::Reference< cssxc::sax::XReferenceResolvedListener >(
            mxMSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SIGNATURECREATOR_COMPONENT ))),
            cssu::UNO_QUERY);

        cssu::Reference<cssl::XInitialization> xInitialization(m_xReferenceListener, cssu::UNO_QUERY);

        cssu::Sequence<cssu::Any> args(5);
        char buf[16];

        sprintf(buf, "%d", m_nSecurityId);
        args[0] = cssu::makeAny(rtl::OUString(RTL_ASCII_USTRINGPARAM(buf)));
        args[1] = cssu::makeAny(m_xSAXEventKeeper);

        sprintf(buf, "%d", m_nSignatureElementCollectorId);
        args[2] = cssu::makeAny(rtl::OUString(RTL_ASCII_USTRINGPARAM(buf)));
        args[3] = cssu::makeAny(m_xXMLSecurityContext->getSecurityEnvironment());
        args[4] = cssu::makeAny(m_xXMLSignature);

        xInitialization->initialize(args);

        int nBlockerId = m_xSAXEventKeeper->addBlocker();
        m_xSAXEventKeeper->setSecurityId(nBlockerId, m_nSecurityId);

        cssu::Reference<cssxc::sax::XBlockerMonitor> xBlockerMonitor(m_xReferenceListener, cssu::UNO_QUERY);
        xBlockerMonitor->setBlockerId(nBlockerId);

        cssu::Reference< cssxc::sax::XSignatureCreationResultBroadcaster > xSignatureCreationResultBroadcaster
            (m_xReferenceListener, cssu::UNO_QUERY);
        xSignatureCreationResultBroadcaster->addSignatureCreationResultListener(pListener);
    }
    else
    {
        m_nSignatureElementCollectorId =
            m_xSAXEventKeeper->addSecurityElementCollector(
                cssxc::sax::ElementMarkPriority_BEFOREMODIFY,
                false);

        m_xSAXEventKeeper->setSecurityId(m_nSignatureElementCollectorId, m_nSecurityId);

        m_xReferenceListener = cssu::Reference< cssxc::sax::XReferenceResolvedListener >(
            mxMSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SIGNATUREVERIFIER_COMPONENT ))),
            cssu::UNO_QUERY);

        cssu::Reference<cssl::XInitialization> xInitialization(m_xReferenceListener, cssu::UNO_QUERY);

        cssu::Sequence<cssu::Any> args(5);
        char buf[16];

        sprintf(buf, "%d", m_nSecurityId);
        args[0] = cssu::makeAny(rtl::OUString(RTL_ASCII_USTRINGPARAM(buf)));
        args[1] = cssu::makeAny(m_xSAXEventKeeper);

        sprintf(buf, "%d", m_nSignatureElementCollectorId);
        args[2] = cssu::makeAny(rtl::OUString(RTL_ASCII_USTRINGPARAM(buf)));
        args[3] = cssu::makeAny(m_xXMLSecurityContext);
        args[4] = cssu::makeAny(m_xXMLSignature);
        xInitialization->initialize(args);

        cssu::Reference< cssxc::sax::XSignatureVerifyResultBroadcaster > xSignatureVerifyResultBroadcaster
            (m_xReferenceListener, cssu::UNO_QUERY);
        xSignatureVerifyResultBroadcaster->addSignatureVerifyResultListener(pListener);
    }

    cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster> xReferenceResolvedBroadcaster
        (m_xSAXEventKeeper, cssu::UNO_QUERY);
    xReferenceResolvedBroadcaster->addReferenceResolvedListener(
        m_nSignatureElementCollectorId, m_xReferenceListener);
}

void SignatureEntity::addReferenceURI(const rtl::OUString& ouUri)
{
    m_vReferenceIds.push_back(ouUri);
}

void SignatureEntity::setReferenceNumber() const
{
    cssu::Reference<cssxc::sax::XReferenceCollector> xReferenceCollector
        (m_xReferenceListener, cssu::UNO_QUERY);
    xReferenceCollector->setReferenceCount(m_vReferenceIds.size());
}

bool SignatureEntity::hasReference(const rtl::OUString& ouUri) const
{
    bool rc = false;

    std::vector<const rtl::OUString>::const_iterator ii;
    for (ii = m_vReferenceIds.begin(); ii != m_vReferenceIds.end(); ++ii)
    {
        if (ouUri == *ii)
        {
            rc = true;
            break;
        }
    }

    return rc;
}

bool SignatureEntity::setReference(const rtl::OUString& ouUri, bool bIsExporting) const
{
    bool rc = false;

    if (hasReference(ouUri))
    {
        int nKeeperId = m_xSAXEventKeeper->addSecurityElementCollector(
                bIsExporting ?
                (cssxc::sax::ElementMarkPriority_AFTERMODIFY):
                (cssxc::sax::ElementMarkPriority_BEFOREMODIFY),
                false);

        m_xSAXEventKeeper->setSecurityId(nKeeperId, m_nSecurityId);

        cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster> xReferenceResolvedBroadcaster
            (m_xSAXEventKeeper, cssu::UNO_QUERY);
        xReferenceResolvedBroadcaster->addReferenceResolvedListener(nKeeperId, m_xReferenceListener);

        cssu::Reference<cssxc::sax::XReferenceCollector> xReferenceCollector
            (m_xReferenceListener, cssu::UNO_QUERY);
        xReferenceCollector->setReferenceId(nKeeperId);

        rc = true;
    }

    return rc;
}

/* XDocumentHandler */
void SAL_CALL XSecTester::startDocument()
    throw (cssu::RuntimeException)
{
}

void SAL_CALL XSecTester::endDocument()
    throw (cssu::RuntimeException)
{
}

void SAL_CALL XSecTester::characters(const class rtl::OUString & chars)
    throw (cssu::RuntimeException)
{
    m_xExportHandler->characters(chars);
}

void SAL_CALL XSecTester::processingInstruction(const rtl::OUString & target, const rtl::OUString &data)
    throw (cssu::RuntimeException)
{
    m_xExportHandler->processingInstruction(target, data);
}

void SAL_CALL XSecTester::ignorableWhitespace(const rtl::OUString &)
    throw (cssu::RuntimeException)
{

}

void SAL_CALL XSecTester::startElement(const rtl::OUString & name, const cssu::Reference<cssxs::XAttributeList> &xAttribs)
    throw (cssu::RuntimeException)
{
    rtl::OUString ouIdAttr = xAttribs->getValueByName(
        rtl::OUString(RTL_ASCII_USTRINGPARAM("id")));

    if (ouIdAttr == NULL)
    {
        ouIdAttr = xAttribs->getValueByName(
            rtl::OUString(RTL_ASCII_USTRINGPARAM("Id")));
    }

    bool bHasIdAttr = (ouIdAttr != NULL && ouIdAttr.getLength() > 0 );
    bool needResend = false;

    if (bHasIdAttr || name.equalsAscii( SIGNATURE_STR ))
    {
        if (foundSecurityRelated() && ! m_bIsExporting)
        {
            needResend = true;
        }
    }

    if ( !m_bIsSAXEventKeeperOnTheSAXChain )
    {
        addStartAncestorEvent(name, xAttribs);
    }

    bool bSuppressingForwarding = checkSecurityElement(name, xAttribs);

    checkReference(name, xAttribs, ouIdAttr);

    if (needResend)
    {
        m_xSAXEventKeeper->setNextHandler(NULL);

        cssu::Reference<cssxs::XDocumentHandler> xSAXEventKeeperHandler
            (m_xSAXEventKeeper, cssu::UNO_QUERY);

        xSAXEventKeeperHandler->startElement(name, xAttribs);
        m_xSAXEventKeeper->setNextHandler(this);
    }

    if (!bSuppressingForwarding)
    {
        m_xExportHandler->startElement(name, xAttribs);
    }
}

void SAL_CALL XSecTester::endElement(const rtl::OUString& name)
    throw (cssu::RuntimeException)
{
    if (!m_stCurrentPath.empty())
    {
        void* pSignedInfo = m_stCurrentPath.top();
        bool bIsStringType = m_stCurrentPathType.top();

        m_stCurrentPath.pop();
        m_stCurrentPathType.pop();

        if (bIsStringType && !strcmp((const char *)pSignedInfo, SIGNEDINFO_STR))
        {
            if (!m_stCurrentPath.empty())
            {
                void* pSignature = m_stCurrentPath.top();
                bIsStringType = m_stCurrentPathType.top();

                if (!bIsStringType && pSignature != NULL)
                {
                    ((SignatureEntity *) pSignature)->setReferenceNumber();
                }
            }
        }
    }

    if ( !m_bIsSAXEventKeeperOnTheSAXChain )
    {
        addEndAncestorEvent(name);
    }

    m_xExportHandler->endElement(name);
}

void SAL_CALL XSecTester::setDocumentLocator( const cssu::Reference<cssxs::XLocator>& )
    throw (cssu::RuntimeException)
{
}

void XSecTester::changeOutput()
{
    if (m_bIsExporting)
    {
        if (m_bIsSAXEventKeeperOnTheSAXChain)
        {
            m_xExportHandler = cssu::Reference<cssxs::XDocumentHandler>
                (m_xSAXEventKeeper, cssu::UNO_QUERY);

            m_xSAXEventKeeper->setNextHandler(NULL);

            flushAncestorEvents(m_xExportHandler);

            m_xSAXEventKeeper->setNextHandler(m_xOutputHandler);
        }
        else
        {
            m_xExportHandler = m_xOutputHandler;
        }
    }
     else
     {
        if (m_bIsSAXEventKeeperOnTheSAXChain)
        {
            cssu::Reference<cssxs::XDocumentHandler> xSAXEventKeeperHandler
                (m_xSAXEventKeeper, cssu::UNO_QUERY);

            m_xSAXEventKeeper->setNextHandler(NULL);

            flushAncestorEvents(xSAXEventKeeperHandler);

            m_xSaxParser->setDocumentHandler(xSAXEventKeeperHandler);
            m_xSAXEventKeeper->setNextHandler(this);
        }
        else
        {
            m_xSaxParser->setDocumentHandler(this);
        }

    }
}

bool XSecTester::foundSecurityRelated()
{
    if (m_xSAXEventKeeper == NULL)
    {
        m_bIsBlocking = false;
        m_bIsInsideCollectedElement = false;

        m_xXMLDocumentWrapper = cssu::Reference<cssxw::XXMLDocumentWrapper>
            (mxMSF->createInstance( m_ouXMLDocumentWrapperComponentName ),
             cssu::UNO_QUERY);

        m_xSAXEventKeeper = cssu::Reference< cssxc::sax::XSecuritySAXEventKeeper >
            (mxMSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SAXEVENTKEEPER_COMPONENT ))),
             cssu::UNO_QUERY);

        cssu::Reference<cssl::XInitialization> xInitialization(m_xSAXEventKeeper,  cssu::UNO_QUERY);

        cssu::Sequence <cssu::Any> arg(1);
        arg[0] = cssu::makeAny(m_xXMLDocumentWrapper);
        xInitialization->initialize(arg);

        cssu::Reference<cssxc::sax::XSAXEventKeeperStatusChangeBroadcaster>
            xSAXEventKeeperStatusChangeBroadcaster(m_xSAXEventKeeper, cssu::UNO_QUERY);
        xSAXEventKeeperStatusChangeBroadcaster->addSAXEventKeeperStatusChangeListener(this);
    }

    bool rc = false;

    if (!m_bIsSAXEventKeeperOnTheSAXChain)
    {
        rc = true;
    }

    m_bIsSAXEventKeeperOnTheSAXChain=true;
    changeOutput();

    return rc;
}

void XSecTester::findKeyOrReference(SecurityEntity* pSecurityEntity, const rtl::OUString& ouUri, bool bIsFindingKey)
{
    std::vector<rtl::OUString>::iterator ii_referenceURIs;
    std::vector<int>::iterator ii_referenceKeeperIds;
    std::vector<int>::iterator ii_referenceRefNums;

    for (ii_referenceURIs = m_vUnsolvedReferenceURIs.begin(),
         ii_referenceKeeperIds = m_vUnsolvedReferenceKeeperIds.begin(),
         ii_referenceRefNums = m_vUnsolvedReferenceRefNums.begin();
          ii_referenceURIs != m_vUnsolvedReferenceURIs.end(); )
    {
        rtl::OUString ouReferenceUri = *ii_referenceURIs;

        if (ouReferenceUri == ouUri)
        {
            int nKeeperId = *ii_referenceKeeperIds;
            int nRefNum = *ii_referenceRefNums;

            if ( bIsFindingKey )
            {
                 int nClonedKeeperId = m_xSAXEventKeeper->cloneElementCollector(
                     nKeeperId,
                     m_bIsExporting?
                     (cssxc::sax::ElementMarkPriority_BEFOREMODIFY):
                    (cssxc::sax::ElementMarkPriority_AFTERMODIFY));

                 pSecurityEntity->setKeyId(nClonedKeeperId);

                 m_xSAXEventKeeper->setSecurityId(nClonedKeeperId, pSecurityEntity->getSecurityId());

                 cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster>
                    xReferenceResolvedBroadcaster(m_xSAXEventKeeper, cssu::UNO_QUERY);
                xReferenceResolvedBroadcaster->addReferenceResolvedListener(
                     nClonedKeeperId,
                     pSecurityEntity->getReferenceListener());
            }
            else
            {
                int nClonedKeeperId = m_xSAXEventKeeper->cloneElementCollector(
                    nKeeperId,
                    m_bIsExporting?
                    (cssxc::sax::ElementMarkPriority_AFTERMODIFY):
                    (cssxc::sax::ElementMarkPriority_BEFOREMODIFY));

                m_xSAXEventKeeper->setSecurityId(nClonedKeeperId, pSecurityEntity->getSecurityId());

                cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster>
                    xReferenceResolvedBroadcaster
                    (m_xSAXEventKeeper, cssu::UNO_QUERY);
                xReferenceResolvedBroadcaster->addReferenceResolvedListener(
                    nClonedKeeperId,
                    pSecurityEntity->getReferenceListener());

                cssu::Reference<cssxc::sax::XReferenceCollector> xReferenceCollector
                        (pSecurityEntity->getReferenceListener(), cssu::UNO_QUERY);
                xReferenceCollector->setReferenceId(nClonedKeeperId);
            }

            nRefNum--;
            if (nRefNum == 0)
            {
                m_xSAXEventKeeper->removeElementCollector(nKeeperId);

                ii_referenceURIs = m_vUnsolvedReferenceURIs.erase(ii_referenceURIs);
                ii_referenceKeeperIds = m_vUnsolvedReferenceKeeperIds.erase(ii_referenceKeeperIds);
                ii_referenceRefNums = m_vUnsolvedReferenceRefNums.erase(ii_referenceRefNums);
            }
            else
            {
                (*ii_referenceRefNums) = nRefNum;

                ii_referenceURIs++;
                ii_referenceKeeperIds++;
                ii_referenceRefNums++;
            }

            if (bIsFindingKey)
            {
                break;
            }
        }
        else
        {
            ii_referenceURIs++;
            ii_referenceKeeperIds++;
            ii_referenceRefNums++;
        }
    }
}

bool XSecTester::checkSecurityElement(
    const rtl::OUString& ouLocalName,
    const cssu::Reference<cssxs::XAttributeList>& xAttribs)
{
    bool rc = false;

    if (ouLocalName.equalsAscii(SIGNATURE_STR))
    {
        SignatureEntity* pSignatureEntity = new SignatureEntity(
            m_xSAXEventKeeper,
            m_bIsExporting,
            this,
            m_xXMLSecurityContext,
            m_xXMLSignature,
            mxMSF);

        m_vSignatureList.push_back(pSignatureEntity);

        m_stCurrentPath.push(pSignatureEntity);
        m_stCurrentPathType.push(false);
    }
    else if (ouLocalName.equalsAscii(REFERENCE_STR))
    {
        if (!m_stCurrentPath.empty())
        {
             void* pSignedInfo = m_stCurrentPath.top();
             bool bIsStringType = m_stCurrentPathType.top();

            m_stCurrentPath.pop();
            m_stCurrentPathType.pop();

            if (bIsStringType && !m_stCurrentPath.empty())
            {
                void* pSignature = m_stCurrentPath.top();
                bool bIsStringType2 = m_stCurrentPathType.top();

                if (!strcmp((const char*)pSignedInfo, SIGNEDINFO_STR) && !bIsStringType2)
                {
                    rtl::OUString ouUri = xAttribs->getValueByName
                        (rtl::OUString(RTL_ASCII_USTRINGPARAM( URI_ATTR_STR )));

                    if (ouUri.matchAsciiL("#", 1, 0))
                    {
                        rtl::OUString uri = ouUri.copy(1);
                         SignatureEntity* pSignatureEntity = (SignatureEntity *)pSignature;

                        if (uri != NULL && uri.getLength()>0)
                        {
                            pSignatureEntity->addReferenceURI(uri);
                            findKeyOrReference(pSignatureEntity, uri, true);
                        }
                    }
                }
            }
            m_stCurrentPath.push(pSignedInfo);
            m_stCurrentPathType.push(bIsStringType);
        }
        m_stCurrentPath.push( (void *)REFERENCE_STR);
        m_stCurrentPathType.push(true);
    }
    else if(ouLocalName.equalsAscii(KEYVALUE_STR) ||
        ouLocalName.equalsAscii(KEYNAME_STR) ||
        ouLocalName.equalsAscii(X509DATA_STR) ||
        ouLocalName.equalsAscii(ENCRYPTEDKEY_STR))
    {
        if (!m_stCurrentPath.empty())
        {
            void* pKeyInfo = m_stCurrentPath.top();
            bool bIsStringType = m_stCurrentPathType.top();

            m_stCurrentPath.pop();
            m_stCurrentPathType.pop();

             if (bIsStringType && !m_stCurrentPath.empty())
             {
                bool bIsStringType2 = m_stCurrentPathType.top();

                if (!bIsStringType2)
                {
                    SecurityEntity *pSecurityEntity =
                        (SecurityEntity *) (m_stCurrentPath.top());
                    pSecurityEntity->setKeyId(0);
                }
             }

            m_stCurrentPath.push(pKeyInfo);
            m_stCurrentPathType.push(bIsStringType);
        }

        m_stCurrentPath.push((void *)KEYVALUE_STR);
        m_stCurrentPathType.push(true);
    }
    else if(ouLocalName.equalsAscii(RETRIEVALMETHOD_STR))
    {
        if (!m_stCurrentPath.empty())
        {
            void* pKeyInfo = m_stCurrentPath.top();
            bool bIsStringType = m_stCurrentPathType.top();

            m_stCurrentPath.pop();
            m_stCurrentPathType.pop();

            if (bIsStringType && !m_stCurrentPath.empty())
            {
                bool bIsStringType2 = m_stCurrentPathType.top();

                if (!bIsStringType2)
                {
                    SecurityEntity *pSecurityEntity =
                        (SecurityEntity *) m_stCurrentPath.top();
                    rtl::OUString ouUri = xAttribs->getValueByName(
                        rtl::OUString(RTL_ASCII_USTRINGPARAM( URI_ATTR_STR )));

                    if (!strcmp((const char *)pKeyInfo, KEYINFO_STR) &&
                        ouUri != NULL && ouUri.getLength()>0)
                    {
                        pSecurityEntity->setKeyURI(ouUri);
                        findKeyOrReference(pSecurityEntity, ouUri, true);
                    }
                }

            }

            m_stCurrentPath.push(pKeyInfo);
            m_stCurrentPathType.push(bIsStringType);
        }

        m_stCurrentPath.push((void *)RETRIEVALMETHOD_STR);
        m_stCurrentPathType.push(true);
    }
    else if(ouLocalName.equalsAscii(KEYINFO_STR))
    {
        m_stCurrentPath.push((void *)KEYINFO_STR);
        m_stCurrentPathType.push(true);
    }
    else if(ouLocalName.equalsAscii(SIGNEDINFO_STR))
    {
        m_stCurrentPath.push((void *)SIGNEDINFO_STR);
        m_stCurrentPathType.push(true);
    }
    else
    {
        m_stCurrentPath.push((void *)OTHER_ELEMENT_STR);
        m_stCurrentPathType.push(true);
    }

    return rc;
}

void XSecTester::checkReference(
    const rtl::OUString& ouLocalName,
    const cssu::Reference<cssxs::XAttributeList>& xAttribs,
    const rtl::OUString& ouId)
{
    rtl::OUString refNumStr =
        xAttribs->getValueByName(rtl::OUString(RTL_ASCII_USTRINGPARAM(REFNUM_ATTR_STR)));

    if (ouId != NULL && ouId.getLength()>0 )
    {
        int nRefNum = 999;
        if (refNumStr != NULL && refNumStr.getLength()>0 )
        {
            nRefNum = refNumStr.toInt32();
        }

        int nLength = m_vSignatureList.size();
        for (int i = 0; i<nLength; ++i)
        {
            SignatureEntity* pSignatureEntity = m_vSignatureList.at(i);

            if (pSignatureEntity->setReference(ouId, m_bIsExporting))
            {
                nRefNum--;
            }

            if (pSignatureEntity->setKey(ouId, m_bIsExporting))
            {
                nRefNum--;
            }
        }

        if (nRefNum>0)
        {
            int nKeeperId;

            if (ouLocalName.equalsAscii(ENCRYPTEDKEY_STR))
            {
                nKeeperId = m_xSAXEventKeeper->addSecurityElementCollector(
                    m_bIsExporting ?
                    (cssxc::sax::ElementMarkPriority_BEFOREMODIFY):
                    (cssxc::sax::ElementMarkPriority_AFTERMODIFY),
                    true);
            }
            else
            {
                nKeeperId = m_xSAXEventKeeper->addSecurityElementCollector(
                    m_bIsExporting?
                    (cssxc::sax::ElementMarkPriority_AFTERMODIFY):
                    (cssxc::sax::ElementMarkPriority_BEFOREMODIFY),
                    false);
            }

            m_vUnsolvedReferenceURIs.push_back(ouId);
            m_vUnsolvedReferenceKeeperIds.push_back(nKeeperId);
            m_vUnsolvedReferenceRefNums.push_back(nRefNum);
        }
    }
}

void XSecTester::endMission()
{
    while (m_vSignatureList.size() > 0)
    {
        if (m_vSignatureList.size()>0)
        {
            SignatureEntity * pSignatureEntity = m_vSignatureList.at(0);
            m_vSignatureList.erase(m_vSignatureList.begin());
            pSignatureEntity->endMission();
            delete pSignatureEntity;
        }
    }

    while (m_vUnsolvedReferenceURIs.size()>0)
    {
        int nKeeperId = m_vUnsolvedReferenceKeeperIds.at(0);
        m_xSAXEventKeeper->removeElementCollector(nKeeperId);
        m_vUnsolvedReferenceURIs.erase(m_vUnsolvedReferenceURIs.begin());
        m_vUnsolvedReferenceKeeperIds.erase(m_vUnsolvedReferenceKeeperIds.begin());
        m_vUnsolvedReferenceRefNums.erase(m_vUnsolvedReferenceRefNums.begin());
    }
}

void XSecTester::addStartAncestorEvent(
    const rtl::OUString& ouName,
    const cssu::Reference< cssxs::XAttributeList >& xAttribs)
{
    sal_Int32 nLength = xAttribs->getLength();
    AncestorEvent* ancestorEvent = new AncestorEvent( nLength );

    ancestorEvent->bIsStartElement = true;
    ancestorEvent->ouName = ouName;

    for (int i = 0; i<nLength; ++i)
    {
        (ancestorEvent->aAttributeList[i]).sName = xAttribs->getNameByIndex((short)i);
        (ancestorEvent->aAttributeList[i]).sValue =xAttribs->getValueByIndex((short)i);
    }

    m_vAncestorEvents.push_back(ancestorEvent);
}

void XSecTester::addEndAncestorEvent(const rtl::OUString& ouName)
{
    AncestorEvent* ancestorEvent = new AncestorEvent(0);

    ancestorEvent->bIsStartElement = false;
    ancestorEvent->ouName = ouName;

    m_vAncestorEvents.push_back(ancestorEvent);
}

void XSecTester::sendAncestorStartElementEvent(
    const rtl::OUString& ouName,
    const cssu::Sequence< cssxcsax::XMLAttribute >& attrList,
    const cssu::Reference< cssxs::XDocumentHandler >& xDocumentHandler) const
{
    SvXMLAttributeList* pAttributeList = new SvXMLAttributeList();
    cssu::Reference < cssxs::XAttributeList > xAttrList
        = cssu::Reference< cssxs::XAttributeList > (pAttributeList);

    sal_Int32 nLength = attrList.getLength();

    for (int i = 0; i<nLength; ++i)
    {
        pAttributeList->AddAttribute( attrList[i].sName, attrList[i].sValue);
    }

    xDocumentHandler->startElement(ouName, xAttrList);
}

void XSecTester::sendAncestorEndElementEvent(
    const rtl::OUString& ouName,
    const cssu::Reference< cssxs::XDocumentHandler >& xDocumentHandler) const
{
    xDocumentHandler->endElement(ouName);
}

std::vector< AncestorEvent* >::const_iterator XSecTester::checkAncestorStartElementEvent(
    const std::vector< AncestorEvent* >::const_iterator& ii,
    const cssu::Reference< cssxs::XDocumentHandler >& xDocumentHandler) const
{
    std::vector< AncestorEvent* >::const_iterator next = ii+1;

    if (next == m_vAncestorEvents.end())
    {
        sendAncestorStartElementEvent(
            (*ii)->ouName, (*ii)->aAttributeList, xDocumentHandler);
    }
    else
    {
        while ((next != m_vAncestorEvents.end()) && ((*next)->bIsStartElement))
        {
            next = checkAncestorStartElementEvent(next, xDocumentHandler);
        }

        if (next != m_vAncestorEvents.end())
        {
            next++;
        }
    }

    return next;
}

void XSecTester::flushAncestorEvents(
    const cssu::Reference< cssxs::XDocumentHandler >& xDocumentHandler)
{
    std::vector< AncestorEvent* >::const_iterator ii;

    if (xDocumentHandler != NULL)
    {
        ii = m_vAncestorEvents.begin();

        while (ii != m_vAncestorEvents.end())
        {
            AncestorEvent* ancestorEvent = *ii;

            if (ancestorEvent->bIsStartElement)
            {
                ii = checkAncestorStartElementEvent(ii, xDocumentHandler);
            }
            else
            {
                sendAncestorEndElementEvent((*ii)->ouName, xDocumentHandler);
                ii++;
            }
        }
    }

    /* free the ancestor events list */
    std::vector< AncestorEvent* >::iterator jj;

    while (m_vAncestorEvents.size()>0)
    {
        jj = m_vAncestorEvents.begin();
        delete *jj;
        m_vAncestorEvents.erase(jj);
    }
}

/*
 * Get the length of a file in a platform independant fashion
 */
int getLength(const char *pInputFileName)
{
    int nSize = 0;
    std::ifstream data(pInputFileName);

    data.seekg(0, std::ios_base::end);
    nSize = data.tellg();

    return nSize;
}

void outputHeader()
{
    fprintf(stderr, "%16s%4s%8s%12s%12s%12s%12s\n", "File Name", "E/I", "Size", "-C++", "-Java", "Forw-O", "No S/E");
    fprintf(stderr, "===============================================================================\n");
}

/*
 * print the output on the screen as well as in the GNUPlot data file
 */
void output(const rtl::OUString& ouInputFileName,
    const rtl::OUString& ouTime_C,
    const rtl::OUString& ouTime_Java,
    const rtl::OUString& ouTime_NoSecurity,
    const rtl::OUString& ouTime_JavaForwardOnly,
    const rtl::OUString& ouRemark_C,
    const rtl::OUString& ouRemark_Java,
    bool bIsExporting)
{
    int nSize = getLength(rtl::OString(ouInputFileName, ouInputFileName.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());
    std::ofstream data;

    /* print screen */
    int nPosition = ouInputFileName.lastIndexOf('\\');
    rtl::OUString fileName = ouInputFileName.copy(nPosition + 1);

    fprintf(stderr, "%16s", rtl::OString(fileName, fileName.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());

    fprintf(stderr, "%4s", bIsExporting?"E":"I");
    fprintf(stderr, "%7dK", nSize/1024);
    fprintf(stderr, "%8s %3s",
        rtl::OString(ouTime_C, ouTime_C.getLength(), RTL_TEXTENCODING_ASCII_US).getStr(),
        rtl::OString(ouRemark_C, ouRemark_C.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());
    fprintf(stderr, "%8s %3s",
        rtl::OString(ouTime_Java, ouTime_Java.getLength(), RTL_TEXTENCODING_ASCII_US).getStr(),
        rtl::OString(ouRemark_Java, ouRemark_Java.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());
    fprintf(stderr, "%12s", rtl::OString(ouTime_JavaForwardOnly, ouTime_JavaForwardOnly.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());
    fprintf(stderr, "%12s", rtl::OString(ouTime_NoSecurity, ouTime_NoSecurity.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());
    fprintf(stderr, "\n");

    /* output the data as GNUPlot data file */
    /*
    char str[32];
    sprintf(str, "%d %s", nSize, rtl::OString(ouTime_C, ouTime_C.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());
    data.open("d:\\time_vs_size.txt", std::ios::app);
    data <<  str << std::endl;
    data.close();

    sprintf(str, "%d %s", nSize, rtl::OString(ouTime_Java, ouTime_Java.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());
    data.open("d:\\time_vs_size_without_sec.txt", std::ios::app);
    data <<  str << std::endl;
    data.close();
    */
}

int main( int argc, char **argv )
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: testtool <exportbatchfile> <importbatchfile> [<cppcryptotoken>] [<javacryptotoken>]\n");
        exit (-1);
    }

    rtl::OUString aExportBatchFile = rtl::OUString::createFromAscii(argv[1]);
    rtl::OUString aImportBatchFile = rtl::OUString::createFromAscii(argv[2]);
    rtl::OUString aCPPCryptoToken;
    if ( argc > 3 )
        aCPPCryptoToken = rtl::OUString::createFromAscii(argv[3]);
    rtl::OUString aJavaCryptoToken;
    if ( argc > 4 )
        aJavaCryptoToken = rtl::OUString::createFromAscii(argv[4]);

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF = CreateDemoServiceFactory();

        XSecTester* pTester = new XSecTester( xMSF );
        uno::Reference< xml::sax::XDocumentHandler > xKeepARef = pTester;

        pTester->setCryptoDir( aJavaCryptoToken, aCPPCryptoToken );

        rtl::OUString ouTime_C, ouTime_Java, ouTime_NoSecurity, ouTime_JavaForwardOnly;
        rtl::OUString ouInputFileName;
        rtl::OUString outputFileName1;
        rtl::OUString outputFileName2;
        rtl::OUString ouRemark_C, ouRemark_Java;

        outputHeader();

        std::ifstream batch_export, batch_import;

        batch_export.open(OUStringToOString( aExportBatchFile, RTL_TEXTENCODING_ASCII_US ).getStr());

        const int MAX_LINE = 80;
        char line[MAX_LINE + 1];

        while (batch_export.getline(line, MAX_LINE))
        {
            ouInputFileName = rtl::OUString::createFromAscii(line);
            int nPosition = ouInputFileName.lastIndexOf('.');
            int nPosition1;

            /*
             * export the file with signautre/encryption (C++)
             */
            outputFileName1 = ouInputFileName.copy(0, nPosition) +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-ex.xml"));
            ouTime_C = pTester->export_xml(ouInputFileName, outputFileName1, sal_False);
            nPosition1 = ouTime_C.lastIndexOf('\t');
            ouRemark_C = ouTime_C.copy(nPosition1 + 1);
            ouTime_C = ouTime_C.copy(0, nPosition1);

            /*
             * export the file with signautre/encryption (Java)
             */
            outputFileName1 = ouInputFileName.copy(0, nPosition) +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-ex2.xml"));
            ouTime_Java = pTester->export_xml(ouInputFileName, outputFileName1, sal_True);
            nPosition1 = ouTime_Java.lastIndexOf('\t');
            ouRemark_Java = ouTime_Java.copy(nPosition1 + 1);
            ouTime_Java = ouTime_Java.copy(0, nPosition1);

            /*
             * export the file without signautre/encryption
             */
            outputFileName2 = ouInputFileName.copy(0, nPosition) +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-ex-no.xml"));
            ouTime_NoSecurity = pTester->transfer_without_sec(ouInputFileName, outputFileName2, sal_False);

            /*
             * export the file with Java Flat Filter
             */
            outputFileName2 = ouInputFileName.copy(0, nPosition) +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-ex-jf.xml"));
            ouTime_JavaForwardOnly = pTester->transfer_without_sec(ouInputFileName, outputFileName2, sal_True);

            /*
             * print output
             */
            output(ouInputFileName, ouTime_C, ouTime_Java, ouTime_NoSecurity, ouTime_JavaForwardOnly, ouRemark_C, ouRemark_Java, true);
        }

        batch_export.close();

        batch_import.open(OUStringToOString( aImportBatchFile, RTL_TEXTENCODING_ASCII_US ).getStr());

        while (batch_import.getline(line, MAX_LINE))
        {
            ouInputFileName = rtl::OUString::createFromAscii(line);
            int nPosition = ouInputFileName.lastIndexOf('.');
            int nPosition1;

            /*
             * import the file with signautre/encryption (C++)
             */
            outputFileName1 = ouInputFileName.copy(0, nPosition) +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-im.xml"));
            ouTime_C = pTester->import_xml(ouInputFileName, outputFileName1, sal_False);
            nPosition1 = ouTime_C.lastIndexOf('\t');
            ouRemark_C = ouTime_C.copy(nPosition1 + 1);
            ouTime_C = ouTime_C.copy(0, nPosition1);

            /*
             * import the file with signautre/encryption (Java)
             */
            outputFileName1 = ouInputFileName.copy(0, nPosition) +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-im2.xml"));
            ouTime_Java = pTester->import_xml(ouInputFileName, outputFileName1, sal_True);
            nPosition1 = ouTime_Java.lastIndexOf('\t');
            ouRemark_Java = ouTime_Java.copy(nPosition1 + 1);
            ouTime_Java = ouTime_Java.copy(0, nPosition1);

            /*
             * import the file without signautre/encryption
             */
            outputFileName2 = ouInputFileName.copy(0, nPosition) +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-im-no.xml"));
            ouTime_NoSecurity = pTester->transfer_without_sec(ouInputFileName, outputFileName2, sal_False);

            /*
             * import the file without signautre/encryption
             */

            outputFileName2 = ouInputFileName.copy(0, nPosition) +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-im-jf.xml"));
            ouTime_JavaForwardOnly = pTester->transfer_without_sec(ouInputFileName, outputFileName2, sal_True);

            /*
             * print output
             */
            output(ouInputFileName, ouTime_C, ouTime_Java, ouTime_NoSecurity, ouTime_JavaForwardOnly, ouRemark_C, ouRemark_Java, false);
        }

        batch_import.close();

        fprintf(stderr, "\n");
    }
    catch( cssu::Exception& e )
    {
        fprintf( stderr , "\nEXCEPTION! Error Message: %s\n" ,
                 rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
