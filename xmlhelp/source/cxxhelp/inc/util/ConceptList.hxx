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
#ifndef _XMLSEARCH_UTIL_CONCEPTLIST_HXX_
#define _XMLSEARCH_UTIL_CONCEPTLIST_HXX_

#include <excep/XmlSearchExceptions.hxx>
#include <qe/DocGenerator.hxx>
#include <util/Decompressor.hxx>
#include <util/CompressorIterator.hxx>


namespace xmlsearch {

    namespace util {


        class CompressorIterator;



        class ConceptList
            : public CompressorIterator,
              public xmlsearch::qe::NonnegativeIntegerGenerator
        {
        public:

            ConceptList( sal_Int8* array,sal_Int32 arrl,sal_Int32 index )
                : k_( array[index] ),
                  value_( 0 ),
                  list_( arrl,array,index + 1 )  { }

            void value( sal_Int32 val ) { value_ += val; }

        private:

            sal_Int32             k_,value_;
            ByteArrayDecompressor list_;

        };

    }

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
