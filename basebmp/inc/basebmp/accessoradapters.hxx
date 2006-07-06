/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessoradapters.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-06 10:00:39 $
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

#include <vigra/numerictraits.hxx>

namespace basebmp
{

/** Interpose given accessor's set and get methods with two unary
    functors.

    @tpl WrappedAccessor
    Wrapped type must provide the usual get and set accessor methods,
    with the usual signatures (see StandardAccessor for a conforming
    example). Furthermore, the type must provide a wrapped typedef
    value_type
 */
template< class WrappedAccessor,
          typename GetterFunctor,
          typename SetterFunctor > class UnaryFunctionAccessorAdapter
{
private:
    // we don't derive from wrapped type, to avoid ambiguities
    // regarding templatized getter/setter methods.
    WrappedAccessor maAccessor;
    GetterFunctor   maGetterFunctor;
    SetterFunctor   maSetterFunctor;

public:
    typedef typename WrappedAccessor::value_type value_type;

    UnaryFunctionAccessorAdapter() :
        maAccessor(),
        maGetterFunctor(),
        maSetterFunctor()
    {}

    template< class T > explicit UnaryFunctionAccessorAdapter( T accessor ) :
        maAccessor( accessor ),
        maGetterFunctor(),
        maSetterFunctor()
    {}

    template< class T > UnaryFunctionAccessorAdapter( T             accessor,
                                                      GetterFunctor getterFunctor,
                                                      SetterFunctor setterFunctor) :
        maAccessor( accessor ),
        maGetterFunctor( getterFunctor ),
        maSetterFunctor( setterFunctor )
    {}

    // -------------------------------------------------------

    WrappedAccessor const& getWrappedAccessor() const { return maAccessor; }
    WrappedAccessor&       getWrappedAccessor() { return maAccessor; }

    // -------------------------------------------------------

    template< class Iterator >
    value_type operator()(Iterator const& i) const
    {
        return maGetterFunctor( maAccessor(i) );
    }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return maGetterFunctor( maAccessor(i,diff) );
    }

    // -------------------------------------------------------

    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        maAccessor.set(
            maSetterFunctor(
                vigra::detail::RequiresExplicitCast<value_type>::cast(value) ));
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        maAccessor.set(
            maSetterFunctor(
                vigra::detail::RequiresExplicitCast<value_type>::cast(value) ));
    }

};

//-----------------------------------------------------------------------------

/** Interpose given accessor's set methods with a binary function,
    taking both old and new value.

    The wrappee's getter methods kept as-is.

    @tpl WrappedAccessor
    Wrapped type must provide the usual get and set accessor methods,
    with the usual signatures (see StandardAccessor for a conforming
    example). Furthermore, the type must provide a wrapped typedef
    value_type
 */
template< class WrappedAccessor,
          typename SetterFunctor > class BinarySetterFunctionAccessorAdapter
{
private:
    WrappedAccessor    maAccessor;
    SetterFunctor      maFunctor;

public:
    typedef typename WrappedAccessor::value_type value_type;

    BinarySetterFunctionAccessorAdapter() :
        maAccessor(),
        maFunctor()
    {}

    template< class T > explicit BinarySetterFunctionAccessorAdapter( T accessor ) :
        maAccessor( accessor ),
        maFunctor()
    {}

    template< class T > BinarySetterFunctionAccessorAdapter( T             accessor,
                                                             SetterFunctor functor ) :
        maAccessor( accessor ),
        maFunctor( functor )
    {}

    // -------------------------------------------------------

    WrappedAccessor const& getWrappedAccessor() const { return maAccessor; }
    WrappedAccessor&       getWrappedAccessor() { return maAccessor; }

    // -------------------------------------------------------

    template< class Iterator >
    value_type operator()(Iterator const& i) const
    {
        return maAccessor(i);
    }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return maAccessor(i,diff);
    }

    // -------------------------------------------------------

    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        maAccessor.set(
            maFunctor(maAccessor(i),
                      vigra::detail::RequiresExplicitCast<value_type>::cast(value)),
            i );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        maAccessor.set(
            maFunctor(maAccessor(i,diff),
                      vigra::detail::RequiresExplicitCast<value_type>::cast(value)),
            i,
            diff );
    }

};

//-----------------------------------------------------------------------------

/** Write through a CompositeIterator's first wrapped iterator, by
    piping the first wrapped iterator value, the second iterator
    value, and the specified new value through a ternary function.

    Passed iterator must fulfill the CompositeIterator concept. Note
    that the getter/setter methods are not templatized regarding the
    iterator type, to make the mask calculation optimization below
    safe (see the maskedAccessor template metafunction below)

    @tpl WrappedAccessor1
    Wrapped type must provide the usual get and set accessor methods,
    with the usual signatures (see StandardAccessor for a conforming
    example). Furthermore, the type must provide a wrapped typedef
    value_type
 */
template< class WrappedAccessor1,
          class WrappedAccessor2,
          typename Functor > class TernarySetterFunctionAccessorAdapter
{
private:
    WrappedAccessor1 ma1stAccessor;
    WrappedAccessor2 ma2ndAccessor;
    Functor          maFunctor;

public:
    typedef typename WrappedAccessor1::value_type value_type;

    TernarySetterFunctionAccessorAdapter() :
        ma1stAccessor(),
        ma2ndAccessor(),
        maFunctor()
    {}

    template< class T > explicit TernarySetterFunctionAccessorAdapter( T accessor ) :
        ma1stAccessor( accessor ),
        ma2ndAccessor(),
        maFunctor()
    {}

    template< class T1, class T2 >
    TernarySetterFunctionAccessorAdapter( T1 accessor1,
                                          T2 accessor2 ) :
        ma1stAccessor( accessor1 ),
        ma2ndAccessor( accessor2 ),
        maFunctor()
    {}

    template< class T1, class T2 >
    TernarySetterFunctionAccessorAdapter( T1      accessor1,
                                          T2      accessor2,
                                          Functor func ) :
        ma1stAccessor( accessor1 ),
        ma2ndAccessor( accessor2 ),
        maFunctor( func )
    {}

    // -------------------------------------------------------

    WrappedAccessor1 const& get1stWrappedAccessor() const { return ma1stAccessor; }
    WrappedAccessor1&       get1stWrappedAccessor() { return ma1stAccessor; }

    WrappedAccessor2 const& get2ndWrappedAccessor() const { return ma2ndAccessor; }
    WrappedAccessor2&       get2ndWrappedAccessor() { return ma2ndAccessor; }

    // -------------------------------------------------------

    template< class Iterator >
    value_type operator()(Iterator const& i) const
    {
        return ma1stAccessor(i);
    }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return ma1stAccessor(i,diff);
    }

    // -------------------------------------------------------

    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        ma1stAccessor.set(
            maFunctor(ma1stAccessor(i.first()),
                      ma2ndAccessor(i.second()),
                      vigra::detail::RequiresExplicitCast<value_type>::cast(value)),
            i.first() );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        ma1stAccessor.set(
            maFunctor(ma1stAccessor(i.first(), diff),
                      ma2ndAccessor(i.second(),diff),
                      vigra::detail::RequiresExplicitCast<value_type>::cast(value)),
            i.first(),
            diff );
    }

};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ACCESSORADAPTERS_HXX */
