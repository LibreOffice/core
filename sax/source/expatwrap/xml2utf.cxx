/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include <string.h>

#include <algorithm>

#include <sal/types.h>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>

#include <com/sun/star/io/XInputStream.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;


#include "xml2utf.hxx"

namespace sax_expatwrap {

sal_Int32 XMLFile2UTFConverter::readAndConvert( Sequence<sal_Int8> &seq , sal_Int32 nMaxToRead )
    throw ( IOException, NotConnectedException , BufferSizeExceededException , RuntimeException )
{
    if( ! m_in.is() ) {
        throw NotConnectedException();
    }
    if( ! m_bStarted ) {
        
        
        nMaxToRead = ::std::max( sal_Int32(512) , nMaxToRead );
    }

    sal_Int32 nRead;
    Sequence< sal_Int8 > seqStart;
    while( true )
    {
        nRead = m_in->readSomeBytes( seq , nMaxToRead );

        if( nRead + seqStart.getLength())
        {
            
            if( ! m_bStarted && nRead )
            {
                
                if( seqStart.getLength() )
                {
                  
                  sal_Int32 nLength = seq.getLength();
                  seq.realloc( seqStart.getLength() + nLength );

                  memmove (seq.getArray() + seqStart.getLength(),
                       seq.getConstArray(),
                       nLength);
                  memcpy  (seq.getArray(),
                       seqStart.getConstArray(),
                       seqStart.getLength());
                }

                
                if( ! isEncodingRecognizable( seq ) )
                {
                  
                  seqStart = seq;

                  
                  continue;
                }
                if( scanForEncoding( seq ) || !m_sEncoding.isEmpty() ) {
                    
                    initializeDecoding();
                }
                nRead = seq.getLength();
                seqStart = Sequence < sal_Int8 > ();
            }

            
            if( m_pText2Unicode && m_pUnicode2Text &&
                m_pText2Unicode->canContinue() && m_pUnicode2Text->canContinue() ) {

                Sequence<sal_Unicode> seqUnicode = m_pText2Unicode->convert( seq );
                seq = m_pUnicode2Text->convert( seqUnicode.getConstArray(), seqUnicode.getLength() );
            }

            if( ! m_bStarted )
            {
                
                
                
                
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

        
        OString str( (sal_Char * ) pSource , seq.getLength() );

        
        
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
                
                memmove(        &( seq.getArray()[nFound] ) ,
                                &( seq.getArray()[nStop+1]) ,
                                seq.getLength() - nStop -1);
                seq.realloc( seq.getLength() - ( nStop+1 - nFound ) );

            }
        }
    }
}


sal_Bool XMLFile2UTFConverter::isEncodingRecognizable( const Sequence< sal_Int8 > &seq)
{
    const sal_Int8 *pSource = seq.getConstArray();
    sal_Bool bCheckIfFirstClosingBracketExsists = sal_False;

    if( seq.getLength() < 8 ) {
        
        return sal_False;
    }

    if( ! strncmp( (const char * ) pSource , "<?xml" , 4 ) ) {
        
        bCheckIfFirstClosingBracketExsists = sal_True;
    }
    else if( ('<' == pSource[0] || '<' == pSource[2] ) &&
             ( ('?' == pSource[4] || '?' == pSource[6] ) ) )
    {
        
        bCheckIfFirstClosingBracketExsists = sal_True;
    }
    else if( ( '<' == pSource[1] || '<' == pSource[3] ) &&
             ( '?' == pSource[5] || '?' == pSource[7] ) )
    {
        
        bCheckIfFirstClosingBracketExsists = sal_True;
    }

    if( bCheckIfFirstClosingBracketExsists )
    {
        for( sal_Int32 i = 0; i < seq.getLength() ; i ++ )
        {
            
            if( '>' == pSource[ i ] )
            {
                return sal_True;
            }
        }
        return sal_False;
    }

    
    return sal_True;
}

sal_Bool XMLFile2UTFConverter::scanForEncoding( Sequence< sal_Int8 > &seq )
{
    const sal_uInt8 *pSource = reinterpret_cast<const sal_uInt8*>( seq.getConstArray() );
    sal_Bool bReturn = sal_True;

    if( seq.getLength() < 4 ) {
        
        return sal_False;
    }

    
    if( ! strncmp( (const char * ) pSource , "<?xml" , 4 ) ) {

        
        OString str( (const sal_Char *) pSource , seq.getLength() );

        
        
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
                
                m_sEncoding = str.copy( nStart+1 , nStop - nStart - 1 );
            }
        }
    }
    else if( 0xFE == pSource[0] &&
             0xFF == pSource[1] ) {
        
        
        m_sEncoding = "utf-16";
    }
    else if( 0xFF == pSource[0] &&
             0xFE == pSource[1] ) {
        
        
        m_sEncoding = "utf-16";
    }
    else if( 0x00 == pSource[0] && 0x3c == pSource[1]  && 0x00 == pSource[2] && 0x3f == pSource[3] ) {
        
        

        
        seq.realloc( seq.getLength() + 2 );
        memmove( &( seq.getArray()[2] ) , seq.getArray() , seq.getLength() - 2 );
        ((sal_uInt8*)seq.getArray())[0] = 0xFE;
        ((sal_uInt8*)seq.getArray())[1] = 0xFF;

        m_sEncoding = "utf-16";
    }
    else if( 0x3c == pSource[0] && 0x00 == pSource[1]  && 0x3f == pSource[2] && 0x00 == pSource[3] ) {
        
        

        seq.realloc( seq.getLength() + 2 );
        memmove( &( seq.getArray()[2] ) , seq.getArray() , seq.getLength() - 2 );
        ((sal_uInt8*)seq.getArray())[0] = 0xFF;
        ((sal_uInt8*)seq.getArray())[1] = 0xFE;

        m_sEncoding = "utf-16";
    }
    else if( 0xEF == pSource[0] &&
             0xBB == pSource[1] &&
             0xBF == pSource[2] )
    {
        
        
        memmove( seq.getArray(), &( seq.getArray()[3] ), seq.getLength()-3 );
        seq.realloc( seq.getLength() - 3 );
        m_sEncoding = "utf-8";
    }
    else if( 0x00 == pSource[0] && 0x00 == pSource[1]  && 0x00 == pSource[2] && 0x3c == pSource[3] ) {
        
        m_sEncoding = "ucs-4";
    }
    else if( 0x3c == pSource[0] && 0x00 == pSource[1]  && 0x00 == pSource[2] && 0x00 == pSource[3] ) {
        
        m_sEncoding = "ucs-4";
    }
/* TODO: no need to test for the moment since we return sal_False like default case anyway
    else if( 0x4c == pSource[0] && 0x6f == pSource[1]  &&
             0xa7 == static_cast<unsigned char> (pSource[2]) &&
             0x94 == static_cast<unsigned char> (pSource[3]) ) {
        
        bReturn = sal_False;   
    }
*/
    else {
        
        
        bReturn = sal_False;
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



//

//

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

    
    sal_Int32   nSourceSize = seqText.getLength() + m_seqSource.getLength();
    Sequence<sal_Unicode>   seqUnicode ( nSourceSize );

    const sal_Int8 *pbSource = seqText.getConstArray();
    sal_Int8 *pbTempMem = 0;

    if( m_seqSource.getLength() ) {
        
        pbTempMem = new sal_Int8[ nSourceSize ];
        memcpy( pbTempMem , m_seqSource.getConstArray() , m_seqSource.getLength() );
        memcpy( &(pbTempMem[ m_seqSource.getLength() ]) , seqText.getConstArray() , seqText.getLength() );
        pbSource = pbTempMem;

        
        m_seqSource = Sequence< sal_Int8 >();
    }

    while( true ) {

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
        delete [] pbTempMem;
    }

    
    seqUnicode.realloc( nTargetCount );

    return seqUnicode;
}




//

//

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
        
        
        
        
        
        puTempMem = new sal_Unicode[ nSourceSize + m_seqSource.getLength()];
        memcpy( puTempMem ,
                m_seqSource.getConstArray() ,
                m_seqSource.getLength() * sizeof( sal_Unicode ) );
        memcpy(
            &(puTempMem[ m_seqSource.getLength() ]) ,
            puSource ,
            nSourceSize*sizeof( sal_Unicode ) );
        puSource = puTempMem;
        nSourceSize += m_seqSource.getLength();

        m_seqSource = Sequence< sal_Unicode > ();
    }


    sal_Size nTargetCount = 0;
    sal_Size nSourceCount = 0;

    sal_uInt32 uiInfo;
    sal_Size nSrcCvtChars;

    
    
    
    sal_Int32 nSeqSize =  nSourceSize * 3;

    Sequence<sal_Int8>  seqText( nSeqSize );
    sal_Char *pTarget = (sal_Char *) seqText.getArray();
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
            seqText.realloc( nSeqSize );  
            pTarget = ( sal_Char * ) seqText.getArray();
            continue;
        }
        break;
    }

    
    if( uiInfo & RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL ) {
        m_seqSource.realloc( nSourceSize - nSourceCount );
        memcpy( m_seqSource.getArray() ,
                &(puSource[nSourceCount]),
                (nSourceSize - nSourceCount) * sizeof( sal_Unicode ) );
    }

    if( puTempMem ) {
        delete [] puTempMem;
    }

    
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


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
