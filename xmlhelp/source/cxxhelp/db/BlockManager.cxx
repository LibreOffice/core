/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BlockManager.cxx,v $
 * $Revision: 1.7 $
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
#include <db/BlockManager.hxx>
#include <util/RandomAccessStream.hxx>



using namespace ::xmlsearch;
using namespace ::xmlsearch::excep;
using namespace ::xmlsearch::db;



BlockManager::BlockManager( DBEnv* dbenv ) throw( IOException )
    : dbenv_( dbenv ),
      oldest_( 0 ),
      newest_( 0 ),
      blockused_( 0 ),
      blockTable_( dbenv_ ? dbenv_->getBlockCount() : 0 )   // Hardcoded headerlen?
{
    if( blockTable_.size() )
        mapStorage2Memory( 0 );
    else
    {
        delete dbenv_;
        throw IOException( rtl::OUString::createFromAscii( "BlockManager::BlockManager -> no blockcount" ) );
    }
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



