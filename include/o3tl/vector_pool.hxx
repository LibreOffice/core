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
                    this->push_back(value_type(rCopy));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
