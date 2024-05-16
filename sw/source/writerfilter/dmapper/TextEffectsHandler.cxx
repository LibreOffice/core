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
        case NS_ooxml::LN_ST_SchemeColorVal_bg1: return u"bg1"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_tx1: return u"tx1"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_bg2: return u"bg2"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_tx2: return u"tx2"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_accent1: return u"accent1"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_accent2: return u"accent2"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_accent3: return u"accent3"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_accent4: return u"accent4"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_accent5: return u"accent5"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_accent6: return u"accent6"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_hlink: return u"hlink"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_folHlink: return u"folHlink"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_dk1: return u"dk1"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_lt1: return u"lt1"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_dk2: return u"dk2"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_lt2: return u"lt2"_ustr;
        case NS_ooxml::LN_ST_SchemeColorVal_phClr: return u"phClr"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getRectAlignmentString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_RectAlignment_none: return u"none"_ustr;
        case NS_ooxml::LN_ST_RectAlignment_tl: return u"tl"_ustr;
        case NS_ooxml::LN_ST_RectAlignment_t: return u"t"_ustr;
        case NS_ooxml::LN_ST_RectAlignment_tr: return u"tr"_ustr;
        case NS_ooxml::LN_ST_RectAlignment_l: return u"l"_ustr;
        case NS_ooxml::LN_ST_RectAlignment_ctr: return u"ctr"_ustr;
        case NS_ooxml::LN_ST_RectAlignment_r: return u"r"_ustr;
        case NS_ooxml::LN_ST_RectAlignment_bl: return u"bl"_ustr;
        case NS_ooxml::LN_ST_RectAlignment_b: return u"b"_ustr;
        case NS_ooxml::LN_ST_RectAlignment_br: return u"br"_ustr;

        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getLineCapString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_LineCap_rnd: return u"rnd"_ustr;
        case NS_ooxml::LN_ST_LineCap_sq: return u"sq"_ustr;
        case NS_ooxml::LN_ST_LineCap_flat: return u"flat"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getCompoundLineString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_CompoundLine_sng: return u"sng"_ustr;
        case NS_ooxml::LN_ST_CompoundLine_dbl: return u"dbl"_ustr;
        case NS_ooxml::LN_ST_CompoundLine_thickThin: return u"thickThin"_ustr;
        case NS_ooxml::LN_ST_CompoundLine_thinThick: return u"thinThick"_ustr;
        case NS_ooxml::LN_ST_CompoundLine_tri: return u"tri"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPenAlignmentString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PenAlignment_ctr: return u"ctr"_ustr;
        case NS_ooxml::LN_ST_PenAlignment_in: return u"in"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getOnOffString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_OnOff_true: return u"true"_ustr;
        case NS_ooxml::LN_ST_OnOff_false: return u"false"_ustr;
        case NS_ooxml::LN_ST_OnOff_1: return u"1"_ustr;
        case NS_ooxml::LN_ST_OnOff_0: return u"0"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPathShadeTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PathShadeType_shape: return u"shape"_ustr;
        case NS_ooxml::LN_ST_PathShadeType_circle: return u"circle"_ustr;
        case NS_ooxml::LN_ST_PathShadeType_rect: return u"rect"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPresetLineDashValString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PresetLineDashVal_solid: return u"solid"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_dot: return u"dot"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDot: return u"sysDot"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_dash: return u"dash"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDash: return u"sysDash"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_lgDash: return u"lgDash"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_dashDot: return u"dashDot"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDashDot: return u"sysDashDot"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_lgDashDot: return u"lgDashDot"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_lgDashDotDot: return u"lgDashDotDot"_ustr;
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDashDotDot: return u"sysDashDotDot"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPresetCameraTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueTopLeft: return u"legacyObliqueTopLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueTop: return u"legacyObliqueTop"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueTopRight: return u"legacyObliqueTopRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueLeft: return u"legacyObliqueLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueFront: return u"legacyObliqueFront"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueRight: return u"legacyObliqueRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueBottomLeft: return u"legacyObliqueBottomLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueBottom: return u"legacyObliqueBottom"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueBottomRight: return u"legacyObliqueBottomRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveTopLeft: return u"legacyPerspectiveTopLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveTop: return u"legacyPerspectiveTop"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveTopRight: return u"legacyPerspectiveTopRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveLeft: return u"legacyPerspectiveLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveFront: return u"legacyPerspectiveFront"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveRight: return u"legacyPerspectiveRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveBottomLeft: return u"legacyPerspectiveBottomLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveBottom: return u"legacyPerspectiveBottom"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveBottomRight: return u"legacyPerspectiveBottomRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_orthographicFront: return u"orthographicFront"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricTopUp: return u"isometricTopUp"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricTopDown: return u"isometricTopDown"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricBottomUp: return u"isometricBottomUp"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricBottomDown: return u"isometricBottomDown"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricLeftUp: return u"isometricLeftUp"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricLeftDown: return u"isometricLeftDown"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricRightUp: return u"isometricRightUp"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricRightDown: return u"isometricRightDown"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis1Left: return u"isometricOffAxis1Left"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis1Right: return u"isometricOffAxis1Right"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis1Top: return u"isometricOffAxis1Top"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis2Left: return u"isometricOffAxis2Left"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis2Right: return u"isometricOffAxis2Right"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis2Top: return u"isometricOffAxis2Top"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis3Left: return u"isometricOffAxis3Left"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis3Right: return u"isometricOffAxis3Right"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis3Bottom: return u"isometricOffAxis3Bottom"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis4Left: return u"isometricOffAxis4Left"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis4Right: return u"isometricOffAxis4Right"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis4Bottom: return u"isometricOffAxis4Bottom"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_obliqueTopLeft: return u"obliqueTopLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_obliqueTop: return u"obliqueTop"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_obliqueTopRight: return u"obliqueTopRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_obliqueLeft: return u"obliqueLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_obliqueRight: return u"obliqueRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_obliqueBottomLeft: return u"obliqueBottomLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_obliqueBottom: return u"obliqueBottom"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_obliqueBottomRight: return u"obliqueBottomRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveFront: return u"perspectiveFront"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveLeft: return u"perspectiveLeft"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveRight: return u"perspectiveRight"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveAbove: return u"perspectiveAbove"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveBelow: return u"perspectiveBelow"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveAboveLeftFacing: return u"perspectiveAboveLeftFacing"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveAboveRightFacing: return u"perspectiveAboveRightFacing"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveContrastingLeftFacing: return u"perspectiveContrastingLeftFacing"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveContrastingRightFacing: return u"perspectiveContrastingRightFacing"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicLeftFacing: return u"perspectiveHeroicLeftFacing"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicRightFacing: return u"perspectiveHeroicRightFacing"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicExtremeLeftFacing: return u"perspectiveHeroicExtremeLeftFacing"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicExtremeRightFacing: return u"perspectiveHeroicExtremeRightFacing"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveRelaxed: return u"perspectiveRelaxed"_ustr;
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveRelaxedModerately: return u"perspectiveRelaxedModerately"_ustr;
        default: break;
    }
    return OUString();
}


OUString TextEffectsHandler::getLightRigTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_LightRigType_legacyFlat1: return u"legacyFlat1"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyFlat2: return u"legacyFlat2"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyFlat3: return u"legacyFlat3"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyFlat4: return u"legacyFlat4"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyNormal1: return u"legacyNormal1"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyNormal2: return u"legacyNormal2"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyNormal3: return u"legacyNormal3"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyNormal4: return u"legacyNormal4"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh1: return u"legacyHarsh1"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh2: return u"legacyHarsh2"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh3: return u"legacyHarsh3"_ustr;
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh4: return u"legacyHarsh4"_ustr;
        case NS_ooxml::LN_ST_LightRigType_threePt: return u"threePt"_ustr;
        case NS_ooxml::LN_ST_LightRigType_balanced: return u"balanced"_ustr;
        case NS_ooxml::LN_ST_LightRigType_soft: return u"soft"_ustr;
        case NS_ooxml::LN_ST_LightRigType_harsh: return u"harsh"_ustr;
        case NS_ooxml::LN_ST_LightRigType_flood: return u"flood"_ustr;
        case NS_ooxml::LN_ST_LightRigType_contrasting: return u"contrasting"_ustr;
        case NS_ooxml::LN_ST_LightRigType_morning: return u"morning"_ustr;
        case NS_ooxml::LN_ST_LightRigType_sunrise: return u"sunrise"_ustr;
        case NS_ooxml::LN_ST_LightRigType_sunset: return u"sunset"_ustr;
        case NS_ooxml::LN_ST_LightRigType_chilly: return u"chilly"_ustr;
        case NS_ooxml::LN_ST_LightRigType_freezing: return u"freezing"_ustr;
        case NS_ooxml::LN_ST_LightRigType_flat: return u"flat"_ustr;
        case NS_ooxml::LN_ST_LightRigType_twoPt: return u"twoPt"_ustr;
        case NS_ooxml::LN_ST_LightRigType_glow: return u"glow"_ustr;
        case NS_ooxml::LN_ST_LightRigType_brightRoom: return u"brightRoom"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getLightRigDirectionString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_LightRigDirection_tl: return u"tl"_ustr;
        case NS_ooxml::LN_ST_LightRigDirection_t: return u"t"_ustr;
        case NS_ooxml::LN_ST_LightRigDirection_tr: return u"tr"_ustr;
        case NS_ooxml::LN_ST_LightRigDirection_l: return u"l"_ustr;
        case NS_ooxml::LN_ST_LightRigDirection_r: return u"r"_ustr;
        case NS_ooxml::LN_ST_LightRigDirection_bl: return u"bl"_ustr;
        case NS_ooxml::LN_ST_LightRigDirection_b: return u"b"_ustr;
        case NS_ooxml::LN_ST_LightRigDirection_br: return u"br"_ustr;

        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getBevelPresetTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_BevelPresetType_relaxedInset: return u"relaxedInset"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_circle: return u"circle"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_slope: return u"slope"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_cross: return u"cross"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_angle: return u"angle"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_softRound: return u"softRound"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_convex: return u"convex"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_coolSlant: return u"coolSlant"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_divot: return u"divot"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_riblet: return u"riblet"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_hardEdge: return u"hardEdge"_ustr;
        case NS_ooxml::LN_ST_BevelPresetType_artDeco: return u"artDeco"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPresetMaterialTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PresetMaterialType_legacyMatte: return u"legacyMatte"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_legacyPlastic: return u"legacyPlastic"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_legacyMetal: return u"legacyMetal"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_legacyWireframe: return u"legacyWireframe"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_matte: return u"matte"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_plastic: return u"plastic"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_metal: return u"metal"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_warmMatte: return u"warmMatte"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_translucentPowder: return u"translucentPowder"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_powder: return u"powder"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_dkEdge: return u"dkEdge"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_softEdge: return u"softEdge"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_clear: return u"clear"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_flat: return u"flat"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_softmetal: return u"softmetal"_ustr;
        case NS_ooxml::LN_ST_PresetMaterialType_none: return u"none"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getLigaturesString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_Ligatures_none: return u"none"_ustr;
        case NS_ooxml::LN_ST_Ligatures_standard: return u"standard"_ustr;
        case NS_ooxml::LN_ST_Ligatures_contextual: return u"contextual"_ustr;
        case NS_ooxml::LN_ST_Ligatures_historical: return u"historical"_ustr;
        case NS_ooxml::LN_ST_Ligatures_discretional: return u"discretional"_ustr;
        case NS_ooxml::LN_ST_Ligatures_standardContextual: return u"standardContextual"_ustr;
        case NS_ooxml::LN_ST_Ligatures_standardHistorical: return u"standardHistorical"_ustr;
        case NS_ooxml::LN_ST_Ligatures_contextualHistorical: return u"contextualHistorical"_ustr;
        case NS_ooxml::LN_ST_Ligatures_standardDiscretional: return u"standardDiscretional"_ustr;
        case NS_ooxml::LN_ST_Ligatures_contextualDiscretional: return u"contextualDiscretional"_ustr;
        case NS_ooxml::LN_ST_Ligatures_historicalDiscretional: return u"historicalDiscretional"_ustr;
        case NS_ooxml::LN_ST_Ligatures_standardContextualHistorical: return u"standardContextualHistorical"_ustr;
        case NS_ooxml::LN_ST_Ligatures_standardContextualDiscretional: return u"standardContextualDiscretional"_ustr;
        case NS_ooxml::LN_ST_Ligatures_standardHistoricalDiscretional: return u"standardHistoricalDiscretional"_ustr;
        case NS_ooxml::LN_ST_Ligatures_contextualHistoricalDiscretional: return u"contextualHistoricalDiscretional"_ustr;
        case NS_ooxml::LN_ST_Ligatures_all: return u"all"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getNumFormString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_NumForm_default: return u"default"_ustr;
        case NS_ooxml::LN_ST_NumForm_lining: return u"lining"_ustr;
        case NS_ooxml::LN_ST_NumForm_oldStyle: return u"oldStyle"_ustr;
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getNumSpacingString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_NumSpacing_default: return u"default"_ustr;
        case NS_ooxml::LN_ST_NumSpacing_proportional: return u"proportional"_ustr;
        case NS_ooxml::LN_ST_NumSpacing_tabular: return u"tabular"_ustr;
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
            mpGrabBagStack->addInt32(u"val"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Glow_rad:
            mpGrabBagStack->addInt32(u"rad"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_SchemeColor_val:
            {
                OUString aString = getSchemeColorValTypeString(sal_Int32(aValue.getInt()));
                mpGrabBagStack->addString(u"val"_ustr, aString);
            }
            break;
        case NS_ooxml::LN_CT_SRgbColor_val:
            {
                OUString aBuffer = OUString::number(aValue.getInt(), 16);
                OUStringBuffer aString;
                comphelper::string::padToLength(aString, 6 - aBuffer.getLength(), '0');
                aString.append(aBuffer.getStr());
                mpGrabBagStack->addString(u"val"_ustr, aString.makeStringAndClear().toAsciiUpperCase());
            }
            break;
        case NS_ooxml::LN_CT_Shadow_blurRad:
        case NS_ooxml::LN_CT_Reflection_blurRad:
            mpGrabBagStack->addInt32(u"blurRad"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_stA:
            mpGrabBagStack->addInt32(u"stA"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_stPos:
            mpGrabBagStack->addInt32(u"stPos"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_endA:
            mpGrabBagStack->addInt32(u"endA"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_endPos:
            mpGrabBagStack->addInt32(u"endPos"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_dist:
        case NS_ooxml::LN_CT_Reflection_dist:
            mpGrabBagStack->addInt32(u"dist"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_dir:
        case NS_ooxml::LN_CT_Reflection_dir:
            mpGrabBagStack->addInt32(u"dir"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Reflection_fadeDir:
            mpGrabBagStack->addInt32(u"fadeDir"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_sx:
        case NS_ooxml::LN_CT_Reflection_sx:
            mpGrabBagStack->addInt32(u"sx"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_sy:
        case NS_ooxml::LN_CT_Reflection_sy:
            mpGrabBagStack->addInt32(u"sy"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_kx:
        case NS_ooxml::LN_CT_Reflection_kx:
            mpGrabBagStack->addInt32(u"kx"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_ky:
        case NS_ooxml::LN_CT_Reflection_ky:
            mpGrabBagStack->addInt32(u"ky"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Shadow_algn:
        case NS_ooxml::LN_CT_Reflection_algn:
            {
                uno::Any aAny(getRectAlignmentString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"algn"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_w:
            mpGrabBagStack->addInt32(u"w"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_cap:
            {
                uno::Any aAny(getLineCapString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"cap"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_cmpd:
            {
                uno::Any aAny(getCompoundLineString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"cmpd"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_algn:
            {
                uno::Any aAny(getPenAlignmentString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"algn"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_GradientStop_pos:
            mpGrabBagStack->addInt32(u"pos"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_LinearShadeProperties_ang:
            mpGrabBagStack->addInt32(u"ang"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_LinearShadeProperties_scaled:
            {
                uno::Any aAny(getOnOffString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"scaled"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_PathShadeProperties_path:
            {
                uno::Any aAny(getPathShadeTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"path"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_RelativeRect_l:
            mpGrabBagStack->addInt32(u"l"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_RelativeRect_t:
            mpGrabBagStack->addInt32(u"t"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_RelativeRect_r:
            mpGrabBagStack->addInt32(u"r"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_RelativeRect_b:
            mpGrabBagStack->addInt32(u"b"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_PresetLineDashProperties_val:
            {
                uno::Any aAny(getPresetLineDashValString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"val"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_LineJoinMiterProperties_lim:
            mpGrabBagStack->addInt32(u"lim"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Camera_prst:
            {
                uno::Any aAny(getPresetCameraTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"prst"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_LightRig_rig:
            {
                uno::Any aAny(getLightRigTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"rig"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_LightRig_dir:
            {
                uno::Any aAny(getLightRigDirectionString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"dir"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_SphereCoords_lat:
            mpGrabBagStack->addInt32(u"lat"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_SphereCoords_lon:
            mpGrabBagStack->addInt32(u"lon"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_SphereCoords_rev:
            mpGrabBagStack->addInt32(u"rev"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Props3D_extrusionH:
            mpGrabBagStack->addInt32(u"extrusionH"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Props3D_contourW:
            mpGrabBagStack->addInt32(u"contourW"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Props3D_prstMaterial:
            {
                uno::Any aAny(getPresetMaterialTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"prstMaterial"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_Bevel_w:
            mpGrabBagStack->addInt32(u"w"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Bevel_h:
            mpGrabBagStack->addInt32(u"h"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Bevel_prst:
            {
                uno::Any aAny(getBevelPresetTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"prst"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_Ligatures_val:
            {
                uno::Any aAny(getLigaturesString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"val"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_NumForm_val:
            {
                uno::Any aAny(getNumFormString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"val"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_NumSpacing_val:
            {
                uno::Any aAny(getNumSpacingString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"val"_ustr, aAny);
            }
            break;
        case NS_ooxml::LN_CT_StyleSet_id:
            mpGrabBagStack->addInt32(u"id"_ustr, sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_StyleSet_val:
        case NS_ooxml::LN_CT_OnOff_val:
            {
                uno::Any aAny(getOnOffString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement(u"val"_ustr, aAny);
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
    auto it = aMap.find(u"solidFill"_ustr);
    if (it == aMap.end())
    {
        return 0;
    }

    comphelper::SequenceAsHashMap aSolidFillMap(it->second);
    it = aSolidFillMap.find(u"srgbClr"_ustr);
    if (it == aSolidFillMap.end())
    {
        it = aSolidFillMap.find(u"schemeClr"_ustr);
        if (it == aSolidFillMap.end())
            return 0;
    }

    comphelper::SequenceAsHashMap aSrgbClrMap(it->second);
    it = aSrgbClrMap.find(u"alpha"_ustr);
    if (it == aSrgbClrMap.end())
    {
        return 0;
    }

    comphelper::SequenceAsHashMap aAlphaMap(it->second);
    it = aAlphaMap.find(u"attributes"_ustr);
    if (it == aAlphaMap.end())
    {
        return 0;
    }

    comphelper::SequenceAsHashMap aAttributesMap(it->second);
    it = aAttributesMap.find(u"val"_ustr);
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
