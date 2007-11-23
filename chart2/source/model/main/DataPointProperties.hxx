/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataPointProperties.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 11:59:10 $
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
#ifndef CHART_DATAPOINTPROPERTIES_HXX
#define CHART_DATAPOINTPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#include <vector>

namespace chart
{

// implements service DataPointProperties
class DataPointProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // common
        PROP_DATAPOINT_COLOR = FAST_PROPERTY_ID_START_DATA_POINT,
        PROP_DATAPOINT_TRANSPARENCY,

        // fill
        PROP_DATAPOINT_FILL_STYLE,
//         PROP_DATAPOINT_TRANSPARENCY_STYLE,
        PROP_DATAPOINT_TRANSPARENCY_GRADIENT_NAME,
        PROP_DATAPOINT_GRADIENT_NAME,
        PROP_DATAPOINT_GRADIENT_STEPCOUNT,
        PROP_DATAPOINT_HATCH_NAME,
        PROP_DATAPOINT_FILL_BITMAP_NAME,
        PROP_DATAPOINT_FILL_BACKGROUND,

        // border (of filled objects)
        PROP_DATAPOINT_BORDER_COLOR,
        PROP_DATAPOINT_BORDER_STYLE,
        PROP_DATAPOINT_BORDER_WIDTH,
        PROP_DATAPOINT_BORDER_DASH_NAME,
        PROP_DATAPOINT_BORDER_TRANSPARENCY,

        // others
        PROP_DATAPOINT_SYMBOL_PROP,
        PROP_DATAPOINT_OFFSET,
        PROP_DATAPOINT_GEOMETRY3D,
        PROP_DATAPOINT_LABEL,
        PROP_DATAPOINT_LABEL_SEPARATOR,
        PROP_DATAPOINT_NUMBER_FORMAT,
        PROP_DATAPOINT_PERCENTAGE_NUMBER_FORMAT,
        PROP_DATAPOINT_LABEL_PLACEMENT,
        PROP_DATAPOINT_REFERENCE_DIAGRAM_SIZE,

        // statistics
        PROP_DATAPOINT_ERROR_BAR_X,
        PROP_DATAPOINT_ERROR_BAR_Y,
        PROP_DATAPOINT_SHOW_ERROR_BOX,
        PROP_DATAPOINT_PERCENT_DIAGONAL

        // additionally some properites from ::chart::LineProperties
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    static void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

private:
    // not implemented
    DataPointProperties();
};

} //  namespace chart

// CHART_DATAPOINTPROPERTIES_HXX
#endif
