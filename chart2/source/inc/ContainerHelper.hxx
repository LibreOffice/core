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
#ifndef INCLUDED_CHART2_SOURCE_INC_CONTAINERHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_CONTAINERHELPER_HXX

#include <vector>
#include <set>
#include <map>

#include <algorithm>
#include <functional>
#include <o3tl/functional.hxx>

namespace chart
{
namespace ContainerHelper
{

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
    ::std::copy( rSeq.begin(), rSeq.end(), aResult.begin() );
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

} // namespace ContainerHelper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_CONTAINERHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
