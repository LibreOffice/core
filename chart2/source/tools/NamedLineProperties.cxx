/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NamedLineProperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:02:32 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "NamedLineProperties.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::rtl::OUString;

namespace chart
{

// @deprecated !!
void NamedLineProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    const uno::Type tCppuTypeString = ::getCppuType( reinterpret_cast< const OUString * >(0));

    // Line Properties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "LineDash" ),
                  PROP_LINE_DASH,
                  ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    if( bIncludeLineEnds )
    {
        rOutProperties.push_back(
            Property( C2U( "LineStartName" ),
                      PROP_LINE_START_NAME,
                      tCppuTypeString,
                      beans::PropertyAttribute::BOUND
                      | beans::PropertyAttribute::MAYBEDEFAULT
                      | beans::PropertyAttribute::MAYBEVOID ));

        rOutProperties.push_back(
            Property( C2U( "LineEndName" ),
                      PROP_LINE_END_NAME,
                      tCppuTypeString,
                      beans::PropertyAttribute::BOUND
                      | beans::PropertyAttribute::MAYBEDEFAULT
                      | beans::PropertyAttribute::MAYBEVOID ));
    }
}

void NamedLineProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap,
    bool bIncludeLineEnds /* = false */ )
{
}

// static
OUString NamedLineProperties::GetPropertyNameForHandle( sal_Int32 nHandle )
{
    //will return e.g. "LineDashName" for PROP_LINE_DASH_NAME
    switch( nHandle )
    {
        case PROP_LINE_DASH_NAME:
            return C2U( "LineDashName" );
        case PROP_LINE_START_NAME:
        case PROP_LINE_END_NAME:
            break;
    }
    return OUString();
}

} //  namespace chart
