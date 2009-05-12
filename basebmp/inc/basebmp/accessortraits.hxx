/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessortraits.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_BASEBMP_ACCESSORTRAITS_HXX
#define INCLUDED_BASEBMP_ACCESSORTRAITS_HXX

#include <basebmp/accessorfunctors.hxx>
#include <basebmp/accessoradapters.hxx>
#include <basebmp/metafunctions.hxx>

#include <functional>

namespace basebmp
{

struct FastMask;
struct NoFastMask;

/// Metafunction to select output mask functor from iterator and mask value type
template< typename T, typename M, bool polarity, typename DUMMY > struct outputMaskFunctorSelector : public
    ifBothScalarIntegral< T, M,
                          IntegerOutputMaskFunctor< T, M, polarity >,
                          GenericOutputMaskFunctor< T, M, polarity > >
{
};
template< typename T, typename M, bool polarity > struct outputMaskFunctorSelector< T, M, polarity, FastMask > : public
    ifBothScalarIntegral< T, M,
                          FastIntegerOutputMaskFunctor< T, M, polarity >,
                          GenericOutputMaskFunctor< T, M, polarity > >
{
};

/** Metafunction providing a point of configuration for iterators
    capable of employing the fast output mask functor.

    Specialize this metafunction for your case, and pass FastMask to
    the outputMaskFunctorSelector.
 */
template< class Accessor,
          class MaskAccessor,
          class Iterator,
          class MaskIterator,
          bool  polarity > struct maskedAccessorSelector
{
    typedef TernarySetterFunctionAccessorAdapter<
        Accessor,
        MaskAccessor,
        typename outputMaskFunctorSelector<
            typename Accessor::value_type,
            typename MaskAccessor::value_type,
            polarity,
            NoFastMask > ::type >
        type;
};

//-----------------------------------------------------------------------------

/** Traits template for Accessor

    Provides wrapped types for color lookup, raw pixel access, xor and
    mask accessors.
 */
template< class Accessor > struct AccessorTraits
{
    /// value type of described accessor
    typedef typename Accessor::value_type           value_type;

    /// Retrieve stand-alone color lookup function for given Accessor type
    typedef std::project2nd< Accessor, value_type > color_lookup;

    /// Retrieve raw pixel data accessor for given Accessor type
    typedef Accessor                                raw_accessor;

    /// Retrieve wrapped accessor for XOR setter access
    typedef BinarySetterFunctionAccessorAdapter<
        Accessor,
        XorFunctor< value_type > >                  xor_accessor;

    /** Retrieve masked accessor for given types

        A masked accessor works like a filter, where the mask gates
        the accessor's setter methods (if the mask contains a 0 at a
        given iterator position, the original value is
        preserved. Otherwise, the new value gets set).

        @attention be careful when retrieving a masked accessor for a
        set of types, and using it for a different one - there are
        partial specializations that take an optimized functor for
        certain mask accessors.
     */
    template< class MaskAccessor,
              class Iterator,
              class MaskIterator,
              bool  polarity > struct               masked_accessor :
        public maskedAccessorSelector< Accessor,
                                       MaskAccessor,
                                       Iterator,
                                       MaskIterator,
                                       polarity >
    {};

};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ACCESSORTRAITS_HXX */
