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


#ifndef _XMLEARCH_UTIL_RANDOMACCESSSTREAM_HXX_
#define _XMLEARCH_UTIL_RANDOMACCESSSTREAM_HXX_

#include <osl/file.hxx>

namespace xmlsearch {

    namespace util {


        class RandomAccessStream
        {
        public:

            virtual ~RandomAccessStream() { };

            // The calle is responsible for allocating the buffer
            virtual void seek( sal_Int32 ) = 0;
            virtual sal_Int32 readBytes( sal_Int8*,sal_Int32 ) = 0;
            virtual void writeBytes( sal_Int8*, sal_Int32 ) = 0;
            virtual sal_Int32 length() = 0;
            virtual void close() = 0;


        protected:

            enum OPENFLAG { Read = OpenFlag_Read,
                            Write = OpenFlag_Write,
                            Create = OpenFlag_Create };

        };


    }
}

#endif
