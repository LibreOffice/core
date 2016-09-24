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

#include <TextEffectsHandler.hxx>

#include <rtl/ustrbuf.hxx>
#include <comphelper/string.hxx>
#include <ooxml/resourceids.hxx>

namespace writerfilter {
namespace dmapper
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
    }
    return aIdMap[aId];
}

const char constAttributesSequenceName[] = "attributes";

}

OUString TextEffectsHandler::getSchemeColorValTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_SchemeColorVal_bg1: return OUString("bg1");
        case NS_ooxml::LN_ST_SchemeColorVal_tx1: return OUString("tx1");
        case NS_ooxml::LN_ST_SchemeColorVal_bg2: return OUString("bg2");
        case NS_ooxml::LN_ST_SchemeColorVal_tx2: return OUString("tx2");
        case NS_ooxml::LN_ST_SchemeColorVal_accent1: return OUString("accent1");
        case NS_ooxml::LN_ST_SchemeColorVal_accent2: return OUString("accent2");
        case NS_ooxml::LN_ST_SchemeColorVal_accent3: return OUString("accent3");
        case NS_ooxml::LN_ST_SchemeColorVal_accent4: return OUString("accent4");
        case NS_ooxml::LN_ST_SchemeColorVal_accent5: return OUString("accent5");
        case NS_ooxml::LN_ST_SchemeColorVal_accent6: return OUString("accent6");
        case NS_ooxml::LN_ST_SchemeColorVal_hlink: return OUString("hlink");
        case NS_ooxml::LN_ST_SchemeColorVal_folHlink: return OUString("folHlink");
        case NS_ooxml::LN_ST_SchemeColorVal_dk1: return OUString("dk1");
        case NS_ooxml::LN_ST_SchemeColorVal_lt1: return OUString("lt1");
        case NS_ooxml::LN_ST_SchemeColorVal_dk2: return OUString("dk2");
        case NS_ooxml::LN_ST_SchemeColorVal_lt2: return OUString("lt2");
        case NS_ooxml::LN_ST_SchemeColorVal_phClr: return OUString("phClr");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getRectAlignmentString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_RectAlignment_none: return OUString("none");
        case NS_ooxml::LN_ST_RectAlignment_tl: return OUString("tl");
        case NS_ooxml::LN_ST_RectAlignment_t: return OUString("t");
        case NS_ooxml::LN_ST_RectAlignment_tr: return OUString("tr");
        case NS_ooxml::LN_ST_RectAlignment_l: return OUString("l");
        case NS_ooxml::LN_ST_RectAlignment_ctr: return OUString("ctr");
        case NS_ooxml::LN_ST_RectAlignment_r: return OUString("r");
        case NS_ooxml::LN_ST_RectAlignment_bl: return OUString("bl");
        case NS_ooxml::LN_ST_RectAlignment_b: return OUString("b");
        case NS_ooxml::LN_ST_RectAlignment_br: return OUString("br");

        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getLineCapString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_LineCap_rnd: return OUString("rnd");
        case NS_ooxml::LN_ST_LineCap_sq: return OUString("sq");
        case NS_ooxml::LN_ST_LineCap_flat: return OUString("flat");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getCompoundLineString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_CompoundLine_sng: return OUString("sng");
        case NS_ooxml::LN_ST_CompoundLine_dbl: return OUString("dbl");
        case NS_ooxml::LN_ST_CompoundLine_thickThin: return OUString("thickThin");
        case NS_ooxml::LN_ST_CompoundLine_thinThick: return OUString("thinThick");
        case NS_ooxml::LN_ST_CompoundLine_tri: return OUString("tri");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPenAlignmentString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PenAlignment_ctr: return OUString("ctr");
        case NS_ooxml::LN_ST_PenAlignment_in: return OUString("in");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getOnOffString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_OnOff_true: return OUString("true");
        case NS_ooxml::LN_ST_OnOff_false: return OUString("false");
        case NS_ooxml::LN_ST_OnOff_1: return OUString("1");
        case NS_ooxml::LN_ST_OnOff_0: return OUString("0");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPathShadeTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PathShadeType_shape: return OUString("shape");
        case NS_ooxml::LN_ST_PathShadeType_circle: return OUString("circle");
        case NS_ooxml::LN_ST_PathShadeType_rect: return OUString("rect");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPresetLineDashValString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PresetLineDashVal_solid: return OUString("solid");
        case NS_ooxml::LN_ST_PresetLineDashVal_dot: return OUString("dot");
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDot: return OUString("sysDot");
        case NS_ooxml::LN_ST_PresetLineDashVal_dash: return OUString("dash");
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDash: return OUString("sysDash");
        case NS_ooxml::LN_ST_PresetLineDashVal_lgDash: return OUString("lgDash");
        case NS_ooxml::LN_ST_PresetLineDashVal_dashDot: return OUString("dashDot");
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDashDot: return OUString("sysDashDot");
        case NS_ooxml::LN_ST_PresetLineDashVal_lgDashDot: return OUString("lgDashDot");
        case NS_ooxml::LN_ST_PresetLineDashVal_lgDashDotDot: return OUString("lgDashDotDot");
        case NS_ooxml::LN_ST_PresetLineDashVal_sysDashDotDot: return OUString("sysDashDotDot");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPresetCameraTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueTopLeft: return OUString("legacyObliqueTopLeft");
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueTop: return OUString("legacyObliqueTop");
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueTopRight: return OUString("legacyObliqueTopRight");
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueLeft: return OUString("legacyObliqueLeft");
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueFront: return OUString("legacyObliqueFront");
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueRight: return OUString("legacyObliqueRight");
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueBottomLeft: return OUString("legacyObliqueBottomLeft");
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueBottom: return OUString("legacyObliqueBottom");
        case NS_ooxml::LN_ST_PresetCameraType_legacyObliqueBottomRight: return OUString("legacyObliqueBottomRight");
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveTopLeft: return OUString("legacyPerspectiveTopLeft");
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveTop: return OUString("legacyPerspectiveTop");
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveTopRight: return OUString("legacyPerspectiveTopRight");
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveLeft: return OUString("legacyPerspectiveLeft");
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveFront: return OUString("legacyPerspectiveFront");
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveRight: return OUString("legacyPerspectiveRight");
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveBottomLeft: return OUString("legacyPerspectiveBottomLeft");
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveBottom: return OUString("legacyPerspectiveBottom");
        case NS_ooxml::LN_ST_PresetCameraType_legacyPerspectiveBottomRight: return OUString("legacyPerspectiveBottomRight");
        case NS_ooxml::LN_ST_PresetCameraType_orthographicFront: return OUString("orthographicFront");
        case NS_ooxml::LN_ST_PresetCameraType_isometricTopUp: return OUString("isometricTopUp");
        case NS_ooxml::LN_ST_PresetCameraType_isometricTopDown: return OUString("isometricTopDown");
        case NS_ooxml::LN_ST_PresetCameraType_isometricBottomUp: return OUString("isometricBottomUp");
        case NS_ooxml::LN_ST_PresetCameraType_isometricBottomDown: return OUString("isometricBottomDown");
        case NS_ooxml::LN_ST_PresetCameraType_isometricLeftUp: return OUString("isometricLeftUp");
        case NS_ooxml::LN_ST_PresetCameraType_isometricLeftDown: return OUString("isometricLeftDown");
        case NS_ooxml::LN_ST_PresetCameraType_isometricRightUp: return OUString("isometricRightUp");
        case NS_ooxml::LN_ST_PresetCameraType_isometricRightDown: return OUString("isometricRightDown");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis1Left: return OUString("isometricOffAxis1Left");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis1Right: return OUString("isometricOffAxis1Right");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis1Top: return OUString("isometricOffAxis1Top");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis2Left: return OUString("isometricOffAxis2Left");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis2Right: return OUString("isometricOffAxis2Right");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis2Top: return OUString("isometricOffAxis2Top");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis3Left: return OUString("isometricOffAxis3Left");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis3Right: return OUString("isometricOffAxis3Right");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis3Bottom: return OUString("isometricOffAxis3Bottom");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis4Left: return OUString("isometricOffAxis4Left");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis4Right: return OUString("isometricOffAxis4Right");
        case NS_ooxml::LN_ST_PresetCameraType_isometricOffAxis4Bottom: return OUString("isometricOffAxis4Bottom");
        case NS_ooxml::LN_ST_PresetCameraType_obliqueTopLeft: return OUString("obliqueTopLeft");
        case NS_ooxml::LN_ST_PresetCameraType_obliqueTop: return OUString("obliqueTop");
        case NS_ooxml::LN_ST_PresetCameraType_obliqueTopRight: return OUString("obliqueTopRight");
        case NS_ooxml::LN_ST_PresetCameraType_obliqueLeft: return OUString("obliqueLeft");
        case NS_ooxml::LN_ST_PresetCameraType_obliqueRight: return OUString("obliqueRight");
        case NS_ooxml::LN_ST_PresetCameraType_obliqueBottomLeft: return OUString("obliqueBottomLeft");
        case NS_ooxml::LN_ST_PresetCameraType_obliqueBottom: return OUString("obliqueBottom");
        case NS_ooxml::LN_ST_PresetCameraType_obliqueBottomRight: return OUString("obliqueBottomRight");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveFront: return OUString("perspectiveFront");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveLeft: return OUString("perspectiveLeft");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveRight: return OUString("perspectiveRight");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveAbove: return OUString("perspectiveAbove");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveBelow: return OUString("perspectiveBelow");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveAboveLeftFacing: return OUString("perspectiveAboveLeftFacing");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveAboveRightFacing: return OUString("perspectiveAboveRightFacing");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveContrastingLeftFacing: return OUString("perspectiveContrastingLeftFacing");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveContrastingRightFacing: return OUString("perspectiveContrastingRightFacing");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicLeftFacing: return OUString("perspectiveHeroicLeftFacing");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicRightFacing: return OUString("perspectiveHeroicRightFacing");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicExtremeLeftFacing: return OUString("perspectiveHeroicExtremeLeftFacing");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveHeroicExtremeRightFacing: return OUString("perspectiveHeroicExtremeRightFacing");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveRelaxed: return OUString("perspectiveRelaxed");
        case NS_ooxml::LN_ST_PresetCameraType_perspectiveRelaxedModerately: return OUString("perspectiveRelaxedModerately");
        default: break;
    }
    return OUString();
}


OUString TextEffectsHandler::getLightRigTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_LightRigType_legacyFlat1: return OUString("legacyFlat1");
        case NS_ooxml::LN_ST_LightRigType_legacyFlat2: return OUString("legacyFlat2");
        case NS_ooxml::LN_ST_LightRigType_legacyFlat3: return OUString("legacyFlat3");
        case NS_ooxml::LN_ST_LightRigType_legacyFlat4: return OUString("legacyFlat4");
        case NS_ooxml::LN_ST_LightRigType_legacyNormal1: return OUString("legacyNormal1");
        case NS_ooxml::LN_ST_LightRigType_legacyNormal2: return OUString("legacyNormal2");
        case NS_ooxml::LN_ST_LightRigType_legacyNormal3: return OUString("legacyNormal3");
        case NS_ooxml::LN_ST_LightRigType_legacyNormal4: return OUString("legacyNormal4");
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh1: return OUString("legacyHarsh1");
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh2: return OUString("legacyHarsh2");
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh3: return OUString("legacyHarsh3");
        case NS_ooxml::LN_ST_LightRigType_legacyHarsh4: return OUString("legacyHarsh4");
        case NS_ooxml::LN_ST_LightRigType_threePt: return OUString("threePt");
        case NS_ooxml::LN_ST_LightRigType_balanced: return OUString("balanced");
        case NS_ooxml::LN_ST_LightRigType_soft: return OUString("soft");
        case NS_ooxml::LN_ST_LightRigType_harsh: return OUString("harsh");
        case NS_ooxml::LN_ST_LightRigType_flood: return OUString("flood");
        case NS_ooxml::LN_ST_LightRigType_contrasting: return OUString("contrasting");
        case NS_ooxml::LN_ST_LightRigType_morning: return OUString("morning");
        case NS_ooxml::LN_ST_LightRigType_sunrise: return OUString("sunrise");
        case NS_ooxml::LN_ST_LightRigType_sunset: return OUString("sunset");
        case NS_ooxml::LN_ST_LightRigType_chilly: return OUString("chilly");
        case NS_ooxml::LN_ST_LightRigType_freezing: return OUString("freezing");
        case NS_ooxml::LN_ST_LightRigType_flat: return OUString("flat");
        case NS_ooxml::LN_ST_LightRigType_twoPt: return OUString("twoPt");
        case NS_ooxml::LN_ST_LightRigType_glow: return OUString("glow");
        case NS_ooxml::LN_ST_LightRigType_brightRoom: return OUString("brightRoom");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getLightRigDirectionString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_LightRigDirection_tl: return OUString("tl");
        case NS_ooxml::LN_ST_LightRigDirection_t: return OUString("t");
        case NS_ooxml::LN_ST_LightRigDirection_tr: return OUString("tr");
        case NS_ooxml::LN_ST_LightRigDirection_l: return OUString("l");
        case NS_ooxml::LN_ST_LightRigDirection_r: return OUString("r");
        case NS_ooxml::LN_ST_LightRigDirection_bl: return OUString("bl");
        case NS_ooxml::LN_ST_LightRigDirection_b: return OUString("b");
        case NS_ooxml::LN_ST_LightRigDirection_br: return OUString("br");

        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getBevelPresetTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_BevelPresetType_relaxedInset: return OUString("relaxedInset");
        case NS_ooxml::LN_ST_BevelPresetType_circle: return OUString("circle");
        case NS_ooxml::LN_ST_BevelPresetType_slope: return OUString("slope");
        case NS_ooxml::LN_ST_BevelPresetType_cross: return OUString("cross");
        case NS_ooxml::LN_ST_BevelPresetType_angle: return OUString("angle");
        case NS_ooxml::LN_ST_BevelPresetType_softRound: return OUString("softRound");
        case NS_ooxml::LN_ST_BevelPresetType_convex: return OUString("convex");
        case NS_ooxml::LN_ST_BevelPresetType_coolSlant: return OUString("coolSlant");
        case NS_ooxml::LN_ST_BevelPresetType_divot: return OUString("divot");
        case NS_ooxml::LN_ST_BevelPresetType_riblet: return OUString("riblet");
        case NS_ooxml::LN_ST_BevelPresetType_hardEdge: return OUString("hardEdge");
        case NS_ooxml::LN_ST_BevelPresetType_artDeco: return OUString("artDeco");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getPresetMaterialTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_PresetMaterialType_legacyMatte: return OUString("legacyMatte");
        case NS_ooxml::LN_ST_PresetMaterialType_legacyPlastic: return OUString("legacyPlastic");
        case NS_ooxml::LN_ST_PresetMaterialType_legacyMetal: return OUString("legacyMetal");
        case NS_ooxml::LN_ST_PresetMaterialType_legacyWireframe: return OUString("legacyWireframe");
        case NS_ooxml::LN_ST_PresetMaterialType_matte: return OUString("matte");
        case NS_ooxml::LN_ST_PresetMaterialType_plastic: return OUString("plastic");
        case NS_ooxml::LN_ST_PresetMaterialType_metal: return OUString("metal");
        case NS_ooxml::LN_ST_PresetMaterialType_warmMatte: return OUString("warmMatte");
        case NS_ooxml::LN_ST_PresetMaterialType_translucentPowder: return OUString("translucentPowder");
        case NS_ooxml::LN_ST_PresetMaterialType_powder: return OUString("powder");
        case NS_ooxml::LN_ST_PresetMaterialType_dkEdge: return OUString("dkEdge");
        case NS_ooxml::LN_ST_PresetMaterialType_softEdge: return OUString("softEdge");
        case NS_ooxml::LN_ST_PresetMaterialType_clear: return OUString("clear");
        case NS_ooxml::LN_ST_PresetMaterialType_flat: return OUString("flat");
        case NS_ooxml::LN_ST_PresetMaterialType_softmetal: return OUString("softmetal");
        case NS_ooxml::LN_ST_PresetMaterialType_none: return OUString("none");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getLigaturesString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_Ligatures_none: return OUString("none");
        case NS_ooxml::LN_ST_Ligatures_standard: return OUString("standard");
        case NS_ooxml::LN_ST_Ligatures_contextual: return OUString("contextual");
        case NS_ooxml::LN_ST_Ligatures_historical: return OUString("historical");
        case NS_ooxml::LN_ST_Ligatures_discretional: return OUString("discretional");
        case NS_ooxml::LN_ST_Ligatures_standardContextual: return OUString("standardContextual");
        case NS_ooxml::LN_ST_Ligatures_standardHistorical: return OUString("standardHistorical");
        case NS_ooxml::LN_ST_Ligatures_contextualHistorical: return OUString("contextualHistorical");
        case NS_ooxml::LN_ST_Ligatures_standardDiscretional: return OUString("standardDiscretional");
        case NS_ooxml::LN_ST_Ligatures_contextualDiscretional: return OUString("contextualDiscretional");
        case NS_ooxml::LN_ST_Ligatures_historicalDiscretional: return OUString("historicalDiscretional");
        case NS_ooxml::LN_ST_Ligatures_standardContextualHistorical: return OUString("standardContextualHistorical");
        case NS_ooxml::LN_ST_Ligatures_standardContextualDiscretional: return OUString("standardContextualDiscretional");
        case NS_ooxml::LN_ST_Ligatures_standardHistoricalDiscretional: return OUString("standardHistoricalDiscretional");
        case NS_ooxml::LN_ST_Ligatures_contextualHistoricalDiscretional: return OUString("contextualHistoricalDiscretional");
        case NS_ooxml::LN_ST_Ligatures_all: return OUString("all");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getNumFormString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_NumForm_default: return OUString("default");
        case NS_ooxml::LN_ST_NumForm_lining: return OUString("lining");
        case NS_ooxml::LN_ST_NumForm_oldStyle: return OUString("oldStyle");
        default: break;
    }
    return OUString();
}

OUString TextEffectsHandler::getNumSpacingString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_ST_NumSpacing_default: return OUString("default");
        case NS_ooxml::LN_ST_NumSpacing_proportional: return OUString("proportional");
        case NS_ooxml::LN_ST_NumSpacing_tabular: return OUString("tabular");
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
    LoggedProperties("TextEffectsHandler"),
    mpGrabBagStack(nullptr)
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
                OUStringBuffer aBuffer = OUString::number(aValue.getInt(), 16);
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
                uno::Any aAny = uno::makeAny(getRectAlignmentString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("algn", aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_w:
            mpGrabBagStack->addInt32("w", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_cap:
            {
                uno::Any aAny = uno::makeAny(getLineCapString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("cap", aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_cmpd:
            {
                uno::Any aAny = uno::makeAny(getCompoundLineString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("cmpd", aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_algn:
            {
                uno::Any aAny = uno::makeAny(getPenAlignmentString(sal_Int32(aValue.getInt())));
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
                uno::Any aAny = uno::makeAny(getOnOffString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("scaled", aAny);
            }
            break;
        case NS_ooxml::LN_CT_PathShadeProperties_path:
            {
                uno::Any aAny = uno::makeAny(getPathShadeTypeString(sal_Int32(aValue.getInt())));
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
                uno::Any aAny = uno::makeAny(getPresetLineDashValString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        case NS_ooxml::LN_CT_LineJoinMiterProperties_lim:
            mpGrabBagStack->addInt32("lim", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_Camera_prst:
            {
                uno::Any aAny = uno::makeAny(getPresetCameraTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("prst", aAny);
            }
            break;
        case NS_ooxml::LN_CT_LightRig_rig:
            {
                uno::Any aAny = uno::makeAny(getLightRigTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("rig", aAny);
            }
            break;
        case NS_ooxml::LN_CT_LightRig_dir:
            {
                uno::Any aAny = uno::makeAny(getLightRigDirectionString(sal_Int32(aValue.getInt())));
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
                uno::Any aAny = uno::makeAny(getPresetMaterialTypeString(sal_Int32(aValue.getInt())));
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
                uno::Any aAny = uno::makeAny(getBevelPresetTypeString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("prst", aAny);
            }
            break;
        case NS_ooxml::LN_CT_Ligatures_val:
            {
                uno::Any aAny = uno::makeAny(getLigaturesString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        case NS_ooxml::LN_CT_NumForm_val:
            {
                uno::Any aAny = uno::makeAny(getNumFormString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        case NS_ooxml::LN_CT_NumSpacing_val:
            {
                uno::Any aAny = uno::makeAny(getNumSpacingString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        case NS_ooxml::LN_CT_StyleSet_id:
            mpGrabBagStack->addInt32("id", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_StyleSet_val:
        case NS_ooxml::LN_CT_OnOff_val:
            {
                uno::Any aAny = uno::makeAny(getOnOffString(sal_Int32(aValue.getInt())));
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
    if( !pProperties.get())
        return;

    pProperties.get()->resolve( *this );

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

}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
