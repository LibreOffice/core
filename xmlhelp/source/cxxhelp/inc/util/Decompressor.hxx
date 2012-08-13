/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ByteArrayDecompressor->getNextByte()" )) );
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
