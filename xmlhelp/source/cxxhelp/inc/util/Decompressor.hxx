/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Decompressor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:19:12 $
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
#ifndef _XMLSEARCH_UTIL_DECOMPRESSOR_HXX_
#define _XMLSEARCH_UTIL_DECOMPRESSOR_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif
#ifndef _XMLEARCH_UTIL_RANDOMACCESSSTREAM_HXX_
#include <util/RandomAccessStream.hxx>
#endif


namespace xmlsearch {

    namespace util {


        class CompressorIterator;


        class Decompressor
        {
        public:

            Decompressor()
                : toRead_( 0 ),
                  path_( 0 )
            {
            }

            virtual ~Decompressor() { }

            virtual sal_Int32 getNextByte() = 0;

            sal_Int32 read( sal_Int32 kBits ) throw( xmlsearch::excep::XmlSearchException );

            sal_Int32 ascendingDecode( sal_Int32 k,sal_Int32 start,sal_Int32 *concepts )
                throw( xmlsearch::excep::XmlSearchException );

            void ascDecode( sal_Int32 k,std::vector< sal_Int32 >& array) throw( xmlsearch::excep::XmlSearchException );

            void decode(sal_Int32 k, sal_Int32* array ) throw( xmlsearch::excep::XmlSearchException );

            void decode( sal_Int32 k,std::vector< sal_Int32 >& array) throw( xmlsearch::excep::XmlSearchException );

            virtual void initReading()
            {
                toRead_ = 0;
            }

            bool readNext( sal_Int32 k,CompressorIterator* it) throw( xmlsearch::excep::XmlSearchException );

        private:

            static const sal_Int32 BitsInByte;
            static const sal_Int32 NBits;

            sal_Int32 readByte_, toRead_, path_;

            sal_Int32 read() throw( xmlsearch::excep::XmlSearchException );

            sal_Int32 countZeroes() throw( xmlsearch::excep::XmlSearchException );

        };




        class StreamDecompressor
            : public Decompressor
        {
        public:

            StreamDecompressor( RandomAccessStream* in )
                : in_( in )
            {
            }

            ~StreamDecompressor() { }


            virtual sal_Int32 getNextByte();

        private:

            RandomAccessStream* in_;

        };



        class ByteArrayDecompressor
            : public Decompressor
        {
        public:

            ByteArrayDecompressor( sal_Int32 arrayL,sal_Int8* array,sal_Int32 index )
            {
                initReading(array,arrayL,index);
            }


            ~ByteArrayDecompressor() { }

            sal_Int32 bytesRead()
            {
                return index_ - index0_;
            }


            sal_Int32 getNextByte() throw( xmlsearch::excep::XmlSearchException )
            {
                if( arrayL_ <= index_ )
                    throw  xmlsearch::excep::XmlSearchException(
                        rtl::OUString::createFromAscii( "ByteArrayDecompressor->getNextByte()" ) );
                return array_[index_++] & 0xFF;
            }


        private:

            sal_Int32  arrayL_;
            sal_Int8   *array_;

            sal_Int32  index_,index0_;

            using xmlsearch::util::Decompressor::initReading;
            void initReading( sal_Int8* array,sal_Int32 arrayL,sal_Int32 index )
            {
                arrayL_ = arrayL;
                array_ = array;
                index_ = index0_ = index;
                Decompressor::initReading();
            }

        };


    }

}


#endif
