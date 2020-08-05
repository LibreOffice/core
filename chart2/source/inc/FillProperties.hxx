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
#pragma once

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"

#include <vector>

namespace com::sun::star::beans { struct Property; }

namespace chart
{

// implements service FillProperties
namespace FillProperties
{
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.drawing.FillProperties
        PROP_FILL_STYLE = FAST_PROPERTY_ID_START_FILL_PROP
        , PROP_FILL_COLOR
        , PROP_FILL_TRANSPARENCE
        , PROP_FILL_TRANSPARENCE_GRADIENT_NAME
//        , PROP_FILL_TRANSPARENCE_GRADIENT //optional
        , PROP_FILL_GRADIENT_NAME
        , PROP_FILL_GRADIENT_STEPCOUNT
//        , PROP_FILL_GRADIENT   //optional
        , PROP_FILL_HATCH_NAME
//        , PROP_FILL_HATCH  //optional
        // bitmap properties start
        , PROP_FILL_BITMAP_NAME
//        , PROP_FILL_BITMAP //optional
//        , PROP_FILL_BITMAP_URL //optional
        , PROP_FILL_BITMAP_OFFSETX
        , PROP_FILL_BITMAP_OFFSETY
        , PROP_FILL_BITMAP_POSITION_OFFSETX
        , PROP_FILL_BITMAP_POSITION_OFFSETY
        , PROP_FILL_BITMAP_RECTANGLEPOINT
        , PROP_FILL_BITMAP_LOGICALSIZE
        , PROP_FILL_BITMAP_SIZEX
        , PROP_FILL_BITMAP_SIZEY
        , PROP_FILL_BITMAP_MODE
        // bitmap properties end
        , PROP_FILL_BACKGROUND
    };

    OOO_DLLPUBLIC_CHARTTOOLS void AddPropertiesToVector(
        std::vector< css::beans::Property > & rOutProperties );

    OOO_DLLPUBLIC_CHARTTOOLS void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
