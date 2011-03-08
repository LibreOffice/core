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
#ifndef CHART_NAMEDFILLPROPERTIES_HXX
#define CHART_NAMEDFILLPROPERTIES_HXX

#error "Deprecated, do not include this file"

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include <com/sun/star/beans/Property.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
