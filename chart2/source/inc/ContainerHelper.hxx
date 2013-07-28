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
#ifndef CHART2_CONTAINERHELPER_HXX
#define CHART2_CONTAINERHELPER_HXX

#include <vector>
#include <set>
#include <map>

#include <algorithm>
#include <functional>
#include <o3tl/compat_functional.hxx>

namespace chart
{
namespace ContainerHelper
{

/** converts a standard container into a sequence of the same type

    input:   standard container
    output:  css::uno::Sequence< container::value_type >

    example:

    ::std::vector< sal_Int32 > aVector;
    Sequence< sal_Int32 > aSequence( ContainerHelper::ContainerToSequence( aVector ));
 */
template< class Container >
    ::com::sun::star::uno::Sequence< typename Container::value_type >
    ContainerToSequence( const Container & rCont )
{
    ::com::sun::star::uno::Sequence< typename Container::value_type > aResult( rCont.size());
    ::std::copy( rCont.begin(), rCont.end(), aResult.getArray());
    return aResult;
}

/** converts a UNO sequence into a standard "Sequence" container. For
    convenience see the methods SequenceToVector, etc. below.

    input:  uno::Sequence
    output: a standard container of the same value type implementing the Concept
            of a Sequence (vector, deque, list, slist)

    example:

    Sequence< sal_Int32 > aSequence;
    ::std::vector< sal_Int32 > aVector(
        ContainerToSequence::SequenceToSTLSequenceContainer< ::std::vector< sal_Int32 > >( aSequence );
*/
template< class Container >
    Container
    SequenceToSTLSequenceContainer( const ::com::sun::star::uno::Sequence< typename Container::value_type > & rSeq )
{
    Container aResult( rSeq.getLength());
    ::std::copy( rSeq.getConstArray(), rSeq.getConstArray() + rSeq.getLength(),
                 aResult.begin() );
    return aResult;
}

/** converts a UNO sequence into a standard container. For convenience see the
    methods SequenceToVector, etc. below. (In contrast to
    SequenceToSTLSequenceContainer this works for all standard containers)

    input:  uno::Sequence
    output: a standard container that has an insert( iterator, key ) method (all
            standard containers)
    note:   for containers implementing the Concept of a Sequence (vector, deque,
            list, slist) use SequenceToSTLSequenceContainer for better speed

    example:

    Sequence< sal_Int32 > aSequence;
    ::std::set< sal_Int32 > aVector(
        ContainerToSequence::SequenceToSTLContainer< ::std::set< sal_Int32 > >( aSequence );
*/
template< class Container >
    Container
    SequenceToSTLContainer( const ::com::sun::star::uno::Sequence< typename Container::value_type > & rSeq )
{
    Container aResult;
    ::std::copy( rSeq.getConstArray(), rSeq.getConstArray() + rSeq.getLength(),
                 ::std::inserter< Container >( aResult, aResult.begin()));
    return aResult;
}

// concrete container methods for convenience

/** converts a UNO sequence into a standard vector of same value type

    example:

    Sequence< sal_Int32 > aSequence;
    ::std::vector< sal_Int32 > aVector( ContainerHelper::SequenceToVector( aSequence ));
 */
template< typename T >
    ::std::vector< T >
    SequenceToVector( const ::com::sun::star::uno::Sequence< T > & rSeq )
{
    return SequenceToSTLSequenceContainer< ::std::vector< T > >( rSeq );
}

/** converts a UNO sequence into a standard set of same value type

    example:

    Sequence< sal_Int32 > aSequence;
    ::std::set< sal_Int32 > aVector( ContainerHelper::SequenceToSet( aSequence ));
 */
template< typename T >
    ::std::set< T >
    SequenceToSet( const ::com::sun::star::uno::Sequence< T > & rSeq )
{
    return SequenceToSTLContainer< ::std::set< T > >( rSeq );
}

/** converts the keys of a Pair Associative Container into a UNO sequence

    example:

    ::std::multimap< sal_Int32, OUString > aMyMultiMap;
    uno::Sequence< sal_Int32 > aMyKeys( ContainerHelper::MapKeysToSequence( aMyMultiMap ));
    // note: aMyKeys may contain duplicate keys here
 */
template< class Map >
::com::sun::star::uno::Sequence< typename Map::key_type > MapKeysToSequence(
    const Map & rCont )
{
    ::com::sun::star::uno::Sequence< typename Map::key_type > aResult( rCont.size());
    ::std::transform( rCont.begin(), rCont.end(), aResult.getArray(),
                      ::o3tl::select1st< typename Map::value_type >());
    return aResult;
}

/** converts the values of a Pair Associative Container into a UNO sequence

    example:

    ::std::map< sal_Int32, OUString > aMyMultiMap;
    uno::Sequence< OUString > aMyValues( ContainerHelper::MapValuesToSequence( aMyMultiMap ));
 */
template< class Map >
::com::sun::star::uno::Sequence< typename Map::mapped_type > MapValuesToSequence(
    const Map & rCont )
{
    ::com::sun::star::uno::Sequence< typename Map::mapped_type > aResult( rCont.size());
    ::std::transform( rCont.begin(), rCont.end(), aResult.getArray(),
                      ::o3tl::select2nd< typename Map::value_type >());
    return aResult;
}

} // namespace ContainerHelper
} //  namespace chart

// CHART2_CONTAINERHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
