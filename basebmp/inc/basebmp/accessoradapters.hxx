/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessoradapters.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: thb $ $Date: 2006-06-07 14:27:34 $
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

#include <basebmp/metafunctions.hxx>
#include <basebmp/packedpixeliterator.hxx>
#include <basebmp/paletteimageaccessor.hxx>

#include <vigra/numerictraits.hxx>

namespace basebmp
{

/** Interpose given accessor's set methods with a binary function,
    taking both old and new value.

    All other wrappee methods are inherited as-is.
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
                      vigra::detail::RequiresExplicitCast<typename WrappedAccessor::value_type>::cast(value)),
            i );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        WrappedAccessor::set(
            maFunctor(WrappedAccessor::operator()(i,diff),
                      vigra::detail::RequiresExplicitCast<typename WrappedAccessor::value_type>::cast(value)),
            i,
            diff );
    }
};

/** Read from a CompositeIterator via two given accessors, pipe that
    through given functor, and write result to the first iterator

    Note: iterator type is fixed, to facilitate type-safe mask
    optimizations (see below).

    Passed iterator must fulfill the CompositeIterator concept

    Set functionality is unimplemented
 */
template< class WrappedAccessor1,
          class WrappedAccessor2,
          typename Iterator1,
          typename Iterator2,
          typename Functor > class BinaryInputAccessorAdapter
{
private:
    WrappedAccessor1 ma1stAccessor;
    WrappedAccessor2 ma2ndAccessor;
    Functor          maFunctor;

public:
    typedef typename WrappedAccessor1::value_type     value_type;

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

    template< typename Iterator > value_type operator()(Iterator const& i) const
    {
        return maFunctor(ma1stAccessor(i.first()),
                         ma2ndAccessor(i.second()));
    }
    template< typename Iterator, typename Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return maFunctor(ma1stAccessor(i.first(),diff),
                         ma2ndAccessor(i.second(),diff));
    }
};

/** Write through a CompositeIterator's first wrapped iterator, by
    piping the first wrapped iterator value, the second iterator
    value, and the specified new value through a ternary function.

    Passed iterator must fulfill the CompositeIterator concept

    All other wrappee methods are inherited as-is.
 */
template< class WrappedAccessor1,
          class WrappedAccessor2,
          typename Iterator1,
          typename Iterator2,
          typename Functor > class TernarySetterFunctionAccessorAdapter : public WrappedAccessor1
{
private:
    WrappedAccessor2 ma2ndAccessor;
    Functor          maFunctor;

public:
    TernarySetterFunctionAccessorAdapter() :
        WrappedAccessor1(),
        ma2ndAccessor(),
        maFunctor()
    {}

    explicit TernarySetterFunctionAccessorAdapter( WrappedAccessor1 accessor1 ) :
        WrappedAccessor1( accessor1 ),
        ma2ndAccessor(),
        maFunctor()
    {}

    TernarySetterFunctionAccessorAdapter( WrappedAccessor1 accessor1,
                                          WrappedAccessor2 accessor2 ) :
        WrappedAccessor1( accessor1 ),
        ma2ndAccessor( accessor2 ),
        maFunctor()
    {}

    TernarySetterFunctionAccessorAdapter( WrappedAccessor1 accessor1,
                                          WrappedAccessor2 accessor2,
                                          Functor          func ) :
        WrappedAccessor1( accessor1 ),
        ma2ndAccessor( accessor2 ),
        maFunctor( func )
    {}

    template< typename V, typename Iterator >
    void set(V const& value, Iterator const& i) const
    {
        WrappedAccessor1::set(
            maFunctor(WrappedAccessor1::operator()(i.first()),
                      ma2ndAccessor(i.second()),
                      vigra::detail::RequiresExplicitCast<typename WrappedAccessor1::value_type>::cast(value)),
            i.first() );
    }

    template< typename V, typename Iterator, typename Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        WrappedAccessor1::set(
            maFunctor(WrappedAccessor1::operator()(i.first(),diff),
                      ma2ndAccessor(i.second(),diff),
                      vigra::detail::RequiresExplicitCast<typename WrappedAccessor1::value_type>::cast(value)),
            i.first(),
            diff );
    }
};

/** Accessor adapter that blends input value against fixed color value

    Used to blend an alpha mask 'through' a fixed color value into the
    destination.
 */
template< class WrappedAccessor,
          typename ColorType > class ConstantColorBlendAccessorAdapter
{
private:
    WrappedAccessor maWrappee;
    ColorType       maBlendColor;

    // TODO(Q3): Merge with
    // BinarySetterFunctionAccessorAdapter. Problem there: how to
    // generate the functor, needs construction with accessor and
    // fixed color

public:
    typedef typename WrappedAccessor::value_type value_type;

    ConstantColorBlendAccessorAdapter() :
        maWrappee(),
        maBlendColor()
    {}

    ConstantColorBlendAccessorAdapter( WrappedAccessor acc,
                                       ColorType       col ) :
        maWrappee(acc),
        maBlendColor(col)
    {}

    template< typename IteratorType > value_type operator()(IteratorType const& i) const
    {
        return maWrappee(i);
    }
    template< typename IteratorType, class Difference >
    value_type operator()(IteratorType const& i, Difference const& diff) const
    {
        return maWrappee(i,diff);
    }

    template< typename V, typename IteratorType >
    void set(V const& value, IteratorType const& i) const
    {
        maWrappee.set(
            maBlendColor*value,
            i );
    }

    template< typename V, typename IteratorType, class Difference >
    void set(V const& value, IteratorType const& i, Difference const& diff) const
    {
        maWrappee.set(
            maBlendColor*value,
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
template< typename T > struct InputMaskFunctor
{
    /** Mask v with state of m

        @return v, if m != 0, and vigra::NumericTraits<T>::zero
        otherwise.
     */
    template< typename M> T operator()( T v, M m ) const
    {
        // TODO(Q3): use traits to get unsigned type for T (if
        // not already)

        // TODO(F3): use specialization for float types (which need
        // branching, instead of the bit fiddling used here)

        // mask will be 0, iff m == 0, and 1 otherwise
        const T mask( static_cast<unsigned int>(m | -m) >> (sizeof(unsigned int)*8 - 1) );
        return v*mask;
    }
};
// Faster mask (assuming mask accessor output is already either 0 or 1)
template< typename T > struct FastInputMaskFunctor
{
    template<typename M> T operator()( T v, M m ) const
    {
        return v*m;
    }
};

template< typename T > struct OutputMaskFunctor
{
    /** Mask v with state of m

        @return v2, if m != 0, v1 otherwise.
     */
    template< typename M > T operator()( T v1, M m, T v2 ) const
    {
        // TODO(Q3): use traits to get unsigned type for T (if
        // not unsigned already)

        // TODO(F3): use specialization for float types (which need
        // branching, instead of the bit fiddling used here)

        // mask will be 0, iff m == 0, and 1 otherwise
        const T mask( static_cast<unsigned int>(m | -m) >> (sizeof(unsigned int)*8 - 1) );
        return v1*(M)(1-mask) + v2*mask;
    }
};
// Faster mask (assuming mask accessor output is already either 0 or 1)
template< typename T > struct FastOutputMaskFunctor
{
    template< typename M> T operator()( T v1, M m, T v2 ) const
    {
        return v1*(M)(1-m) + v2*m;
    }
};

// Chosen function crucially depends on iterator - we can choose the
// faster direkt masking for 1bpp packed pixel iterators
template< class WrappedAccessor,
          class MaskAccessor,
          class Iterator,
          class MaskIterator > struct maskedAccessor
{
    typedef TernarySetterFunctionAccessorAdapter< WrappedAccessor,
                                                  MaskAccessor,
                                                  Iterator,
                                                  MaskIterator,
                                                  OutputMaskFunctor<
        typename WrappedAccessor::value_type > >
        type;
};
// partial specialization, to use fast 1bpp mask function for
// corresponding iterator type
template< class WrappedAccessor,
          class MaskAccessor,
          class Iterator > struct maskedAccessor< WrappedAccessor,
                                                  MaskAccessor,
                                                  Iterator,
                                                  PackedPixelIterator< typename MaskAccessor::value_type,
                                                                       1,
                                                                       true > >
{
    typedef TernarySetterFunctionAccessorAdapter< WrappedAccessor,
                                                  MaskAccessor,
                                                  Iterator,
                                                  PackedPixelIterator< typename MaskAccessor::value_type,
                                                                       1,
                                                                       true >,
                                                  FastOutputMaskFunctor< typename WrappedAccessor::value_type > >
        type;
};

template< class WrappedAccessor,
          class MaskAccessor,
          class Iterator > struct maskedAccessor< WrappedAccessor,
                                                  MaskAccessor,
                                                  Iterator,
                                                  PackedPixelIterator< typename MaskAccessor::value_type,
                                                                       1,
                                                                       false > >
{
    typedef TernarySetterFunctionAccessorAdapter< WrappedAccessor,
                                                  MaskAccessor,
                                                  Iterator,
                                                  PackedPixelIterator< typename MaskAccessor::value_type,
                                                                       1,
                                                                       false >,
                                                  FastOutputMaskFunctor< typename WrappedAccessor::value_type > >
    type;
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ACCESSORADAPTERS_HXX */
