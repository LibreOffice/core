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

#ifndef INCLUDED_BASEBMP_INC_ACCESSORFUNCTORS_HXX
#define INCLUDED_BASEBMP_INC_ACCESSORFUNCTORS_HXX

#include <osl/diagnose.h>
#include <metafunctions.hxx>

#include <functional>

namespace basebmp
{

// Some common accessor functors



/// combine two values via XOR
template< typename T > struct XorFunctor : public std::binary_function<T,T,T>
{
    T operator()( T v1, T v2 ) const { return v1 ^ v2; }
};



/// Base class, passing on the arg types
template< typename T, typename M > struct MaskFunctorBase :
        public TernaryFunctorBase<T,M,T,T> {};


/** Let a mask flag decide between two values

    @tpl polarity
    Mask polarity. When true, a false in the mask denotes
    transparency, i.e. the original value will display. And vice
    versa.
 */
template< typename T,
          typename M,
          bool     polarity > struct GenericOutputMaskFunctor : public MaskFunctorBase<T,M>
{
    /// Ternary mask operation - selects v1 for !m == polarity, v2 otherwise
    T operator()( T v1, M m, T v2 ) const
    {
        return !m == polarity ? v1 : v2;
    }
};

/** Let a mask bit decide between two values (specialization for
    integer mask types)
 */
template< typename T,
          typename M,
          bool     polarity > struct IntegerOutputMaskFunctor;
template< typename T,
          typename M > struct IntegerOutputMaskFunctor<T,M,true> : public MaskFunctorBase<T,M>
{
    /** Mask v with state of m

        @return v2, if m != 0, v1 otherwise.
     */
    T operator()( T v1, M m, T v2 ) const
    {
        typedef typename make_unsigned<T>::type unsigned_T;

        // mask will be 0, iff m == 0, and 1 otherwise
        const T mask( unsigned_cast<T>(m | -m) >> (sizeof(unsigned_T)*8 - 1) );
        return v1*static_cast<M>(1-mask) + v2*mask;
    }
};
template< typename T,
          typename M > struct IntegerOutputMaskFunctor<T,M,false> : public MaskFunctorBase<T,M>
{
    /** Mask v with state of m

        @return v2, if m != 0, v1 otherwise.
     */
    T operator()( T v1, M m, T v2 ) const
    {
        typedef typename make_unsigned<T>::type unsigned_T;

        // mask will be 0, iff m == 0, and 1 otherwise
        const T mask( unsigned_cast<T>(m | -m) >> (sizeof(unsigned_T)*8 - 1) );
        return v1*mask + v2*static_cast<M>(1-mask);
    }
};

/** Let a mask bit decide between two values (specialization for
    binary-valued mask types)
 */
template< typename T, typename M, bool polarity > struct FastIntegerOutputMaskFunctor;
template< typename T, typename M > struct FastIntegerOutputMaskFunctor<T,M,true> :
   public MaskFunctorBase<T,M>
{
    /// Specialization, only valid if mask can only attain 0 or 1
    T operator()( T v1, M m, T v2 ) const
    {
        OSL_ASSERT(m<=1);

        return v1*static_cast<M>(1-m) + v2*m;
    }
};
template< typename T, typename M > struct FastIntegerOutputMaskFunctor<T,M,false> :
   public MaskFunctorBase<T,M>
{
    /// Specialization, only valid if mask can only attain 0 or 1
    T operator()( T v1, M m, T v2 ) const
    {
        OSL_ASSERT(m<=1);

        return v1*m + v2*static_cast<M>(1-m);
    }
};



/** Split a pair value from a JoinImageAccessorAdapter into its
    individual values, and pass it on to a ternary functor

    This wrapper is an adaptable binary functor, and can thus be used
    with a BinarySetterFunctionAccessorAdapter. Useful e.g. for
    out-of-image alpha channel, or a masked image.

    @tpl Functor
    An adaptable ternary functor (as can e.g. be passed to the
    TernarySetterFunctionAccessorAdapter)
 */
template< typename Functor > struct BinaryFunctorSplittingWrapper :
        public std::binary_function<typename Functor::first_argument_type,
                                    std::pair<typename Functor::third_argument_type,
                                              typename Functor::second_argument_type>,
                                    typename Functor::result_type>
{
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
// making all members public, if no member template friends
private:
    template<class A> friend struct BinaryFunctorSplittingWrapper;
#endif
    Functor maFunctor;

public:
    BinaryFunctorSplittingWrapper() : maFunctor() {}

    template< class A > explicit
    BinaryFunctorSplittingWrapper(
        BinaryFunctorSplittingWrapper<A> const& src ) : maFunctor(src.maFunctor) {}

    template< class F > explicit
    BinaryFunctorSplittingWrapper( F const& func ) : maFunctor(func) {}

    typename Functor::result_type operator()(
        typename Functor::first_argument_type                      v1,
        std::pair< typename Functor::third_argument_type,
                   typename Functor::second_argument_type > const& v2 ) const
    {
        return maFunctor( v1, v2.second, v2.first );
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_ACCESSORFUNCTORS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
