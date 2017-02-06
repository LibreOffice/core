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


#include "xsecparser.hxx"
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <cppuhelper/exc_hlp.hxx>

#include <string.h>

namespace cssu = com::sun::star::uno;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxs = com::sun::star::xml::sax;

XSecParser::XSecParser(XSecController* pXSecController,
    const cssu::Reference< cssxs::XDocumentHandler >& xNextHandler)
    : m_bInX509IssuerName(false)
    , m_bInX509SerialNumber(false)
    , m_bInX509Certificate(false)
    , m_bInCertDigest(false)
    , m_bInEncapsulatedX509Certificate(false)
    , m_bInSigningTime(false)
    , m_bInDigestValue(false)
    , m_bInSignatureValue(false)
    , m_bInDate(false)
    , m_bInDescription(false)
    , m_pXSecController(pXSecController)
    , m_xNextHandler(xNextHandler)
    , m_bReferenceUnresolved(false)
    , m_nReferenceDigestID(cssxc::DigestID::SHA1)
{
}

OUString XSecParser::getIdAttr(const cssu::Reference< cssxs::XAttributeList >& xAttribs )
{
    OUString ouIdAttr = xAttribs->getValueByName("id");

    if (ouIdAttr.isEmpty())
    {
        ouIdAttr = xAttribs->getValueByName("Id");
    }

    return ouIdAttr;
}

/*
 * XDocumentHandler
 */
void SAL_CALL XSecParser::startDocument(  )
{
    m_bInX509IssuerName = false;
    m_bInX509SerialNumber = false;
    m_bInX509Certificate = false;
    m_bInSignatureValue = false;
    m_bInDigestValue = false;
    m_bInDate = false;
    m_bInDescription = false;

    if (m_xNextHandler.is())
    {
        m_xNextHandler->startDocument();
    }
}

void SAL_CALL XSecParser::endDocument(  )
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->endDocument();
    }
}

void SAL_CALL XSecParser::startElement(
    const OUString& aName,
    const cssu::Reference< cssxs::XAttributeList >& xAttribs )
{
    try
    {
        OUString ouIdAttr = getIdAttr(xAttribs);
        if (!ouIdAttr.isEmpty())
        {
            m_pXSecController->collectToVerify( ouIdAttr );
        }

        if ( aName == "Signature" )
        {
            m_pXSecController->addSignature();
            if (!ouIdAttr.isEmpty())
            {
                m_pXSecController->setId( ouIdAttr );
            }
        }
        else if ( aName == "Reference" )
        {
            OUString ouUri = xAttribs->getValueByName("URI");
            SAL_WARN_IF( ouUri.isEmpty(), "xmlsecurity.helper", "URI is empty" );
            if (ouUri.startsWith("#"))
            {
                /*
                * remove the first character '#' from the attribute value
                */
                m_pXSecController->addReference( ouUri.copy(1), m_nReferenceDigestID );
            }
            else
            {
                /*
                * remember the uri
                */
                m_currentReferenceURI = ouUri;
                m_bReferenceUnresolved = true;
            }
        }
        else if (aName == "DigestMethod")
        {
            OUString ouAlgorithm = xAttribs->getValueByName("Algorithm");

            SAL_WARN_IF( ouAlgorithm.isEmpty(), "xmlsecurity.helper", "no Algorithm in Reference" );
            if (!ouAlgorithm.isEmpty())
            {
                SAL_WARN_IF( ouAlgorithm != ALGO_XMLDSIGSHA1 && ouAlgorithm != ALGO_XMLDSIGSHA256,
                             "xmlsecurity.helper", "Algorithm neither SHA1 or SHA256");
                if (ouAlgorithm == ALGO_XMLDSIGSHA1)
                    m_nReferenceDigestID = cssxc::DigestID::SHA1;
                else if (ouAlgorithm == ALGO_XMLDSIGSHA256)
                    m_nReferenceDigestID = cssxc::DigestID::SHA256;
            }
        }
        else if (aName == "Transform")
        {
            if ( m_bReferenceUnresolved )
            {
                OUString ouAlgorithm = xAttribs->getValueByName("Algorithm");

                if (ouAlgorithm == ALGO_C14N)
                    /*
                     * a xml stream
                     */
                {
                    m_pXSecController->addStreamReference( m_currentReferenceURI, false, m_nReferenceDigestID );
                    m_bReferenceUnresolved = false;
                }
            }
        }
        else if (aName == "X509IssuerName")
        {
            m_ouX509IssuerName.clear();
            m_bInX509IssuerName = true;
        }
        else if (aName == "X509SerialNumber")
        {
            m_ouX509SerialNumber.clear();
            m_bInX509SerialNumber = true;
        }
        else if (aName == "X509Certificate")
        {
            m_ouX509Certificate.clear();
            m_bInX509Certificate = true;
        }
        else if (aName == "SignatureValue")
        {
            m_ouSignatureValue.clear();
            m_bInSignatureValue = true;
        }
        else if (aName == "DigestValue" && !m_bInCertDigest)
        {
            m_ouDigestValue.clear();
            m_bInDigestValue = true;
        }
        else if (aName == "xd:CertDigest")
        {
            m_ouCertDigest.clear();
            m_bInCertDigest = true;
        }
        // FIXME: Existing code here in xmlsecurity uses "xd" as the namespace prefix for XAdES,
        // while the sample document attached to tdf#76142 uses "xades". So accept either here. Of
        // course this is idiotic and wrong, the right thing would be to use a proper way to parse
        // XML that would handle namespaces correctly. I have no idea how substantial re-plumbing of
        // this code that would require.
        else if (aName == "xd:EncapsulatedX509Certificate" || aName == "xades:EncapsulatedX509Certificate")
        {
            m_ouEncapsulatedX509Certificate.clear();
            m_bInEncapsulatedX509Certificate = true;
        }
        else if (aName == "xd:SigningTime" || aName == "xades:SigningTime")
        {
            m_ouDate.clear();
            m_bInSigningTime = true;
        }
        else if ( aName == "SignatureProperty" )
        {
            if (!ouIdAttr.isEmpty())
            {
                m_pXSecController->setPropertyId( ouIdAttr );
            }
        }
        else if (aName == "dc:date")
        {
            if (m_ouDate.isEmpty())
                m_bInDate = true;
        }
        else if (aName == "dc:description")
        {
            m_ouDescription.clear();
            m_bInDescription = true;
        }

        if (m_xNextHandler.is())
        {
            m_xNextHandler->startElement(aName, xAttribs);
        }
    }
    catch (cssu::Exception& )
    {//getCaughtException MUST be the first line in the catch block
        cssu::Any exc =  cppu::getCaughtException();
        throw cssxs::SAXException(
            "xmlsecurity: Exception in XSecParser::startElement",
            nullptr, exc);
    }
    catch (...)
    {
        throw cssxs::SAXException(
            "xmlsecurity: unexpected exception in XSecParser::startElement", nullptr,
            cssu::Any());
    }
}

void SAL_CALL XSecParser::endElement( const OUString& aName )
{
    try
    {
        if (aName == "DigestValue" && !m_bInCertDigest)
        {
            m_bInDigestValue = false;
        }
        else if ( aName == "Reference" )
        {
            if ( m_bReferenceUnresolved )
            /*
            * it must be a octet stream
            */
            {
                m_pXSecController->addStreamReference( m_currentReferenceURI, true, m_nReferenceDigestID );
                m_bReferenceUnresolved = false;
            }

            m_pXSecController->setDigestValue( m_nReferenceDigestID, m_ouDigestValue );
        }
        else if ( aName == "SignedInfo" )
        {
            m_pXSecController->setReferenceCount();
        }
        else if ( aName == "SignatureValue" )
        {
            m_pXSecController->setSignatureValue( m_ouSignatureValue );
            m_bInSignatureValue = false;
        }
        else if (aName == "X509IssuerName")
        {
            m_pXSecController->setX509IssuerName( m_ouX509IssuerName );
            m_bInX509IssuerName = false;
        }
        else if (aName == "X509SerialNumber")
        {
            m_pXSecController->setX509SerialNumber( m_ouX509SerialNumber );
            m_bInX509SerialNumber = false;
        }
        else if (aName == "X509Certificate")
        {
            m_pXSecController->setX509Certificate( m_ouX509Certificate );
            m_bInX509Certificate = false;
        }
        else if (aName == "xd:CertDigest")
        {
            m_pXSecController->setCertDigest( m_ouCertDigest );
            m_bInCertDigest = false;
        }
        else if (aName == "xd:EncapsulatedX509Certificate" || aName == "xades:EncapsulatedX509Certificate")
        {
            m_pXSecController->addEncapsulatedX509Certificate( m_ouEncapsulatedX509Certificate );
            m_bInEncapsulatedX509Certificate = false;
        }
        else if (aName == "xd:SigningTime" || aName == "xades:SigningTime")
        {
            m_pXSecController->setDate( m_ouDate );
            m_bInSigningTime = false;
        }
        else if (aName == "dc:date")
        {
            if (m_bInDate)
            {
                m_pXSecController->setDate( m_ouDate );
                m_bInDate = false;
            }
        }
        else if (aName == "dc:description")
        {
            m_pXSecController->setDescription( m_ouDescription );
            m_bInDescription = false;
        }

        if (m_xNextHandler.is())
        {
            m_xNextHandler->endElement(aName);
        }
    }
    catch (cssu::Exception& )
    {//getCaughtException MUST be the first line in the catch block
        cssu::Any exc =  cppu::getCaughtException();
        throw cssxs::SAXException(
            "xmlsecurity: Exception in XSecParser::endElement",
            nullptr, exc);
    }
    catch (...)
    {
        throw cssxs::SAXException(
            "xmlsecurity: unexpected exception in XSecParser::endElement", nullptr,
            cssu::Any());
    }
}

void SAL_CALL XSecParser::characters( const OUString& aChars )
{
    if (m_bInX509IssuerName)
    {
        m_ouX509IssuerName += aChars;
    }
    else if (m_bInX509SerialNumber)
    {
        m_ouX509SerialNumber += aChars;
    }
    else if (m_bInX509Certificate)
    {
        m_ouX509Certificate += aChars;
    }
    else if (m_bInSignatureValue)
    {
        m_ouSignatureValue += aChars;
    }
    else if (m_bInDigestValue && !m_bInCertDigest)
    {
        m_ouDigestValue += aChars;
    }
    else if (m_bInDate)
    {
        m_ouDate += aChars;
    }
    else if (m_bInDescription)
    {
        m_ouDescription += aChars;
    }
    else if (m_bInCertDigest)
    {
        m_ouCertDigest += aChars;
    }
    else if (m_bInEncapsulatedX509Certificate)
    {
        m_ouEncapsulatedX509Certificate += aChars;
    }
    else if (m_bInSigningTime)
    {
        m_ouDate += aChars;
    }

    if (m_xNextHandler.is())
    {
        m_xNextHandler->characters(aChars);
    }
}

void SAL_CALL XSecParser::ignorableWhitespace( const OUString& aWhitespaces )
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->ignorableWhitespace( aWhitespaces );
    }
}

void SAL_CALL XSecParser::processingInstruction( const OUString& aTarget, const OUString& aData )
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->processingInstruction(aTarget, aData);
    }
}

void SAL_CALL XSecParser::setDocumentLocator( const cssu::Reference< cssxs::XLocator >& xLocator )
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->setDocumentLocator( xLocator );
    }
}

/*
 * XInitialization
 */
void SAL_CALL XSecParser::initialize(
    const cssu::Sequence< cssu::Any >& aArguments )
{
    aArguments[0] >>= m_xNextHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
