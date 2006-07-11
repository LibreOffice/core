/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessortraits.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-11 11:38:54 $
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

#ifndef INCLUDED_BASEBMP_ACCESSORTRAITS_HXX
#define INCLUDED_BASEBMP_ACCESSORTRAITS_HXX

#include <osl/diagnose.h>
#include <basebmp/accessoradapters.hxx>
#include <basebmp/metafunctions.hxx>

#include <functional>

namespace basebmp
{

// Some common accessor functors
// ------------------------------------------------------------


//-----------------------------------------------------------------------------

// XOR
template< typename T > struct XorFunctor : public std::binary_function<T,T,T>
{
    T operator()( T v1, T v2 ) const { return v1 ^ v2; }
};

//-----------------------------------------------------------------------------

// Mask
template< typename T, typename M > struct MaskFunctorBase
{
    typedef T first_argument_type;
    typedef M second_argument_type;
    typedef T third_argument_type;
    typedef T result_type;
};


// Mask
template< typename T, typename M > struct GenericOutputMaskFunctor : MaskFunctorBase<T,M>
{
    /// Ternary mask operation - selects v1 for m == 0, v2 otherwise
    T operator()( T v1, M m, T v2 ) const
    {
        return m == 0 ? v1 : v2;
    }
};

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

struct FastMask;
struct NoFastMask;

/// Metafunction to select output mask functor from iterator and mask value type
template< typename T, typename M, typename DUMMY > struct outputMaskFunctorSelector : public
    ifBothScalarIntegral< T, M,
                          IntegerOutputMaskFunctor< T, M >,
                          GenericOutputMaskFunctor< T, M > >
{
};
template< typename T, typename M > struct outputMaskFunctorSelector< T, M, FastMask > : public
    ifBothScalarIntegral< T, M,
                          FastIntegerOutputMaskFunctor< T, M >,
                          GenericOutputMaskFunctor< T, M > >
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
          class MaskIterator > struct maskedAccessorSelector
{
    typedef TernarySetterFunctionAccessorAdapter<
        Accessor,
        MaskAccessor,
        typename outputMaskFunctorSelector<
            typename Accessor::value_type,
            typename MaskAccessor::value_type,
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
              class MaskIterator > struct           masked_accessor :
        public maskedAccessorSelector< Accessor,MaskAccessor,Iterator,MaskIterator >
    {};

};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ACCESSORTRAITS_HXX */
