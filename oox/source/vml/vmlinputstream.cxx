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

#include "oox/vml/vmlinputstream.hxx"

#include <com/sun/star/io/XTextInputStream2.hpp>
#include <map>
#include <string.h>
#include <rtl/strbuf.hxx>
#include "oox/helper/helper.hxx"
#include "oox/helper/textinputstream.hxx"

namespace oox {
namespace vml {



using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;



namespace {

inline const sal_Char* lclFindCharacter( const sal_Char* pcBeg, const sal_Char* pcEnd, sal_Char cChar )
{
    sal_Int32 nIndex = rtl_str_indexOfChar_WithLength( pcBeg, static_cast< sal_Int32 >( pcEnd - pcBeg ), cChar );
    return (nIndex < 0) ? pcEnd : (pcBeg + nIndex);
}

inline bool lclIsWhiteSpace( sal_Char cChar )
{
    return cChar < 32;
}

const sal_Char* lclFindWhiteSpace( const sal_Char* pcBeg, const sal_Char* pcEnd )
{
    for( ; pcBeg < pcEnd; ++pcBeg )
        if( lclIsWhiteSpace( *pcBeg ) )
            return pcBeg;
    return pcEnd;
}

const sal_Char* lclFindNonWhiteSpace( const sal_Char* pcBeg, const sal_Char* pcEnd )
{
    for( ; pcBeg < pcEnd; ++pcBeg )
        if( !lclIsWhiteSpace( *pcBeg ) )
            return pcBeg;
    return pcEnd;
}

const sal_Char* lclTrimWhiteSpaceFromEnd( const sal_Char* pcBeg, const sal_Char* pcEnd )
{
    while( (pcBeg < pcEnd) && lclIsWhiteSpace( pcEnd[ -1 ] ) )
        --pcEnd;
    return pcEnd;
}

inline void lclAppendToBuffer( OStringBuffer& rBuffer, const sal_Char* pcBeg, const sal_Char* pcEnd )
{
    rBuffer.append( pcBeg, static_cast< sal_Int32 >( pcEnd - pcBeg ) );
}



void lclProcessAttribs( OStringBuffer& rBuffer, const sal_Char* pcBeg, const sal_Char* pcEnd )
{
    /*  Map attribute names to char-pointer of all attributes. This map is used
        to find multiple occurrences of attributes with the same name. The
        mapped pointers are used as map key in the next map below. */
    typedef ::std::map< OString, const sal_Char* > AttributeNameMap;
    AttributeNameMap aAttributeNames;

    /*  Map the char-pointers of all attributes to the full attribute definition
        string. This preserves the original order of the used attributes. */
    typedef ::std::map< const sal_Char*, OString > AttributeDataMap;
    AttributeDataMap aAttributes;

    bool bOk = true;
    const sal_Char* pcNameBeg = pcBeg;
    while( bOk && (pcNameBeg < pcEnd) )
    {
        
        const sal_Char* pcEqualSign = lclFindCharacter( pcNameBeg, pcEnd, '=' );
        if ((bOk = (pcEqualSign < pcEnd)) == true)
        {
            
            const sal_Char* pcNameEnd = lclTrimWhiteSpaceFromEnd( pcNameBeg, pcEqualSign );
            if( (bOk = (pcNameBeg < pcNameEnd)) == true )
            {
                
                const sal_Char* pcValueBeg = lclFindNonWhiteSpace( pcEqualSign + 1, pcEnd );
                if( (bOk = (pcValueBeg < pcEnd) && ((*pcValueBeg == '\'') || (*pcValueBeg == '"'))) == true )
                {
                    
                    const sal_Char* pcValueEnd = lclFindCharacter( pcValueBeg + 1, pcEnd, *pcValueBeg );
                    if( (bOk = (pcValueEnd < pcEnd)) == true )
                    {
                        ++pcValueEnd;
                        OString aAttribName( pcNameBeg, static_cast< sal_Int32 >( pcNameEnd - pcNameBeg ) );
                        OString aAttribData( pcNameBeg, static_cast< sal_Int32 >( pcValueEnd - pcNameBeg ) );
                        
                        AttributeNameMap::iterator aIt = aAttributeNames.find( aAttribName );
                        
                        if( aIt != aAttributeNames.end() )
                            aAttributes.erase( aIt->second );
                        
                        aAttributeNames[ aAttribName ] = pcNameBeg;
                        aAttributes[ pcNameBeg ] = aAttribData;
                        
                        pcNameBeg = pcValueEnd;
                        if( (pcNameBeg < pcEnd) && ((bOk = lclIsWhiteSpace( *pcNameBeg )) == true) )
                            pcNameBeg = lclFindNonWhiteSpace( pcNameBeg + 1, pcEnd );
                    }
                }
            }
        }
    }

    
    if( bOk )
        for( AttributeDataMap::iterator aIt = aAttributes.begin(), aEnd = aAttributes.end(); aIt != aEnd; ++aIt )
            rBuffer.append( ' ' ).append( aIt->second );
    
    else
        lclAppendToBuffer( rBuffer, pcBeg, pcEnd );
}

void lclProcessElement( OStringBuffer& rBuffer, const OString& rElement )
{
    
    sal_Int32 nElementLen = rElement.getLength();
    if( nElementLen == 0 )
        return;

    const sal_Char* pcOpen = rElement.getStr();
    const sal_Char* pcClose = pcOpen + nElementLen - 1;

    
    if( (pcOpen >= pcClose) || (*pcOpen != '<') || (*pcClose != '>') )
    {
        
        rBuffer.append( rElement );
    }

    
    else if( (nElementLen >= 5) && (pcOpen[ 1 ] == '!') && (pcOpen[ 2 ] == '[') && (pcClose[ -1 ] == ']') )
    {
        
    }

    
    else if( (nElementLen >= 4) && (pcOpen[ 1 ] == 'b') && (pcOpen[ 2 ] == 'r') && (lclFindNonWhiteSpace( pcOpen + 3, pcClose ) == pcClose) )
    {
        rBuffer.append( '\n' );
    }

    
    else if( pcOpen[ 1 ] != '/' )
    {
        
        const sal_Char* pcContentBeg = pcOpen + 1;
        bool bIsEmptyElement = pcClose[ -1 ] == '/';
        const sal_Char* pcContentEnd = bIsEmptyElement ? (pcClose - 1) : pcClose;
        
        const sal_Char* pcWhiteSpace = lclFindWhiteSpace( pcContentBeg, pcContentEnd );
        lclAppendToBuffer( rBuffer, pcOpen, pcWhiteSpace );
        
        const sal_Char* pcAttribBeg = lclFindNonWhiteSpace( pcWhiteSpace, pcContentEnd );
        if( pcAttribBeg < pcContentEnd )
            lclProcessAttribs( rBuffer, pcAttribBeg, pcContentEnd );
        
        if( bIsEmptyElement )
            rBuffer.append( '/' );
        rBuffer.append( '>' );
    }

    
    else
    {
        rBuffer.append( rElement );
    }
}

bool lclProcessCharacters( OStringBuffer& rBuffer, const OString& rChars )
{
    /*  MSO has a very weird way to store and handle whitespaces. The stream
        may contain lots of spaces, tabs, and newlines which have to be handled
        as single space character. This will be done in this function.

        If the element text contains a literal line break, it will be stored as
        <br> tag (without matching </br> element). This input stream wrapper
        will replace this element with a literal LF character (see below).

        A single space character for its own is stored as is. Example: The
        element
            <font> </font>
        represents a single space character. The XML parser will ignore this
        space character completely without issuing a 'characters' event. The
        VML import filter implementation has to react on this case manually.

        A single space character following another character is stored
        literally and must not be stipped away here. Example: The element
            <font>abc </font>
        contains the three letters a, b, and c, followed by a space character.

        Consecutive space characters, or a leading single space character, are
        stored in a <span> element. If there are N space characters (N > 1),
        then the <span> element contains exactly (N-1) NBSP (non-breaking
        space) characters, followed by a regular space character. Examples:
        The element
            <font><span style='mso-spacerun:yes'>\xA0\xA0\xA0 </span></font>
        represents 4 consecutive space characters. Has to be handled by the
        implementation. The element
            <font><span style='mso-spacerun:yes'> abc</span></font>
        represents a space characters followed by the letters a, b, c. These
        strings have to be handled by the VML import filter implementation.
     */

    
    const sal_Char* pcBeg = rChars.getStr();
    const sal_Char* pcEnd = pcBeg + rChars.getLength();
    bool bHasBracket = (pcBeg < pcEnd) && (pcEnd[ -1 ] == '<');
    if( bHasBracket ) --pcEnd;

    
    const sal_Char* pcContentsBeg = lclFindNonWhiteSpace( pcBeg, pcEnd );
    while( pcContentsBeg < pcEnd )
    {
        const sal_Char* pcWhitespaceBeg = lclFindWhiteSpace( pcContentsBeg + 1, pcEnd );
        lclAppendToBuffer( rBuffer, pcContentsBeg, pcWhitespaceBeg );
        if( pcWhitespaceBeg < pcEnd )
            rBuffer.append( ' ' );
        pcContentsBeg = lclFindNonWhiteSpace( pcWhitespaceBeg, pcEnd );
    }

    return bHasBracket;
}

} 



InputStream::InputStream( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm ) :
    
    mxTextStrm( TextInputStream::createXTextInputStream( rxContext, rxInStrm, RTL_TEXTENCODING_ISO_8859_1 ) ),
    maOpeningBracket( 1 ),
    maClosingBracket( 1 ),
    maOpeningCData( CREATE_OSTRING( "<![CDATA[" ) ),
    maClosingCData( CREATE_OSTRING( "]]>" ) ),
    mnBufferPos( 0 )
{
    maOpeningBracket[ 0 ] = '<';
    maClosingBracket[ 0 ] = '>';
}

InputStream::~InputStream()
{
}

sal_Int32 SAL_CALL InputStream::readBytes( Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead )
        throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if( nBytesToRead < 0 )
        throw IOException();

    rData.realloc( nBytesToRead );
    sal_Int8* pcDest = rData.getArray();
    sal_Int32 nRet = 0;
    while( (nBytesToRead > 0) && !mxTextStrm->isEOF() )
    {
        updateBuffer();
        sal_Int32 nReadSize = ::std::min( nBytesToRead, maBuffer.getLength() - mnBufferPos );
        if( nReadSize > 0 )
        {
            memcpy( pcDest + nRet, maBuffer.getStr() + mnBufferPos, static_cast< size_t >( nReadSize ) );
            mnBufferPos += nReadSize;
            nBytesToRead -= nReadSize;
            nRet += nReadSize;
        }
    }
    if( nRet < rData.getLength() )
        rData.realloc( nRet );
    return nRet;
}

sal_Int32 SAL_CALL InputStream::readSomeBytes( Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead )
        throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return readBytes( rData, nMaxBytesToRead );
}

void SAL_CALL InputStream::skipBytes( sal_Int32 nBytesToSkip )
        throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if( nBytesToSkip < 0 )
        throw IOException();

    while( (nBytesToSkip > 0) && !mxTextStrm->isEOF() )
    {
        updateBuffer();
        sal_Int32 nSkipSize = ::std::min( nBytesToSkip, maBuffer.getLength() - mnBufferPos );
        mnBufferPos += nSkipSize;
        nBytesToSkip -= nSkipSize;
    }
}

sal_Int32 SAL_CALL InputStream::available() throw (NotConnectedException, IOException, RuntimeException)
{
    updateBuffer();
    return maBuffer.getLength() - mnBufferPos;
}

void SAL_CALL InputStream::closeInput() throw (NotConnectedException, IOException, RuntimeException)
{
    mxTextStrm->closeInput();
}



void InputStream::updateBuffer() throw (IOException, RuntimeException)
{
    while( (mnBufferPos >= maBuffer.getLength()) && !mxTextStrm->isEOF() )
    {
        
        OStringBuffer aBuffer;

        
        OString aChars = readToElementBegin();
        bool bHasOpeningBracket = lclProcessCharacters( aBuffer, aChars );

        
        OSL_ENSURE( bHasOpeningBracket || mxTextStrm->isEOF(), "InputStream::updateBuffer - missing opening bracket of XML element" );
        if( bHasOpeningBracket && !mxTextStrm->isEOF() )
        {
            
            OString aElement = OString( '<' ) + readToElementEnd();
            
            if( aElement.match( maOpeningCData ) )
            {
                
                while( ((aElement.getLength() < maClosingCData.getLength()) || !aElement.match( maClosingCData, aElement.getLength() - maClosingCData.getLength() )) && !mxTextStrm->isEOF() )
                    aElement += readToElementEnd();
                
                aBuffer.append( aElement );
            }
            else
            {
                
                lclProcessElement( aBuffer, aElement );
            }
        }

        maBuffer = aBuffer.makeStringAndClear();
        mnBufferPos = 0;
    }
}

OString InputStream::readToElementBegin() throw (IOException, RuntimeException)
{
    return OUStringToOString( mxTextStrm->readString( maOpeningBracket, sal_False ), RTL_TEXTENCODING_ISO_8859_1 );
}

OString InputStream::readToElementEnd() throw (IOException, RuntimeException)
{
    OString aText = OUStringToOString( mxTextStrm->readString( maClosingBracket, sal_False ), RTL_TEXTENCODING_ISO_8859_1 );
    OSL_ENSURE( aText.endsWith(">"), "InputStream::readToElementEnd - missing closing bracket of XML element" );
    return aText;
}



} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
