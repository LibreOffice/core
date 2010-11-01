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
#ifndef CHART2_CLONEHELPER_HXX
#define CHART2_CLONEHELPER_HXX

#include <com/sun/star/util/XCloneable.hpp>

#include <map>
#include <functional>
#include <algorithm>

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
