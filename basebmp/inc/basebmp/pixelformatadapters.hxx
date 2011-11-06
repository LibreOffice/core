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



#ifndef INCLUDED_BASEBMP_PIXELFORMATADAPTERS_HXX
#define INCLUDED_BASEBMP_PIXELFORMATADAPTERS_HXX

#include <basebmp/accessortraits.hxx>
#include <basebmp/accessoradapters.hxx>

#include <vigra/metaprogramming.hxx>

namespace basebmp
{

// convenience functionality, providing everything necessary for a new
// pixel format. simply plug in two conversion functors from/to a
// common color format.

/** Accessor selection metafunction, used to wrap a given accessor
    with one converting between the pixel and color types

    Use the nested template's typedef type, to retrieve an
    AccessorAdapter which operates on a pixel value accessor, and
    provides color values to the outside.

    Nested like this, to avoid template template parameters at other
    places: an instantiated version of AccessorSelector can be passed
    to other templates, which in turn can invoke the nested meta
    function.
 */
template< typename Getter,
          typename Setter > struct AccessorSelector
{
    template< typename Accessor > struct wrap_accessor
    {
        typedef UnaryFunctionAccessorAdapter< Accessor,
                                              Getter,
                                              Setter > type;
    };
};

//-----------------------------------------------------------------------------

/** Convert color value to pixel data type
 */
template< class Accessor, typename DataType > struct ColorConvert
{
    DataType operator()( const Accessor&               acc,
                         typename Accessor::value_type v ) const
    {
        return acc.setter(v);
    }
};

//-----------------------------------------------------------------------------

/** Macro generates partial specialization for color-conversion
    UnaryFunctionAccessorAdapter, and the given getter/setter functors
 */
#define BASEBMP_SPECIALIZE_ACCESSORTRAITS(Getter,Setter)          \
template< class Accessor > struct AccessorTraits<                 \
    UnaryFunctionAccessorAdapter< Accessor,                       \
                                  Getter,                         \
                                  Setter > >                      \
{                                                                 \
    typedef typename Accessor::value_type          data_type;     \
    typedef UnaryFunctionAccessorAdapter<                         \
        Accessor,                                                 \
        Getter,                                                   \
        Setter >                                   accessor_type; \
    typedef typename accessor_type::value_type     value_type;    \
    typedef ColorConvert< accessor_type,                          \
                          data_type >              color_lookup;  \
    typedef Accessor                               raw_accessor;  \
    typedef vigra::VigraFalseType                  xor_accessor;  \
    template< class MaskAccessor,                                 \
              class Iterator,                                     \
              class MaskIterator > struct          masked_accessor\
    { typedef vigra::VigraFalseType type; };                      \
}

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_PIXELFORMATADAPTERS_HXX */
