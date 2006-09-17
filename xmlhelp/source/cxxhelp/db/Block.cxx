/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Block.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:14:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _XMLSEARCH_DB_BLOCK_HXX_
#include <db/Block.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_RANDOMACCESSSTREAM_HXX_
#include <util/RandomAccessStream.hxx>
#endif
#ifndef _XMLSEARCH_DB_DBENV_HXX_
#include <db/DBEnv.hxx>
#endif


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



