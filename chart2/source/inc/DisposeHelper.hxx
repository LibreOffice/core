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
#ifndef CHART2_DISPOSEHELPER_HXX
#define CHART2_DISPOSEHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XComponent.hpp>

#include <functional>
#include <algorithm>
#include <utility>

namespace chart
{
namespace DisposeHelper
{

template< class T >
    void Dispose( const T & xIntf )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp(
        xIntf, ::com::sun::star::uno::UNO_QUERY );
    if( xComp.is())
        xComp->dispose();
}

template< class Intf >
    void DisposeAndClear( ::com::sun::star::uno::Reference< Intf > & rIntf )
{
    Dispose< ::com::sun::star::uno::Reference< Intf > >( rIntf );
    rIntf.set( 0 );
}

template< class T >
    struct DisposeFunctor : public ::std::unary_function< T, void >
{
    void operator() ( const T & xIntf )
    {
        Dispose< T >( xIntf );
    }
};

template< typename T >
    struct DisposeFirstOfPairFunctor : public ::std::unary_function< T, void >
{
    void operator() ( const T & rElem )
    {
        Dispose< typename T::first_type >( rElem.first );
    }
};

template< typename T >
    struct DisposeSecondOfPairFunctor : public ::std::unary_function< T, void >
{
    void operator() ( const T & rElem )
    {
        Dispose< typename T::second_type >( rElem.second );
    }
};

template< class Container >
    void DisposeAllElements( Container & rContainer )
{
    ::std::for_each( rContainer.begin(), rContainer.end(), DisposeFunctor< typename Container::value_type >());
}

template< class Map >
    void DisposeAllMapElements( Map & rContainer )
{
    ::std::for_each( rContainer.begin(), rContainer.end(), DisposeSecondOfPairFunctor< typename Map::value_type >());
}

} //  namespace DisposeHelper
} //  namespace chart

// CHART2_DISPOSEHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
