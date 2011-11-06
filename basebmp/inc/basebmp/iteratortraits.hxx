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



#ifndef INCLUDED_BASEBMP_ITERATORTRAITS_HXX
#define INCLUDED_BASEBMP_ITERATORTRAITS_HXX

#include <basebmp/accessor.hxx>
#include <basebmp/nonstandarditerator.hxx>

namespace basebmp
{

template< class Iterator > struct IteratorTraits
{
    /// VigraTrueType, if iterator does not provide *operator()/operator[] methods
    typedef typename vigra::IsDerivedFrom<Iterator,NonStandardIterator>::result
            isNonStandardIterator;

    /// Retrieve default accessor for this iterator (and given value type)
    template< typename ValueType > struct defaultAccessor : public
        // select according to non-standardness of iterator type
        vigra::If< isNonStandardIterator,
            NonStandardAccessor< ValueType >,
            StandardAccessor< ValueType > >
    {};

};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ITERATORTRAITS_HXX */
