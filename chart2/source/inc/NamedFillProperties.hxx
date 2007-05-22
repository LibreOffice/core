/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NamedFillProperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:19:57 $
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
#ifndef CHART_NAMEDFILLPROPERTIES_HXX
#define CHART_NAMEDFILLPROPERTIES_HXX

#error "Deprecated, do not include this file"

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#include <vector>

namespace chart
{

// CHANGED! : these are the UNnamed properties!
// @deprecated
class NamedFillProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.drawing.FillProperties (only named properties)
        //optional properties:
        PROP_FILL_TRANSPARENCE_GRADIENT,
        PROP_FILL_GRADIENT,
        PROP_FILL_HATCH,
        PROP_FILL_BITMAP,

        FAST_PROPERTY_ID_END_NAMED_FILL_PROP
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    static void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

    //will return e.g. "FillGradientName" for nHandle == PROP_FILL_GRADIENT_NAME
    static ::rtl::OUString GetPropertyNameForHandle( sal_Int32 nHandle );

private:
    // not implemented
    NamedFillProperties();
};

} //  namespace chart

// CHART_NAMEDFILLPROPERTIES_HXX
#endif
