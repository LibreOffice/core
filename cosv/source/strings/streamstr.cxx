/*************************************************************************
 *
 *  $RCSfile: streamstr.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:48:01 $
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

#include <precomp.h>
#include <cosv/streamstr.hxx>

// NOT FULLY DECLARED SERVICES
#include <string.h>
#include <stdio.h>
#include <stdarg.h> // both are needed to satisfy all compilers
#include <cstdarg>  // std::va_list and friends

#include <cosv/comfunc.hxx>
#include <cosv/template/swelist.hxx>



namespace csv
{

const uintt C_short_max_size    = sizeof(short) / 2 + 1;
const uintt C_int_max_size      = sizeof(int)   / 2 + 1;
const uintt C_long_max_size     = sizeof(long)  / 2 + 1;


inline void
StreamStr::Advance(size_type i_nAddedSize)
{ pCur += i_nAddedSize; }



StreamStr::StreamStr( size_type     i_nCapacity )
    :   nCapacity1( i_nCapacity + 1 ),
        dpData( new char [i_nCapacity + 1] ),
        pEnd(dpData),
        pCur(dpData),
        eMode(str::overwrite)
{
    *pEnd = '\0';
}

StreamStr::StreamStr( const char *  i_sInitStr,
                      size_type     i_nCapacity )
    :   nCapacity1(0),
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

StreamStr::StreamStr( size_type         i_nGuessedCapacity,
                      const char *      str1,
                      const char *      str2,
                                        ... )
    :   nCapacity1( i_nGuessedCapacity + 1 ),
        dpData( new char [i_nGuessedCapacity + 1] ),
        pEnd(dpData),
        pCur(dpData),
        eMode(str::overwrite)
{
    *pEnd = '\0';

    operator<<(str1);
    operator<<(str2);

    ::va_list ap;

    va_start(ap, str2);
    for ( const char * strAdd = va_arg(ap,const char*);
          strAdd != 0;
          strAdd = va_arg(ap,const char*) )
    {
        size_type nLen = strlen(strAdd);
        ProvideAddingSize( nLen );
        memcpy(pCur, strAdd, nLen);
        Advance(nLen);
    }  // end for
    va_end(ap);
}

StreamStr::StreamStr( const self & i_rOther )
    :   nCapacity1( i_rOther.nCapacity1 ),
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
StreamStr::push_front( const char * i_str )
{
    insert_mode eOriginalMode = eMode;
    char * pOriginalCur = pCur;
    eMode = str::insert;
    pCur = dpData;

    operator<<(i_str);

    eMode = eOriginalMode;
    pCur = pOriginalCur + strlen(i_str);
}

void
StreamStr::push_front( char i_c )
{
    insert_mode eOriginalMode = eMode;
    char * pOriginalCur = pCur;
    eMode = str::insert;
    pCur = dpData;

    operator<<(i_c);

    eMode = eOriginalMode;
    pCur = pOriginalCur + 1;
}

void
StreamStr::push_back( const char * i_str )
{
    insert_mode eOriginalMode = eMode;
    eMode = str::overwrite;

    operator<<(i_str);

    eMode = eOriginalMode;
}

void
StreamStr::push_back( char i_c )
{
    insert_mode eOriginalMode = eMode;
    eMode = str::overwrite;

    operator<<(i_c);

    eMode = eOriginalMode;
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

    bool bEndOfStream = nCount == 0;
    // Check for line-end:
    if ( NOT bEndOfStream AND c != 0 )
    {
        char oldc = c;
          if (i_src.read(&c, 1) == 1)
        {
            if (c != 13 AND c != 10 OR c == oldc)
                i_src.seek(-1,::csv::cur);
        }
    }
    return *this;
}

void
StreamStr::strip_front(char i_cToRemove)
{
    const_iterator it = begin();
    for ( ;
          it != end() ? *it == i_cToRemove : false;
          ++it );
    pop_front(it - begin());
}

void
StreamStr::strip_back(char i_cToRemove)
{
    const_iterator it = end();
    for ( ;
          it != begin() ? *(it-1) == i_cToRemove : false;
          --it );
    pop_back(end() - it);
}

void
StreamStr::strip_frontback(char i_cToRemove)
{
    strip_front(i_cToRemove);
    strip_back(i_cToRemove);
}

void
StreamStr::strip_front_whitespace()
{
    const_iterator it = begin();
    for ( ;
          it != end() ? *it < 33 : false;
          ++it );
    pop_front(it - begin());
}

void
StreamStr::strip_back_whitespace()
{
    const_iterator it = end();
    for ( ;
          it != begin() ? *(it-1) < 33 : false;
          --it );
    pop_back(end() - it);
}

void
StreamStr::strip_frontback_whitespace()
{
    strip_front_whitespace();
    strip_back_whitespace();
}

void
StreamStr::replace( position_type       i_nStart,
                    size_type           i_nSize,
                    Area                i_aReplacement )
{
   if (i_nStart >= length() OR i_nSize < 1)
      return;

   insert_mode eOldMode = eMode;
   eMode = str::insert;
   pCur = dpData + i_nStart;

   size_type anz = min( length() - i_nStart, i_nSize );

   if ( anz < i_aReplacement.nLength )
   {
        ProvideAddingSize( i_aReplacement.nLength - anz );
   }
   else if ( anz > i_aReplacement.nLength )
   {
        seek_type nMove = seek_type(anz - i_aReplacement.nLength);

        MoveData( dpData + i_nStart + anz,
                  pEnd,
                  -nMove );
        pEnd -= nMove;
        *pEnd = '\0';
   }

   memcpy( dpData + i_nStart, i_aReplacement.sStr, i_aReplacement.nLength );
   Advance(i_aReplacement.nLength);

   eMode = eOldMode;
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

void
StreamStr::replace_all( Area                i_aStrToSearch,
                        Area                i_aReplacement )
{
    position_type p = 0;
    const char *  pSearch = i_aStrToSearch.sStr;
    size_type     nSearch = i_aStrToSearch.nLength;

    while ( p <= length() - nSearch )
    {
        if ( strncmp(dpData+p, pSearch, nSearch) == 0 )
        {
            replace( p, nSearch, i_aReplacement );
            p += i_aReplacement.nLength;
        }
        else
        {
            ++p;
        }
   }    // end while
}

StreamStr &
StreamStr::to_lower( position_type       i_nStart,
                     size_type           i_nLength )
{
    static char cLower[128] =
    { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
     64, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
    112,113,114,115,116,117,118,119,120,121,122, 91, 92, 93, 94, 95,
     96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
    112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 };

    if ( i_nStart < length() )
    {
        char * pStop = i_nStart + i_nLength < length()
                            ?   dpData + i_nStart + i_nLength
                            :   pEnd;
        for ( char * pChange = dpData + i_nStart;
              pChange != pStop;
              ++pChange )
        {
            *pChange =  (*pChange & char(0x80)) == '\0'
                            ?   cLower[*pChange]
                            :   *pChange;
        }
    }
    return *this;
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
            *pChange =  (*pChange & char(0x80)) == '\0'
                            ?   cUpper[*pChange]
                            :   *pChange;
        }
    }
    return *this;
}


StreamStr::size_type
StreamStr::token_count( char i_cSplit ) const
{
    return count_chars(dpData, i_cSplit) + 1;
}

String
StreamStr::token( position_type i_nNr,
                  char          i_cSplit ) const
{
    // Find begin:
    const char * pTokenBegin = dpData;
       for ( position_type nNr = i_nNr;
          nNr > 0;
          --nNr )
    {
        pTokenBegin = strchr(pTokenBegin,i_cSplit);
        if (pTokenBegin == 0)
            return String("");
        ++pTokenBegin;
    }

    // Find end:
    const char * pTokenEnd = strchr(pTokenBegin, i_cSplit);
    if (pTokenEnd == 0)
        pTokenEnd = pEnd;

    return String(pTokenBegin, size_type(pTokenEnd-pTokenBegin) );
}

class StreamStrPool
{
  public:
                        StreamStrPool();
                        ~StreamStrPool();
  private:
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
        register const char * pSrc  = i_pStart;
        register char * pDest = i_pStart + i_nDiff;
        for ( ; pSrc != i_pEnd; ++pSrc, ++pDest )
        {
            *pDest = *pSrc;
        }
    }
}

// Dummy, needed for debug-versions of some compilers.
void
c_str()
{
    // Does nothing.
}


}   // namespace csv

