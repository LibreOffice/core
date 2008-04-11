/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BlockManager.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _XMLSEARCH_DB_BLOCKMANAGER_HXX_
#define _XMLSEARCH_DB_BLOCKMANAGER_HXX_

#include  <rtl/ustring.hxx>
#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#include <db/Block.hxx>
#include <db/BlockFactory.hxx>
#include <db/DBEnv.hxx>
#include <excep/XmlSearchExceptions.hxx>

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
