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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "EnhancedCustomShapeToken.hxx"
#include <osl/mutex.hxx>
#include <boost/unordered_map.hpp>
#include <string.h>

namespace xmloff { namespace EnhancedCustomShapeToken {

struct TCheck
{
    bool operator()( const char* s1, const char* s2 ) const
    {
        return strcmp( s1, s2 ) == 0;
    }
};
typedef boost::unordered_map< const char*, EnhancedCustomShapeTokenEnum, std::hash<const char*>, TCheck> TypeNameHashMap;
static TypeNameHashMap* pHashMap = NULL;
static ::osl::Mutex& getHashMapMutex()
{
    static osl::Mutex s_aHashMapProtection;
    return s_aHashMapProtection;
}

struct TokenTable
{
    const char*                         pS;
    EnhancedCustomShapeTokenEnum        pE;
};

static const TokenTable pTokenTableArray[] =
{
    { "type",                               EAS_type },
    { "name",                               EAS_name },
    { "mirror-horizontal",                  EAS_mirror_horizontal },
    { "mirror-vertical",                    EAS_mirror_vertical },
    { "viewBox",                            EAS_viewBox },
    { "text-rotate-angle",                  EAS_text_rotate_angle },
    { "extrusion-allowed",                  EAS_extrusion_allowed },
    { "extrusion-text-path-allowed",        EAS_text_path_allowed },
    { "extrusion-concentric-gradient-fill", EAS_concentric_gradient_fill_allowed },
    { "extrusion",                          EAS_extrusion },
    { "extrusion-brightness",               EAS_extrusion_brightness },
    { "extrusion-depth",                    EAS_extrusion_depth },
    { "extrusion-diffusion",                EAS_extrusion_diffusion },
    { "extrusion-number-of-line-segments",  EAS_extrusion_number_of_line_segments },
    { "extrusion-light-face",               EAS_extrusion_light_face },
    { "extrusion-first-light-harsh",        EAS_extrusion_first_light_harsh },
    { "extrusion-second-light-harsh",       EAS_extrusion_second_light_harsh },
    { "extrusion-first-light-livel",        EAS_extrusion_first_light_level },
    { "extrusion-second-light-level",       EAS_extrusion_second_light_level },
    { "extrusion-first-light-direction",    EAS_extrusion_first_light_direction },
    { "extrusion-second-light-direction",   EAS_extrusion_second_light_direction },
    { "extrusion-metal",                    EAS_extrusion_metal },
    { "shade-mode",                         EAS_shade_mode },
    { "extrusion-rotation-angle",           EAS_extrusion_rotation_angle },
    { "extrusion-rotation-center",          EAS_extrusion_rotation_center },
    { "extrusion-shininess",                EAS_extrusion_shininess },
    { "extrusion-skew",                     EAS_extrusion_skew },
    { "extrusion-specularity",              EAS_extrusion_specularity },
    { "projection",                         EAS_projection },
    { "extrusion-viewpoint",                EAS_extrusion_viewpoint },
    { "extrusion-origin",                   EAS_extrusion_origin },
    { "extrusion-color",                    EAS_extrusion_color },
    { "enhanced-path",                      EAS_enhanced_path },
    { "path-stretchpoint-x",                EAS_path_stretchpoint_x },
    { "path-stretchpoint-y",                EAS_path_stretchpoint_y },
    { "text-areas",                         EAS_text_areas },
    { "glue-points",                        EAS_glue_points },
    { "glue-point-type",                    EAS_glue_point_type },
    { "glue-point-leaving-directions",      EAS_glue_point_leaving_directions },
    { "text-path",                          EAS_text_path },
    { "text-path-mode",                     EAS_text_path_mode },
    { "text-path-scale",                    EAS_text_path_scale },
    { "text-path-same-letter-heights",      EAS_text_path_same_letter_heights },
    { "modifiers",                          EAS_modifiers },
    { "equation",                           EAS_equation },
    { "formula",                            EAS_formula },
    { "handle",                             EAS_handle },
    { "handle-mirror-horizontal",           EAS_handle_mirror_horizontal },
    { "handle-mirror-vertical",             EAS_handle_mirror_vertical },
    { "handle-switched",                    EAS_handle_switched },
    { "handle-position",                    EAS_handle_position },
    { "handle-range-x-minimum",             EAS_handle_range_x_minimum },
    { "handle-range-x-maximum",             EAS_handle_range_x_maximum },
    { "handle-range-y-minimum",             EAS_handle_range_y_minimum },
    { "handle-range-y-maximum",             EAS_handle_range_y_maximum },
    { "handle-polar",                       EAS_handle_polar },
    { "handle-radius-range-minimum",        EAS_handle_radius_range_minimum },
    { "handle-radius-range-maximum",        EAS_handle_radius_range_maximum },

    { "CustomShapeEngine",                  EAS_CustomShapeEngine },
    { "CustomShapeData",                    EAS_CustomShapeData },
    { "Type",                               EAS_Type },
    { "MirroredX",                          EAS_MirroredX },
    { "MirroredY",                          EAS_MirroredY },
    { "ViewBox",                            EAS_ViewBox },
    { "TextRotateAngle",                    EAS_TextRotateAngle },
    { "ExtrusionAllowed",                   EAS_ExtrusionAllowed },
    { "TextPathAllowed",                    EAS_TextPathAllowed },
    { "ConcentricGradientFillAllowed",      EAS_ConcentricGradientFillAllowed },
    { "Extrusion",                          EAS_Extrusion },
    { "Equations",                          EAS_Equations },
    { "Equation",                           EAS_Equation },
    { "Path",                               EAS_Path },
    { "TextPath",                           EAS_TextPath },
    { "Handles",                            EAS_Handles },
    { "Handle",                             EAS_Handle },
    { "Brightness",                         EAS_Brightness },
    { "Depth",                              EAS_Depth },
    { "Diffusion",                          EAS_Diffusion },
    { "NumberOfLineSegments",               EAS_NumberOfLineSegments },
    { "LightFace",                          EAS_LightFace },
    { "FirstLightHarsh",                    EAS_FirstLightHarsh },
    { "SecondLightHarsh",                   EAS_SecondLightHarsh },
    { "FirstLightLevel",                    EAS_FirstLightLevel },
    { "SecondLightLevel",                   EAS_SecondLightLevel },
    { "FirstLightDirection",                EAS_FirstLightDirection },
    { "SecondLightDirection",               EAS_SecondLightDirection },
    { "Metal",                              EAS_Metal },
    { "ShadeMode",                          EAS_ShadeMode },
    { "RotateAngle",                        EAS_RotateAngle },
    { "RotationCenter",                     EAS_RotationCenter },
    { "Shininess",                          EAS_Shininess },
    { "Skew",                               EAS_Skew },
    { "Specularity",                        EAS_Specularity },
    { "ProjectionMode",                     EAS_ProjectionMode },
    { "ViewPoint",                          EAS_ViewPoint },
    { "Origin",                             EAS_Origin },
    { "Color",                              EAS_Color },
    { "Switched",                           EAS_Switched },
     { "Polar",                             EAS_Polar },
    { "RangeXMinimum",                      EAS_RangeXMinimum },
    { "RangeXMaximum",                      EAS_RangeXMaximum },
    { "RangeYMinimum",                      EAS_RangeYMinimum },
    { "RangeYMaximum",                      EAS_RangeYMaximum },
    { "RadiusRangeMinimum",                 EAS_RadiusRangeMinimum },
    { "RadiusRangeMaximum",                 EAS_RadiusRangeMaximum },
    { "Coordinates",                        EAS_Coordinates },
    { "Segments",                           EAS_Segments },
    { "StretchX",                           EAS_StretchX },
    { "StretchY",                           EAS_StretchY },
    { "TextFrames",                         EAS_TextFrames },
    { "GluePoints",                         EAS_GluePoints },
    { "GluePointLeavingDirections",         EAS_GluePointLeavingDirections },
    { "GluePointType",                      EAS_GluePointType },
    { "TextPathMode",                       EAS_TextPathMode },
    { "ScaleX",                             EAS_ScaleX },
    { "SameLetterHeights",                  EAS_SameLetterHeights },
    { "Position",                           EAS_Position },
    { "AdjustmentValues",                   EAS_AdjustmentValues },

    { "Last",                               EAS_Last },
    { "NotFound",                           EAS_NotFound }
};

EnhancedCustomShapeTokenEnum EASGet( const rtl::OUString& rShapeType )
{
    if ( !pHashMap )
    {   // init hash map
        ::osl::MutexGuard aGuard( getHashMapMutex() );
        if ( !pHashMap )
        {
            TypeNameHashMap* pH = new TypeNameHashMap;
            const TokenTable* pPtr = pTokenTableArray;
            const TokenTable* pEnd = pPtr + ( sizeof( pTokenTableArray ) / sizeof( TokenTable ) );
            for ( ; pPtr < pEnd; pPtr++ )
                (*pH)[ pPtr->pS ] = pPtr->pE;
            pHashMap = pH;
        }
    }
    EnhancedCustomShapeTokenEnum eRetValue = EAS_NotFound;
    int i, nLen = rShapeType.getLength();
    char* pBuf = new char[ nLen + 1 ];
    for ( i = 0; i < nLen; i++ )
        pBuf[ i ] = (char)rShapeType[ i ];
    pBuf[ i ] = 0;
    TypeNameHashMap::iterator aHashIter( pHashMap->find( pBuf ) );
    delete[] pBuf;
    if ( aHashIter != pHashMap->end() )
        eRetValue = (*aHashIter).second;
    return eRetValue;
}

rtl::OUString EASGet( const EnhancedCustomShapeTokenEnum eToken )
{
    sal_uInt32 i = eToken >= EAS_Last
        ? (sal_uInt32)EAS_NotFound
        : (sal_uInt32)eToken;
    return rtl::OUString::createFromAscii( pTokenTableArray[ i ].pS );
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
