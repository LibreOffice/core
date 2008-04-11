/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Block.cxx,v $
 * $Revision: 1.6 $
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
#include <rtl/memory.h>
#include <db/Block.hxx>
#ifndef _XMLSEARCH_UTIL_RANDOMACCESSSTREAM_HXX_
#include <util/RandomAccessStream.hxx>
#endif
#include <db/DBEnv.hxx>


using namespace xmlsearch;
using namespace xmlsearch::db;


/******************************************************************************/
/*                                                                            */
/*                              Block                                         */
/*                                                                            */
/******************************************************************************/


Block::Block( const DBEnv* dbenv )
  : dbenv_( dbenv ),
    isLeaf_( true ),
    free_( 0 ),
    num_( 0 ),
    dataL_( dbenv->getDataLen() ),
    data_( new sal_Int8[ dbenv->getDataLen() ] )
{
    rtl_zeroMemory( data_,dataL_ );
}


Block::~Block()
{
  delete[] data_;
}



void Block::read( util::RandomAccessStream* in )
{
  sal_Int8 buffer[4];
  in->readBytes( buffer,4 );
  num_ = getInteger_( buffer );

  in->readBytes( buffer,4 );
  sal_Int32 twoFields = getInteger_( buffer );

  isLeaf_ = ( twoFields & 0x80000000 ) != 0;
  free_ = twoFields & 0x7FFFFFFF;

  in->readBytes( data_,dbenv_->getDataLen() );
}


void Block::write( util::RandomAccessStream* out ) const
{
  out->writeBytes( (sal_Int8*)(&num_),4 );
  sal_Int32 twoFields = ( free_ | ( isLeaf_ ? 0x80000000 : 0 ) );
  out->writeBytes( (sal_Int8*)(&twoFields),4 );
  out->writeBytes( data_,dbenv_->getDataLen() );
}


void Block::setInteger( sal_Int32 i,sal_Int32 value ) throw( excep::IllegalIndexException )
{
  if( i < 0 || dbenv_->getDataLen() <= i  )
    throw excep::IllegalIndexException(
                       rtl::OUString::createFromAscii( "Block::setInteger -> index out of range" ) );

  ::setInteger_( &data_[i],value );
}


sal_Int32 Block::getInteger( sal_Int32 i ) const throw( excep::IllegalIndexException )
{
  if( i < 0 || dbenv_->getDataLen() <= i  )
    throw excep::IllegalIndexException(
                       rtl::OUString::createFromAscii( "Block::setInteger -> index out of range" ) );

    sal_Int32 ret = ::getInteger_( &data_[i] );
  return ret;
}



