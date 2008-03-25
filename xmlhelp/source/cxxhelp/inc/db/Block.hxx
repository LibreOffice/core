/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Block.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:15:19 $
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
#ifndef _XMLSEARCH_DB_BLOCK_HXX_
#define _XMLSEARCH_DB_BLOCK_HXX_

#ifndef _RTL_USTRING_
#include  <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif


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

            void setInteger( sal_Int32 i,sal_Int32 value ) throw( xmlsearch::excep::IllegalIndexException );
            sal_Int32 getInteger( sal_Int32 i ) const throw( xmlsearch::excep::IllegalIndexException );

            void trueLeaf() { isLeaf_ = true; }
            void falseLeaf() { isLeaf_ = false; }
            bool isLeaf() const { return isLeaf_; }

            void setFree( sal_Int32 free ) { free_ = free; }
            sal_Int32 getFree() const { return free_; }

            void setNum( sal_Int32 num ) { num_ = num; }
            sal_Int32 getNum() const { return num_; }

            void read( util::RandomAccessStream* in );
            void write( util::RandomAccessStream* out ) const;

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
    return (((((( v[0]&0xFF ) << 8 )
               | v[1]&0xFF ) << 8 )
             | v[2]&0xFF ) << 8 )
        | v[3]&0xFF;
}



inline void setInteger_( sal_Int8* v,sal_Int32 value )
{
    v[0] = sal_Int8((value >> 24) & 0xFF);
    v[1] = sal_Int8((value >> 16) & 0xFF);
    v[2] = sal_Int8((value >>  8) & 0xFF);
    v[3] = sal_Int8( value & 0xFF );
}


#endif
