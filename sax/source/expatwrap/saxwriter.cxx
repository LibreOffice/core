/*************************************************************************
 *
 *  $RCSfile: saxwriter.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:39:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <string.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

#ifndef _COM_SUN_STAR_XML_SAX_SAXINVALIDCHARACTEREXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXInvalidCharacterException.hpp>
#endif

#include <com/sun/star/io/XActiveDataSource.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase3.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/byteseq.hxx>
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace ::rtl;
using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;

#include "factory.hxx"
#include "xml2utf.hxx"

#define LINEFEED 10
#define SAXWRITER_CHECK_FOR_INVALID_CHARS
#define SEQUENCESIZE 1024
#define MAXCOLUMNCOUNT 72

/******
*
*
* Character conversion functions
*
*
*****/

namespace sax_expatwrap {
/*****
*
* Calculates the length of the sequence after conversion, but the conversion is not done.
* .g. &<>"' plus some more are
* special characters in XML that need to be transformed
*
* @param bConvertAll For Attributes it is necessary to convert every symbol (including line feed and tab)
*                    Set this to true, if you want to perform this special conversion
* @return The returned value is equal to the length of the incoming sequence, when no
+         conversion is necessary, otherwise it is larger than the length of the sequence.
****/
//  inline sal_Int32 CalcXMLLen( const Sequence<sal_Int8> & seq , sal_Bool bConvertAll ) throw()
//  {
//      sal_Int32 nLen = 0;
//      const sal_Int8 *pArray = seq.getConstArray();

//      for( int i = 0 ; i < seq.getLength() ; i ++ ) {

//          sal_Int8 c = pArray[i];
//          switch( c )
//          {
//          case '&':       // resemble to &amp;
//              nLen +=5;
//              break;
//          case '<':       // &lt;
//          case '>':       // &gt;
//              nLen +=4;
//              break;
//          case 39:        // 39 == ''', &apos;
//          case '"':       // &quot;
//          case 13:        // &#x0d;
//              nLen += 6;
//              break;

//          case 10:        // &#x0a;
//          case 9:         // &#x09;
//              if( bConvertAll )
//              {
//                  nLen += 6;        //
//              }
//              break;
//          default:
//              nLen ++;
//          }
//      }

//      return nLen;
//  }

enum SaxInvalidCharacterError
{
    SAX_NONE,
    SAX_WARNING,
    SAX_ERROR
};

class SaxWriterHelper
{
    Reference< XOutputStream >  m_out;
    Sequence < sal_Int8 >       m_Sequence;
    sal_Int8*                   mp_Sequence;

    sal_Int32                   nLastLineFeedPos; // is negative after writing a sequence
    sal_uInt32                  nCurrentPos;
    sal_Bool                    m_bStartElementFinished;


    inline sal_uInt32 writeSequence() throw( SAXException );

    // use only if to insert the bytes more space in the sequence is needed and
    // so the sequence has to write out and reset rPos to 0
    // writes sequence only on overflow, sequence could be full on the end (rPos == SEQUENCESIZE)
    inline void AddBytes(sal_Int8* pTarget, sal_uInt32& rPos,
                const sal_Int8* pBytes, sal_uInt32 nBytesCount) throw( SAXException );
    inline sal_Bool convertToXML(const sal_Unicode * pStr,
                        sal_Int32 nStrLen,
                        sal_Bool bDoNormalization,
                        sal_Bool bNormalizeWhitespace,
                        sal_Int8 *pTarget,
                        sal_uInt32& rPos) throw( SAXException );
    inline void FinishStartElement() throw( SAXException );
public:
    SaxWriterHelper(Reference< XOutputStream > m_TempOut) :
        m_Sequence(SEQUENCESIZE),
        m_out(m_TempOut),
        nCurrentPos(0),
        nLastLineFeedPos(0),
        mp_Sequence(NULL),
        m_bStartElementFinished(sal_True)
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
    inline sal_Bool writeString(const rtl::OUString& rWriteOutString,
                        sal_Bool bDoNormalization,
                        sal_Bool bNormalizeWhitespace) throw( SAXException );

    sal_uInt32 GetLastColumnCount() { return (sal_uInt32)(nCurrentPos - nLastLineFeedPos); }

    inline void startDocument() throw( SAXException );

// returns whether it works correct or invalid characters were in the strings
// If there are invalid characters in one of the strings it returns sal_False.
// Than the calling method has to throw the needed Exception.
    inline SaxInvalidCharacterError startElement(const rtl::OUString& rName, const Reference< XAttributeList >& xAttribs) throw( SAXException );
    inline sal_Bool FinishEmptyElement() throw( SAXException );

// returns whether it works correct or invalid characters were in the string
// If there are invalid characters in the string it returns sal_False.
// Than the calling method has to throw the needed Exception.
    inline sal_Bool endElement(const rtl::OUString& rName) throw( SAXException );
    inline void endDocument() throw( SAXException );

// returns whether it works correct or invalid characters were in the strings
// If there are invalid characters in the string it returns sal_False.
// Than the calling method has to throw the needed Exception.
    inline sal_Bool processingInstruction(const rtl::OUString& rTarget, const rtl::OUString& rData) throw( SAXException );
    inline void startCDATA() throw( SAXException );
    inline void endCDATA() throw( SAXException );

// returns whether it works correct or invalid characters were in the strings
// If there are invalid characters in the string it returns sal_False.
// Than the calling method has to throw the needed Exception.
    inline sal_Bool comment(const rtl::OUString& rComment) throw( SAXException );
};

const sal_Bool g_bValidCharsBelow32[31] =
{
//  0 1 2 3 4 5 6 7
    0,0,0,0,0,0,0,0,  //0
    0,1,1,0,0,1,0,0,  //8
    0,0,0,0,0,0,0,0,  //16
    0,0,0,0,0,0,0
};

inline sal_Bool IsInvalidChar(const sal_Unicode aChar)
{
    sal_Bool bRet(sal_False);
#ifdef SAXWRITER_CHECK_FOR_INVALID_CHARS
        // check first for the most common characters
    if( aChar < 32 || aChar >= 0xd800 )
        bRet = ( (aChar < 32 && ! g_bValidCharsBelow32[aChar]) ||
            (aChar >= 0xd800 && aChar <= 0xdfff) ||
            aChar == 0xffff ||
            aChar == 0xfffe );
#endif
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
    catch( IOException & e )
    {
        Any a;
        a <<= e;
        throw SAXException(
            OUString::createFromAscii( "io exception during writing" ),
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
inline sal_Bool SaxWriterHelper::convertToXML( const sal_Unicode * pStr,
                        sal_Int32 nStrLen,
                        sal_Bool bDoNormalization,
                        sal_Bool bNormalizeWhitespace,
                        sal_Int8 *pTarget,
                        sal_uInt32& rPos ) throw( SAXException )
{
    sal_Int32 nOutputLength(0);
    sal_Bool bRet(sal_True);

    for( sal_Int32 i = 0 ; i < nStrLen ; i ++ )
    {
        sal_uInt16 c = pStr[i];
        if (IsInvalidChar(c))
            bRet = sal_False;
        else if( (c >= 0x0001) && (c <= 0x007F) )
        {
            if( bDoNormalization )
            {
                switch( c )
                {
                    case '&':  // resemble to &amp;
                    {
                        if ((rPos + 5) > SEQUENCESIZE)
                            AddBytes(pTarget, rPos, (sal_Int8*)"&amp;", 5);
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
                            AddBytes(pTarget, rPos, (sal_Int8*)"&lt;", 4);
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
                            AddBytes(pTarget, rPos, (sal_Int8*)"&gt;", 4);
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
                            AddBytes(pTarget, rPos, (sal_Int8*)"&apos;", 6);
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
                            AddBytes(pTarget, rPos, (sal_Int8*)"&quot;", 6);
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
                            AddBytes(pTarget, rPos, (sal_Int8*)"&#x0d;", 6);
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
                                AddBytes(pTarget, rPos, (sal_Int8*)"&#x0a;" , 6);
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
                                AddBytes(pTarget, rPos, (sal_Int8*)"&#x09;" , 6);
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
        m_bStartElementFinished = sal_True;
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
            sal_Int8* pBytes = new sal_Int8[nCount];
            pBytes[0] = LINEFEED;
            memset( &(pBytes[1]), 32, m_nLevel );
            AddBytes(mp_Sequence, nCurrentPos, pBytes, nCount);
            delete[] pBytes;
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

inline sal_Bool SaxWriterHelper::writeString( const rtl::OUString& rWriteOutString,
                        sal_Bool bDoNormalization,
                        sal_Bool bNormalizeWhitespace ) throw( SAXException )
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
    if ((nCurrentPos + nLen + 1) <= SEQUENCESIZE)
    {
        memcpy( mp_Sequence, pc , nLen );
        nCurrentPos += nLen;
        mp_Sequence[nCurrentPos] = LINEFEED;
        nCurrentPos++;
    }
    else
    {
        AddBytes(mp_Sequence, nCurrentPos, (sal_Int8*)pc, nLen);
        OSL_ENSURE(nCurrentPos <= SEQUENCESIZE, "not reset current position");
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();
        mp_Sequence[nCurrentPos] = LINEFEED;
        nCurrentPos++;
    }
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
}

inline SaxInvalidCharacterError SaxWriterHelper::startElement(const rtl::OUString& rName, const Reference< XAttributeList >& xAttribs) throw( SAXException )
{
    FinishStartElement();
    mp_Sequence[nCurrentPos] = '<';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    SaxInvalidCharacterError eRet(SAX_NONE);
    if (!writeString(rName, sal_False, sal_False))
        eRet = SAX_ERROR;

    sal_Int8 nAttribCount = xAttribs.is() ? xAttribs->getLength() : 0;
    for(sal_Int8 i = 0 ; i < nAttribCount ; i++ )
    {
        mp_Sequence[nCurrentPos] = ' ';
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();

        if (!writeString(xAttribs->getNameByIndex( i ), sal_False, sal_False))
            eRet = SAX_ERROR;

        mp_Sequence[nCurrentPos] = '=';
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();
        mp_Sequence[nCurrentPos] = '"';
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();

        if (!writeString(xAttribs->getValueByIndex( i ), sal_True, sal_True) &&
            !(eRet == SAX_ERROR))
            eRet = SAX_WARNING;

        mp_Sequence[nCurrentPos] = '"';
        nCurrentPos++;
        if (nCurrentPos == SEQUENCESIZE)
            nCurrentPos = writeSequence();
    }

    m_bStartElementFinished = sal_False;    // because the '>' character is not added,
                                            // because it is possible, that the "/>"
                                            // characters have to add
    return eRet;
}

inline sal_Bool SaxWriterHelper::FinishEmptyElement() throw( SAXException )
{
    if (m_bStartElementFinished)
        return sal_False;

    mp_Sequence[nCurrentPos] = '/';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
    mp_Sequence[nCurrentPos] = '>';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    m_bStartElementFinished = sal_True;

    return sal_True;
}

inline sal_Bool SaxWriterHelper::endElement(const rtl::OUString& rName) throw( SAXException )
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

    sal_Bool bRet(writeString( rName, sal_False, sal_False));

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

inline sal_Bool SaxWriterHelper::processingInstruction(const rtl::OUString& rTarget, const rtl::OUString& rData) throw( SAXException )
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

    sal_Bool bRet(writeString( rTarget, sal_False, sal_False ));

    mp_Sequence[nCurrentPos] = ' ';
    nCurrentPos++;
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();

    if (!writeString( rData, sal_False, sal_False ))
        bRet = sal_False;

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
        AddBytes(mp_Sequence, nCurrentPos, (sal_Int8*)"<![CDATA[" , 9);
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
        AddBytes(mp_Sequence, nCurrentPos, (sal_Int8*)"]]>" , 3);
    if (nCurrentPos == SEQUENCESIZE)
        nCurrentPos = writeSequence();
}

inline sal_Bool SaxWriterHelper::comment(const rtl::OUString& rComment) throw( SAXException )
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

    sal_Bool bRet(writeString( rComment, sal_False, sal_False));

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
                                    sal_Bool bDoNormalization,
                                    sal_Bool bNormalizeWhitespace )
{
    sal_Int32 nOutputLength = 0;

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
                        nOutputLength += 6;       //
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
        else if( c > 0x07FF )
        {
            nOutputLength += 3;
        }
        else
        {
            nOutputLength += 2;
        }
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

/** returns position of last ascii 10 within sequence, -1 when no 10 in string.
 */
static inline sal_Int32 getLastLineBreak( const Sequence<sal_Int8>  & seq) throw ()
{
    const sal_Int8 *pSource = seq.getConstArray();
    sal_Int32 nLen  = seq.getLength();

    for( int n = nLen-1; n >= 0 ; n -- )
    {
        if( LINEFEED == pSource[n] ) {
            return n;
        }
    }
    return -1;
}


class SAXWriter :
    public WeakImplHelper3<
            XActiveDataSource,
            XExtendedDocumentHandler,
              XServiceInfo >
{
public:
    SAXWriter( ) :
        m_bForceLineBreak(sal_False),
        m_bAllowLineBreak(sal_False),
        m_seqStartElement(),
        mp_SaxWriterHelper( NULL )
        {}
    ~SAXWriter()
    {
        delete mp_SaxWriterHelper;
    }

public: // XActiveDataSource
    virtual void SAL_CALL setOutputStream(const Reference< XOutputStream > & aStream)
        throw (RuntimeException)
            {
                m_out = aStream;
                delete mp_SaxWriterHelper;
                mp_SaxWriterHelper = new SaxWriterHelper(m_out);
                m_bDocStarted = sal_False;
                m_nLevel = 0;
                m_bIsCDATA = sal_False;
            }
    virtual Reference< XOutputStream >  SAL_CALL getOutputStream(void)
        throw(RuntimeException)
            { return m_out; }

public: // XDocumentHandler
    virtual void SAL_CALL startDocument(void)
        throw(SAXException, RuntimeException);

    virtual void SAL_CALL endDocument(void)
        throw(SAXException, RuntimeException);

    virtual void SAL_CALL startElement(const OUString& aName,
                                       const Reference< XAttributeList > & xAttribs)
        throw (SAXException, RuntimeException);

    virtual void SAL_CALL endElement(const OUString& aName)
        throw(SAXException, RuntimeException);

    virtual void SAL_CALL characters(const OUString& aChars)
        throw(SAXException, RuntimeException);

    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw(SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                const OUString& aData)
        throw(SAXException, RuntimeException);
    virtual void SAL_CALL setDocumentLocator(const Reference< XLocator > & xLocator)
        throw(SAXException, RuntimeException);

public: // XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA(void) throw(SAXException, RuntimeException);
    virtual void SAL_CALL endCDATA(void) throw(RuntimeException);
    virtual void SAL_CALL comment(const OUString& sComment)
        throw(SAXException, RuntimeException);
    virtual void SAL_CALL unknown(const OUString& sString)
        throw(SAXException, RuntimeException);
    virtual void SAL_CALL allowLineBreak(void)
        throw(SAXException,RuntimeException);

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw();
    Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw();
    sal_Bool                    SAL_CALL supportsService(const OUString& ServiceName) throw();

private:

    void writeSequence( const Sequence<sal_Int8> & seq );
    sal_Int32 getIndentPrefixLength( sal_Int32 nFirstLineBreakOccurence ) throw();

    Reference< XOutputStream >  m_out;
    Sequence < sal_Int8 >       m_seqStartElement;
    SaxWriterHelper*            mp_SaxWriterHelper;

    // Status information
    sal_Bool m_bDocStarted : 1;
    sal_Bool m_bIsCDATA : 1;
    sal_Bool m_bForceLineBreak : 1;
    sal_Bool m_bAllowLineBreak : 1;
    sal_Int32 m_nLevel;
};


//--------------------------------------
// the extern interface
//---------------------------------------
Reference < XInterface > SAL_CALL SaxWriter_CreateInstance(
    const Reference < XMultiServiceFactory >  & rSMgr )
    throw (Exception)
{
    SAXWriter *p = new SAXWriter;
    return Reference< XInterface > ( SAL_STATIC_CAST(OWeakObject *, p ) );
}

OUString SaxWriter_getServiceName() throw()
{
    return OUString::createFromAscii( "com.sun.star.xml.sax.Writer" );
}

OUString SaxWriter_getImplementationName() throw()
{
    return OUString::createFromAscii( "com.sun.star.extensions.xml.sax.Writer" );
}

Sequence< OUString >    SaxWriter_getSupportedServiceNames(void) throw()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = SaxWriter_getServiceName();
    return aRet;
}


sal_Int32 SAXWriter::getIndentPrefixLength( sal_Int32 nFirstLineBreakOccurence ) throw()
{
    sal_Int32 nLength =-1;
    if (mp_SaxWriterHelper)
    {
        if( m_bForceLineBreak ||
            m_bAllowLineBreak &&
            ((nFirstLineBreakOccurence + mp_SaxWriterHelper->GetLastColumnCount()) > MAXCOLUMNCOUNT) )
            nLength = m_nLevel;
    }
    m_bForceLineBreak = sal_False;
    m_bAllowLineBreak = sal_False;
    return nLength;
}

static inline sal_Bool isFirstCharWhitespace( const sal_Unicode *p ) throw()
{
    return *p == ' ';
}


// XServiceInfo
OUString SAXWriter::getImplementationName() throw()
{
    return SaxWriter_getImplementationName();
}

// XServiceInfo
sal_Bool SAXWriter::supportsService(const OUString& ServiceName) throw()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > SAXWriter::getSupportedServiceNames(void) throw ()
{
    Sequence<OUString> seq(1);
    seq.getArray()[0] = SaxWriter_getServiceName();
    return seq;
}



void SAXWriter::startDocument()                     throw(SAXException, RuntimeException )
{
    if( m_bDocStarted || ! m_out.is() || !mp_SaxWriterHelper ) {
        throw SAXException();
    }
    m_bDocStarted = sal_True;
    mp_SaxWriterHelper->startDocument();
}


void SAXWriter::endDocument(void)                   throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        throw SAXException(
            OUString::createFromAscii( "endDocument called before startDocument" ),
            Reference< XInterface >() , Any() );
    }
    if( m_nLevel ) {
        throw SAXException(
            OUString::createFromAscii( "unexpected end of document" ),
            Reference< XInterface >() , Any() );
    }
    mp_SaxWriterHelper->endDocument();
    try
    {
        m_out->closeOutput();
    }
    catch( IOException & e )
    {
        Any a;
        a <<= e;
        throw SAXException(
            OUString::createFromAscii( "IO exception during closing the IO Stream" ),
            Reference< XInterface > (),
            a );
    }
}


void SAXWriter::startElement(const OUString& aName, const Reference< XAttributeList >& xAttribs)
    throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "startElement called before startDocument" ));
        throw except;
    }
    if( m_bIsCDATA )
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "startElement call not allowed with CDATA sections" ));
        throw except;
    }

    sal_Int32 nLength(0);
    if (m_bAllowLineBreak)
    {
        sal_Int32 nAttribCount = xAttribs.is() ? xAttribs->getLength() : 0;

        nLength ++; // "<"
        nLength += calcXMLByteLength( aName.getStr() , aName.getLength(),
                                  sal_False, sal_False ); // the tag name

        int n;
        for( n = 0 ; n < nAttribCount ; n ++ ) {
            nLength ++; // " "
            OUString tmp =  xAttribs->getNameByIndex( n );

            nLength += calcXMLByteLength( tmp.getStr() , tmp.getLength() , sal_False, sal_False );

            nLength += 2; // ="

            tmp = xAttribs->getValueByIndex( n );

            nLength += calcXMLByteLength( tmp.getStr(), tmp.getLength(), sal_True, sal_True );

            nLength += 1; // "
        }

        nLength ++;  // '>'
    }

    // Is there a new indentation necesarry ?
    sal_Int32 nPrefix(getIndentPrefixLength( nLength ));

    // write into sequence
    if( nPrefix >= 0 )
        mp_SaxWriterHelper->insertIndentation( nPrefix );

    SaxInvalidCharacterError eRet(mp_SaxWriterHelper->startElement(aName, xAttribs));

    m_nLevel++;

    if (eRet == SAX_WARNING)
    {
        SAXInvalidCharacterException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid charcter during XML-Export in a attribute value" ) );
        throw except;
    }
    else if (eRet == SAX_ERROR)
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid charcter during XML-Export" ) );
        throw except;
    }
}

void SAXWriter::endElement(const OUString& aName)   throw (SAXException, RuntimeException)
{
    if( ! m_bDocStarted ) {
        throw SAXException ();
    }
    m_nLevel --;

    if( m_nLevel < 0 ) {
        throw SAXException();
    }
    sal_Bool bRet(sal_True);

    if( mp_SaxWriterHelper->FinishEmptyElement() )
        m_bForceLineBreak = sal_False;
    else
    {
        // only ascii chars allowed
        sal_Int32 nLength(0);
        if (m_bAllowLineBreak)
            nLength = 3 + calcXMLByteLength( aName.getStr(), aName.getLength(), sal_False, sal_False );
        sal_Int32 nPrefix = getIndentPrefixLength( nLength );

        if( nPrefix >= 0 )
            mp_SaxWriterHelper->insertIndentation( nPrefix );

        bRet = mp_SaxWriterHelper->endElement(aName);
    }

    if (!bRet)
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid charcter during XML-Export" ) );
        throw except;
    }
}

void SAXWriter::characters(const OUString& aChars)  throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "characters method called before startDocument" ) );
        throw except;
    }

    sal_Bool bThrowException(sal_False);
    if( aChars.getLength() )
    {
        if( m_bIsCDATA )
            bThrowException = !mp_SaxWriterHelper->writeString( aChars, sal_False, sal_False );
        else
        {
            // Note : nFirstLineBreakOccurence is not exact, because we don't know, how
            //        many 2 and 3 byte chars are inbetween. However this whole stuff
            //        is eitherway for pretty printing only, so it does not need to be exact.
            sal_Int32 nLength(0);
            sal_Int32 nIndentPrefix(-1);
            if (m_bAllowLineBreak)
            {
                sal_Int32 nFirstLineBreakOccurence = getFirstLineBreak( aChars );

                nLength = calcXMLByteLength( aChars.getStr(), aChars.getLength(),
                                               ! m_bIsCDATA , sal_False );
                nIndentPrefix = getIndentPrefixLength(
                    nFirstLineBreakOccurence >= 0 ? nFirstLineBreakOccurence : nLength );
            }
            else
                nIndentPrefix = getIndentPrefixLength(nLength);

            // insert indentation
            sal_Int32 nPos = 0;
            if( nIndentPrefix >= 0 )
            {
                if( isFirstCharWhitespace( aChars.getStr() ) )
                    mp_SaxWriterHelper->insertIndentation( nIndentPrefix - 1 );
                else
                    mp_SaxWriterHelper->insertIndentation( nIndentPrefix );
            }
            bThrowException = !mp_SaxWriterHelper->writeString(aChars, sal_True , sal_False);
        }
    }
    if (bThrowException)
    {
        SAXInvalidCharacterException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid charcter during XML-Export" ) );
        throw except;
    }
}


void SAXWriter::ignorableWhitespace(const OUString& aWhitespaces) throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted )
    {
        throw SAXException ();
    }

    m_bForceLineBreak = sal_True;
}

void SAXWriter::processingInstruction(const OUString& aTarget, const OUString& aData)
    throw (SAXException, RuntimeException)
{
    if( ! m_bDocStarted || m_bIsCDATA )
    {
        throw SAXException();
    }

    sal_Int32 nLength(0);
    if (m_bAllowLineBreak)
    {
        nLength = 2;  // "<?"
        nLength += calcXMLByteLength( aTarget.getStr(), aTarget.getLength(), sal_False, sal_False );

        nLength += 1;  // " "

        nLength += calcXMLByteLength( aData.getStr(), aData.getLength(), sal_False, sal_False );

        nLength += 2; // "?>"
    }

    sal_Int32 nPrefix = getIndentPrefixLength( nLength );

    if( nPrefix >= 0 )
        mp_SaxWriterHelper->insertIndentation( nPrefix );

    if (!mp_SaxWriterHelper->processingInstruction(aTarget, aData))
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid charcter during XML-Export" ) );
        throw except;
    }
}


void SAXWriter::setDocumentLocator(const Reference< XLocator >& xLocator)
        throw (SAXException, RuntimeException)
{

}

void SAXWriter::startCDATA(void) throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted || m_bIsCDATA)
    {
        throw SAXException ();
    }

    sal_Int32 nLength = 9;
    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
        mp_SaxWriterHelper->insertIndentation( nPrefix );

    mp_SaxWriterHelper->startCDATA();

    m_bIsCDATA = sal_True;
}

void SAXWriter::endCDATA(void) throw (RuntimeException)
{
    if( ! m_bDocStarted | ! m_bIsCDATA)
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "endCDATA was called without startCDATA" ) );
        throw except;
    }

    sal_Int32 nLength = 3;
    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
        mp_SaxWriterHelper->insertIndentation( nPrefix );

    mp_SaxWriterHelper->endCDATA();

    m_bIsCDATA = sal_False;
}


void SAXWriter::comment(const OUString& sComment) throw(SAXException, RuntimeException)
{
    if( ! m_bDocStarted || m_bIsCDATA )
    {
        throw SAXException();
    }

    sal_Int32 nLength(0);
    if (m_bAllowLineBreak)
    {
        nLength = 4; // "<!--"
        nLength += calcXMLByteLength( sComment.getStr(), sComment.getLength(), sal_False, sal_False);

        nLength += 3;
    }

    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
        mp_SaxWriterHelper->insertIndentation( nPrefix );

    if (!mp_SaxWriterHelper->comment(sComment))
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid charcter during XML-Export" ) );
        throw except;
    }
}


void SAXWriter::allowLineBreak( )   throw ( SAXException , RuntimeException)
{
    if( ! m_bDocStarted || m_bAllowLineBreak ) {
        throw SAXException();
    }

     m_bAllowLineBreak = sal_True;
}

void SAXWriter::unknown(const OUString& sString) throw (SAXException, RuntimeException)
{

    if( ! m_bDocStarted )
    {
        throw SAXException ();
    }
    if( m_bIsCDATA )
    {
        throw SAXException();
    }

    if( sString.matchAsciiL( "<?xml", 5 ) )
        return;

    sal_Int32 nLength(0);
    if (m_bAllowLineBreak)
        nLength = calcXMLByteLength( sString.getStr(), sString.getLength(), sal_False, sal_False );

    sal_Int32 nPrefix = getIndentPrefixLength( nLength );
    if( nPrefix >= 0 )
        mp_SaxWriterHelper->insertIndentation( nPrefix );

    if (!mp_SaxWriterHelper->writeString( sString, sal_False, sal_False))
    {
        SAXException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid charcter during XML-Export" ) );
        throw except;
    }
}

}

