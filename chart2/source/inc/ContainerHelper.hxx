/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

// ----------------------------------------

/** converts the keys of a Pair Associative Container into a UNO sequence

    example:

    ::std::multimap< sal_Int32, ::rtl::OUString > aMyMultiMap;
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

    ::std::map< sal_Int32, ::rtl::OUString > aMyMultiMap;
    uno::Sequence< ::rtl::OUString > aMyValues( ContainerHelper::MapValuesToSequence( aMyMultiMap ));
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
