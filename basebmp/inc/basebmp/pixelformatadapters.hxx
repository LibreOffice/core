/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pixelformatadapters.hxx,v $
 * $Revision: 1.2 $
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
