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

#ifndef INCLUDED_BASEBMP_INC_ACCESSORADAPTERS_HXX
#define INCLUDED_BASEBMP_INC_ACCESSORADAPTERS_HXX

#include <vigra/numerictraits.hxx>

namespace basebmp
{

/** Interpose given accessor's set and get methods with two unary
    functors.

    @tpl WrappedAccessor
    Wrapped type must provide the usual get and set accessor methods,
    with the usual signatures (see StandardAccessor for a conforming
    example).

    @tpl GetterFunctor
    An Adaptable Unary Function (i.e. providing result_type and
    argument_type typedefs)

    @tpl SetterFunctor
    An Adaptable Unary Function (i.e. providing result_type and
    argument_type typedefs)
 */
template< class WrappedAccessor,
          typename GetterFunctor,
          typename SetterFunctor > class UnaryFunctionAccessorAdapter
{
public:
    typedef typename GetterFunctor::result_type   value_type;
    typedef typename SetterFunctor::argument_type argument_type;

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
// making all members public, if no member template friends
private:
    template<class A, typename G, typename S> friend class UnaryFunctionAccessorAdapter;
#endif

    // we don't derive from wrapped type, to avoid ambiguities
    // regarding templatized getter/setter methods.
    WrappedAccessor maAccessor;
    GetterFunctor   maGetterFunctor;
    SetterFunctor   maSetterFunctor;

public:
    UnaryFunctionAccessorAdapter() :
        maAccessor(),
        maGetterFunctor(),
        maSetterFunctor()
    {}

    template< class A > explicit
    UnaryFunctionAccessorAdapter( UnaryFunctionAccessorAdapter< A,
                                                                GetterFunctor,
                                                                SetterFunctor > const& rSrc ) :
        maAccessor( rSrc.maAccessor ),
        maGetterFunctor( rSrc.maGetterFunctor ),
        maSetterFunctor( rSrc.maSetterFunctor )
    {}

    template< class T > explicit UnaryFunctionAccessorAdapter( T const& accessor ) :
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



    WrappedAccessor const& getWrappedAccessor() const { return maAccessor; }
    WrappedAccessor&       getWrappedAccessor() { return maAccessor; }



    value_type getter(typename GetterFunctor::argument_type v) const
    {
        return maGetterFunctor(v);
    }
    typename SetterFunctor::result_type setter(argument_type v) const
    {
        return maSetterFunctor(v);
    }



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



    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        maAccessor.set(
            maSetterFunctor(
                vigra::detail::RequiresExplicitCast<argument_type>::cast(value) ),
            i );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        maAccessor.set(
            maSetterFunctor(
                vigra::detail::RequiresExplicitCast<argument_type>::cast(value) ),
            i,
            diff );
    }

};



/** Interpose given accessor's set methods with a binary function,
    taking both old and new value.

    The wrappee's getter methods kept as-is.

    @tpl WrappedAccessor
    Wrapped type must provide the usual get and set accessor methods,
    with the usual signatures (see StandardAccessor for a conforming
    example). Furthermore, must provide a nested typedef value_type.

    @tpl SetterFunctor
    An adaptable binary function (i.e. providing nested typedefs for
    result_type and first and second argument type)
 */
template< class WrappedAccessor,
          typename SetterFunctor > class BinarySetterFunctionAccessorAdapter
{
public:
    typedef typename WrappedAccessor::value_type         value_type;
    typedef typename SetterFunctor::second_argument_type argument_type;

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
// making all members public, if no member template friends
private:
    template<class A, typename S> friend class BinarySetterFunctionAccessorAdapter;
#endif

    WrappedAccessor    maAccessor;
    SetterFunctor      maFunctor;

public:
    BinarySetterFunctionAccessorAdapter() :
        maAccessor(),
        maFunctor()
    {}

    template< class A > explicit
    BinarySetterFunctionAccessorAdapter(
        BinarySetterFunctionAccessorAdapter< A,
                                             SetterFunctor > const& rSrc ) :
        maAccessor( rSrc.maAccessor ),
        maFunctor( rSrc.maFunctor )
    {}

    template< class T > explicit BinarySetterFunctionAccessorAdapter( T const& accessor ) :
        maAccessor( accessor ),
        maFunctor()
    {}

    template< class T > BinarySetterFunctionAccessorAdapter( T             accessor,
                                                             SetterFunctor functor ) :
        maAccessor( accessor ),
        maFunctor( functor )
    {}



    WrappedAccessor const& getWrappedAccessor() const { return maAccessor; }
    WrappedAccessor&       getWrappedAccessor() { return maAccessor; }



    static typename SetterFunctor::result_type setter(
        typename SetterFunctor::first_argument_type v1,
        argument_type                               v2 )
    {
        return maSetterFunctor(v1,v2);
    }



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



    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        maAccessor.set(
            maFunctor(maAccessor(i),
                      vigra::detail::RequiresExplicitCast<argument_type>::cast(value)),
            i );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        maAccessor.set(
            maFunctor(maAccessor(i,diff),
                      vigra::detail::RequiresExplicitCast<argument_type>::cast(value)),
            i,
            diff );
    }

};



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
    example). Furthermore, the type must provide a nested typedef
    value_type (the selection of WrappedAccessor1 as the provider for
    that typedef is rather arbitrary. Could have been
    WrappedAccessor2, too. So sue me)

    @tpl Functor
    An adaptable ternary function (i.e. providing nested typedefs for
    result_type and first, second and third argument type)
 */
template< class WrappedAccessor1,
          class WrappedAccessor2,
          typename Functor > class TernarySetterFunctionAccessorAdapter
{
public:
    typedef typename WrappedAccessor1::value_type value_type;
    typedef typename Functor::third_argument_type argument_type;

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
// making all members public, if no member template friends
private:
    template<class A1, class A2, typename F> friend class TernarySetterFunctionAccessorAdapter;
#endif

    WrappedAccessor1 ma1stAccessor;
    WrappedAccessor2 ma2ndAccessor;
    Functor          maFunctor;

public:
    TernarySetterFunctionAccessorAdapter() :
        ma1stAccessor(),
        ma2ndAccessor(),
        maFunctor()
    {}

    template< class T > explicit TernarySetterFunctionAccessorAdapter( T const& accessor ) :
        ma1stAccessor( accessor ),
        ma2ndAccessor(),
        maFunctor()
    {}

    template< class A1, class A2 > explicit
    TernarySetterFunctionAccessorAdapter(
        TernarySetterFunctionAccessorAdapter< A1,
                                              A2,
                                              Functor > const& rSrc ) :
        ma1stAccessor( rSrc.ma1stAccessor ),
        ma2ndAccessor( rSrc.ma2ndAccessor ),
        maFunctor( rSrc.maFunctor )
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



    WrappedAccessor1 const& get1stWrappedAccessor() const { return ma1stAccessor; }
    WrappedAccessor1&       get1stWrappedAccessor() { return ma1stAccessor; }

    WrappedAccessor2 const& get2ndWrappedAccessor() const { return ma2ndAccessor; }
    WrappedAccessor2&       get2ndWrappedAccessor() { return ma2ndAccessor; }



    static typename Functor::result_type setter(
        typename Functor::first_argument_type  v1,
        typename Functor::second_argument_type v2,
        argument_type                          v3 )
    {
        return maSetterFunctor(v1,v2,v3);
    }



    template< class Iterator >
    value_type operator()(Iterator const& i) const
    {
        return ma1stAccessor(i.first());
    }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return ma1stAccessor(i.second(),diff);
    }



    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        ma1stAccessor.set(
            maFunctor(ma1stAccessor(i.first()),
                      ma2ndAccessor(i.second()),
                      vigra::detail::RequiresExplicitCast<argument_type>::cast(value)),
            i.first() );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        ma1stAccessor.set(
            maFunctor(ma1stAccessor(i.first(), diff),
                      ma2ndAccessor(i.second(),diff),
                      vigra::detail::RequiresExplicitCast<argument_type>::cast(value)),
            i.first(),
            diff );
    }

};



/** Access two distinct images simultaneously

    Passed iterator must fulfill the CompositeIterator concept
    (i.e. wrap the two image's iterators into one
    CompositeIterator). The getter and setter methods expect and
    return a pair of values, with types equal to the two accessors
    value types

    @tpl WrappedAccessor1
    Wrapped type must provide the usual get and set accessor methods,
    with the usual signatures (see StandardAccessor for a conforming
    example). Furthermore, the type must provide a nested typedef
    value_type.

    @tpl WrappedAccessor2
    Wrapped type must provide the usual get and set accessor methods,
    with the usual signatures (see StandardAccessor for a conforming
    example). Furthermore, the type must provide a nested typedef
    value_type.
 */
template< class WrappedAccessor1,
          class WrappedAccessor2 > class JoinImageAccessorAdapter
{
public:
    // TODO(F3): Need numeric traits and a few free functions to
    // actually calculate with a pair (semantic: apply every operation
    // individually to the contained types)
    typedef std::pair<typename WrappedAccessor1::value_type,
                      typename WrappedAccessor2::value_type>    value_type;

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
// making all members public, if no member template friends
private:
    template<class A1, class A2> friend class JoinImageAccessorAdapter;
#endif

    WrappedAccessor1 ma1stAccessor;
    WrappedAccessor2 ma2ndAccessor;

public:
    JoinImageAccessorAdapter() :
        ma1stAccessor(),
        ma2ndAccessor()
    {}

    template< class T > explicit JoinImageAccessorAdapter( T const& accessor ) :
        ma1stAccessor( accessor ),
        ma2ndAccessor()
    {}

    template< class A1, class A2 > explicit
    JoinImageAccessorAdapter(
        JoinImageAccessorAdapter< A1,
                                  A2 > const& rSrc ) :
        ma1stAccessor( rSrc.ma1stAccessor ),
        ma2ndAccessor( rSrc.ma2ndAccessor )
    {}

    template< class T1, class T2 >
    JoinImageAccessorAdapter( T1 accessor1,
                              T2 accessor2 ) :
        ma1stAccessor( accessor1 ),
        ma2ndAccessor( accessor2 )
    {}



    WrappedAccessor1 const& get1stWrappedAccessor() const { return ma1stAccessor; }
    WrappedAccessor1&       get1stWrappedAccessor() { return ma1stAccessor; }

    WrappedAccessor2 const& get2ndWrappedAccessor() const { return ma2ndAccessor; }
    WrappedAccessor2&       get2ndWrappedAccessor() { return ma2ndAccessor; }



    template< class Iterator >
    value_type operator()(Iterator const& i) const
    {
        return std::make_pair(ma1stAccessor(i.first()),
                              ma2ndAccessor(i.second()));
    }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return std::make_pair(ma1stAccessor(i.first(),diff),
                              ma2ndAccessor(i.second(),diff));
    }



    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        ma1stAccessor.set(
            vigra::detail::RequiresExplicitCast<typename WrappedAccessor1::value_type>::cast(
                value.first),
            i.first() );
        ma2ndAccessor.set(
            vigra::detail::RequiresExplicitCast<typename WrappedAccessor2::value_type>::cast(
                value.second),
            i.second() );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        ma1stAccessor.set(
            vigra::detail::RequiresExplicitCast<typename WrappedAccessor1::value_type>::cast(
                value.first),
            i.first(),
            diff );
        ma2ndAccessor.set(
            vigra::detail::RequiresExplicitCast<typename WrappedAccessor2::value_type>::cast(
                value.second),
            i.second(),
            diff );
    }

};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_ACCESSORADAPTERS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
