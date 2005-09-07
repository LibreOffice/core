/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:45:16 $
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
#ifndef CHART_PROPERTYHELPER_HXX
#define CHART_PROPERTYHELPER_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include <map>

namespace chart
{

class PropertyHelper
{
public:
    static void copyProperties(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xSource,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xDestination
        );

private:
    // not implemented
    PropertyHelper();
};

namespace helper
{

struct PropertyNameLess
{
    inline bool operator() ( const ::com::sun::star::beans::Property & first,
                             const ::com::sun::star::beans::Property & second )
    {
        return ( first.Name.compareTo( second.Name ) < 0 );
    }
};

typedef ::std::map< sal_Int32, ::com::sun::star::uno::Any >
    tPropertyValueMap;

} //  namespace helper
} //  namespace chart

// CHART_PROPERTYHELPER_HXX
#endif
