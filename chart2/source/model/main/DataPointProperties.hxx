/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART_DATAPOINTPROPERTIES_HXX
#define CHART_DATAPOINTPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include <com/sun/star/beans/Property.hpp>

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
        PROP_DATAPOINT_TEXT_ROTATION,

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
