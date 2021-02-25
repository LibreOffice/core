/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "ooxmlsecparser.hxx"
#include <xmlsignaturehelper.hxx>
#include <xsecctl.hxx>
#include <sal/log.hxx>

using namespace com::sun::star;

OOXMLSecParser::OOXMLSecParser(XMLSignatureHelper& rXMLSignatureHelper, XSecController* pXSecController)
    : m_pXSecController(pXSecController)
    ,m_bInDigestValue(false)
    ,m_bInSignatureValue(false)
    ,m_bInX509Certificate(false)
    ,m_bInMdssiValue(false)
    ,m_bInSignatureComments(false)
    ,m_bInX509IssuerName(false)
    ,m_bInX509SerialNumber(false)
    ,m_bInCertDigest(false)
    ,m_bInValidSignatureImage(false)
    ,m_bInInvalidSignatureImage(false)
    ,m_bInSignatureLineId(false)
    ,m_bReferenceUnresolved(false)
    ,m_rXMLSignatureHelper(rXMLSignatureHelper)
{
}

OOXMLSecParser::~OOXMLSecParser()
{
}

void SAL_CALL OOXMLSecParser::startDocument()
{
    if (m_xNextHandler.is())
        m_xNextHandler->startDocument();
}

void SAL_CALL OOXMLSecParser::endDocument()
{
    if (m_xNextHandler.is())
        m_xNextHandler->endDocument();
}

void SAL_CALL OOXMLSecParser::startElement(const OUString& rName, const uno::Reference<xml::sax::XAttributeList>& xAttribs)
{
    OUString aId = xAttribs->getValueByName("Id");
    if (!aId.isEmpty())
        m_pXSecController->collectToVerify(aId);

    if (rName == "Signature")
    {
        m_rXMLSignatureHelper.StartVerifySignatureElement();
        m_pXSecController->addSignature();
        if (!aId.isEmpty())
            m_pXSecController->setId(aId);
    }
    else if (rName == "SignatureMethod")
    {
        OUString ouAlgorithm = xAttribs->getValueByName("Algorithm");
        if (ouAlgorithm == ALGO_ECDSASHA1 || ouAlgorithm == ALGO_ECDSASHA256
            || ouAlgorithm == ALGO_ECDSASHA512)
            m_pXSecController->setSignatureMethod(svl::crypto::SignatureMethodAlgorithm::ECDSA);
    }
    else if (rName == "Reference")
    {
        OUString aURI = xAttribs->getValueByName("URI");
        if (aURI.startsWith("#"))
            m_pXSecController->addReference(aURI.copy(1), xml::crypto::DigestID::SHA1, OUString());
        else
        {
            m_aReferenceURI = aURI;
            m_bReferenceUnresolved = true;
        }
    }
    else if (rName == "Transform")
    {
        if (m_bReferenceUnresolved)
        {
            OUString aAlgorithm = xAttribs->getValueByName("Algorithm");
            if (aAlgorithm == ALGO_RELATIONSHIP)
            {
                m_pXSecController->addStreamReference(m_aReferenceURI, /*isBinary=*/false, /*nDigestID=*/xml::crypto::DigestID::SHA256);
                m_bReferenceUnresolved = false;
            }
        }
    }
    else if (rName == "DigestValue" && !m_bInCertDigest)
    {
        m_aDigestValue.clear();
        m_bInDigestValue = true;
    }
    else if (rName == "SignatureValue")
    {
        m_aSignatureValue.clear();
        m_bInSignatureValue = true;
    }
    else if (rName == "X509Certificate")
    {
        m_aX509Certificate.clear();
        m_bInX509Certificate = true;
    }
    else if (rName == "mdssi:Value")
    {
        m_aMdssiValue.clear();
        m_bInMdssiValue = true;
    }
    else if (rName == "SignatureComments")
    {
        m_aSignatureComments.clear();
        m_bInSignatureComments = true;
    }
    else if (rName == "X509IssuerName")
    {
        m_aX509IssuerName.clear();
        m_bInX509IssuerName = true;
    }
    else if (rName == "X509SerialNumber")
    {
        m_aX509SerialNumber.clear();
        m_bInX509SerialNumber = true;
    }
    else if (rName == "xd:CertDigest")
    {
        m_aCertDigest.clear();
        m_bInCertDigest = true;
    }
    else if (rName == "Object")
    {
        OUString sId = xAttribs->getValueByName("Id");
        if (sId == "idValidSigLnImg")
        {
            m_aValidSignatureImage.clear();
            m_bInValidSignatureImage = true;
        }
        else if (sId == "idInvalidSigLnImg")
        {
            m_aInvalidSignatureImage.clear();
            m_bInInvalidSignatureImage = true;
        }
        else
        {
            SAL_INFO("xmlsecurity.ooxml", "Unknown 'Object' child element: " << rName);
        }
    }
    else if (rName == "SetupID")
    {
        m_aSignatureLineId.clear();
        m_bInSignatureLineId = true;
    }
    else
    {
        SAL_INFO("xmlsecurity.ooxml", "Unknown xml element: " << rName);
    }

    if (m_xNextHandler.is())
        m_xNextHandler->startElement(rName, xAttribs);
}

void SAL_CALL OOXMLSecParser::endElement(const OUString& rName)
{
    if (rName == "SignedInfo")
        m_pXSecController->setReferenceCount();
    else if (rName == "Reference")
    {
        if (m_bReferenceUnresolved)
        {
            // No transform algorithm found, assume binary.
            m_pXSecController->addStreamReference(m_aReferenceURI, /*isBinary=*/true, /*nDigestID=*/xml::crypto::DigestID::SHA256);
            m_bReferenceUnresolved = false;
        }
        m_pXSecController->setDigestValue(xml::crypto::DigestID::SHA256, m_aDigestValue);
    }
    else if (rName == "DigestValue" && !m_bInCertDigest)
        m_bInDigestValue = false;
    else if (rName == "SignatureValue")
    {
        m_pXSecController->setSignatureValue(m_aSignatureValue);
        m_bInSignatureValue = false;
    }
    else if (rName == "X509Data")
    {
        std::vector<std::pair<OUString, OUString>> X509IssuerSerials;
        std::vector<OUString> X509Certificates;
        if (!m_aX509Certificate.isEmpty())
        {
            X509Certificates.emplace_back(m_aX509Certificate);
        }
        if (!m_aX509IssuerName.isEmpty() && !m_aX509SerialNumber.isEmpty())
        {
            X509IssuerSerials.emplace_back(m_aX509IssuerName, m_aX509SerialNumber);
        }
        m_pXSecController->setX509Data(X509IssuerSerials, X509Certificates);
    }
    else if (rName == "X509Certificate")
    {
        m_bInX509Certificate = false;
    }
    else if (rName == "mdssi:Value")
    {
        m_pXSecController->setDate("", m_aMdssiValue);
        m_bInMdssiValue = false;
    }
    else if (rName == "SignatureComments")
    {
        m_pXSecController->setDescription("", m_aSignatureComments);
        m_bInSignatureComments = false;
    }
    else if (rName == "X509IssuerName")
    {
        m_bInX509IssuerName = false;
    }
    else if (rName == "X509SerialNumber")
    {
        m_bInX509SerialNumber = false;
    }
    else if (rName == "xd:Cert")
    {
        m_pXSecController->setX509CertDigest(m_aCertDigest, css::xml::crypto::DigestID::SHA1, m_aX509IssuerName, m_aX509SerialNumber);
    }
    else if (rName == "xd:CertDigest")
    {
        m_bInCertDigest = false;
    }
    else if (rName == "Object")
    {
        if (m_bInValidSignatureImage)
        {
            m_pXSecController->setValidSignatureImage(m_aValidSignatureImage);
            m_bInValidSignatureImage = false;
        }
        else if (m_bInInvalidSignatureImage)
        {
            m_pXSecController->setInvalidSignatureImage(m_aInvalidSignatureImage);
            m_bInInvalidSignatureImage = false;
        }
    }
    else if (rName == "SetupID")
    {
        m_pXSecController->setSignatureLineId(m_aSignatureLineId);
        m_bInSignatureLineId = false;
    }

    if (m_xNextHandler.is())
        m_xNextHandler->endElement(rName);
}

void SAL_CALL OOXMLSecParser::characters(const OUString& rChars)
{
    if (m_bInDigestValue && !m_bInCertDigest)
        m_aDigestValue += rChars;
    else if (m_bInSignatureValue)
        m_aSignatureValue += rChars;
    else if (m_bInX509Certificate)
        m_aX509Certificate += rChars;
    else if (m_bInMdssiValue)
        m_aMdssiValue += rChars;
    else if (m_bInSignatureComments)
        m_aSignatureComments += rChars;
    else if (m_bInX509IssuerName)
        m_aX509IssuerName += rChars;
    else if (m_bInX509SerialNumber)
        m_aX509SerialNumber += rChars;
    else if (m_bInCertDigest)
        m_aCertDigest += rChars;
    else if (m_bInValidSignatureImage)
        m_aValidSignatureImage += rChars;
    else if (m_bInInvalidSignatureImage)
        m_aInvalidSignatureImage += rChars;
    else if (m_bInSignatureLineId)
        m_aSignatureLineId += rChars;

    if (m_xNextHandler.is())
        m_xNextHandler->characters(rChars);
}

void SAL_CALL OOXMLSecParser::ignorableWhitespace(const OUString& rWhitespace)
{
    if (m_xNextHandler.is())
        m_xNextHandler->ignorableWhitespace(rWhitespace);
}

void SAL_CALL OOXMLSecParser::processingInstruction(const OUString& rTarget, const OUString& rData)
{
    if (m_xNextHandler.is())
        m_xNextHandler->processingInstruction(rTarget, rData);
}

void SAL_CALL OOXMLSecParser::setDocumentLocator(const uno::Reference<xml::sax::XLocator>& xLocator)
{
    if (m_xNextHandler.is())
        m_xNextHandler->setDocumentLocator(xLocator);
}

void SAL_CALL OOXMLSecParser::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    rArguments[0] >>= m_xNextHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
