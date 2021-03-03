/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <xsecctl.hxx>
#include <documentsignaturehelper.hxx>
#include <framework/saxeventkeeperimpl.hxx>
#include <xmlsec/xmldocumentwrapper_xmlsecimpl.hxx>
#if HAVE_FEATURE_GPGME
# include <gpg/xmlsignature_gpgimpl.hxx>
#endif

#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#include <com/sun/star/xml/crypto/SecurityOperationStatus.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>

#include <xmloff/attrlist.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <unotools/datetime.hxx>
#include "ooxmlsecexporter.hxx"
#include <xmlsignaturehelper2.hxx>

using namespace com::sun::star;

namespace
{
OUString getDigestURI(sal_Int32 nID)
{
    switch( nID )
    {
        case css::xml::crypto::DigestID::SHA1:
            return ALGO_XMLDSIGSHA1;
        case css::xml::crypto::DigestID::SHA256:
            return ALGO_XMLDSIGSHA256;
        case css::xml::crypto::DigestID::SHA512:
            return ALGO_XMLDSIGSHA512;
        default:
            return ALGO_XMLDSIGSHA1;
    }
}
OUString getSignatureURI(svl::crypto::SignatureMethodAlgorithm eAlgorithm, sal_Int32 nDigestID)
{
    OUString aRet;

    if (eAlgorithm == svl::crypto::SignatureMethodAlgorithm::ECDSA)
    {
        switch (nDigestID)
        {
            case css::xml::crypto::DigestID::SHA1:
                aRet = ALGO_ECDSASHA1;
                break;
            case css::xml::crypto::DigestID::SHA256:
                aRet = ALGO_ECDSASHA256;
                break;
            case css::xml::crypto::DigestID::SHA512:
                aRet = ALGO_ECDSASHA512;
                break;
            default:
                aRet = ALGO_ECDSASHA1;
                break;
        }
    }
    if (!aRet.isEmpty())
        return aRet;

    switch (nDigestID)
    {
        case css::xml::crypto::DigestID::SHA1:
            return ALGO_RSASHA1;
        case css::xml::crypto::DigestID::SHA256:
            return ALGO_RSASHA256;
        case css::xml::crypto::DigestID::SHA512:
            return ALGO_RSASHA512;
        default:
            return ALGO_RSASHA1;
    }
}
}

XSecController::XSecController( const css::uno::Reference<css::uno::XComponentContext>& rxCtx )
    : mxCtx(rxCtx)
    , m_nNextSecurityId(1)
    , m_bIsPreviousNodeInitializable(false)
    , m_bIsSAXEventKeeperConnected(false)
    , m_bIsCollectingElement(false)
    , m_bIsBlocking(false)
    , m_eStatusOfSecurityComponents(InitializationState::UNINITIALIZED)
    , m_bIsSAXEventKeeperSticky(false)
    , m_nReservedSignatureId(0)
    , m_bVerifyCurrentSignature(false)
{
}

XSecController::~XSecController()
{
}


/*
 * private methods
 */
int XSecController::findSignatureInfor( sal_Int32 nSecurityId) const
/****** XSecController/findSignatureInfor *************************************
 *
 *   NAME
 *  findSignatureInfor -- find SignatureInformation struct for a particular
 *                        signature
 *
 *   SYNOPSIS
 *  index = findSignatureInfor( nSecurityId );
 *
 *   INPUTS
 *  nSecurityId - the signature's id
 *
 *   RESULT
 *  index - the index of the signature, or -1 when no such signature
 *          existing
 ******************************************************************************/
{
    int i;
    int size = m_vInternalSignatureInformations.size();

    for (i=0; i<size; ++i)
    {
        if (m_vInternalSignatureInformations[i].signatureInfor.nSecurityId == nSecurityId)
        {
            return i;
        }
    }

    return -1;
}

void XSecController::createXSecComponent( )
/****** XSecController/createXSecComponent ************************************
 *
 *   NAME
 *  bResult = createXSecComponent -- creates xml security components
 *
 *   FUNCTION
 *  Creates xml security components, including:
 *  1. an xml signature bridge component
 *  2. an XMLDocumentWrapper component
 *  3. a SAXEventKeeper component
 ******************************************************************************/
{
    /*
     * marks all security components are not available.
     */
    m_eStatusOfSecurityComponents = InitializationState::FAILTOINITIALIZED;
    m_xXMLSignature = nullptr;
    m_xXMLDocumentWrapper = nullptr;
    m_xSAXEventKeeper = nullptr;

    css::uno::Reference< css::lang::XMultiComponentFactory > xMCF( mxCtx->getServiceManager() );

#if HAVE_FEATURE_GPGME
    uno::Reference< lang::XServiceInfo > xServiceInfo( m_xSecurityContext, css::uno::UNO_QUERY );
    if (xServiceInfo->getImplementationName() == "com.sun.star.xml.security.gpg.XMLSecurityContext_GpgImpl")
        m_xXMLSignature.set(new XMLSignature_GpgImpl());
    else // xmlsec or mscrypt
#endif
        m_xXMLSignature.set(xMCF->createInstanceWithContext("com.sun.star.xml.crypto.XMLSignature", mxCtx), css::uno::UNO_QUERY);

    bool bSuccess = m_xXMLSignature.is();
    if ( bSuccess )
    /*
     * XMLSignature created successfully.
     */
        m_xXMLDocumentWrapper = new XMLDocumentWrapper_XmlSecImpl();

    bSuccess &= m_xXMLDocumentWrapper.is();
    if ( bSuccess )
        m_xSAXEventKeeper = new SAXEventKeeperImpl();

    bSuccess &= m_xSAXEventKeeper.is();

    if (!bSuccess)
    /*
     * SAXEventKeeper created successfully.
     */
        return;

    css::uno::Sequence <css::uno::Any> arg(1);
    arg[0] <<= uno::Reference<xml::wrapper::XXMLDocumentWrapper>(m_xXMLDocumentWrapper.get());
    m_xSAXEventKeeper->initialize(arg);

    css::uno::Reference< css::xml::crypto::sax::XSAXEventKeeperStatusChangeListener >
        xStatusChangeListener = this;

    m_xSAXEventKeeper->addSAXEventKeeperStatusChangeListener( xStatusChangeListener );

    m_eStatusOfSecurityComponents = InitializationState::INITIALIZED;
}

bool XSecController::chainOn()
/****** XSecController/chainOn ************************************************
 *
 *   NAME
 *  chainOn -- tries to connect the SAXEventKeeper with the SAX chain.
 *
 *   SYNOPSIS
 *  bJustChainingOn = chainOn();
 *
 *   FUNCTION
 *  First, checks whether the SAXEventKeeper is on the SAX chain. If not,
 *  creates xml security components, and chains the SAXEventKeeper into
 *  the SAX chain.
 *  Before being chained in, the SAXEventKeeper needs to receive all
 *  missed key SAX events, which can promise the DOM tree buffered by the
 *  SAXEventKeeper has the same structure with the original document.
 *
 *   RESULT
 *  bJustChainingOn - whether the SAXEventKeeper is just chained into the
 *                    SAX chain.
 *
 *   NOTES
 *  Sometimes, the last key SAX event can't be transferred to the
 *  SAXEventKeeper together.
 *  For instance, at the time a referenced element is detected, the
 *  startElement event has already been reserved by the ElementStackKeeper.
 *  Meanwhile, an ElementCollector needs to be created before the
 *  SAXEventKeeper receives that startElement event.
 *  So for the SAXEventKeeper, it needs to receive all missed key SAX
 *  events except that startElement event, then adds a new
 *  ElementCollector, then receives that startElement event.
 ******************************************************************************/
{
    bool rc = false;

    if (!m_bIsSAXEventKeeperSticky && !m_bIsSAXEventKeeperConnected)
    {
        if ( m_eStatusOfSecurityComponents == InitializationState::UNINITIALIZED )
        {
            createXSecComponent();
        }

        if ( m_eStatusOfSecurityComponents == InitializationState::INITIALIZED )
        /*
         * if all security components are ready, chains on the SAXEventKeeper
         */
        {
            /*
             * disconnect the SAXEventKeeper with its current output handler,
             * to make sure no SAX event is forwarded during the connecting
             * phase.
             */
            m_xSAXEventKeeper->setNextHandler( nullptr );

            css::uno::Reference< css::xml::sax::XDocumentHandler > xSEKHandler(static_cast<cppu::OWeakObject*>(m_xSAXEventKeeper.get()), css::uno::UNO_QUERY);

            /*
             * connects the previous document handler on the SAX chain
             */
            if ( m_xPreviousNodeOnSAXChain.is() )
            {
                if ( m_bIsPreviousNodeInitializable )
                {
                    css::uno::Reference< css::lang::XInitialization > xInitialization
                        (m_xPreviousNodeOnSAXChain, css::uno::UNO_QUERY);

                    css::uno::Sequence<css::uno::Any> aArgs( 1 );
                    aArgs[0] <<= xSEKHandler;
                    xInitialization->initialize(aArgs);
                }
                else
                {
                    css::uno::Reference< css::xml::sax::XParser > xParser
                        (m_xPreviousNodeOnSAXChain, css::uno::UNO_QUERY);
                    xParser->setDocumentHandler( xSEKHandler );
                }
            }

            /*
             * connects the next document handler on the SAX chain
             */
            m_xSAXEventKeeper->setNextHandler(uno::Reference<xml::sax::XDocumentHandler>());

            m_bIsSAXEventKeeperConnected = true;

            rc = true;
        }
    }

    return rc;
}

void XSecController::chainOff()
/****** XSecController/chainOff ***********************************************
 *
 *   NAME
 *  chainOff -- disconnects the SAXEventKeeper from the SAX chain.
 ******************************************************************************/
{
    if (m_bIsSAXEventKeeperSticky )
        return;

    if (!m_bIsSAXEventKeeperConnected)
        return;

    m_xSAXEventKeeper->setNextHandler( nullptr );

    if ( m_xPreviousNodeOnSAXChain.is() )
    {
        if ( m_bIsPreviousNodeInitializable )
        {
            css::uno::Reference< css::lang::XInitialization > xInitialization
                (m_xPreviousNodeOnSAXChain, css::uno::UNO_QUERY);

            css::uno::Sequence<css::uno::Any> aArgs( 1 );
            aArgs[0] <<= uno::Reference<xml::sax::XDocumentHandler>();
            xInitialization->initialize(aArgs);
        }
        else
        {
            css::uno::Reference< css::xml::sax::XParser > xParser(m_xPreviousNodeOnSAXChain, css::uno::UNO_QUERY);
            xParser->setDocumentHandler(uno::Reference<xml::sax::XDocumentHandler>());
        }
    }

    m_bIsSAXEventKeeperConnected = false;
}

void XSecController::checkChainingStatus()
/****** XSecController/checkChainingStatus ************************************
 *
 *   NAME
 *  checkChainingStatus -- connects or disconnects the SAXEventKeeper
 *  according to the current situation.
 *
 *   SYNOPSIS
 *  checkChainingStatus( );
 *
 *   FUNCTION
 *  The SAXEventKeeper is chained into the SAX chain, when:
 *  1. some element is being collected, or
 *  2. the SAX event stream is blocking.
 *  Otherwise, chain off the SAXEventKeeper.
 ******************************************************************************/
{
    if ( m_bIsCollectingElement || m_bIsBlocking )
    {
        chainOn();
    }
    else
    {
        chainOff();
    }
}

void XSecController::initializeSAXChain()
/****** XSecController/initializeSAXChain *************************************
 *
 *   NAME
 *  initializeSAXChain -- initializes the SAX chain according to the
 *  current setting.
 *
 *   FUNCTION
 *  Initializes the SAX chain, if the SAXEventKeeper is asked to be always
 *  on the SAX chain, chains it on. Otherwise, starts the
 *  ElementStackKeeper to reserve key SAX events.
 ******************************************************************************/
{
    m_bIsSAXEventKeeperConnected = false;
    m_bIsCollectingElement = false;
    m_bIsBlocking = false;

    chainOff();
}

css::uno::Reference< css::io::XInputStream >
    XSecController::getObjectInputStream( const OUString& objectURL )
/****** XSecController/getObjectInputStream ************************************
 *
 *   NAME
 *  getObjectInputStream -- get a XInputStream interface from a SotStorage
 *
 *   SYNOPSIS
 *  xInputStream = getObjectInputStream( objectURL );
 *
 *   INPUTS
 *  objectURL - the object uri
 *
 *   RESULT
 *  xInputStream - the XInputStream interface
 ******************************************************************************/
{
    css::uno::Reference< css::io::XInputStream > xObjectInputStream;

    SAL_WARN_IF( !m_xUriBinding.is(), "xmlsecurity.helper", "Need XUriBinding!" );

    xObjectInputStream = m_xUriBinding->getUriBinding(objectURL);

    return xObjectInputStream;
}

/*
 * public methods
 */

sal_Int32 XSecController::getNewSecurityId(  )
{
    sal_Int32 nId = m_nNextSecurityId;
    m_nNextSecurityId++;
    return nId;
}

void XSecController::startMission(const rtl::Reference<UriBindingHelper>& xUriBinding, const css::uno::Reference< css::xml::crypto::XXMLSecurityContext >& xSecurityContext )
/****** XSecController/startMission *******************************************
 *
 *   NAME
 *  startMission -- starts a new security mission.
 *
 *   FUNCTION
 *  get ready for a new mission.
 *
 *   INPUTS
 *  xUriBinding       - the Uri binding that provide maps between uris and
 *                          XInputStreams
 *  xSecurityContext  - the security context component which can provide
 *                      cryptoken
 ******************************************************************************/
{
    m_xUriBinding = xUriBinding;

    m_eStatusOfSecurityComponents = InitializationState::UNINITIALIZED;
    m_xSecurityContext = xSecurityContext;

    m_vInternalSignatureInformations.clear();

    m_bVerifyCurrentSignature = false;
}

void XSecController::setSAXChainConnector(const css::uno::Reference< css::lang::XInitialization >& xInitialization)
/****** XSecController/setSAXChainConnector ***********************************
 *
 *   NAME
 *  setSAXChainConnector -- configures the components which will
 *  collaborate with the SAXEventKeeper on the SAX chain.
 *
 *   SYNOPSIS
 *  setSAXChainConnector(xInitialization);
 *
 *   INPUTS
 *  xInitialization     - the previous node on the SAX chain
 ******************************************************************************/
{
    m_bIsPreviousNodeInitializable = true;
    m_xPreviousNodeOnSAXChain = xInitialization;

    initializeSAXChain( );
}

void XSecController::clearSAXChainConnector()
/****** XSecController/clearSAXChainConnector *********************************
 *
 *   NAME
 *  clearSAXChainConnector -- resets the collaborating components.
 ******************************************************************************/
{
    chainOff();

    m_xPreviousNodeOnSAXChain = nullptr;
}

void XSecController::endMission()
/****** XSecController/endMission *********************************************
 *
 *   NAME
 *  endMission -- forces to end all missions
 *
 *   FUNCTION
 *  Deletes all signature information and forces all missions to an end.
 ******************************************************************************/
{
    sal_Int32 size = m_vInternalSignatureInformations.size();

    for (int i=0; i<size; ++i)
    {
        if ( m_eStatusOfSecurityComponents == InitializationState::INITIALIZED )
        /*
         * ResolvedListener only exist when the security components are created.
         */
        {
            css::uno::Reference< css::xml::crypto::sax::XMissionTaker > xMissionTaker
                ( m_vInternalSignatureInformations[i].xReferenceResolvedListener, css::uno::UNO_QUERY );

            /*
             * asks the SignatureCreator/SignatureVerifier to release
             * all resources it uses.
             */
            xMissionTaker->endMission();
        }
    }

    m_xUriBinding = nullptr;
    m_xSecurityContext = nullptr;

    /*
     * free the status change listener reference to this object
     */
    if (m_xSAXEventKeeper.is())
        m_xSAXEventKeeper->addSAXEventKeeperStatusChangeListener( nullptr );
}

namespace
{
void writeUnsignedProperties(
    const css::uno::Reference<css::xml::sax::XDocumentHandler>& xDocumentHandler,
    const SignatureInformation& signatureInfo)
{
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("Id", "idUnsignedProperties");
        xDocumentHandler->startElement("xd:UnsignedProperties", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }

    {
        xDocumentHandler->startElement("xd:UnsignedSignatureProperties", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));

        {
            xDocumentHandler->startElement("xd:CertificateValues", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));

            {
                for (const auto& i: signatureInfo.maEncapsulatedX509Certificates)
                {
                    xDocumentHandler->startElement("xd:EncapsulatedX509Certificate", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
                    xDocumentHandler->characters(i);
                    xDocumentHandler->endElement("xd:EncapsulatedX509Certificate");
                }
            }

            xDocumentHandler->endElement("xd:CertificateValues");
        }

        xDocumentHandler->endElement("xd:UnsignedSignatureProperties");
    }

    xDocumentHandler->endElement("xd:UnsignedProperties");
}

}

void XSecController::exportSignature(
    const css::uno::Reference<css::xml::sax::XDocumentHandler>& xDocumentHandler,
    const SignatureInformation& signatureInfo,
    bool bXAdESCompliantIfODF )
/****** XSecController/exportSignature ****************************************
 *
 *   NAME
 *  exportSignature -- export a signature structure to an XDocumentHandler
 *
 *   SYNOPSIS
 *  exportSignature( xDocumentHandler, signatureInfo);
 *
 *   INPUTS
 *  xDocumentHandler    - the document handler to receive the signature
 *  signatureInfo       - signature to be exported
 ******************************************************************************/
{
    const SignatureReferenceInformations& vReferenceInfors = signatureInfo.vSignatureReferenceInfors;
    SvXMLAttributeList *pAttributeList;

    /*
     * Write Signature element
     */
    pAttributeList = new SvXMLAttributeList();
    pAttributeList->AddAttribute(
        "xmlns",
        NS_XMLDSIG);

    if (!signatureInfo.ouSignatureId.isEmpty())
    {
        pAttributeList->AddAttribute(
            "Id",
            signatureInfo.ouSignatureId);
    }

    xDocumentHandler->startElement( "Signature", css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList));
    {
        /* Write SignedInfo element */
        xDocumentHandler->startElement(
            "SignedInfo",
            css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
        {
            /* Write CanonicalizationMethod element */
            pAttributeList = new SvXMLAttributeList();
            pAttributeList->AddAttribute(
                "Algorithm",
                ALGO_C14N);
            xDocumentHandler->startElement( "CanonicalizationMethod", css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList) );
            xDocumentHandler->endElement( "CanonicalizationMethod" );

            /* Write SignatureMethod element */
            pAttributeList = new SvXMLAttributeList();

            // TODO: actually roundtrip this value from parsing documentsignatures.xml - entirely
            // broken to assume this would in any way relate to the 1st reference's digest algo

            // Assume that all Reference elements use the same DigestMethod:Algorithm, and that the
            // SignatureMethod:Algorithm should be the corresponding one.
            pAttributeList->AddAttribute(
                "Algorithm",
                getSignatureURI(signatureInfo.eAlgorithmID, vReferenceInfors[0].nDigestID));
            xDocumentHandler->startElement( "SignatureMethod", css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList) );
            xDocumentHandler->endElement( "SignatureMethod" );

            /* Write Reference element */
            int j;
            int refNum = vReferenceInfors.size();

            for(j=0; j<refNum; ++j)
            {
                const SignatureReferenceInformation& refInfor = vReferenceInfors[j];

                pAttributeList = new SvXMLAttributeList();
                if ( refInfor.nType != SignatureReferenceType::SAMEDOCUMENT )
                /*
                 * stream reference
                 */
                {
                    pAttributeList->AddAttribute(
                        "URI",
                        refInfor.ouURI);
                }
                else
                /*
                 * same-document reference
                 */
                {
                    pAttributeList->AddAttribute(
                        "URI",
                        "#" + refInfor.ouURI);

                    if (bXAdESCompliantIfODF && refInfor.ouURI == "idSignedProperties" && !refInfor.ouType.isEmpty())
                    {
                        // The reference which points to the SignedProperties
                        // shall have this specific type.
                        pAttributeList->AddAttribute("Type",
                                                     refInfor.ouType);
                    }
                }

                xDocumentHandler->startElement( "Reference", css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList) );
                {
                    /* Write Transforms element */
                    if (refInfor.nType == SignatureReferenceType::XMLSTREAM)
                    /*
                     * xml stream, so c14n transform is needed
                     */
                    {
                        xDocumentHandler->startElement(
                            "Transforms",
                            css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
                        {
                            pAttributeList = new SvXMLAttributeList();
                            pAttributeList->AddAttribute(
                                "Algorithm",
                                ALGO_C14N);
                            xDocumentHandler->startElement(
                                "Transform",
                                css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList) );
                            xDocumentHandler->endElement( "Transform" );
                        }
                        xDocumentHandler->endElement( "Transforms" );
                    }

                    /* Write DigestMethod element */
                    pAttributeList = new SvXMLAttributeList();
                    pAttributeList->AddAttribute(
                        "Algorithm",
                        getDigestURI(refInfor.nDigestID));
                    xDocumentHandler->startElement(
                        "DigestMethod",
                        css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList) );
                    xDocumentHandler->endElement( "DigestMethod" );

                    /* Write DigestValue element */
                    xDocumentHandler->startElement(
                        "DigestValue",
                        css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
                    xDocumentHandler->characters( refInfor.ouDigestValue );
                    xDocumentHandler->endElement( "DigestValue" );
                }
                xDocumentHandler->endElement( "Reference" );
            }
        }
        xDocumentHandler->endElement( "SignedInfo" );

        /* Write SignatureValue element */
        xDocumentHandler->startElement(
            "SignatureValue",
            css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
        xDocumentHandler->characters( signatureInfo.ouSignatureValue );
        xDocumentHandler->endElement( "SignatureValue" );

        /* Write KeyInfo element */
        xDocumentHandler->startElement(
            "KeyInfo",
            css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
        {
            // GPG or X509 key?
            if (!signatureInfo.ouGpgCertificate.isEmpty())
            {
                pAttributeList = new SvXMLAttributeList();
                pAttributeList->AddAttribute("xmlns:loext", NS_LOEXT);
                /* Write PGPData element */
                xDocumentHandler->startElement(
                    "PGPData",
                    css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList));
                {
                    /* Write keyid element */
                    xDocumentHandler->startElement(
                        "PGPKeyID",
                        css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
                    xDocumentHandler->characters(signatureInfo.ouGpgKeyID);
                    xDocumentHandler->endElement( "PGPKeyID" );

                    /* Write PGPKeyPacket element */
                    if (!signatureInfo.ouGpgCertificate.isEmpty())
                    {
                        xDocumentHandler->startElement(
                            "PGPKeyPacket",
                            css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
                        xDocumentHandler->characters( signatureInfo.ouGpgCertificate );
                        xDocumentHandler->endElement( "PGPKeyPacket" );
                    }

                    /* Write PGPOwner element */
                    xDocumentHandler->startElement(
                        "loext:PGPOwner",
                        css::uno::Reference< css::xml::sax::XAttributeList >(new SvXMLAttributeList()));
                    xDocumentHandler->characters( signatureInfo.ouGpgOwner );
                    xDocumentHandler->endElement( "loext:PGPOwner" );
                }
                xDocumentHandler->endElement( "PGPData" );
            }
            else
            {
                assert(signatureInfo.GetSigningCertificate());
                for (auto const& rData : signatureInfo.X509Datas)
                {
                    /* Write X509Data element */
                    xDocumentHandler->startElement(
                        "X509Data",
                        css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
                    {
                        for (auto const& it : rData)
                        {
                            /* Write X509IssuerSerial element */
                            xDocumentHandler->startElement(
                                "X509IssuerSerial",
                                css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
                            {
                                /* Write X509IssuerName element */
                                xDocumentHandler->startElement(
                                    "X509IssuerName",
                                    css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
                                xDocumentHandler->characters(it.X509IssuerName);
                                xDocumentHandler->endElement( "X509IssuerName" );

                                /* Write X509SerialNumber element */
                                xDocumentHandler->startElement(
                                    "X509SerialNumber",
                                    css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
                                xDocumentHandler->characters(it.X509SerialNumber);
                                xDocumentHandler->endElement( "X509SerialNumber" );
                            }
                            xDocumentHandler->endElement( "X509IssuerSerial" );

                            /* Write X509Certificate element */
                            if (!it.X509Certificate.isEmpty())
                            {
                                xDocumentHandler->startElement(
                                    "X509Certificate",
                                    css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
                                xDocumentHandler->characters(it.X509Certificate);
                                xDocumentHandler->endElement( "X509Certificate" );
                            }
                        }
                    }
                    xDocumentHandler->endElement( "X509Data" );
                }
            }
        }
        xDocumentHandler->endElement( "KeyInfo" );

        OUString sDate;

        /* Write Object element */
        xDocumentHandler->startElement(
            "Object",
            css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
        {
            /* Write SignatureProperties element */
            xDocumentHandler->startElement(
                "SignatureProperties",
                css::uno::Reference< css::xml::sax::XAttributeList > (new SvXMLAttributeList()));
            {
                /* Write SignatureProperty element */
                pAttributeList = new SvXMLAttributeList();
                pAttributeList->AddAttribute(
                    "Id",
                    signatureInfo.ouDateTimePropertyId);
                pAttributeList->AddAttribute(
                    "Target",
                    "#" + signatureInfo.ouSignatureId);
                xDocumentHandler->startElement(
                    "SignatureProperty",
                    css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList));
                {
                    /* Write timestamp element */

                    pAttributeList = new SvXMLAttributeList();
                    pAttributeList->AddAttribute(
                        "xmlns:dc",
                        NS_DC);

                    xDocumentHandler->startElement(
                        "dc:date",
                        css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList));

                    OUStringBuffer buffer;
                    //If the xml signature was already contained in the document,
                    //then we use the original date and time string, rather than the
                    //converted one. This avoids writing a different string due to
                    //e.g. rounding issues and thus breaking the signature.
                    if (!signatureInfo.ouDateTime.isEmpty())
                        buffer = signatureInfo.ouDateTime;
                    else
                    {
                        buffer = utl::toISO8601(signatureInfo.stDateTime);
                        // xsd:dateTime must use period as separator for fractional seconds, while
                        // utl::toISO8601 uses comma (as allowed, and even recommended, by ISO8601).
                        buffer.replace(',', '.');
                    }
                    sDate = buffer.makeStringAndClear();
                    xDocumentHandler->characters( sDate );

                    xDocumentHandler->endElement(
                        "dc:date");
                }
                xDocumentHandler->endElement( "SignatureProperty" );
            }

            // Write signature description.
            if (!signatureInfo.ouDescription.isEmpty())
            {
                // SignatureProperty element.
                pAttributeList = new SvXMLAttributeList();
                pAttributeList->AddAttribute("Id", signatureInfo.ouDescriptionPropertyId);
                pAttributeList->AddAttribute("Target", "#" + signatureInfo.ouSignatureId);
                xDocumentHandler->startElement("SignatureProperty", uno::Reference<xml::sax::XAttributeList>(pAttributeList));

                {
                    // Description element.
                    pAttributeList = new SvXMLAttributeList();
                    pAttributeList->AddAttribute("xmlns:dc", NS_DC);

                    xDocumentHandler->startElement("dc:description", uno::Reference<xml::sax::XAttributeList>(pAttributeList));
                    xDocumentHandler->characters(signatureInfo.ouDescription);
                    xDocumentHandler->endElement("dc:description");
                }

                xDocumentHandler->endElement("SignatureProperty");
            }

            xDocumentHandler->endElement( "SignatureProperties" );
        }
        xDocumentHandler->endElement( "Object" );

        //  In XAdES, write another Object element for the QualifyingProperties
        if (bXAdESCompliantIfODF)
        {
            pAttributeList =  new SvXMLAttributeList();
            pAttributeList->AddAttribute("xmlns:xd", NS_XD);
            xDocumentHandler->startElement(
                "Object",
                css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList));
            {
                pAttributeList = new SvXMLAttributeList();
                pAttributeList->AddAttribute("Target", "#" + signatureInfo.ouSignatureId);
                xDocumentHandler->startElement(
                    "xd:QualifyingProperties",
                    css::uno::Reference< css::xml::sax::XAttributeList > (pAttributeList));
                DocumentSignatureHelper::writeSignedProperties(xDocumentHandler, signatureInfo, sDate, true);
                writeUnsignedProperties(xDocumentHandler, signatureInfo);
                xDocumentHandler->endElement( "xd:QualifyingProperties" );
            }
            xDocumentHandler->endElement( "Object" );
        }
    }
    xDocumentHandler->endElement( "Signature" );
}

void XSecController::exportOOXMLSignature(const uno::Reference<embed::XStorage>& xRootStorage, const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler, const SignatureInformation& rInformation)
{
    OOXMLSecExporter aExporter(mxCtx, xRootStorage, xDocumentHandler, rInformation);
    aExporter.writeSignature();
}

void XSecController::UpdateSignatureInformation(sal_Int32 const nSecurityId,
    std::vector<SignatureInformation::X509Data> const& rDatas)
{
    SignatureInformation aInf( 0 );
    int const nIndex = findSignatureInfor(nSecurityId);
    assert(nIndex != -1); // nothing should touch this between parsing and verify
    m_vInternalSignatureInformations[nIndex].signatureInfor.X509Datas = rDatas;
}

SignatureInformation XSecController::getSignatureInformation( sal_Int32 nSecurityId ) const
{
    SignatureInformation aInf( 0 );
    int nIndex = findSignatureInfor(nSecurityId);
    SAL_WARN_IF( nIndex == -1, "xmlsecurity.helper", "getSignatureInformation - SecurityId is invalid!" );
    if ( nIndex != -1)
    {
        aInf = m_vInternalSignatureInformations[nIndex].signatureInfor;
    }
    return aInf;
}

SignatureInformations XSecController::getSignatureInformations() const
{
    SignatureInformations vInfors;
    int sigNum = m_vInternalSignatureInformations.size();

    for (int i=0; i<sigNum; ++i)
    {
        SignatureInformation si = m_vInternalSignatureInformations[i].signatureInfor;
        vInfors.push_back(si);
    }

    return vInfors;
}

/*
 * XSAXEventKeeperStatusChangeListener
 */

void SAL_CALL XSecController::blockingStatusChanged( sal_Bool isBlocking )
{
    m_bIsBlocking = isBlocking;
    checkChainingStatus();
}

void SAL_CALL XSecController::collectionStatusChanged(
    sal_Bool isInsideCollectedElement )
{
    m_bIsCollectingElement = isInsideCollectedElement;
    checkChainingStatus();
}

void SAL_CALL XSecController::bufferStatusChanged( sal_Bool /*isBufferEmpty*/)
{

}

/*
 * XSignatureCreationResultListener
 */
void SAL_CALL XSecController::signatureCreated( sal_Int32 securityId, css::xml::crypto::SecurityOperationStatus nResult )
{
    int index = findSignatureInfor(securityId);
    assert(index != -1 && "Signature Not Found!");
    SignatureInformation& signatureInfor = m_vInternalSignatureInformations.at(index).signatureInfor;
    signatureInfor.nStatus = nResult;
}

/*
 * XSignatureVerifyResultListener
 */
void SAL_CALL XSecController::signatureVerified( sal_Int32 securityId, css::xml::crypto::SecurityOperationStatus nResult )
{
    int index = findSignatureInfor(securityId);
    assert(index != -1 && "Signature Not Found!");
    SignatureInformation& signatureInfor = m_vInternalSignatureInformations.at(index).signatureInfor;
    signatureInfor.nStatus = nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
