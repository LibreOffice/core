/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <map>

#include "TextEffectsHandler.hxx"

#include <rtl/ustrbuf.hxx>
#include <comphelper/string.hxx>
#include <ooxml/resourceids.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

namespace writerfilter::dmapper
{

using namespace com::sun::star;

namespace
{

OUString lclGetNameForElementId(sal_uInt32 aId)
{
    static std::map<sal_uInt32, OUString> aIdMap;
    if(aIdMap.empty())
    {
        aIdMap[NS_ooxml::LN_EG_ColorChoice_srgbClr]             = "srgbClr";
        aIdMap[NS_ooxml::LN_EG_ColorChoice_schemeClr]           = "schemeClr";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_tint]             = "tint";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_shade]            = "shade";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_alpha]            = "alpha";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_hueMod]           = "hueMod";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_sat]              = "sat";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_satOff]           = "satOff";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_satMod]           = "satMod";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_lum]              = "lum";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_lumOff]           = "lumOff";
        aIdMap[NS_ooxml::LN_EG_ColorTransform_lumMod]           = "lumMod";
        aIdMap[NS_ooxml::LN_EG_FillProperties_noFill]           = "noFill";
        aIdMap[NS_ooxml::LN_EG_FillProperties_solidFill]        = "solidFill";
        aIdMap[NS_ooxml::LN_EG_FillProperties_gradFill]         = "gradFill";
        aIdMap[NS_ooxml::LN_CT_GradientFillProperties_gsLst]    = "gsLst";
        aIdMap[NS_ooxml::LN_CT_GradientStopList_gs]             = "gs";
        aIdMap[NS_ooxml::LN_CT_GradientStop_pos]                = "pos";
        aIdMap[NS_ooxml::LN_EG_ShadeProperties_lin]             = "lin";
        aIdMap[NS_ooxml::LN_EG_ShadeProperties_path]            = "path";
        aIdMap[NS_ooxml::LN_CT_PathShadeProperties_fillToRect]  = "fillToRect";
        aIdMap[NS_ooxml::LN_EG_LineDashProperties_prstDash]     = "prstDash";
        aIdMap[NS_ooxml::LN_EG_LineJoinProperties_round]        = "round";
        aIdMap[NS_ooxml::LN_EG_LineJoinProperties_bevel]        = "bevel";
        aIdMap[NS_ooxml::LN_EG_LineJoinProperties_miter]        = "miter";
        aIdMap[NS_ooxml::LN_CT_Scene3D_camera]                  = "camera";
        aIdMap[NS_ooxml::LN_CT_Scene3D_lightRig]                = "lightRig";
        aIdMap[NS_ooxml::LN_CT_LightRig_rot]                    = "rot";
        aIdMap[NS_ooxml::LN_CT_Props3D_bevelT]                  = "bevelT";
        aIdMap[NS_ooxml::LN_CT_Props3D_bevelB]                  = "bevelB";
        aIdMap[NS_ooxml::LN_CT_Props3D_extrusionClr]            = "extrusionClr";
        aIdMap[NS_ooxml::LN_CT_Props3D_contourClr]              = "contourClr";
        aIdMap[NS_ooxml::LN_CT_StylisticSets_styleSet]          = "styleSet";
        aIdMap[NS_ooxml::LN_cntxtAlts_cntxtAlts]                = "cntxtAlts";
    }
    return aIdMap[aId];
}

constexpr OUString constAttributesSequenceName = u"attributes"_ustr;

}

OUString TextEffectsHandler::getSchemeColorValTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_SchemeColorVal_bg1: return "bg1";
        case NS_ooxml::LN_ST_SchemeColorVal_tx1: return "tx1";
        case NS_ooxml::LN_ST_SchemeColorVal_bg2: return "bg2";
        case NS_ooxml::LN_ST_SchemeColorVal_tx2: return "tx2";
        case NS_ooxml::LN_ST_SchemeColorVal_accent1: return "accent1";
        case NS_ooxml::LN_ST_SchemeColorVal_accent2: return "accent2";
        case NS_ooxml::LN_ST_SchemeColorVal_accent3: return "accent3";
        case NS_ooxml::LN_ST_SchemeColorVal_accent4: return "accent4";
        case NS_ooxml::LN_ST_SchemeColorVal_accent5: return "accent5";
        case NS_ooxml::LN_ST_SchemeColorVal_accent6: return "accent6";
        case NS_ooxml::LN_ST_SchemeColorVal_hlink: return "hlink";
        case NS_ooxml::LN_ST_SchemeColorVal_folHlink: return "folHlink";
        case NS_ooxml::LN_ST_SchemeColorVal_dk1: return "dk1";
        case NS_ooxml::LN_ST_SchemeColorVal_lt1: return "lt1";
        case NS_ooxml::LN_ST_SchemeColorVal_dk2: return "dk2";
        case NS_ooxml::LN_ST_SchemeColorVal_lt2: return "lt2";
        case NS_ooxml::LN_ST_SchemeColorVal_phClr: return "phClr";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getRectAlignmentString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_RectAlignment_none: return "none";
        case NS_ooxml::LN_ST_RectAlignment_tl: return "tl";
        case NS_ooxml::LN_ST_RectAlignment_t: return "t";
        case NS_ooxml::LN_ST_RectAlignment_tr: return "tr";
        case NS_ooxml::LN_ST_RectAlignment_l: return "l";
        case NS_ooxml::LN_ST_RectAlignment_ctr: return "ctr";
        case NS_ooxml::LN_ST_RectAlignment_r: return "r";
        case NS_ooxml::LN_ST_RectAlignment_bl: return "bl";
        case NS_ooxml::LN_ST_RectAlignment_b: return "b";
        case NS_ooxml::LN_ST_RectAlignment_br: return "br";

        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getLineCapString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_LineCap_rnd: return "rnd";
        case NS_ooxml::LN_ST_LineCap_sq: return "sq";
        case NS_ooxml::LN_ST_LineCap_flat: return "flat";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getCompoundLineString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_CompoundLine_sng: return "sng";
        case NS_ooxml::LN_ST_CompoundLine_dbl: return "dbl";
        case NS_ooxml::LN_ST_CompoundLine_thickThin: return "thickThin";
        case NS_ooxml::LN_ST_CompoundLine_thinThick: return "thinThick";
        case NS_ooxml::LN_ST_CompoundLine_tri: return "tri";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPenAlignmentString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PenAlignment_ctr: return "ctr";
        case NS_ooxml::LN_ST_PenAlignment_in: return "in";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getOnOffString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_OnOff_true: return "true";
        case NS_ooxml::LN_ST_OnOff_false: return "false";
        case NS_ooxml::LN_ST_OnOff_1: return "1";
        case NS_ooxml::LN_ST_OnOff_0: return "0";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPathShadeTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PathShadeType_shape: return "shape";
        case NS_ooxml::LN_ST_PathShadeType_circle: return "circle";
        case NS_ooxml::LN_ST_PathShadeType_rect: return "rect";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPresetLineDashValString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PresetLineDashVal_solid: return "solid";
        case NS_ooxml::LN_ST_PresetLineDashVal_dot: return "dot";
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDot: return "sysDot";
        case NS_ooxml::LN_ST_PresetLineDashVal_dash: return "dash";
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDash: return "sysDash";
        case NS_ooxml::LN_ST_PresetLineDashVal_lgDash: return "lgDash";
        case NS_ooxml::LN_ST_PresetLineDashVal_dashDot: return "dashDot";
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDashDot: return "sysDashDot";
        case NS_ooxml::LN_ST_PresetLineDashVal_lgDashDot: return "lgDashDot";
        case NS_ooxml::LN_ST_PresetLineDashVal_lgDashDotDot: return "lgDashDotDot";
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDashDotDot: return "sysDashDotDot";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPresetCameraTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueTopLeft: return "legacyObliqueTopLeft";
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueTop: return "legacyObliqueTop";
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueTopRight: return "legacyObliqueTopRight";
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueLeft: return "legacyObliqueLeft";
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueFront: return "legacyObliqueFront";
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueRight: return "legacyObliqueRight";
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueBottomLeft: return "legacyObliqueBottomLeft";
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueBottom: return "legacyObliqueBottom";
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueBottomRight: return "legacyObliqueBottomRight";
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveTopLeft: return "legacyPerspectiveTopLeft";
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveTop: return "legacyPerspectiveTop";
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveTopRight: return "legacyPerspectiveTopRight";
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveLeft: return "legacyPerspectiveLeft";
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveFront: return "legacyPerspectiveFront";
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveRight: return "legacyPerspectiveRight";
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveBottomLeft: return "legacyPerspectiveBottomLeft";
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveBottom: return "legacyPerspectiveBottom";
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveBottomRight: return "legacyPerspectiveBottomRight";
        case NS_ooxml::LN_ST_PresetCameraType_orthographicFront: return "orthographicFront";
        case NS_ooxml::LN_ST_PresetCameraType_isometricTopUp: return "isometricTopUp";
        case NS_ooxml::LN_ST_PresetCameraType_isometricTopDown: return "isometricTopDown";
        case NS_ooxml::LN_ST_PresetCameraType_isometricBottomUp: return "isometricBottomUp";
        case NS_ooxml::LN_ST_PresetCameraType_isometricBottomDown: return "isometricBottomDown";
        case NS_ooxml::LN_ST_PresetCameraType_isometricLeftUp: return "isometricLeftUp";
        case NS_ooxml::LN_ST_PresetCameraType_isometricLeftDown: return "isometricLeftDown";
        case NS_ooxml::LN_ST_PresetCameraType_isometricRightUp: return "isometricRightUp";
        case NS_ooxml::LN_ST_PresetCameraType_isometricRightDown: return "isometricRightDown";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis1Left: return "isometricOffAxis1Left";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis1Right: return "isometricOffAxis1Right";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis1Top: return "isometricOffAxis1Top";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis2Left: return "isometricOffAxis2Left";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis2Right: return "isometricOffAxis2Right";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis2Top: return "isometricOffAxis2Top";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis3Left: return "isometricOffAxis3Left";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis3Right: return "isometricOffAxis3Right";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis3Bottom: return "isometricOffAxis3Bottom";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis4Left: return "isometricOffAxis4Left";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis4Right: return "isometricOffAxis4Right";
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis4Bottom: return "isometricOffAxis4Bottom";
        case NS_ooxml::LN_ST_PresetCameraType_obliqueTopLeft: return "obliqueTopLeft";
        case NS_ooxml::LN_ST_PresetCameraType_obliqueTop: return "obliqueTop";
        case NS_ooxml::LN_ST_PresetCameraType_obliqueTopRight: return "obliqueTopRight";
        case NS_ooxml::LN_ST_PresetCameraType_obliqueLeft: return "obliqueLeft";
        case NS_ooxml::LN_ST_PresetCameraType_obliqueRight: return "obliqueRight";
        case NS_ooxml::LN_ST_PresetCameraType_obliqueBottomLeft: return "obliqueBottomLeft";
        case NS_ooxml::LN_ST_PresetCameraType_obliqueBottom: return "obliqueBottom";
        case NS_ooxml::LN_ST_PresetCameraType_obliqueBottomRight: return "obliqueBottomRight";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveFront: return "perspectiveFront";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveLeft: return "perspectiveLeft";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveRight: return "perspectiveRight";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveAbove: return "perspectiveAbove";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveBelow: return "perspectiveBelow";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveAboveLeftFacing: return "perspectiveAboveLeftFacing";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveAboveRightFacing: return "perspectiveAboveRightFacing";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveContrastingLeftFacing: return "perspectiveContrastingLeftFacing";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveContrastingRightFacing: return "perspectiveContrastingRightFacing";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicLeftFacing: return "perspectiveHeroicLeftFacing";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicRightFacing: return "perspectiveHeroicRightFacing";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicExtremeLeftFacing: return "perspectiveHeroicExtremeLeftFacing";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicExtremeRightFacing: return "perspectiveHeroicExtremeRightFacing";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveRelaxed: return "perspectiveRelaxed";
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveRelaxedModerately: return "perspectiveRelaxedModerately";
        default: break;
    }
    return OUString();
}


OUString TextEffectsHandler::getLightRigTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_LightRigType_legacyFlat1: return "legacyFlat1";
        case NS_ooxml::LN_ST_LightRigType_legacyFlat2: return "legacyFlat2";
        case NS_ooxml::LN_ST_LightRigType_legacyFlat3: return "legacyFlat3";
        case NS_ooxml::LN_ST_LightRigType_legacyFlat4: return "legacyFlat4";
        case NS_ooxml::LN_ST_LightRigType_legacyNormal1: return "legacyNormal1";
        case NS_ooxml::LN_ST_LightRigType_legacyNormal2: return "legacyNormal2";
        case NS_ooxml::LN_ST_LightRigType_legacyNormal3: return "legacyNormal3";
        case NS_ooxml::LN_ST_LightRigType_legacyNormal4: return "legacyNormal4";
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh1: return "legacyHarsh1";
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh2: return "legacyHarsh2";
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh3: return "legacyHarsh3";
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh4: return "legacyHarsh4";
        case NS_ooxml::LN_ST_LightRigType_threePt: return "threePt";
        case NS_ooxml::LN_ST_LightRigType_balanced: return "balanced";
        case NS_ooxml::LN_ST_LightRigType_soft: return "soft";
        case NS_ooxml::LN_ST_LightRigType_harsh: return "harsh";
        case NS_ooxml::LN_ST_LightRigType_flood: return "flood";
        case NS_ooxml::LN_ST_LightRigType_contrasting: return "contrasting";
        case NS_ooxml::LN_ST_LightRigType_morning: return "morning";
        case NS_ooxml::LN_ST_LightRigType_sunrise: return "sunrise";
        case NS_ooxml::LN_ST_LightRigType_sunset: return "sunset";
        case NS_ooxml::LN_ST_LightRigType_chilly: return "chilly";
        case NS_ooxml::LN_ST_LightRigType_freezing: return "freezing";
        case NS_ooxml::LN_ST_LightRigType_flat: return "flat";
        case NS_ooxml::LN_ST_LightRigType_twoPt: return "twoPt";
        case NS_ooxml::LN_ST_LightRigType_glow: return "glow";
        case NS_ooxml::LN_ST_LightRigType_brightRoom: return "brightRoom";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getLightRigDirectionString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_LightRigDirection_tl: return "tl";
        case NS_ooxml::LN_ST_LightRigDirection_t: return "t";
        case NS_ooxml::LN_ST_LightRigDirection_tr: return "tr";
        case NS_ooxml::LN_ST_LightRigDirection_l: return "l";
        case NS_ooxml::LN_ST_LightRigDirection_r: return "r";
        case NS_ooxml::LN_ST_LightRigDirection_bl: return "bl";
        case NS_ooxml::LN_ST_LightRigDirection_b: return "b";
        case NS_ooxml::LN_ST_LightRigDirection_br: return "br";

        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getBevelPresetTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_BevelPresetType_relaxedInset: return "relaxedInset";
        case NS_ooxml::LN_ST_BevelPresetType_circle: return "circle";
        case NS_ooxml::LN_ST_BevelPresetType_slope: return "slope";
        case NS_ooxml::LN_ST_BevelPresetType_cross: return "cross";
        case NS_ooxml::LN_ST_BevelPresetType_angle: return "angle";
        case NS_ooxml::LN_ST_BevelPresetType_softRound: return "softRound";
        case NS_ooxml::LN_ST_BevelPresetType_convex: return "convex";
        case NS_ooxml::LN_ST_BevelPresetType_coolSlant: return "coolSlant";
        case NS_ooxml::LN_ST_BevelPresetType_divot: return "divot";
        case NS_ooxml::LN_ST_BevelPresetType_riblet: return "riblet";
        case NS_ooxml::LN_ST_BevelPresetType_hardEdge: return "hardEdge";
        case NS_ooxml::LN_ST_BevelPresetType_artDeco: return "artDeco";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPresetMaterialTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PresetMaterialType_legacyMatte: return "legacyMatte";
        case NS_ooxml::LN_ST_PresetMaterialType_legacyPlastic: return "legacyPlastic";
        case NS_ooxml::LN_ST_PresetMaterialType_legacyMetal: return "legacyMetal";
        case NS_ooxml::LN_ST_PresetMaterialType_legacyWireframe: return "legacyWireframe";
        case NS_ooxml::LN_ST_PresetMaterialType_matte: return "matte";
        case NS_ooxml::LN_ST_PresetMaterialType_plastic: return "plastic";
        case NS_ooxml::LN_ST_PresetMaterialType_metal: return "metal";
        case NS_ooxml::LN_ST_PresetMaterialType_warmMatte: return "warmMatte";
        case NS_ooxml::LN_ST_PresetMaterialType_translucentPowder: return "translucentPowder";
        case NS_ooxml::LN_ST_PresetMaterialType_powder: return "powder";
        case NS_ooxml::LN_ST_PresetMaterialType_dkEdge: return "dkEdge";
        case NS_ooxml::LN_ST_PresetMaterialType_softEdge: return "softEdge";
        case NS_ooxml::LN_ST_PresetMaterialType_clear: return "clear";
        case NS_ooxml::LN_ST_PresetMaterialType_flat: return "flat";
        case NS_ooxml::LN_ST_PresetMaterialType_softmetal: return "softmetal";
        case NS_ooxml::LN_ST_PresetMaterialType_none: return "none";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getLigaturesString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_Ligatures_none: return "none";
        case NS_ooxml::LN_ST_Ligatures_standard: return "standard";
        case NS_ooxml::LN_ST_Ligatures_contextual: return "contextual";
        case NS_ooxml::LN_ST_Ligatures_historical: return "historical";
        case NS_ooxml::LN_ST_Ligatures_discretional: return "discretional";
        case NS_ooxml::LN_ST_Ligatures_standardContextual: return "standardContextual";
        case NS_ooxml::LN_ST_Ligatures_standardHistorical: return "standardHistorical";
        case NS_ooxml::LN_ST_Ligatures_contextualHistorical: return "contextualHistorical";
        case NS_ooxml::LN_ST_Ligatures_standardDiscretional: return "standardDiscretional";
        case NS_ooxml::LN_ST_Ligatures_contextualDiscretional: return "contextualDiscretional";
        case NS_ooxml::LN_ST_Ligatures_historicalDiscretional: return "historicalDiscretional";
        case NS_ooxml::LN_ST_Ligatures_standardContextualHistorical: return "standardContextualHistorical";
        case NS_ooxml::LN_ST_Ligatures_standardContextualDiscretional: return "standardContextualDiscretional";
        case NS_ooxml::LN_ST_Ligatures_standardHistoricalDiscretional: return "standardHistoricalDiscretional";
        case NS_ooxml::LN_ST_Ligatures_contextualHistoricalDiscretional: return "contextualHistoricalDiscretional";
        case NS_ooxml::LN_ST_Ligatures_all: return "all";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getNumFormString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_NumForm_default: return "default";
        case NS_ooxml::LN_ST_NumForm_lining: return "lining";
        case NS_ooxml::LN_ST_NumForm_oldStyle: return "oldStyle";
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getNumSpacingString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_NumSpacing_default: return "default";
        case NS_ooxml::LN_ST_NumSpacing_proportional: return "proportional";
        case NS_ooxml::LN_ST_NumSpacing_tabular: return "tabular";
        default: break;
    }
    return OUString();
}

void TextEffectsHandler::convertElementIdToPropertyId(sal_Int32 aElementId)
{
    switch(aElementId)
    {
        case NS_ooxml::LN_glow_glow:
            maPropertyId = PROP_CHAR_GLOW_TEXT_EFFECT;
            maElementName = "glow";
            break;
        case NS_ooxml::LN_shadow_shadow:
            maPropertyId = PROP_CHAR_SHADOW_TEXT_EFFECT;
            maElementName = "shadow";
            break;
        case NS_ooxml::LN_reflection_reflection:
            maPropertyId = PROP_CHAR_REFLECTION_TEXT_EFFECT;
            maElementName = "reflection";
            break;
        case NS_ooxml::LN_textOutline_textOutline:
            maPropertyId = PROP_CHAR_TEXTOUTLINE_TEXT_EFFECT;
            maElementName = "textOutline";
            break;
        case NS_ooxml::LN_textFill_textFill:
            maPropertyId = PROP_CHAR_TEXTFILL_TEXT_EFFECT;
            maElementName = "textFill";
            break;
        case NS_ooxml::LN_scene3d_scene3d:
            maPropertyId = PROP_CHAR_SCENE3D_TEXT_EFFECT;
            maElementName = "scene3d";
            break;
        case NS_ooxml::LN_props3d_props3d:
            maPropertyId = PROP_CHAR_PROPS3D_TEXT_EFFECT;
            maElementName = "props3d";
            break;
        case NS_ooxml::LN_ligatures_ligatures:
            maPropertyId = PROP_CHAR_LIGATURES_TEXT_EFFECT;
            maElementName = "ligatures";
            break;
        case NS_ooxml::LN_numForm_numForm:
            maPropertyId = PROP_CHAR_NUMFORM_TEXT_EFFECT;
            maElementName = "numForm";
            break;
        case NS_ooxml::LN_numSpacing_numSpacing:
            maPropertyId = PROP_CHAR_NUMSPACING_TEXT_EFFECT;
            maElementName = "numSpacing";
            break;
        case NS_ooxml::LN_stylisticSets_stylisticSets:
            maPropertyId = PROP_CHAR_STYLISTICSETS_TEXT_EFFECT;
            maElementName = "stylisticSets";
            break;
        case NS_ooxml::LN_cntxtAlts_cntxtAlts:
            maPropertyId = PROP_CHAR_CNTXTALTS_TEXT_EFFECT;
            maElementName = "cntxtAlts";
            break;
        default:
            break;
    }
}

TextEffectsHandler::TextEffectsHandler(sal_uInt32 aElementId) :
    LoggedProperties("TextEffectsHandler")
{
    convertElementIdToPropertyId(aElementId);
    mpGrabBagStack.reset(new oox::GrabBagStack(maElementName));
}

TextEffectsHandler::~TextEffectsHandler()
{
}


void TextEffectsHandler::lcl_attribute(Id aName, Value& aValue)
{
    if (mpGrabBagStack->getCurrentName() != constAttributesSequenceName)
        mpGrabBagStack->push(constAttributesSequenceName);

    switch(aName)
    {
        case NS_ooxml::LN_CT_Percentage_val:
        case NS_ooxml::LN_CT_PositiveFixedPercentage_val:
        case NS_ooxml::LN_CT_PositivePercentage_val:
            mpGrabBagStack->addInt32("val", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Glow_rad:
            mpGrabBagStack->addInt32("rad", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_SchemeColor_val:
            {
                OUString aString = getSchemeColorValTypeString(sal_Int32(aValue.getInt()));
                mpGrabBagStack->addString("val", aString);
            }
            break;
        case NS_ooxml::LN_CT_SRgbColor_val:
            {
                OUString aBuffer = OUString::number(aValue.getInt(), 16);
                OUStringBuffer aString;
                comphelper::string::padToLength(aString, 6 - aBuffer.getLength(), '0');
                aString.append(aBuffer.getStr());
                mpGrabBagStack->addString("val", aString.makeStringAndClear().toAsciiUpperCase());
            }
            break;
        case NS_ooxml::LN_CT_Shadow_blurRad:
        case NS_ooxml::LN_CT_Reflection_blurRad:
            mpGrabBagStack->addInt32("blurRad", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_stA:
            mpGrabBagStack->addInt32("stA", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_stPos:
            mpGrabBagStack->addInt32("stPos", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_endA:
            mpGrabBagStack->addInt32("endA", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_endPos:
            mpGrabBagStack->addInt32("endPos", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_dist:
        case NS_ooxml::LN_CT_Reflection_dist:
            mpGrabBagStack->addInt32("dist", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_dir:
        case NS_ooxml::LN_CT_Reflection_dir:
            mpGrabBagStack->addInt32("dir", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_fadeDir:
            mpGrabBagStack->addInt32("fadeDir", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_sx:
        case NS_ooxml::LN_CT_Reflection_sx:
            mpGrabBagStack->addInt32("sx", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_sy:
        case NS_ooxml::LN_CT_Reflection_sy:
            mpGrabBagStack->addInt32("sy", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_kx:
        case NS_ooxml::LN_CT_Reflection_kx:
            mpGrabBagStack->addInt32("kx", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_ky:
        case NS_ooxml::LN_CT_Reflection_ky:
            mpGrabBagStack->addInt32("ky", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_algn:
        case NS_ooxml::LN_CT_Reflection_algn:
            {
                uno::Any aAny(getRectAlignmentString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("algn", aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_w:
            mpGrabBagStack->addInt32("w", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_cap:
            {
                uno::Any aAny(getLineCapString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("cap", aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_cmpd:
            {
                uno::Any aAny(getCompoundLineString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("cmpd", aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_algn:
            {
                uno::Any aAny(getPenAlignmentString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("algn", aAny);
            }
            break;
        case NS_ooxml::LN_CT_GradientStop_pos:
            mpGrabBagStack->addInt32("pos", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_LinearShadeProperties_ang:
            mpGrabBagStack->addInt32("ang", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_LinearShadeProperties_scaled:
            {
                uno::Any aAny(getOnOffString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("scaled", aAny);
            }
            break;
        case NS_ooxml::LN_CT_PathShadeProperties_path:
            {
                uno::Any aAny(getPathShadeTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("path", aAny);
            }
            break;
        case NS_ooxml::LN_CT_RelativeRect_l:
            mpGrabBagStack->addInt32("l", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_RelativeRect_t:
            mpGrabBagStack->addInt32("t", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_RelativeRect_r:
            mpGrabBagStack->addInt32("r", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_RelativeRect_b:
            mpGrabBagStack->addInt32("b", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_PresetLineDashProperties_val:
            {
                uno::Any aAny(getPresetLineDashValString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        case NS_ooxml::LN_CT_LineJoinMiterProperties_lim:
            mpGrabBagStack->addInt32("lim", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Camera_prst:
            {
                uno::Any aAny(getPresetCameraTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("prst", aAny);
            }
            break;
        case NS_ooxml::LN_CT_LightRig_rig:
            {
                uno::Any aAny(getLightRigTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("rig", aAny);
            }
            break;
        case NS_ooxml::LN_CT_LightRig_dir:
            {
                uno::Any aAny(getLightRigDirectionString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("dir", aAny);
            }
            break;
        case NS_ooxml::LN_CT_SphereCoords_lat:
            mpGrabBagStack->addInt32("lat", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_SphereCoords_lon:
            mpGrabBagStack->addInt32("lon", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_SphereCoords_rev:
            mpGrabBagStack->addInt32("rev", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Props3D_extrusionH:
            mpGrabBagStack->addInt32("extrusionH", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Props3D_contourW:
            mpGrabBagStack->addInt32("contourW", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Props3D_prstMaterial:
            {
                uno::Any aAny(getPresetMaterialTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("prstMaterial", aAny);
            }
            break;
        case NS_ooxml::LN_CT_Bevel_w:
            mpGrabBagStack->addInt32("w", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Bevel_h:
            mpGrabBagStack->addInt32("h", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Bevel_prst:
            {
                uno::Any aAny(getBevelPresetTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("prst", aAny);
            }
            break;
        case NS_ooxml::LN_CT_Ligatures_val:
            {
                uno::Any aAny(getLigaturesString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        case NS_ooxml::LN_CT_NumForm_val:
            {
                uno::Any aAny(getNumFormString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        case NS_ooxml::LN_CT_NumSpacing_val:
            {
                uno::Any aAny(getNumSpacingString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        case NS_ooxml::LN_CT_StyleSet_id:
            mpGrabBagStack->addInt32("id", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_StyleSet_val:
        case NS_ooxml::LN_CT_OnOff_val:
            {
                uno::Any aAny(getOnOffString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        default:
            break;
    }
}

void TextEffectsHandler::lcl_sprm(Sprm& rSprm)
{
    if (mpGrabBagStack->getCurrentName() == constAttributesSequenceName)
        mpGrabBagStack->pop();

    sal_uInt32 nSprmId = rSprm.getId();
    OUString aElementName = lclGetNameForElementId(nSprmId);
    if(aElementName.isEmpty())
    {
        // Element is unknown -> leave.
        return;
    }

    mpGrabBagStack->push(aElementName);

    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( !pProperties )
        return;

    pProperties->resolve( *this );

    if (mpGrabBagStack->getCurrentName() == constAttributesSequenceName)
        mpGrabBagStack->pop();

    mpGrabBagStack->pop();
}

beans::PropertyValue TextEffectsHandler::getInteropGrabBag()
{
    beans::PropertyValue aReturn = mpGrabBagStack->getRootProperty();
    mpGrabBagStack.reset();
    return aReturn;
}

sal_uInt8 TextEffectsHandler::GetTextFillSolidFillAlpha(const css::beans::PropertyValue& rValue)
{
    if (rValue.Name != "textFill")
    {
        return 0;
    }

    uno::Sequence<beans::PropertyValue> aPropertyValues;
    rValue.Value >>= aPropertyValues;
    comphelper::SequenceAsHashMap aMap(aPropertyValues);
    auto it = aMap.find("solidFill");
    if (it == aMap.end())
    {
        return 0;
    }

    comphelper::SequenceAsHashMap aSolidFillMap(it->second);
    it = aSolidFillMap.find("srgbClr");
    if (it == aSolidFillMap.end())
    {
        it = aSolidFillMap.find("schemeClr");
        if (it == aSolidFillMap.end())
            return 0;
    }

    comphelper::SequenceAsHashMap aSrgbClrMap(it->second);
    it = aSrgbClrMap.find("alpha");
    if (it == aSrgbClrMap.end())
    {
        return 0;
    }

    comphelper::SequenceAsHashMap aAlphaMap(it->second);
    it = aAlphaMap.find("attributes");
    if (it == aAlphaMap.end())
    {
        return 0;
    }

    comphelper::SequenceAsHashMap aAttributesMap(it->second);
    it = aAttributesMap.find("val");
    if (it == aAttributesMap.end())
    {
        return 0;
    }
    sal_Int32 nVal = 0;
    it->second >>= nVal;
    return nVal / oox::drawingml::PER_PERCENT;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
