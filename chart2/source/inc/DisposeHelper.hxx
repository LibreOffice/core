/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DisposeHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:15:49 $
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
#ifndef CHART2_DISPOSEHELPER_HXX
#define CHART2_DISPOSEHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

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
