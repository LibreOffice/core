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
#ifndef CHART_USERDEFINEDPROPERTIES_HXX
#define CHART_USERDEFINEDPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/beans/Property.hpp>

#include <vector>

namespace chart
{

// implements service com.sun.star.xml.UserDefinedAttributeSupplier
class OOO_DLLPUBLIC_CHARTTOOLS UserDefinedProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // UserDefined attributes in the chart-properties
        PROP_XML_USERDEF_CHART = FAST_PROPERTY_ID_START_USERDEF_PROP,
        PROP_XML_USERDEF_TEXT,
        PROP_XML_USERDEF_PARA,
        // com.sun.star.xml.UserDefinedAttributeSupplier
        // UserDefined attributes in the other properties-sections. (as long as
        // there is no reliable mechanism, properties may move between the
        // sections)
        PROP_XML_USERDEF
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

private:
    // not implemented
    UserDefinedProperties();
};

} //  namespace chart

// CHART_USERDEFINEDPROPERTIES_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
