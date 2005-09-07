/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FillProperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:40:38 $
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
#ifndef CHART_FILLPROPERTIES_HXX
#define CHART_FILLPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#include <vector>

namespace chart
{

// implements service FillProperties
class FillProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.drawing.FillProperties
        PROP_FILL_STYLE = FAST_PROPERTY_ID_START_FILL_PROP,
        PROP_FILL_COLOR,
        PROP_FILL_TRANSPARENCE,
        PROP_FILL_GRADIENT,
        PROP_FILL_HATCH,
        PROP_FILL_TRANSPARENCY_STYLE,
        PROP_FILL_TRANSPARENCY_GRADIENT
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties,
        bool bIncludeStyleProperties = false );

    static void AddDefaultsToMap( ::chart::helper::tPropertyValueMap & rOutMap, bool bIncludeStyleProperties = false );

private:
    // not implemented
    FillProperties();
};

} //  namespace chart

// CHART_FILLPROPERTIES_HXX
#endif
