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
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <rtl/ustrbuf.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace oox::core {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;

/** Information about a processed element. */
struct ElementInfo
{
    OUStringBuffer      maChars;            /// Collected element characters.
    sal_Int32           mnElement;          /// The element identifier.
    bool                mbTrimSpaces;       /// True = trims leading/trailing spaces from text data.

    explicit     ElementInfo() : maChars( 0), mnElement( XML_TOKEN_INVALID ), mbTrimSpaces( false ) {}
};

ContextHandler2Helper::ContextHandler2Helper( bool bEnableTrimSpace, XmlFilterBase& rFilter ) :
    mxContextStack( std::make_shared<ContextStack>() ),
    mnRootStackSize( 0 ),
    mbEnableTrimSpace( bEnableTrimSpace ),
    mrFilter( rFilter )
{
    pushElementInfo( XML_ROOT_CONTEXT );
}

ContextHandler2Helper::ContextHandler2Helper( const ContextHandler2Helper& rParent ) :
    mxContextStack( rParent.mxContextStack ),
    mnRootStackSize( rParent.mxContextStack->size() ),
    mbEnableTrimSpace( rParent.mbEnableTrimSpace ),
    mrFilter(rParent.mrFilter)
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
    if( (nCountBack < 0) || (mxContextStack->size() < o3tl::make_unsigned( nCountBack )) )
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
    return xContext;
}

void ContextHandler2Helper::implStartElement( sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
{
    AttributeList aAttribs( rxAttribs );
    pushElementInfo( nElement ).mbTrimSpaces = aAttribs.getToken( XML_TOKEN( space ), XML_TOKEN_INVALID ) != XML_preserve;
    onStartElement( aAttribs );
}

void ContextHandler2Helper::implCharacters( std::u16string_view rChars )
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
    if( getNamespace( nElement ) == NMSP_mce ) // TODO for checking 'Ignorable'
    {
        if( prepareMceContext( nElement, AttributeList( rxAttribs ) ) )
            return this;
        return nullptr;
    }

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

bool ContextHandler2Helper::prepareMceContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case MCE_TOKEN( AlternateContent ):
            addMCEState( MCE_STATE::Started );
            break;

        case MCE_TOKEN( Choice ):
            {
                if (isMCEStateEmpty() || getMCEState() != MCE_STATE::Started)
                    return false;

                OUString aRequires = rAttribs.getString( XML_Requires, u"none"_ustr );

                // At this point we can't access namespaces as the correct xml filter
                // is long gone. For now let's decide depending on a list of supported
                // namespaces like we do in writerfilter

                std::vector<OUString> aSupportedNS =
                {
                    u"a14"_ustr, // Impress needs this to import math formulas.
                    u"p14"_ustr,
                    u"p15"_ustr,
                    u"x12ac"_ustr,
                    u"v"_ustr
                };

                Reference<XServiceInfo> xModel(getDocFilter().getModel(), UNO_QUERY);
                if (xModel.is() && xModel->supportsService(u"com.sun.star.sheet.SpreadsheetDocument"_ustr))
                {
                    // No a14 for Calc documents, it would cause duplicated shapes as-is.
                    auto it = std::find(aSupportedNS.begin(), aSupportedNS.end(), "a14");
                    if (it != aSupportedNS.end())
                    {
                        aSupportedNS.erase(it);
                    }
                }

                if (std::find(aSupportedNS.begin(), aSupportedNS.end(), aRequires) != aSupportedNS.end())
                    setMCEState( MCE_STATE::FoundChoice ) ;
                else
                    return false;
            }
            break;

        case MCE_TOKEN( Fallback ):
            if( !isMCEStateEmpty() && getMCEState() == MCE_STATE::Started )
                break;
            return false;
        default:
            {
                OUString str = rAttribs.getStringDefaulted( MCE_TOKEN( Ignorable ));
                if( !str.isEmpty() )
                {
                    // Sequence< css::xml::FastAttribute > attrs = rAttribs.getFastAttributeList()->getFastAttributes();
                    // printf("MCE: %s\n", OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
                    // TODO: Check & Get the namespaces in "Ignorable"
                    // printf("NS: %d : %s\n", attrs.getLength(), OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }
            return false;
    }
    return true;
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

} // namespace oox::core

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
