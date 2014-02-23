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
        case NS_ooxml::LN_textFill_textFill:
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
        default:
            break;
    }
}

void TextEffectsHandler::lcl_sprm(Sprm& rSprm)
{
    if (mpGrabBagStack->getCurrentName() == "attributes")
        mpGrabBagStack->pop();

    sal_uInt32 nSprmId = rSprm.getId();

    switch(nSprmId)
    {
        case NS_ooxml::LN_EG_ColorChoice_srgbClr:
            mpGrabBagStack->push("srgbClr");
            break;
        case NS_ooxml::LN_EG_ColorChoice_schemeClr:
            mpGrabBagStack->push("schemeClr");
            break;
        case NS_ooxml::LN_EG_ColorTransform_tint:
            mpGrabBagStack->push("tint");
            break;
        case NS_ooxml::LN_EG_ColorTransform_shade:
            mpGrabBagStack->push("shade");
            break;
        case NS_ooxml::LN_EG_ColorTransform_alpha:
            mpGrabBagStack->push("alpha");
            break;
        case NS_ooxml::LN_EG_ColorTransform_hueMod:
            mpGrabBagStack->push("hueMod");
            break;
        case NS_ooxml::LN_EG_ColorTransform_sat:
            mpGrabBagStack->push("sat");
            break;
        case NS_ooxml::LN_EG_ColorTransform_satOff:
            mpGrabBagStack->push("satOff");
            break;
        case NS_ooxml::LN_EG_ColorTransform_satMod:
            mpGrabBagStack->push("satMod");
            break;
        case NS_ooxml::LN_EG_ColorTransform_lum:
            mpGrabBagStack->push("lum");
            break;
        case NS_ooxml::LN_EG_ColorTransform_lumOff:
            mpGrabBagStack->push("lumOff");
            break;
        case NS_ooxml::LN_EG_ColorTransform_lumMod:
            mpGrabBagStack->push("lumMod");
            break;

        default:
            break;
    }

    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( !pProperties.get())
        return;

    pProperties.get()->resolve( *this );

    if (mpGrabBagStack->getCurrentName() == "attributes")
        mpGrabBagStack->pop();

    switch(nSprmId)
    {
        case NS_ooxml::LN_EG_ColorChoice_srgbClr:
        case NS_ooxml::LN_EG_ColorChoice_schemeClr:
        case NS_ooxml::LN_EG_ColorTransform_tint:
        case NS_ooxml::LN_EG_ColorTransform_shade:
        case NS_ooxml::LN_EG_ColorTransform_alpha:
        case NS_ooxml::LN_EG_ColorTransform_hueMod:
        case NS_ooxml::LN_EG_ColorTransform_sat:
        case NS_ooxml::LN_EG_ColorTransform_satOff:
        case NS_ooxml::LN_EG_ColorTransform_satMod:
        case NS_ooxml::LN_EG_ColorTransform_lum:
        case NS_ooxml::LN_EG_ColorTransform_lumOff:
        case NS_ooxml::LN_EG_ColorTransform_lumMod:
            mpGrabBagStack->pop();
            break;
    }
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
