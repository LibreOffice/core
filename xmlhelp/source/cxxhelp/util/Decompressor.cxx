/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Decompressor.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"
#include <db/Block.hxx>
#include <util/Decompressor.hxx>
#include <util/CompressorIterator.hxx>



using namespace xmlsearch;
using namespace xmlsearch::util;



const sal_Int32 Decompressor::BitsInByte = 8;
const sal_Int32 Decompressor::NBits = 32;


sal_Int32 Decompressor::read( sal_Int32 kBits ) throw( excep::XmlSearchException )
{
    sal_Int32 shift = BitsInByte - toRead_;
    if( kBits <= toRead_ )
        return ( ( readByte_ << shift ) & 0xFF) >>/*>*/ (shift + ( toRead_ -= kBits));
    else
    {
        sal_Int32 result = toRead_ > 0 ? ( ( readByte_ << shift ) & 0xFF ) >>/*>*/ shift : 0;
        for( kBits -= toRead_; kBits >= BitsInByte; kBits -= BitsInByte )
            result = ( result << BitsInByte ) | getNextByte();
        if( kBits > 0 )
            return ( result << kBits ) | ( ( readByte_ = getNextByte() ) >>/*>*/ ( toRead_ = BitsInByte - kBits ) );
        else
        {
            toRead_ = 0;
            return result;
        }
    }
}


sal_Int32 Decompressor::ascendingDecode( sal_Int32 k,sal_Int32 start,sal_Int32* array )
  throw( xmlsearch::excep::XmlSearchException )
{
  sal_Int32 path = 0, index = 0;
 LOOP:
  while( true )
    if( read() )
      array[index++] = (start += path | read(k));
    else
      for( sal_Int32 cnt = 0;; readByte_ = getNextByte(), toRead_ = BitsInByte )
    while( toRead_-- > 0 )
      if( readByte_ & ( 1 << toRead_ ) )
        {
          ++cnt;
          sal_Int32 Path = ( (path >>/*>*/ ( k + cnt ) << cnt ) | read( cnt ) ) << k;
          if (Path != path)
        {
          array[index++] = ( start += ( path = Path ) | read( k ) );
          goto LOOP;
        }
          else
        return index;
        }
      else
        ++cnt;
}



void Decompressor::ascDecode( sal_Int32 k,std::vector< sal_Int32 >& array) throw( excep::XmlSearchException )
{
  for( sal_Int32 path = 0,start = 0; ; )
    if( read() != 0 )
      array.push_back( start += path | read( k ) );
    else
      {
    sal_Int32 count = countZeroes() + 1;
    sal_Int32 saved = path;
    path = ( ( path >>/*>*/ ( k + count ) << count) | read( count ) ) << k;
    if( path != saved ) // convention for end
      array.push_back( start += path | read( k ) );
    else
      break;
      }
}


void Decompressor::decode( sal_Int32 k,std::vector< sal_Int32 >& array) throw( excep::XmlSearchException )
{
  for( sal_Int32 path = 0; ; )
    if( read() )
      array.push_back( path | read( k ) );
    else
      {
    sal_Int32 count = countZeroes() + 1;
    sal_Int32 saved = path;
    path = ( ( path >>/*>*/ ( k + count ) << count ) | read( count ) ) << k;
    if( path != saved ) // convention for end
      array.push_back( path | read( k ) );
    else
      break;
      }
}



void Decompressor::decode(sal_Int32 k, sal_Int32* array ) throw( excep::XmlSearchException )
{
  for( sal_Int32 path = 0,index = 0; ; )
    if( read() )
      array[ index++] = ( path | read( k ) );
    else
      {
    sal_Int32 count = countZeroes() + 1;
    sal_Int32 saved = path;
    path = ( ( path >>/*>*/ ( k + count ) << count ) | read( count ) ) << k;
    if( path != saved ) // convention for end
      array[ index++ ] = ( path | read( k ) );
    else
      break;
      }
}



sal_Int32 Decompressor::read() throw( excep::XmlSearchException )
{
  if( toRead_-- > 0 )
    return readByte_ & ( 1 << toRead_ );
  else
    {  // get next word
      toRead_ = BitsInByte - 1;
      return ( readByte_ = getNextByte() ) & 0x80;
    }
}


sal_Int32 Decompressor::countZeroes() throw( excep::XmlSearchException )
{
  for( sal_Int32 count = 0; ;readByte_ = getNextByte(),toRead_ = BitsInByte )
    while( toRead_-- > 0 )
      if( readByte_ & ( 1 << toRead_ ) )
    return count;
      else
    ++count;
}


bool Decompressor::readNext( sal_Int32 k,CompressorIterator* it) throw( excep::XmlSearchException )
{
  if( read() != 0 )
    {
      it->value( path_ | read(k) );
      return true;
    }
  else
    for( sal_Int32 count = 1; ; readByte_ = getNextByte(), toRead_ = BitsInByte )
      while( toRead_-- > 0 )
    if( readByte_ & ( 1 << toRead_ ) )
      {
        sal_Int32 saved = path_;
        path_ = ( (path_ >>/*>*/ ( k + count ) << count ) | read( count ) ) << k;
        if( path_ != saved )
          {
        it->value( path_ | read( k ) );
        return true;
          }
        else
          return false;
    }
    else
      ++count;
}


sal_Int32 StreamDecompressor::getNextByte()
{
  sal_Int8 a[4];
  a[0] = a[1] = a[2] = 0;
  if( in_->readBytes( &a[3],1 ) != -1 )
    return ::getInteger_( a );
  else
    return -1;
}
