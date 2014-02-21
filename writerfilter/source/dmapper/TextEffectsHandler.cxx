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
};

OUString TextEffectsHandler::getSchemeColorTypeString(sal_Int32 nType)
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


TextEffectsHandler::TextEffectsHandler() :
    LoggedProperties(dmapper_logger, "TextEffectsHandler"),
    mpGrabBagStack(NULL)
{
}

TextEffectsHandler::~TextEffectsHandler()
{
}

void TextEffectsHandler::lcl_attribute(Id aName, Value& aValue)
{

    if (mpGrabBagStack->getCurrentName() != "attributes")
        mpGrabBagStack->push("attributes");

    switch(aName)
    {
        case NS_ooxml::LN_CT_Percentage_val:
            mpGrabBagStack->appendElement("val", makeAny(sal_Int32(aValue.getInt())));
            break;
        case NS_ooxml::LN_CT_PositiveFixedPercentage_val:
            mpGrabBagStack->appendElement("val", makeAny(sal_Int32(aValue.getInt())));
            break;
        case NS_ooxml::LN_CT_PositivePercentage_val:
            mpGrabBagStack->appendElement("val", makeAny(sal_Int32(aValue.getInt())));
            break;
        case NS_ooxml::LN_CT_SchemeColor_val:
            mpGrabBagStack->appendElement("val", makeAny(getSchemeColorTypeString(sal_Int32(aValue.getInt()))));
            break;
        case NS_ooxml::LN_CT_SRgbColor_val:
            {
                OUStringBuffer aBuf = OUString::number(aValue.getInt(), 16);
                OUStringBuffer aStr;
                comphelper::string::padToLength(aStr, 6 - aBuf.getLength(), '0');
                aStr.append(aBuf.getStr());
                mpGrabBagStack->appendElement("val", makeAny(aStr.makeStringAndClear()));
            }
            break;
        case NS_ooxml::LN_CT_Glow_rad:
            mpGrabBagStack->appendElement("rad", makeAny(sal_Int32(aValue.getInt())));
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

void TextEffectsHandler::enableInteropGrabBag(OUString aName)
{
    mpGrabBagStack.reset(new GrabBagStack(aName));
}

void TextEffectsHandler::disableInteropGrabBag()
{
    mpGrabBagStack.reset();
}

bool TextEffectsHandler::isInteropGrabBagEnabled()
{
    return mpGrabBagStack.get() != NULL;
}


}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
