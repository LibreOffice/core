/*************************************************************************
 *
 *  $RCSfile: xsecparser.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-28 02:26:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "xsecparser.hxx"

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

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
    m_bInTime = false;

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
        m_ouX509IssuerName = rtl::OUString::createFromAscii("");
        m_bInX509IssuerName = true;
        }
        else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_X509SERIALNUMBER)))
        {
        m_ouX509SerialNumber = rtl::OUString::createFromAscii("");
        m_bInX509SerialNumber = true;
        }
        else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_X509CERTIFICATE)))
        {
        m_ouX509Certificate = rtl::OUString::createFromAscii("");
        m_bInX509Certificate = true;
        }
        else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_SIGNATUREVALUE)))
        {
        m_ouSignatureValue = rtl::OUString::createFromAscii("");
            m_bInSignatureValue = true;
        }
        else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_DIGESTVALUE)))
        {
        m_ouDigestValue = rtl::OUString::createFromAscii("");
            m_bInDigestValue = true;
        }
        else if ( aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_SIGNATUREPROPERTY)) )
    {
        if (ouIdAttr != NULL)
        {
            m_pXSecController->setPropertyId( ouIdAttr );
        }
    }
        else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_DATE)))
        {
        m_ouDate = rtl::OUString::createFromAscii("");
            m_bInDate = true;
        }
        else if (aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_TIME)))
        {
        m_ouTime = rtl::OUString::createFromAscii("");
            m_bInTime = true;
        }

    if (m_xNextHandler.is())
    {
        m_xNextHandler->startElement(aName, xAttribs);
    }
}

void SAL_CALL XSecParser::endElement( const rtl::OUString& aName )
    throw (cssxs::SAXException, cssu::RuntimeException)
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
    else if ( aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_DATE)) )
    {
        m_pXSecController->setDate( m_ouDate );
            m_bInDate = false;
    }
    else if ( aName == rtl::OUString(RTL_ASCII_USTRINGPARAM(TAG_TIME)) )
    {
        m_pXSecController->setTime( m_ouTime );
            m_bInTime = false;
    }

    if (m_xNextHandler.is())
    {
        m_xNextHandler->endElement(aName);
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
    else if (m_bInTime)
    {
        m_ouTime += aChars;
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
