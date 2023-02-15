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

#include "OOXMLFastDocumentHandler.hxx"
#include "OOXMLFastContextHandler.hxx"
#include "OOXMLFactory.hxx"
#include <sal/log.hxx>
#include <utility>

namespace writerfilter::ooxml
{
using namespace ::com::sun::star;


OOXMLFastDocumentHandler::OOXMLFastDocumentHandler(
    uno::Reference< uno::XComponentContext > context,
    Stream* pStream,
    OOXMLDocumentImpl* pDocument,
    sal_Int32 nXNoteId )
    : m_xContext(std::move(context))
    , mpStream( pStream )
    , mpDocument( pDocument )
    , mnXNoteId( nXNoteId )
{
}

OOXMLFastDocumentHandler::~OOXMLFastDocumentHandler() {}

// css::xml::sax::XFastContextHandler:
void SAL_CALL OOXMLFastDocumentHandler::startFastElement(sal_Int32 Element
, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
{
    SAL_INFO("writerfilter", "start element:" << fastTokenToId(Element));
}

void SAL_CALL OOXMLFastDocumentHandler::startUnknownElement
(const OUString & Namespace
, const OUString & Name
, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
{
    SAL_INFO("writerfilter", "start unknown element:" << Namespace  << ":" << Name);
}

void SAL_CALL OOXMLFastDocumentHandler::endFastElement(sal_Int32 Element)
{
    SAL_INFO("writerfilter", "end element:" << fastTokenToId(Element));
}

void SAL_CALL OOXMLFastDocumentHandler::endUnknownElement
(const OUString & Namespace
, const OUString & Name)
{
    SAL_INFO("writerfilter", "end unknown element:" << Namespace << ":" << Name);
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
(const OUString & Namespace
, const OUString & Name
, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
{
    SAL_INFO("writerfilter", "createUnknownChildContext:" << Namespace << ":"<< Name);

    return uno::Reference< xml::sax::XFastContextHandler >
        ( new OOXMLFastDocumentHandler( m_xContext, nullptr, nullptr, 0 ) );
}

void SAL_CALL OOXMLFastDocumentHandler::characters(const OUString & /*aChars*/)
{
}

// css::xml::sax::XFastDocumentHandler:
void SAL_CALL OOXMLFastDocumentHandler::startDocument()
{
}

void SAL_CALL OOXMLFastDocumentHandler::endDocument()
{
}

void SAL_CALL OOXMLFastDocumentHandler::processingInstruction( const OUString& /*rTarget*/, const OUString& /*rData*/ )
{
}

void SAL_CALL OOXMLFastDocumentHandler::setDocumentLocator
(const uno::Reference< xml::sax::XLocator > & /*xLocator*/)
{
}

void OOXMLFastDocumentHandler::setIsSubstream( bool bSubstream )
{
    if ( mpStream != nullptr && mpDocument != nullptr )
    {
        getContextHandler( )->getParserState( )->setInSectionGroup( bSubstream );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
