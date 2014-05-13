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

#include "personasdochandler.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// XDocumentHandler
void SAL_CALL
PersonasDocHandler::startDocument()
throw ( xml::sax::SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL
PersonasDocHandler::endDocument()
throw ( xml::sax::SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL
PersonasDocHandler::characters( const OUString & aChars)
    throw ( xml::sax::SAXException, RuntimeException, std::exception )
{
    if( m_bLearnmoreTag )
        m_vLearnmoreURLs.push_back( aChars );
}

void SAL_CALL
PersonasDocHandler::ignorableWhitespace( const OUString & )
    throw ( xml::sax::SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL
PersonasDocHandler::processingInstruction(
    const OUString &, const OUString & )
        throw ( xml::sax::SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL
PersonasDocHandler::setDocumentLocator(
    const Reference< xml::sax::XLocator >& )
        throw ( xml::sax::SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL
PersonasDocHandler::startElement( const OUString& aName,
    const Reference< xml::sax::XAttributeList > & )
        throw ( xml::sax::SAXException,
            RuntimeException, std::exception )
{
    if ( aName == "learnmore" )
        m_bLearnmoreTag = true;
    else
        m_bLearnmoreTag = false;
}

void SAL_CALL PersonasDocHandler::endElement( const OUString & )
   throw ( xml::sax::SAXException, RuntimeException, std::exception )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
