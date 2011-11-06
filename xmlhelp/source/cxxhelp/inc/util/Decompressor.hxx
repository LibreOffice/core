/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _XMLSEARCH_UTIL_DECOMPRESSOR_HXX_
#define _XMLSEARCH_UTIL_DECOMPRESSOR_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#include <excep/XmlSearchExceptions.hxx>
#include <util/RandomAccessStream.hxx>


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

            virtual void initReading()
            {
                toRead_ = 0;
            }

        private:

            static const sal_Int32 BitsInByte;
            static const sal_Int32 NBits;

            sal_Int32 readByte_, toRead_, path_;
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
