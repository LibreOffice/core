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

#include <precomp.h>
#include <cosv/streamstr.hxx>

// NOT FULLY DECLARED SERVICES
#include <string.h>
#include <stdio.h>
#include <stdarg.h> // both are needed to satisfy all compilers
#include <cstdarg>  // std::va_list and friends

#include <cosv/comfunc.hxx>
#include <cosv/tpl/swelist.hxx>



namespace csv
{


// Maximal sizes of resulting integers in text form:
const uintt C_short_max_size    = sizeof(short) * 3;
const uintt C_int_max_size      = sizeof(int) * 3;
const uintt C_long_max_size     = sizeof(long) * 3;


inline void
StreamStr::Advance(size_type i_nAddedSize)
{ pCur += i_nAddedSize; }



StreamStr::StreamStr( size_type     i_nCapacity )
    :   bostream(),
        nCapacity1( i_nCapacity + 1 ),
        dpData( new char [i_nCapacity + 1] ),
        pEnd(dpData),
        pCur(dpData),
        eMode(str::overwrite)
{
    *pEnd = '\0';
}

StreamStr::StreamStr( const char *  i_sInitStr,
                      size_type     i_nCapacity )
    :   bostream(),
        nCapacity1(0),
        dpData(0),
        pEnd(0),
        pCur(0),
        eMode(str::overwrite)
{
    size_type nLength = strlen(i_sInitStr);
    nCapacity1 = csv::max(nLength, i_nCapacity) + 1;
    dpData = new char [nCapacity1];
    strcpy(dpData, i_sInitStr);     // SAFE STRCPY (#100211# - checked)
    pCur = dpData + nLength;
    pEnd = pCur;
}

StreamStr::StreamStr( const self & i_rOther )
    :   bostream(),
        nCapacity1( i_rOther.nCapacity1 ),
        dpData( new char [i_rOther.nCapacity1] ),
        pEnd( dpData + strlen(i_rOther.dpData) ),
        pCur( dpData + i_rOther.tellp() ),
        eMode(i_rOther.eMode)
{
    strcpy( dpData, i_rOther.dpData );      // SAFE STRCPY (#100211# - checked)
}

StreamStr::~StreamStr()
{
    delete [] dpData;
}


StreamStr &
StreamStr::operator=( const self & i_rOther )
{
    delete [] dpData;

    nCapacity1 = i_rOther.nCapacity1;
    dpData = new char [i_rOther.nCapacity1];
    pEnd = dpData + strlen(i_rOther.dpData);
    strcpy( dpData, i_rOther.dpData );          // SAFE STRCPY (#100211# - checked)
    pCur = dpData + i_rOther.tellp();
    eMode = i_rOther.eMode;

    return *this;
}

StreamStr &
StreamStr::operator<<( const char * i_s )
{
    size_type nLength = strlen(i_s);

    ProvideAddingSize( nLength );
    memcpy( pCur, i_s, nLength );
    Advance(nLength);

    return *this;
}

StreamStr &
StreamStr::operator<<( const String & i_s )
{
    size_type nLength = i_s.length();

    ProvideAddingSize( nLength );
    memcpy( pCur, i_s.c_str(), nLength );
    Advance(nLength);

    return *this;
}

StreamStr &
StreamStr::operator<<( char i_c )
{
    ProvideAddingSize( 1 );
    *pCur = i_c;
    Advance(1);

    return *this;
}

StreamStr &
StreamStr::operator<<( unsigned char i_c )
{
    return operator<<( char(i_c) );
}

StreamStr &
StreamStr::operator<<( signed char i_c )
{
    return operator<<( char(i_c) );
}

StreamStr &
StreamStr::operator<<( short i_n )
{
    char buf[C_short_max_size] = "";
    sprintf( buf, "%hi", i_n );         // SAFE SPRINTF (#100211# - checked)

    size_type nLength = strlen(buf);
    ProvideAddingSize( nLength );
    memcpy( pCur, buf, nLength );
    Advance( nLength );

    return *this;
}

StreamStr &
StreamStr::operator<<( unsigned short i_n )
{
    char buf[C_short_max_size] = "";
    sprintf( buf, "%hu", i_n );         // SAFE SPRINTF (#100211# - checked)

    size_type nLength = strlen(buf);
    ProvideAddingSize( nLength );
    memcpy( pCur, buf, nLength );
    Advance( nLength );

    return *this;
}

StreamStr &
StreamStr::operator<<( int i_n )
{
    char buf[C_int_max_size] = "";
    sprintf( buf, "%i", i_n );          // SAFE SPRINTF (#100211# - checked)

    size_type nLength = strlen(buf);
    ProvideAddingSize( nLength );
    memcpy( pCur, buf, nLength );
    Advance( nLength );

    return *this;
}

StreamStr &
StreamStr::operator<<( unsigned int i_n )
{
    char buf[C_int_max_size] = "";
    sprintf( buf, "%u", i_n );          // SAFE SPRINTF (#100211# - checked)

    size_type nLength = strlen(buf);
    ProvideAddingSize( nLength );
    memcpy( pCur, buf, nLength );
    Advance( nLength );

    return *this;
}

StreamStr &
StreamStr::operator<<( long i_n )
{
    char buf[C_long_max_size] = "";
    sprintf( buf, "%li", i_n );         // SAFE SPRINTF (#100211# - checked)

    size_type nLength = strlen(buf);
    ProvideAddingSize( nLength );
    memcpy( pCur, buf, nLength );
    Advance( nLength );

    return *this;
}

StreamStr &
StreamStr::operator<<( unsigned long i_n )
{
    char buf[C_long_max_size] = "";
    sprintf( buf, "%lu", i_n );         // SAFE SPRINTF (#100211# - checked)

    size_type nLength = strlen(buf);
    ProvideAddingSize( nLength );
    memcpy( pCur, buf, nLength );
    Advance( nLength );

    return *this;
}

StreamStr &
StreamStr::operator<<( float i_n )
{
    const int C_float_max_size = 20;
    char buf[C_float_max_size] = "";
    sprintf( buf, "%.*g", C_float_max_size-8, i_n );    // SAFE SPRINTF (#100211# - checked)

    size_type nLength = strlen(buf);
    ProvideAddingSize( nLength );
    memcpy( pCur, buf, nLength );
    Advance( nLength );

    return *this;
}

StreamStr &
StreamStr::operator<<( double i_n )
{
    const int C_double_max_size = 30;
    char buf[C_double_max_size] = "";
    sprintf( buf, "%.*lg", C_double_max_size-8, i_n );  // SAFE SPRINTF (#100211# - checked)

    size_type nLength = strlen(buf);
    ProvideAddingSize( nLength );
    memcpy( pCur, buf, nLength );
    Advance( nLength );

    return *this;
}

const char &
StreamStr::operator[]( position_type i_nPosition ) const
{
    static const char aNull_ = '\0';

    if ( position_type(pEnd - dpData) > i_nPosition )
        return dpData[i_nPosition];
    return aNull_;
}

char &
StreamStr::operator[]( position_type i_nPosition )
{
    static char aDummy_ = '\0';

    if ( position_type(pEnd - dpData) > i_nPosition )
        return dpData[i_nPosition];
    return aDummy_;
}

void
StreamStr::resize( size_type i_nMinimumCapacity )
{
    if ( i_nMinimumCapacity <= capacity() )
        return;

    Resize(i_nMinimumCapacity);
}

StreamStr &
StreamStr::seekp( seek_type           i_nCount,
                  seek_dir            i_eDirection )
{
    seek_type nLength = seek_type( length() );
    seek_type nNewPos = tellp();

    switch ( i_eDirection )
    {
         case ::csv::beg:  nNewPos = i_nCount;
                          break;
        case ::csv::cur:  nNewPos += i_nCount;
                          break;
        case ::csv::end:  nNewPos = nLength + i_nCount;
                          break;
    }

    if ( in_range<seek_type>(0, nNewPos, nLength + 1) )
    {
        pCur = dpData + nNewPos;
        if (eMode == str::overwrite)
        {
             pEnd = pCur;
            *pEnd = '\0';
        }
    }

    return *this;
}

StreamStr &
StreamStr::set_insert_mode( insert_mode i_eMode )
{
    eMode = i_eMode;
    return *this;
}

void
StreamStr::pop_front( size_type i_nCount )
{
    size_type nCount = min(i_nCount, length());

    MoveData( dpData + nCount, pEnd, -(seek_type(nCount)) );

    pCur -= nCount;
    pEnd -= nCount;
    *pEnd = '\0';
}

void
StreamStr::pop_back( size_type i_nCount )
{
    size_type nCount = min(i_nCount, length());
    pEnd -= nCount;
    if (pCur > pEnd)
        pCur = pEnd;
    *pEnd = '\0';
}

StreamStr &
StreamStr::operator_join( std::vector<String>::const_iterator i_rBegin,
                          std::vector<String>::const_iterator i_rEnd,
                          const char *                        i_sLink )
{
    std::vector<String>::const_iterator it = i_rBegin;
    if ( it != i_rEnd )
    {
        operator<<(*it);
        for ( ++it; it != i_rEnd; ++it )
        {
            operator<<(i_sLink);
            operator<<(*it);
        }
    }
    return *this;
}

StreamStr &
StreamStr::operator_add_substr( const char *        i_sText,
                                size_type           i_nLength )
{
    size_type nLength = csv::min<size_type>(i_nLength, strlen(i_sText));

    ProvideAddingSize( nLength );
    memcpy( pCur, i_sText, nLength );
    Advance(nLength);

    return *this;
}

StreamStr &
StreamStr::operator_add_token( const char *        i_sText,
                               char                i_cDelimiter )
{
    const char * pTokenEnd = strchr(i_sText, i_cDelimiter);
    if (pTokenEnd == 0)
        operator<<(i_sText);
    else
        operator_add_substr(i_sText, pTokenEnd-i_sText);
    return *this;
}

StreamStr &
StreamStr::operator_read_line( bstream & i_src )
{
    char c = 0;
    intt nCount = 0;
    for ( nCount = i_src.read(&c, 1);
          nCount == 1 AND c != 13 AND c != 10;
          nCount = i_src.read(&c, 1) )
    {
        operator<<(c);
    }

    // Check for line-end:
    if ( NOT (nCount == 0) AND c != 0 )
    {
        char oldc = c;
          if (i_src.read(&c, 1) == 1)
        {
            if ( (c != 13 AND c != 10) OR c == oldc)
                i_src.seek(-1,::csv::cur);
        }
    }
    return *this;
}

void
StreamStr::strip_front_whitespace()
{
    const_iterator it = begin();
    for ( ;
          it != end() ? *it < 33 : false;
          ++it ) ;
    pop_front(it - begin());
}

void
StreamStr::strip_back_whitespace()
{
    const_iterator it = end();
    for ( ;
          it != begin() ? *(it-1) < 33 : false;
          --it ) ;
    pop_back(end() - it);
}

void
StreamStr::strip_frontback_whitespace()
{
    strip_front_whitespace();
    strip_back_whitespace();
}

void
StreamStr::replace_all( char i_cCarToSearch,
                        char i_cReplacement )
{
   for ( char * p = dpData; p != pEnd; ++p )
   {
        if (*p == i_cCarToSearch)
            *p = i_cReplacement;
   }
}

StreamStr &
StreamStr::to_upper( position_type       i_nStart,
                     size_type           i_nLength )
{
    static char cUpper[128] =
    { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
     80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
     96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
     80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123,124,125,126,127 };

    if ( i_nStart < length() )
    {
        char * pStop = i_nStart + i_nLength < length()
                            ?   dpData + i_nStart + i_nLength
                            :   pEnd;
        for ( char * pChange = dpData + i_nStart;
              pChange != pStop;
              ++pChange )
        {
            *pChange =  (static_cast< unsigned char >(*pChange) & 0x80) == 0
                            ?   cUpper[ UINT8(*pChange) ]
                            :   *pChange;
        }
    }
    return *this;
}

class StreamStrPool
{
  public:
                        StreamStrPool();
                        ~StreamStrPool();
  private:
    // Non-copyable
    StreamStrPool(StreamStrPool &); // not defined
    void operator =(StreamStrPool &); // not defined

    // Interface to:
    friend class StreamStrLock;
    static StreamStr &  AcquireFromPool_(
                            uintt               i_nMinimalSize );
    static void         ReleaseToPool_(
                            DYN StreamStr *     let_dpUsedStr );

    // DATA
    SweList< DYN StreamStr* >
                        aPool;
};

StreamStrPool::StreamStrPool()
{
}

StreamStrPool::~StreamStrPool()
{
    for ( SweList< DYN StreamStr* >::iterator it = aPool.begin();
          it != aPool.end();
          ++it )
    {
         delete (*it);
    }
}

namespace
{
    static StreamStrPool aPool_;
}


StreamStr &
StreamStrPool::AcquireFromPool_( uintt i_nMinimalSize )
{
    if ( aPool_.aPool.empty() )
    {
        return *new StreamStr(i_nMinimalSize);
    }

    StreamStr & ret = *aPool_.aPool.front();
    aPool_.aPool.pop_front();
    ret.resize(i_nMinimalSize);
    ret.seekp(0);
    ret.set_insert_mode(str::overwrite);
    return ret;
}

void
StreamStrPool::ReleaseToPool_( DYN StreamStr * let_dpUsedStr )
{
    aPool_.aPool.push_back( let_dpUsedStr );
}

StreamStrLock::StreamStrLock( uintt i_nMinimalSize )
    :   pStr( &StreamStrPool::AcquireFromPool_(i_nMinimalSize) )
{
}

StreamStrLock::~StreamStrLock()
{
    StreamStrPool::ReleaseToPool_(pStr);
}


UINT32
StreamStr::do_write( const void *    i_pSrc,
                     UINT32          i_nNrofBytes )
{
    ProvideAddingSize( i_nNrofBytes );
    memcpy( pCur, i_pSrc, i_nNrofBytes );
    Advance(i_nNrofBytes);

    return i_nNrofBytes;
}

void
StreamStr::ProvideAddingSize( size_type i_nSize2Add )
{
    size_type nLength = length();
    if ( capacity() - nLength < i_nSize2Add )
        Resize( nLength + i_nSize2Add );

    pEnd += i_nSize2Add;
      *pEnd = '\0';

    if (eMode == str::insert AND pCur != pEnd)
    {
        MoveData( pCur, pCur + i_nSize2Add, seek_type(i_nSize2Add) );
    }
}

void
StreamStr::Resize( size_type i_nMinimumCapacity )
{
    size_type nNewSize = nCapacity1 < 128
                            ?   nCapacity1 << 1
                            :   (nCapacity1 << 1) - (nCapacity1 >> 1);
    nCapacity1 = csv::max( nNewSize, size_type(i_nMinimumCapacity + 1) );

    char * pNew = new char[nCapacity1];
    strcpy ( pNew, dpData );            // SAFE STRCPY (#100211# - checked)
    pEnd = pNew + (pEnd - dpData);
    pCur = pNew + (pCur - dpData);

    delete [] dpData;
    dpData = pNew;
}

void
StreamStr::MoveData( char *        i_pStart,
                     char *        i_pEnd,
                     seek_type     i_nDiff )
{
    if (i_nDiff > 0)
    {
        register const char * pSrc  = i_pEnd;
        register char * pDest = i_pEnd + i_nDiff;
        for ( ; pSrc != i_pStart; --pSrc, --pDest )
        {
            *pDest = *pSrc;
        }
        *pDest = *pSrc;
    }
    else if (i_nDiff < 0)
    {
        const char * pSrc  = i_pStart;
        char * pDest = i_pStart + i_nDiff;
        for ( ; pSrc != i_pEnd; ++pSrc, ++pDest )
        {
            *pDest = *pSrc;
        }
    }
}

// Does nothing, only the name of this function is needed.
void
c_str()
{
    // Does nothing.
}

}   // namespace csv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
