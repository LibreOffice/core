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
#ifndef CHART_SCENEPROPERTIES_HXX
#define CHART_SCENEPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/beans/Property.hpp>

#include <vector>

namespace chart
{

// implements service SceneProperties
class OOO_DLLPUBLIC_CHARTTOOLS SceneProperties
{
public:
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

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    static void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

private:
    // not implemented
    SceneProperties();
};

} //  namespace chart

// CHART_FILLPROPERTIES_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
