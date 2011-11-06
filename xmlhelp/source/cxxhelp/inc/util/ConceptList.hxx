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
