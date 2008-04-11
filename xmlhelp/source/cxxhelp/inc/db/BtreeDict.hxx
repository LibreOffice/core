/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BtreeDict.hxx,v $
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
#ifndef _XMLSEARCH_DB_BTREEDICT_HXX_
#define _XMLSEARCH_DB_BTREEDICT_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#include  <rtl/ustring.hxx>
#include <excep/XmlSearchExceptions.hxx>
#include <db/BlockManager.hxx>
#include <db/EntryProcessor.hxx>
#include <util/IndexAccessor.hxx>

namespace xmlsearch {

    namespace db {


        class DictBlock;
        class DBEnv;


        class BtreeDict
        {
        public:

            static const sal_Int32 ENTHEADERLEN;
            static const sal_Int32 BLOCKSIZE;
            static const sal_Int32 HEADERLEN;
            static const sal_Int32 DATALEN;

            static const sal_Int32 nBlocksLimit;
            static const sal_Int32 MaxKeyLength;
            static const sal_Int32 lastPtrIndex;   //!!! Careful with that number, Eugene

            BtreeDict( const xmlsearch::util::IndexAccessor& indexAccessor )
                throw( xmlsearch::excep::IOException );
            ~BtreeDict();

            void lock( sal_Int32   blNum ) const throw( xmlsearch::excep::IllegalIndexException );
            void unlock( sal_Int32 blNum ) const throw( xmlsearch::excep::IllegalIndexException );
            bool isLocked( sal_Int32 blNum ) const throw( xmlsearch::excep::IllegalIndexException );

            const DictBlock* accessBlock( sal_Int32 id ) const;

            sal_Int32 fetch( const rtl::OUString& key ) const throw( xmlsearch::excep::XmlSearchException );
            rtl::OUString fetch( sal_Int32 conceptID ) const throw( xmlsearch::excep::XmlSearchException );

            std::vector< sal_Int32 > withPrefix( const rtl::OUString& prefix ) const throw( xmlsearch::excep::XmlSearchException );
            sal_Int32* get_blocks() const { return blocks_; }

            void test();

        private:

            mutable BlockManager         blockManager_;
            sal_Int32                    root_;
            sal_Int32                    blocksL_;
            sal_Int32*                   blocks_;

            sal_Int32 find( const DictBlock* bl,
                            const sal_Int8* key,
                            sal_Int32 inputKeyLen ) const throw( xmlsearch::excep::XmlSearchException );

            sal_Int32 find( const DictBlock* bl,
                            const sal_Int8* key,
                            sal_Int32 inputKeyLen,
                            sal_Int32 index ) const throw( xmlsearch::excep::XmlSearchException );

            const DictBlock* child( const DictBlock* bl,
                                    sal_Int32 index) const throw( xmlsearch::excep::XmlSearchException );

            rtl::OUString findID( sal_Int32 blNum,sal_Int32 id ) const throw( xmlsearch::excep::XmlSearchException );

        };


    }
}


#endif
