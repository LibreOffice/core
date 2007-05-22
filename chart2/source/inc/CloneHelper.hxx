/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CloneHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:13:25 $
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
#ifndef CHART2_CLONEHELPER_HXX
#define CHART2_CLONEHELPER_HXX

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

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
