/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Block.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _XMLSEARCH_DB_BLOCK_HXX_
#define _XMLSEARCH_DB_BLOCK_HXX_

#include  <rtl/ustring.hxx>
#include <excep/XmlSearchExceptions.hxx>


namespace xmlsearch {

    namespace util {

        class RandomAccessStream;

    }

    namespace db {


        class DBEnv;


        class Block     // refcounted class
        {
        public:

            Block( const DBEnv* );
            virtual ~Block();

            sal_Int32 getInteger( sal_Int32 i ) const throw( xmlsearch::excep::IllegalIndexException );

            void trueLeaf() { isLeaf_ = true; }
            void falseLeaf() { isLeaf_ = false; }
            bool isLeaf() const { return isLeaf_; }

            void setFree( sal_Int32 free ) { free_ = free; }
            sal_Int32 getFree() const { return free_; }

            void setNum( sal_Int32 num ) { num_ = num; }
            sal_Int32 getNum() const { return num_; }

            void read( util::RandomAccessStream* in );

            const sal_Int8* getData() const { return data_; }


        protected:

            const DBEnv*  dbenv_;   // not owner


        private:

            bool          isLeaf_;
            sal_Int32     free_;
            sal_Int32     num_;
            sal_Int32     dataL_;
            sal_Int8*     data_;

            // methods

            Block( const Block& );
            Block& operator=( const Block& );

        };  // end class Block


    }
}


inline sal_Int32 getInteger_( const sal_Int8* v )
{
    return (((((( (v[0]&0xFF) ) << 8 )
               | (v[1]&0xFF) ) << 8 )
             | (v[2]&0xFF) ) << 8 )
        | (v[3]&0xFF);
}



inline void setInteger_( sal_Int8* v,sal_Int32 value )
{
    v[0] = sal_Int8((value >> 24) & 0xFF);
    v[1] = sal_Int8((value >> 16) & 0xFF);
    v[2] = sal_Int8((value >>  8) & 0xFF);
    v[3] = sal_Int8( value & 0xFF );
}


#endif
