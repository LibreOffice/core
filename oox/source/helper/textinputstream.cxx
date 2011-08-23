/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/helper/textinputstream.hxx"
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/binaryinputstream.hxx"

using ::rtl::OStringBuffer;
using ::rtl::OStringToOUString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace oox {

// ============================================================================

namespace {

/** Reads a text line from stream. First, tries to skip the second character of
    a two-character line end sequence. Returns the new line-end character. */
template< typename BufferType, typename CharType, typename StreamDataType >
sal_Unicode lclReadLine( BufferType& orBuffer, BinaryInputStream& rInStrm, sal_Unicode cLastEolChar )
{
    // try to skip LF following CR, or CR following LF
    if( !rInStrm.isEof() && (cLastEolChar != 0) )
    {
        CharType cChar = static_cast< CharType >( rInStrm.readValue< StreamDataType >() );
        // return on EOF after line-end
        if( rInStrm.isEof() )
            return 0;
        // return on sequence of equal line-end characters
        bool bIsEolChar = (cChar == 10) || (cChar == 13);
        if( bIsEolChar && (cChar == cLastEolChar) )
            return cChar;
        // append the character, if it is not the other line-end charcter
        if( !bIsEolChar )
            orBuffer.append( cChar );
    }

    // read chars until EOF or line end character (LF or CR)
    while( true )
    {
        CharType cChar = static_cast< CharType >( rInStrm.readValue< StreamDataType >() );
        if( rInStrm.isEof() )
            return 0;
        if( (cChar == 10) || (cChar == 13) )
            return cChar;
        orBuffer.append( cChar );
    }
}

} // namespace

// ============================================================================

TextInputStream::TextInputStream( BinaryInputStream& rInStrm, rtl_TextEncoding eTextEnc ) :
    mrInStrm( rInStrm ),
    meTextEnc( eTextEnc ),
    mcLastEolChar( 0 )
{
}

bool TextInputStream::isEof() const
{
    // do not return EOF, if last text line missed line-end character (see below)
    return mrInStrm.isEof() && (mcLastEolChar == 0);
}

OUString TextInputStream::readLine()
{
    if( mrInStrm.isEof() )
    {
        mcLastEolChar = 0;
        return OUString();
    }
        
    OUString aLine;
    if( meTextEnc == RTL_TEXTENCODING_UCS2 )
    {
        // read 16-bit characters for UCS2 encoding
        OUStringBuffer aBuffer;
        mcLastEolChar = lclReadLine< OUStringBuffer, sal_Unicode, sal_uInt16 >( aBuffer, mrInStrm, mcLastEolChar );
        aLine = aBuffer.makeStringAndClear();
    }
    else
    {
        // otherwise, read 8-bit characters and convert according to text encoding
        OStringBuffer aBuffer;
        mcLastEolChar = lclReadLine< OStringBuffer, sal_Char, sal_uInt8 >( aBuffer, mrInStrm, mcLastEolChar );
        aLine = OStringToOUString( aBuffer.makeStringAndClear(), meTextEnc );
    }

    // if last line is not empty but line-end character is missing, do not return EOF
    if( mrInStrm.isEof() && (aLine.getLength() > 0) )
        mcLastEolChar = 10;
        
    return aLine;
}

// ============================================================================

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
