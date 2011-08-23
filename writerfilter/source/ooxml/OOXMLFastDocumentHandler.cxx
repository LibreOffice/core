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

#include <iostream>
#include <boost/shared_ptr.hpp>
#ifdef DEBUG_ELEMENT
#include "ooxmlLoggers.hxx"
#include <resourcemodel/Protocol.hxx>
#endif
#include "OOXMLFastDocumentHandler.hxx"
#include "OOXMLFastContextHandler.hxx"
#include "OOXMLFastTokens.hxx"
#include "OOXMLFactory.hxx"

namespace writerfilter {
namespace ooxml
{
using namespace ::com::sun::star;
using namespace ::std;


OOXMLFastDocumentHandler::OOXMLFastDocumentHandler
(uno::Reference< uno::XComponentContext > const & context)
: m_xContext(context)
{}

// ::com::sun::star::xml::sax::XFastContextHandler:
void SAL_CALL OOXMLFastDocumentHandler::startFastElement
(::sal_Int32 
#ifdef DEBUG_CONTEXT_STACK
Element
#endif
, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":start element:" 
         << fastTokenToId(Element)
         << endl;
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::startUnknownElement
(const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Namespace
#endif
, const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Name
#endif
, 
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/) 
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":start unknown element:" 
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::endFastElement(::sal_Int32 
#ifdef DEBUG_CONTEXT_STACK
Element
#endif
) 
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":end element:" 
         << fastTokenToId(Element)
         << endl;
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::endUnknownElement
(const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Namespace
#endif
, const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Name
#endif
) 
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":end unknown element:" 
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
#endif
}

OOXMLFastContextHandler::Pointer_t 
OOXMLFastDocumentHandler::getContextHandler() const
{
    if (mpContextHandler == OOXMLFastContextHandler::Pointer_t())
    {
        mpContextHandler.reset
        (new OOXMLFastContextHandler(m_xContext));
        mpContextHandler->setStream(mpStream);
        mpContextHandler->setDocument(mpDocument);
        mpContextHandler->setXNoteId(msXNoteId);
        mpContextHandler->setForwardEvents(true);
    }

    return mpContextHandler;
}
    
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
 OOXMLFastDocumentHandler::createFastChildContext
(::sal_Int32 Element, 
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":createFastChildContext:" 
         << fastTokenToId(Element)
         << endl;
#endif
    
    return OOXMLFactory::getInstance()->createFastChildContextFromStart(getContextHandler().get(), Element);
}
    
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL 
OOXMLFastDocumentHandler::createUnknownChildContext
(const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Namespace
#endif
, 
 const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Name
#endif
, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":createUnknownChildContext:" 
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
#endif

    return uno::Reference< xml::sax::XFastContextHandler >
        (new OOXMLFastDocumentHandler(m_xContext));
}

void SAL_CALL OOXMLFastDocumentHandler::characters(const ::rtl::OUString & /*aChars*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    // TODO: Insert your implementation for "characters" here.
}

// ::com::sun::star::xml::sax::XFastDocumentHandler:
void SAL_CALL OOXMLFastDocumentHandler::startDocument() 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
}

void SAL_CALL OOXMLFastDocumentHandler::endDocument() 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    OOXMLFastContextHandler::dumpOpenContexts();
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::setDocumentLocator
(const uno::Reference< xml::sax::XLocator > & /*xLocator*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    // TODO: Insert your implementation for "setDocumentLocator" here.
}

void OOXMLFastDocumentHandler::setStream(Stream * pStream)
{ 
#ifdef DEBUG_PROTOCOL
    mpTmpStream.reset(new StreamProtocol(pStream, debug_logger));
    mpStream = mpTmpStream.get();
#else
    mpStream = pStream;
#endif
}

void OOXMLFastDocumentHandler::setDocument(OOXMLDocument * pDocument)
{
    mpDocument = pDocument;
}

void OOXMLFastDocumentHandler::setXNoteId(const ::rtl::OUString & rXNoteId)
{
    msXNoteId = rXNoteId;
}

void OOXMLFastDocumentHandler::setIsSubstream( bool bSubstream )
{
    getContextHandler( )->getParserState( )->setInSectionGroup( bSubstream );
}

}}
