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

#include <oox/core/contexthandler2.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

namespace oox {
namespace core {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

/** Information about a processed element. */
struct ElementInfo
{
    OUStringBuffer      maChars;            /// Collected element characters.
    sal_Int32           mnElement;          /// The element identifier.
    bool                mbTrimSpaces;       /// True = trims leading/trailing spaces from text data.

    explicit     ElementInfo() : maChars( 0), mnElement( XML_TOKEN_INVALID ), mbTrimSpaces( false ) {}
};

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
    auto It = std::find_if(mxContextStack->rbegin(), mxContextStack->rend(),
        [](const ElementInfo& rItem) { return getNamespace(rItem.mnElement) != NMSP_mce; });
    if (It != mxContextStack->rend())
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
    OSL_ENSURE( getCurrentElementWithMce() == nRecId, "ContextHandler2Helper::implEndRecord - context stack broken" );
    if( !mxContextStack->empty() )
    {
        onEndRecord();
        popElementInfo();
    }
}

ElementInfo& ContextHandler2Helper::pushElementInfo( sal_Int32 nElement )
{
    mxContextStack->emplace_back();
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
    if (mxContextStack->empty())
        return;
    ElementInfo& rInfo = mxContextStack->back();
    if( !rInfo.maChars.isEmpty() )
    {
        OUString aChars = rInfo.maChars.makeStringAndClear();
        if( mbEnableTrimSpace && rInfo.mbTrimSpaces )
            aChars = aChars.trim();
        if( !aChars.isEmpty() )
            onCharacters( aChars );
    }
}

ContextHandler2::ContextHandler2( ContextHandler2Helper const & rParent ) :
    ContextHandler( dynamic_cast< ContextHandler const & >( rParent ) ),
    ContextHandler2Helper( rParent )
{
}

ContextHandler2::~ContextHandler2()
{
}

// com.sun.star.xml.sax.XFastContextHandler interface -------------------------

Reference< XFastContextHandler > SAL_CALL ContextHandler2::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
{
    return implCreateChildContext( nElement, rxAttribs );
}

void SAL_CALL ContextHandler2::startFastElement(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
{
    implStartElement( nElement, rxAttribs );
}

void SAL_CALL ContextHandler2::characters( const OUString& rChars )
{
    implCharacters( rChars );
}

void SAL_CALL ContextHandler2::endFastElement( sal_Int32 nElement )
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
    return nullptr;
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
    return nullptr;
}

void ContextHandler2::onStartRecord( SequenceInputStream& )
{
}

void ContextHandler2::onEndRecord()
{
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
