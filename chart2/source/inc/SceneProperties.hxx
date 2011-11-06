/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
