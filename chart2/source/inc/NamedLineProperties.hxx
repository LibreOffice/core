/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NamedLineProperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:20:07 $
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
#ifndef CHART_NAMEDLINEPROPERTIES_HXX
#define CHART_NAMEDLINEPROPERTIES_HXX

#error "Deprecated, do not include this file"

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#include <vector>

namespace chart
{


// @depreated !!
class NamedLineProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.drawing.LineProperties (only named properties)
        PROP_LINE_DASH_NAME = FAST_PROPERTY_ID_START_NAMED_LINE_PROP,
        PROP_LINE_END_NAME,
        PROP_LINE_START_NAME,

        FAST_PROPERTY_ID_END_NAMED_LINE_PROP
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties,
        bool bIncludeLineEnds = false );

    static void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap,
                                  bool bIncludeLineEnds = false );

    //will return e.g. "LineDashName" for PROP_LINE_DASH_NAME
    static ::rtl::OUString GetPropertyNameForHandle( sal_Int32 nHandle );

private:
    // not implemented
    NamedLineProperties();
};

} //  namespace chart

// CHART_NAMEDLINEPROPERTIES_HXX
#endif
