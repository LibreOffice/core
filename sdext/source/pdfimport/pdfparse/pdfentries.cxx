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


#include <pdfparse.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/alloc.h>
#include <rtl/digest.h>
#include <rtl/cipher.h>
#include <rtl/memory.h>
#ifdef SYSTEM_ZLIB
#include "zlib.h"
#else
#include <zlib/zlib.h>
#endif

#include <math.h>
#include <map>

#include <stdio.h>
#include <string.h>

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringHash;
using ::rtl::OStringBuffer;


namespace pdfparse
{

struct EmitImplData
{
    // xref table: maps object number to a pair of (generation, buffer offset)
    typedef std::map< unsigned int, std::pair< unsigned int, unsigned int > > XRefTable;
    XRefTable m_aXRefTable;
    // container of all indirect objects (usually a PDFFile*)
    const PDFContainer* m_pObjectContainer;
    unsigned int m_nDecryptObject;
    unsigned int m_nDecryptGeneration;

    // returns true if the xref table was updated
    bool insertXref( unsigned int nObject, unsigned int nGeneration, unsigned int nOffset )
    {
        XRefTable::iterator it = m_aXRefTable.find( nObject );
        if( it == m_aXRefTable.end() )
        {
            // new entry
            m_aXRefTable[ nObject ] = std::pair<unsigned int, unsigned int>(nGeneration,nOffset);
            return true;
        }
        // update old entry, if generation number is higher
        if( it->second.first < nGeneration )
        {
            it->second = std::pair<unsigned int, unsigned int>(nGeneration,nOffset);
            return true;
        }
        return false;
    }

    EmitImplData( const PDFContainer* pTopContainer ) :
        m_pObjectContainer( pTopContainer ),
        m_nDecryptObject( 0 ),
        m_nDecryptGeneration( 0 )
    {}
    ~EmitImplData() {}
    bool decrypt( const sal_uInt8* pInBuffer, sal_uInt32 nLen, sal_uInt8* pOutBuffer,
                  unsigned int nObject, unsigned int nGeneration ) const
    {
        const PDFFile* pFile = dynamic_cast<const PDFFile*>(m_pObjectContainer);
        return pFile ? pFile->decrypt( pInBuffer, nLen, pOutBuffer, nObject, nGeneration ) : false;
    }

    void setDecryptObject( unsigned int nObject, unsigned int nGeneration )
    {
        m_nDecryptObject = nObject;
        m_nDecryptGeneration = nGeneration;
    }
};

}

using namespace pdfparse;

EmitContext::EmitContext( const PDFContainer* pTop ) :
    m_bDeflate( false ),
    m_bDecrypt( false ),
    m_pImplData( NULL )
{
    if( pTop )
        m_pImplData = new EmitImplData( pTop );
}

EmitContext::~EmitContext()
{
    delete m_pImplData;
}

PDFEntry::~PDFEntry()
{
}

EmitImplData* PDFEntry::getEmitData( EmitContext& rContext ) const
{
    return rContext.m_pImplData;
}

void PDFEntry::setEmitData( EmitContext& rContext, EmitImplData* pNewEmitData ) const
{
    if( rContext.m_pImplData && rContext.m_pImplData != pNewEmitData )
        delete rContext.m_pImplData;
    rContext.m_pImplData = pNewEmitData;
}

PDFValue::~PDFValue()
{
}

PDFComment::~PDFComment()
{
}

bool PDFComment::emit( EmitContext& rWriteContext ) const
{
    return rWriteContext.write( m_aComment.getStr(), m_aComment.getLength() );
}

PDFEntry* PDFComment::clone() const
{
    return new PDFComment( m_aComment );
}

PDFName::~PDFName()
{
}

bool PDFName::emit( EmitContext& rWriteContext ) const
{
    if( ! rWriteContext.write( " /", 2 ) )
        return false;
    return rWriteContext.write( m_aName.getStr(), m_aName.getLength() );
}

PDFEntry* PDFName::clone() const
{
    return new PDFName( m_aName );
}

OUString PDFName::getFilteredName() const
{
    OStringBuffer aFilter( m_aName.getLength() );
    const sal_Char* pStr = m_aName.getStr();
    unsigned int nLen = m_aName.getLength();
    for( unsigned int i = 0; i < nLen; i++ )
    {
        if( (i < nLen - 3) && pStr[i] == '#' )
        {
            sal_Char rResult = 0;
            i++;
            if( pStr[i] >= '0' && pStr[i] <= '9' )
                rResult = sal_Char( pStr[i]-'0' ) << 4;
            else if( pStr[i] >= 'a' && pStr[i] <= 'f' )
                rResult = sal_Char( pStr[i]-'a' + 10 ) << 4;
            else if( pStr[i] >= 'A' && pStr[i] <= 'F' )
                rResult = sal_Char( pStr[i]-'A' + 10 ) << 4;
            i++;
            if( pStr[i] >= '0' && pStr[i] <= '9' )
                rResult |= sal_Char( pStr[i]-'0' );
            else if( pStr[i] >= 'a' && pStr[i] <= 'f' )
                rResult |= sal_Char( pStr[i]-'a' + 10 );
            else if( pStr[i] >= 'A' && pStr[i] <= 'F' )
                rResult |= sal_Char( pStr[i]-'A' + 10 );
            aFilter.append( rResult );
        }
        else
            aFilter.append( pStr[i] );
    }
    return OStringToOUString( aFilter.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
}

PDFString::~PDFString()
{
}

bool PDFString::emit( EmitContext& rWriteContext ) const
{
    if( ! rWriteContext.write( " ", 1 ) )
        return false;
    EmitImplData* pEData = getEmitData( rWriteContext );
    if( rWriteContext.m_bDecrypt && pEData && pEData->m_nDecryptObject )
    {
        OString aFiltered( getFilteredString() );
        // decrypt inplace (evil since OString is supposed to be const
        // however in this case we know that getFilteredString returned a singular string instance
        pEData->decrypt( (sal_uInt8*)aFiltered.getStr(), aFiltered.getLength(),
                         (sal_uInt8*)aFiltered.getStr(),
                         pEData->m_nDecryptObject, pEData->m_nDecryptGeneration );
        // check for string or hex string
        const sal_Char* pStr = aFiltered.getStr();
        if( aFiltered.getLength() > 1 &&
           ( ((unsigned char)pStr[0] == 0xff && (unsigned char)pStr[1] == 0xfe) ||
             ((unsigned char)pStr[0] == 0xfe && (unsigned char)pStr[1] == 0xff) ) )
        {
            static const char pHexTab[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            if( ! rWriteContext.write( "<", 1 ) )
                return false;
            for( sal_Int32 i = 0; i < aFiltered.getLength(); i++ )
            {
                if( ! rWriteContext.write( pHexTab + ((sal_uInt32(pStr[i]) >> 4) & 0x0f), 1 ) )
                    return false;
                if( ! rWriteContext.write( pHexTab + (sal_uInt32(pStr[i]) & 0x0f), 1 ) )
                    return false;
            }
            if( ! rWriteContext.write( ">", 1 ) )
                return false;
        }
        else
        {
            if( ! rWriteContext.write( "(", 1 ) )
                return false;
            if( ! rWriteContext.write( aFiltered.getStr(), aFiltered.getLength() ) )
                return false;
            if( ! rWriteContext.write( ")", 1 ) )
                return false;
        }
        return true;
    }
    return rWriteContext.write( m_aString.getStr(), m_aString.getLength() );
}

PDFEntry* PDFString::clone() const
{
    return new PDFString( m_aString );
}

OString PDFString::getFilteredString() const
{
    int nLen = m_aString.getLength();
    OStringBuffer aBuf( nLen );

    const sal_Char* pStr = m_aString.getStr();
    if( *pStr == '(' )
    {
        const sal_Char* pRun = pStr+1;
        while( pRun - pStr < nLen-1 )
        {
            if( *pRun == '\\' )
            {
                pRun++;
                if( pRun - pStr < nLen )
                {
                    sal_Char aEsc = 0;
                    if( *pRun == 'n' )
                        aEsc = '\n';
                    else if( *pRun == 'r' )
                        aEsc = '\r';
                    else if( *pRun == 't' )
                        aEsc = '\t';
                    else if( *pRun == 'b' )
                        aEsc = '\b';
                    else if( *pRun == 'f' )
                        aEsc = '\f';
                    else if( *pRun == '(' )
                        aEsc = '(';
                    else if( *pRun == ')' )
                        aEsc = ')';
                    else if( *pRun == '\\' )
                        aEsc = '\\';
                    else if( *pRun == '\n' )
                    {
                        pRun++;
                        continue;
                    }
                    else if( *pRun == '\r' )
                    {
                        pRun++;
                        if( *pRun == '\n' )
                            pRun++;
                        continue;
                    }
                    else
                    {
                        int i = 0;
                        while( i++ < 3 && *pRun >= '0' && *pRun <= '7' )
                            aEsc = 8*aEsc + (*pRun++ - '0');
                        // move pointer back to last character of octal sequence
                        pRun--;
                    }
                    aBuf.append( aEsc );
                }
            }
            else
                aBuf.append( *pRun );
            // move pointer to next character
            pRun++;
        }
    }
    else if( *pStr == '<' )
    {
        const sal_Char* pRun = pStr+1;
        while( *pRun != '>' && pRun - pStr < nLen )
        {
            sal_Char rResult = 0;
            if( *pRun >= '0' && *pRun <= '9' )
                rResult = sal_Char( *pRun-'0' ) << 4;
            else if( *pRun >= 'a' && *pRun <= 'f' )
                rResult = sal_Char( *pRun-'a' + 10 ) << 4;
            else if( *pRun >= 'A' && *pRun <= 'F' )
                rResult = sal_Char( *pRun-'A' + 10 ) << 4;
            pRun++;
            if( *pRun != '>' && pRun - pStr < nLen )
            {
                if( *pRun >= '0' && *pRun <= '9' )
                    rResult |= sal_Char( *pRun-'0' );
                else if( *pRun >= 'a' && *pRun <= 'f' )
                    rResult |= sal_Char( *pRun-'a' + 10 );
                else if( *pRun >= 'A' && *pRun <= 'F' )
                    rResult |= sal_Char( *pRun-'A' + 10 );
            }
            pRun++;
            aBuf.append( rResult );
        }
    }

    return aBuf.makeStringAndClear();
}

PDFNumber::~PDFNumber()
{
}

bool PDFNumber::emit( EmitContext& rWriteContext ) const
{
    rtl::OStringBuffer aBuf( 32 );
    aBuf.append( ' ' );

    double fValue = m_fValue;
    bool bNeg = false;
    int nPrecision = 5;
    if( fValue < 0.0 )
    {
        bNeg = true;
        fValue=-fValue;
    }

    sal_Int64 nInt = (sal_Int64)fValue;
    fValue -= (double)nInt;
    // optimizing hardware may lead to a value of 1.0 after the subtraction
    if( fValue == 1.0 || log10( 1.0-fValue ) <= -nPrecision )
    {
        nInt++;
        fValue = 0.0;
    }
    sal_Int64 nFrac = 0;
    if( fValue )
    {
        fValue *= pow( 10.0, (double)nPrecision );
        nFrac = (sal_Int64)fValue;
    }
    if( bNeg && ( nInt || nFrac ) )
        aBuf.append( '-' );
    aBuf.append( nInt );
    if( nFrac )
    {
        int i;
        aBuf.append( '.' );
        sal_Int64 nBound = (sal_Int64)(pow( 10.0, nPrecision - 1.0 )+0.5);
        for ( i = 0; ( i < nPrecision ) && nFrac; i++ )
        {
            sal_Int64 nNumb = nFrac / nBound;
            nFrac -= nNumb * nBound;
            aBuf.append( nNumb );
            nBound /= 10;
        }
    }

    return rWriteContext.write( aBuf.getStr(), aBuf.getLength() );
}

PDFEntry* PDFNumber::clone() const
{
    return new PDFNumber( m_fValue );
}


PDFBool::~PDFBool()
{
}

bool PDFBool::emit( EmitContext& rWriteContext ) const
{
    return m_bValue ? rWriteContext.write( " true", 5 ) : rWriteContext.write( " false", 6 );
}

PDFEntry* PDFBool::clone() const
{
    return new PDFBool( m_bValue );
}

PDFNull::~PDFNull()
{
}

bool PDFNull::emit( EmitContext& rWriteContext ) const
{
    return rWriteContext.write( " null", 5 );
}

PDFEntry* PDFNull::clone() const
{
    return new PDFNull();
}


PDFObjectRef::~PDFObjectRef()
{
}

bool PDFObjectRef::emit( EmitContext& rWriteContext ) const
{
    OStringBuffer aBuf( 16 );
    aBuf.append( ' ' );
    aBuf.append( sal_Int32( m_nNumber ) );
    aBuf.append( ' ' );
    aBuf.append( sal_Int32( m_nGeneration ) );
    aBuf.append( " R", 2 );
    return rWriteContext.write( aBuf.getStr(), aBuf.getLength() );
}

PDFEntry* PDFObjectRef::clone() const
{
    return new PDFObjectRef( m_nNumber, m_nGeneration );
}

PDFContainer::~PDFContainer()
{
    int nEle = m_aSubElements.size();
    for( int i = 0; i < nEle; i++ )
        delete m_aSubElements[i];
}

bool PDFContainer::emitSubElements( EmitContext& rWriteContext ) const
{
    int nEle = m_aSubElements.size();
    for( int i = 0; i < nEle; i++ )
    {
        if( rWriteContext.m_bDecrypt )
        {
            const PDFName* pName = dynamic_cast<PDFName*>(m_aSubElements[i]);
            if (pName && pName->m_aName.equalsL(RTL_CONSTASCII_STRINGPARAM("Encrypt")))
            {
                i++;
                continue;
            }
        }
        if( ! m_aSubElements[i]->emit( rWriteContext ) )
            return false;
    }
    return true;
}

void PDFContainer::cloneSubElements( std::vector<PDFEntry*>& rNewSubElements ) const
{
    int nEle = m_aSubElements.size();
    for( int i = 0; i < nEle; i++ )
        rNewSubElements.push_back( m_aSubElements[i]->clone() );
}

PDFObject* PDFContainer::findObject( unsigned int nNumber, unsigned int nGeneration ) const
{
    unsigned int nEle = m_aSubElements.size();
    for( unsigned int i = 0; i < nEle; i++ )
    {
        PDFObject* pObject = dynamic_cast<PDFObject*>(m_aSubElements[i]);
        if( pObject &&
            pObject->m_nNumber == nNumber &&
            pObject->m_nGeneration == nGeneration )
        {
            return pObject;
        }
    }
    return NULL;
}

PDFArray::~PDFArray()
{
}

bool PDFArray::emit( EmitContext& rWriteContext ) const
{
    if( ! rWriteContext.write( "[", 1 ) )
        return false;
    if( ! emitSubElements( rWriteContext ) )
        return false;
    return rWriteContext.write( "]", 1 );
}

PDFEntry* PDFArray::clone() const
{
    PDFArray* pNewAr = new PDFArray();
    cloneSubElements( pNewAr->m_aSubElements );
    return pNewAr;
}

PDFDict::~PDFDict()
{
}

bool PDFDict::emit( EmitContext& rWriteContext ) const
{
    if( ! rWriteContext.write( "<<\n", 3 ) )
        return false;
    if( ! emitSubElements( rWriteContext ) )
        return false;
    return rWriteContext.write( "\n>>\n", 4 );
}

void PDFDict::insertValue( const OString& rName, PDFEntry* pValue )
{
    if( ! pValue )
        eraseValue( rName );

    boost::unordered_map<OString,PDFEntry*,OStringHash>::iterator it = m_aMap.find( rName );
    if( it == m_aMap.end() )
    {
        // new name/value, pair, append it
        m_aSubElements.push_back( new PDFName( rName ) );
        m_aSubElements.push_back( pValue );
    }
    else
    {
        unsigned int nSub = m_aSubElements.size();
        for( unsigned int i = 0; i < nSub; i++ )
            if( m_aSubElements[i] == it->second )
                m_aSubElements[i] = pValue;
        delete it->second;
    }
    m_aMap[ rName ] = pValue;
}

void PDFDict::eraseValue( const OString& rName )
{
    unsigned int nEle = m_aSubElements.size();
    for( unsigned int i = 0; i < nEle; i++ )
    {
        PDFName* pName = dynamic_cast<PDFName*>(m_aSubElements[i]);
        if( pName && pName->m_aName.equals( rName ) )
        {
            for( unsigned int j = i+1; j < nEle; j++ )
            {
                if( dynamic_cast<PDFComment*>(m_aSubElements[j]) == NULL )
                {
                    // free name and value
                    delete m_aSubElements[j];
                    delete m_aSubElements[i];
                    // remove subelements from vector
                    m_aSubElements.erase( m_aSubElements.begin()+j );
                    m_aSubElements.erase( m_aSubElements.begin()+i );
                    buildMap();
                    return;
                }
            }
        }
    }
}

PDFEntry* PDFDict::buildMap()
{
    // clear map
    m_aMap.clear();
    // build map
    unsigned int nEle = m_aSubElements.size();
    PDFName* pName = NULL;
    for( unsigned int i = 0; i < nEle; i++ )
    {
        if( dynamic_cast<PDFComment*>(m_aSubElements[i]) == NULL )
        {
            if( pName )
            {
                m_aMap[ pName->m_aName ] = m_aSubElements[i];
                pName = NULL;
            }
            else if( (pName = dynamic_cast<PDFName*>(m_aSubElements[i])) == NULL )
                return m_aSubElements[i];
        }
    }
    return pName;
}

PDFEntry* PDFDict::clone() const
{
    PDFDict* pNewDict = new PDFDict();
    cloneSubElements( pNewDict->m_aSubElements );
    pNewDict->buildMap();
    return pNewDict;
}

PDFStream::~PDFStream()
{
}

bool PDFStream::emit( EmitContext& rWriteContext ) const
{
    return rWriteContext.copyOrigBytes( m_nBeginOffset, m_nEndOffset-m_nBeginOffset );
}

PDFEntry* PDFStream::clone() const
{
    return new PDFStream( m_nBeginOffset, m_nEndOffset, NULL );
}

unsigned int PDFStream::getDictLength( const PDFContainer* pContainer ) const
{
    if( ! m_pDict )
        return 0;
    // find /Length entry, can either be a direct or indirect number object
    boost::unordered_map<OString,PDFEntry*,OStringHash>::const_iterator it =
        m_pDict->m_aMap.find( "Length" );
    if( it == m_pDict->m_aMap.end() )
        return 0;
    PDFNumber* pNum = dynamic_cast<PDFNumber*>(it->second);
    if( ! pNum && pContainer )
    {
        PDFObjectRef* pRef = dynamic_cast<PDFObjectRef*>(it->second);
        if( pRef )
        {
            int nEle = pContainer->m_aSubElements.size();
            for( int i = 0; i < nEle && ! pNum; i++ )
            {
                PDFObject* pObj = dynamic_cast<PDFObject*>(pContainer->m_aSubElements[i]);
                if( pObj &&
                    pObj->m_nNumber == pRef->m_nNumber &&
                    pObj->m_nGeneration == pRef->m_nGeneration )
                {
                    if( pObj->m_pObject )
                        pNum = dynamic_cast<PDFNumber*>(pObj->m_pObject);
                    break;
                }
            }
        }
    }
    return pNum ? static_cast<unsigned int>(pNum->m_fValue) : 0;
}

PDFObject::~PDFObject()
{
}

bool PDFObject::getDeflatedStream( char** ppStream, unsigned int* pBytes, const PDFContainer* pObjectContainer, EmitContext& rContext ) const
{
    bool bIsDeflated = false;
    if( m_pStream && m_pStream->m_pDict &&
        m_pStream->m_nEndOffset > m_pStream->m_nBeginOffset+15
        )
    {
        unsigned int nOuterStreamLen = m_pStream->m_nEndOffset - m_pStream->m_nBeginOffset;
        *ppStream = static_cast<char*>(rtl_allocateMemory( nOuterStreamLen ));
        if( ! ppStream )
        {
            *pBytes = 0;
            return false;
        }
        unsigned int nRead = rContext.readOrigBytes( m_pStream->m_nBeginOffset, nOuterStreamLen, *ppStream );
        if( nRead != nOuterStreamLen )
        {
            rtl_freeMemory( *ppStream );
            *ppStream = NULL;
            *pBytes = 0;
            return false;
        }
        // is there a filter entry ?
        boost::unordered_map<OString,PDFEntry*,OStringHash>::const_iterator it =
            m_pStream->m_pDict->m_aMap.find( "Filter" );
        if( it != m_pStream->m_pDict->m_aMap.end() )
        {
            PDFName* pFilter = dynamic_cast<PDFName*>(it->second);
            if( ! pFilter )
            {
                PDFArray* pArray = dynamic_cast<PDFArray*>(it->second);
                if( pArray && ! pArray->m_aSubElements.empty() )
                {
                    pFilter = dynamic_cast<PDFName*>(pArray->m_aSubElements.front());
                }
            }

            // is the (first) filter FlateDecode ?
            if (pFilter && pFilter->m_aName.equalsL(RTL_CONSTASCII_STRINGPARAM("FlateDecode")))
            {
                bIsDeflated = true;
            }
        }
        // prepare compressed data section
        char* pStream = *ppStream;
        if( pStream[0] == 's' )
            pStream += 6; // skip "stream"
        // skip line end after "stream"
        while( *pStream == '\r' || *pStream == '\n' )
            pStream++;
        // get the compressed length
        *pBytes = m_pStream->getDictLength( pObjectContainer );
        if( pStream != *ppStream )
            rtl_moveMemory( *ppStream, pStream, *pBytes );
        if( rContext.m_bDecrypt )
        {
            EmitImplData* pEData = getEmitData( rContext );
            pEData->decrypt( reinterpret_cast<const sal_uInt8*>(*ppStream),
                             *pBytes,
                             reinterpret_cast<sal_uInt8*>(*ppStream),
                             m_nNumber,
                             m_nGeneration
                             ); // decrypt inplace
        }
    }
    else
        *ppStream = NULL, *pBytes = 0;
    return bIsDeflated;
}

static void unzipToBuffer( const char* pBegin, unsigned int nLen,
                           sal_uInt8** pOutBuf, sal_uInt32* pOutLen )
{
    z_stream aZStr;
    aZStr.next_in       = (Bytef*)pBegin;
    aZStr.avail_in      = nLen;
    aZStr.zalloc        = ( alloc_func )0;
    aZStr.zfree         = ( free_func )0;
    aZStr.opaque        = ( voidpf )0;
    inflateInit(&aZStr);

    const unsigned int buf_increment_size = 16384;

    *pOutBuf = (sal_uInt8*)rtl_reallocateMemory( *pOutBuf, buf_increment_size );
    aZStr.next_out      = (Bytef*)*pOutBuf;
    aZStr.avail_out     = buf_increment_size;
    int err = Z_OK;
    *pOutLen = buf_increment_size;
    while( err != Z_STREAM_END && err >= Z_OK && aZStr.avail_in )
    {
        err = inflate( &aZStr, Z_NO_FLUSH );
        if( aZStr.avail_out == 0 )
        {
            if( err != Z_STREAM_END )
            {
                const int nNewAlloc = *pOutLen + buf_increment_size;
                *pOutBuf = (sal_uInt8*)rtl_reallocateMemory( *pOutBuf, nNewAlloc );
                aZStr.next_out = (Bytef*)(*pOutBuf + *pOutLen);
                aZStr.avail_out = buf_increment_size;
                *pOutLen = nNewAlloc;
            }
        }
    }
    if( err == Z_STREAM_END )
    {
        if( aZStr.avail_out > 0 )
            *pOutLen -= aZStr.avail_out;;
    }
    inflateEnd(&aZStr);
    if( err < Z_OK )
    {
        rtl_freeMemory( *pOutBuf );
        *pOutBuf = NULL;
        *pOutLen = 0;
    }
}

bool PDFObject::writeStream( EmitContext& rWriteContext, const PDFFile* pParsedFile ) const
{
    bool bSuccess = false;
    if( m_pStream )
    {
        char* pStream = NULL;
        unsigned int nBytes = 0;
        if( getDeflatedStream( &pStream, &nBytes, pParsedFile, rWriteContext ) && nBytes && rWriteContext.m_bDeflate )
        {
            sal_uInt8* pOutBytes = NULL;
            sal_uInt32 nOutBytes = 0;
            unzipToBuffer( pStream, nBytes, &pOutBytes, &nOutBytes );
            rWriteContext.write( pOutBytes, nOutBytes );
            rtl_freeMemory( pOutBytes );
        }
        else if( pStream && nBytes )
            rWriteContext.write( pStream, nBytes );
        rtl_freeMemory( pStream );
    }
    return bSuccess;
}

bool PDFObject::emit( EmitContext& rWriteContext ) const
{
    if( ! rWriteContext.write( "\n", 1 ) )
        return false;

    EmitImplData* pEData = getEmitData( rWriteContext );
    if( pEData )
        pEData->insertXref( m_nNumber, m_nGeneration, rWriteContext.getCurPos() );

    OStringBuffer aBuf( 32 );
    aBuf.append( sal_Int32( m_nNumber ) );
    aBuf.append( ' ' );
    aBuf.append( sal_Int32( m_nGeneration ) );
    aBuf.append( " obj\n" );
    if( ! rWriteContext.write( aBuf.getStr(), aBuf.getLength() ) )
        return false;

    if( pEData )
        pEData->setDecryptObject( m_nNumber, m_nGeneration );
    if( (rWriteContext.m_bDeflate || rWriteContext.m_bDecrypt) && pEData )
    {
        char* pStream = NULL;
        unsigned int nBytes = 0;
        bool bDeflate = getDeflatedStream( &pStream, &nBytes, pEData->m_pObjectContainer, rWriteContext );
        if( pStream && nBytes )
        {
            // unzip the stream
            sal_uInt8* pOutBytes = NULL;
            sal_uInt32 nOutBytes = 0;
            if( bDeflate && rWriteContext.m_bDeflate )
                unzipToBuffer( pStream, nBytes, &pOutBytes, &nOutBytes );
            else
            {
                // nothing to deflate, but decryption has happened
                pOutBytes = (sal_uInt8*)pStream;
                nOutBytes = (sal_uInt32)nBytes;
            }

            if( nOutBytes )
            {
                // clone this object
                PDFObject* pClone = static_cast<PDFObject*>(clone());
                // set length in the dictionary to new stream length
                PDFNumber* pNewLen = new PDFNumber( double(nOutBytes) );
                pClone->m_pStream->m_pDict->insertValue( "Length", pNewLen );

                if( bDeflate && rWriteContext.m_bDeflate )
                {
                    // delete flatedecode filter
                    boost::unordered_map<OString,PDFEntry*,OStringHash>::const_iterator it =
                    pClone->m_pStream->m_pDict->m_aMap.find( "Filter" );
                    if( it != pClone->m_pStream->m_pDict->m_aMap.end() )
                    {
                        PDFName* pFilter = dynamic_cast<PDFName*>(it->second);
                        if (pFilter && pFilter->m_aName.equalsL(RTL_CONSTASCII_STRINGPARAM("FlateDecode")))
                            pClone->m_pStream->m_pDict->eraseValue( "Filter" );
                        else
                        {
                            PDFArray* pArray = dynamic_cast<PDFArray*>(it->second);
                            if( pArray && ! pArray->m_aSubElements.empty() )
                            {
                                pFilter = dynamic_cast<PDFName*>(pArray->m_aSubElements.front());
                                if (pFilter && pFilter->m_aName.equalsL(RTL_CONSTASCII_STRINGPARAM("FlateDecode")))
                                {
                                    delete pFilter;
                                    pArray->m_aSubElements.erase( pArray->m_aSubElements.begin() );
                                }
                            }
                        }
                    }
                }

                // write sub elements except stream
                bool bRet = true;
                unsigned int nEle = pClone->m_aSubElements.size();
                for( unsigned int i = 0; i < nEle && bRet; i++ )
                {
                    if( pClone->m_aSubElements[i] != pClone->m_pStream )
                        bRet = pClone->m_aSubElements[i]->emit( rWriteContext );
                }
                delete pClone;
                // write stream
                if( bRet )
                    rWriteContext.write( "stream\n", 7 );
                if( bRet )
                    bRet = rWriteContext.write( pOutBytes, nOutBytes );
                if( bRet )
                    bRet = rWriteContext.write( "\nendstream\nendobj\n", 18 );
                rtl_freeMemory( pStream );
                if( pOutBytes != (sal_uInt8*)pStream )
                    rtl_freeMemory( pOutBytes );
                if( pEData )
                    pEData->setDecryptObject( 0, 0 );
                return bRet;
            }
            if( pOutBytes != (sal_uInt8*)pStream )
                rtl_freeMemory( pOutBytes );
        }
        rtl_freeMemory( pStream );
    }

    bool bRet = emitSubElements( rWriteContext ) &&
                rWriteContext.write( "\nendobj\n", 8 );
    if( pEData )
        pEData->setDecryptObject( 0, 0 );
    return bRet;
}

PDFEntry* PDFObject::clone() const
{
    PDFObject* pNewOb = new PDFObject( m_nNumber, m_nGeneration );
    cloneSubElements( pNewOb->m_aSubElements );
    unsigned int nEle = m_aSubElements.size();
    for( unsigned int i = 0; i < nEle; i++ )
    {
        if( m_aSubElements[i] == m_pObject )
            pNewOb->m_pObject = pNewOb->m_aSubElements[i];
        else if( m_aSubElements[i] == m_pStream && pNewOb->m_pObject )
        {
            pNewOb->m_pStream = dynamic_cast<PDFStream*>(pNewOb->m_aSubElements[i]);
            PDFDict* pNewDict = dynamic_cast<PDFDict*>(pNewOb->m_pObject);
            if( pNewDict )
                pNewOb->m_pStream->m_pDict = pNewDict;
        }
    }
    return pNewOb;
}

PDFTrailer::~PDFTrailer()
{
}

bool PDFTrailer::emit( EmitContext& rWriteContext ) const
{
    // get xref offset
    unsigned int nXRefPos = rWriteContext.getCurPos();
    // begin xref section, object 0 is always free
    if( ! rWriteContext.write( "xref\r\n"
                               "0 1\r\n"
                               "0000000000 65535 f\r\n", 31 ) )
        return false;
    // check if we are emitting a complete PDF file
    EmitImplData* pEData = getEmitData( rWriteContext );
    if( pEData )
    {
        // emit object xrefs
        const EmitImplData::XRefTable& rXRefs = pEData->m_aXRefTable;
        EmitImplData::XRefTable::const_iterator section_begin, section_end;
        section_begin = rXRefs.begin();
        while( section_begin != rXRefs.end() )
        {
            // find end of continuous object numbers
            section_end = section_begin;
            unsigned int nLast = section_begin->first;
            while( (++section_end) != rXRefs.end() &&
                   section_end->first == nLast+1 )
                nLast = section_end->first;
            // write first object number and number of following entries
            OStringBuffer aBuf( 21 );
            aBuf.append( sal_Int32( section_begin->first ) );
            aBuf.append( ' ' );
            aBuf.append( sal_Int32(nLast - section_begin->first + 1) );
            aBuf.append( "\r\n" );
            if( ! rWriteContext.write( aBuf.getStr(), aBuf.getLength() ) )
                return false;
            while( section_begin != section_end )
            {
                // write 20 char entry of form
                // 0000offset 00gen n\r\n
                aBuf.setLength( 0 );
                OString aOffset( OString::valueOf( sal_Int64(section_begin->second.second ) ) );
                int nPad = 10 - aOffset.getLength();
                for( int i = 0; i < nPad; i++ )
                    aBuf.append( '0' );
                aBuf.append( aOffset );
                aBuf.append( ' ' );
                OString aGeneration( OString::valueOf( sal_Int32(section_begin->second.first ) ) );
                nPad = 5 - aGeneration.getLength();
                for( int i = 0; i < nPad; i++ )
                    aBuf.append( '0' );
                aBuf.append( aGeneration );
                aBuf.append( " n\r\n" );
                if( ! rWriteContext.write( aBuf.getStr(), 20 ) )
                    return false;
                ++section_begin;
            }
        }
    }
    if( ! rWriteContext.write( "trailer\n", 8 ) )
        return false;
    if( ! emitSubElements( rWriteContext ) )
        return false;
    if( ! rWriteContext.write( "startxref\n", 10 ) )
        return false;
    rtl::OString aOffset( rtl::OString::valueOf( sal_Int32(nXRefPos) ) );
    if( ! rWriteContext.write( aOffset.getStr(), aOffset.getLength() ) )
        return false;
    return rWriteContext.write( "\n%%EOF\n", 7 );
}

PDFEntry* PDFTrailer::clone() const
{
    PDFTrailer* pNewTr = new PDFTrailer();
    cloneSubElements( pNewTr->m_aSubElements );
    unsigned int nEle = m_aSubElements.size();
    for( unsigned int i = 0; i < nEle; i++ )
    {
        if( m_aSubElements[i] == m_pDict )
        {
            pNewTr->m_pDict = dynamic_cast<PDFDict*>(pNewTr->m_aSubElements[i]);
            break;
        }
    }
    return pNewTr;
}

#define ENCRYPTION_KEY_LEN 16
#define ENCRYPTION_BUF_LEN 32

namespace pdfparse {
struct PDFFileImplData
{
    bool        m_bIsEncrypted;
    bool        m_bStandardHandler;
    sal_uInt32  m_nAlgoVersion;
    sal_uInt32  m_nStandardRevision;
    sal_uInt32  m_nKeyLength;
    sal_uInt8   m_aOEntry[32];
    sal_uInt8   m_aUEntry[32];
    sal_uInt32  m_nPEntry;
    OString     m_aDocID;
    rtlCipher   m_aCipher;
    rtlDigest   m_aDigest;

    sal_uInt8   m_aDecryptionKey[ENCRYPTION_KEY_LEN+5]; // maximum handled key length

    PDFFileImplData() :
        m_bIsEncrypted( false ),
        m_bStandardHandler( false ),
        m_nAlgoVersion( 0 ),
        m_nStandardRevision( 0 ),
        m_nKeyLength( 0 ),
        m_nPEntry( 0 ),
        m_aCipher( NULL ),
        m_aDigest( NULL )
    {
        memset( m_aOEntry, 0, sizeof( m_aOEntry ) );
        memset( m_aUEntry, 0, sizeof( m_aUEntry ) );
        memset( m_aDecryptionKey, 0, sizeof( m_aDecryptionKey ) );
    }

    ~PDFFileImplData()
    {
        if( m_aCipher )
            rtl_cipher_destroyARCFOUR( m_aCipher );
        if( m_aDigest )
            rtl_digest_destroyMD5( m_aDigest );
    }
};
}

PDFFile::~PDFFile()
{
    if( m_pData )
        delete m_pData;
}

bool PDFFile::isEncrypted() const
{
    return impl_getData()->m_bIsEncrypted;
}

bool PDFFile::decrypt( const sal_uInt8* pInBuffer, sal_uInt32 nLen, sal_uInt8* pOutBuffer,
                       unsigned int nObject, unsigned int nGeneration ) const
{
    if( ! isEncrypted() )
        return false;

    if( ! m_pData->m_aCipher )
        m_pData->m_aCipher = rtl_cipher_createARCFOUR( rtl_Cipher_ModeStream );

    // modify encryption key
    sal_uInt32 i = m_pData->m_nKeyLength;
    m_pData->m_aDecryptionKey[i++] = sal_uInt8(nObject&0xff);
    m_pData->m_aDecryptionKey[i++] = sal_uInt8((nObject>>8)&0xff);
    m_pData->m_aDecryptionKey[i++] = sal_uInt8((nObject>>16)&0xff);
    m_pData->m_aDecryptionKey[i++] = sal_uInt8(nGeneration&0xff);
    m_pData->m_aDecryptionKey[i++] = sal_uInt8((nGeneration>>8)&0xff);

    sal_uInt8 aSum[ENCRYPTION_KEY_LEN];
    rtl_digest_updateMD5( m_pData->m_aDigest, m_pData->m_aDecryptionKey, i );
    rtl_digest_getMD5( m_pData->m_aDigest, aSum, sizeof( aSum ) );

    if( i > 16 )
        i = 16;

    rtlCipherError aErr = rtl_cipher_initARCFOUR( m_pData->m_aCipher,
                                                  rtl_Cipher_DirectionDecode,
                                                  aSum, i,
                                                  NULL, 0 );
    if( aErr == rtl_Cipher_E_None )
        aErr = rtl_cipher_decodeARCFOUR( m_pData->m_aCipher,
                                         pInBuffer, nLen,
                                         pOutBuffer, nLen );
    return aErr == rtl_Cipher_E_None;
}

static const sal_uInt8 nPadString[32] =
{
    0x28, 0xBF, 0x4E, 0x5E, 0x4E, 0x75, 0x8A, 0x41, 0x64, 0x00, 0x4E, 0x56, 0xFF, 0xFA, 0x01, 0x08,
    0x2E, 0x2E, 0x00, 0xB6, 0xD0, 0x68, 0x3E, 0x80, 0x2F, 0x0C, 0xA9, 0xFE, 0x64, 0x53, 0x69, 0x7A
};

static void pad_or_truncate_to_32( const OString& rStr, sal_Char* pBuffer )
{
    int nLen = rStr.getLength();
    if( nLen > 32 )
        nLen = 32;
    const sal_Char* pStr = rStr.getStr();
    memcpy( pBuffer, pStr, nLen );
    int i = 0;
    while( nLen < 32 )
        pBuffer[nLen++] = nPadString[i++];
}

// pass at least pData->m_nKeyLength bytes in
static sal_uInt32 password_to_key( const OString& rPwd, sal_uInt8* pOutKey, PDFFileImplData* pData, bool bComputeO )
{
    // see PDF reference 1.4 Algorithm 3.2
    // encrypt pad string
    sal_Char aPadPwd[ENCRYPTION_BUF_LEN];
    pad_or_truncate_to_32( rPwd, aPadPwd );
    rtl_digest_updateMD5( pData->m_aDigest, aPadPwd, sizeof( aPadPwd ) );
    if( ! bComputeO )
    {
        rtl_digest_updateMD5( pData->m_aDigest, pData->m_aOEntry, 32 );
        sal_uInt8 aPEntry[4];
        aPEntry[0] = static_cast<sal_uInt8>(pData->m_nPEntry & 0xff);
        aPEntry[1] = static_cast<sal_uInt8>((pData->m_nPEntry >> 8 ) & 0xff);
        aPEntry[2] = static_cast<sal_uInt8>((pData->m_nPEntry >> 16) & 0xff);
        aPEntry[3] = static_cast<sal_uInt8>((pData->m_nPEntry >> 24) & 0xff);
        rtl_digest_updateMD5( pData->m_aDigest, aPEntry, sizeof(aPEntry) );
        rtl_digest_updateMD5( pData->m_aDigest, pData->m_aDocID.getStr(), pData->m_aDocID.getLength() );
    }
    sal_uInt8 nSum[RTL_DIGEST_LENGTH_MD5];
    rtl_digest_getMD5( pData->m_aDigest, nSum, sizeof(nSum) );
    if( pData->m_nStandardRevision == 3 )
    {
        for( int i = 0; i < 50; i++ )
        {
            rtl_digest_updateMD5( pData->m_aDigest, nSum, sizeof(nSum) );
            rtl_digest_getMD5( pData->m_aDigest, nSum, sizeof(nSum) );
        }
    }
    sal_uInt32 nLen = pData->m_nKeyLength;
    if( nLen > RTL_DIGEST_LENGTH_MD5 )
        nLen = RTL_DIGEST_LENGTH_MD5;
    memcpy( pOutKey, nSum, nLen );
    return nLen;
}

static bool check_user_password( const OString& rPwd, PDFFileImplData* pData )
{
    // see PDF reference 1.4 Algorithm 3.6
    bool bValid = false;
    sal_uInt8 aKey[ENCRYPTION_KEY_LEN];
    sal_uInt8 nEncryptedEntry[ENCRYPTION_BUF_LEN];
    memset( nEncryptedEntry, 0, sizeof(nEncryptedEntry) );
    sal_uInt32 nKeyLen = password_to_key( rPwd, aKey, pData, false );
    // save (at this time potential) decryption key for later use
    memcpy( pData->m_aDecryptionKey, aKey, nKeyLen );
    if( pData->m_nStandardRevision == 2 )
    {
        // see PDF reference 1.4 Algorithm 3.4
        // encrypt pad string
        rtl_cipher_initARCFOUR( pData->m_aCipher, rtl_Cipher_DirectionEncode,
                                aKey, nKeyLen,
                                NULL, 0 );
        rtl_cipher_encodeARCFOUR( pData->m_aCipher, nPadString, sizeof( nPadString ),
                                  nEncryptedEntry, sizeof( nEncryptedEntry ) );
        bValid = (memcmp( nEncryptedEntry, pData->m_aUEntry, 32 ) == 0);
    }
    else if( pData->m_nStandardRevision == 3 )
    {
        // see PDF reference 1.4 Algorithm 3.5
        rtl_digest_updateMD5( pData->m_aDigest, nPadString, sizeof( nPadString ) );
        rtl_digest_updateMD5( pData->m_aDigest, pData->m_aDocID.getStr(), pData->m_aDocID.getLength() );
        rtl_digest_getMD5( pData->m_aDigest, nEncryptedEntry, sizeof(nEncryptedEntry) );
        rtl_cipher_initARCFOUR( pData->m_aCipher, rtl_Cipher_DirectionEncode,
                                aKey, sizeof(aKey), NULL, 0 );
        rtl_cipher_encodeARCFOUR( pData->m_aCipher,
                                  nEncryptedEntry, 16,
                                  nEncryptedEntry, 16 ); // encrypt in place
        for( int i = 1; i <= 19; i++ ) // do it 19 times, start with 1
        {
            sal_uInt8 aTempKey[ENCRYPTION_KEY_LEN];
            for( sal_uInt32 j = 0; j < sizeof(aTempKey); j++ )
                aTempKey[j] = static_cast<sal_uInt8>( aKey[j] ^ i );

            rtl_cipher_initARCFOUR( pData->m_aCipher, rtl_Cipher_DirectionEncode,
                                    aTempKey, sizeof(aTempKey), NULL, 0 );
            rtl_cipher_encodeARCFOUR( pData->m_aCipher,
                                      nEncryptedEntry, 16,
                                      nEncryptedEntry, 16 ); // encrypt in place
        }
        bValid = (memcmp( nEncryptedEntry, pData->m_aUEntry, 16 ) == 0);
    }
    return bValid;
}

bool PDFFile::setupDecryptionData( const OString& rPwd ) const
{
    if( !impl_getData()->m_bIsEncrypted )
        return rPwd.isEmpty();

    // check if we can handle this encryption at all
    if( ! m_pData->m_bStandardHandler ||
        m_pData->m_nAlgoVersion < 1 ||
        m_pData->m_nAlgoVersion > 2 ||
        m_pData->m_nStandardRevision < 2 ||
        m_pData->m_nStandardRevision > 3 )
        return false;

    if( ! m_pData->m_aCipher )
        m_pData->m_aCipher = rtl_cipher_createARCFOUR(rtl_Cipher_ModeStream);
    if( ! m_pData->m_aDigest )
        m_pData->m_aDigest = rtl_digest_createMD5();

    // first try user password
    bool bValid = check_user_password( rPwd, m_pData );

    if( ! bValid )
    {
        // try owner password
        // see PDF reference 1.4 Algorithm 3.7
        sal_uInt8 aKey[ENCRYPTION_KEY_LEN];
        sal_uInt8 nPwd[ENCRYPTION_BUF_LEN];
        memset( nPwd, 0, sizeof(nPwd) );
        sal_uInt32 nKeyLen = password_to_key( rPwd, aKey, m_pData, true );
        if( m_pData->m_nStandardRevision == 2 )
        {
            rtl_cipher_initARCFOUR( m_pData->m_aCipher, rtl_Cipher_DirectionDecode,
                                    aKey, nKeyLen, NULL, 0 );
            rtl_cipher_decodeARCFOUR( m_pData->m_aCipher,
                                      m_pData->m_aOEntry, 32,
                                      nPwd, 32 );
        }
        else if( m_pData->m_nStandardRevision == 3 )
        {
            memcpy( nPwd, m_pData->m_aOEntry, 32 );
            for( int i = 19; i >= 0; i-- )
            {
                sal_uInt8 nTempKey[ENCRYPTION_KEY_LEN];
                for( unsigned int j = 0; j < sizeof(nTempKey); j++ )
                    nTempKey[j] = sal_uInt8(aKey[j] ^ i);
                rtl_cipher_initARCFOUR( m_pData->m_aCipher, rtl_Cipher_DirectionDecode,
                                        nTempKey, nKeyLen, NULL, 0 );
                rtl_cipher_decodeARCFOUR( m_pData->m_aCipher,
                                          nPwd, 32,
                                          nPwd, 32 ); // decrypt inplace
            }
        }
        bValid = check_user_password( OString( (sal_Char*)nPwd, 32 ), m_pData );
    }

    return bValid;
}

rtl::OUString PDFFile::getDecryptionKey() const
{
    rtl::OUStringBuffer aBuf( ENCRYPTION_KEY_LEN * 2 );
    if( impl_getData()->m_bIsEncrypted )
    {
        for( sal_uInt32 i = 0; i < m_pData->m_nKeyLength; i++ )
        {
            static const sal_Unicode pHexTab[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                                     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            aBuf.append( pHexTab[(m_pData->m_aDecryptionKey[i] >> 4) & 0x0f] );
            aBuf.append( pHexTab[(m_pData->m_aDecryptionKey[i] & 0x0f)] );
        }

    }
    return aBuf.makeStringAndClear();
}

PDFFileImplData* PDFFile::impl_getData() const
{
    if( m_pData )
        return m_pData;
    m_pData = new PDFFileImplData();
    // check for encryption dict in a trailer
    unsigned int nElements = m_aSubElements.size();
    while( nElements-- > 0 )
    {
        PDFTrailer* pTrailer = dynamic_cast<PDFTrailer*>(m_aSubElements[nElements]);
        if( pTrailer && pTrailer->m_pDict )
        {
            // search doc id
            PDFDict::Map::iterator doc_id = pTrailer->m_pDict->m_aMap.find( "ID" );
            if( doc_id != pTrailer->m_pDict->m_aMap.end() )
            {
                PDFArray* pArr = dynamic_cast<PDFArray*>(doc_id->second);
                if( pArr && pArr->m_aSubElements.size() > 0 )
                {
                    PDFString* pStr = dynamic_cast<PDFString*>(pArr->m_aSubElements[0]);
                    if( pStr )
                        m_pData->m_aDocID = pStr->getFilteredString();
                    #if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "DocId is <" );
                    for( int i = 0; i < m_pData->m_aDocID.getLength(); i++ )
                        fprintf( stderr, "%.2x", (unsigned int)sal_uInt8(m_pData->m_aDocID.getStr()[i]) );
                    fprintf( stderr, ">\n" );
                    #endif
                }
            }
            // search Encrypt entry
            PDFDict::Map::iterator enc =
                pTrailer->m_pDict->m_aMap.find( "Encrypt" );
            if( enc != pTrailer->m_pDict->m_aMap.end() )
            {
                PDFDict* pDict = dynamic_cast<PDFDict*>(enc->second);
                if( ! pDict )
                {
                    PDFObjectRef* pRef = dynamic_cast<PDFObjectRef*>(enc->second);
                    if( pRef )
                    {
                        PDFObject* pObj = findObject( pRef );
                        if( pObj && pObj->m_pObject )
                            pDict = dynamic_cast<PDFDict*>(pObj->m_pObject);
                    }
                }
                if( pDict )
                {
                    PDFDict::Map::iterator filter = pDict->m_aMap.find( "Filter" );
                    PDFDict::Map::iterator version = pDict->m_aMap.find( "V" );
                    PDFDict::Map::iterator len = pDict->m_aMap.find( "Length" );
                    PDFDict::Map::iterator o_ent = pDict->m_aMap.find( "O" );
                    PDFDict::Map::iterator u_ent = pDict->m_aMap.find( "U" );
                    PDFDict::Map::iterator r_ent = pDict->m_aMap.find( "R" );
                    PDFDict::Map::iterator p_ent = pDict->m_aMap.find( "P" );
                    if( filter != pDict->m_aMap.end() )
                    {
                        m_pData->m_bIsEncrypted = true;
                        m_pData->m_nKeyLength = 5;
                        if( version != pDict->m_aMap.end() )
                        {
                            PDFNumber* pNum = dynamic_cast<PDFNumber*>(version->second);
                            if( pNum )
                                m_pData->m_nAlgoVersion = static_cast<sal_uInt32>(pNum->m_fValue);
                        }
                        if( m_pData->m_nAlgoVersion >= 3 )
                            m_pData->m_nKeyLength = 16;
                        if( len != pDict->m_aMap.end() )
                        {
                            PDFNumber* pNum = dynamic_cast<PDFNumber*>(len->second);
                            if( pNum )
                                m_pData->m_nKeyLength = static_cast<sal_uInt32>(pNum->m_fValue) / 8;
                        }
                        PDFName* pFilter = dynamic_cast<PDFName*>(filter->second);
                        if( pFilter && pFilter->getFilteredName() == "Standard" )
                            m_pData->m_bStandardHandler = true;
                        if( o_ent != pDict->m_aMap.end() )
                        {
                            PDFString* pString = dynamic_cast<PDFString*>(o_ent->second);
                            if( pString )
                            {
                                OString aEnt = pString->getFilteredString();
                                if( aEnt.getLength() == 32 )
                                    memcpy( m_pData->m_aOEntry, aEnt.getStr(), 32 );
                                #if OSL_DEBUG_LEVEL > 1
                                else
                                {
                                    fprintf( stderr, "O entry has length %d, should be 32 <", (int)aEnt.getLength() );
                                    for( int i = 0; i < aEnt.getLength(); i++ )
                                        fprintf( stderr, " %.2X", (unsigned int)sal_uInt8(aEnt.getStr()[i]) );
                                    fprintf( stderr, ">\n" );
                                }
                                #endif
                            }
                        }
                        if( u_ent != pDict->m_aMap.end() )
                        {
                            PDFString* pString = dynamic_cast<PDFString*>(u_ent->second);
                            if( pString )
                            {
                                OString aEnt = pString->getFilteredString();
                                if( aEnt.getLength() == 32 )
                                    memcpy( m_pData->m_aUEntry, aEnt.getStr(), 32 );
                                #if OSL_DEBUG_LEVEL > 1
                                else
                                {
                                    fprintf( stderr, "U entry has length %d, should be 32 <", (int)aEnt.getLength() );
                                    for( int i = 0; i < aEnt.getLength(); i++ )
                                        fprintf( stderr, " %.2X", (unsigned int)sal_uInt8(aEnt.getStr()[i]) );
                                    fprintf( stderr, ">\n" );
                                }
                                #endif
                            }
                        }
                        if( r_ent != pDict->m_aMap.end() )
                        {
                            PDFNumber* pNum = dynamic_cast<PDFNumber*>(r_ent->second);
                            if( pNum )
                                m_pData->m_nStandardRevision = static_cast<sal_uInt32>(pNum->m_fValue);
                        }
                        if( p_ent != pDict->m_aMap.end() )
                        {
                            PDFNumber* pNum = dynamic_cast<PDFNumber*>(p_ent->second);
                            if( pNum )
                                m_pData->m_nPEntry = static_cast<sal_uInt32>(static_cast<sal_Int32>(pNum->m_fValue));
                        #if OSL_DEBUG_LEVEL > 1
                            fprintf( stderr, "p entry is %" SAL_PRIxUINT32 "\n", m_pData->m_nPEntry );
                        #endif
                        }
                        #if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "Encryption dict: sec handler: %s, version = %d, revision = %d, key length = %d\n",
                                 pFilter ? OUStringToOString( pFilter->getFilteredName(), RTL_TEXTENCODING_UTF8 ).getStr() : "<unknown>",
                                 (int)m_pData->m_nAlgoVersion, (int)m_pData->m_nStandardRevision, (int)m_pData->m_nKeyLength );
                        #endif
                        break;
                    }
                }
            }
        }
    }

    return m_pData;
}

bool PDFFile::emit( EmitContext& rWriteContext ) const
{
    setEmitData(  rWriteContext, new EmitImplData( this ) );

    OStringBuffer aBuf( 32 );
    aBuf.append( "%PDF-" );
    aBuf.append( sal_Int32( m_nMajor ) );
    aBuf.append( '.' );
    aBuf.append( sal_Int32( m_nMinor ) );
    aBuf.append( "\n" );
    if( ! rWriteContext.write( aBuf.getStr(), aBuf.getLength() ) )
        return false;
    return emitSubElements( rWriteContext );
}

PDFEntry* PDFFile::clone() const
{
    PDFFile* pNewFl = new PDFFile();
    pNewFl->m_nMajor = m_nMajor;
    pNewFl->m_nMinor = m_nMinor;
    cloneSubElements( pNewFl->m_aSubElements );
    return pNewFl;
}

PDFPart::~PDFPart()
{
}

bool PDFPart::emit( EmitContext& rWriteContext ) const
{
    return emitSubElements( rWriteContext );
}

PDFEntry* PDFPart::clone() const
{
    PDFPart* pNewPt = new PDFPart();
    cloneSubElements( pNewPt->m_aSubElements );
    return pNewPt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
