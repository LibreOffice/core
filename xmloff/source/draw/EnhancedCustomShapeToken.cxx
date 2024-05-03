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

#include <EnhancedCustomShapeToken.hxx>
#include <xmloff/xmlimp.hxx>
#include <unordered_map>
#include <memory>

namespace xmloff::EnhancedCustomShapeToken {


namespace {

struct TokenTable
{
    OUString                            aS;
    EnhancedCustomShapeTokenEnum        pE;
};

}

constexpr TokenTable pTokenTableArray[] =
{
    { u"type"_ustr,                               EAS_type },
    { u"name"_ustr,                               EAS_name },
    { u"mirror-horizontal"_ustr,                  EAS_mirror_horizontal },
    { u"mirror-vertical"_ustr,                    EAS_mirror_vertical },
    { u"viewBox"_ustr,                            EAS_viewBox },
    { u"text-rotate-angle"_ustr,                  EAS_text_rotate_angle },
    { u"extrusion-allowed"_ustr,                  EAS_extrusion_allowed },
    { u"extrusion-text-path-allowed"_ustr,        EAS_text_path_allowed },
    { u"extrusion-concentric-gradient-fill"_ustr, EAS_concentric_gradient_fill_allowed },
    { u"extrusion"_ustr,                          EAS_extrusion },
    { u"extrusion-brightness"_ustr,               EAS_extrusion_brightness },
    { u"extrusion-depth"_ustr,                    EAS_extrusion_depth },
    { u"extrusion-diffusion"_ustr,                EAS_extrusion_diffusion },
    { u"extrusion-number-of-line-segments"_ustr,  EAS_extrusion_number_of_line_segments },
    { u"extrusion-light-face"_ustr,               EAS_extrusion_light_face },
    { u"extrusion-first-light-harsh"_ustr,        EAS_extrusion_first_light_harsh },
    { u"extrusion-second-light-harsh"_ustr,       EAS_extrusion_second_light_harsh },
    { u"extrusion-first-light-level"_ustr,        EAS_extrusion_first_light_level },
    { u"extrusion-second-light-level"_ustr,       EAS_extrusion_second_light_level },
    { u"extrusion-first-light-direction"_ustr,    EAS_extrusion_first_light_direction },
    { u"extrusion-second-light-direction"_ustr,   EAS_extrusion_second_light_direction },
    { u"extrusion-metal"_ustr,                    EAS_extrusion_metal },
    { u"extrusion-metal-type"_ustr,               EAS_extrusion_metal_type },
    { u"shade-mode"_ustr,                         EAS_shade_mode },
    { u"extrusion-rotation-angle"_ustr,           EAS_extrusion_rotation_angle },
    { u"extrusion-rotation-center"_ustr,          EAS_extrusion_rotation_center },
    { u"extrusion-shininess"_ustr,                EAS_extrusion_shininess },
    { u"extrusion-skew"_ustr,                     EAS_extrusion_skew },
    { u"extrusion-specularity"_ustr,              EAS_extrusion_specularity },
    { u"extrusion-specularity-loext"_ustr,        EAS_extrusion_specularity_loext },
    { u"projection"_ustr,                         EAS_projection },
    { u"extrusion-viewpoint"_ustr,                EAS_extrusion_viewpoint },
    { u"extrusion-origin"_ustr,                   EAS_extrusion_origin },
    { u"extrusion-color"_ustr,                    EAS_extrusion_color },
    { u"enhanced-path"_ustr,                      EAS_enhanced_path },
    { u"path-stretchpoint-x"_ustr,                EAS_path_stretchpoint_x },
    { u"path-stretchpoint-y"_ustr,                EAS_path_stretchpoint_y },
    { u"text-areas"_ustr,                         EAS_text_areas },
    { u"glue-points"_ustr,                        EAS_glue_points },
    { u"glue-point-type"_ustr,                    EAS_glue_point_type },
    { u"glue-point-leaving-directions"_ustr,      EAS_glue_point_leaving_directions },
    { u"text-path"_ustr,                          EAS_text_path },
    { u"text-path-mode"_ustr,                     EAS_text_path_mode },
    { u"text-path-scale"_ustr,                    EAS_text_path_scale },
    { u"text-path-same-letter-heights"_ustr,      EAS_text_path_same_letter_heights },
    { u"modifiers"_ustr,                          EAS_modifiers },
    { u"equation"_ustr,                           EAS_equation },
    { u"formula"_ustr,                            EAS_formula },
    { u"handle"_ustr,                             EAS_handle },
    { u"handle-mirror-horizontal"_ustr,           EAS_handle_mirror_horizontal },
    { u"handle-mirror-vertical"_ustr,             EAS_handle_mirror_vertical },
    { u"handle-switched"_ustr,                    EAS_handle_switched },
    { u"handle-position"_ustr,                    EAS_handle_position },
    { u"handle-range-x-minimum"_ustr,             EAS_handle_range_x_minimum },
    { u"handle-range-x-maximum"_ustr,             EAS_handle_range_x_maximum },
    { u"handle-range-y-minimum"_ustr,             EAS_handle_range_y_minimum },
    { u"handle-range-y-maximum"_ustr,             EAS_handle_range_y_maximum },
    { u"handle-polar"_ustr,                       EAS_handle_polar },
    { u"handle-radius-range-minimum"_ustr,        EAS_handle_radius_range_minimum },
    { u"handle-radius-range-maximum"_ustr,        EAS_handle_radius_range_maximum },
    { u"sub-view-size"_ustr,                      EAS_sub_view_size },

    { u"CustomShapeEngine"_ustr,                  EAS_CustomShapeEngine },
    { u"CustomShapeData"_ustr,                    EAS_CustomShapeData },
    { u"Type"_ustr,                               EAS_Type },
    { u"MirroredX"_ustr,                          EAS_MirroredX },
    { u"MirroredY"_ustr,                          EAS_MirroredY },
    { u"ViewBox"_ustr,                            EAS_ViewBox },
    { u"TextRotateAngle"_ustr,                    EAS_TextRotateAngle },
    { u"TextPreRotateAngle"_ustr,                 EAS_TextPreRotateAngle },
    { u"ExtrusionAllowed"_ustr,                   EAS_ExtrusionAllowed },
    { u"TextPathAllowed"_ustr,                    EAS_TextPathAllowed },
    { u"ConcentricGradientFillAllowed"_ustr,      EAS_ConcentricGradientFillAllowed },
    { u"Extrusion"_ustr,                          EAS_Extrusion },
    { u"Equations"_ustr,                          EAS_Equations },
    { u"Equation"_ustr,                           EAS_Equation },
    { u"Path"_ustr,                               EAS_Path },
    { u"TextPath"_ustr,                           EAS_TextPath },
    { u"Handles"_ustr,                            EAS_Handles },
    { u"Handle"_ustr,                             EAS_Handle },
    { u"Brightness"_ustr,                         EAS_Brightness },
    { u"Depth"_ustr,                              EAS_Depth },
    { u"Diffusion"_ustr,                          EAS_Diffusion },
    { u"NumberOfLineSegments"_ustr,               EAS_NumberOfLineSegments },
    { u"LightFace"_ustr,                          EAS_LightFace },
    { u"FirstLightHarsh"_ustr,                    EAS_FirstLightHarsh },
    { u"SecondLightHarsh"_ustr,                   EAS_SecondLightHarsh },
    { u"FirstLightLevel"_ustr,                    EAS_FirstLightLevel },
    { u"SecondLightLevel"_ustr,                   EAS_SecondLightLevel },
    { u"FirstLightDirection"_ustr,                EAS_FirstLightDirection },
    { u"SecondLightDirection"_ustr,               EAS_SecondLightDirection },
    { u"Metal"_ustr,                              EAS_Metal },
    { u"MetalType"_ustr,                          EAS_MetalType },
    { u"ShadeMode"_ustr,                          EAS_ShadeMode },
    { u"RotateAngle"_ustr,                        EAS_RotateAngle },
    { u"RotationCenter"_ustr,                     EAS_RotationCenter },
    { u"Shininess"_ustr,                          EAS_Shininess },
    { u"Skew"_ustr,                               EAS_Skew },
    { u"Specularity"_ustr,                        EAS_Specularity },
    { u"ProjectionMode"_ustr,                     EAS_ProjectionMode },
    { u"ViewPoint"_ustr,                          EAS_ViewPoint },
    { u"Origin"_ustr,                             EAS_Origin },
    { u"Color"_ustr,                              EAS_Color },
    { u"Switched"_ustr,                           EAS_Switched },
    { u"Polar"_ustr,                              EAS_Polar },
    { u"RangeXMinimum"_ustr,                      EAS_RangeXMinimum },
    { u"RangeXMaximum"_ustr,                      EAS_RangeXMaximum },
    { u"RangeYMinimum"_ustr,                      EAS_RangeYMinimum },
    { u"RangeYMaximum"_ustr,                      EAS_RangeYMaximum },
    { u"RadiusRangeMinimum"_ustr,                 EAS_RadiusRangeMinimum },
    { u"RadiusRangeMaximum"_ustr,                 EAS_RadiusRangeMaximum },
    { u"Coordinates"_ustr,                        EAS_Coordinates },
    { u"Segments"_ustr,                           EAS_Segments },
    { u"StretchX"_ustr,                           EAS_StretchX },
    { u"StretchY"_ustr,                           EAS_StretchY },
    { u"TextFrames"_ustr,                         EAS_TextFrames },
    { u"GluePoints"_ustr,                         EAS_GluePoints },
    { u"GluePointLeavingDirections"_ustr,         EAS_GluePointLeavingDirections },
    { u"GluePointType"_ustr,                      EAS_GluePointType },
    { u"TextPathMode"_ustr,                       EAS_TextPathMode },
    { u"ScaleX"_ustr,                             EAS_ScaleX },
    { u"SameLetterHeights"_ustr,                  EAS_SameLetterHeights },
    { u"Position"_ustr,                           EAS_Position },
    { u"AdjustmentValues"_ustr,                   EAS_AdjustmentValues },
    { u"SubViewSize"_ustr,                        EAS_SubViewSize },

    { u"Last"_ustr,                               EAS_Last },
    { u"NotFound"_ustr,                           EAS_NotFound }
};

typedef std::unordered_map< OUString, EnhancedCustomShapeTokenEnum> TypeNameHashMap;
static const TypeNameHashMap& GetNameHashMap()
{
    static TypeNameHashMap aHashMap = []()
        {   // init hash map
            TypeNameHashMap res;
            for (auto const & pair : pTokenTableArray)
                res[pair.aS] = pair.pE;
            return res;
        }();

    return aHashMap;
}

EnhancedCustomShapeTokenEnum EASGet( const OUString& rShapeType )
{
    EnhancedCustomShapeTokenEnum eRetValue = EAS_NotFound;
    auto& rHashMap = GetNameHashMap();
    TypeNameHashMap::const_iterator aHashIter( rHashMap.find( rShapeType ) );
    if ( aHashIter != rHashMap.end() )
        eRetValue = (*aHashIter).second;
    return eRetValue;
}

EnhancedCustomShapeTokenEnum EASGet( sal_Int32 nToken )
{
    return EASGet(SvXMLImport::getNameFromToken(nToken));
}

const OUString & EASGet( const EnhancedCustomShapeTokenEnum eToken )
{
    sal_uInt32 i = eToken >= EAS_Last
        ? sal_uInt32(EAS_NotFound)
        : static_cast<sal_uInt32>(eToken);
    return pTokenTableArray[ i ].aS;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
