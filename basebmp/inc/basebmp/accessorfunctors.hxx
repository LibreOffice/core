/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessorfunctors.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-12 15:09:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_BASEBMP_ACCESSORFUNCTORS_HXX
#define INCLUDED_BASEBMP_ACCESSORFUNCTORS_HXX

#include <osl/diagnose.h>
#include <basebmp/metafunctions.hxx>

#include <functional>

namespace basebmp
{

// Some common accessor functors
// ------------------------------------------------------------


/// combine two values via XOR
template< typename T > struct XorFunctor : public std::binary_function<T,T,T>
{
    T operator()( T v1, T v2 ) const { return v1 ^ v2; }
};

//-----------------------------------------------------------------------------

/// Base class for an adaptable ternary functor
template< typename A1, typename A2, typename A3, typename R > struct TernaryFunctorBase
{
    typedef A1 first_argument_type;
    typedef A2 second_argument_type;
    typedef A3 third_argument_type;
    typedef R  result_type;
};

/// Base class, passing on the arg types
template< typename T, typename M > struct MaskFunctorBase :
        public TernaryFunctorBase<T,M,T,T> {};


/// Let a mask flag decide between two values
template< typename T, typename M > struct GenericOutputMaskFunctor : MaskFunctorBase<T,M>
{
    /// Ternary mask operation - selects v1 for !m == true, v2 otherwise
    T operator()( T v1, M m, T v2 ) const
    {
        return !m ? v1 : v2;
    }
};

/** Let a mask bit decide between two values (specialization for
    integer mask types)
 */
template< typename T, typename M > struct IntegerOutputMaskFunctor : MaskFunctorBase<T,M>
{
    /** Mask v with state of m

        @return v2, if m != 0, v1 otherwise.
     */
    T operator()( T v1, M m, T v2 ) const
    {
        typedef typename make_unsigned<T>::type unsigned_T;

        // mask will be 0, iff m == 0, and 1 otherwise
        const T mask( unsigned_cast<T>(m | -m) >> (sizeof(unsigned_T)*8 - 1) );
        return v1*(M)(1-mask) + v2*mask;
    }
};

/** Let a mask bit decide between two values (specialization for
    binary-valued mask types)
 */
template< typename T, typename M > struct FastIntegerOutputMaskFunctor : MaskFunctorBase<T,M>
{
    /// Specialization, only valid if mask can only attain 0 or 1
    T operator()( T v1, M m, T v2 ) const
    {
        OSL_ASSERT(m<=1);

        return v1*(M)(1-m) + v2*m;
    }
};

//-----------------------------------------------------------------------------

/** Split a pair value from a JoinImageAccessorAdapter into its
    individual values, and pass it on to a ternary functor

    This wrapper is an adaptable binary functor, and can thus be used
    with a BinarySetterFunctionAccessorAdapter. Useful e.g. for
    out-of-image alpha channel, or a masked image.

    @tpl Functor
    An adaptable ternary functor (as can e.g. be passed to the
    TernarySetterFunctionAccessorAdapter)
 */
template< typename Functor > class BinaryFunctorSplittingWrapper :
        public std::binary_function<typename Functor::first_argument_type,
                                    std::pair<typename Functor::third_argument_type,
                                              typename Functor::second_argument_type>,
                                    typename Functor::result_type>
{
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
// making all members public, if no member template friends
private:
    template<class A> friend class BinaryFunctorSplittingWrapper;
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

#endif /* INCLUDED_BASEBMP_ACCESSORFUNCTORS_HXX */
