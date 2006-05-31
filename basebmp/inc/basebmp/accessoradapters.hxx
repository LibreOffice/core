/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessoradapters.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-05-31 10:12:11 $
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

#ifndef INCLUDED_BASEBMP_ACCESSORADAPTERS_HXX
#define INCLUDED_BASEBMP_ACCESSORADAPTERS_HXX

#include "metafunctions.hxx"
#include "packedpixeliterator.hxx"
#include "paletteimageaccessor.hxx"

namespace basebmp
{

/** Interpose given accessor's set methods with a binary function,
    taking both old and new value.
 */
template< class WrappedAccessor, typename Functor > class BinarySetterFunctionAccessorAdapter :
        public WrappedAccessor
{
private:
    Functor maFunctor;

public:
    BinarySetterFunctionAccessorAdapter() :
        WrappedAccessor(),
        maFunctor()
    {}

    explicit BinarySetterFunctionAccessorAdapter( WrappedAccessor accessor ) :
        WrappedAccessor( accessor ),
        maFunctor()
    {}

    BinarySetterFunctionAccessorAdapter( WrappedAccessor accessor,
                                         Functor         functor ) :
        WrappedAccessor( accessor ),
        maFunctor( functor )
    {}

    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        WrappedAccessor::set(
            maFunctor(WrappedAccessor::operator()(i),
                      value),
            i );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        WrappedAccessor::set(
            maFunctor(WrappedAccessor::operator()(i,diff),
                      value),
            i,
            diff );
    }
};

/** Read from two input iterators, pipe that through given functor, and write result
    to the first iterator

    Note: iterator type is fixed, to facilitate type-safe mask
    optimizations (see below)
 */
template< class WrappedAccessor1,
          class WrappedAccessor2,
          class Iterator1,
          class Iterator2,
          typename Functor > class BinaryInputAccessorAdapter
{
private:
    WrappedAccessor1 ma1stAccessor;
    WrappedAccessor2 ma2ndAccessor;
    Functor          maFunctor;

public:
    typedef typename WrappedAccessor1::value_type value_type;

    BinaryInputAccessorAdapter() :
        ma1stAccessor(),
        ma2ndAccessor(),
        maFunctor()
    {}

    explicit BinaryInputAccessorAdapter( WrappedAccessor1 accessor1 ) :
        ma1stAccessor( accessor1 ),
        ma2ndAccessor(),
        maFunctor()
    {}

    BinaryInputAccessorAdapter( WrappedAccessor1 accessor1,
                                WrappedAccessor2 accessor2 ) :
        ma1stAccessor( accessor1 ),
        ma2ndAccessor( accessor2 ),
        maFunctor()
    {}

    BinaryInputAccessorAdapter( WrappedAccessor1 accessor1,
                                WrappedAccessor2 accessor2,
                                Functor          func ) :
        ma1stAccessor( accessor1 ),
        ma2ndAccessor( accessor2 ),
        maFunctor( func )
    {}

    value_type operator()(Iterator1 const& i, Iterator2 const& j) const
    {
        return maFunctor(ma1stAccessor(i),
                         ma2ndAccessor(j));
    }
    template< class Difference >
    value_type operator()(Iterator1 const& i, Iterator2 const& j, Difference const& diff) const
    {
        return maFunctor(ma1stAccessor(i,diff),
                         ma2ndAccessor(j,diff));
    }

    template< typename V >
    void set(V const& value, Iterator1 const& i, Iterator2 const& j) const
    {
        ma1stAccessor.set(
            maFunctor(ma1stAccessor(i),
                      ma2ndAccessor(j)),
            i );
    }

    template< typename V, class Difference >
    void set(V const& value, Iterator1 const& i, Iterator2 const& j, Difference const& diff) const
    {
        ma1stAccessor.set(
            maFunctor(ma1stAccessor(i,diff),
                      ma2ndAccessor(j,diff)),
            i,
            diff );
    }
};

// Some common accessor wrappers
// ------------------------------------------------------------

// XOR
template< typename T > struct XorFunctor
{
    T operator()( T v1, T v2 ) const { return v1 ^ v2; }
};
template< class WrappedAccessor > struct xorAccessor
{
    typedef BinarySetterFunctionAccessorAdapter< WrappedAccessor,
                                                 XorFunctor< typename WrappedAccessor::value_type > >
        type;
};

// Mask
template< typename T > struct MaskFunctor
{
    /** Mask v with state of m

        @return v, if m != 0, and vigra::NumericTraits<T>::zero
        otherwise.
     */
    T operator()( T v, T m ) const
    {
        // TODO(Q3): use traits to get unsigned type for T (if
        // not already)

        // mask will be 0, iff m == 0, and 1 otherwise
        const T mask( static_cast<unsigned int>(m | -m) >> (sizeof(unsigned int)*8 - 1) );
        return mask*v;
    }
};
// Faster mask (assuming mask accessor output is already either 0 or 1)
template< typename T > struct FastMaskFunctor
{
    T operator()( T v, T m ) const
    {
        return m*v;
    }
};
// Chosen function crucially depends on iterator - we can choose the
// faster direkt masking for 1bpp packed pixel iterators
template< class WrappedAccessor,
          class MaskAccessor,
          class Iterator,
          class MaskIterator > struct maskedAccessor
{
    typedef BinaryInputAccessorAdapter< WrappedAccessor,
                                        MaskAccessor,
                                        Iterator,
                                        MaskIterator,
                                        MaskFunctor< typename WrappedAccessor::value_type > >
        type;
};
// partial specialization, to use fast 1bpp mask function for
// corresponding iterator type
template< class WrappedAccessor,
          class MaskAccessor,
          class Iterator > struct maskedAccessor< WrappedAccessor,
                                                  MaskAccessor,
                                                  Iterator,
                                                  PackedPixelIterator< typename MaskAccessor::data_type,
                                                                       typename MaskAccessor::value_type,
                                                                       1,
                                                                       true > >
{
    typedef BinaryInputAccessorAdapter< WrappedAccessor,
                                        MaskAccessor,
                                        Iterator,
                                        PackedPixelIterator< typename MaskAccessor::data_type,
                                                             typename MaskAccessor::value_type,
                                                             1,
                                                             true >,
                                        FastMaskFunctor< typename WrappedAccessor::value_type > >
        type;
};
template< class WrappedAccessor,
          class MaskAccessor,
          class Iterator > struct maskedAccessor< WrappedAccessor,
                                                  MaskAccessor,
                                                  Iterator,
                                                  PackedPixelIterator< typename MaskAccessor::data_type,
                                                                       typename MaskAccessor::value_type,
                                                                       1,
                                                                       false > >
{
    typedef BinaryInputAccessorAdapter< WrappedAccessor,
                                        MaskAccessor,
                                        Iterator,
                                        PackedPixelIterator< typename MaskAccessor::data_type,
                                                             typename MaskAccessor::value_type,
                                                             1,
                                                             false >,
                                        FastMaskFunctor< typename WrappedAccessor::value_type > >
        type;
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ACCESSORADAPTERS_HXX */
