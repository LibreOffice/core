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

#include "EnhancedCustomShapeToken.hxx"
#include <osl/mutex.hxx>
#include <unordered_map>
#include <memory>
#include <string.h>

namespace xmloff { namespace EnhancedCustomShapeToken {

typedef std::unordered_map< const char*, EnhancedCustomShapeTokenEnum, rtl::CStringHash, rtl::CStringEqual> TypeNameHashMap;
static TypeNameHashMap* pHashMap = nullptr;
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
    { "extrusion-first-light-level",        EAS_extrusion_first_light_level },
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
    { "sub-view-size",                      EAS_sub_view_size },

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
    { "SubViewSize",                        EAS_SubViewSize },

    { "Last",                               EAS_Last },
    { "NotFound",                           EAS_NotFound }
};

EnhancedCustomShapeTokenEnum EASGet( const OUString& rShapeType )
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
    std::unique_ptr<char[]> pBuf(new char[ nLen + 1 ]);
    for ( i = 0; i < nLen; i++ )
        pBuf[ i ] = (char)rShapeType[ i ];
    pBuf[ i ] = 0;
    TypeNameHashMap::iterator aHashIter( pHashMap->find( pBuf.get() ) );
    if ( aHashIter != pHashMap->end() )
        eRetValue = (*aHashIter).second;
    return eRetValue;
}

OUString EASGet( const EnhancedCustomShapeTokenEnum eToken )
{
    sal_uInt32 i = eToken >= EAS_Last
        ? (sal_uInt32)EAS_NotFound
        : (sal_uInt32)eToken;
    return OUString::createFromAscii( pTokenTableArray[ i ].pS );
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
