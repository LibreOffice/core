/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SceneProperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:22:47 $
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
#ifndef CHART_SCENEPROPERTIES_HXX
#define CHART_SCENEPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#include <vector>

namespace chart
{

// implements service SceneProperties
class SceneProperties
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
