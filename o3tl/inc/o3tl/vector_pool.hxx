/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lazy_update.hxx,v $
 * $Revision: 1.3 $
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
