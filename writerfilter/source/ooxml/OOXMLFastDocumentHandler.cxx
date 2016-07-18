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

#include <iostream>
#include "OOXMLFastDocumentHandler.hxx"
#include "OOXMLFastContextHandler.hxx"
#include "OOXMLFactory.hxx"

namespace writerfilter {
namespace ooxml
{
using namespace ::com::sun::star;
using namespace ::std;


OOXMLFastDocumentHandler::OOXMLFastDocumentHandler(
    uno::Reference< uno::XComponentContext > const & context,
    Stream* pStream,
    OOXMLDocumentImpl* pDocument,
    sal_Int32 nXNoteId )
    : m_xContext(context)
    , mpStream( pStream )
    , mpDocument( pDocument )
    , mnXNoteId( nXNoteId )
    , mxContextHandler()
{
}

OOXMLFastDocumentHandler::~OOXMLFastDocumentHandler() {}

// css::xml::sax::XFastContextHandler:
void SAL_CALL OOXMLFastDocumentHandler::startFastElement
(::sal_Int32
#ifdef DEBUG_WRITERFILTER
Element
#endif
, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
#ifdef DEBUG_WRITERFILTER
    clog << this << ":start element:"
         << fastTokenToId(Element)
         << endl;
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::startUnknownElement
(const OUString &
#ifdef DEBUG_WRITERFILTER
Namespace
#endif
, const OUString &
#ifdef DEBUG_WRITERFILTER
Name
#endif
,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
#ifdef DEBUG_WRITERFILTER
    clog << this << ":start unknown element:"
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::endFastElement(::sal_Int32
#ifdef DEBUG_WRITERFILTER
Element
#endif
)
throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
#ifdef DEBUG_WRITERFILTER
    clog << this << ":end element:"
         << fastTokenToId(Element)
         << endl;
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::endUnknownElement
(const OUString &
#ifdef DEBUG_WRITERFILTER
Namespace
#endif
, const OUString &
#ifdef DEBUG_WRITERFILTER
Name
#endif
)
throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
#ifdef DEBUG_WRITERFILTER
    clog << this << ":end unknown element:"
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
#endif
}

rtl::Reference< OOXMLFastContextHandler > const &
OOXMLFastDocumentHandler::getContextHandler() const
{
    if (!mxContextHandler.is())
    {
        mxContextHandler = new OOXMLFastContextHandler(m_xContext);
        mxContextHandler->setStream(mpStream);
        mxContextHandler->setDocument(mpDocument);
        mxContextHandler->setXNoteId(mnXNoteId);
        mxContextHandler->setForwardEvents(true);
    }

    return mxContextHandler;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
 OOXMLFastDocumentHandler::createFastChildContext
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    if ( mpStream == nullptr && mpDocument == nullptr )
    {
        // document handler has been created as unknown child - see <OOXMLFastDocumentHandler::createUnknownChildContext(..)>
        // --> do not provide a child context
        return nullptr;
    }

    return OOXMLFactory::createFastChildContextFromStart(getContextHandler().get(), Element);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastDocumentHandler::createUnknownChildContext
(const OUString &
#ifdef DEBUG_WRITERFILTER
Namespace
#endif
,
 const OUString &
#ifdef DEBUG_WRITERFILTER
Name
#endif
, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
#ifdef DEBUG_WRITERFILTER
    clog << this << ":createUnknownChildContext:"
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
#endif

    return uno::Reference< xml::sax::XFastContextHandler >
        ( new OOXMLFastDocumentHandler( m_xContext, nullptr, nullptr, 0 ) );
}

void SAL_CALL OOXMLFastDocumentHandler::characters(const OUString & /*aChars*/)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

// css::xml::sax::XFastDocumentHandler:
void SAL_CALL OOXMLFastDocumentHandler::startDocument()
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

void SAL_CALL OOXMLFastDocumentHandler::endDocument()
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

void SAL_CALL OOXMLFastDocumentHandler::setDocumentLocator
(const uno::Reference< xml::sax::XLocator > & /*xLocator*/)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

void OOXMLFastDocumentHandler::setIsSubstream( bool bSubstream )
{
    if ( mpStream != nullptr && mpDocument != nullptr )
    {
        getContextHandler( )->getParserState( )->setInSectionGroup( bSubstream );
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
