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
    : m_pXSecController(pXSecController)
{
}

OOXMLSecParser::~OOXMLSecParser()
{
}

void SAL_CALL OOXMLSecParser::startDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL OOXMLSecParser::endDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL OOXMLSecParser::startElement(const OUString& /*rName*/, const uno::Reference<xml::sax::XAttributeList>& /*xAttribs*/)
throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL OOXMLSecParser::endElement(const OUString& /*rName*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL OOXMLSecParser::characters(const OUString& /*rChars*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL OOXMLSecParser::ignorableWhitespace(const OUString& /*rWhitespace*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL OOXMLSecParser::processingInstruction(const OUString& /*rTarget*/, const OUString& /*rData*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL OOXMLSecParser::setDocumentLocator(const uno::Reference<xml::sax::XLocator>& /*xLocator*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL OOXMLSecParser::initialize(const uno::Sequence<uno::Any>& /*rArguments*/) throw (uno::Exception, uno::RuntimeException, std::exception)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
