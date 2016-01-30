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

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <stdio.h>

#include <xml/saxnamespacefilter.hxx>

#include <comphelper/attributelist.hxx>

#include <vcl/svapp.hxx>

using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

namespace framework{

SaxNamespaceFilter::SaxNamespaceFilter( Reference< XDocumentHandler >& rSax1DocumentHandler ) :
     m_xLocator( nullptr ),
     xDocumentHandler( rSax1DocumentHandler ),
     m_aXMLAttributeNamespace( "xmlns" ),
     m_aXMLAttributeType( "CDATA" )
{
}

SaxNamespaceFilter::~SaxNamespaceFilter()
{
}

// XDocumentHandler
void SAL_CALL SaxNamespaceFilter::startDocument()
    throw ( SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL SaxNamespaceFilter::endDocument()
    throw(  SAXException, RuntimeException, std::exception )
{
}

void SAL_CALL SaxNamespaceFilter::startElement(
    const OUString& rName, const Reference< XAttributeList > &xAttribs )
    throw(  SAXException, RuntimeException, std::exception )
{
    XMLNamespaces aXMLNamespaces;
    if ( !m_aNamespaceStack.empty() )
        aXMLNamespaces = m_aNamespaceStack.top();

    ::comphelper::AttributeList* pNewList = new ::comphelper::AttributeList();

    // examine all namespaces for this level
    ::std::vector< sal_Int16 > aAttributeIndexes;
    {
        for ( sal_Int16 i=0; i< xAttribs->getLength(); i++ )
        {
            OUString aName = xAttribs->getNameByIndex( i );
            if ( aName.startsWith( m_aXMLAttributeNamespace ) )
                aXMLNamespaces.addNamespace( aName, xAttribs->getValueByIndex( i ));
            else
                aAttributeIndexes.push_back( i );
        }
    }

    // current namespaces for this level
    m_aNamespaceStack.push( aXMLNamespaces );

    try
    {
        // apply namespaces to all remaining attributes
        for ( ::std::vector< sal_Int16 >::const_iterator i(
                  aAttributeIndexes.begin());
              i != aAttributeIndexes.end(); ++i )
        {
            OUString aAttributeName           = xAttribs->getNameByIndex( *i );
            OUString aValue                   = xAttribs->getValueByIndex( *i );
            OUString aNamespaceAttributeName = aXMLNamespaces.applyNSToAttributeName( aAttributeName );
            pNewList->AddAttribute( aNamespaceAttributeName, m_aXMLAttributeType, aValue );
        }
    }
    catch ( SAXException& e )
    {
        e.Message = getErrorLineString() + e.Message;
        throw;
    }

    OUString aNamespaceElementName;

    try
    {
         aNamespaceElementName = aXMLNamespaces.applyNSToElementName( rName );
    }
    catch ( SAXException& e )
    {
        e.Message = getErrorLineString() + e.Message;
        throw;
    }

    xDocumentHandler->startElement( aNamespaceElementName, pNewList );
}

void SAL_CALL SaxNamespaceFilter::endElement(const OUString& aName)
    throw(  SAXException, RuntimeException, std::exception )
{
    XMLNamespaces& aXMLNamespaces = m_aNamespaceStack.top();
    OUString aNamespaceElementName;

    try
    {
        aNamespaceElementName = aXMLNamespaces.applyNSToElementName( aName );
    }
    catch ( SAXException& e )
    {
        e.Message = getErrorLineString() + e.Message;
        throw;
    }

    xDocumentHandler->endElement( aNamespaceElementName );
    m_aNamespaceStack.pop();
}

void SAL_CALL SaxNamespaceFilter::characters(const OUString& aChars)
    throw(  SAXException, RuntimeException, std::exception )
{
    xDocumentHandler->characters( aChars );
}

void SAL_CALL SaxNamespaceFilter::ignorableWhitespace(const OUString& aWhitespaces)
    throw(  SAXException, RuntimeException, std::exception )
{
    xDocumentHandler->ignorableWhitespace( aWhitespaces );
}

void SAL_CALL SaxNamespaceFilter::processingInstruction(
    const OUString& aTarget, const OUString& aData)
    throw(  SAXException, RuntimeException, std::exception )
{
    xDocumentHandler->processingInstruction( aTarget, aData );
}

void SAL_CALL SaxNamespaceFilter::setDocumentLocator(
    const Reference< XLocator > &xLocator)
    throw(  SAXException, RuntimeException, std::exception )
{
    m_xLocator = xLocator;
    xDocumentHandler->setDocumentLocator( xLocator );
}

OUString SaxNamespaceFilter::getErrorLineString()
{
    if ( m_xLocator.is() )
    {
        char buffer[32];
        snprintf( buffer, sizeof(buffer), "Line: %ld - ", static_cast<long>( m_xLocator->getLineNumber() ));
        return OUString::createFromAscii( buffer );
    }
    else
        return OUString();
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
