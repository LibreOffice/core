/*************************************************************************
 *
 *  $RCSfile: DataPointProperties.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        PROP_DATAPOINT_STYLE = FAST_PROPERTY_ID_START_DATA_POINT,
        PROP_DATAPOINT_COLOR,
        PROP_DATAPOINT_TRANSPARENCY,

        // fill
        PROP_DATAPOINT_FILL_STYLE,
        PROP_DATAPOINT_TRANSPARENCY_STYLE,
        PROP_DATAPOINT_TRANSPARENCY_GRADIENT,
        PROP_DATAPOINT_GRADIENT,
        PROP_DATAPOINT_HATCH,
        PROP_DATAPOINT_BITMAP,

        // border (of filled objects)
        PROP_DATAPOINT_BORDER_COLOR,
        PROP_DATAPOINT_BORDER_STYLE,
        PROP_DATAPOINT_BORDER_WIDTH,
        PROP_DATAPOINT_BORDER_DASH,
        PROP_DATAPOINT_BORDER_TRANSPARENCY,

        // line
        PROP_DATAPOINT_LINE_STYLE,
        PROP_DATAPOINT_LINE_WIDTH,
        PROP_DATAPOINT_LINE_DASH,

        // others
        PROP_DATAPOINT_SYMBOL,
        PROP_DATAPOINT_OFFSET,
        PROP_DATAPOINT_DATA_CAPTION,
        PROP_DATAPOINT_NUMBER_FORMAT
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties,
        bool bIncludeStyleProperties = false );

    static void AddDefaultsToMap( ::chart::helper::tPropertyValueMap & rOutMap, bool bIncludeStyleProperties = false );

private:
    // not implemented
    DataPointProperties();
};

} //  namespace chart

// CHART_DATAPOINTPROPERTIES_HXX
#endif
