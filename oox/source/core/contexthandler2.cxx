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

#include "oox/core/contexthandler2.hxx"
#include <rtl/ustrbuf.hxx>

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

/** Information about a processed element. */
struct ElementInfo
{
    OUStringBuffer      maChars;            /// Collected element characters.
    sal_Int32           mnElement;          /// The element identifier.
    bool                mbTrimSpaces;       /// True = trims leading/trailing spaces from text data.

    inline explicit     ElementInfo() : mnElement( XML_TOKEN_INVALID ), mbTrimSpaces( false ) {}
                        ElementInfo( sal_Int32 nElement ) : mnElement( nElement ), mbTrimSpaces(false) {}
};

// ============================================================================

ContextHandler2Helper::ContextHandler2Helper( bool bEnableTrimSpace ) :
    mxContextStack( new ContextStack ),
    mnRootStackSize( 0 ),
    mbEnableTrimSpace( bEnableTrimSpace )
{
    pushElementInfo( XML_ROOT_CONTEXT );
}

ContextHandler2Helper::ContextHandler2Helper( const ContextHandler2Helper& rParent ) :
    mxContextStack( rParent.mxContextStack ),
    mnRootStackSize( rParent.mxContextStack->size() ),
    mbEnableTrimSpace( rParent.mbEnableTrimSpace )
{
}

ContextHandler2Helper::~ContextHandler2Helper()
{
}

sal_Int32 ContextHandler2Helper::getCurrentElementWithMce() const
{
    return mxContextStack->empty() ? XML_ROOT_CONTEXT : mxContextStack->back().mnElement;
}

sal_Int32 ContextHandler2Helper::getCurrentElement() const
{
    for ( ContextStack::reverse_iterator It = mxContextStack->rbegin();
          It != mxContextStack->rend(); ++It )
        if( getNamespace( It->mnElement ) != NMSP_mce )
            return It->mnElement;
    return XML_ROOT_CONTEXT;
}

sal_Int32 ContextHandler2Helper::getParentElement( sal_Int32 nCountBack ) const
{
    if( (nCountBack < 0) || (mxContextStack->size() < static_cast< size_t >( nCountBack )) )
        return XML_TOKEN_INVALID;
    return (mxContextStack->size() == static_cast< size_t >( nCountBack )) ?
        XML_ROOT_CONTEXT : (*mxContextStack)[ mxContextStack->size() - nCountBack - 1 ].mnElement;
}

bool ContextHandler2Helper::isRootElement() const
{
    return mxContextStack->size() == mnRootStackSize + 1;
}

Reference< XFastContextHandler > ContextHandler2Helper::implCreateChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
{
    // #i76091# process collected characters (calls onCharacters() if needed)
    processCollectedChars();
    ContextHandlerRef xContext = onCreateContext( nElement, AttributeList( rxAttribs ) );
    return Reference< XFastContextHandler >( xContext.get() );
}

void ContextHandler2Helper::implStartElement( sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
{
    AttributeList aAttribs( rxAttribs );
    pushElementInfo( nElement ).mbTrimSpaces = aAttribs.getToken( XML_TOKEN( space ), XML_TOKEN_INVALID ) != XML_preserve;
    onStartElement( aAttribs );
}

void ContextHandler2Helper::implCharacters( const OUString& rChars )
{
    // #i76091# collect characters until new element starts or this element ends
    if( !mxContextStack->empty() )
        mxContextStack->back().maChars.append(rChars);
}

void ContextHandler2Helper::implEndElement( sal_Int32 nElement )
{
    (void)nElement;     // prevent "unused parameter" warning in product build
    OSL_ENSURE( getCurrentElementWithMce() == nElement, "ContextHandler2Helper::implEndElement - context stack broken" );
    if( !mxContextStack->empty() )
    {
        // #i76091# process collected characters (calls onCharacters() if needed)
        processCollectedChars();
        onEndElement();
        popElementInfo();
    }
}

ContextHandlerRef ContextHandler2Helper::implCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    return onCreateRecordContext( nRecId, rStrm );
}

void ContextHandler2Helper::implStartRecord( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    pushElementInfo( nRecId );
    onStartRecord( rStrm );
}

void ContextHandler2Helper::implEndRecord( sal_Int32 nRecId )
{
    (void)nRecId;   // prevent "unused parameter" warning in product build
    OSL_ENSURE( getCurrentElementWithMce() == nRecId, "ContextHandler2Helper::implEndRecord - context stack broken" );
    if( !mxContextStack->empty() )
    {
        onEndRecord();
        popElementInfo();
    }
}

ElementInfo& ContextHandler2Helper::pushElementInfo( sal_Int32 nElement )
{
    mxContextStack->resize( mxContextStack->size() + 1 );
    ElementInfo& rInfo = mxContextStack->back();
    rInfo.mnElement = nElement;
    return rInfo;
}

void ContextHandler2Helper::popElementInfo()
{
    OSL_ENSURE( !mxContextStack->empty(), "ContextHandler2Helper::popElementInfo - context stack broken" );
    if( !mxContextStack->empty() )
        mxContextStack->pop_back();
}

void ContextHandler2Helper::processCollectedChars()
{
    OSL_ENSURE( !mxContextStack->empty(), "ContextHandler2Helper::processCollectedChars - no context info" );
    ElementInfo& rInfo = mxContextStack->back();
    if( rInfo.maChars.getLength() > 0 )
    {
        OUString aChars = rInfo.maChars.makeStringAndClear();
        if( mbEnableTrimSpace && rInfo.mbTrimSpaces )
            aChars = aChars.trim();
        if( aChars.getLength() > 0 )
            onCharacters( aChars );
    }
}

// ============================================================================

ContextHandler2::ContextHandler2( ContextHandler2Helper& rParent ) :
    ContextHandler( dynamic_cast< ContextHandler& >( rParent ) ),
    ContextHandler2Helper( rParent )
{
}

ContextHandler2::~ContextHandler2()
{
}

// com.sun.star.xml.sax.XFastContextHandler interface -------------------------

Reference< XFastContextHandler > SAL_CALL ContextHandler2::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException )
{
    return implCreateChildContext( nElement, rxAttribs );
}

void SAL_CALL ContextHandler2::startFastElement(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException )
{
    implStartElement( nElement, rxAttribs );
}

void SAL_CALL ContextHandler2::characters( const OUString& rChars ) throw( SAXException, RuntimeException )
{
    implCharacters( rChars );
}

void SAL_CALL ContextHandler2::endFastElement( sal_Int32 nElement ) throw( SAXException, RuntimeException )
{
    implEndElement( nElement );
}

// oox.core.RecordContext interface -------------------------------------------

ContextHandlerRef ContextHandler2::createRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    return implCreateRecordContext( nRecId, rStrm );
}

void ContextHandler2::startRecord( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    implStartRecord( nRecId, rStrm );
}

void ContextHandler2::endRecord( sal_Int32 nRecId )
{
    implEndRecord( nRecId );
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef ContextHandler2::onCreateContext( sal_Int32, const AttributeList& )
{
    return 0;
}

void ContextHandler2::onStartElement( const AttributeList& )
{
}

void ContextHandler2::onCharacters( const OUString& )
{
}

void ContextHandler2::onEndElement()
{
}

ContextHandlerRef ContextHandler2::onCreateRecordContext( sal_Int32, SequenceInputStream& )
{
    return 0;
}

void ContextHandler2::onStartRecord( SequenceInputStream& )
{
}

void ContextHandler2::onEndRecord()
{
}

// ============================================================================

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
