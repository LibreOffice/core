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

#include "oox/vml/vmlinputstream.hxx"
#include <map>
#include <rtl/strbuf.hxx>
#include <rtl/strbuf.hxx>
#include "oox/helper/helper.hxx"

using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::io::XInputStream;

namespace oox {
namespace vml {

// ============================================================================

namespace {

inline const sal_Char* lclFindCharacter( const sal_Char* pcBeg, const sal_Char* pcEnd, sal_Char cChar )
{
    sal_Int32 nIndex = rtl_str_indexOfChar_WithLength( pcBeg, static_cast< sal_Int32 >( pcEnd - pcBeg ), cChar );
    return (nIndex < 0) ? pcEnd : (pcBeg + nIndex);
}

inline bool lclIsWhiteSpace( sal_Char cChar )
{
    return (cChar == ' ') || (cChar == '\t') || (cChar == '\n') || (cChar == '\r');
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

// ----------------------------------------------------------------------------

void lclProcessAttribs( OStringBuffer& rBuffer, const sal_Char* pcBeg, const sal_Char* pcEnd )
{
    /*  Map attribute names to char-pointer of all attributes. This map is used
        to find multiple occurences of attributes with the same name. The
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
        // pcNameBeg points to begin of attribute name, find equality sign
        const sal_Char* pcEqualSign = lclFindCharacter( pcNameBeg, pcEnd, '=' );
        if( (bOk = pcEqualSign < pcEnd) == true )
        {
            // find end of attribute name (ignore whitespace between name and equality sign)
            const sal_Char* pcNameEnd = lclTrimWhiteSpaceFromEnd( pcNameBeg, pcEqualSign );
            if( (bOk = pcNameBeg < pcNameEnd) == true )
            {
                // find begin of attribute value (must be single or double quote)
                const sal_Char* pcValueBeg = lclFindNonWhiteSpace( pcEqualSign + 1, pcEnd );
                if( (bOk = (pcValueBeg < pcEnd) && ((*pcValueBeg == '\'') || (*pcValueBeg == '"'))) == true )
                {
                    // find end of attribute value (matching quote character)
                    const sal_Char* pcValueEnd = lclFindCharacter( pcValueBeg + 1, pcEnd, *pcValueBeg );
                    if( (bOk = pcValueEnd < pcEnd) == true )
                    {
                        ++pcValueEnd;
                        OString aAttribName( pcNameBeg, static_cast< sal_Int32 >( pcNameEnd - pcNameBeg ) );
                        OString aAttribData( pcNameBeg, static_cast< sal_Int32 >( pcValueEnd - pcNameBeg ) );
                        // search for an existing attribute with the same name
                        AttributeNameMap::iterator aIt = aAttributeNames.find( aAttribName );
                        // remove its definition from the data map
                        if( aIt != aAttributeNames.end() )
                            aAttributes.erase( aIt->second );
                        // insert the attribute into both maps
                        aAttributeNames[ aAttribName ] = pcNameBeg;
                        aAttributes[ pcNameBeg ] = aAttribData;
                        // continue with next attribute (skip whitespace after this attribute)
                        pcNameBeg = pcValueEnd;
                        if( (pcNameBeg < pcEnd) && ((bOk = lclIsWhiteSpace( *pcNameBeg )) == true) )
                            pcNameBeg = lclFindNonWhiteSpace( pcNameBeg + 1, pcEnd );
                    }
                }
            }
        }
    }

    // if no error has occured, build the resulting attribute list
    if( bOk )
        for( AttributeDataMap::iterator aIt = aAttributes.begin(), aEnd = aAttributes.end(); aIt != aEnd; ++aIt )
            rBuffer.append( ' ' ).append( aIt->second );
    // on error, just append the complete passed string
    else
        lclAppendToBuffer( rBuffer, pcBeg, pcEnd );
}

} // namespace

// ============================================================================

StreamDataContainer::StreamDataContainer( const Reference< XInputStream >& rxInStrm )
{
    if( rxInStrm.is() ) try
    {
        // read all bytes we can read
        rxInStrm->readBytes( maDataSeq, SAL_MAX_INT32 );
    }
    catch( Exception& )
    {
    }

    if( maDataSeq.hasElements() )
    {
        const OString aCDataOpen = CREATE_OSTRING( "<![CDATA[" );
        const OString aCDataClose = CREATE_OSTRING( "]]>" );
        
        OStringBuffer aBuffer;
        aBuffer.ensureCapacity( maDataSeq.getLength() + 256 );
        const sal_Char* pcCurr = reinterpret_cast< const sal_Char* >( maDataSeq.getConstArray() );
        const sal_Char* pcEnd = pcCurr + maDataSeq.getLength();
        while( pcCurr < pcEnd )
        {
            // look for the next opening angle bracket
            const sal_Char* pcOpen = lclFindCharacter( pcCurr, pcEnd, '<' );
            // copy all characters from current position to opening bracket
            lclAppendToBuffer( aBuffer, pcCurr, pcOpen );
            
            // nothing to do if no opening bracket has been found
            if( pcOpen < pcEnd )
            {
                // string length from opening bracket to end
                sal_Int32 nLengthToEnd = static_cast< sal_Int32 >( pcEnd - pcOpen );

                // check for CDATA part, starting with '<![CDATA['
                if( rtl_str_compare_WithLength( pcOpen, nLengthToEnd, aCDataOpen.getStr(), aCDataOpen.getLength() ) == 0 )
                {
                    // search the position after the end tag ']]>'
                    sal_Int32 nClosePos = rtl_str_indexOfStr_WithLength( pcOpen, nLengthToEnd, aCDataClose.getStr(), aCDataClose.getLength() );
                    pcCurr = (nClosePos < 0) ? pcEnd : (pcOpen + nClosePos + aCDataClose.getLength());
                    // copy the entire CDATA part
                    lclAppendToBuffer( aBuffer, pcOpen, pcCurr );
                }

                // no CDATA part - process the element starting at pcOpen
                else
                {
                    // look for the next closing angle bracket
                    const sal_Char* pcClose = lclFindCharacter( pcOpen + 1, pcEnd, '>' );
                    // complete element found?
                    if( pcClose < pcEnd )
                    {
                        // continue after closing bracket
                        pcCurr = pcClose + 1;
                        // length of entire element with angle brackets
                        sal_Int32 nElementLen = static_cast< sal_Int32 >( pcCurr - pcOpen );

                        // skip parser instructions: '<![...]>'
                        if( (nElementLen >= 5) && (pcOpen[ 1 ] == '!') && (pcOpen[ 2 ] == '[') && (pcClose[ -1 ] == ']') )
                        {
                            // do nothing
                        }

                        // replace '<br>' elements with '<br/>' elements
                        else if( (nElementLen >= 4) && (pcOpen[ 1 ] == 'b') && (pcOpen[ 2 ] == 'r') && (lclFindNonWhiteSpace( pcOpen + 3, pcClose ) == pcClose) )
                        {
                            aBuffer.append( RTL_CONSTASCII_STRINGPARAM( "<br/>" ) );
                        }

                        // check start elements and empty elements for repeated attributes
                        else if( pcOpen[ 1 ] != '/' )
                        {
                            // find positions of text content inside brackets, exclude '/' in '<emptyelement/>'
                            const sal_Char* pcContentBeg = pcOpen + 1;
                            bool bIsEmptyElement = pcClose[ -1 ] == '/';
                            const sal_Char* pcContentEnd = bIsEmptyElement ? (pcClose - 1) : pcClose;
                            // append element name to buffer
                            const sal_Char* pcWhiteSpace = lclFindWhiteSpace( pcContentBeg, pcContentEnd );
                            lclAppendToBuffer( aBuffer, pcOpen, pcWhiteSpace );
                            // find begin of attributes, and process all attributes
                            const sal_Char* pcAttribBeg = lclFindNonWhiteSpace( pcWhiteSpace, pcContentEnd );
                            if( pcAttribBeg < pcContentEnd )
                                lclProcessAttribs( aBuffer, pcAttribBeg, pcContentEnd );
                            // close the element
                            if( bIsEmptyElement )
                                aBuffer.append( '/' );
                            aBuffer.append( '>' );
                        }

                        // append end elements without further processing
                        else
                        {
                            lclAppendToBuffer( aBuffer, pcOpen, pcCurr );
                        }
                    }
                    else
                    {
                        // no complete element found, copy all from opening bracket to end
                        lclAppendToBuffer( aBuffer, pcOpen, pcEnd );
                        pcCurr = pcEnd;
                    }
                }
            }
        }

        // set the final data sequence
        maDataSeq = ::comphelper::ByteSequence( reinterpret_cast< const sal_Int8* >( aBuffer.getStr() ), aBuffer.getLength() );
    }
}

// ============================================================================

InputStream::InputStream( const Reference< XInputStream >& rxInStrm ) :
    StreamDataContainer( rxInStrm ),
    ::comphelper::SequenceInputStream( maDataSeq )
{
}

InputStream::~InputStream()
{
}

// ============================================================================

} // namespace vml
} // namespave oox

