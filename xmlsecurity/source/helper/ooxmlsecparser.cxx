/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "ooxmlsecparser.hxx"

using namespace com::sun::star;

OOXMLSecParser::OOXMLSecParser(XSecController* pXSecController)
    : m_pXSecController(pXSecController),
      m_bInDigestValue(false)
{
}

OOXMLSecParser::~OOXMLSecParser()
{
}

void SAL_CALL OOXMLSecParser::startDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
        m_xNextHandler->startDocument();
}

void SAL_CALL OOXMLSecParser::endDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
        m_xNextHandler->endDocument();
}

void SAL_CALL OOXMLSecParser::startElement(const OUString& rName, const uno::Reference<xml::sax::XAttributeList>& xAttribs)
throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    OUString aId = xAttribs->getValueByName("Id");
    if (!aId.isEmpty())
        m_pXSecController->collectToVerify(aId);

    if (rName == "Signature")
    {
        m_pXSecController->addSignature();
        if (!aId.isEmpty())
            m_pXSecController->setId(aId);
    }
    else if (rName == "Reference")
    {
        OUString aURI = xAttribs->getValueByName("URI");
        if (aURI.startsWith("#"))
            m_pXSecController->addReference(aURI.copy(1));
        // TODO else
    }
    else if (rName == "DigestValue")
    {
        m_aDigestValue.clear();
        m_bInDigestValue = true;
    }

    if (m_xNextHandler.is())
        m_xNextHandler->startElement(rName, xAttribs);
}

void SAL_CALL OOXMLSecParser::endElement(const OUString& rName) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (rName == "SignedInfo")
        m_pXSecController->setReferenceCount();
    else if (rName == "Reference")
        m_pXSecController->setDigestValue(m_aDigestValue);
    else if (rName == "DigestValue")
        m_bInDigestValue = false;

    if (m_xNextHandler.is())
        m_xNextHandler->endElement(rName);
}

void SAL_CALL OOXMLSecParser::characters(const OUString& rChars) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (m_bInDigestValue)
        m_aDigestValue += rChars;

    if (m_xNextHandler.is())
        m_xNextHandler->characters(rChars);
}

void SAL_CALL OOXMLSecParser::ignorableWhitespace(const OUString& rWhitespace) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
        m_xNextHandler->ignorableWhitespace(rWhitespace);
}

void SAL_CALL OOXMLSecParser::processingInstruction(const OUString& rTarget, const OUString& rData) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
        m_xNextHandler->processingInstruction(rTarget, rData);
}

void SAL_CALL OOXMLSecParser::setDocumentLocator(const uno::Reference<xml::sax::XLocator>& xLocator) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
        m_xNextHandler->setDocumentLocator(xLocator);
}

void SAL_CALL OOXMLSecParser::initialize(const uno::Sequence<uno::Any>& rArguments) throw (uno::Exception, uno::RuntimeException, std::exception)
{
    rArguments[0] >>= m_xNextHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
