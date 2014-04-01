/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _XMLSEARCH_UTIL_DECOMPRESSOR_HXX_
#define _XMLSEARCH_UTIL_DECOMPRESSOR_HXX_

#include <vector>
#include <excep/XmlSearchExceptions.hxx>
#include <util/RandomAccessStream.hxx>

namespace xmlsearch {

    namespace util {


        class Decompressor
        {
        public:

            Decompressor()
                : toRead_( 0 )
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

            sal_Int32 toRead_;
        };




        class StreamDecompressor
            : public Decompressor
        {
        public:

            StreamDecompressor( RandomAccessStream* in )
                : in_( in )
            {
            }

            virtual ~StreamDecompressor() { }


            virtual sal_Int32 getNextByte() SAL_OVERRIDE;

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


            virtual ~ByteArrayDecompressor() { }

            sal_Int32 bytesRead()
            {
                return index_ - index0_;
            }


            sal_Int32 getNextByte() throw( xmlsearch::excep::XmlSearchException ) SAL_OVERRIDE
            {
                if( arrayL_ <= index_ )
                    throw  xmlsearch::excep::XmlSearchException(
                        OUString( "ByteArrayDecompressor->getNextByte()" ) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
