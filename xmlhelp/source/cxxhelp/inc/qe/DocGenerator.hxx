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


#ifndef _XMLSEARCH_QE_DOCGENERATOR_HXX_
#define _XMLSEARCH_QE_DOCGENERATOR_HXX_

#include <rtl/ref.hxx>
#ifndef _RTL_USTRING_HXX__
#include  <rtl/ustring.hxx>
#endif
#include <excep/XmlSearchExceptions.hxx>
#include <util/CompressorIterator.hxx>
#include <util/Decompressor.hxx>


namespace xmlsearch {

    namespace qe {


        class Query;


        class NonnegativeIntegerGenerator
        {
        public:

            static const sal_Int32 END;

            virtual ~NonnegativeIntegerGenerator() { };
            virtual sal_Int32 first() throw( xmlsearch::excep::XmlSearchException ) = 0;
            virtual sal_Int32 next() throw( xmlsearch::excep::XmlSearchException ) = 0;
        };


        class RoleFiller
        {
        public:

            static RoleFiller* STOP() { return &roleFiller_; }

            RoleFiller();

            ~RoleFiller();

            void acquire() { ++m_nRefcount; }
            void release() { if( ! --m_nRefcount ) delete this; }

        private:

            static RoleFiller roleFiller_;

            sal_uInt32     m_nRefcount;
            sal_uInt8      fixedRole_;
            sal_Int16      filled_;
            sal_Int32      begin_,end_,parentContext_,limit_;

            RoleFiller*    next_;
            std::vector< RoleFiller* > fillers_;
        };


        class GeneratorHeap
        {
        public:

            GeneratorHeap()
                : heapSize_( 0 ),
                  free_( 0 )
            { }
        private:

            sal_Int32   heapSize_,free_;
        };



    }
}

#endif
