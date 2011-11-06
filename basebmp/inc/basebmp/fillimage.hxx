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



#ifndef INCLUDED_BASEBMP_FILLIMAGE_HXX
#define INCLUDED_BASEBMP_FILLIMAGE_HXX

#include <vigra/tuple.hxx>
#include <vigra/iteratortraits.hxx>

namespace basebmp
{

template< class DestIterator, class DestAccessor, typename T >
void fillImage( DestIterator begin,
                DestIterator end,
                DestAccessor ad,
                T            fillVal )
{
    const int width ( end.x - begin.x );
    const int height( end.y - begin.y );

    for( int y=0; y<height; ++y, ++begin.y )
    {
        typename vigra::IteratorTraits<DestIterator>::row_iterator
            rowIter( begin.rowIterator() );
        const typename vigra::IteratorTraits<DestIterator>::row_iterator
            rowEnd( rowIter + width );

        // TODO(P2): Provide specialized span fill methods on the
        // iterator/accessor
        while( rowIter != rowEnd )
            ad.set(fillVal, rowIter++);
    }
}

template< class DestIterator, class DestAccessor, typename T >
inline void fillImage( vigra::triple<DestIterator,DestIterator,DestAccessor> const& src,
                       T                                                            fillVal )
{
    fillImage(src.first,src.second,src.third,fillVal);
}

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_FILLIMAGE_HXX */
