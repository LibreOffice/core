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

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::xml::sax::XFastContextHandler;

namespace oox {
namespace core {

// ============================================================================

FragmentHandler2::FragmentHandler2( XmlFilterBase& rFilter, const OUString& rFragmentPath, bool bEnableTrimSpace ) :
    FragmentHandler( rFilter, rFragmentPath ),
    ContextHandler2Helper( bEnableTrimSpace )
{
}

FragmentHandler2::~FragmentHandler2()
{
}

ContextHandler& FragmentHandler2::queryContextHandler()
{
    return *this;
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

// com.sun.star.xml.sax.XFastContextHandler interface -------------------------

Reference< XFastContextHandler > SAL_CALL FragmentHandler2::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException )
{
    return implCreateChildContext( nElement, rxAttribs );
}

void SAL_CALL FragmentHandler2::startFastElement(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException )
{
    implStartCurrentContext( nElement, rxAttribs );
}

void SAL_CALL FragmentHandler2::characters( const OUString& rChars ) throw( SAXException, RuntimeException )
{
    implCharacters( rChars );
}

void SAL_CALL FragmentHandler2::endFastElement( sal_Int32 nElement ) throw( SAXException, RuntimeException )
{
    implEndCurrentContext( nElement );
}

// oox.core.ContextHandler interface ------------------------------------------

ContextHandlerRef FragmentHandler2::createRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    return implCreateRecordContext( nRecId, rStrm );
}

void FragmentHandler2::startRecord( sal_Int32 nRecId, RecordInputStream& rStrm )
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

void FragmentHandler2::onEndElement( const OUString& )
{
}

ContextHandlerRef FragmentHandler2::onCreateRecordContext( sal_Int32, RecordInputStream& )
{
    return 0;
}

void FragmentHandler2::onStartRecord( RecordInputStream& )
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

