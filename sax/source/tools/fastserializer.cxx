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
#include <rtl/math.h>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>

#include <cassert>
#include <optional>
#include <string.h>
#include <string_view>
#include <utility>

#if OSL_DEBUG_LEVEL > 0
#include <iostream>
#include <set>
#endif

using ::com::sun::star::uno::Sequence;

static constexpr bool HAS_NAMESPACE(sal_Int32 x) { return (x & 0xffff0000) != 0; }
static constexpr sal_Int32 NAMESPACE(sal_Int32 x) { return x >> 16; }
static constexpr sal_Int32 TOKEN(sal_Int32 x) { return x & 0xffff; }

namespace sax_fastparser {
    FastSaxSerializer::FastSaxSerializer( const css::uno::Reference< css::io::XOutputStream >& xOutputStream )
        : mbMarkStackEmpty(true)
        , mpDoubleStr(nullptr)
        , mnDoubleStrCapacity(RTL_STR_MAX_VALUEOFDOUBLE)
        , mbXescape(true)
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
        write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
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

    void FastSaxSerializer::write( std::u16string_view sOutput, bool bEscape )
    {
        write( OUStringToOString(sOutput, RTL_TEXTENCODING_UTF8), bEscape );

    }

    void FastSaxSerializer::write( std::string_view sOutput, bool bEscape )
    {
        write( sOutput.data(), sOutput.length(), bEscape );
    }

    /** Characters not allowed in XML 1.0
        XML 1.1 would exclude only U+0000

        This assumes that `string` is UTF-8, but which appears to generally be the case:  The only
        user of this FastSaxSerializer code is FastSerializerHelper, and when its constructor
        (sax/source/tools/fshelper.cxx) is called with bWriteHeader being true, it calls
        FastSaxSerializer::startDocument, which writes sXmlHeader claiming encoding="UTF-8".  The
        only place that appears to construct FastSerializerHelper appears to be
        XmlFilterBase::openFragmentStreamWithSerializer (oox/source/core/xmlfilterbase.cxx), and it
        only passes false for bWriteHeader when the given rMediaType contains "vml" but not "+xml"
        (see <https://git.libreoffice.org/core/+/6a11add2c4ea975356cfb7bab02301788c79c904%5E!/>
        "XLSX VML Export fixes", stating "Don't write xml headers for vml files").  But let's assume
        that even such Vector Markup Language files are written as UTF-8.
     */
    template<typename Int> static std::optional<std::pair<unsigned, Int>> invalidChar(
        char const * string, Int length, Int index )
    {
        assert(index < length);
        auto const c = string[index];

        if (static_cast<unsigned char>(c) >= 0x20 && c != '\xEF')
            return {};

        switch (c)
        {
            case 0x09:
            case 0x0a:
            case 0x0d:
                return {};
            case '\xEF': // U+FFFE, U+FFFF:
                if (length - index >= 3 && string[index + 1] == '\xBF') {
                    switch (string[index + 2]) {
                    case '\xBE':
                        return std::pair(0xFFFE, 3);
                    case '\xBF':
                        return std::pair(0xFFFF, 3);
                    }
                }
                return {};
        }
        return std::pair(static_cast<unsigned char>(c), 1);
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

        bool bGood = true;
        const sal_Int32 kXescapeLen = 7;
        char bufXescape[kXescapeLen+1];
        sal_Int32 nNextXescape = 0;
        for (sal_Int32 i = 0; i < nLen;)
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
                            if (mbXescape)
                            {
                                char c1, c2, c3, c4;
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
                                        isHexDigit( c1 = pStr[i+2] ) &&
                                        isHexDigit( c2 = pStr[i+3] ) &&
                                        isHexDigit( c3 = pStr[i+4] ) &&
                                        isHexDigit( c4 = pStr[i+5] ))
                                {
                                    // OOXML has the odd habit to write some
                                    // names using this that when re-saving
                                    // should *not* be escaped, specifically
                                    // _x0020_ for blanks in w:xpath values.
                                    if (!(c1 == '0' && c2 == '0' && c3 == '2' && c4 == '0'))
                                    {
                                        // When encountering "_x005F_xHHHH_"
                                        // assume that is an already escaped
                                        // sequence that was not unescaped and
                                        // shall be written as is, to not end
                                        // up with "_x005F_x005F_xHHHH_" and
                                        // repeated...
                                        if (c1 == '0' && c2 == '0' && c3 == '5' && (c4 | 0x20) == 'f' &&
                                                i + kXescapeLen <= nLen - 6 &&
                                                pStr[i+kXescapeLen+5] == '_' &&
                                                ((pStr[i+kXescapeLen+0] | 0x20) == 'x') &&
                                                isHexDigit( pStr[i+kXescapeLen+1] ) &&
                                                isHexDigit( pStr[i+kXescapeLen+2] ) &&
                                                isHexDigit( pStr[i+kXescapeLen+3] ) &&
                                                isHexDigit( pStr[i+kXescapeLen+4] ))
                                        {
                                            writeBytes( &c, 1 );
                                            // Remember this fake escapement.
                                            nNextXescape = i + kXescapeLen + 6;
                                        }
                                        else
                                        {
                                            writeBytes( "_x005F_", kXescapeLen);
                                            // Remember this escapement so in
                                            // _xHHHH_xHHHH_ only the first '_'
                                            // is escaped.
                                            nNextXescape = i + kXescapeLen;
                                        }
                                        break;
                                    }
                                }
                                if (auto const inv = invalidChar(pStr, nLen, i))
                                {
                                    snprintf( bufXescape, kXescapeLen+1, "_x%04x_",
                                            inv->first);
                                    writeBytes( bufXescape, kXescapeLen);
                                    i += inv->second;
                                    continue;
                                }
                            }
#if OSL_DEBUG_LEVEL > 0
                            else
                            {
                                if (bGood && invalidChar(pStr, nLen, i))
                                {
                                    bGood = false;
                                    // The SAL_WARN() for the single character is
                                    // issued in writeBytes(), just gather for the
                                    // SAL_WARN_IF() below.
                                }
                            }
#endif
                            writeBytes( &c, 1 );
                break;
            }
            ++i;
        }
        SAL_WARN_IF( !bGood && nLen > 1, "sax", "in '" << OString(pStr,std::min<sal_Int32>(nLen,42)) << "'");
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
            assert(Namespace.hasElements());
            writeBytes(Namespace);
            write(":");
            auto const Element(mxFastTokenHandler->getUTF8Identifier(TOKEN(nElement)));
            assert(Element.hasElements());
            writeBytes(Element);
        } else {
            auto const Element(mxFastTokenHandler->getUTF8Identifier(nElement));
            assert(Element.hasElements());
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
            return std::string_view(
                     reinterpret_cast<char const*>(ns.getConstArray()), ns.getLength())
                 + ":"_ostr
                 + std::string_view(
                     reinterpret_cast<char const*>(name.getConstArray()), name.getLength());
        } else {
            Sequence<sal_Int8> const name(
                mxFastTokenHandler->getUTF8Identifier(nElement));
            return OString(reinterpret_cast<char const*>(name.getConstArray()), name.getLength());
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

        write("<");

        writeId(Element);
        if (pAttrList)
            writeFastAttributeList(*pAttrList);
        else
            writeTokenValueList();

        write(">");
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

        write("</");

        writeId(Element);

        write(">");
    }

    void FastSaxSerializer::singleFastElement( ::sal_Int32 Element, FastAttributeList const * pAttrList )
    {
        if ( !mbMarkStackEmpty )
        {
            maCachedOutputStream.flush();
            maMarkStack.top()->setCurrentElement( Element );
        }

        write("<");

        writeId(Element);
        if (pAttrList)
            writeFastAttributeList(*pAttrList);
        else
            writeTokenValueList();

        write("/>");
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
            write(" ");

            sal_Int32 nToken = rTokenValue.nToken;
            writeId(nToken);

#ifdef DBG_UTIL
            // Well-formedness constraint: Unique Att Spec
            OString const nameId(getId(nToken));
            assert(DebugAttributes.find(nameId) == DebugAttributes.end());
            DebugAttributes.insert(nameId);
#endif

            write("=\"");

            write(rTokenValue.pValue, true);

            write("\"");
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
            write(" ");

            sal_Int32 nToken = Tokens[j];
            writeId(nToken);

#ifdef DBG_UTIL
            // Well-formedness constraint: Unique Att Spec
            OString const nameId(getId(nToken));
            SAL_WARN_IF(DebugAttributes.find(nameId) != DebugAttributes.end(),  "sax", "Duplicate attribute: " << nameId );
            assert(DebugAttributes.find(nameId) == DebugAttributes.end());
            DebugAttributes.insert(nameId);
#endif

            write("=\"");

            const char* pAttributeValue = rAttrList.getFastAttributeValue(j);

            // tdf#117274 don't escape the special VML shape type id "#_x0000_t202"
            bool bEscape = !(pAttributeValue
                    && *pAttributeValue != '\0'
                    && (*pAttributeValue == '#'
                        ? strncmp(pAttributeValue, "#_x0000_t", 9) == 0
                        : strncmp(pAttributeValue, "_x0000_t", 8) == 0));

            write(pAttributeValue, rAttrList.AttributeValueLength(j), bEscape);

            write("\"");
        }
    }

    void FastSaxSerializer::mark(sal_Int32 const nTag, const Int32Sequence& rOrder)
    {
        if (rOrder.hasElements())
        {
            auto pSort = std::make_shared<ForSort>(nTag, rOrder);
            maMarkStack.push( pSort );
            maCachedOutputStream.setOutput( pSort );
        }
        else
        {
            auto pMerge = std::make_shared<ForMerge>(nTag);
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
            Int8Sequence aSeq( maMarkStack.top()->getData() );
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
            for (size_t i=0; i < nLen;)
            {
                if (auto const inv = invalidChar(pStr, nLen, i))
                {
                    bGood = false;
                    SAL_WARN("sax", "FastSaxSerializer::writeBytes - illegal XML character 0x" <<
                            std::hex << inv->first);
                    i += inv->second;
                    continue;
                }
                ++i;
            }
            SAL_WARN_IF( !bGood && nLen > 1, "sax", "in '" << OString(pStr,std::min<sal_Int32>(nLen,42)) << "'");
        }
#endif
        maCachedOutputStream.writeBytes( reinterpret_cast<const sal_Int8*>(pStr), nLen );
    }

    Int8Sequence& FastSaxSerializer::ForMerge::getData()
    {
        merge( maData, maPostponed.data(), maPostponed.size(), true );
        maPostponed.resize(0);

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
        for ( sal_Int32 i=0, len=maPostponed.size(); i < len; i++ )
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
        const sal_Int8* pData = rWhat.getConstArray();
        maPostponed.insert(maPostponed.end(), pData, pData + rWhat.getLength());
    }

    void FastSaxSerializer::ForMerge::merge(Int8Sequence &rTop, const sal_Int8* pMerge, sal_Int32 nMergeLen, bool bAppend)
    {
        if ( nMergeLen <= 0 )
            return;

        sal_Int32 nTopLen = rTop.getLength();

        rTop.realloc( nTopLen + nMergeLen );
        if ( bAppend )
        {
            // append the rMerge to the rTop
            memcpy( rTop.getArray() + nTopLen, pMerge, nMergeLen );
        }
        else
        {
            // prepend the rMerge to the rTop
            memmove( rTop.getArray() + nMergeLen, rTop.getConstArray(), nTopLen );
            memcpy( rTop.getArray(), pMerge, nMergeLen );
        }
    }

    void FastSaxSerializer::ForMerge::merge( Int8Sequence &rTop, const Int8Sequence &rMerge, bool bAppend )
    {
        merge(rTop, rMerge.getConstArray(), rMerge.getLength(), bAppend);
    }

    void FastSaxSerializer::ForMerge::resetData( )
    {
        maData = Int8Sequence();
    }

    void FastSaxSerializer::ForSort::setCurrentElement( sal_Int32 nElement )
    {
        const auto & rOrder = maOrder;
        if( std::find( rOrder.begin(), rOrder.end(), nElement ) != rOrder.end() )
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
        for (const auto nIndex : maOrder)
        {
            iter = maData.find( nIndex );
            if ( iter != maData.end() )
                ForMerge::append( iter->second );
        }
    }

    Int8Sequence& FastSaxSerializer::ForSort::getData()
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
