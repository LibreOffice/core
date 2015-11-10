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


#include "xsecparser.hxx"
#include <tools/debug.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <string.h>

namespace cssu = com::sun::star::uno;
namespace cssxs = com::sun::star::xml::sax;

XSecParser::XSecParser(XSecController* pXSecController,
    const cssu::Reference< cssxs::XDocumentHandler >& xNextHandler)
    : m_bInX509IssuerName(false)
    , m_bInX509SerialNumber(false)
    , m_bInX509Certificate(false)
    , m_bInDigestValue(false)
    , m_bInSignatureValue(false)
    , m_bInDate(false)
    , m_pXSecController(pXSecController)
    , m_xNextHandler(xNextHandler)
    , m_bReferenceUnresolved(false)
{
}

OUString XSecParser::getIdAttr(const cssu::Reference< cssxs::XAttributeList >& xAttribs )
{
    OUString ouIdAttr = xAttribs->getValueByName("id");

    if (ouIdAttr == nullptr)
    {
        ouIdAttr = xAttribs->getValueByName("Id");
    }

    return ouIdAttr;
}

/*
 * XDocumentHandler
 */
void SAL_CALL XSecParser::startDocument(  )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    m_bInX509IssuerName = false;
    m_bInX509SerialNumber = false;
    m_bInX509Certificate = false;
    m_bInSignatureValue = false;
    m_bInDigestValue = false;
    m_bInDate = false;

    if (m_xNextHandler.is())
    {
        m_xNextHandler->startDocument();
    }
}

void SAL_CALL XSecParser::endDocument(  )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->endDocument();
    }
}

void SAL_CALL XSecParser::startElement(
    const OUString& aName,
    const cssu::Reference< cssxs::XAttributeList >& xAttribs )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    try
    {
        OUString ouIdAttr = getIdAttr(xAttribs);
        if (ouIdAttr != nullptr)
        {
            m_pXSecController->collectToVerify( ouIdAttr );
        }

        if ( aName == TAG_SIGNATURE )
        {
            m_pXSecController->addSignature();
            if (ouIdAttr != nullptr)
            {
                m_pXSecController->setId( ouIdAttr );
            }
        }
        else if ( aName == TAG_REFERENCE )
        {
            OUString ouUri = xAttribs->getValueByName(ATTR_URI);
            DBG_ASSERT( ouUri != nullptr, "URI == NULL" );

            if (ouUri.startsWith(CHAR_FRAGMENT))
            {
                /*
                * remove the first character '#' from the attribute value
                */
                m_pXSecController->addReference( ouUri.copy(1) );
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
            else if (aName == TAG_TRANSFORM)
            {
            if ( m_bReferenceUnresolved )
            {
                OUString ouAlgorithm = xAttribs->getValueByName(ATTR_ALGORITHM);

                if (ouAlgorithm != nullptr && ouAlgorithm == ALGO_C14N)
                /*
                * a xml stream
                */
                {
                    m_pXSecController->addStreamReference( m_currentReferenceURI, false);
                    m_bReferenceUnresolved = false;
                }
            }
            }
            else if (aName == TAG_X509ISSUERNAME)
            {
            m_ouX509IssuerName.clear();
            m_bInX509IssuerName = true;
            }
            else if (aName == TAG_X509SERIALNUMBER)
            {
            m_ouX509SerialNumber.clear();
            m_bInX509SerialNumber = true;
            }
            else if (aName == TAG_X509CERTIFICATE)
            {
            m_ouX509Certificate.clear();
            m_bInX509Certificate = true;
            }
            else if (aName == TAG_SIGNATUREVALUE)
            {
            m_ouSignatureValue.clear();
                m_bInSignatureValue = true;
            }
            else if (aName == TAG_DIGESTVALUE)
            {
                m_ouDigestValue.clear();
                m_bInDigestValue = true;
            }
            else if ( aName == TAG_SIGNATUREPROPERTY )
        {
            if (ouIdAttr != nullptr)
            {
                m_pXSecController->setPropertyId( ouIdAttr );
            }
        }
            else if (aName == NSTAG_DC ":" TAG_DATE)
            {
            m_ouDate.clear();
                m_bInDate = true;
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
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    try
    {
        if (aName == TAG_DIGESTVALUE)
            {
                m_bInDigestValue = false;
            }
        else if ( aName == TAG_REFERENCE )
        {
            if ( m_bReferenceUnresolved )
            /*
            * it must be a octet stream
            */
            {
                m_pXSecController->addStreamReference( m_currentReferenceURI, true);
                m_bReferenceUnresolved = false;
            }

            m_pXSecController->setDigestValue( m_ouDigestValue );
        }
        else if ( aName == TAG_SIGNEDINFO )
        {
            m_pXSecController->setReferenceCount();
        }
        else if ( aName == TAG_SIGNATUREVALUE )
        {
            m_pXSecController->setSignatureValue( m_ouSignatureValue );
                m_bInSignatureValue = false;
        }
            else if (aName == TAG_X509ISSUERNAME)
            {
            m_pXSecController->setX509IssuerName( m_ouX509IssuerName );
            m_bInX509IssuerName = false;
            }
            else if (aName == TAG_X509SERIALNUMBER)
            {
            m_pXSecController->setX509SerialNumber( m_ouX509SerialNumber );
            m_bInX509SerialNumber = false;
            }
            else if (aName == TAG_X509CERTIFICATE)
            {
            m_pXSecController->setX509Certificate( m_ouX509Certificate );
            m_bInX509Certificate = false;
            }
            else if (aName == NSTAG_DC ":" TAG_DATE)
        {
            m_pXSecController->setDate( m_ouDate );
                m_bInDate = false;
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
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
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
    else if (m_bInDigestValue)
    {
        m_ouDigestValue += aChars;
    }
    else if (m_bInDate)
    {
        m_ouDate += aChars;
    }

    if (m_xNextHandler.is())
    {
        m_xNextHandler->characters(aChars);
        }
}

void SAL_CALL XSecParser::ignorableWhitespace( const OUString& aWhitespaces )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->ignorableWhitespace( aWhitespaces );
        }
}

void SAL_CALL XSecParser::processingInstruction( const OUString& aTarget, const OUString& aData )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->processingInstruction(aTarget, aData);
        }
}

void SAL_CALL XSecParser::setDocumentLocator( const cssu::Reference< cssxs::XLocator >& xLocator )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
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
    throw(cssu::Exception, cssu::RuntimeException, std::exception)
{
    aArguments[0] >>= m_xNextHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
