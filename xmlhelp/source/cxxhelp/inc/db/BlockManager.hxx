/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BlockManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:15:50 $
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
#ifndef _XMLSEARCH_DB_BLOCKMANAGER_HXX_
#define _XMLSEARCH_DB_BLOCKMANAGER_HXX_

#ifndef _RTL_USTRING_
#include  <rtl/ustring.hxx>
#endif
#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _XMLSEARCH_DB_BLOCK_HXX_
#include <db/Block.hxx>
#endif
#ifndef _XMLSEARCH_DB_BLOCKFACTORY_HXX_
#include <db/BlockFactory.hxx>
#endif
#ifndef _XMLSEARCH_DB_DBENV_HXX_
#include <db/DBEnv.hxx>
#endif
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif

namespace xmlsearch {

    namespace util {

        class RandomAccessStream;

    }

    namespace db {


        class BlockDecoratour // internal class for BlockManager
        {
        public:

            BlockDecoratour()
                : lock_( false ),
                  modf_( false ),
                  prev_( 0 ),
                  next_( 0 ),
                  block_( 0 )
            {
            }

            ~BlockDecoratour() { delete block_; }

            void lock() { lock_ = true; }
            void unlock() { lock_ = false; }
            bool isLocked() const { return lock_; }

            bool isModf() const { return modf_; }
            void modf() { modf_ = true; }

            void next( sal_Int32 parnext ) { next_ = parnext; }
            sal_Int32 next() const { return next_; }
            void prev( sal_Int32 parprev ) {  prev_ = parprev; }
            sal_Int32 prev() const { return prev_; }

            void read( DBEnv* dbenv,sal_Int32 blNum ) { dbenv->read( blNum,block_ ); }
            void write(DBEnv* dbenv,sal_Int32 blNum ) { dbenv->write( blNum,block_ ); }

            Block* reset() { lock_ = modf_ = false; prev_ = next_ = 0; Block* tmp = block_; block_ = 0; return tmp; }
            void add( Block* block ) { delete block_; block_ = block; }
            bool is() const { return block_ != 0; }

            const Block* getBlock() const { return block_; }

        private:

            bool                  lock_;
            bool                  modf_;
            sal_Int32             prev_;
            sal_Int32             next_;
            Block*                block_;

        };



        class BlockProcessor
        {
        public:

            virtual void process( Block* block ) const = 0;
            virtual ~BlockProcessor() {}
        };


        class BlockManager
        {
        public:

            // Buffered mapping of stream into memory

            BlockManager( DBEnv* ) throw( xmlsearch::excep::IOException );
            virtual ~BlockManager();

            // From BlockManagerBase
            virtual void lock( sal_Int32   blNum ) throw( xmlsearch::excep::IllegalIndexException );
            virtual void unlock( sal_Int32 blNum ) throw( xmlsearch::excep::IllegalIndexException );
            virtual bool isLocked( sal_Int32 blNum ) throw( xmlsearch::excep::IllegalIndexException );

            virtual void modf( sal_Int32 blNum ) throw( xmlsearch::excep::IllegalIndexException );
            virtual void close() throw( xmlsearch::excep::IOException );

            virtual const Block* accessBlock( sal_Int32 blNum ) throw( xmlsearch::excep::NoSuchBlock );
            virtual void mapBlocks( const BlockProcessor& blProc );


        private:

            // types

            xmlsearch::db::DBEnv*                  dbenv_;

            sal_Int32                              oldest_;
            sal_Int32                              newest_;
            BlockDecoratour                        dummy_;
            sal_Int32                              blockused_;
            std::vector< BlockDecoratour >         blockTable_;


            // private methods

            void moveToFront( sal_Int32 index );
            void mapStorage2Memory( sal_Int32 blNum ) throw( xmlsearch::excep::IOException );
            void remap( sal_Int32 blNum ) throw( xmlsearch::excep::XmlSearchException );
            void add( sal_Int32 blNum );
        };


    }
}



#endif
