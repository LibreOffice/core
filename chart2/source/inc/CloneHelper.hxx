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
#ifndef CHART2_CLONEHELPER_HXX
#define CHART2_CLONEHELPER_HXX

#include <com/sun/star/util/XCloneable.hpp>

#include <map>
#include <functional>
#include <algorithm>
#include <iterator>

namespace chart
{
namespace CloneHelper
{

/// functor that clones a UNO-Reference
template< class Interface >
    struct CreateRefClone : public ::std::unary_function< Interface, Interface >
{
    Interface operator() ( const Interface & xOther )
    {
        Interface xResult;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >
              xCloneable( xOther, ::com::sun::star::uno::UNO_QUERY );
        if( xCloneable.is())
            xResult.set( xCloneable->createClone(), ::com::sun::star::uno::UNO_QUERY );

        return xResult;
    }
};

/// functor that clones a map element with a UNO-Reference as value
template< typename Key, class Interface >
    struct CreateRefWithKeyClone : public ::std::unary_function<
        ::std::pair<  Key, Interface >,
        ::std::pair<  Key, Interface > >
{
    ::std::pair< Key, Interface > operator() (
        const ::std::pair< Key, Interface > & rOther )
    {
        Interface xResult;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >
              xCloneable( rOther.second, ::com::sun::star::uno::UNO_QUERY );
        if( xCloneable.is())
            xResult.set( xCloneable->createClone(), ::com::sun::star::uno::UNO_QUERY );

        return ::std::make_pair< Key, Interface >( rOther.first, xResult );
    }
};

/// clones a vector of UNO-References
template< class Interface >
    void CloneRefVector(
        const ::std::vector< Interface > & rSource,
        ::std::vector< Interface > & rDestination )
{
    ::std::transform( rSource.begin(), rSource.end(),
                      ::std::back_inserter( rDestination ),
                      CreateRefClone< Interface >());
}

template< typename  Key, class Interface >
    void CloneRefPairVector(
        const ::std::vector< ::std::pair< Key, Interface > > & rSource,
        ::std::vector< ::std::pair< Key, Interface > > & rDestination )
{
    ::std::transform( rSource.begin(), rSource.end(),
                      ::std::back_inserter( rDestination ),
                      CreateRefWithKeyClone< Key, Interface >());
}

/// clones a map of elements with a UNO-Reference as value
template< typename Key, class Interface >
    void CloneRefMap(
        const ::std::map< Key, Interface > & rSource,
        ::std::map< Key, Interface > & rDestination )
{
    ::std::transform( rSource.begin(), rSource.end(),
                      ::std::inserter( rDestination, rDestination.begin() ),
                      CreateRefWithKeyClone< const Key, Interface >());
}

/// clones a UNO-sequence of UNO-References
template< class Interface >
    void CloneRefSequence(
        const ::com::sun::star::uno::Sequence< Interface > & rSource,
        ::com::sun::star::uno::Sequence< Interface > & rDestination )
{
    rDestination.realloc( rSource.getLength());
    ::std::transform( rSource.getConstArray(), rSource.getConstArray() + rSource.getLength(),
                      rDestination.getArray(),
                      CreateRefClone< Interface >());
}

} //  namespace CloneHelper
} //  namespace chart

// CHART2_CLONEHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
