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

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::xml::sax::XFastContextHandler;

namespace oox {
namespace core {

// ============================================================================

/** Information about a processed context element. */
struct ContextInfo
{
    OUStringBuffer      maCurrChars;        /// Collected characters from context.
    OUStringBuffer      maFinalChars;       /// Finalized (stipped) characters.
    sal_Int32           mnElement;          /// The element identifier.
    bool                mbTrimSpaces;       /// True = trims leading/trailing spaces from text data.

    explicit            ContextInfo();
};

ContextInfo::ContextInfo() :
    mnElement( XML_TOKEN_INVALID ),
    mbTrimSpaces( false )
{
}

// ============================================================================

ContextHandler2Helper::ContextHandler2Helper( bool bEnableTrimSpace ) :
    mxContextStack( new ContextStack ),
    mnRootStackSize( 0 ),
    mbEnableTrimSpace( bEnableTrimSpace )
{
    pushContextInfo( XML_ROOT_CONTEXT );
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

sal_Int32 ContextHandler2Helper::getCurrentElement() const
{
    return mxContextStack->empty() ? XML_ROOT_CONTEXT : mxContextStack->back().mnElement;
}

sal_Int32 ContextHandler2Helper::getPreviousElement( sal_Int32 nCountBack ) const
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

Reference< XFastContextHandler > ContextHandler2Helper::implCreateChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
{
    appendCollectedChars();
    ContextHandlerRef xContext = onCreateContext( nElement, AttributeList( rxAttribs ) );
    return Reference< XFastContextHandler >( xContext.get() );
}

void ContextHandler2Helper::implStartCurrentContext( sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
{
    AttributeList aAttribs( rxAttribs );
    pushContextInfo( nElement ).mbTrimSpaces = aAttribs.getToken( NMSP_XML | XML_space, XML_TOKEN_INVALID ) != XML_preserve;
    onStartElement( aAttribs );
}

void ContextHandler2Helper::implCharacters( const OUString& rChars )
{
    // #i76091# collect characters until context ends
    if( !mxContextStack->empty() )
        mxContextStack->back().maCurrChars.append( rChars );
}

void ContextHandler2Helper::implEndCurrentContext( sal_Int32 nElement )
{
    (void)nElement;     // prevent "unused parameter" warning in product build
    OSL_ENSURE( getCurrentElement() == nElement, "ContextHandler2Helper::implEndCurrentContext - context stack broken" );
    if( !mxContextStack->empty() )
    {
        // #i76091# process collected characters
        appendCollectedChars();
        // finalize the current context and pop context info from stack
        onEndElement( mxContextStack->back().maFinalChars.makeStringAndClear() );
        popContextInfo();
    }
}

ContextHandlerRef ContextHandler2Helper::implCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    return onCreateRecordContext( nRecId, rStrm );
}

void ContextHandler2Helper::implStartRecord( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    pushContextInfo( nRecId );
    onStartRecord( rStrm );
}

void ContextHandler2Helper::implEndRecord( sal_Int32 nRecId )
{
    (void)nRecId;   // prevent "unused parameter" warning in product build
    OSL_ENSURE( getCurrentElement() == nRecId, "ContextHandler2Helper::implEndRecord - context stack broken" );
    if( !mxContextStack->empty() )
    {
        // finalize the current context and pop context info from stack
        onEndRecord();
        popContextInfo();
    }
}

ContextInfo& ContextHandler2Helper::pushContextInfo( sal_Int32 nElement )
{
    mxContextStack->resize( mxContextStack->size() + 1 );
    ContextInfo& rInfo = mxContextStack->back();
    rInfo.mnElement = nElement;
    return rInfo;
}

void ContextHandler2Helper::popContextInfo()
{
    OSL_ENSURE( !mxContextStack->empty(), "ContextHandler2Helper::popContextInfo - context stack broken" );
    if( !mxContextStack->empty() )
        mxContextStack->pop_back();
}

void ContextHandler2Helper::appendCollectedChars()
{
    OSL_ENSURE( !mxContextStack->empty(), "ContextHandler2Helper::appendCollectedChars - no context info" );
    ContextInfo& rInfo = mxContextStack->back();
    if( rInfo.maCurrChars.getLength() > 0 )
    {
        OUString aChars = rInfo.maCurrChars.makeStringAndClear();
        rInfo.maFinalChars.append( (mbEnableTrimSpace && rInfo.mbTrimSpaces) ? aChars.trim() : aChars );
    }
}

// ============================================================================

ContextHandler2::ContextHandler2( ContextHandler2Helper& rParent ) :
    ContextHandler( rParent.queryContextHandler() ),
    ContextHandler2Helper( rParent )
{
}

ContextHandler2::~ContextHandler2()
{
}

ContextHandler& ContextHandler2::queryContextHandler()
{
    return *this;
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
    implStartCurrentContext( nElement, rxAttribs );
}

void SAL_CALL ContextHandler2::characters( const OUString& rChars ) throw( SAXException, RuntimeException )
{
    implCharacters( rChars );
}

void SAL_CALL ContextHandler2::endFastElement( sal_Int32 nElement ) throw( SAXException, RuntimeException )
{
    implEndCurrentContext( nElement );
}

// oox.core.RecordContext interface -------------------------------------------

ContextHandlerRef ContextHandler2::createRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    return implCreateRecordContext( nRecId, rStrm );
}

void ContextHandler2::startRecord( sal_Int32 nRecId, RecordInputStream& rStrm )
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

void ContextHandler2::onEndElement( const OUString& )
{
}

ContextHandlerRef ContextHandler2::onCreateRecordContext( sal_Int32, RecordInputStream& )
{
    return 0;
}

void ContextHandler2::onStartRecord( RecordInputStream& )
{
}

void ContextHandler2::onEndRecord()
{
}

// ============================================================================

} // namespace core
} // namespace oox

