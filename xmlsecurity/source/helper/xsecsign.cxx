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


#include <xsecctl.hxx>

#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <rtl/uuid.h>
#include <sal/log.hxx>

#include <framework/signaturecreatorimpl.hxx>
#include <framework/saxeventkeeperimpl.hxx>

namespace com::sun::star::graphic { class XGraphic; }

using namespace css;
using namespace css::uno;
using namespace css::graphic;

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

css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener > XSecController::prepareSignatureToWrite(
    InternalSignatureInformation& internalSignatureInfor,
    sal_Int32 nStorageFormat,
    bool bXAdESCompliantIfODF)
{
    sal_Int32 nSecurityId = internalSignatureInfor.signatureInfor.nSecurityId;
    SignatureReferenceInformations& vReferenceInfors = internalSignatureInfor.signatureInfor.vSignatureReferenceInfors;

    sal_Int32 nIdOfSignatureElementCollector;

    nIdOfSignatureElementCollector =
        m_xSAXEventKeeper->addSecurityElementCollector( css::xml::crypto::sax::ElementMarkPriority_AFTERMODIFY, true );

    m_xSAXEventKeeper->setSecurityId(nIdOfSignatureElementCollector, nSecurityId);

    rtl::Reference<SignatureCreatorImpl> xSignatureCreator(new SignatureCreatorImpl);

    css::uno::Sequence<css::uno::Any> args(5);
    args[0] <<= OUString::number(nSecurityId);
    args[1] <<= uno::Reference<xml::crypto::sax::XSecuritySAXEventKeeper>(static_cast<cppu::OWeakObject*>(m_xSAXEventKeeper.get()), uno::UNO_QUERY);
    args[2] <<= OUString::number(nIdOfSignatureElementCollector);

    //for nss, the internal module is used for signing, which needs to be improved later
    args[3] <<= m_xSecurityContext->getSecurityEnvironment();

    args[4] <<= m_xXMLSignature;
    xSignatureCreator->initialize(args);

    sal_Int32 nBlockerId = m_xSAXEventKeeper->addBlocker();
    m_xSAXEventKeeper->setSecurityId(nBlockerId, nSecurityId);

    xSignatureCreator->setBlockerId(nBlockerId);

    xSignatureCreator->addSignatureCreationResultListener(this);

    m_xSAXEventKeeper->addReferenceResolvedListener(nIdOfSignatureElementCollector, xSignatureCreator);

    int size = vReferenceInfors.size();
    sal_Int32 nReferenceCount = 0;

    for(int i=0; i<size; ++i)
    {
        sal_Int32 keeperId = internalSignatureInfor.vKeeperIds[i];

        if ( keeperId != -1)
        {
            m_xSAXEventKeeper->setSecurityId(keeperId, nSecurityId);
            m_xSAXEventKeeper->addReferenceResolvedListener( keeperId, xSignatureCreator);
            xSignatureCreator->setReferenceId( keeperId );
            nReferenceCount++;
        }
    }

    xSignatureCreator->setReferenceCount( nReferenceCount );

    /*
     * adds all URI binding
     */
    for(int i=0; i<size; ++i)
    {
        const SignatureReferenceInformation& refInfor = vReferenceInfors[i];

        css::uno::Reference< css::io::XInputStream > xInputStream
            = getObjectInputStream( refInfor.ouURI );

        if (xInputStream.is())
            xSignatureCreator->setUriBinding(refInfor.ouURI,xInputStream);
    }

    xSignatureCreator->setKeyId(0);

    // use sha512 for gpg signing unconditionally
    const sal_Int32 digestID = !internalSignatureInfor.signatureInfor.ouGpgCertificate.isEmpty()?
        css::xml::crypto::DigestID::SHA512 : (bXAdESCompliantIfODF ? css::xml::crypto::DigestID::SHA256 : css::xml::crypto::DigestID::SHA1);

    if (nStorageFormat != embed::StorageFormats::OFOPXML)
    {
        internalSignatureInfor.signatureInfor.ouSignatureId = createId();
        internalSignatureInfor.signatureInfor.ouDateTimePropertyId = createId();
        internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, internalSignatureInfor.signatureInfor.ouDateTimePropertyId, -1, OUString() );
        size++;

        if (bXAdESCompliantIfODF)
        {
            // We write a new reference, so it's possible to use the correct type URI.
            internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, "idSignedProperties", -1, "http://uri.etsi.org/01903#SignedProperties");
            size++;
        }

        if (!internalSignatureInfor.signatureInfor.ouDescription.isEmpty())
        {
            // Only mention the hash of the description in the signature if it's non-empty.
            internalSignatureInfor.signatureInfor.ouDescriptionPropertyId = createId();
            internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, internalSignatureInfor.signatureInfor.ouDescriptionPropertyId, -1, OUString());
            size++;
        }
    }
    else
    {
        internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, "idPackageObject", -1, OUString());
        size++;
        internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, "idOfficeObject", -1, OUString());
        size++;
        internalSignatureInfor.addReference(SignatureReferenceType::SAMEDOCUMENT, digestID, "idSignedProperties", -1, OUString());
        size++;
    }

    /*
     * replace both digestValues and signatureValue to " "
     */
    for(int i=0; i<size; ++i)
    {
        SignatureReferenceInformation& refInfor = vReferenceInfors[i];
        refInfor.ouDigestValue = " ";
    }

    internalSignatureInfor.signatureInfor.ouSignatureValue = " ";

    return xSignatureCreator;
}

void XSecController::signAStream( sal_Int32 securityId, const OUString& uri, bool isBinary, bool bXAdESCompliantIfODF)
{
    const SignatureReferenceType type = isBinary ? SignatureReferenceType::BINARYSTREAM : SignatureReferenceType::XMLSTREAM;
    sal_Int32 digestID = bXAdESCompliantIfODF ? css::xml::crypto::DigestID::SHA256 : css::xml::crypto::DigestID::SHA1;

    int index = findSignatureInfor( securityId );

    if (index == -1)
    {
        InternalSignatureInformation isi(securityId, nullptr);
        isi.addReference(type, digestID, uri, -1, OUString());
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        // use sha512 for gpg signing unconditionally
        if (!m_vInternalSignatureInformations[index].signatureInfor.ouGpgCertificate.isEmpty())
            digestID = css::xml::crypto::DigestID::SHA512;
        m_vInternalSignatureInformations[index].addReference(type, digestID, uri, -1, OUString());
    }
}

// note: this is called when creating a new signature from scratch
void XSecController::setX509Certificate(
    sal_Int32 nSecurityId,
    const OUString& ouX509IssuerName,
    const OUString& ouX509SerialNumber,
    const OUString& ouX509Cert,
    const OUString& ouX509CertDigest,
    svl::crypto::SignatureMethodAlgorithm eAlgorithmID)
{
    int index = findSignatureInfor( nSecurityId );

    if ( index == -1 )
    {
        InternalSignatureInformation isi(nSecurityId, nullptr);
        isi.signatureInfor.X509Datas.clear();
        isi.signatureInfor.X509Datas.emplace_back();
        isi.signatureInfor.X509Datas.back().emplace_back();
        isi.signatureInfor.X509Datas.back().back().X509IssuerName = ouX509IssuerName;
        isi.signatureInfor.X509Datas.back().back().X509SerialNumber = ouX509SerialNumber;
        isi.signatureInfor.X509Datas.back().back().X509Certificate = ouX509Cert;
        isi.signatureInfor.X509Datas.back().back().CertDigest = ouX509CertDigest;
        isi.signatureInfor.eAlgorithmID = eAlgorithmID;
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        SignatureInformation &si
            = m_vInternalSignatureInformations[index].signatureInfor;
        si.X509Datas.clear();
        si.X509Datas.emplace_back();
        si.X509Datas.back().emplace_back();
        si.X509Datas.back().back().X509IssuerName = ouX509IssuerName;
        si.X509Datas.back().back().X509SerialNumber = ouX509SerialNumber;
        si.X509Datas.back().back().X509Certificate = ouX509Cert;
        si.X509Datas.back().back().CertDigest = ouX509CertDigest;
    }
}

void XSecController::setGpgCertificate(
        sal_Int32 nSecurityId,
        const OUString& ouKeyDigest,
        const OUString& ouCert,
        const OUString& ouOwner)
{
    int index = findSignatureInfor( nSecurityId );

    if ( index == -1 )
    {
        InternalSignatureInformation isi(nSecurityId, nullptr);
        isi.signatureInfor.ouGpgCertificate = ouCert;
        isi.signatureInfor.ouGpgOwner = ouOwner;
        isi.signatureInfor.ouGpgKeyID = ouKeyDigest;
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        SignatureInformation &si
            = m_vInternalSignatureInformations[index].signatureInfor;
        si.X509Datas.clear(); // it is a PGP signature now
        si.ouGpgCertificate = ouCert;
        si.ouGpgOwner = ouOwner;
        si.ouGpgKeyID = ouKeyDigest;
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

void XSecController::setSignatureLineId(sal_Int32 nSecurityId, const OUString& rSignatureLineId)
{
    int nIndex = findSignatureInfor(nSecurityId);

    if (nIndex == -1)
    {
        InternalSignatureInformation aInformation(nSecurityId, nullptr);
        aInformation.signatureInfor.ouSignatureLineId = rSignatureLineId;
        m_vInternalSignatureInformations.push_back(aInformation);
    }
    else
    {
        SignatureInformation& rInformation = m_vInternalSignatureInformations[nIndex].signatureInfor;
        rInformation.ouSignatureLineId = rSignatureLineId;
    }
}

void XSecController::setSignatureLineValidGraphic(sal_Int32 nSecurityId,
                                                  const Reference<XGraphic>& xValidGraphic)
{
    int nIndex = findSignatureInfor(nSecurityId);

    if (nIndex == -1)
    {
        InternalSignatureInformation aInformation(nSecurityId, nullptr);
        aInformation.signatureInfor.aValidSignatureImage = xValidGraphic;
        m_vInternalSignatureInformations.push_back(aInformation);
    }
    else
    {
        SignatureInformation& rInformation
            = m_vInternalSignatureInformations[nIndex].signatureInfor;
        rInformation.aValidSignatureImage = xValidGraphic;
    }
}

void XSecController::setSignatureLineInvalidGraphic(
    sal_Int32 nSecurityId, const Reference<XGraphic>& xInvalidGraphic)
{
    int nIndex = findSignatureInfor(nSecurityId);

    if (nIndex == -1)
    {
        InternalSignatureInformation aInformation(nSecurityId, nullptr);
        aInformation.signatureInfor.aInvalidSignatureImage = xInvalidGraphic;
        m_vInternalSignatureInformations.push_back(aInformation);
    }
    else
    {
        SignatureInformation& rInformation
            = m_vInternalSignatureInformations[nIndex].signatureInfor;
        rInformation.aInvalidSignatureImage = xInvalidGraphic;
    }
}

bool XSecController::WriteSignature(
    const css::uno::Reference<css::xml::sax::XDocumentHandler>& xDocumentHandler,
    bool bXAdESCompliantIfODF )
{
    bool rc = false;

    SAL_WARN_IF( !xDocumentHandler.is(), "xmlsecurity.helper", "I really need a document handler!" );

    /*
     * chain the SAXEventKeeper to the SAX chain
     */
    chainOn();

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
            css::uno::Reference<css::xml::sax::XDocumentHandler> xSEKHandler(static_cast<cppu::OWeakObject*>(m_xSAXEventKeeper.get()),css::uno::UNO_QUERY);

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
        catch( css::uno::Exception& )
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
    chainOn();

    if (m_eStatusOfSecurityComponents == InitializationState::INITIALIZED)
    {
        m_bIsSAXEventKeeperSticky = true;
        m_xSAXEventKeeper->setNextHandler(xDocumentHandler);

        try
        {
            // Export the signature template.
            css::uno::Reference<xml::sax::XDocumentHandler> xSEKHandler(static_cast<cppu::OWeakObject*>(m_xSAXEventKeeper.get()), uno::UNO_QUERY);

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
