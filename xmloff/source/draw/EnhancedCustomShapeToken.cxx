/*************************************************************************
 *
 *  $RCSfile: EnhancedCustomShapeToken.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-04-02 13:52:27 $
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
#include "EnhancedCustomShapeToken.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <hash_map>

using namespace ::std;

namespace xmloff { namespace EnhancedCustomShapeToken {

struct TCheck
{
    bool operator()( const char* s1, const char* s2 ) const
    {
        return strcmp( s1, s2 ) == 0;
    }
};
typedef hash_map< const char*, EnhancedCustomShapeTokenEnum, std::hash<const char*>, TCheck> TypeNameHashMap;
static TypeNameHashMap* pHashMap = NULL;
static ::osl::Mutex& getHashMapMutex()
{
    static osl::Mutex s_aHashMapProtection;
    return s_aHashMapProtection;
}

struct TokenTable
{
    char*                           pS;
    EnhancedCustomShapeTokenEnum        pE;
};

static const TokenTable pTokenTableArray[] =
{
    { "predefined-type",                    EAS_predefined_type },
    { "mirror-horizontal",                  EAS_mirror_horizontal },
    { "mirror-vertical",                    EAS_mirror_vertical },
    { "coordinate-origin-x",                EAS_coordinate_origin_x },
    { "coordinate-origin-y",                EAS_coordinate_origin_y },
    { "coordinate-width",                   EAS_coordinate_width },
    { "coordinate-height",                  EAS_coordinate_height },
    { "text-rotate-angle",                  EAS_text_rotate_angle },
    { "extrusion-allowed",                  EAS_extrusion_allowed },
    { "extrusion-text-path-allowed",        EAS_text_path_allowed },
    { "extrusion-concentric-gradient-fill", EAS_concentric_gradient_fill_allowed },
    { "extrusion",                          EAS_extrusion },
    { "extrusion-auto-rotation-center",     EAS_extrusion_auto_rotation_center },
    { "extrusion-backward-depth",           EAS_extrusion_backward_depth },
    { "extrusion-brightness",               EAS_extrusion_brightness },
    { "extrusion-diffusity",                EAS_extrusion_diffusity },
    { "extrusion-edge",                     EAS_extrusion_edge },
    { "extrusion-facet",                    EAS_extrusion_facet },
    { "extrusion-foreward-depth",           EAS_extrusion_foreward_depth },
    { "extrusion-light-face",               EAS_extrusion_light_face },
    { "extrusion-light-harsh1",             EAS_extrusion_light_harsh1 },
    { "extrusion-light-harsh2",             EAS_extrusion_light_harsh2 },
    { "extrusion-light-livel1",             EAS_extrusion_light_level1 },
    { "extrusion-light-level2",             EAS_extrusion_light_level2 },
    { "extrusion-light-direction1",         EAS_extrusion_light_direction1 },
    { "extrusion-light-direction2",         EAS_extrusion_light_direction2 },
    { "extrusion-metal",                    EAS_extrusion_metal },
    { "extrusion-plane",                    EAS_extrusion_plane },
    { "extrusion-render-mode",              EAS_extrusion_render_mode },
    { "extrusion-rotation-angle-x",         EAS_extrusion_rotation_angle_x },
    { "extrusion-rotation-angle-y",         EAS_extrusion_rotation_angle_y },
    { "extrusion-rotation-center-x",        EAS_extrusion_rotation_center_x },
    { "extrusion-rotation-center-y",        EAS_extrusion_rotation_center_y },
    { "extrusion-rotation-center-z",        EAS_extrusion_rotation_center_z },
    { "extrusion-shininess",                EAS_extrusion_shininess },
    { "extrusion-skew",                     EAS_extrusion_skew },
    { "extrusion-skew-angle",               EAS_extrusion_skew_angle },
    { "extrusion-specularity",              EAS_extrusion_specularity },
    { "extrusion-parallel",                 EAS_extrusion_parallel },
    { "extrusion-viewpoint",                EAS_extrusion_viewpoint },
    { "extrusion-origin-x",                 EAS_extrusion_origin_x },
    { "extrusion-origin-y",                 EAS_extrusion_origin_y },
    { "extrusion-color",                    EAS_extrusion_color },
    { "enhanced-path",                      EAS_enhanced_path },
    { "path-stretchpoint-x",                EAS_path_stretchpoint_x },
    { "path-stretchpoint-y",                EAS_path_stretchpoint_y },
    { "text-frames",                        EAS_text_frames },
    { "glue-points",                        EAS_glue_points },
    { "glue-point-type",                    EAS_glue_point_type },
    { "glue-point-leaving-directions",      EAS_glue_point_leaving_directions },
    { "text-path",                          EAS_text_path },
    { "text-path-fit-text",                 EAS_text_path_fit_text },
    { "text-path-fit-shape",                EAS_text_path_fit_shape },
    { "text-path-scale-x",                  EAS_text_path_scale_x },
    { "text-path-same-letter-heights",      EAS_text_path_same_letter_heights },
    { "adjustments",                        EAS_adjustments },
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
    { "callout",                            EAS_callout },
    { "callout-accent-bar",                 EAS_callout_accent_bar },
    { "callout-angle",                      EAS_callout_angle },
    { "callout-drop-distance",              EAS_callout_drop_distance },
    { "callout-drop",                       EAS_callout_drop },
    { "callout-drop-automatic",             EAS_callout_drop_automatic },
    { "callout-gap",                        EAS_callout_gap },
    { "callout-length",                     EAS_callout_length },
    { "callout-length-specified",           EAS_callout_length_specified },
    { "callout-flip-x",                     EAS_callout_flip_x },
    { "callout-flip-y",                     EAS_callout_flip_y },
    { "callout-text-border",                EAS_callout_text_border },
    { "callout-type",                       EAS_callout_type },

    { "CustomShapeEngine",                  EAS_CustomShapeEngine },
    { "CustomShapeData",                        EAS_CustomShapeData },
    { "PredefinedType",                     EAS_PredefinedType },
    { "MirroredX",                          EAS_MirroredX },
    { "MirroredY",                          EAS_MirroredY },
    { "CoordinateOrigin",                   EAS_CoordinateOrigin },
    { "CoordinateSize",                     EAS_CoordinateSize },
    { "TextRotateAngle",                    EAS_TextRotateAngle },
    { "ExtrusionAllowed",                   EAS_ExtrusionAllowed },
    { "TextPathAllowed",                    EAS_TextPathAllowed },
    { "ConcentricGradientFillAllowed",      EAS_ConcentricGradientFillAllowed },
    { "Callout",                            EAS_Callout },
    { "Extrusion",                          EAS_Extrusion },
    { "Equations",                          EAS_Equations },
    { "Equation",                           EAS_Equation },
    { "Path",                               EAS_Path },
    { "TextPath",                           EAS_TextPath },
    { "Handles",                            EAS_Handles },
    { "Handle",                             EAS_Handle },
    { "On",                                 EAS_On },
    { "AutoRotationCenter",                 EAS_AutoRotationCenter },
    { "BackwardDepth",                      EAS_BackwardDepth },
    { "Brightness",                         EAS_Brightness },
    { "Diffusity",                          EAS_Diffusity },
    { "Edge",                               EAS_Edge },
    { "Facet",                              EAS_Facet },
    { "ForewardDepth",                      EAS_ForewardDepth },
    { "LightFace",                          EAS_LightFace },
    { "LightHarsh1",                        EAS_LightHarsh1 },
    { "LightHarsh2",                        EAS_LightHarsh2 },
    { "LightLevel1",                        EAS_LightLevel1 },
    { "LightLevel2",                        EAS_LightLevel2 },
    { "LightDirection1",                    EAS_LightDirection1 },
    { "LightDirection2",                    EAS_LightDirection2 },
    { "Metal",                              EAS_Metal },
    { "Plane",                              EAS_Plane },
    { "RenderMode",                         EAS_RenderMode },
    { "AngleX",                             EAS_AngleX },
    { "AngleY",                             EAS_AngleY },
    { "RotationCenterX",                    EAS_RotationCenterX },
    { "RotationCenterY",                    EAS_RotationCenterY },
    { "RotationCenterZ",                    EAS_RotationCenterZ },
    { "Shininess",                          EAS_Shininess },
    { "Skew",                               EAS_Skew },
    { "SkewAngle",                          EAS_SkewAngle },
    { "Specularity",                        EAS_Specularity },
    { "Parallel",                           EAS_Parallel },
    { "ViewPoint",                          EAS_ViewPoint },
    { "OriginX",                            EAS_OriginX },
    { "OriginY",                            EAS_OriginY },
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
    { "StretchPoint",                       EAS_StretchPoint },
    { "TextFrames",                         EAS_TextFrames },
    { "GluePoints",                         EAS_GluePoints },
    { "GluePointLeavingDirections",         EAS_GluePointLeavingDirections },
    { "GluePointType",                      EAS_GluePointType },
    { "FitPath",                            EAS_FitPath },
    { "FitShape",                           EAS_FitShape },
    { "ScaleX",                             EAS_ScaleX },
    { "SameLetterHeights",                  EAS_SameLetterHeights },
    { "AccentBar",                          EAS_AccentBar },
    { "Angle",                              EAS_Angle },
    { "Distance",                           EAS_Distance },
    { "Drop",                               EAS_Drop },
    { "DropAuto",                           EAS_DropAuto },
    { "Gap",                                EAS_Gap },
    { "Length",                             EAS_Length },
    { "LengthSpecified",                    EAS_LengthSpecified },
    { "FlipX",                              EAS_FlipX },
    { "FlipY",                              EAS_FlipY },
    { "TextBorder",                         EAS_TextBorder },
    { "Type",                               EAS_Type },
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
