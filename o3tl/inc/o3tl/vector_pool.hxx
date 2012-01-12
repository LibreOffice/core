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



#ifndef INCLUDED_O3TL_VECTOR_POOL_HXX
#define INCLUDED_O3TL_VECTOR_POOL_HXX

#include <sal/types.h>
#include <vector>

namespace o3tl
{
    namespace detail
    {
        template<typename ValueType, class Container> class simple_pool_impl :
            public Container
        {
            typedef typename Container::value_type value_type;
            std::ptrdiff_t mnFirstFreeIndex;

        public:
            simple_pool_impl() :
                mnFirstFreeIndex(-1)
            {}

            std::ptrdiff_t alloc()
            {
                return store(ValueType());
            }

            std::ptrdiff_t store(const ValueType& rCopy)
            {
                if( mnFirstFreeIndex != -1 )
                {
                    std::ptrdiff_t nIdx=mnFirstFreeIndex;
                    mnFirstFreeIndex = this->at(mnFirstFreeIndex).nextFree;
                    this->at(nIdx).value = rCopy;
                    this->at(nIdx).nextFree = -1;

                    return nIdx;
                }
                else
                {
                    push_back(value_type(rCopy));
                    return this->size()-1;
                }
            }

            void free( std::ptrdiff_t nIdx )
            {
                this->at(nIdx).nextFree = mnFirstFreeIndex;
                mnFirstFreeIndex = nIdx;
            }

            const ValueType& get( std::ptrdiff_t nIdx ) const
            {
                return this->operator[](nIdx).value;
            }
            ValueType& get( std::ptrdiff_t nIdx )
            {
                return this->operator[](nIdx).value;
            }
        };

        template< typename ValueType > struct struct_from_value
        {
            struct type
            {
                type() :
                    value(),
                    nextFree(-1)
                {}
                explicit type( const ValueType& val ) :
                    value(val),
                    nextFree(-1)
                {}

                ValueType      value;
                std::ptrdiff_t nextFree;
            };
        };
    }

    /** Simple vector-based memory pool allocator

        This template can be used to provide simple pooled memory
        allocation from a container class that adheres to the stl
        random access container concept. Note that alloc/free works
        with _indices_ into the container!

        @example
        <pre>
vector_pool<type> myPool;
int nIdx=myPool.alloc();
myPool[nIdx] = myVal;
 ... do stuff ...
myPool.free(nIdx);
        </pre>
     */
    template<typename ValueType> struct vector_pool :
        public detail::simple_pool_impl<ValueType,
                                       std::vector<typename detail::struct_from_value<ValueType>::type > >
    {};
}

#endif /* INCLUDED_O3TL_VECTOR_POOL_HXX */
