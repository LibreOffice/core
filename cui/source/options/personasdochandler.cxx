/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
    if( m_isLearnmoreTag )
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
    const Reference< xml::sax::XAttributeList > &xAttribs )
        throw ( xml::sax::SAXException,
            RuntimeException, std::exception )
{
    if( aName == "searchresults" )
    {
        OUString aTotalResults = xAttribs->getValueByName( "total_results" );
        if( aTotalResults != "0" )
            m_hasResults = true;
    }

    if ( aName == "learnmore" )
        m_isLearnmoreTag = true;
    else
        m_isLearnmoreTag = false;
}

void SAL_CALL PersonasDocHandler::endElement( const OUString & )
   throw ( xml::sax::SAXException, RuntimeException, std::exception )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
