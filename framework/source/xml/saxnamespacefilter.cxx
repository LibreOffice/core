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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <stdio.h>

#include <xml/saxnamespacefilter.hxx>

#include <comphelper/attributelist.hxx>

#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>

using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

const ::rtl::OUString aXMLAttributeNamespace( RTL_CONSTASCII_USTRINGPARAM( "xmlns" ));
const ::rtl::OUString aXMLAttributeType( RTL_CONSTASCII_USTRINGPARAM( "CDATA" ));

namespace framework{


SaxNamespaceFilter::SaxNamespaceFilter( Reference< XDocumentHandler >& rSax1DocumentHandler ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
     m_xLocator( 0 ),
     xDocumentHandler( rSax1DocumentHandler ),
     m_nDepth( 0 )
{
}

SaxNamespaceFilter::~SaxNamespaceFilter()
{
}

// XDocumentHandler
void SAL_CALL SaxNamespaceFilter::startDocument(void)
    throw ( SAXException, RuntimeException )
{
}

void SAL_CALL SaxNamespaceFilter::endDocument(void)
    throw(  SAXException, RuntimeException )
{
}

void SAL_CALL SaxNamespaceFilter::startElement(
    const rtl::OUString& rName, const Reference< XAttributeList > &xAttribs )
    throw(  SAXException, RuntimeException )
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
            ::rtl::OUString aName = xAttribs->getNameByIndex( i );
            if ( aName.compareTo( aXMLAttributeNamespace, aXMLAttributeNamespace.getLength() ) == 0 )
                aXMLNamespaces.addNamespace( aName, xAttribs->getValueByIndex( i ));
            else
                aAttributeIndexes.push_back( i );
        }
    }

    // current namespaces for this level
    m_aNamespaceStack.push( aXMLNamespaces );

    try
    {
        // apply namespaces to all remaing attributes
        for ( ::std::vector< sal_Int16 >::const_iterator i(
                  aAttributeIndexes.begin());
              i != aAttributeIndexes.end(); ++i )
        {
            ::rtl::OUString aAttributeName           = xAttribs->getNameByIndex( *i );
            ::rtl::OUString aValue                   = xAttribs->getValueByIndex( *i );
            ::rtl::OUString aNamespaceAttributeName = aXMLNamespaces.applyNSToAttributeName( aAttributeName );
            pNewList->AddAttribute( aNamespaceAttributeName, aXMLAttributeType, aValue );
        }
    }
    catch ( SAXException& e )
    {
        e.Message = ::rtl::OUString( getErrorLineString() + e.Message );
        throw e;
    }

    ::rtl::OUString aNamespaceElementName;

    try
    {
         aNamespaceElementName = aXMLNamespaces.applyNSToElementName( rName );
    }
    catch ( SAXException& e )
    {
        e.Message = ::rtl::OUString( getErrorLineString() + e.Message );
        throw e;
    }

    xDocumentHandler->startElement( aNamespaceElementName, pNewList );
}

void SAL_CALL SaxNamespaceFilter::endElement(const rtl::OUString& aName)
    throw(  SAXException, RuntimeException )
{
    XMLNamespaces& aXMLNamespaces = m_aNamespaceStack.top();
    ::rtl::OUString aNamespaceElementName;

    try
    {
        aNamespaceElementName = aXMLNamespaces.applyNSToElementName( aName );
    }
    catch ( SAXException& e )
    {
        e.Message = ::rtl::OUString( getErrorLineString() + e.Message );
        throw e;
    }

    xDocumentHandler->endElement( aNamespaceElementName );
    m_aNamespaceStack.pop();
}

void SAL_CALL SaxNamespaceFilter::characters(const rtl::OUString& aChars)
    throw(  SAXException, RuntimeException )
{
    xDocumentHandler->characters( aChars );
}

void SAL_CALL SaxNamespaceFilter::ignorableWhitespace(const rtl::OUString& aWhitespaces)
    throw(  SAXException, RuntimeException )
{
    xDocumentHandler->ignorableWhitespace( aWhitespaces );
}

void SAL_CALL SaxNamespaceFilter::processingInstruction(
    const rtl::OUString& aTarget, const rtl::OUString& aData)
    throw(  SAXException, RuntimeException )
{
    xDocumentHandler->processingInstruction( aTarget, aData );
}

void SAL_CALL SaxNamespaceFilter::setDocumentLocator(
    const Reference< XLocator > &xLocator)
    throw(  SAXException, RuntimeException )
{
    m_xLocator = xLocator;
    xDocumentHandler->setDocumentLocator( xLocator );
}

::rtl::OUString SaxNamespaceFilter::getErrorLineString()
{
    char buffer[32];

    if ( m_xLocator.is() )
    {
        snprintf( buffer, sizeof(buffer), "Line: %ld - ", static_cast<long>( m_xLocator->getLineNumber() ));
        return ::rtl::OUString::createFromAscii( buffer );
    }
    else
        return ::rtl::OUString();
}

} // namespace

