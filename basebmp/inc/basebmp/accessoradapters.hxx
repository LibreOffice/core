/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessoradapters.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: thb $ $Date: 2006-06-09 04:21:00 $
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


/// Traits template, to determine alpha blending between two values
template< typename ValueType, typename AlphaType > struct BlendFunctor
{
    ValueType operator()( AlphaType alpha,
                          ValueType v1,
                          ValueType v2 ) const
    {
        const typename vigra::NumericTraits<AlphaType>::RealPromote fAlpha(
            vigra::NumericTraits<AlphaType>::toRealPromote(alpha));
        return (vigra::NumericTraits<AlphaType>::one()-fAlpha)*v1 + fAlpha*v2;
    }
};

template< typename ValueType, typename AlphaType > struct IntegerBlendFunctor
{
    ValueType operator()( AlphaType alpha,
                          ValueType v1,
                          ValueType v2 ) const
    {
        return (vigra::NumericTraits<AlphaType>::toPromote(
                    vigra::NumericTraits<AlphaType>::max()-alpha)*v1 + alpha*v2) /
            vigra::NumericTraits<AlphaType>::max();
    }
};

/// Metafunction to select blend functor from value and alpha type
template< typename ValueType, typename AlphaType > struct blendFunctorSelector : public
    ifScalarIntegral< AlphaType,
                      IntegerBlendFunctor< ValueType, AlphaType >,
                      BlendFunctor< ValueType, AlphaType > >
{
};

/** Accessor adapter that blends input value against fixed color value

    Used to blend an alpha mask 'through' a fixed color value into the
    destination.
 */
template< class WrappedAccessor,
          typename AlphaType > class ConstantColorBlendAccessorAdapter
{
public:
    typedef AlphaType                            alpha_type;
    typedef typename WrappedAccessor::value_type value_type;

private:
    typename blendFunctorSelector< value_type, alpha_type >::type maFunctor;
    WrappedAccessor                                               maWrappee;
    value_type                                                    maBlendColor;

    // TODO(Q3): Merge with
    // BinarySetterFunctionAccessorAdapter. Problem there: how to
    // generate the functor, needs construction with accessor and
    // fixed color

public:
    ConstantColorBlendAccessorAdapter() :
        maFunctor(),
        maWrappee(),
        maBlendColor()
    {}

    explicit ConstantColorBlendAccessorAdapter( WrappedAccessor acc ) :
        maFunctor(),
        maWrappee(acc),
        maBlendColor()
    {}

    ConstantColorBlendAccessorAdapter( WrappedAccessor acc,
                                       value_type      col ) :
        maFunctor(),
        maWrappee(acc),
        maBlendColor(col)
    {}

    void setColor( value_type col ) { maBlendColor=col; }

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
            maFunctor(
                vigra::detail::RequiresExplicitCast<alpha_type>::cast(value),
                maWrappee(i),
                maBlendColor),
            i );
    }

    template< typename V, typename IteratorType, class Difference >
    void set(V const& value, IteratorType const& i, Difference const& diff) const
    {
        maWrappee.set(
            maFunctor(
                vigra::detail::RequiresExplicitCast<alpha_type>::cast(value),
                maWrappee(i,diff),
                maBlendColor),
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


// Masking functors for binary input
//--------------------------------------------------------

template< typename T, typename M > struct GenericInputMaskFunctor
{
    /** Mask v with state of m

        @return v, if m != 0, and vigra::NumericTraits<T>::zero
        otherwise.
     */
    T operator()( T v, M m ) const
    {
        return m == 0 ? vigra::NumericTraits<T>::zero : v;
    }
};

template< typename T, typename M > struct IntegerInputMaskFunctor
{
    /** Mask v with state of m

        @return v, if m != 0, and vigra::NumericTraits<T>::zero
        otherwise.
     */
    T operator()( T v, M m ) const
    {
        // TODO(Q3): use traits to get unsigned type for T (if
        // not already)

        // mask will be 0, iff m == 0, and 1 otherwise
        const T mask( static_cast<unsigned int>(m | -m) >> (sizeof(unsigned int)*8 - 1) );
        return v*mask;
    }
};

template< typename T, typename M > struct FastIntegerInputMaskFunctor
{
    T operator()( T v, M m ) const
    {
        return v*m;
    }
};


// Masking functors for TernarySetterFunctionAccessorAdapter
//-----------------------------------------------------------

template< typename T, typename M > struct GenericOutputMaskFunctor
{
    /// Ternary mask operation - selects v1 for m == 0, v2 otherwise
    T operator()( T v1, M m, T v2 ) const
    {
        return m == 0 ? v1 : v2;
    }
};

template< typename T, typename M > struct IntegerOutputMaskFunctor
{
    /** Mask v with state of m

        @return v2, if m != 0, v1 otherwise.
     */
    T operator()( T v1, M m, T v2 ) const
    {
        // mask will be 0, iff m == 0, and 1 otherwise
        const T mask( static_cast<unsigned int>(m | -m) >> (sizeof(unsigned int)*8 - 1) );
        return v1*(M)(1-mask) + v2*mask;
    }
};

template< typename T, typename M > struct FastIntegerOutputMaskFunctor
{
    T operator()( T v1, M m, T v2 ) const
    {
        return v1*(M)(1-m) + v2*m;
    }
};

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

// Chosen function crucially depends on iterator - we can choose the
// faster direkt masking for 1bpp packed pixel iterators
template< class WrappedAccessor,
          class MaskAccessor,
          class Iterator,
          class MaskIterator > struct maskedAccessor
{
    typedef TernarySetterFunctionAccessorAdapter<
        WrappedAccessor,
        MaskAccessor,
        Iterator,
        MaskIterator,
        typename outputMaskFunctorSelector<
            typename WrappedAccessor::value_type,
            typename MaskAccessor::value_type,
            NoFastMask>::type >
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
    typedef TernarySetterFunctionAccessorAdapter<
        WrappedAccessor,
        MaskAccessor,
        Iterator,
        PackedPixelIterator<
            typename MaskAccessor::value_type,
            1,
            true >,
        typename outputMaskFunctorSelector<
            typename WrappedAccessor::value_type,
            typename MaskAccessor::value_type,
            FastMask>::type >
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
    typedef TernarySetterFunctionAccessorAdapter<
        WrappedAccessor,
        MaskAccessor,
        Iterator,
        PackedPixelIterator<
            typename MaskAccessor::value_type,
            1,
            false >,
        typename outputMaskFunctorSelector<
            typename WrappedAccessor::value_type,
            typename MaskAccessor::value_type,
            FastMask>::type >
        type;
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ACCESSORADAPTERS_HXX */
