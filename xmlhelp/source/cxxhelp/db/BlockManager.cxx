/*************************************************************************
 *
 *  $RCSfile: BlockManager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: abi $ $Date: 2001-06-06 14:48:47 $
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
#ifndef _XMLSEARCH_DB_BLOCKMANAGER_HXX_
#include <db/BlockManager.hxx>
#endif
#ifndef _XMLEARCH_UTIL_RANDOMACCESSSTREAM_HXX_
#include <util/RandomAccessStream.hxx>
#endif



using namespace ::xmlsearch;
using namespace ::xmlsearch::db;



BlockManager::BlockManager( DBEnv* dbenv )
    : oldest_( 0 ),
      newest_( 0 ),
      dbenv_( dbenv ),
      blockused_( 0 ),
      blockTable_( dbenv_ ? dbenv_->getBlockCount() : 0 )   // Hardcoded headerlen?
{
    mapStorage2Memory( 0 );
}



BlockManager::~BlockManager()
{
    delete dbenv_;
}



void BlockManager::lock( sal_Int32 blNum ) throw( excep::IllegalIndexException )
{
    if( blNum < 0 ||  blockTable_.size() <= sal_uInt32( blNum ) )
        throw excep::IllegalIndexException( rtl::OUString::createFromAscii( "BlockManager::lock ->" ) );

    blockTable_[ blNum ].lock();
}



void BlockManager::unlock( sal_Int32 blNum ) throw( excep::IllegalIndexException )
{
    if( blNum < 0 ||  blockTable_.size() <= sal_uInt32( blNum ) )
        throw excep::IllegalIndexException( rtl::OUString::createFromAscii( "BlockManager::unlock ->" ) );

    blockTable_[ blNum ].unlock();
}


bool BlockManager::isLocked( sal_Int32 blNum ) throw( excep::IllegalIndexException )
{
    if( blNum < 0 ||  blockTable_.size() <= sal_uInt32( blNum ) )
        throw excep::IllegalIndexException( rtl::OUString::createFromAscii( "BlockManager::isLocked ->" ) );

    return blockTable_[ blNum ].isLocked();
}


void BlockManager::modf( sal_Int32 blNum ) throw( excep::IllegalIndexException )
{
    if( blNum < 0 ||  blockTable_.size() <= sal_uInt32( blNum ) )
        throw excep::IllegalIndexException( rtl::OUString::createFromAscii( "BlockManager::modf ->" ) );

    blockTable_[ blNum ].modf();
}


void BlockManager::close() throw( excep::IOException )
{
    if( ! dbenv_ )
        throw excep::IOException( rtl::OUString::createFromAscii( "BlockManager::save -> no environment" ) );

    for( sal_uInt32 i = 0; i < blockTable_.size(); ++i )
        if( blockTable_[i].isModf() )
            blockTable_[i].write( dbenv_,i );
}



const Block* BlockManager::accessBlock( sal_Int32 blNum ) throw( excep::NoSuchBlock )
{
    if( blNum < 0 || blockTable_.size() <= sal_uInt32( blNum ) )
        throw excep::IllegalIndexException(
            rtl::OUString::createFromAscii(
                "BlockManager:accessBlock -> index out of range" ) );

    if( blockTable_[ blNum ].is() )
        moveToFront( blNum );
    else if( blockused_ < dbenv_->getMaximumBlockCount() )
        mapStorage2Memory( blNum );
    else
        remap( blNum );

    return blockTable_[ blNum ].getBlock( );
}



void BlockManager::moveToFront( sal_Int32 blNum )
{
    if( blNum == oldest_ )
    {
        oldest_ = blockTable_[blNum].next();
        blockTable_[blNum].prev( newest_ );
        blockTable_[newest_].next( blNum );
        newest_ = blNum;
    }
    else if( blNum != newest_ )
    {
        blockTable_[blockTable_[blNum].next()].prev( blockTable_[blNum].prev() );
        blockTable_[blockTable_[blNum].prev()].next( blockTable_[blNum].next() );
        blockTable_[blNum].prev( newest_ );
        blockTable_[newest_].next( blNum );
        newest_ = blNum;
    }
}


// maps file into buffer

void BlockManager::mapStorage2Memory( sal_Int32 blNum ) throw( excep::IOException )
{
    add( blNum );
    blockTable_[blNum].read( dbenv_,blNum );
}



void BlockManager::remap( sal_Int32 blNum ) throw( excep::XmlSearchException )
{
    sal_Int32 idx = oldest_;

    while( blockTable_[ idx ].isLocked() && idx != newest_ )
        idx = blockTable_[ idx ].next();

    if( blockTable_[ idx ].isLocked() )
        throw excep::XmlSearchException(
            rtl::OUString::createFromAscii( "everything locked" ) );

    if( blockTable_[ idx].isModf() )
        blockTable_[ idx ].write( dbenv_,idx );

    // delete from double-linked list
    if( idx == oldest_ )
        oldest_ = blockTable_[ idx ].next();
    else if( idx == newest_ )
        newest_ = blockTable_[ idx ].prev();
    else
    {
        blockTable_[ blockTable_[ idx ].next() ].prev( blockTable_[ idx ].prev() );
        blockTable_[ blockTable_[ idx ].prev() ].next( blockTable_[ idx ].next() );
    }

    add( blNum );
    blockTable_[blNum].add( blockTable_[ idx ].reset() );
}



void BlockManager::add( sal_Int32 blNum )
{
    blockTable_[ blNum ].prev( newest_ );
    blockTable_[ newest_ ].next( blNum );
    newest_ = blNum;
}


void BlockManager::mapBlocks( const BlockProcessor& blProc )
{
    sal_Int32 nBlocks = dbenv_->getBlockCount();
    Block* block = 0;
    for (int i = 0; i < nBlocks; i++)
    {
        dbenv_->read( i,block );
        blProc.process( block );
    }
}



