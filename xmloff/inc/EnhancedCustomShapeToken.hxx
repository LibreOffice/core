/*************************************************************************
 *
 *  $RCSfile: EnhancedCustomShapeToken.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 14:08:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _ENHANCED_CUSTOMSHAPE_TOKEN_HXX
#define _ENHANCED_CUSTOMSHAPE_TOKEN_HXX

#include <rtl/ustring.hxx>

namespace xmloff { namespace EnhancedCustomShapeToken {

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
        EAS_text_path_scale_x,
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

        EAS_CustomShapeEngine,
        EAS_CustomShapeData,
        EAS_Type,
        EAS_MirroredX,
        EAS_MirroredY,
        EAS_ViewBox,
        EAS_TextRotateAngle,
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

        EAS_Last,
        EAS_NotFound
    };

    EnhancedCustomShapeTokenEnum EASGet( const rtl::OUString& );
    rtl::OUString EASGet( const EnhancedCustomShapeTokenEnum );
}
}

#endif
