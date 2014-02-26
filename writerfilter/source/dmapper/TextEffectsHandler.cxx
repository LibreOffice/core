/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <TextEffectsHandler.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/string.hxx>
#include <ooxml/resourceids.hxx>
#include "dmapperLoggers.hxx"
#include <stack>

namespace writerfilter {
namespace dmapper
{

using namespace std;
using namespace css::uno;
using namespace css::beans;

struct GrabBagStackElement
{
    OUString maName;
    std::vector<beans::PropertyValue> maPropertyList;
};

/// Tool that is useful for construction of a nested Sequence/PropertyValue hierarchy
class GrabBagStack
{
public:
    GrabBagStack(OUString aName)
    {
        mCurrentElement.maName = aName;
    }

    virtual ~GrabBagStack()
    {}

    std::stack<GrabBagStackElement> mStack;
    GrabBagStackElement mCurrentElement;

    OUString getCurrentName()
    {
        return mCurrentElement.maName;
    }

    PropertyValue getRootProperty()
    {
        while(!mStack.empty())
            pop();

        PropertyValue aProperty;
        aProperty.Name = mCurrentElement.maName;

        Sequence<PropertyValue> aSequence(mCurrentElement.maPropertyList.size());
        PropertyValue* pSequence = aSequence.getArray();
        std::vector<PropertyValue>::iterator i;
        for (i = mCurrentElement.maPropertyList.begin(); i != mCurrentElement.maPropertyList.end(); ++i)
            *pSequence++ = *i;

        aProperty.Value = makeAny(aSequence);

        return aProperty;
    }

    void appendElement(OUString aName, Any aAny)
    {
        PropertyValue aValue;
        aValue.Name = aName;
        aValue.Value = aAny;
        mCurrentElement.maPropertyList.push_back(aValue);
    }

    void push(OUString aKey)
    {
        mStack.push(mCurrentElement);
        mCurrentElement.maName = aKey;
        mCurrentElement.maPropertyList.clear();
    }

    void pop()
    {
        OUString aName = mCurrentElement.maName;
        Sequence<PropertyValue> aSequence(mCurrentElement.maPropertyList.size());
        PropertyValue* pSequence = aSequence.getArray();
        std::vector<PropertyValue>::iterator i;
        for (i = mCurrentElement.maPropertyList.begin(); i != mCurrentElement.maPropertyList.end(); ++i)
            *pSequence++ = *i;

        mCurrentElement = mStack.top();
        mStack.pop();
        appendElement(aName, makeAny(aSequence));
    }

    void addInt32(OUString aElementName, sal_Int32 aIntValue)
    {
        appendElement(aElementName, makeAny(aIntValue));
    }

    void addString(OUString aElementName, OUString aStringValue)
    {
        appendElement(aElementName, makeAny(aStringValue));
    }
};

namespace
{

OUString getNameForElementId(sal_uInt32 aId)
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
    }
    return aIdMap[aId];
}

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
        case NS_ooxml::LN_props3d_props3d:
        case NS_ooxml::LN_ligatures_ligatures:
        case NS_ooxml::LN_numForm_numForm:
        case NS_ooxml::LN_numSpacing_numSpacing:
        case NS_ooxml::LN_stylisticSets_stylisticSets:
        case NS_ooxml::LN_cntxtAlts_cntxtAlts:
        default:
            break;
    }
}

TextEffectsHandler::TextEffectsHandler(sal_uInt32 aElementId) :
    LoggedProperties(dmapper_logger, "TextEffectsHandler"),
    mpGrabBagStack(NULL)
{
    convertElementIdToPropertyId(aElementId);
    mpGrabBagStack.reset(new GrabBagStack(maElementName));
}

TextEffectsHandler::~TextEffectsHandler()
{
}

boost::optional<PropertyIds> TextEffectsHandler::getGrabBagPropertyId()
{
    return maPropertyId;
}

void TextEffectsHandler::lcl_attribute(Id aName, Value& aValue)
{
    if (mpGrabBagStack->getCurrentName() != "attributes")
        mpGrabBagStack->push("attributes");

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
                uno::Any aAny = makeAny(getRectAlignmentString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("algn", aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_w:
            mpGrabBagStack->addInt32("w", sal_Int32(aValue.getInt()));
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_cap:
            {
                uno::Any aAny = makeAny(getLineCapString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("cap", aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_cmpd:
            {
                uno::Any aAny = makeAny(getCompoundLineString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("cmpd", aAny);
            }
            break;
        case NS_ooxml::LN_CT_TextOutlineEffect_algn:
            {
                uno::Any aAny = makeAny(getPenAlignmentString(sal_Int32(aValue.getInt())));
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
                uno::Any aAny = makeAny(getOnOffString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("scaled", aAny);
            }
            break;
        case NS_ooxml::LN_CT_PathShadeProperties_path:
            {
                uno::Any aAny = makeAny(getPathShadeTypeString(sal_Int32(aValue.getInt())));
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
                uno::Any aAny = makeAny(getPresetLineDashValString(sal_Int32(aValue.getInt())));
                mpGrabBagStack->appendElement("val", aAny);
            }
            break;
        case NS_ooxml::LN_CT_LineJoinMiterProperties_lim:
            mpGrabBagStack->addInt32("lim", sal_Int32(aValue.getInt()));
            break;
        default:
            break;
    }
}

void TextEffectsHandler::lcl_sprm(Sprm& rSprm)
{
    if (mpGrabBagStack->getCurrentName() == "attributes")
        mpGrabBagStack->pop();

    sal_uInt32 nSprmId = rSprm.getId();
    OUString aElementName = getNameForElementId(nSprmId);
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

    if (mpGrabBagStack->getCurrentName() == "attributes")
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
