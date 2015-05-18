/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CHART2_SOURCE_INC_LINEPROPERTIESHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_LINEPROPERTIESHELPER_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/beans/Property.hpp>

#include <vector>

namespace chart
{

// implements service LineProperties
namespace LinePropertiesHelper
{
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.drawing.LineProperties
        PROP_LINE_STYLE = FAST_PROPERTY_ID_START_LINE_PROP,
        PROP_LINE_DASH,
        PROP_LINE_DASH_NAME, //not in service description
        PROP_LINE_COLOR,
        PROP_LINE_TRANSPARENCE,
        PROP_LINE_WIDTH,
        PROP_LINE_JOINT
    };

    OOO_DLLPUBLIC_CHARTTOOLS void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    OOO_DLLPUBLIC_CHARTTOOLS void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

    bool IsLineVisible( const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >& xLineProperties );
    void SetLineVisible( const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >& xLineProperties );
    void SetLineInvisible( const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >& xLineProperties );
}

}

// INCLUDED_CHART2_SOURCE_INC_LINEPROPERTIESHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
