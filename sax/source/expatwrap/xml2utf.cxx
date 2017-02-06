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

#include <algorithm>

#include <sal/types.h>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XInputStream.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;


#include "xml2utf.hxx"
#include <memory>

namespace sax_expatwrap {

sal_Int32 XMLFile2UTFConverter::readAndConvert( Sequence<sal_Int8> &seq , sal_Int32 nMaxToRead )
{
    if( ! m_in.is() ) {
        throw NotConnectedException();
    }
    if( ! m_bStarted ) {
        // it should be possible to find the encoding attribute
        // within the first 512 bytes == 128 chars in UCS-4
        nMaxToRead = ::std::max( sal_Int32(512) , nMaxToRead );
    }

    sal_Int32 nRead;
    Sequence< sal_Int8 > seqStart;
    while( true )
    {
        nRead = m_in->readSomeBytes( seq , nMaxToRead );

        if( nRead + seqStart.getLength())
        {
            // if nRead is 0, the file is already eof.
            if( ! m_bStarted && nRead )
            {
                // ensure that enough data is available to parse encoding
                if( seqStart.getLength() )
                {
                  // prefix with what we had so far.
                  sal_Int32 nLength = seq.getLength();
                  seq.realloc( seqStart.getLength() + nLength );

                  memmove (seq.getArray() + seqStart.getLength(),
                       seq.getConstArray(),
                       nLength);
                  memcpy  (seq.getArray(),
                       seqStart.getConstArray(),
                       seqStart.getLength());
                }

                // autodetection with the first bytes
                if( ! isEncodingRecognizable( seq ) )
                {
                  // remember what we have so far.
                  seqStart = seq;

                  // read more !
                  continue;
                }
                if( scanForEncoding( seq ) || !m_sEncoding.isEmpty() ) {
                    // initialize decoding
                    initializeDecoding();
                }
                seqStart = Sequence < sal_Int8 > ();
            }

            // do the encoding
            if( m_pText2Unicode && m_pUnicode2Text &&
                m_pText2Unicode->canContinue() ) {

                Sequence<sal_Unicode> seqUnicode = m_pText2Unicode->convert( seq );
                seq = m_pUnicode2Text->convert( seqUnicode.getConstArray(), seqUnicode.getLength() );
            }

            if( ! m_bStarted )
            {
                // it must now be ensured, that no encoding attribute exist anymore
                // ( otherwise the expat-Parser will crash )
                // This must be done after decoding !
                // ( e.g. Files decoded in ucs-4 cannot be read properly )
                m_bStarted = true;
                removeEncoding( seq );
            }
            nRead = seq.getLength();
        }

        break;
    }
    return nRead;
}


XMLFile2UTFConverter::~XMLFile2UTFConverter()
{
    delete m_pText2Unicode;
    delete m_pUnicode2Text;
}


void XMLFile2UTFConverter::removeEncoding( Sequence<sal_Int8> &seq )
{
    const sal_Int8 *pSource = seq.getArray();
    if( ! strncmp( reinterpret_cast<const char *>(pSource), "<?xml", 4) )
    {

        // scan for encoding
        OString str( reinterpret_cast<char const *>(pSource), seq.getLength() );

        // cut sequence to first line break
        // find first line break;
        int nMax = str.indexOf( 10 );
        if( nMax >= 0 )
        {
            str = str.copy( 0 , nMax );
        }

        int nFound = str.indexOf( " encoding" );
        if( nFound >= 0 ) {
            int nStop;
            int nStart = str.indexOf( "\"" , nFound );
            if( nStart < 0 || str.indexOf( "'" , nFound ) < nStart )
            {
                nStart = str.indexOf( "'" , nFound );
                nStop  = str.indexOf( "'" , nStart +1 );
            }
            else
            {
                nStop  = str.indexOf( "\"" , nStart +1);
            }

            if( nStart >= 0 && nStop >= 0 && nStart+1 < nStop )
            {
                // remove encoding tag from file
                memmove(        &( seq.getArray()[nFound] ) ,
                                &( seq.getArray()[nStop+1]) ,
                                seq.getLength() - nStop -1);
                seq.realloc( seq.getLength() - ( nStop+1 - nFound ) );
//              str = String( (char * ) seq.getArray() , seq.getLen() );
            }
        }
    }
}

// Checks, if enough data has been accumulated to recognize the encoding
bool XMLFile2UTFConverter::isEncodingRecognizable( const Sequence< sal_Int8 > &seq)
{
    const sal_Int8 *pSource = seq.getConstArray();
    bool bCheckIfFirstClosingBracketExsists = false;

    if( seq.getLength() < 8 ) {
        // no recognition possible, when less than 8 bytes are available
        return false;
    }

    if( ! strncmp( reinterpret_cast<const char *>(pSource), "<?xml", 4 ) ) {
        // scan if the <?xml tag finishes within this buffer
        bCheckIfFirstClosingBracketExsists = true;
    }
    else if( ('<' == pSource[0] || '<' == pSource[2] ) &&
             ( ('?' == pSource[4] || '?' == pSource[6] ) ) )
    {
        // check for utf-16
        bCheckIfFirstClosingBracketExsists = true;
    }
    else if( ( '<' == pSource[1] || '<' == pSource[3] ) &&
             ( '?' == pSource[5] || '?' == pSource[7] ) )
    {
        // check for
        bCheckIfFirstClosingBracketExsists = true;
    }

    if( bCheckIfFirstClosingBracketExsists )
    {
        for( sal_Int32 i = 0; i < seq.getLength() ; i ++ )
        {
            // whole <?xml tag is valid
            if( '>' == pSource[ i ] )
            {
                return true;
            }
        }
        return false;
    }

    // No <? tag in front, no need for a bigger buffer
    return true;
}

bool XMLFile2UTFConverter::scanForEncoding( Sequence< sal_Int8 > &seq )
{
    const sal_uInt8 *pSource = reinterpret_cast<const sal_uInt8*>( seq.getConstArray() );
    bool bReturn = true;

    if( seq.getLength() < 4 ) {
        // no recognition possible, when less than 4 bytes are available
        return false;
    }

    // first level : detect possible file formats
    if( ! strncmp( reinterpret_cast<const char *>(pSource), "<?xml", 4 ) ) {

        // scan for encoding
        OString str( reinterpret_cast<const char *>(pSource), seq.getLength() );

        // cut sequence to first line break
        //find first line break;
        int nMax = str.indexOf( 10 );
        if( nMax >= 0 )
        {
            str = str.copy( 0 , nMax );
        }

        int nFound = str.indexOf( " encoding" );
        if( nFound >= 0 ) {
            int nStop;
            int nStart = str.indexOf( "\"" , nFound );
            if( nStart < 0 || str.indexOf( "'" , nFound ) < nStart )
            {
                nStart = str.indexOf( "'" , nFound );
                nStop  = str.indexOf( "'" , nStart +1 );
            }
            else
            {
                nStop  = str.indexOf( "\"" , nStart +1);
            }
            if( nStart >= 0 && nStop >= 0 && nStart+1 < nStop )
            {
                // encoding found finally
                m_sEncoding = str.copy( nStart+1 , nStop - nStart - 1 );
            }
        }
    }
    else if( 0xFE == pSource[0] &&
             0xFF == pSource[1] ) {
        // UTF-16 big endian
        // conversion is done so that encoding information can be easily extracted
        m_sEncoding = "utf-16";
    }
    else if( 0xFF == pSource[0] &&
             0xFE == pSource[1] ) {
        // UTF-16 little endian
        // conversion is done so that encoding information can be easily extracted
        m_sEncoding = "utf-16";
    }
    else if( 0x00 == pSource[0] && 0x3c == pSource[1]  && 0x00 == pSource[2] && 0x3f == pSource[3] ) {
        // UTF-16 big endian without byte order mark (this is (strictly speaking) an error.)
        // The byte order mark is simply added

        // simply add the byte order mark !
        seq.realloc( seq.getLength() + 2 );
        memmove( &( seq.getArray()[2] ) , seq.getArray() , seq.getLength() - 2 );
        reinterpret_cast<sal_uInt8*>(seq.getArray())[0] = 0xFE;
        reinterpret_cast<sal_uInt8*>(seq.getArray())[1] = 0xFF;

        m_sEncoding = "utf-16";
    }
    else if( 0x3c == pSource[0] && 0x00 == pSource[1]  && 0x3f == pSource[2] && 0x00 == pSource[3] ) {
        // UTF-16 little endian without byte order mark (this is (strictly speaking) an error.)
        // The byte order mark is simply added

        seq.realloc( seq.getLength() + 2 );
        memmove( &( seq.getArray()[2] ) , seq.getArray() , seq.getLength() - 2 );
        reinterpret_cast<sal_uInt8*>(seq.getArray())[0] = 0xFF;
        reinterpret_cast<sal_uInt8*>(seq.getArray())[1] = 0xFE;

        m_sEncoding = "utf-16";
    }
    else if( 0xEF == pSource[0] &&
             0xBB == pSource[1] &&
             0xBF == pSource[2] )
    {
        // UTF-8 BOM (byte order mark); signifies utf-8, and not byte order
        // The BOM is removed.
        memmove( seq.getArray(), &( seq.getArray()[3] ), seq.getLength()-3 );
        seq.realloc( seq.getLength() - 3 );
        m_sEncoding = "utf-8";
    }
    else if( 0x00 == pSource[0] && 0x00 == pSource[1]  && 0x00 == pSource[2] && 0x3c == pSource[3] ) {
        // UCS-4 big endian
        m_sEncoding = "ucs-4";
    }
    else if( 0x3c == pSource[0] && 0x00 == pSource[1]  && 0x00 == pSource[2] && 0x00 == pSource[3] ) {
        // UCS-4 little endian
        m_sEncoding = "ucs-4";
    }
/* TODO: no need to test for the moment since we return sal_False like default case anyway
    else if( 0x4c == pSource[0] && 0x6f == pSource[1]  &&
             0xa7 == static_cast<unsigned char> (pSource[2]) &&
             0x94 == static_cast<unsigned char> (pSource[3]) ) {
        // EBCDIC
        bReturn = sal_False;   // must be extended
    }
*/
    else {
        // other
        // UTF8 is directly recognized by the parser.
        bReturn = false;
    }

    return bReturn;
}

void XMLFile2UTFConverter::initializeDecoding()
{

    if( !m_sEncoding.isEmpty() )
    {
        rtl_TextEncoding encoding = rtl_getTextEncodingFromMimeCharset( m_sEncoding.getStr() );
        if( encoding != RTL_TEXTENCODING_UTF8 )
        {
            m_pText2Unicode = new Text2UnicodeConverter( m_sEncoding );
            m_pUnicode2Text = new Unicode2TextConverter( RTL_TEXTENCODING_UTF8 );
        }
    }
}


// Text2UnicodeConverter


Text2UnicodeConverter::Text2UnicodeConverter( const OString &sEncoding )
    : m_convText2Unicode(nullptr)
    , m_contextText2Unicode(nullptr)
    , m_rtlEncoding(RTL_TEXTENCODING_DONTKNOW)
{
    rtl_TextEncoding encoding = rtl_getTextEncodingFromMimeCharset( sEncoding.getStr() );
    if( RTL_TEXTENCODING_DONTKNOW == encoding )
    {
        m_bCanContinue = false;
        m_bInitialized = false;
    }
    else
    {
        init( encoding );
    }
}

Text2UnicodeConverter::~Text2UnicodeConverter()
{
    if( m_bInitialized )
    {
        rtl_destroyTextToUnicodeContext( m_convText2Unicode , m_contextText2Unicode );
        rtl_destroyUnicodeToTextConverter( m_convText2Unicode );
    }
}

void Text2UnicodeConverter::init( rtl_TextEncoding encoding )
{
    m_bCanContinue = true;
    m_bInitialized = true;

    m_convText2Unicode  = rtl_createTextToUnicodeConverter(encoding);
    m_contextText2Unicode = rtl_createTextToUnicodeContext( m_convText2Unicode );
    m_rtlEncoding = encoding;
}


Sequence<sal_Unicode> Text2UnicodeConverter::convert( const Sequence<sal_Int8> &seqText )
{
    sal_uInt32 uiInfo;
    sal_Size nSrcCvtBytes   = 0;
    sal_Size nTargetCount   = 0;
    sal_Size nSourceCount   = 0;

    // the whole source size
    sal_Int32   nSourceSize = seqText.getLength() + m_seqSource.getLength();
    Sequence<sal_Unicode>   seqUnicode ( nSourceSize );

    const sal_Int8 *pbSource = seqText.getConstArray();
    std::unique_ptr<sal_Int8[]> pbTempMem;

    if( m_seqSource.getLength() ) {
        // put old rest and new byte sequence into one array
        pbTempMem.reset(new sal_Int8[ nSourceSize ]);
        memcpy( pbTempMem.get() , m_seqSource.getConstArray() , m_seqSource.getLength() );
        memcpy( &(pbTempMem[ m_seqSource.getLength() ]) , seqText.getConstArray() , seqText.getLength() );
        pbSource = pbTempMem.get();

        // set to zero again
        m_seqSource = Sequence< sal_Int8 >();
    }

    while( true ) {

        /* All invalid characters are transformed to the unicode undefined char */
        nTargetCount +=     rtl_convertTextToUnicode(
                                    m_convText2Unicode,
                                    m_contextText2Unicode,
                                    reinterpret_cast<const char *>(&( pbSource[nSourceCount] )),
                                    nSourceSize - nSourceCount ,
                                    &( seqUnicode.getArray()[ nTargetCount ] ),
                                    seqUnicode.getLength() - nTargetCount,
                                    RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT   |
                                    RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                    RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                    &uiInfo,
                                    &nSrcCvtBytes );
        nSourceCount += nSrcCvtBytes;

        if( uiInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL ) {
            // save necessary bytes for next conversion
            seqUnicode.realloc( seqUnicode.getLength() * 2 );
            continue;
        }
        break;
    }
    if( uiInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL ) {
        m_seqSource.realloc( nSourceSize - nSourceCount );
        memcpy( m_seqSource.getArray() , &(pbSource[nSourceCount]) , nSourceSize-nSourceCount );
    }

    // set to correct unicode size
    seqUnicode.realloc( nTargetCount );

    return seqUnicode;
}


// Unicode2TextConverter


Unicode2TextConverter::Unicode2TextConverter( rtl_TextEncoding encoding )
{
    m_convUnicode2Text  = rtl_createUnicodeToTextConverter( encoding );
    m_contextUnicode2Text = rtl_createUnicodeToTextContext( m_convUnicode2Text );
    m_rtlEncoding = encoding;
}


Unicode2TextConverter::~Unicode2TextConverter()
{
    rtl_destroyUnicodeToTextContext( m_convUnicode2Text , m_contextUnicode2Text );
    rtl_destroyUnicodeToTextConverter( m_convUnicode2Text );
}


Sequence<sal_Int8> Unicode2TextConverter::convert(const sal_Unicode *puSource , sal_Int32 nSourceSize)
{
    std::unique_ptr<sal_Unicode[]> puTempMem;

    if( m_seqSource.getLength() ) {
        // For surrogates !
        // put old rest and new byte sequence into one array
        // In general when surrogates are used, they should be rarely
        // cut off between two convert()-calls. So this code is used
        // rarely and the extra copy is acceptable.
        puTempMem.reset(new sal_Unicode[ nSourceSize + m_seqSource.getLength()]);
        memcpy( puTempMem.get() ,
                m_seqSource.getConstArray() ,
                m_seqSource.getLength() * sizeof( sal_Unicode ) );
        memcpy(
            &(puTempMem[ m_seqSource.getLength() ]) ,
            puSource ,
            nSourceSize*sizeof( sal_Unicode ) );
        puSource = puTempMem.get();
        nSourceSize += m_seqSource.getLength();

        m_seqSource = Sequence< sal_Unicode > ();
    }


    sal_Size nTargetCount = 0;
    sal_Size nSourceCount = 0;

    sal_uInt32 uiInfo;
    sal_Size nSrcCvtChars;

    // take nSourceSize * 3 as preference
    // this is an upper boundary for converting to utf8,
    // which most often used as the target.
    sal_Int32 nSeqSize =  nSourceSize * 3;

    Sequence<sal_Int8>  seqText( nSeqSize );
    sal_Char *pTarget = reinterpret_cast<char *>(seqText.getArray());
    while( true ) {

        nTargetCount += rtl_convertUnicodeToText(
                                    m_convUnicode2Text,
                                    m_contextUnicode2Text,
                                    &( puSource[nSourceCount] ),
                                    nSourceSize - nSourceCount ,
                                    &( pTarget[nTargetCount] ),
                                    nSeqSize - nTargetCount,
                                    RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |
                                    RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT ,
                                    &uiInfo,
                                    &nSrcCvtChars);
        nSourceCount += nSrcCvtChars;

        if( uiInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL ) {
            nSeqSize = nSeqSize *2;
            seqText.realloc( nSeqSize );  // double array size
            pTarget = reinterpret_cast<char *>(seqText.getArray());
            continue;
        }
        break;
    }

    // for surrogates
    if( uiInfo & RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL ) {
        m_seqSource.realloc( nSourceSize - nSourceCount );
        memcpy( m_seqSource.getArray() ,
                &(puSource[nSourceCount]),
                (nSourceSize - nSourceCount) * sizeof( sal_Unicode ) );
    }

    // reduce the size of the buffer (fast, no copy necessary)
    seqText.realloc( nTargetCount );

    return seqText;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
