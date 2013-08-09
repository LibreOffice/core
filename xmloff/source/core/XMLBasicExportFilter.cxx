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

#include "XMLBasicExportFilter.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// XMLBasicExportFilter

XMLBasicExportFilter::XMLBasicExportFilter( const Reference< xml::sax::XDocumentHandler >& rxHandler )
    :m_xHandler( rxHandler )
{
}

XMLBasicExportFilter::~XMLBasicExportFilter()
{
}

// XDocumentHandler

void XMLBasicExportFilter::startDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // do nothing, filter this
}

void XMLBasicExportFilter::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // do nothing, filter this
}

void XMLBasicExportFilter::startElement( const OUString& aName,
        const Reference< xml::sax::XAttributeList >& xAttribs )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->startElement( aName, xAttribs );
}

void XMLBasicExportFilter::endElement( const OUString& aName )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->endElement( aName );
}

void XMLBasicExportFilter::characters( const OUString& aChars )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->characters( aChars );
}

void XMLBasicExportFilter::ignorableWhitespace( const OUString& aWhitespaces )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->ignorableWhitespace( aWhitespaces );
}

void XMLBasicExportFilter::processingInstruction( const OUString& aTarget,
        const OUString& aData )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->processingInstruction( aTarget, aData );
}

void XMLBasicExportFilter::setDocumentLocator( const Reference< xml::sax::XLocator >& xLocator )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->setDocumentLocator( xLocator );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
