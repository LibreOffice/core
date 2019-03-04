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

#include "fastserializer.hxx"

#include <com/sun/star/xml/sax/FastTokenHandler.hpp>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>

#include <string.h>

#if OSL_DEBUG_LEVEL > 0
#include <iostream>
#include <set>
#endif

using ::std::vector;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::io::XOutputStream;

#define HAS_NAMESPACE(x) ((x & 0xffff0000) != 0)
#define NAMESPACE(x) (x >> 16)
#define TOKEN(x) (x & 0xffff)
// number of characters without terminating 0
#define N_CHARS(string) (SAL_N_ELEMENTS(string) - 1)

static const char sClosingBracket[] = ">";
static const char sSlashAndClosingBracket[] = "/>";
static const char sColon[] = ":";
static const char sOpeningBracket[] = "<";
static const char sOpeningBracketAndSlash[] = "</";
static const char sQuote[] = "\"";
static const char sEqualSignAndQuote[] = "=\"";
static const char sSpace[] = " ";
static const char sXmlHeader[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";

namespace sax_fastparser {
    FastSaxSerializer::FastSaxSerializer( const css::uno::Reference< css::io::XOutputStream >& xOutputStream )
        : maCachedOutputStream()
        , maMarkStack()
        , mbMarkStackEmpty(true)
        , mpDoubleStr(nullptr)
        , mnDoubleStrCapacity(RTL_STR_MAX_VALUEOFDOUBLE)
    {
        rtl_string_new_WithLength(&mpDoubleStr, mnDoubleStrCapacity);
        mxFastTokenHandler = css::xml::sax::FastTokenHandler::create(
                ::comphelper::getProcessComponentContext());
        assert(xOutputStream.is()); // cannot do anything without that
        maCachedOutputStream.setOutputStream( xOutputStream );
    }

    FastSaxSerializer::~FastSaxSerializer()
    {
        rtl_string_release(mpDoubleStr);
    }

    void FastSaxSerializer::startDocument()
    {
        writeBytes(sXmlHeader, N_CHARS(sXmlHeader));
    }

    void FastSaxSerializer::write( double value )
    {
        rtl_math_doubleToString(
            &mpDoubleStr, &mnDoubleStrCapacity, 0, value, rtl_math_StringFormat_G,
            RTL_STR_MAX_VALUEOFDOUBLE - RTL_CONSTASCII_LENGTH("-x.E-xxx"), '.', nullptr,
            0, true);

        write(mpDoubleStr->buffer, mpDoubleStr->length);
        // and "clear" the string
        mpDoubleStr->length = 0;
        mnDoubleStrCapacity = RTL_STR_MAX_VALUEOFDOUBLE;
    }

    void FastSaxSerializer::write( const OUString& sOutput, bool bEscape )
    {
        write( OUStringToOString(sOutput, RTL_TEXTENCODING_UTF8), bEscape );

    }

    void FastSaxSerializer::write( const OString& sOutput, bool bEscape )
    {
        write( sOutput.getStr(), sOutput.getLength(), bEscape );
    }

    /** Characters not allowed in XML 1.0
        XML 1.1 would exclude only U+0000
     */
    static bool invalidChar( char c )
    {
        if (static_cast<unsigned char>(c) >= 0x20)
            return false;

        switch (c)
        {
            case 0x09:
            case 0x0a:
            case 0x0d:
                return false;
        }
        return true;
    }

    static bool isHexDigit( char c )
    {
        return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
    }

    void FastSaxSerializer::write( const char* pStr, sal_Int32 nLen, bool bEscape )
    {
        if (nLen == -1)
            nLen = pStr ? strlen(pStr) : 0;

        if (!bEscape)
        {
            writeBytes( pStr, nLen );
            return;
        }

        const sal_Int32 kXescapeLen = 7;
        char bufXescape[kXescapeLen+1];
        sal_Int32 nNextXescape = 0;
        for (sal_Int32 i = 0; i < nLen; ++i)
        {
            char c = pStr[ i ];
            switch( c )
            {
                case '<':   writeBytes( "&lt;", 4 );     break;
                case '>':   writeBytes( "&gt;", 4 );     break;
                case '&':   writeBytes( "&amp;", 5 );    break;
                case '\'':  writeBytes( "&apos;", 6 );   break;
                case '"':   writeBytes( "&quot;", 6 );   break;
                case '\t':
#if 0
                            // Seems OOXML prefers the _xHHHH_ escape over the
                            // entity in *some* cases, apparently in attribute
                            // values but not in element data.
                            // Would need to distinguish at a higher level.
                            if (mbXescape)
                            {
                                snprintf( bufXescape, kXescapeLen+1, "_x%04x_",
                                        static_cast<unsigned int>(static_cast<unsigned char>(c)));
                                writeBytes( bufXescape, kXescapeLen);
                            }
                            else
#endif
                            {
                                writeBytes( "&#9;", 4 );
                            }
                break;
                case '\n':
#if 0
                            if (mbXescape)
                            {
                                snprintf( bufXescape, kXescapeLen+1, "_x%04x_",
                                        static_cast<unsigned int>(static_cast<unsigned char>(c)));
                                writeBytes( bufXescape, kXescapeLen);
                            }
                            else
#endif
                            {
                                writeBytes( "&#10;", 5 );
                            }
                break;
                case '\r':
#if 0
                            if (mbXescape)
                            {
                                snprintf( bufXescape, kXescapeLen+1, "_x%04x_",
                                        static_cast<unsigned int>(static_cast<unsigned char>(c)));
                                writeBytes( bufXescape, kXescapeLen);
                            }
                            else
#endif
                            {
                                writeBytes( "&#13;", 5 );
                            }
                break;
                default:
                            {
                                // Escape characters not valid in XML 1.0 as
                                // _xHHHH_. A literal "_xHHHH_" has to be
                                // escaped as _x005F_xHHHH_ (effectively
                                // escaping the leading '_').
                                // See ECMA-376-1:2016 page 3736,
                                // 22.4.2.4 bstr (Basic String)
                                // for reference.
                                if (c == '_' && i >= nNextXescape && i <= nLen - kXescapeLen &&
                                        pStr[i+6] == '_' &&
                                        ((pStr[i+1] | 0x20) == 'x') &&
                                        isHexDigit( pStr[i+2] ) &&
                                        isHexDigit( pStr[i+3] ) &&
                                        isHexDigit( pStr[i+4] ) &&
                                        isHexDigit( pStr[i+5] ))
                                {
                                    // OOXML has the odd habit to write some
                                    // names using this that when re-saving
                                    // should *not* be escaped, specifically
                                    // _x0020_ for blanks in w:xpath values.
                                    if (strncmp( pStr+i+2, "0020", 4) != 0)
                                    {
                                        writeBytes( "_x005F_", kXescapeLen);
                                        // Remember this escapement so in
                                        // _xHHHH_xHHHH_ only the first '_' is
                                        // escaped.
                                        nNextXescape = i + kXescapeLen;
                                        break;
                                    }
                                }
                                if (invalidChar(c))
                                {
                                    snprintf( bufXescape, kXescapeLen+1, "_x%04x_",
                                            static_cast<unsigned int>(static_cast<unsigned char>(c)));
                                    writeBytes( bufXescape, kXescapeLen);
                                    break;
                                }
                                /* TODO: also U+FFFE and U+FFFF are not allowed
                                 * in XML 1.0, assuming we're writing UTF-8
                                 * those should be escaped as well to be
                                 * conformant. Likely that would involve
                                 * scanning for both encoded sequences and
                                 * write as _xHHHH_? */
                            }
                            writeBytes( &c, 1 );
                break;
            }
        }
    }

    void FastSaxSerializer::endDocument()
    {
        assert(mbMarkStackEmpty && maMarkStack.empty());
        maCachedOutputStream.flush();
    }

    void FastSaxSerializer::writeId( ::sal_Int32 nElement )
    {
        if( HAS_NAMESPACE( nElement ) ) {
            auto const Namespace(mxFastTokenHandler->getUTF8Identifier(NAMESPACE(nElement)));
            assert(Namespace.getLength() != 0);
            writeBytes(Namespace);
            writeBytes(sColon, N_CHARS(sColon));
            auto const Element(mxFastTokenHandler->getUTF8Identifier(TOKEN(nElement)));
            assert(Element.getLength() != 0);
            writeBytes(Element);
        } else {
            auto const Element(mxFastTokenHandler->getUTF8Identifier(nElement));
            assert(Element.getLength() != 0);
            writeBytes(Element);
        }
    }

#ifdef DBG_UTIL
    OString FastSaxSerializer::getId( ::sal_Int32 nElement )
    {
        if (HAS_NAMESPACE(nElement)) {
            Sequence<sal_Int8> const ns(
                mxFastTokenHandler->getUTF8Identifier(NAMESPACE(nElement)));
            Sequence<sal_Int8> const name(
                mxFastTokenHandler->getUTF8Identifier(TOKEN(nElement)));
            return OString(reinterpret_cast<sal_Char const*>(ns.getConstArray()), ns.getLength())
                 + OString(sColon, N_CHARS(sColon))
                 + OString(reinterpret_cast<sal_Char const*>(name.getConstArray()), name.getLength());
        } else {
            Sequence<sal_Int8> const name(
                mxFastTokenHandler->getUTF8Identifier(nElement));
            return OString(reinterpret_cast<sal_Char const*>(name.getConstArray()), name.getLength());
        }
    }
#endif

    void FastSaxSerializer::startFastElement( ::sal_Int32 Element, FastAttributeList const * pAttrList )
    {
        if ( !mbMarkStackEmpty )
        {
            maCachedOutputStream.flush();
            maMarkStack.top()->setCurrentElement( Element );
        }

#ifdef DBG_UTIL
        if (mbMarkStackEmpty)
            m_DebugStartedElements.push(Element);
        else
            maMarkStack.top()->m_DebugStartedElements.push_back(Element);
#endif

        writeBytes(sOpeningBracket, N_CHARS(sOpeningBracket));

        writeId(Element);
        if (pAttrList)
            writeFastAttributeList(*pAttrList);
        else
            writeTokenValueList();

        writeBytes(sClosingBracket, N_CHARS(sClosingBracket));
    }

    void FastSaxSerializer::endFastElement( ::sal_Int32 Element )
    {
#ifdef DBG_UTIL
        // Well-formedness constraint: Element Type Match
        if (mbMarkStackEmpty)
        {
            assert(!m_DebugStartedElements.empty());
            assert(Element == m_DebugStartedElements.top());
            m_DebugStartedElements.pop();
        }
        else
        {
            if (dynamic_cast<ForSort*>(maMarkStack.top().get()))
            {
                // Sort is always well-formed fragment
                assert(!maMarkStack.top()->m_DebugStartedElements.empty());
            }
            if (maMarkStack.top()->m_DebugStartedElements.empty())
            {
                maMarkStack.top()->m_DebugEndedElements.push_back(Element);
            }
            else
            {
                assert(Element == maMarkStack.top()->m_DebugStartedElements.back());
                maMarkStack.top()->m_DebugStartedElements.pop_back();
            }
        }
#endif

        writeBytes(sOpeningBracketAndSlash, N_CHARS(sOpeningBracketAndSlash));

        writeId(Element);

        writeBytes(sClosingBracket, N_CHARS(sClosingBracket));
    }

    void FastSaxSerializer::singleFastElement( ::sal_Int32 Element, FastAttributeList const * pAttrList )
    {
        if ( !mbMarkStackEmpty )
        {
            maCachedOutputStream.flush();
            maMarkStack.top()->setCurrentElement( Element );
        }

        writeBytes(sOpeningBracket, N_CHARS(sOpeningBracket));

        writeId(Element);
        if (pAttrList)
            writeFastAttributeList(*pAttrList);
        else
            writeTokenValueList();

        writeBytes(sSlashAndClosingBracket, N_CHARS(sSlashAndClosingBracket));
    }

    css::uno::Reference< css::io::XOutputStream > const & FastSaxSerializer::getOutputStream() const
    {
        return maCachedOutputStream.getOutputStream();
    }

    void FastSaxSerializer::writeTokenValueList()
    {
#ifdef DBG_UTIL
        ::std::set<OString> DebugAttributes;
#endif
        for (const TokenValue & rTokenValue : maTokenValues)
        {
            writeBytes(sSpace, N_CHARS(sSpace));

            sal_Int32 nToken = rTokenValue.nToken;
            writeId(nToken);

#ifdef DBG_UTIL
            // Well-formedness constraint: Unique Att Spec
            OString const nameId(getId(nToken));
            assert(DebugAttributes.find(nameId) == DebugAttributes.end());
            DebugAttributes.insert(nameId);
#endif

            writeBytes(sEqualSignAndQuote, N_CHARS(sEqualSignAndQuote));

            write(rTokenValue.pValue, -1, true);

            writeBytes(sQuote, N_CHARS(sQuote));
        }
        maTokenValues.clear();
    }

    void FastSaxSerializer::writeFastAttributeList(FastAttributeList const & rAttrList)
    {
#ifdef DBG_UTIL
        ::std::set<OString> DebugAttributes;
#endif
        const std::vector< sal_Int32 >& Tokens = rAttrList.getFastAttributeTokens();
        for (size_t j = 0; j < Tokens.size(); j++)
        {
            writeBytes(sSpace, N_CHARS(sSpace));

            sal_Int32 nToken = Tokens[j];
            writeId(nToken);

#ifdef DBG_UTIL
            // Well-formedness constraint: Unique Att Spec
            OString const nameId(getId(nToken));
            SAL_WARN_IF(DebugAttributes.find(nameId) != DebugAttributes.end(),  "sax", "Duplicate attribute: " << nameId );
            assert(DebugAttributes.find(nameId) == DebugAttributes.end());
            DebugAttributes.insert(nameId);
#endif

            writeBytes(sEqualSignAndQuote, N_CHARS(sEqualSignAndQuote));

            write(rAttrList.getFastAttributeValue(j), rAttrList.AttributeValueLength(j), true);

            writeBytes(sQuote, N_CHARS(sQuote));
        }
    }

    void FastSaxSerializer::mark(sal_Int32 const nTag, const Int32Sequence& rOrder)
    {
        if (rOrder.hasElements())
        {
            std::shared_ptr< ForMerge > pSort( new ForSort(nTag, rOrder) );
            maMarkStack.push( pSort );
            maCachedOutputStream.setOutput( pSort );
        }
        else
        {
            std::shared_ptr< ForMerge > pMerge( new ForMerge(nTag) );
            maMarkStack.push( pMerge );
            maCachedOutputStream.setOutput( pMerge );
        }
        mbMarkStackEmpty = false;
    }

#ifdef DBG_UTIL
    static void lcl_DebugMergeAppend(
            std::deque<sal_Int32> & rLeftEndedElements,
            std::deque<sal_Int32> & rLeftStartedElements,
            std::deque<sal_Int32> & rRightEndedElements,
            std::deque<sal_Int32> & rRightStartedElements)
    {
        while (!rRightEndedElements.empty())
        {
            if (rLeftStartedElements.empty())
            {
                rLeftEndedElements.push_back(rRightEndedElements.front());
            }
            else
            {
                assert(rLeftStartedElements.back() == rRightEndedElements.front());
                rLeftStartedElements.pop_back();
            }
            rRightEndedElements.pop_front();
        }
        while (!rRightStartedElements.empty())
        {
            rLeftStartedElements.push_back(rRightStartedElements.front());
            rRightStartedElements.pop_front();
        }
    }

    static void lcl_DebugMergePrepend(
            std::deque<sal_Int32> & rLeftEndedElements,
            std::deque<sal_Int32> & rLeftStartedElements,
            std::deque<sal_Int32> & rRightEndedElements,
            std::deque<sal_Int32> & rRightStartedElements)
    {
        while (!rLeftStartedElements.empty())
        {
            if (rRightEndedElements.empty())
            {
                rRightStartedElements.push_front(rLeftStartedElements.back());
            }
            else
            {
                assert(rRightEndedElements.front() == rLeftStartedElements.back());
                rRightEndedElements.pop_front();
            }
            rLeftStartedElements.pop_back();
        }
        while (!rLeftEndedElements.empty())
        {
            rRightEndedElements.push_front(rLeftEndedElements.back());
            rLeftEndedElements.pop_back();
        }
    }
#endif

    void FastSaxSerializer::mergeTopMarks(
        sal_Int32 const nTag, sax_fastparser::MergeMarks const eMergeType)
    {
        SAL_WARN_IF(mbMarkStackEmpty, "sax", "Empty mark stack - nothing to merge");
        assert(!mbMarkStackEmpty); // should never happen
        if ( mbMarkStackEmpty )
            return;

        assert(maMarkStack.top()->m_Tag == nTag && "mark/merge tag mismatch!");
        (void) nTag;
#ifdef DBG_UTIL
        if (dynamic_cast<ForSort*>(maMarkStack.top().get()))
        {
            // Sort is always well-formed fragment
            assert(maMarkStack.top()->m_DebugStartedElements.empty());
            assert(maMarkStack.top()->m_DebugEndedElements.empty());
        }
        lcl_DebugMergeAppend(
            maMarkStack.top()->m_DebugEndedElements,
            maMarkStack.top()->m_DebugStartedElements,
            maMarkStack.top()->m_DebugPostponedEndedElements,
            maMarkStack.top()->m_DebugPostponedStartedElements);
#endif

        // flush, so that we get everything in getData()
        maCachedOutputStream.flush();

        if (maMarkStack.size() == 1)
        {
#ifdef DBG_UTIL
            while (!maMarkStack.top()->m_DebugEndedElements.empty())
            {
                assert(maMarkStack.top()->m_DebugEndedElements.front() == m_DebugStartedElements.top());
                maMarkStack.top()->m_DebugEndedElements.pop_front();
                m_DebugStartedElements.pop();
            }
            while (!maMarkStack.top()->m_DebugStartedElements.empty())
            {
                m_DebugStartedElements.push(maMarkStack.top()->m_DebugStartedElements.front());
                maMarkStack.top()->m_DebugStartedElements.pop_front();
            }
#endif
            Sequence<sal_Int8> aSeq( maMarkStack.top()->getData() );
            maMarkStack.pop();
            mbMarkStackEmpty = true;
            maCachedOutputStream.resetOutputToStream();
            maCachedOutputStream.writeBytes( aSeq.getConstArray(), aSeq.getLength() );
            return;
        }

#ifdef DBG_UTIL
        ::std::deque<sal_Int32> topDebugStartedElements(maMarkStack.top()->m_DebugStartedElements);
        ::std::deque<sal_Int32> topDebugEndedElements(maMarkStack.top()->m_DebugEndedElements);
#endif
        const Int8Sequence aMerge( maMarkStack.top()->getData() );
        maMarkStack.pop();
#ifdef DBG_UTIL
        switch (eMergeType)
        {
            case MergeMarks::APPEND:
                lcl_DebugMergeAppend(
                    maMarkStack.top()->m_DebugEndedElements,
                    maMarkStack.top()->m_DebugStartedElements,
                    topDebugEndedElements,
                    topDebugStartedElements);
                break;
            case MergeMarks::PREPEND:
                if (dynamic_cast<ForSort*>(maMarkStack.top().get())) // argh...
                {
                    lcl_DebugMergeAppend(
                        maMarkStack.top()->m_DebugEndedElements,
                        maMarkStack.top()->m_DebugStartedElements,
                        topDebugEndedElements,
                        topDebugStartedElements);
                }
                else
                {
                    lcl_DebugMergePrepend(
                        topDebugEndedElements,
                        topDebugStartedElements,
                        maMarkStack.top()->m_DebugEndedElements,
                        maMarkStack.top()->m_DebugStartedElements);
                }
                break;
            case MergeMarks::POSTPONE:
                lcl_DebugMergeAppend(
                    maMarkStack.top()->m_DebugPostponedEndedElements,
                    maMarkStack.top()->m_DebugPostponedStartedElements,
                    topDebugEndedElements,
                    topDebugStartedElements);
                break;
        }
#endif
        if (maMarkStack.empty())
        {
            mbMarkStackEmpty = true;
            maCachedOutputStream.resetOutputToStream();
        }
        else
        {
            maCachedOutputStream.setOutput( maMarkStack.top() );
        }

        switch ( eMergeType )
        {
            case MergeMarks::APPEND:   maMarkStack.top()->append( aMerge );   break;
            case MergeMarks::PREPEND:  maMarkStack.top()->prepend( aMerge );  break;
            case MergeMarks::POSTPONE: maMarkStack.top()->postpone( aMerge ); break;
        }
    }

    void FastSaxSerializer::writeBytes( const Sequence< sal_Int8 >& rData )
    {
        maCachedOutputStream.writeBytes( rData.getConstArray(), rData.getLength() );
    }

    void FastSaxSerializer::writeBytes( const char* pStr, size_t nLen )
    {
#if OSL_DEBUG_LEVEL > 0
        {
            bool bGood = true;
            for (size_t i=0; i < nLen; ++i)
            {
                if (invalidChar(pStr[i]))
                {
                    bGood = false;
                    SAL_WARN("sax", "FastSaxSerializer::writeBytes - illegal XML character 0x" <<
                            std::hex << int(static_cast<unsigned char>(pStr[i])));
                }
            }
            SAL_WARN_IF( !bGood && nLen > 1, "sax", "in '" << OString(pStr,std::min<sal_Int32>(nLen,42)) << "'");
        }
#endif
        maCachedOutputStream.writeBytes( reinterpret_cast<const sal_Int8*>(pStr), nLen );
    }

    FastSaxSerializer::Int8Sequence& FastSaxSerializer::ForMerge::getData()
    {
        merge( maData, maPostponed, true );
        maPostponed.realloc( 0 );

        return maData;
    }

#if OSL_DEBUG_LEVEL > 0
    void FastSaxSerializer::ForMerge::print( )
    {
        std::cerr << "Data: ";
        for ( sal_Int32 i=0, len=maData.getLength(); i < len; i++ )
        {
            std::cerr << maData[i];
        }

        std::cerr << "\nPostponed: ";
        for ( sal_Int32 i=0, len=maPostponed.getLength(); i < len; i++ )
        {
            std::cerr << maPostponed[i];
        }

        std::cerr << "\n";
    }
#endif

    void FastSaxSerializer::ForMerge::prepend( const Int8Sequence &rWhat )
    {
        merge( maData, rWhat, false );
    }

    void FastSaxSerializer::ForMerge::append( const css::uno::Sequence<sal_Int8> &rWhat )
    {
        merge( maData, rWhat, true );
    }

    void FastSaxSerializer::ForMerge::postpone( const Int8Sequence &rWhat )
    {
        merge( maPostponed, rWhat, true );
    }

    void FastSaxSerializer::ForMerge::merge( Int8Sequence &rTop, const Int8Sequence &rMerge, bool bAppend )
    {
        sal_Int32 nMergeLen = rMerge.getLength();
        if ( nMergeLen > 0 )
        {
            sal_Int32 nTopLen = rTop.getLength();

            rTop.realloc( nTopLen + nMergeLen );
            if ( bAppend )
            {
                // append the rMerge to the rTop
                memcpy( rTop.getArray() + nTopLen, rMerge.getConstArray(), nMergeLen );
            }
            else
            {
                // prepend the rMerge to the rTop
                memmove( rTop.getArray() + nMergeLen, rTop.getConstArray(), nTopLen );
                memcpy( rTop.getArray(), rMerge.getConstArray(), nMergeLen );
            }
        }
    }

    void FastSaxSerializer::ForMerge::resetData( )
    {
        maData = Int8Sequence();
    }

    void FastSaxSerializer::ForSort::setCurrentElement( sal_Int32 nElement )
    {
        vector< sal_Int32 > aOrder( comphelper::sequenceToContainer<vector<sal_Int32> >(maOrder) );
        if( std::find( aOrder.begin(), aOrder.end(), nElement ) != aOrder.end() )
        {
            mnCurrentElement = nElement;
            if ( maData.find( nElement ) == maData.end() )
                maData[ nElement ] = Int8Sequence();
        }
    }

    void FastSaxSerializer::ForSort::prepend( const Int8Sequence &rWhat )
    {
        append( rWhat );
    }

    void FastSaxSerializer::ForSort::append( const css::uno::Sequence<sal_Int8> &rWhat )
    {
        merge( maData[mnCurrentElement], rWhat, true );
    }

    void FastSaxSerializer::ForSort::sort()
    {
        // Clear the ForMerge data to avoid duplicate items
        resetData();

        // Sort it all
        std::map< sal_Int32, Int8Sequence >::iterator iter;
        for ( sal_Int32 i=0, len=maOrder.getLength(); i < len; i++ )
        {
            iter = maData.find( maOrder[i] );
            if ( iter != maData.end() )
                ForMerge::append( iter->second );
        }
    }

    FastSaxSerializer::Int8Sequence& FastSaxSerializer::ForSort::getData()
    {
        sort( );
        return ForMerge::getData();
    }

#if OSL_DEBUG_LEVEL > 0
    void FastSaxSerializer::ForSort::print( )
    {
        for ( const auto& [rElement, rData] : maData )
        {
            std::cerr << "pair: " << rElement;
            for ( sal_Int32 i=0, len=rData.getLength(); i < len; ++i )
                std::cerr << rData[i];
            std::cerr << "\n";
        }

        sort( );
        ForMerge::print();
    }
#endif

} // namespace sax_fastparser

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
