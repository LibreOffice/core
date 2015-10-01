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

#ifndef INCLUDED_BASEBMP_INC_ACCESSORTRAITS_HXX
#define INCLUDED_BASEBMP_INC_ACCESSORTRAITS_HXX

#include <accessorfunctors.hxx>
#include <accessoradapters.hxx>
#include <metafunctions.hxx>

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

/// given an Accessor and its value type return its value_type
template< typename Accessor > struct ColorPassThrough
{
    typename Accessor::value_type operator()( const Accessor&,
            const typename Accessor::value_type& x ) const
    {
        return x;
    }
};

/** Traits template for Accessor

    Provides wrapped types for color lookup, raw pixel access, xor and
    mask accessors.
 */
template< class Accessor > struct AccessorTraits
{
    /// value type of described accessor
    typedef typename Accessor::value_type           value_type;

    /// Retrieve stand-alone color lookup function for given Accessor type
    typedef ColorPassThrough< Accessor > color_lookup;

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

#endif /* INCLUDED_BASEBMP_INC_ACCESSORTRAITS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
