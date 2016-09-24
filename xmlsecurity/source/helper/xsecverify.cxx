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


#include "xsecctl.hxx"
#include "xsecparser.hxx"
#include "ooxmlsecparser.hxx"
#include <tools/debug.hxx>

#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultBroadcaster.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <sal/log.hxx>
#include <unotools/datetime.hxx>

using namespace com::sun::star;
namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxs = com::sun::star::xml::sax;

/* xml security framework components */
#define SIGNATUREVERIFIER_COMPONENT "com.sun.star.xml.crypto.sax.SignatureVerifier"

/* protected: for signature verify */
cssu::Reference< cssxc::sax::XReferenceResolvedListener > XSecController::prepareSignatureToRead(
    sal_Int32 nSecurityId)
{
    if ( m_nStatusOfSecurityComponents != INITIALIZED )
    {
        return nullptr;
    }

    sal_Int32 nIdOfSignatureElementCollector;
    cssu::Reference< cssxc::sax::XReferenceResolvedListener > xReferenceResolvedListener;

    nIdOfSignatureElementCollector =
        m_xSAXEventKeeper->addSecurityElementCollector( cssxc::sax::ElementMarkPriority_BEFOREMODIFY, false);

    m_xSAXEventKeeper->setSecurityId(nIdOfSignatureElementCollector, nSecurityId);

        /*
         * create a SignatureVerifier
         */
    cssu::Reference< cssl::XMultiComponentFactory > xMCF( mxCtx->getServiceManager() );
    xReferenceResolvedListener.set(
        xMCF->createInstanceWithContext(SIGNATUREVERIFIER_COMPONENT, mxCtx),
        cssu::UNO_QUERY);

    cssu::Reference<cssl::XInitialization> xInitialization(xReferenceResolvedListener, cssu::UNO_QUERY);

    cssu::Sequence<cssu::Any> args(5);
    args[0] = cssu::makeAny(OUString::number(nSecurityId));
    args[1] = cssu::makeAny(m_xSAXEventKeeper);
    args[2] = cssu::makeAny(OUString::number(nIdOfSignatureElementCollector));
    args[3] = cssu::makeAny(m_xSecurityContext);
    args[4] = cssu::makeAny(m_xXMLSignature);
    xInitialization->initialize(args);

    cssu::Reference< cssxc::sax::XSignatureVerifyResultBroadcaster >
        signatureVerifyResultBroadcaster(xReferenceResolvedListener, cssu::UNO_QUERY);

    signatureVerifyResultBroadcaster->addSignatureVerifyResultListener( this );

    cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster> xReferenceResolvedBroadcaster
        (m_xSAXEventKeeper,
        cssu::UNO_QUERY);

    xReferenceResolvedBroadcaster->addReferenceResolvedListener(
        nIdOfSignatureElementCollector,
        xReferenceResolvedListener);

    cssu::Reference<cssxc::sax::XKeyCollector> keyCollector (xReferenceResolvedListener, cssu::UNO_QUERY);
    keyCollector->setKeyId(0);

    return xReferenceResolvedListener;
}

void XSecController::addSignature()
{
    cssu::Reference< cssxc::sax::XReferenceResolvedListener > xReferenceResolvedListener = nullptr;
    sal_Int32 nSignatureId = 0;


    if (m_bVerifyCurrentSignature)
    {
        chainOn(true);
        xReferenceResolvedListener = prepareSignatureToRead( m_nReservedSignatureId );
        m_bVerifyCurrentSignature = false;
        nSignatureId = m_nReservedSignatureId;
    }

    InternalSignatureInformation isi( nSignatureId, xReferenceResolvedListener );
    m_vInternalSignatureInformations.push_back( isi );
}

void XSecController::addReference( const OUString& ouUri)
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::addReference: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.addReference(SignatureReferenceType::SAMEDOCUMENT,ouUri, -1 );
}

void XSecController::addStreamReference(
    const OUString& ouUri,
    bool isBinary )
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
            cssu::Reference< css::io::XInputStream > xObjectInputStream
                = getObjectInputStream( ouUri );

        if ( xObjectInputStream.is() )
        {
            cssu::Reference<cssxc::XUriBinding> xUriBinding
                (isi.xReferenceResolvedListener, cssu::UNO_QUERY);
            xUriBinding->setUriBinding(ouUri, xObjectInputStream);
        }
    }

    isi.addReference(type, ouUri, -1);
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

    if ( isi.xReferenceResolvedListener.is() )
    {
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

        cssu::Reference<cssxc::sax::XReferenceCollector> xReferenceCollector
            (isi.xReferenceResolvedListener, cssu::UNO_QUERY);
        xReferenceCollector->setReferenceCount( referenceCount );
    }
}

void XSecController::setX509IssuerName( OUString& ouX509IssuerName )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setX509IssuerName: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouX509IssuerName = ouX509IssuerName;
}

void XSecController::setX509SerialNumber( OUString& ouX509SerialNumber )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setX509SerialNumber: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouX509SerialNumber = ouX509SerialNumber;
}

void XSecController::setX509Certificate( OUString& ouX509Certificate )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setX509Certificate: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouX509Certificate = ouX509Certificate;
}

void XSecController::setSignatureValue( OUString& ouSignatureValue )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setSignatureValue: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouSignatureValue = ouSignatureValue;
}

void XSecController::setDigestValue( OUString& ouDigestValue )
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
    reference.ouDigestValue = ouDigestValue;
}

void XSecController::setDate( OUString& ouDate )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setDate: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    utl::ISO8601parseDateTime( ouDate, isi.signatureInfor.stDateTime);
    isi.signatureInfor.ouDateTime = ouDate;
}

void XSecController::setDescription(const OUString& rDescription)
{
    if (m_vInternalSignatureInformations.empty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    rInformation.signatureInfor.ouDescription = rDescription;
}

void XSecController::setSignatureBytes(const uno::Sequence<sal_Int8>& rBytes)
{
    if (m_vInternalSignatureInformations.empty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    rInformation.signatureInfor.aSignatureBytes = rBytes;
}

void XSecController::setCertDigest(const OUString& rCertDigest)
{
    if (m_vInternalSignatureInformations.empty())
        return;

    InternalSignatureInformation& rInformation = m_vInternalSignatureInformations.back();
    rInformation.signatureInfor.ouCertDigest = rCertDigest;
}

void XSecController::setId( OUString& ouId )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setId: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();
    isi.signatureInfor.ouSignatureId = ouId;
}

void XSecController::setPropertyId( OUString& ouPropertyId )
{
    if (m_vInternalSignatureInformations.empty())
    {
        SAL_INFO("xmlsecurity.helper","XSecController::setPropertyId: no signature");
        return;
    }
    InternalSignatureInformation &isi = m_vInternalSignatureInformations.back();

    if (isi.signatureInfor.ouPropertyId.isEmpty())
    {
        // <SignatureProperty> ID attribute is for the date.
        isi.signatureInfor.ouPropertyId = ouPropertyId;
    }
    else
    {
        // <SignatureProperty> ID attribute is for the description.
        isi.signatureInfor.ouDescriptionPropertyId = ouPropertyId;
    }
}

/* public: for signature verify */
void XSecController::collectToVerify( const OUString& referenceId )
{
    /* SAL_WARN_IF( !m_xSAXEventKeeper.is(), "xmlsecurity", "the SAXEventKeeper is NULL" ); */

    if ( m_nStatusOfSecurityComponents == INITIALIZED )
    /*
     * if all security components are ready, verify the signature.
     */
    {
        bool bJustChainingOn = false;
        cssu::Reference< cssxs::XDocumentHandler > xHandler = nullptr;

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
                    if (chainOn(false))
                    {
                        bJustChainingOn = true;
                        xHandler = m_xSAXEventKeeper->setNextHandler(nullptr);
                    }

                    sal_Int32 nKeeperId = m_xSAXEventKeeper->addSecurityElementCollector(
                        cssxc::sax::ElementMarkPriority_BEFOREMODIFY, false );

                    cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster> xReferenceResolvedBroadcaster
                        (m_xSAXEventKeeper,
                        cssu::UNO_QUERY );

                    cssu::Reference<cssxc::sax::XReferenceCollector> xReferenceCollector
                        ( isi.xReferenceResolvedListener, cssu::UNO_QUERY );

                    m_xSAXEventKeeper->setSecurityId(nKeeperId, isi.signatureInfor.nSecurityId);
                    xReferenceResolvedBroadcaster->addReferenceResolvedListener( nKeeperId, isi.xReferenceResolvedListener);
                    xReferenceCollector->setReferenceId( nKeeperId );

                    isi.vKeeperIds[j] = nKeeperId;
                    break;
                }
            }
        }

        if ( bJustChainingOn )
        {
            cssu::Reference< cssxs::XDocumentHandler > xSEKHandler(m_xSAXEventKeeper, cssu::UNO_QUERY);
            if (m_xElementStackKeeper.is())
            {
                m_xElementStackKeeper->retrieve(xSEKHandler, true);
            }
            m_xSAXEventKeeper->setNextHandler(xHandler);
        }
    }
}

void XSecController::addSignature( sal_Int32 nSignatureId )
{
    SAL_WARN_IF( !m_xSecParser.is(), "xmlsecurity.helper", "No XSecParser initialized" );

    m_nReservedSignatureId = nSignatureId;
    m_bVerifyCurrentSignature = true;
}

cssu::Reference< cssxs::XDocumentHandler > const & XSecController::createSignatureReader(sal_Int32 nType)
{
    if (nType == embed::StorageFormats::OFOPXML)
        m_xSecParser = new OOXMLSecParser(this);
    else
        m_xSecParser = new XSecParser( this, nullptr );
    cssu::Reference< cssl::XInitialization > xInitialization(m_xSecParser, uno::UNO_QUERY);

    setSAXChainConnector(xInitialization, nullptr, nullptr);

    return m_xSecParser;
}

void XSecController::releaseSignatureReader()
{
    clearSAXChainConnector( );
    m_xSecParser.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
