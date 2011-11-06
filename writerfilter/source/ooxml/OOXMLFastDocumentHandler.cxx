/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
