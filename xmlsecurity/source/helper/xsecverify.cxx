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

#include <xsecctl.hxx>
#include "xsecparser.hxx"
#include "ooxmlsecparser.hxx"
#include <biginteger.hxx>
#include <framework/signatureverifierimpl.hxx>
#include <framework/saxeventkeeperimpl.hxx>
#include <gpg/xmlsignature_gpgimpl.hxx>
#include <gpg/SEInitializer.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <sal/log.hxx>
#include <unotools/datetime.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>

namespace com::sun::star::graphic { class XGraphic; }

using namespace css;
using namespace css::uno;
using namespace css::beans;

/* protected: for signature verify */
css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener > XSecController::prepareSignatureToRead(
    sal_Int32 nSecurityId)
{
    if ( m_eStatusOfSecurityComponents != InitializationState::INITIALIZED )
    {
        return nullptr;
    }

    sal_Int32 nIdOfSignatureElementCollector;
    css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener > xReferenceResolvedListener;

    nIdOfSignatureElementCollector =
        m_xSAXEventKeeper->addSecurityElementCollector( css::xml::crypto::sax::ElementMarkPriority_BEFOREMODIFY, false);

    m_xSAXEventKeeper->setSecurityId(nIdOfSignatureElementCollector, nSecurityId);

        /*
         * create a SignatureVerifier
         */
    xReferenceResolvedListener = new SignatureVerifierImpl;

    css::uno::Reference<css::lang::XInitialization> xInitialization(xReferenceResolvedListener, css::uno::UNO_QUERY);

    css::uno::Sequence<css::uno::Any> args(5);
    args[0] <<= OUString::number(nSecurityId);
    args[1] <<= uno::Reference<xml::crypto::sax::XSecuritySAXEventKeeper>(static_cast<cppu::OWeakObject*>(m_xSAXEventKeeper.get()), uno::UNO_QUERY);
    args[2] <<= OUString::number(nIdOfSignatureElementCollector);
    args[3] <<= m_xSecurityContext;
    args[4] <<= m_xXMLSignature;
    xInitialization->initialize(args);

    css::uno::Reference< css::xml::crypto::sax::XSignatureVerifyResultBroadcaster >
        signatureVerifyResultBroadcaster(xReferenceResolvedListener, css::uno::UNO_QUERY);

    signatureVerifyResultBroadcaster->addSignatureVerifyResultListener( this );

    m_xSAXEventKeeper->addReferenceResolvedListener(
        nIdOfSignatureElementCollector,
        xReferenceResolvedListener);

    css::uno::Reference<css::xml::crypto::sax::XKeyCollector> keyCollector (xReferenceResolvedListener, css::uno::UNO_QUERY);
    keyCollector->setKeyId(0);

    return xReferenceResolvedListener;
}

void XSecController::addSignature()
{
    css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener > xReferenceResolvedListener;
    sal_Int32 nSignatureId = 0;


    if (m_bVerifyCurrentSignature)
    {
        chainOn();
        xReferenceResolvedListener = prepareSignatureToRead( m_nReservedSignatureId );
        m_bVerifyCurrentSignature = false;
        nSignatureId = m_nReservedSignatureId;
    }

    InternalSignatureInformation isi( nSignatureId, xReferenceResolvedListener );
    m_vInternalSignatureInformations.push_back( isi );
}

void XSecController::setSignatureMethod(svl::crypto::SignatureMethodAlgorithm eAlgorithmID)
{
    if (m_vInternalSignatureInformations.empty())
        return;

    m_vInternalSignatureInformations.back().signatureInfor.eAlgorithmID = eAlgorithmID;
}

void XSecController::switchGpgSignature()
{
#if HAVE_FEATURE_GPGME
    // swap signature verifier for the Gpg one
    m_xXMLSignature.set(new XMLSignature_GpgImpl());
    if (m_vInternalSignatureInformations.empty())
        return;

    SignatureVerifierImpl* pImpl=
        dynamic_cast<SignatureVerifierImpl*>(
            m_vInternalSignatureInformations.back().xReferenceResolvedListener.get());
    if (pImpl)
    {
        css::uno::Reference<css::xml::crypto::XSEInitializer> xGpgSEInitializer(
            new SEInitializerGpg());
        pImpl->updateSignature(new XMLSignature_GpgImpl(),
                               xGpgSEInitializer->createSecurityContext(OUString()));
    }
#else
    (void) this;
#endif
}

bool XSecController::haveReferenceForId(std::u16string_view rId) const
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::haveReferenceForId: no signature");
        return false;
    }
    InternalSignatureInformation const& rIsi(m_vInternalSignatureInformations.back());
    for (SignatureReferenceInformation const& rSri : rIsi.signatureInfor.vSignatureReferenceInfors)
    {
        if (rSri.nType == SignatureReferenceType::SAMEDOCUMENT
            && rSri.ouURI == rId) // ouUri has # stripped
        {
            return true;
        }
    }
    return false;
}

void XSecController::addReference( const OUString& ouUri, sal_Int32 nDigestID, const OUString& ouType )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::addReference: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.addReference(SignatureReferenceType::SAMEDOCUMENT, nDigestID, ouUri, -1, ouType );
}

void XSecController::addStreamReference(
    const OUString& ouUri,
    bool isBinary,
    sal_Int32 nDigestID )
{
    SignatureReferenceType type = (isBinary?SignatureReferenceType::BINARYSTREAM:SignatureReferenceType::XMLSTREAM);

    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::addStreamReference: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();

    if ( isi.xReferenceResolvedListener.is() )
    {
            /*
             * get the input stream
             */
        css::uno::Reference< css::io::XInputStream > xObjectInputStream
                = getObjectInputStream( ouUri );

        if ( xObjectInputStream.is() )
        {
            css::uno::Reference<css::xml::crypto::XUriBinding> xUriBinding
                (isi.xReferenceResolvedListener, css::uno::UNO_QUERY);
            xUriBinding->setUriBinding(ouUri, xObjectInputStream);
        }
    }

    isi.addReference(type, nDigestID, ouUri, -1, OUString());
}

void XSecController::setReferenceCount() const
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setReferenceCount: no signature");
        return;
    }
    const InternalSignatureInformation &isi =
        m_vInternalSignatureInformations.back();

    if ( !isi.xReferenceResolvedListener.is() )
        return;

    const SignatureReferenceInformations &refInfors = isi.signatureInfor.vSignatureReferenceInfors;

    int refNum = refInfors.size();
    sal_Int32 referenceCount = 0;

    for(int i=0 ; i<refNum; ++i)
    {
        if (refInfors[i].nType == SignatureReferenceType::SAMEDOCUMENT )
        /*
         * same-document reference
         */
        {
            referenceCount++;
        }
    }

    css::uno::Reference<css::xml::crypto::sax::XReferenceCollector> xReferenceCollector
        (isi.xReferenceResolvedListener, css::uno::UNO_QUERY);
    xReferenceCollector->setReferenceCount( referenceCount );
}

void XSecController::setX509Data(SignatureInformation::X509Data const& rData)
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setX509IssuerName: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    // TODO: ImplVerifySignatures() handles all-empty case?
    isi.signatureInfor.X509Datas.push_back(rData);
}

void XSecController::setSignatureValue( OUString const & ouSignatureValue )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setSignatureValue: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouSignatureValue = ouSignatureValue;
}

void XSecController::setDigestValue( sal_Int32 nDigestID, OUString const & ouDigestValue )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setDigestValue: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    if (isi.signatureInfor.vSignatureReferenceInfors.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setDigestValue: no signature reference");
        return;
    }
    SignatureReferenceInformation &reference =
        isi.signatureInfor.vSignatureReferenceInfors.back();
    reference.nDigestID = nDigestID;
    reference.ouDigestValue = ouDigestValue;
}

void XSecController::setGpgKeyID( OUString const & ouKeyID )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setGpgKeyID: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouGpgKeyID = ouKeyID;
}

void XSecController::setGpgCertificate( OUString const & ouGpgCert )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setGpgCertificate: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouGpgCertificate = ouGpgCert;
}

void XSecController::setGpgOwner( OUString const & ouGpgOwner )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setGpgOwner: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouGpgOwner = ouGpgOwner;
}

void XSecController::setDate(OUString const& rId, OUString const& ouDate)
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setDate: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    // there may be multiple timestamps in a signature - check them for consistency
    if (!isi.signatureInfor.ouDateTime.isEmpty()
        && isi.signatureInfor.ouDateTime != ouDate)
    {
        isi.signatureInfor.hasInconsistentSigningTime = true;
    }
    (void)utl::ISO8601parseDateTime( ouDate, isi.signatureInfor.stDateTime);
    isi.signatureInfor.ouDateTime = ouDate;
    if (!rId.isEmpty())
    {
        isi.signatureInfor.ouDateTimePropertyId = rId;
    }
}

void XSecController::setDescription(OUString const& rId, OUString const& rDescription)
{
    if (m_vInternalSignatureInformations.empty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    rInformation.signatureInfor.ouDescription = rDescription;
    if (!rId.isEmpty())
    {
        rInformation.signatureInfor.ouDescriptionPropertyId = rId;
    }
}

void XSecController::setSignatureBytes(const uno::Sequence<sal_Int8>& rBytes)
{
    if (m_vInternalSignatureInformations.empty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    rInformation.signatureInfor.aSignatureBytes = rBytes;
}

void XSecController::setX509CertDigest(
    OUString const& rCertDigest, sal_Int32 const /*TODO nReferenceDigestID*/,
    std::u16string_view const& rX509IssuerName, std::u16string_view const& rX509SerialNumber)
{
    if (m_vInternalSignatureInformations.empty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    for (auto & it : rInformation.signatureInfor.X509Datas)
    {
        if (xmlsecurity::EqualDistinguishedNames(it.X509IssuerName, rX509IssuerName)
            && it.X509SerialNumber == rX509SerialNumber)
        {
            it.CertDigest = rCertDigest;
            return;
        }
    }
    // fall-back: read the actual certificates
    for (auto & it : rInformation.signatureInfor.X509Datas)
    {
        if (!it.X509Certificate.isEmpty())
        {
            try
            {
                uno::Reference<xml::crypto::XSecurityEnvironment> const xSecEnv(m_xSecurityContext->getSecurityEnvironment());
                uno::Reference<security::XCertificate> const xCert(xSecEnv->createCertificateFromAscii(it.X509Certificate));
                if (!xCert.is())
                {
                    SAL_INFO("xmlsecurity.helper", "cannot parse X509Certificate");
                }
                else if (xmlsecurity::EqualDistinguishedNames(xCert->getIssuerName(),rX509IssuerName)
                    && xmlsecurity::bigIntegerToNumericString(xCert->getSerialNumber()) == rX509SerialNumber)
                {
                    it.CertDigest = rCertDigest;
                    // note: testInsertCertificate_PEM_DOCX requires these!
                    it.X509SerialNumber = rX509SerialNumber;
                    it.X509IssuerName = rX509IssuerName;
                    return;
                }
            }
            catch (uno::Exception const&)
            {
                SAL_INFO("xmlsecurity.helper", "cannot parse X509Certificate");
            }
        }
    }
    if (!rInformation.signatureInfor.ouGpgCertificate.isEmpty())
    {
        SAL_INFO_IF(rCertDigest != rInformation.signatureInfor.ouGpgKeyID,
            "xmlsecurity.helper", "PGPKeyID vs CertDigest mismatch");
    }
    else
    {
        SAL_INFO("xmlsecurity.helper", "cannot find X509Data for CertDigest");
    }
}

namespace {
Reference<css::graphic::XGraphic> lcl_getGraphicFromString(const OUString& rImage)
{
    Sequence<sal_Int8> seq;
    comphelper::Base64::decode(seq, rImage);

    Reference< graphic::XGraphic > xGraphic;
    if( !seq.hasElements() )
        return Reference<css::graphic::XGraphic>();

    Reference< graphic::XGraphicProvider > xGraphicProvider(
        graphic::GraphicProvider::create(comphelper::getProcessComponentContext()) );
    Reference< io::XInputStream > xInputStream( new ::comphelper::SequenceInputStream( seq ) );

    Sequence< PropertyValue > aArgs( 1 );
    aArgs[ 0 ].Name = "InputStream";
    aArgs[ 0 ].Value <<= xInputStream;
    xGraphic = xGraphicProvider->queryGraphic(aArgs);

    return xGraphic;
}
}

void XSecController::setValidSignatureImage(const OUString& rValidSigImg)
{
    if (m_vInternalSignatureInformations.empty() || rValidSigImg.isEmpty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    rInformation.signatureInfor.aValidSignatureImage = lcl_getGraphicFromString(rValidSigImg);
}

void XSecController::setInvalidSignatureImage(const OUString& rInvalidSigImg)
{
    if (m_vInternalSignatureInformations.empty() || rInvalidSigImg.isEmpty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    rInformation.signatureInfor.aInvalidSignatureImage = lcl_getGraphicFromString(rInvalidSigImg);
}

void XSecController::setSignatureLineId(const OUString& rSignatureLineId)
{
    if (m_vInternalSignatureInformations.empty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    rInformation.signatureInfor.ouSignatureLineId = rSignatureLineId;
}

void XSecController::addEncapsulatedX509Certificate(const OUString& rEncapsulatedX509Certificate)
{
    if (m_vInternalSignatureInformations.empty())
        return;

    if (rEncapsulatedX509Certificate.isEmpty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    rInformation.signatureInfor.maEncapsulatedX509Certificates.insert(rEncapsulatedX509Certificate);
}

void XSecController::setId( OUString const & ouId )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setId: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouSignatureId = ouId;
}

/* public: for signature verify */
void XSecController::collectToVerify( std::u16string_view referenceId )
{
    /* SAL_WARN_IF( !m_xSAXEventKeeper.is(), "xmlsecurity", "the SAXEventKeeper is NULL" ); */

    if ( m_eStatusOfSecurityComponents != InitializationState::INITIALIZED )
    /*
     * if all security components are ready, verify the signature.
     */
        return;

    bool bJustChainingOn = false;
    css::uno::Reference< css::xml::sax::XDocumentHandler > xHandler;

    int i,j;
    int sigNum = m_vInternalSignatureInformations.size();

    for (i=0; i<sigNum; ++i)
    {
        InternalSignatureInformation& isi = m_vInternalSignatureInformations[i];
        SignatureReferenceInformations& vReferenceInfors = isi.signatureInfor.vSignatureReferenceInfors;
        int refNum = vReferenceInfors.size();

        for (j=0; j<refNum; ++j)
        {
            SignatureReferenceInformation &refInfor = vReferenceInfors[j];

            if (refInfor.ouURI == referenceId)
            {
                if (chainOn())
                {
                    bJustChainingOn = true;
                    xHandler = m_xSAXEventKeeper->setNextHandler(nullptr);
                }

                sal_Int32 nKeeperId = m_xSAXEventKeeper->addSecurityElementCollector(
                    css::xml::crypto::sax::ElementMarkPriority_BEFOREMODIFY, false );

                css::uno::Reference<css::xml::crypto::sax::XReferenceCollector> xReferenceCollector
                    ( isi.xReferenceResolvedListener, css::uno::UNO_QUERY );

                m_xSAXEventKeeper->setSecurityId(nKeeperId, isi.signatureInfor.nSecurityId);
                m_xSAXEventKeeper->addReferenceResolvedListener( nKeeperId, isi.xReferenceResolvedListener);
                xReferenceCollector->setReferenceId( nKeeperId );

                isi.vKeeperIds[j] = nKeeperId;
                break;
            }
        }
    }

    if ( bJustChainingOn )
    {
        m_xSAXEventKeeper->setNextHandler(xHandler);
    }
}

void XSecController::addSignature( sal_Int32 nSignatureId )
{
    SAL_WARN_IF( !m_xSecParser.is(), "xmlsecurity.helper", "No XSecParser initialized" );

    m_nReservedSignatureId = nSignatureId;
    m_bVerifyCurrentSignature = true;
}

css::uno::Reference< css::xml::sax::XDocumentHandler > const & XSecController::createSignatureReader(XMLSignatureHelper& rXMLSignatureHelper, sal_Int32 nType)
{
    if (nType == embed::StorageFormats::OFOPXML)
        m_xSecParser = new OOXMLSecParser(rXMLSignatureHelper, this);
    else
        m_xSecParser = new XSecParser(rXMLSignatureHelper, this);
    css::uno::Reference< css::lang::XInitialization > xInitialization(m_xSecParser, uno::UNO_QUERY);

    setSAXChainConnector(xInitialization);

    return m_xSecParser;
}

void XSecController::releaseSignatureReader()
{
    clearSAXChainConnector( );
    m_xSecParser.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
