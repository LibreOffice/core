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

#include <xml/xmlnamespaces.hxx>

#include <com/sun/star/xml/sax/SAXException.hpp>

using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

namespace framework
{

void XMLNamespaces::addNamespace( const OUString& aName, const OUString& aValue )
{
    NamespaceMap::iterator p;
    OUString aNamespaceName( aName );

    // delete preceding "xmlns"
    constexpr char aXMLAttributeNamespace[] = "xmlns";
    if ( aNamespaceName.startsWith( aXMLAttributeNamespace ) )
    {
        constexpr sal_Int32 nXMLNamespaceLength = RTL_CONSTASCII_LENGTH(aXMLAttributeNamespace);
        if ( aNamespaceName.getLength() == nXMLNamespaceLength )
        {
            aNamespaceName.clear();
        }
        else if ( aNamespaceName.getLength() >= nXMLNamespaceLength+2 )
        {
            aNamespaceName = aNamespaceName.copy( nXMLNamespaceLength+1 );
        }
        else
        {
            // a xml namespace without name is not allowed (e.g. "xmlns:" )
            throw SAXException( u"A xml namespace without name is not allowed!"_ustr, Reference< XInterface >(), Any() );
        }
    }

    if ( aValue.isEmpty() && !aNamespaceName.isEmpty() )
    {
        // namespace should be reset - as xml draft states this is only allowed
        // for the default namespace - check and throw exception if check fails
        throw SAXException( u"Clearing xml namespace only allowed for default namespace!"_ustr, Reference< XInterface >(), Any() );
    }

    if ( aNamespaceName.isEmpty() )
        m_aDefaultNamespace = aValue;
    else
    {
        p = m_aNamespaceMap.find( aNamespaceName );
        if ( p != m_aNamespaceMap.end() )
        {
            // replace current namespace definition
            m_aNamespaceMap.erase( p );
            m_aNamespaceMap.emplace( aNamespaceName, aValue );
        }
        else
        {
            m_aNamespaceMap.emplace( aNamespaceName, aValue );
        }
    }
}

OUString XMLNamespaces::applyNSToAttributeName( const OUString& aName ) const
{
    // xml draft: there is no default namespace for attributes!

    int index;
    if (( index = aName.indexOf( ':' )) > 0 )
    {
        if ( aName.getLength() <= index+1 )
        {
            // attribute with namespace but without name "namespace:" is not allowed!!
            throw SAXException( u"Attribute has no name only preceding namespace!"_ustr, Reference< XInterface >(), Any() );
        }
        OUString aAttributeName = getNamespaceValue( aName.copy( 0, index )) + "^" + aName.subView( index+1);
        return aAttributeName;
    }

    return aName;
}

OUString XMLNamespaces::applyNSToElementName( const OUString& aName ) const
{
    // xml draft: element names can have a default namespace

    int         index = aName.indexOf( ':' );
    OUString aNamespace;
    OUString aElementName = aName;

    if ( index > 0 )
        aNamespace = getNamespaceValue( aName.copy( 0, index ) );
    else
        aNamespace = m_aDefaultNamespace;

    if ( !aNamespace.isEmpty() )
    {
        aElementName = aNamespace + "^";
    }
    else
        return aName;

    if ( index > 0 )
    {
        if ( aName.getLength() <= index+1 )
        {
            // attribute with namespace but without a name is not allowed (e.g. "cfg:" )
            throw SAXException( u"Attribute has no name only preceding namespace!"_ustr, Reference< XInterface >(), Any() );
        }
        aElementName += aName.subView( index+1 );
    }
    else
        aElementName += aName;

    return aElementName;
}

OUString const & XMLNamespaces::getNamespaceValue( const OUString& aNamespace ) const
{
    if ( aNamespace.isEmpty() )
        return m_aDefaultNamespace;
    else
    {
        NamespaceMap::const_iterator p = m_aNamespaceMap.find( aNamespace );
        if ( p == m_aNamespaceMap.end() )
        {
            // namespace not defined => throw exception!
            throw SAXException( u"XML namespace used but not defined!"_ustr, Reference< XInterface >(), Any() );
        }
        return p->second;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
