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

#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultBroadcaster.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <rtl/uuid.h>

#include "framework/signaturecreatorimpl.hxx"
#include "framework/saxeventkeeperimpl.hxx"

using namespace com::sun::star;
namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxs = com::sun::star::xml::sax;

/* protected: for signature generation */
OUString XSecController::createId()
{
    sal_uInt8 aSeq[16];
    rtl_createUuid( aSeq, nullptr, true );

    char str[68]="ID_";
    int length = 3;
    for (sal_uInt8 i : aSeq)
    {
        length += sprintf(str+length, "%04x", i);
    }

    return OUString::createFromAscii(str);
}

cssu::Reference< cssxc::sax::XReferenceResolvedListener > XSecController::prepareSignatureToWrite(
    InternalSignatureInformation& internalSignatureInfor,
    sal_Int32 nStorageFormat,
    bool bXAdESCompliantIfODF)
{
    sal_Int32 nSecurityId = internalSignatureInfor.signatureInfor.nSecurityId;
    SignatureReferenceInformations& vReferenceInfors = internalSignatureInfor.signatureInfor.vSignatureReferenceInfors;

    sal_Int32 nIdOfSignatureElementCollector;

    nIdOfSignatureElementCollector =
        m_xSAXEventKeeper->addSecurityElementCollector( cssxc::sax::ElementMarkPriority_AFTERMODIFY, true );

    m_xSAXEventKeeper->setSecurityId(nIdOfSignatureElementCollector, nSecurityId);

    uno::Reference<xml::crypto::sax::XReferenceResolvedListener> xReferenceResolvedListener(new SignatureCreatorImpl);

    cssu::Reference<cssl::XInitialization> xInitialization(xReferenceResolvedListener, cssu::UNO_QUERY);

    cssu::Sequence<cssu::Any> args(5);
    args[0] <<= OUString::number(nSecurityId);
    args[1] <<= uno::Reference<xml::crypto::sax::XSecuritySAXEventKeeper>(static_cast<cppu::OWeakObject*>(m_xSAXEventKeeper.get()), uno::UNO_QUERY);
    args[2] <<= OUString::number(nIdOfSignatureElementCollector);

    //for nss, the internal module is used for signing, which needs to be improved later
    args[3] <<= m_xSecurityContext->getSecurityEnvironment();

    args[4] <<= m_xXMLSignature;
    xInitialization->initialize(args);

    sal_Int32 nBlockerId = m_xSAXEventKeeper->addBlocker();
    m_xSAXEventKeeper->setSecurityId(nBlockerId, nSecurityId);

    cssu::Reference<cssxc::sax::XBlockerMonitor> xBlockerMonitor(xReferenceResolvedListener, cssu::UNO_QUERY);
    xBlockerMonitor->setBlockerId(nBlockerId);

    cssu::Reference< cssxc::sax::XSignatureCreationResultBroadcaster >
        xSignatureCreationResultBroadcaster(xReferenceResolvedListener, cssu::UNO_QUERY);

    xSignatureCreationResultBroadcaster->addSignatureCreationResultListener( this );

    m_xSAXEventKeeper->addReferenceResolvedListener(
        nIdOfSignatureElementCollector,
        xReferenceResolvedListener);

    cssu::Reference<cssxc::sax::XReferenceCollector> xReferenceCollector
        (xReferenceResolvedListener, cssu::UNO_QUERY);

    int i;
    int size = vReferenceInfors.size();
    sal_Int32 nReferenceCount = 0;

    for(i=0; i<size; ++i)
    {
        sal_Int32 keeperId = internalSignatureInfor.vKeeperIds[i];

        if ( keeperId != -1)
        {
            m_xSAXEventKeeper->setSecurityId(keeperId, nSecurityId);
            m_xSAXEventKeeper->addReferenceResolvedListener( keeperId, xReferenceResolvedListener);
            xReferenceCollector->setReferenceId( keeperId );
            nReferenceCount++;
        }
    }

    xReferenceCollector->setReferenceCount( nReferenceCount );

    /*
     * adds all URI binding
     */
    cssu::Reference<cssxc::XUriBinding> xUriBinding
        (xReferenceResolvedListener, cssu::UNO_QUERY);

    for(i=0; i<size; ++i)
    {
        const SignatureReferenceInformation& refInfor = vReferenceInfors[i];

        cssu::Reference< css::io::XInputStream > xInputStream
            = getObjectInputStream( refInfor.ouURI );

        if (xInputStream.is())
        {
            xUriBinding->setUriBinding(refInfor.ouURI,xInputStream);
        }
    }

    cssu::Reference<cssxc::sax::XKeyCollector> keyCollector (xReferenceResolvedListener, cssu::UNO_QUERY);
    keyCollector->setKeyId(0);

    const sal_Int32 digestID = bXAdESCompliantIfODF ? cssxc::DigestID::SHA256 : cssxc::DigestID::SHA1;

    if (nStorageFormat != embed::StorageFormats::OFOPXML)
    {
        internalSignatureInfor.signatureInfor.ouSignatureId = createId();
        internalSignatureInfor.signatureInfor.ouPropertyId = createId();
        internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, internalSignatureInfor.signatureInfor.ouPropertyId, -1 );
        size++;

        if (bXAdESCompliantIfODF)
        {
            internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, "idSignedProperties", -1);
            size++;
        }

        if (!internalSignatureInfor.signatureInfor.ouDescription.isEmpty())
        {
            // Only mention the hash of the description in the signature if it's non-empty.
            internalSignatureInfor.signatureInfor.ouDescriptionPropertyId = createId();
            internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, internalSignatureInfor.signatureInfor.ouDescriptionPropertyId, -1);
            size++;
        }
    }
    else
    {
        internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, "idPackageObject", -1);
        size++;
        internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, "idOfficeObject", -1);
        size++;
        internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, "idSignedProperties", -1);
        size++;
    }

    /*
     * replace both digestValues and signatureValue to " "
     */
    for(i=0; i<size; ++i)
    {
        SignatureReferenceInformation& refInfor = vReferenceInfors[i];
        refInfor.ouDigestValue = " ";
    }

    internalSignatureInfor.signatureInfor.ouSignatureValue = " ";

    return xReferenceResolvedListener;
}

void XSecController::signAStream( sal_Int32 securityId, const OUString& uri, bool isBinary, bool bXAdESCompliantIfODF)
{
    const SignatureReferenceType type = isBinary ? SignatureReferenceType::BINARYSTREAM : SignatureReferenceType::XMLSTREAM;
    const sal_Int32 digestID = bXAdESCompliantIfODF ? cssxc::DigestID::SHA256 : cssxc::DigestID::SHA1;

    int index = findSignatureInfor( securityId );

    if (index == -1)
    {
        InternalSignatureInformation isi(securityId, nullptr);
        isi.addReference(type, digestID, uri, -1);
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        m_vInternalSignatureInformations[index].addReference(type, digestID, uri, -1);
    }
}

void XSecController::setX509Certificate(
    sal_Int32 nSecurityId,
    const OUString& ouX509IssuerName,
    const OUString& ouX509SerialNumber,
    const OUString& ouX509Cert,
    const OUString& ouX509CertDigest)
{
    int index = findSignatureInfor( nSecurityId );

    if ( index == -1 )
    {
        InternalSignatureInformation isi(nSecurityId, nullptr);
        isi.signatureInfor.ouX509IssuerName = ouX509IssuerName;
        isi.signatureInfor.ouX509SerialNumber = ouX509SerialNumber;
        isi.signatureInfor.ouX509Certificate = ouX509Cert;
        isi.signatureInfor.ouCertDigest = ouX509CertDigest;
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        SignatureInformation &si
            = m_vInternalSignatureInformations[index].signatureInfor;
        si.ouX509IssuerName = ouX509IssuerName;
        si.ouX509SerialNumber = ouX509SerialNumber;
        si.ouX509Certificate = ouX509Cert;
        si.ouCertDigest = ouX509CertDigest;
    }
}

void XSecController::setGpgCertificate(
        sal_Int32 nSecurityId,
        const OUString& ouCertDigest,
        const OUString& ouCert,
        const OUString& ouOwner)
{
    int index = findSignatureInfor( nSecurityId );

    if ( index == -1 )
    {
        InternalSignatureInformation isi(nSecurityId, nullptr);
        isi.signatureInfor.ouGpgCertificate = ouCert;
        isi.signatureInfor.ouGpgOwner = ouOwner;
        isi.signatureInfor.ouCertDigest = ouCertDigest;
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        SignatureInformation &si
            = m_vInternalSignatureInformations[index].signatureInfor;
        si.ouGpgCertificate = ouCert;
        si.ouGpgOwner = ouOwner;
        si.ouCertDigest = ouCertDigest;
    }
}

void XSecController::setDate(
    sal_Int32 nSecurityId,
    const css::util::DateTime& rDateTime )
{
    int index = findSignatureInfor( nSecurityId );

    if ( index == -1 )
    {
        InternalSignatureInformation isi(nSecurityId, nullptr);
        isi.signatureInfor.stDateTime = rDateTime;
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        SignatureInformation &si
            = m_vInternalSignatureInformations[index].signatureInfor;
        si.stDateTime = rDateTime;
    }
}

void XSecController::setDescription(sal_Int32 nSecurityId, const OUString& rDescription)
{
    int nIndex = findSignatureInfor(nSecurityId);

    if (nIndex == -1)
    {
        InternalSignatureInformation aInformation(nSecurityId, nullptr);
        aInformation.signatureInfor.ouDescription = rDescription;
        m_vInternalSignatureInformations.push_back(aInformation);
    }
    else
    {
        SignatureInformation& rInformation = m_vInternalSignatureInformations[nIndex].signatureInfor;
        rInformation.ouDescription = rDescription;
    }
}

bool XSecController::WriteSignature(
    const cssu::Reference<cssxs::XDocumentHandler>& xDocumentHandler,
    bool bXAdESCompliantIfODF )
{
    bool rc = false;

    SAL_WARN_IF( !xDocumentHandler.is(), "xmlsecurity.helper", "I really need a document handler!" );

    /*
     * chain the SAXEventKeeper to the SAX chain
     */
    chainOn(true);

    if ( m_eStatusOfSecurityComponents == InitializationState::INITIALIZED )
    /*
     * if all security components are ready, add the signature
     * stream.
     */
    {
        m_bIsSAXEventKeeperSticky = true;
        m_xSAXEventKeeper->setNextHandler(xDocumentHandler);

        try
        {
            /*
             * export the signature template
             */
            cssu::Reference<cssxs::XDocumentHandler> xSEKHandler(static_cast<cppu::OWeakObject*>(m_xSAXEventKeeper.get()),cssu::UNO_QUERY);

            int i;
            int sigNum = m_vInternalSignatureInformations.size();

            for (i=0; i<sigNum; ++i)
            {
                InternalSignatureInformation &isi = m_vInternalSignatureInformations[i];

                // Prepare the signature creator.
                // 0 is not a documented value of embed::StorageFormats, ugh
                isi.xReferenceResolvedListener = prepareSignatureToWrite( isi, 0, bXAdESCompliantIfODF );

                exportSignature( xSEKHandler, isi.signatureInfor, bXAdESCompliantIfODF );
            }

            m_bIsSAXEventKeeperSticky = false;
            chainOff();

            rc = true;
        }
        catch( cssu::Exception& )
        {
        }

        m_xSAXEventKeeper->setNextHandler( nullptr );
        m_bIsSAXEventKeeperSticky = false;
    }

    return rc;
}

bool XSecController::WriteOOXMLSignature(const uno::Reference<embed::XStorage>& xRootStorage, const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler)
{
    bool bRet = false;

    SAL_WARN_IF(!xDocumentHandler.is(), "xmlsecurity.helper", "empty xDocumentHandler reference");

    // Chain the SAXEventKeeper to the SAX chain.
    chainOn(/*bRetrievingLastEvent=*/true);

    if (m_eStatusOfSecurityComponents == InitializationState::INITIALIZED)
    {
        m_bIsSAXEventKeeperSticky = true;
        m_xSAXEventKeeper->setNextHandler(xDocumentHandler);

        try
        {
            // Export the signature template.
            cssu::Reference<xml::sax::XDocumentHandler> xSEKHandler(static_cast<cppu::OWeakObject*>(m_xSAXEventKeeper.get()), uno::UNO_QUERY);

            for (InternalSignatureInformation & rInformation : m_vInternalSignatureInformations)
            {
                // Prepare the signature creator.
                rInformation.xReferenceResolvedListener = prepareSignatureToWrite(rInformation, embed::StorageFormats::OFOPXML, false);

                exportOOXMLSignature(xRootStorage, xSEKHandler, rInformation.signatureInfor);
            }

            m_bIsSAXEventKeeperSticky = false;
            chainOff();

            bRet = true;
        }
        catch(const uno::Exception&)
        {
        }

        m_xSAXEventKeeper->setNextHandler(nullptr);
        m_bIsSAXEventKeeperSticky = false;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
