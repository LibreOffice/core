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

#include "xsecparser.hxx"
#include <tools/debug.hxx>
#include "cppuhelper/exc_hlp.hxx"

#include <string.h>

namespace cssu = com::sun::star::uno;
namespace cssxs = com::sun::star::xml::sax;

#define RTL_ASCII_USTRINGPARAM( asciiStr ) asciiStr, strlen( asciiStr ), RTL_TEXTENCODING_ASCII_US

XSecParser::XSecParser(
    XSecController* pXSecController,
    const cssu::Reference< cssxs::XDocumentHandler >& xNextHandler )
    : m_pXSecController(pXSecController),
      m_xNextHandler(xNextHandler),
      m_bReferenceUnresolved(false)
{
}

rtl::OUString XSecParser::getIdAttr(const cssu::Reference< cssxs::XAttributeList >& xAttribs )
{
    rtl::OUString ouIdAttr = xAttribs->getValueByName(
        rtl::OUString(RTL_ASCII_USTRINGPARAM("id")));

    if (ouIdAttr == NULL)
    {
        ouIdAttr = xAttribs->getValueByName(
            rtl::OUString(RTL_ASCII_USTRINGPARAM("Id")));
    }

    return ouIdAttr;
}

/*
 * XDocumentHandler
 */
void SAL_CALL XSecParser::startDocument(  )
    throw (cssxs::SAXException, cssu::RuntimeException)
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
    throw (cssxs::SAXException, cssu::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->endDocument();
    }
}

void SAL_CALL XSecParser::startElement(
    const rtl::OUString& aName,
    const cssu::Reference< cssxs::XAttributeList >& xAttribs )
    throw (cssxs::SAXException, cssu::RuntimeException)
{
    try
    {
        rtl::OUString ouIdAttr = getIdAttr(xAttribs);
        if (ouIdAttr != NULL)
        {
            m_pXSecController->collectToVerify( ouIdAttr );
        }

        if ( aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_SIGNATURE)) )
        {
            m_pXSecController->addSignature();
            if (ouIdAttr != NULL)
            {
                m_pXSecController->setId( ouIdAttr );
            }
        }
        else if ( aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_REFERENCE)) )
        {
            rtl::OUString ouUri = xAttribs->getValueByName(rtl::OUString(RTL_ASCII_USTRINGPARAM(ATTR_URI)));
            DBG_ASSERT( ouUri != NULL, "URI == NULL" );

            if (0 == ouUri.compareTo(rtl::OUString(RTL_ASCII_USTRINGPARAM(CHAR_FRAGMENT)),1))
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
            else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_TRANSFORM)))
            {
            if ( m_bReferenceUnresolved )
            {
                rtl::OUString ouAlgorithm = xAttribs->getValueByName(rtl::OUString(RTL_ASCII_USTRINGPARAM(ATTR_ALGORITHM)));

                if (ouAlgorithm != NULL && ouAlgorithm == rtl::OUString(RTL_ASCII_USTRINGPARAM(ALGO_C14N)))
                /*
                * a xml stream
                */
                {
                    m_pXSecController->addStreamReference( m_currentReferenceURI, sal_False);
                    m_bReferenceUnresolved = false;
                }
            }
            }
            else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_X509ISSUERNAME)))
            {
            m_ouX509IssuerName = rtl::OUString();
            m_bInX509IssuerName = true;
            }
            else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_X509SERIALNUMBER)))
            {
            m_ouX509SerialNumber = rtl::OUString();
            m_bInX509SerialNumber = true;
            }
            else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_X509CERTIFICATE)))
            {
            m_ouX509Certificate = rtl::OUString();
            m_bInX509Certificate = true;
            }
            else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_SIGNATUREVALUE)))
            {
            m_ouSignatureValue = rtl::OUString();
                m_bInSignatureValue = true;
            }
            else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_DIGESTVALUE)))
            {
            m_ouDigestValue = rtl::OUString();
                m_bInDigestValue = true;
            }
            else if ( aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_SIGNATUREPROPERTY)) )
        {
            if (ouIdAttr != NULL)
            {
                m_pXSecController->setPropertyId( ouIdAttr );
            }
        }
            else if (aName == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(NSTAG_DC))
                        +rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(":"))
                        +rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(TAG_DATE)))
            {
            m_ouDate = rtl::OUString();
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
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "xmlsecurity: Exception in XSecParser::startElement")),
            0, exc);
    }
    catch (...)
    {
        throw cssxs::SAXException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("xmlsecurity: unexpected exception in XSecParser::startElement")), 0,
            cssu::Any());
    }
}

void SAL_CALL XSecParser::endElement( const rtl::OUString& aName )
    throw (cssxs::SAXException, cssu::RuntimeException)
{
    try
    {
        if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_DIGESTVALUE)))
            {
                m_bInDigestValue = false;
            }
        else if ( aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_REFERENCE)) )
        {
            if ( m_bReferenceUnresolved )
            /*
            * it must be a octet stream
            */
            {
                m_pXSecController->addStreamReference( m_currentReferenceURI, sal_True);
                m_bReferenceUnresolved = false;
            }

            m_pXSecController->setDigestValue( m_ouDigestValue );
        }
        else if ( aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_SIGNEDINFO)) )
        {
            m_pXSecController->setReferenceCount();
        }
        else if ( aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_SIGNATUREVALUE)) )
        {
            m_pXSecController->setSignatureValue( m_ouSignatureValue );
                m_bInSignatureValue = false;
        }
            else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_X509ISSUERNAME)))
            {
            m_pXSecController->setX509IssuerName( m_ouX509IssuerName );
            m_bInX509IssuerName = false;
            }
            else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_X509SERIALNUMBER)))
            {
            m_pXSecController->setX509SerialNumber( m_ouX509SerialNumber );
            m_bInX509SerialNumber = false;
            }
            else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_X509CERTIFICATE)))
            {
            m_pXSecController->setX509Certificate( m_ouX509Certificate );
            m_bInX509Certificate = false;
            }
            else if (aName == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(NSTAG_DC))
                        +rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(":"))
                        +rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(TAG_DATE)))
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
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "xmlsecurity: Exception in XSecParser::endElement")),
            0, exc);
    }
    catch (...)
    {
        throw cssxs::SAXException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("xmlsecurity: unexpected exception in XSecParser::endElement")), 0,
            cssu::Any());
    }
}

void SAL_CALL XSecParser::characters( const rtl::OUString& aChars )
    throw (cssxs::SAXException, cssu::RuntimeException)
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

void SAL_CALL XSecParser::ignorableWhitespace( const rtl::OUString& aWhitespaces )
    throw (cssxs::SAXException, cssu::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->ignorableWhitespace( aWhitespaces );
        }
}

void SAL_CALL XSecParser::processingInstruction( const rtl::OUString& aTarget, const rtl::OUString& aData )
    throw (cssxs::SAXException, cssu::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->processingInstruction(aTarget, aData);
        }
}

void SAL_CALL XSecParser::setDocumentLocator( const cssu::Reference< cssxs::XLocator >& xLocator )
    throw (cssxs::SAXException, cssu::RuntimeException)
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
    throw(cssu::Exception, cssu::RuntimeException)
{
    aArguments[0] >>= m_xNextHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
