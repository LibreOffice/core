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

namespace writerfilter::ooxml
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
(::sal_Int32 Element
, const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
#ifdef DBG_UTIL
    clog << this << ":start element:"
         << fastTokenToId(Element)
         << endl;
#endif
    if ( mpStream == nullptr && mpDocument == nullptr )
    {
        // document handler has been created as unknown child - see <OOXMLFastDocumentHandler::createUnknownChildContext(..)>
        // --> do not provide a child context
        maContexts.push(nullptr);
        return;
    }

    css::uno::Reference< css::xml::sax::XFastContextHandler> xContext;
    if (maContexts.empty())
        xContext = OOXMLFactory::createFastChildContextFromStart(getContextHandler().get(), Element);
    else
    {
        auto & parentContext = maContexts.top();
        if (!parentContext)
            // document handler has been created as unknown child - see <OOXMLFastDocumentHandler::createUnknownChildContext(..)>
            // --> do not provide a child context
            ;
        else
        {
            xContext = parentContext->createFastChildContext(Element, Attribs);
        }
    }
    if (xContext)
        xContext->startFastElement(Element, Attribs);
    maContexts.push(xContext);
}

void SAL_CALL OOXMLFastDocumentHandler::startUnknownElement
(const OUString &
#ifdef DBG_UTIL
Namespace
#endif
, const OUString &
#ifdef DBG_UTIL
Name
#endif
,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
{
#ifdef DBG_UTIL
    clog << this << ":start unknown element:"
         << Namespace  << ":" << Name << endl;
#endif
    maContexts.push(nullptr);
}

void SAL_CALL OOXMLFastDocumentHandler::endFastElement(::sal_Int32 Element)
{
#ifdef DBG_UTIL
    clog << this << ":end element:"
         << fastTokenToId(Element)
         << endl;
#endif
    auto & parentContext = maContexts.top();
    if (parentContext)
        parentContext->endFastElement(Element);
    maContexts.pop();
}

void SAL_CALL OOXMLFastDocumentHandler::endUnknownElement
(const OUString &
#ifdef DBG_UTIL
Namespace
#endif
, const OUString &
#ifdef DBG_UTIL
Name
#endif
)
{
#ifdef DBG_UTIL
    clog << this << ":end unknown element:"
         << Namespace << ":" << Name
         << endl;
#endif
    maContexts.pop();
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
