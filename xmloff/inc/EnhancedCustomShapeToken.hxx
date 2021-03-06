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

#include <rtl/ustring.hxx>

namespace xmloff::EnhancedCustomShapeToken {

    enum EnhancedCustomShapeTokenEnum
    {
        EAS_type,
        EAS_name,
        EAS_mirror_horizontal,
        EAS_mirror_vertical,
        EAS_viewBox,
        EAS_text_rotate_angle,
        EAS_extrusion_allowed,
        EAS_text_path_allowed,
        EAS_concentric_gradient_fill_allowed,
        EAS_extrusion,
        EAS_extrusion_brightness,
        EAS_extrusion_depth,
        EAS_extrusion_diffusion,
        EAS_extrusion_number_of_line_segments,
        EAS_extrusion_light_face,
        EAS_extrusion_first_light_harsh,
        EAS_extrusion_second_light_harsh,
        EAS_extrusion_first_light_level,
        EAS_extrusion_second_light_level,
        EAS_extrusion_first_light_direction,
        EAS_extrusion_second_light_direction,
        EAS_extrusion_metal,
        EAS_shade_mode,
        EAS_extrusion_rotation_angle,
        EAS_extrusion_rotation_center,
        EAS_extrusion_shininess,
        EAS_extrusion_skew,
        EAS_extrusion_specularity,
        EAS_projection,
        EAS_extrusion_viewpoint,
        EAS_extrusion_origin,
        EAS_extrusion_color,
        EAS_enhanced_path,
        EAS_path_stretchpoint_x,
        EAS_path_stretchpoint_y,
        EAS_text_areas,
        EAS_glue_points,
        EAS_glue_point_type,
        EAS_glue_point_leaving_directions,
        EAS_text_path,
        EAS_text_path_mode,
        EAS_text_path_scale,
        EAS_text_path_same_letter_heights,
        EAS_modifiers,
        EAS_equation,
        EAS_formula,
        EAS_handle,
        EAS_handle_mirror_horizontal,
        EAS_handle_mirror_vertical,
        EAS_handle_switched,
        EAS_handle_position,
        EAS_handle_range_x_minimum,
        EAS_handle_range_x_maximum,
        EAS_handle_range_y_minimum,
        EAS_handle_range_y_maximum,
        EAS_handle_polar,
        EAS_handle_radius_range_minimum,
        EAS_handle_radius_range_maximum,
        EAS_sub_view_size,

        EAS_CustomShapeEngine,
        EAS_CustomShapeData,
        EAS_Type,
        EAS_MirroredX,
        EAS_MirroredY,
        EAS_ViewBox,
        EAS_TextRotateAngle,
        EAS_TextPreRotateAngle,
        EAS_ExtrusionAllowed,
        EAS_ConcentricGradientFillAllowed,
        EAS_TextPathAllowed,
        EAS_Extrusion,
        EAS_Equations,
        EAS_Equation,
        EAS_Path,
        EAS_TextPath,
        EAS_Handles,
        EAS_Handle,
        EAS_Brightness,
        EAS_Depth,
        EAS_Diffusion,
        EAS_NumberOfLineSegments,
        EAS_LightFace,
        EAS_FirstLightHarsh,
        EAS_SecondLightHarsh,
        EAS_FirstLightLevel,
        EAS_SecondLightLevel,
        EAS_FirstLightDirection,
        EAS_SecondLightDirection,
        EAS_Metal,
        EAS_ShadeMode,
        EAS_RotateAngle,
        EAS_RotationCenter,
        EAS_Shininess,
        EAS_Skew,
        EAS_Specularity,
        EAS_ProjectionMode,
        EAS_ViewPoint,
        EAS_Origin,
        EAS_Color,
        EAS_Switched,
        EAS_Polar,
        EAS_RangeXMinimum,
        EAS_RangeXMaximum,
        EAS_RangeYMinimum,
        EAS_RangeYMaximum,
        EAS_RadiusRangeMinimum,
        EAS_RadiusRangeMaximum,
        EAS_Coordinates,
        EAS_Segments,
        EAS_StretchX,
        EAS_StretchY,
        EAS_TextFrames,
        EAS_GluePoints,
        EAS_GluePointLeavingDirections,
        EAS_GluePointType,
        EAS_TextPathMode,
        EAS_ScaleX,
        EAS_SameLetterHeights,
        EAS_Position,
        EAS_AdjustmentValues,
        EAS_SubViewSize,

        EAS_Last,
        EAS_NotFound
    };

    EnhancedCustomShapeTokenEnum EASGet( const OUString& );
    EnhancedCustomShapeTokenEnum EASGet( sal_Int32 nToken );
    OUString EASGet( const EnhancedCustomShapeTokenEnum );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
