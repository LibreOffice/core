/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "oox/core/fragmenthandler2.hxx"
#include "oox/core/xmlfilterbase.hxx"

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using ::rtl::OUString;


using ::com::sun::star::uno::Sequence;

// ============================================================================

FragmentHandler2::FragmentHandler2( XmlFilterBase& rFilter, const OUString& rFragmentPath, bool bEnableTrimSpace ) :
    FragmentHandler( rFilter, rFragmentPath ),
    ContextHandler2Helper( bEnableTrimSpace )
{
}

FragmentHandler2::~FragmentHandler2()
{
}

// com.sun.star.xml.sax.XFastDocumentHandler interface --------------------

void SAL_CALL FragmentHandler2::startDocument() throw( SAXException, RuntimeException )
{
    initializeImport();
}

void SAL_CALL FragmentHandler2::endDocument() throw( SAXException, RuntimeException )
{
    finalizeImport();
}

bool FragmentHandler2::prepareMceContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case MCE_TOKEN( AlternateContent ):
            aMceState.push_back( MCE_STARTED );
            break;

        case MCE_TOKEN( Choice ):
            {
                OUString aRequires = rAttribs.getString( ( XML_Requires ), OUString(RTL_CONSTASCII_USTRINGPARAM("none")) );
                aRequires = getFilter().getNamespaceURL( aRequires );
                if( getFilter().getNamespaceId( aRequires ) > 0 && !aMceState.empty() && aMceState.back() == MCE_STARTED )
                    aMceState.back() = MCE_FOUND_CHOICE;
                else
                    return false;
            }
            break;

        case MCE_TOKEN( Fallback ):
            if( !aMceState.empty() && aMceState.back() == MCE_STARTED )
                break;
            return false;
        default:
            {
                OUString str = rAttribs.getString( MCE_TOKEN( Ignorable ), OUString() );
                if( !str.isEmpty() )
                {
                    Sequence< ::com::sun::star::xml::FastAttribute > attrs = rAttribs.getFastAttributeList()->getFastAttributes();
                    // printf("MCE: %s\n", ::rtl::OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
                    // TODO: Check & Get the namespaces in "Ignorable"
                    // printf("NS: %d : %s\n", attrs.getLength(), ::rtl::OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }
            return false;
    }
    return true;
}



// com.sun.star.xml.sax.XFastContextHandler interface -------------------------

Reference< XFastContextHandler > SAL_CALL FragmentHandler2::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException )
{
    if( getNamespace( nElement ) == NMSP_mce ) // TODO for checking 'Ignorable'
    {
        if( prepareMceContext( nElement, AttributeList( rxAttribs ) ) )
            return getFastContextHandler();
        return NULL;
    }
    return implCreateChildContext( nElement, rxAttribs );
}

void SAL_CALL FragmentHandler2::startFastElement(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException )
{
    implStartElement( nElement, rxAttribs );
}

void SAL_CALL FragmentHandler2::characters( const OUString& rChars ) throw( SAXException, RuntimeException )
{
    implCharacters( rChars );
}

void SAL_CALL FragmentHandler2::endFastElement( sal_Int32 nElement ) throw( SAXException, RuntimeException )
{
    /* If MCE */
    switch( nElement )
    {
        case MCE_TOKEN( AlternateContent ):
            aMceState.pop_back();
            break;
    }

    implEndElement( nElement );
}

// oox.core.ContextHandler interface ------------------------------------------

ContextHandlerRef FragmentHandler2::createRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    return implCreateRecordContext( nRecId, rStrm );
}

void FragmentHandler2::startRecord( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    implStartRecord( nRecId, rStrm );
}

void FragmentHandler2::endRecord( sal_Int32 nRecId )
{
    implEndRecord( nRecId );
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef FragmentHandler2::onCreateContext( sal_Int32, const AttributeList& )
{
    return 0;
}

void FragmentHandler2::onStartElement( const AttributeList& )
{
}

void FragmentHandler2::onCharacters( const OUString& )
{
}

void FragmentHandler2::onEndElement()
{
}

ContextHandlerRef FragmentHandler2::onCreateRecordContext( sal_Int32, SequenceInputStream& )
{
    return 0;
}

void FragmentHandler2::onStartRecord( SequenceInputStream& )
{
}

void FragmentHandler2::onEndRecord()
{
}

// oox.core.FragmentHandler2 interface ----------------------------------------

void FragmentHandler2::initializeImport()
{
}

void FragmentHandler2::finalizeImport()
{
}

// ============================================================================

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
