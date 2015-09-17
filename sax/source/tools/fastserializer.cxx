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
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::xml::Attribute;
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
        , mpDoubleStr(NULL)
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
            RTL_STR_MAX_VALUEOFDOUBLE - RTL_CONSTASCII_LENGTH("-x.E-xxx"), '.', 0,
            0, sal_True);

        write(mpDoubleStr->buffer, mpDoubleStr->length);
        // and "clear" the string
        mpDoubleStr->length = 0;
        mnDoubleStrCapacity = RTL_STR_MAX_VALUEOFDOUBLE;
    }

    void FastSaxSerializer::write( const OUString& sOutput, bool bEscape )
    {
        const sal_Int32 nLength = sOutput.getLength();
        for (sal_Int32 i = 0; i < nLength; ++i)
        {
            const sal_Unicode cUnicode = sOutput[ i ];
            const char cChar = cUnicode;
            if (cUnicode & 0xff80)
            {
                write( OString(&cUnicode, 1, RTL_TEXTENCODING_UTF8) );
            }
            else if(bEscape) switch( cChar )
            {
                case '<':   writeBytes( "&lt;", 4 );     break;
                case '>':   writeBytes( "&gt;", 4 );     break;
                case '&':   writeBytes( "&amp;", 5 );    break;
                case '\'':  writeBytes( "&apos;", 6 );   break;
                case '"':   writeBytes( "&quot;", 6 );   break;
                case '\n':  writeBytes( "&#10;", 5 );    break;
                case '\r':  writeBytes( "&#13;", 5 );    break;
                default:    writeBytes( &cChar, 1 );     break;
            }
            else
                writeBytes( &cChar, 1 );
        }
    }

    void FastSaxSerializer::write( const OString& sOutput, bool bEscape )
    {
        write( sOutput.getStr(), sOutput.getLength(), bEscape );
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
                case '\n':  writeBytes( "&#10;", 5 );    break;
                case '\r':  writeBytes( "&#13;", 5 );    break;
                default:    writeBytes( &c, 1 );          break;
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
            writeBytes(mxFastTokenHandler->getUTF8Identifier(NAMESPACE(nElement)));
            writeBytes(sColon, N_CHARS(sColon));
            writeBytes(mxFastTokenHandler->getUTF8Identifier(TOKEN(nElement)));
        } else
            writeBytes(mxFastTokenHandler->getUTF8Identifier(nElement));
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

    void FastSaxSerializer::startFastElement( ::sal_Int32 Element, FastAttributeList* pAttrList )
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

    void FastSaxSerializer::singleFastElement( ::sal_Int32 Element, FastAttributeList* pAttrList )
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

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > FastSaxSerializer::getOutputStream()
    {
        return maCachedOutputStream.getOutputStream();
    }

    void FastSaxSerializer::writeTokenValueList()
    {
#ifdef DBG_UTIL
        ::std::set<OString> DebugAttributes;
#endif
        for (size_t j = 0; j < maTokenValues.size(); j++)
        {
            writeBytes(sSpace, N_CHARS(sSpace));

            sal_Int32 nToken = maTokenValues[j].nToken;
            writeId(nToken);

#ifdef DBG_UTIL
            // Well-formedness constraint: Unique Att Spec
            OString const nameId(getId(nToken));
            assert(DebugAttributes.find(nameId) == DebugAttributes.end());
            DebugAttributes.insert(nameId);
#endif

            writeBytes(sEqualSignAndQuote, N_CHARS(sEqualSignAndQuote));

            write(maTokenValues[j].pValue, -1, true);

            writeBytes(sQuote, N_CHARS(sQuote));
        }
        maTokenValues.clear();
    }

    void FastSaxSerializer::writeFastAttributeList(FastAttributeList& rAttrList)
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

        if (maMarkStack.size() == 1 && eMergeType != MergeMarks::IGNORE)
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
            case MergeMarks::IGNORE:
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
            case MergeMarks::IGNORE:   break;

        }
    }

    void FastSaxSerializer::writeBytes( const Sequence< sal_Int8 >& rData )
    {
        maCachedOutputStream.writeBytes( rData.getConstArray(), rData.getLength() );
    }

    void FastSaxSerializer::writeBytes( const char* pStr, size_t nLen )
    {
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

    void FastSaxSerializer::ForMerge::append( const Int8Sequence &rWhat )
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

    void FastSaxSerializer::ForSort::append( const Int8Sequence &rWhat )
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
        std::map< sal_Int32, Int8Sequence >::iterator iter = maData.begin();
        while ( iter != maData.end( ) )
        {
            std::cerr << "pair: " << iter->first;
            for ( sal_Int32 i=0, len=iter->second.getLength(); i < len; ++i )
                std::cerr << iter->second[i];
            std::cerr << "\n";
            ++iter;
        }

        sort( );
        ForMerge::print();
    }
#endif

} // namespace sax_fastparser

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
