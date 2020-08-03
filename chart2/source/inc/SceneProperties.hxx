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

// implements service SceneProperties
namespace SceneProperties
{
    // FastProperty Ids for properties
    enum
    {
        // "com.sun.star.drawing.SceneProperties" (does not exist)
        PROP_SCENE_TRANSF_MATRIX = FAST_PROPERTY_ID_START_SCENE_PROP,
        PROP_SCENE_DISTANCE,
        PROP_SCENE_FOCAL_LENGTH,
        PROP_SCENE_SHADOW_SLANT,
        PROP_SCENE_SHADE_MODE,
        PROP_SCENE_AMBIENT_COLOR,
        PROP_SCENE_TWO_SIDED_LIGHTING,
        PROP_SCENE_CAMERA_GEOMETRY,
        PROP_SCENE_PERSPECTIVE,
        PROP_SCENE_LIGHT_COLOR_1,
        PROP_SCENE_LIGHT_DIRECTION_1,
        PROP_SCENE_LIGHT_ON_1,
        PROP_SCENE_LIGHT_COLOR_2,
        PROP_SCENE_LIGHT_DIRECTION_2,
        PROP_SCENE_LIGHT_ON_2,
        PROP_SCENE_LIGHT_COLOR_3,
        PROP_SCENE_LIGHT_DIRECTION_3,
        PROP_SCENE_LIGHT_ON_3,
        PROP_SCENE_LIGHT_COLOR_4,
        PROP_SCENE_LIGHT_DIRECTION_4,
        PROP_SCENE_LIGHT_ON_4,
        PROP_SCENE_LIGHT_COLOR_5,
        PROP_SCENE_LIGHT_DIRECTION_5,
        PROP_SCENE_LIGHT_ON_5,
        PROP_SCENE_LIGHT_COLOR_6,
        PROP_SCENE_LIGHT_DIRECTION_6,
        PROP_SCENE_LIGHT_ON_6,
        PROP_SCENE_LIGHT_COLOR_7,
        PROP_SCENE_LIGHT_DIRECTION_7,
        PROP_SCENE_LIGHT_ON_7,
        PROP_SCENE_LIGHT_COLOR_8,
        PROP_SCENE_LIGHT_DIRECTION_8,
        PROP_SCENE_LIGHT_ON_8
    };

    OOO_DLLPUBLIC_CHARTTOOLS void AddPropertiesToVector(
        std::vector< css::beans::Property > & rOutProperties );

    OOO_DLLPUBLIC_CHARTTOOLS void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
