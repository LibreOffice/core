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

#include <string.h>

#include <cassert>
#include <set>
#include <stack>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/SAXInvalidCharacterException.hpp>
#include <com/sun/star/xml/sax/XWriter.hpp>

#include <com/sun/star/io/XActiveDataSource.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;

#include "xml2utf.hxx"
#include <memory>

#define LINEFEED 10
#define SEQUENCESIZE 1024
#define MAXCOLUMNCOUNT 72

/******
*
*
* Character conversion functions
*
*
*****/

namespace {

enum SaxInvalidCharacterError
{
    SAX_NONE,
    SAX_WARNING,
    SAX_ERROR
};

class SaxWriterHelper
{
#ifdef DBG_UTIL
public:
    ::std::stack<OUString> m_DebugStartedElements;
#endif

private:
    Reference< XOutputStream >  m_out;
    Sequence < sal_Int8 >       m_Sequence;
    sal_Int8*                   mp_Sequence;

    sal_Int32                   nLastLineFeedPos; // is negative after writing a sequence
    sal_uInt32                  nCurrentPos;
    bool                    m_bStartElementFinished;

    inline sal_uInt32 writeSequence() throw( SAXException );

    // use only if to insert the bytes more space in the sequence is needed and
    // so the sequence has to write out and reset rPos to 0
    // writes sequence only on overflow, sequence could be full on the end (rPos == SEQUENCESIZE)
    inline void AddBytes(sal_Int8* pTarget, sal_uInt32& rPos,
                const sal_Int8* pBytes, sal_uInt32 nBytesCount) throw( SAXException );
    inline bool convertToXML(const sal_Unicode * pStr,
                        sal_Int32 nStrLen,
                        bool bDoNormalization,
                        bool bNormalizeWhitespace,
                        sal_Int8 *pTarget,
                        sal_uInt32& rPos) throw( SAXException );
    inline void FinishStartElement() throw( SAXException );
public:
    explicit SaxWriterHelper(Reference< XOutputStream > m_TempOut)
        : m_out(m_TempOut)
        , m_Sequence(SEQUENCESIZE)
        , mp_Sequence(nullptr)
        , nLastLineFeedPos(0)
        , nCurrentPos(0)
        , m_bStartElementFinished(true)
    {
        OSL_ENSURE(SEQUENCESIZE > 50, "Sequence cache size to small");
        mp_Sequence = m_Sequence.getArray();
    }
    ~SaxWriterHelper()
    {
        OSL_ENSURE(!nCurrentPos, "cached Sequence not written");
        OSL_ENSURE(m_bStartElementFinished, "StartElement not complettly written");
    }

    inline void insertIndentation(sal_uInt32 m_nLevel)  throw( SAXException );

// returns whether it works correct or invalid characters were in the string
// If there are invalid characters in the string it returns sal_False.
// Than the calling method has to throw the needed Exception.
    inline bool writeString(const OUString& rWriteOutString,
                        bool bDoNormalization,
                        bool bNormalizeWhitespace) throw( SAXException );

    sal_uInt32 GetLastColumnCount() const throw()
        { return (sal_uInt32)(nCurrentPos - nLastLineFeedPos); }

    inline void startDocument() throw( SAXException );

// returns whether it works correct or invalid characters were in the strings
// If there are invalid characters in one of the strings it returns sal_False.
// Than the calling method has to throw the needed Exception.
    inline SaxInvalidCharacterError startElement(const OUString& rName, const Reference< XAttributeList >& xAttribs) throw( SAXException );
    inline bool FinishEmptyElement() throw( SAXException );

// returns whether it works correct or invalid characters were in the string
// If there are invalid characters in the string it returns sal_False.
// Than the calling method has to throw the needed Exception.
    inline bool endElement(const OUString& rName) throw( SAXException );
    inline void endDocument() throw( SAXException );

// returns whether it works correct or invalid characters were in the strings
// If there are invalid characters in the string it returns sal_False.
// Than the calling method has to throw the needed Exception.
    inline bool processingInstruction(const OUString& rTarget, const OUString& rData) throw( SAXException );
    inline void startCDATA() throw( SAXException );
    inline void endCDATA() throw( SAXException );

// returns whether it works correct or invalid characters were in the strings
// If there are invalid characters in the string it returns sal_False.
// Than the calling method has to throw the needed Exception.
    inline bool comment(const OUString& rComment) throw( SAXException );

    inline void clearBuffer() throw( SAXException );
};

const sal_Bool g_bValidCharsBelow32[32] =
{
//  0 1 2 3 4 5 6 7
    0,0,0,0,0,0,0,0,  //0
    0,1,1,0,0,1,0,0,  //8
    0,0,0,0,0,0,0,0,  //16
    0,0,0,0,0,0,0,0
};

inline bool IsInvalidChar(const sal_Unicode aChar)
{
    bool bRet(false);
    // check first for the most common characters
    if( aChar < 32 || aChar >= 0xd800 )
        bRet = ( (aChar < 32 && ! g_bValidCharsBelow32[aChar]) ||
            aChar == 0xffff ||
            aChar == 0xfffe );
    return bRet;
}

/********
* write through to the output stream
*
*****/
inline sal_uInt32 SaxWriterHelper::writeSequence() throw( SAXException )
{
    try
    {
        m_out->writeBytes( m_Sequence );
    }
    catch (const IOException & e)
    {
        Any a;
        a <<= e;
        throw SAXException(
            "IO exception during writing",
            Reference< XInterface > (),
            a );
    }
    nLastLineFeedPos -= SEQUENCESIZE;
    return 0;
}

inline void SaxWriterHelper::AddBytes(sal_Int8* pTarget, sal_uInt32& rPos,
                const sal_Int8* pBytes, sal_uInt32 nBytesCount) throw( SAXException )
{
    OSL_ENSURE((rPos + nBytesCount) > SEQUENCESIZE, "wrong use of AddBytesMethod");
    sal_uInt32 nCount(SEQUENCESIZE - rPos);
    memcpy( &(pTarget[rPos]) , pBytes,  nCount);

    OSL_ENSURE(rPos + nCount == SEQUENCESIZE, "the position should be the at the end");

    rPos = writeSequence();
    sal_uInt32 nRestCount(nBytesCount - nCount);
    if ((rPos + nRestCount) <= SEQUENCESIZE)
    {
        memcpy( &(pTarget[rPos]), &pBytes[nCount], nRestCount);
        rPos += nRestCount;
    }
    else
        AddBytes(pTarget, rPos, &pBytes[nCount], nRestCount);
}

/** Converts an UTF16 string to UTF8 and does XML normalization

    @param pTarget
           Pointer to a piece of memory, to where the output should be written. The caller
           must call calcXMLByteLength on the same string, to ensure,
           that there is enough memory for converting.
 */
inline bool SaxWriterHelper::convertToXML( const sal_Unicode * pStr,
                        sal_Int32 nStrLen,
                        bool bDoNormalization,
                        bool bNormalizeWhitespace,
                        sal_Int8 *pTarget,
                        sal_uInt32& rPos ) throw( SAXException )
{
    bool bRet(true);
    sal_uInt32 nSurrogate = 0;

    for( sal_Int32 i = 0 ; i < nStrLen ; i ++ )
    {
        sal_uInt16 c = pStr[i];
        if (IsInvalidChar(c))
            bRet = false;
        else if( (c >= 0x0001) && (c <= 0x007F) )
        {
            if( bDoNormalization )
            {
                switch( c )
                {
                    case '&':  // resemble to &amp;
                    {
                        if ((rPos + 5) > SEQUENCESIZE)
                            AddBytes(pTarget, rPos, reinterpret_cast<sal_Int8 const *>("&amp;"), 5);
                        else
                        {
                            memcpy( &(pTarget[rPos]) , "&amp;", 5 );
                            rPos += 5;
                        }
                    }
                    break;
                    case '<':
                    {
                        if ((rPos + 4) > SEQUENCESIZE)
                            AddBytes(pTarget, rPos, reinterpret_cast<sal_Int8 const *>("&lt;"), 4);
                        else
                        {
                            memcpy( &(pTarget[rPos]) , "&lt;" , 4 );
                            rPos += 4;        // &lt;
                        }
                    }
                    break;
                    case '>':
                    {
                        if ((rPos + 4) > SEQUENCESIZE)
                            AddBytes(pTarget, rPos, reinterpret_cast<sal_Int8 const *>("&gt;"), 4);
                        else
                        {
                            memcpy( &(pTarget[rPos]) , "&gt;" , 4 );
                            rPos += 4;        // &gt;
                        }
                    }
                    break;
                    case 39:                 // 39 == '''
                    {
                        if ((rPos + 6) > SEQUENCESIZE)
                            AddBytes(pTarget, rPos, reinterpret_cast<sal_Int8 const *>("&apos;"), 6);
                        else
                        {
                            memcpy( &(pTarget[rPos]) , "&apos;" , 6 );
                            rPos += 6;        // &apos;
                        }
                    }
                    break;
                    case '"':
                    {
                        if ((rPos + 6) > SEQUENCESIZE)
                            AddBytes(pTarget, rPos, reinterpret_cast<sal_Int8 const *>("&quot;"), 6);
                        else
                        {
                            memcpy( &(pTarget[rPos]) , "&quot;" , 6 );
                            rPos += 6;        // &quot;
                        }
                    }
                    break;
                    case 13:
                    {
                        if ((rPos + 6) > SEQUENCESIZE)
                            AddBytes(pTarget, rPos, reinterpret_cast<sal_Int8 const *>("&#x0d;"), 6);
                        else
                        {
                            memcpy( &(pTarget[rPos]) , "&#x0d;" , 6 );
                            rPos += 6;
                        }
                    }
                    break;
                    case LINEFEED:
                    {
                        if( bNormalizeWhitespace )
                        {
                            if ((rPos + 6) > SEQUENCESIZE)
                                AddBytes(pTarget, rPos, reinterpret_cast<sal_Int8 const *>("&#x0a;"), 6);
                            else
                            {
                                memcpy( &(pTarget[rPos]) , "&#x0a;" , 6 );
                                rPos += 6;
                            }
                        }
                        else
                        {
                            pTarget[rPos] = LINEFEED;
                            nLastLineFeedPos = rPos;
                            rPos ++;
                        }
                    }
                    break;
                    case 9:
                    {
                        if( bNormalizeWhitespace )
                        {
                            if ((rPos + 6) > SEQUENCESIZE)
                                AddBytes(pTarget, rPos, reinterpret_cast<sal_Int8 const *>("&#x09;"), 6);
                            else
                            {
                                memcpy( &(pTarget[rPos]) , "&#x09;" , 6 );
                                rPos += 6;
                            }
                        }
                        else
                        {
                            pTarget[rPos] = 9;
                            rPos ++;
                        }
                    }
                    break;
                    default:
                    {
                        pTarget[rPos] = (sal_Int8)c;
                        rPos ++;
                    }
                    break;
                }
            }
            else
            {
                pTarget[rPos] = (sal_Int8)c;
                if ((sal_Int8)c == LINEFEED)
                    nLastLineFeedPos = rPos;
                rPos ++;
            }
        }
        else if( c >= 0xd800 && c < 0xdc00  )
        {
            // 1. surrogate: save (until 2. surrogate)
            OSL_ENSURE( nSurrogate == 0, "left-over Unicode surrogate" );
            nSurrogate = ( ( c & 0x03ff ) + 0x0040 );
        }
        else if( c >= 0xdc00 && c < 0xe000 )
        {
            // 2. surrogate: write as UTF-8
            OSL_ENSURE( nSurrogate != 0, "lone 2nd Unicode surrogate" );

            nSurrogate = ( nSurrogate << 10 ) | ( c & 0x03ff );
            if( nSurrogate >= 0x00010000  &&  nSurrogate <= 0x0010FFFF )
            {
                sal_Int8 aBytes[] = { sal_Int8(0xF0 | ((nSurrogate >> 18) & 0x0F)),
                                      sal_Int8(0x80 | ((nSurrogate >> 12) & 0x3F)),
                                      sal_Int8(0x80 | ((nSurrogate >>  6) & 0x3F)),
                                      sal_Int8(0x80 | ((nSurrogate >>  0) & 0x3F)) };
                if ((rPos + 4) > SEQUENCESIZE)
                    AddBytes(pTarget, rPos, aBytes, 4);
                else
                {
                    pTarget[rPos] = aBytes[0];
                    rPos ++;
                    pTarget[rPos] = aBytes[1];
                    rPos ++;
                    pTarget[rPos] = aBytes[2];
                    rPos ++;
                    pTarget[rPos] = aBytes[3];
                    rPos ++;
                }
            }
            else
            {
                OSL_FAIL( "illegal Unicode character" );
                bRet = false;
            }

            // reset surrogate
            nSurrogate = 0;
        }
        else if( c > 0x07FF )
        {
            sal_Int8 aBytes[] = { sal_Int8(0xE0 | ((c >> 12) & 0x0F)),
                                  sal_Int8(0x80 | ((c >>  6) & 0x3F)),
                                  sal_Int8(0x80 | ((c >>  0) & 0x3F)) };
            if ((rPos + 3) > SEQUENCESIZE)
                AddBytes(pTarget, rPos, aBytes, 3);
            else
            {
                pTarget[rPos] = aBytes[0];
                rPos ++;
                pTarget[rPos] = aBytes[1];
                rPos ++;
                pTarget[rPos] = aBytes[2];
                rPos ++;
            }
        }
        else
        {
            sal_Int8 aBytes[] = { sal_Int8(0xC0 | ((c >>  6) & 0x1F)),
                                sal_Int8(0x80 | ((c >>  0) & 0x3F)) };
            if ((rPos + 2) > SEQUENCESIZE)
                AddBytes(pTarget, rPos, aBytes, 2);
            else
            {
                pTarget[rPos] = aBytes[0];
                rPos ++;
                pTarget[rPos] = aBytes[1];
                rPos ++;
            }
        }
        OSL_ENSURE(rPos <= SEQUENCESIZE, "not reset current position");
        if (rPos == SEQUENCESIZE)
            rPos = writeSequence();

        // reset left-over surrogate
        if( ( nSurrogate != 0 ) && !( c >= 0xd800 && c < 0xdc00 ) )
        {
            OSL_ENSURE( nSurrogate != 0, "left-over Unicode surrogate" );
            nSurrogate = 0;
            bRet = false;
        }
    }
    return bRet;
}

inline void SaxWriterHelper::FinishStartElement() throw( SAXException )
{
    if (!m_bStartElementFinished)
    {
        mp_Sequence[nCurrentPos] = '>';
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();
        m_bStartElementFinished = true;
    }
}

inline void SaxWriterHelper::insertIndentation(sal_uInt32 m_nLevel) throw( SAXException )
{
    FinishStartElement();
    if (m_nLevel > 0)
    {
        if ((nCurrentPos + m_nLevel + 1) <= SEQUENCESIZE)
        {
            mp_Sequence[nCurrentPos] = LINEFEED;
            nLastLineFeedPos = nCurrentPos;
            nCurrentPos++;
            memset( &(mp_Sequence[nCurrentPos]) , 32 , m_nLevel );
            nCurrentPos += m_nLevel;
            if (nCurrentPos == SEQUENCESIZE)
                nCurrentPos = writeSequence();
        }
        else
        {
            sal_uInt32 nCount(m_nLevel + 1);
            std::unique_ptr<sal_Int8[]> pBytes(new sal_Int8[nCount]);
            pBytes[0] = LINEFEED;
            memset( &(pBytes[1]), 32, m_nLevel );
            AddBytes(mp_Sequence, nCurrentPos, pBytes.get(), nCount);
            pBytes.reset();
            nLastLineFeedPos = nCurrentPos - nCount;
            if (nCurrentPos == SEQUENCESIZE)
                nCurrentPos = writeSequence();
        }
    }
    else
    {
        mp_Sequence[nCurrentPos] = LINEFEED;
        nLastLineFeedPos = nCurrentPos;
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();
    }
}

inline bool SaxWriterHelper::writeString( const OUString& rWriteOutString,
                        bool bDoNormalization,
                        bool bNormalizeWhitespace ) throw( SAXException )
{
    FinishStartElement();
    return convertToXML(rWriteOutString.getStr(),
                    rWriteOutString.getLength(),
                    bDoNormalization,
                    bNormalizeWhitespace,
                    mp_Sequence,
                    nCurrentPos);
}

inline void SaxWriterHelper::startDocument() throw( SAXException )
{
    const char pc[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    const int nLen = strlen( pc );
    if ((nCurrentPos + nLen) <= SEQUENCESIZE)
    {
        memcpy( mp_Sequence, pc , nLen );
        nCurrentPos += nLen;
    }
    else
    {
        AddBytes(mp_Sequence, nCurrentPos, reinterpret_cast<sal_Int8 const *>(pc), nLen);
    }
    OSL_ENSURE(nCurrentPos <= SEQUENCESIZE, "not reset current position");
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = LINEFEED;
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
}

inline SaxInvalidCharacterError SaxWriterHelper::startElement(const OUString& rName, const Reference< XAttributeList >& xAttribs) throw( SAXException )
{
    FinishStartElement();

#ifdef DBG_UTIL
    m_DebugStartedElements.push(rName);
    ::std::set<OUString> DebugAttributes;
#endif

    mp_Sequence[nCurrentPos] = '<';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    SaxInvalidCharacterError eRet(SAX_NONE);
    if (!writeString(rName, false, false))
        eRet = SAX_ERROR;

    sal_Int16 nAttribCount = xAttribs.is() ? static_cast<sal_Int16>(xAttribs->getLength()) : 0;
    for(sal_Int16 i = 0 ; i < nAttribCount ; i++ )
    {
        mp_Sequence[nCurrentPos] = ' ';
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();

        OUString const& rAttrName(xAttribs->getNameByIndex(i));
#ifdef DBG_UTIL
        // Well-formedness constraint: Unique Att Spec
        assert(DebugAttributes.find(rAttrName) == DebugAttributes.end());
        DebugAttributes.insert(rAttrName);
#endif
        if (!writeString(rAttrName, false, false))
            eRet = SAX_ERROR;

        mp_Sequence[nCurrentPos] = '=';
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();
        mp_Sequence[nCurrentPos] = '"';
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();

        if (!writeString(xAttribs->getValueByIndex( i ), true, true) &&
            !(eRet == SAX_ERROR))
            eRet = SAX_WARNING;

        mp_Sequence[nCurrentPos] = '"';
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();
    }

    m_bStartElementFinished = false;    // because the '>' character is not added,
                                            // because it is possible, that the "/>"
                                            // characters have to add
    return eRet;
}

inline bool SaxWriterHelper::FinishEmptyElement() throw( SAXException )
{
    if (m_bStartElementFinished)
        return false;

    mp_Sequence[nCurrentPos] = '/';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '>';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    m_bStartElementFinished = true;

    return true;
}

inline bool SaxWriterHelper::endElement(const OUString& rName) throw( SAXException )
{
    FinishStartElement();

    mp_Sequence[nCurrentPos] = '<';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '/';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    bool bRet(writeString( rName, false, false));

    mp_Sequence[nCurrentPos] = '>';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    return bRet;
}

inline void SaxWriterHelper::endDocument() throw( SAXException )
{
    if (nCurrentPos > 0)
    {
        m_Sequence.realloc(nCurrentPos);
        nCurrentPos = writeSequence();
        //m_Sequence.realloc(SEQUENCESIZE);
    }
}

inline void SaxWriterHelper::clearBuffer() throw( SAXException )
{
    FinishStartElement();
    if (nCurrentPos > 0)
    {
        m_Sequence.realloc(nCurrentPos);
        nCurrentPos = writeSequence();
        m_Sequence.realloc(SEQUENCESIZE);
        // Be sure to update the array pointer after the reallocation.
        mp_Sequence = m_Sequence.getArray();
    }
}

inline bool SaxWriterHelper::processingInstruction(const OUString& rTarget, const OUString& rData) throw( SAXException )
{
    FinishStartElement();
    mp_Sequence[nCurrentPos] = '<';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '?';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    bool bRet(writeString( rTarget, false, false ));

    mp_Sequence[nCurrentPos] = ' ';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    if (!writeString( rData, false, false ))
        bRet = false;

    mp_Sequence[nCurrentPos] = '?';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '>';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    return bRet;
}

inline void SaxWriterHelper::startCDATA() throw( SAXException )
{
    FinishStartElement();
    if ((nCurrentPos + 9) <= SEQUENCESIZE)
    {
        memcpy( &(mp_Sequence[nCurrentPos]), "<![CDATA[" , 9 );
        nCurrentPos += 9;
    }
    else
        AddBytes(mp_Sequence, nCurrentPos, reinterpret_cast<sal_Int8 const *>("<![CDATA["), 9);
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
}

inline void SaxWriterHelper::endCDATA() throw( SAXException )
{
    FinishStartElement();
    if ((nCurrentPos + 3) <= SEQUENCESIZE)
    {
        memcpy( &(mp_Sequence[nCurrentPos]), "]]>" , 3 );
        nCurrentPos += 3;
    }
    else
        AddBytes(mp_Sequence, nCurrentPos, reinterpret_cast<sal_Int8 const *>("]]>"), 3);
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
}

inline bool SaxWriterHelper::comment(const OUString& rComment) throw( SAXException )
{
    FinishStartElement();
    mp_Sequence[nCurrentPos] = '<';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '!';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '-';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '-';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    bool bRet(writeString( rComment, false, false));

    mp_Sequence[nCurrentPos] = '-';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '-';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '>';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    return bRet;
}

inline sal_Int32 calcXMLByteLength( const sal_Unicode *pStr, sal_Int32 nStrLen,
                                    bool bDoNormalization,
                                    bool bNormalizeWhitespace )
{
    sal_Int32 nOutputLength = 0;
    sal_uInt32 nSurrogate = 0;

    for( sal_Int32 i = 0 ; i < nStrLen ; i++ )
    {
        sal_uInt16 c = pStr[i];
        if( !IsInvalidChar(c) && (c >= 0x0001) && (c <= 0x007F) )
        {
            if( bDoNormalization )
            {
                switch( c )
                {
                case '&':       // resemble to &amp;
                    nOutputLength +=5;
                    break;
                case '<':       // &lt;
                case '>':       // &gt;
                    nOutputLength +=4;
                    break;
                case 39:        // 39 == ''', &apos;
                case '"':       // &quot;
                case 13:        // &#x0d;
                    nOutputLength += 6;
                    break;

                case 10:        // &#x0a;
                case 9:         // &#x09;
                    if( bNormalizeWhitespace )
                    {
                        nOutputLength += 6;
                    }
                    else
                    {
                        nOutputLength ++;
                    }
                    break;
                default:
                    nOutputLength ++;
                }
            }
            else
            {
                nOutputLength ++;
            }
        }
        else if( c >= 0xd800 && c < 0xdc00  )
        {
            // save surrogate
            nSurrogate = ( ( c & 0x03ff ) + 0x0040 );
        }
        else if( c >= 0xdc00 && c < 0xe000 )
        {
            // 2. surrogate: write as UTF-8 (if range is OK
            nSurrogate = ( nSurrogate << 10 ) | ( c & 0x03ff );
            if( nSurrogate >= 0x00010000  &&  nSurrogate <= 0x0010FFFF )
                nOutputLength += 4;
            nSurrogate = 0;
        }
        else if( c > 0x07FF )
        {
            nOutputLength += 3;
        }
        else
        {
            nOutputLength += 2;
        }

        // surrogate processing
        if( ( nSurrogate != 0 ) && !( c >= 0xd800 && c < 0xdc00 ) )
            nSurrogate = 0;
    }

    return nOutputLength;
}

/** returns position of first ascii 10 within the string, -1 when no 10 in string.
 */
static inline sal_Int32 getFirstLineBreak( const OUString & str ) throw ()
{
    const sal_Unicode *pSource = str.getStr();
    sal_Int32 nLen  = str.getLength();

    for( int n = 0; n < nLen ; n ++ )
    {
        if( LINEFEED == pSource[n] ) {
            return n;
        }
    }
    return -1;
}

class SAXWriter :
    public WeakImplHelper<
            XWriter,
            XServiceInfo >
{
public:
    SAXWriter()
        : m_pSaxWriterHelper(nullptr)
        , m_bDocStarted(false)
        , m_bIsCDATA(false)
        , m_bForceLineBreak(false)
        , m_bAllowLineBreak(false)
        , m_nLevel(0)
    {
    }
    virtual ~SAXWriter()
    {
        delete m_pSaxWriterHelper;
    }

public: // XActiveDataSource
    virtual void SAL_CALL setOutputStream(const Reference< XOutputStream > & aStream)
        throw (RuntimeException, std::exception) override
    {
        try
        {
            // temporary: set same stream again to clear buffer
            if ( m_out == aStream && m_pSaxWriterHelper && m_bDocStarted )
                m_pSaxWriterHelper->clearBuffer();
            else
            {
                m_out = aStream;
                delete m_pSaxWriterHelper;
                m_pSaxWriterHelper = new SaxWriterHelper(m_out);
                m_bDocStarted = false;
                m_nLevel = 0;
                m_bIsCDATA = false;
            }
        }
        catch (const SAXException& e)
        {
            throw css::lang::WrappedTargetRuntimeException(
                   e.Message,
                   static_cast < OWeakObject * > ( this ),
                   e.WrappedException);
        }
    }
    virtual Reference< XOutputStream >  SAL_CALL getOutputStream()
        throw(RuntimeException, std::exception) override
    {
        return m_out;
    }

public: // XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw(SAXException, RuntimeException, std::exception) override;

    virtual void SAL_CALL endDocument()
        throw(SAXException, RuntimeException, std::exception) override;

    virtual void SAL_CALL startElement(const OUString& aName,
                                       const Reference< XAttributeList > & xAttribs)
        throw (SAXException, RuntimeException, std::exception) override;

    virtual void SAL_CALL endElement(const OUString& aName)
        throw(SAXException, RuntimeException, std::exception) override;

    virtual void SAL_CALL characters(const OUString& aChars)
        throw(SAXException, RuntimeException, std::exception) override;

    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw(SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                const OUString& aData)
        throw(SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator(const Reference< XLocator > & xLocator)
        throw(SAXException, RuntimeException, std::exception) override;

public: // XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA() throw(SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL endCDATA() throw(SAXException,RuntimeException, std::exception) override;
    virtual void SAL_CALL comment(const OUString& sComment)
        throw(SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL unknown(const OUString& sString)
        throw(SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL allowLineBreak()
        throw(SAXException,RuntimeException, std::exception) override;

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw(std::exception) override;
    Sequence< OUString >         SAL_CALL getSupportedServiceNames() throw(std::exception) override;
    sal_Bool                    SAL_CALL supportsService(const OUString& ServiceName) throw(std::exception) override;

private:
    sal_Int32 getIndentPrefixLength( sal_Int32 nFirstLineBreakOccurrence ) throw();

    Reference< XOutputStream >  m_out;
    SaxWriterHelper*            m_pSaxWriterHelper;

    // Status information
    bool m_bDocStarted : 1;
    bool m_bIsCDATA : 1;
    bool m_bForceLineBreak : 1;
    bool m_bAllowLineBreak : 1;
    sal_Int32 m_nLevel;
};

sal_Int32 SAXWriter::getIndentPrefixLength( sal_Int32 nFirstLineBreakOccurrence ) throw()
{
    sal_Int32 nLength =-1;
    if (m_pSaxWriterHelper)
    {
        if ( m_bForceLineBreak ||
            (m_bAllowLineBreak &&
            ((nFirstLineBreakOccurrence + m_pSaxWriterHelper->GetLastColumnCount()) > MAXCOLUMNCOUNT)) )
            nLength = m_nLevel;
    }
    m_bForceLineBreak = false;
    m_bAllowLineBreak = false;
    return nLength;
}

static inline bool isFirstCharWhitespace( const sal_Unicode *p ) throw()
{
    return *p == ' ';
}

// XServiceInfo
OUString SAXWriter::getImplementationName() throw(std::exception)
{
    return OUString("com.sun.star.extensions.xml.sax.Writer");
}

// XServiceInfo
sal_Bool SAXWriter::supportsService(const OUString& ServiceName) throw(std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > SAXWriter::getSupportedServiceNames() throw (std::exception)
{
    Sequence<OUString> seq { "com.sun.star.xml.sax.Writer" };
    return seq;
}

void SAXWriter::startDocument()                     throw(SAXException, RuntimeException, std::exception )
{
    if( m_bDocStarted || ! m_out.is() || !m_pSaxWriterHelper ) {
        throw SAXException();
    }
    m_bDocStarted = true;
    m_pSaxWriterHelper->startDocument();
}


void SAXWriter::endDocument()                   throw(SAXException, RuntimeException, std::exception)
{
    if( ! m_bDocStarted )
    {
        throw SAXException(
            "endDocument called before startDocument",
            Reference< XInterface >() , Any() );
    }
    if( m_nLevel ) {
        throw SAXException(
            "unexpected end of document",
            Reference< XInterface >() , Any() );
    }
    m_pSaxWriterHelper->endDocument();
    try
    {
        m_out->closeOutput();
    }
    catch (const IOException & e)
    {
        Any a;
        a <<= e;
        throw SAXException(
            "IO exception during closing the IO Stream",
            Reference< XInterface > (),
            a );
    }
}


void SAXWriter::startElement(const OUString& aName, const Reference< XAttributeList >& xAttribs)
    throw(SAXException, RuntimeException, std::exception)
{
    if( ! m_bDocStarted )
    {
        SAXException except;
        except.Message = "startElement called before startDocument";
        throw except;
    }
    if( m_bIsCDATA )
    {
        SAXException except;
        except.Message =  "startElement call not allowed with CDATA sections";
        throw except;
    }

    sal_Int32 nLength(0);
    if (m_bAllowLineBreak)
    {
        sal_Int32 nAttribCount = xAttribs.is() ? xAttribs->getLength() : 0;

        nLength ++; // "<"
        nLength += calcXMLByteLength( aName.getStr() , aName.getLength(),
                                  false, false ); // the tag name

        sal_Int16 n;
        for( n = 0 ; n < static_cast<sal_Int16>(nAttribCount) ; n ++ ) {
            nLength ++; // " "
            OUString tmp =  xAttribs->getNameByIndex( n );

            nLength += calcXMLByteLength( tmp.getStr() , tmp.getLength() , false, false );

            nLength += 2; // ="

            tmp = xAttribs->getValueByIndex( n );

            nLength += calcXMLByteLength( tmp.getStr(), tmp.getLength(), true, true );

            nLength += 1; // "
        }

        nLength ++;  // '>'
    }

    // Is there a new indentation necesarry ?
    sal_Int32 nPrefix(getIndentPrefixLength( nLength ));

    // write into sequence
    if( nPrefix >= 0 )
        m_pSaxWriterHelper->insertIndentation( nPrefix );

    SaxInvalidCharacterError eRet(m_pSaxWriterHelper->startElement(aName, xAttribs));

    m_nLevel++;

    if (eRet == SAX_WARNING)
    {
        SAXInvalidCharacterException except;
        except.Message = "Invalid character during XML-Export in a attribute value";
        throw except;
    }
    else if (eRet == SAX_ERROR)
    {
        SAXException except;
        except.Message = "Invalid character during XML-Export";
        throw except;
    }
}

void SAXWriter::endElement(const OUString& aName)   throw (SAXException, RuntimeException, std::exception)
{
    if( ! m_bDocStarted ) {
        throw SAXException ();
    }
    m_nLevel --;

    if( m_nLevel < 0 ) {
        throw SAXException();
    }
    bool bRet(true);

    // check here because Helper's endElement is not always called
#ifdef DBG_UTIL
    assert(!m_pSaxWriterHelper->m_DebugStartedElements.empty());
    // Well-formedness constraint: Element Type Match
    assert(aName == m_pSaxWriterHelper->m_DebugStartedElements.top());
    m_pSaxWriterHelper->m_DebugStartedElements.pop();
#endif

    if( m_pSaxWriterHelper->FinishEmptyElement() )
        m_bForceLineBreak = false;
    else
    {
        // only ascii chars allowed
        sal_Int32 nLength(0);
        if (m_bAllowLineBreak)
            nLength = 3 + calcXMLByteLength( aName.getStr(), aName.getLength(), false, false );
        sal_Int32 nPrefix = getIndentPrefixLength( nLength );

        if( nPrefix >= 0 )
            m_pSaxWriterHelper->insertIndentation( nPrefix );

        bRet = m_pSaxWriterHelper->endElement(aName);
    }

    if (!bRet)
    {
        SAXException except;
        except.Message = "Invalid character during XML-Export";
        throw except;
    }
}

void SAXWriter::characters(const OUString& aChars)  throw(SAXException, RuntimeException, std::exception)
{
    if( ! m_bDocStarted )
    {
        SAXException except;
        except.Message = "characters method called before startDocument";
        throw except;
    }

    bool bThrowException(false);
    if( !aChars.isEmpty() )
    {
        if( m_bIsCDATA )
            bThrowException = !m_pSaxWriterHelper->writeString( aChars, false, false );
        else
        {
            // Note : nFirstLineBreakOccurrence is not exact, because we don't know, how
            //        many 2 and 3 byte chars are inbetween. However this whole stuff
            //        is eitherway for pretty printing only, so it does not need to be exact.
            sal_Int32 nLength(0);
            sal_Int32 nIndentPrefix(-1);
            if (m_bAllowLineBreak)
            {
                sal_Int32 nFirstLineBreakOccurrence = getFirstLineBreak( aChars );

                nLength = calcXMLByteLength( aChars.getStr(), aChars.getLength(),
                                               ! m_bIsCDATA , false );
                nIndentPrefix = getIndentPrefixLength(
                    nFirstLineBreakOccurrence >= 0 ? nFirstLineBreakOccurrence : nLength );
            }
            else
                nIndentPrefix = getIndentPrefixLength(nLength);

            // insert indentation
            if( nIndentPrefix >= 0 )
            {
                if( isFirstCharWhitespace( aChars.getStr() ) )
                    m_pSaxWriterHelper->insertIndentation( nIndentPrefix - 1 );
                else
                    m_pSaxWriterHelper->insertIndentation( nIndentPrefix );
            }
            bThrowException = !m_pSaxWriterHelper->writeString(aChars, true , false);
        }
    }
    if (bThrowException)
    {
        SAXInvalidCharacterException except;
        except.Message = "Invalid character during XML-Export";
        throw except;
    }
}


void SAXWriter::ignorableWhitespace(const OUString&) throw(SAXException, RuntimeException, std::exception)
{
    if( ! m_bDocStarted )
    {
        throw SAXException ();
    }

    m_bForceLineBreak = true;
}

void SAXWriter::processingInstruction(const OUString& aTarget, const OUString& aData)
    throw (SAXException, RuntimeException, std::exception)
{
    if( ! m_bDocStarted || m_bIsCDATA )
    {
        throw SAXException();
    }

    sal_Int32 nLength(0);
    if (m_bAllowLineBreak)
    {
        nLength = 2;  // "<?"
        nLength += calcXMLByteLength( aTarget.getStr(), aTarget.getLength(), false, false );

        nLength += 1;  // " "

        nLength += calcXMLByteLength( aData.getStr(), aData.getLength(), false, false );

        nLength += 2; // "?>"
    }

    sal_Int32 nPrefix = getIndentPrefixLength( nLength );

    if( nPrefix >= 0 )
        m_pSaxWriterHelper->insertIndentation( nPrefix );

    if (!m_pSaxWriterHelper->processingInstruction(aTarget, aData))
    {
        SAXException except;
        except.Message = "Invalid character during XML-Export";
        throw except;
    }
}


void SAXWriter::setDocumentLocator(const Reference< XLocator >&)
        throw (SAXException, RuntimeException, std::exception)
{

}

void SAXWriter::startCDATA() throw(SAXException, RuntimeException, std::exception)
{
    if( ! m_bDocStarted || m_bIsCDATA)
    {
        throw SAXException ();
    }

    sal_Int32 nLength = 9;
    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
        m_pSaxWriterHelper->insertIndentation( nPrefix );

    m_pSaxWriterHelper->startCDATA();

    m_bIsCDATA = true;
}

void SAXWriter::endCDATA() throw (SAXException,RuntimeException, std::exception)
{
    if( ! m_bDocStarted || ! m_bIsCDATA)
    {
        SAXException except;
        except.Message = "endCDATA was called without startCDATA";
        throw except;
    }

    sal_Int32 nLength = 3;
    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
        m_pSaxWriterHelper->insertIndentation( nPrefix );

    m_pSaxWriterHelper->endCDATA();

    m_bIsCDATA = false;
}


void SAXWriter::comment(const OUString& sComment) throw(SAXException, RuntimeException, std::exception)
{
    if( ! m_bDocStarted || m_bIsCDATA )
    {
        throw SAXException();
    }

    sal_Int32 nLength(0);
    if (m_bAllowLineBreak)
    {
        nLength = 4; // "<!--"
        nLength += calcXMLByteLength( sComment.getStr(), sComment.getLength(), false, false);

        nLength += 3;
    }

    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
        m_pSaxWriterHelper->insertIndentation( nPrefix );

    if (!m_pSaxWriterHelper->comment(sComment))
    {
        SAXException except;
        except.Message = "Invalid character during XML-Export";
        throw except;
    }
}


void SAXWriter::allowLineBreak( )   throw ( SAXException , RuntimeException, std::exception)
{
    if( ! m_bDocStarted || m_bAllowLineBreak ) {
        throw SAXException();
    }

     m_bAllowLineBreak = true;
}

void SAXWriter::unknown(const OUString& sString) throw (SAXException, RuntimeException, std::exception)
{

    if( ! m_bDocStarted )
    {
        throw SAXException ();
    }
    if( m_bIsCDATA )
    {
        throw SAXException();
    }

    if( sString.startsWith( "<?xml" ) )
        return;

    sal_Int32 nLength(0);
    if (m_bAllowLineBreak)
        nLength = calcXMLByteLength( sString.getStr(), sString.getLength(), false, false );

    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
        m_pSaxWriterHelper->insertIndentation( nPrefix );

    if (!m_pSaxWriterHelper->writeString( sString, false, false))
    {
        SAXException except;
        except.Message = "Invalid character during XML-Export";
        throw except;
    }
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_extensions_xml_sax_Writer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SAXWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
