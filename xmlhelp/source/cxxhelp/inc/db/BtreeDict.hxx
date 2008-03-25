/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BtreeDict.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:16:02 $
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
#ifndef _XMLSEARCH_DB_BTREEDICT_HXX_
#define _XMLSEARCH_DB_BTREEDICT_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _RTL_USTRING_
#include  <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif
#ifndef _XMLSEARCH_DB_BLOCKMANAGER_HXX_
#include <db/BlockManager.hxx>
#endif
#ifndef _XMLSEARCH_DB_ENTRYPROCESSOR_HXX_
#include <db/EntryProcessor.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_INDEXACCESSOR_HXX_
#include <util/IndexAccessor.hxx>
#endif

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
