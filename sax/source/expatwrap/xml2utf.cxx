/*************************************************************************
 *
 *  $RCSfile: xml2utf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:43:13 $
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
#include <assert.h>

#include <sal/types.h>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>


#include <com/sun/star/io/XInputStream.hpp>

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

#include "xml2utf.hxx"


sal_Int32 XMLFile2UTFConverter::readAndConvert( Sequence<sal_Int8> &seq , sal_Int32 nMaxToRead )
    throw ( IOException, NotConnectedException , BufferSizeExceededException , RuntimeException )
{

    Sequence<sal_Int8> seqIn;

    if( ! m_in.is() ) {
        throw NotConnectedException();
    }
    if( ! m_bStarted ) {
        nMaxToRead = Max( 512 , nMaxToRead );   // it should be possible to find the encoding attribute
                                                 // within the first 512 bytes == 128 chars in UCS-4
    }

    sal_Int32 nRead;
    Sequence< sal_Int8 > seqStart;
    while( sal_True )
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
                    seq.realloc( seqStart.getLength() + seq.getLength() );
                    memcpy( (sal_Int8*)seq.getConstArray() + seqStart.getLength() ,
                            seq.getConstArray() ,
                            seq.getLength() );
                    memcpy( (sal_Int8*)seq.getConstArray() ,
                            seqStart.getConstArray(),
                            seqStart.getLength() );
                }

                // autodetection with the first bytes
                if( ! isEncodingRecognizable( seq ) )
                {
                    seqStart.realloc( seqStart.getLength() + seq.getLength() );
                    memcpy( (sal_Int8*)seqStart.getConstArray() + seqStart.getLength(),
                            seq.getConstArray(),
                            seq.getLength());
                    // read more !
                    continue;
                }
                if( scanForEncoding( seq ) || m_sEncoding.getLength() ) {
                    // initialize decoding
                    initializeDecoding();
                }
                nRead = seq.getLength();
                seqStart = Sequence < sal_Int8 > ();
            }

            // do the encoding
            if( m_pText2Unicode && m_pUnicode2Text &&
                m_pText2Unicode->canContinue() && m_pUnicode2Text->canContinue() ) {

                Sequence<sal_Unicode> seqUnicode = m_pText2Unicode->convert( seq );
                seq = m_pUnicode2Text->convert( seqUnicode.getConstArray(), seqUnicode.getLength() );
            }

            if( ! m_bStarted )
            {
                // it must now be ensured, that no encoding attribute exist anymore
                // ( otherwise the expat-Parser will crash )
                // This must be done after decoding !
                // ( e.g. Files decoded in ucs-4 cannot be read properly )
                m_bStarted = sal_True;
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
    if( m_pText2Unicode )
        delete m_pText2Unicode;
    if( m_pUnicode2Text )
        delete m_pUnicode2Text;
}


void XMLFile2UTFConverter::removeEncoding( Sequence<sal_Int8> &seq )
{
    const sal_Int8 *pSource = seq.getArray();
    if( ! strncmp( (const char * ) pSource , "<?xml" , 4) )
    {

        // scan for encoding
        OString str( (sal_Char * ) pSource , seq.getLength() );

        // cut sequence to first line break
        // find first line break;
        int nMax = str.indexOf( 10 );
        if( nMax >= 0 )
        {
            str = str.copy( 0 , nMax );
        }

        int nFound = str.indexOf( " encoding" );
        if( nFound < str.getLength() ) {
            int nStop;
            int nStart = str.indexOf( "\"" , nFound );
            if( nStart < 0 || str.indexOf( "'" , nFound ) < nStart )
            {
                nStart = str.indexOf( "'" , nFound );
                nStop  = str.indexOf( "'" , nStart +1 );
            }
            else
            {
                int nStop  = str.indexOf( "\"" , nStart +1);
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
sal_Bool XMLFile2UTFConverter::isEncodingRecognizable( const Sequence< sal_Int8 > &seq)
{
    const sal_Int8 *pSource = seq.getConstArray();
    sal_Bool bCheckIfFirstClosingBracketExsists = sal_False;

    if( seq.getLength() < 8 ) {
        // no recognition possible, when less than 8 bytes are available
        return sal_False;
    }

    if( ! strncmp( (const char * ) pSource , "<?xml" , 4 ) ) {
        // scan if the <?xml tag finishes within this buffer
        bCheckIfFirstClosingBracketExsists = sal_True;
    }
    else if( ('<' == pSource[0] || '<' == pSource[2] ) &&
             ( ('?' == pSource[4] || '?' == pSource[6] ) ) )
    {
        // check for utf-16
        bCheckIfFirstClosingBracketExsists = sal_True;
    }
    else if( ( '<' == pSource[1] || '<' == pSource[3] ) &&
             ( '?' == pSource[5] || '?' == pSource[7] ) )
    {
        // check for
        bCheckIfFirstClosingBracketExsists = sal_True;
    }

    if( bCheckIfFirstClosingBracketExsists )
    {
        for( sal_Int32 i = 0; i < seq.getLength() ; i ++ )
        {
            // whole <?xml tag is valid
            if( '>' == pSource[ i ] )
            {
                return sal_True;
            }
        }
        return sal_False;
    }

    // No <? tag in front, no need for a bigger buffer
    return sal_True;
}

sal_Bool XMLFile2UTFConverter::scanForEncoding( Sequence< sal_Int8 > &seq )
{
    const sal_Int8 *pSource = seq.getConstArray();
    sal_Bool bReturn = sal_True;

    if( seq.getLength() < 4 ) {
        // no recognition possible, when less than 4 bytes are available
        return sal_False;
    }

    // first level : detect possible file formats
    if( ! strncmp( (const char * ) pSource , "<?xml" , 4 ) ) {

        // scan for encoding
        OString str( (const sal_Char *) pSource , seq.getLength() );

        // cut sequence to first line break
        //find first line break;
        int nMax = str.indexOf( 10 );
        if( nMax >= 0 )
        {
            str = str.copy( 0 , nMax );
        }

        int nFound = str.indexOf( " encoding" );
        if( nFound < str.getLength() ) {
            int nStop;
            int nStart = str.indexOf( "\"" , nFound );
            if( nStart < 0 || str.indexOf( "'" , nFound ) < nStart )
            {
                nStart = str.indexOf( "'" , nFound );
                nStop  = str.indexOf( "'" , nStart +1 );
            }
            else
            {
                int nStop  = str.indexOf( "\"" , nStart +1);
            }
            if( nStart >= 0 && nStop >= 0 && nStart+1 < nStop )
            {
                // encoding found finally
                m_sEncoding = str.copy( nStart+1 , nStop - nStart - 1 );
            }
        }
    }
    else if( 0xFE == pSource[0] && 0xFF == pSource[1] ) {
        // UTF-16 big endian
        // conversion is done so that encoding information can be easily extracted
        m_sEncoding = "utf-16";
    }
    else if( 0xFF == pSource[0] && 0xFE == pSource[1] ) {
        // UTF-16 little endian
        // conversion is done so that encoding information can be easily extracted
        m_sEncoding = "utf-16";
    }
    else if( 0x00 == pSource[0] && 0x3c == pSource[1]  && 0x00 == pSource[2] && 0x3f == pSource[3] ) {
        // UTF-16 big endian without byte order mark (this is (strictly speaking) an error.)
        // The byte order mark is simply added

        // simply add the byte order mark !
        seq.realloc( seq.getLength() + 2 );
        memmove( &( seq.getArray()[2] ) , seq.getArray() , seq.getLength() );
        ((sal_uInt8*)seq.getArray())[0] = 0xFE;
        ((sal_uInt8*)seq.getArray())[1] = 0xFF;

        m_sEncoding = "utf-16";
    }
    else if( 0x3c == pSource[0] && 0x00 == pSource[1]  && 0x3f == pSource[2] && 0x00 == pSource[3] ) {
        // UTF-16 little endian without byte order mark (this is (strictly speaking) an error.)
        // The byte order mark is simply added

        seq.realloc( seq.getLength() + 2 );
        memmove( &( seq.getArray()[2] ) , seq.getArray() , seq.getLength() );
        ((sal_uInt8*)seq.getArray())[0] = 0xFF;
        ((sal_uInt8*)seq.getArray())[1] = 0xFE;

        m_sEncoding = "utf-16";
    }
    else if( 0x00 == pSource[0] && 0x00 == pSource[1]  && 0x00 == pSource[2] && 0x3c == pSource[3] ) {
        // UCS-4 big endian
        m_sEncoding = "ucs-4";
    }
    else if( 0x3c == pSource[0] && 0x00 == pSource[1]  && 0x00 == pSource[2] && 0x00 == pSource[3] ) {
        // UCS-4 little endian
        m_sEncoding = "ucs-4";
    }
    else if( 0x4c == pSource[0] && 0x6f == pSource[1]  && 0xa7 == pSource[2] && 0x94 == pSource[3] ) {
        // EBCDIC
        bReturn = sal_False;   // must be extended
    }
    else {
        // other
        // UTF8 is directly recognized by the parser.
        bReturn = sal_False;
    }

    return bReturn;
}

void XMLFile2UTFConverter::initializeDecoding()
{

    if( m_sEncoding.getLength() )
    {
        rtl_TextEncoding encoding = rtl_getTextEncodingFromMimeCharset( m_sEncoding.getStr() );
        if( encoding != RTL_TEXTENCODING_UTF8 )
        {
            m_pText2Unicode = new Text2UnicodeConverter( m_sEncoding );
            m_pUnicode2Text = new Unicode2TextConverter( RTL_TEXTENCODING_UTF8 );
        }
    }
}


//----------------------------------------------
//
// Text2UnicodeConverter
//
//----------------------------------------------
Text2UnicodeConverter::Text2UnicodeConverter( const OString &sEncoding )
{
    rtl_TextEncoding encoding = rtl_getTextEncodingFromMimeCharset( sEncoding.getStr() );
    if( RTL_TEXTENCODING_DONTKNOW == encoding )
    {
        m_bCanContinue = sal_False;
        m_bInitialized = sal_False;
    }
    else
    {
        init( encoding );
    }
}

Text2UnicodeConverter::Text2UnicodeConverter( rtl_TextEncoding encoding )
{
    init( encoding );
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
    m_bCanContinue = sal_True;
    m_bInitialized = sal_True;

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
    sal_Int8 *pbTempMem = 0;

    if( m_seqSource.getLength() ) {
        // put old rest and new byte sequence into one array
        pbTempMem = new sal_Int8[ nSourceSize ];
        memcpy( pbTempMem , m_seqSource.getConstArray() , m_seqSource.getLength() );
        memcpy( &(pbTempMem[ m_seqSource.getLength() ]) , seqText.getConstArray() , seqText.getLength() );
        pbSource = pbTempMem;

        // set to zero again
        m_seqSource = Sequence< sal_Int8 >();
    }

    while( sal_True ) {

        /* All invalid characters are transformed to the unicode undefined char */
        nTargetCount +=     rtl_convertTextToUnicode(
                                    m_convText2Unicode,
                                    m_contextText2Unicode,
                                    ( const sal_Char * ) &( pbSource[nSourceCount] ),
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


    if( pbTempMem ) {
        delete pbTempMem;
    }

    // set to correct unicode size
    seqUnicode.realloc( nTargetCount );

    return seqUnicode;
}



//----------------------------------------------
//
// Unicode2TextConverter
//
//----------------------------------------------
Unicode2TextConverter::Unicode2TextConverter( const OString &sEncoding )
{
    rtl_TextEncoding encoding = rtl_getTextEncodingFromMimeCharset( sEncoding.getStr() );
    if( RTL_TEXTENCODING_DONTKNOW == encoding ) {
        m_bCanContinue = sal_False;
        m_bInitialized = sal_False;
    }
    else {
        init( encoding );
    }

}

Unicode2TextConverter::Unicode2TextConverter( rtl_TextEncoding encoding )
{
    init( encoding );
}


Unicode2TextConverter::~Unicode2TextConverter()
{
    if( m_bInitialized ) {
        rtl_destroyUnicodeToTextContext( m_convUnicode2Text , m_contextUnicode2Text );
        rtl_destroyUnicodeToTextConverter( m_convUnicode2Text );
    }
}


Sequence<sal_Int8> Unicode2TextConverter::convert(const sal_Unicode *puSource , sal_Int32 nSourceSize)
{
    sal_Unicode *puTempMem = 0;

    if( m_seqSource.getLength() ) {
        // For surrogates !
        // put old rest and new byte sequence into one array
        // In general when surrogates are used, they should be rarely
        // cut off between two convert()-calls. So this code is used
        // rarely and the extra copy is acceptable.
        nSourceSize += m_seqSource.getLength();

        puTempMem = new sal_Unicode[ nSourceSize ];
        memcpy( puTempMem ,
                m_seqSource.getConstArray() ,
                m_seqSource.getLength() * sizeof( sal_Unicode ) );
        memcpy(
            &(puTempMem[ m_seqSource.getLength() ]) ,
            puSource ,
            nSourceSize*sizeof( sal_Unicode ) );
        puSource = puTempMem;

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
    sal_Char *pTarget = (sal_Char *) seqText.getArray();
    while( sal_True ) {

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
            pTarget = ( sal_Char * ) seqText.getArray();
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

    if( puTempMem ) {
        delete puTempMem;
    }

    // reduce the size of the buffer (fast, no copy necessary)
    seqText.realloc( nTargetCount );

    return seqText;
}

void Unicode2TextConverter::init( rtl_TextEncoding encoding )
{
    m_bCanContinue = sal_True;
    m_bInitialized = sal_True;

    m_convUnicode2Text  = rtl_createUnicodeToTextConverter( encoding );
    m_contextUnicode2Text = rtl_createUnicodeToTextContext( m_convUnicode2Text );
    m_rtlEncoding = encoding;
};


