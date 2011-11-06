/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
