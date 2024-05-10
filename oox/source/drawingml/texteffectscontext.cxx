/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <drawingml/texteffectscontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>

#include <map>

namespace oox::drawingml {

namespace
{

OUString const & lclGetNameForElementId(sal_uInt32 aId)
{
    static std::map<sal_uInt32, OUString> const aIdMap
    {
        { OOX_TOKEN(w14, srgbClr)      , "srgbClr" },
        { OOX_TOKEN(w14, schemeClr)    , "schemeClr" },
        { OOX_TOKEN(w14, tint)         , "tint" },
        { OOX_TOKEN(w14, shade)        , "shade" },
        { OOX_TOKEN(w14, alpha)        , "alpha" },
        { OOX_TOKEN(w14, hueMod)       , "hueMod" },
        { OOX_TOKEN(w14, sat)          , "sat" },
        { OOX_TOKEN(w14, satOff)       , "satOff" },
        { OOX_TOKEN(w14, satMod)       , "satMod" },
        { OOX_TOKEN(w14, lum)          , "lum" },
        { OOX_TOKEN(w14, lumOff)       , "lumOff" },
        { OOX_TOKEN(w14, lumMod)       , "lumMod" },
        { OOX_TOKEN(w14, noFill)       , "noFill" },
        { OOX_TOKEN(w14, solidFill)    , "solidFill" },
        { OOX_TOKEN(w14, gradFill)     , "gradFill" },
        { OOX_TOKEN(w14, gsLst)        , "gsLst" },
        { OOX_TOKEN(w14, gs)           , "gs" },
        { OOX_TOKEN(w14, pos)          , "pos" },
        { OOX_TOKEN(w14, lin)          , "lin" },
        { OOX_TOKEN(w14, path)         , "path" },
        { OOX_TOKEN(w14, fillToRect)   , "fillToRect" },
        { OOX_TOKEN(w14, prstDash)     , "prstDash" },
        { OOX_TOKEN(w14, round)        , "round" },
        { OOX_TOKEN(w14, bevel)        , "bevel" },
        { OOX_TOKEN(w14, miter)        , "miter" },
        { OOX_TOKEN(w14, camera)       , "camera" },
        { OOX_TOKEN(w14, lightRig)     , "lightRig" },
        { OOX_TOKEN(w14, rot)          , "rot" },
        { OOX_TOKEN(w14, bevelT)       , "bevelT" },
        { OOX_TOKEN(w14, bevelB)       , "bevelB" },
        { OOX_TOKEN(w14, extrusionClr) , "extrusionClr" },
        { OOX_TOKEN(w14, contourClr)   , "contourClr"} ,
        { OOX_TOKEN(w14, styleSet)     , "styleSet" },

        { OOX_TOKEN(w14, glow)         , "glow" },
        { OOX_TOKEN(w14, shadow)       , "shadow" },
        { OOX_TOKEN(w14, reflection)   , "reflection" },
        { OOX_TOKEN(w14, textOutline)  , "textOutline" },
        { OOX_TOKEN(w14, textFill)     , "textFill" },
        { OOX_TOKEN(w14, scene3d)      , "scene3d" },
        { OOX_TOKEN(w14, props3d)      , "props3d" },
        { OOX_TOKEN(w14, ligatures)    , "ligatures" },
        { OOX_TOKEN(w14, numForm)      , "numForm" },
        { OOX_TOKEN(w14, numSpacing)   , "numSpacing" },
        { OOX_TOKEN(w14, stylisticSets), "stylisticSets" },
        { OOX_TOKEN(w14, cntxtAlts)    , "cntxtAlts" },
    };
    const auto iter = aIdMap.find(aId);
    assert(iter != aIdMap.end());
    return iter->second;
}

OUString const & lclGetGrabBagName(sal_uInt32 aId)
{
    static std::map<sal_uInt32, OUString> const aGrabBagNameMap
    {
        { OOX_TOKEN(w14, glow)         , "CharGlowTextEffect" },
        { OOX_TOKEN(w14, shadow)       , "CharShadowTextEffect" },
        { OOX_TOKEN(w14, reflection)   , "CharReflectionTextEffect" },
        { OOX_TOKEN(w14, textOutline)  , "CharTextOutlineTextEffect" },
        { OOX_TOKEN(w14, textFill)     , "CharTextFillTextEffect" },
        { OOX_TOKEN(w14, scene3d)      , "CharScene3DTextEffect" },
        { OOX_TOKEN(w14, props3d)      , "CharProps3DTextEffect" },
        { OOX_TOKEN(w14, ligatures)    , "CharLigaturesTextEffect" },
        { OOX_TOKEN(w14, numForm)      , "CharNumFormTextEffect" },
        { OOX_TOKEN(w14, numSpacing)   , "CharNumSpacingTextEffect" },
        { OOX_TOKEN(w14, stylisticSets), "CharStylisticSetsTextEffect" },
        { OOX_TOKEN(w14, cntxtAlts)    , "CharCntxtAltsTextEffect" },
    };
    const auto iter = aGrabBagNameMap.find(aId);
    assert(iter != aGrabBagNameMap.end());
    return iter->second;
}

}

using namespace oox::core;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

TextEffectsContext::TextEffectsContext(
                        ContextHandler2Helper const & rParent,
                        sal_Int32 aElementToken,
                        std::vector<PropertyValue>& rTextEffectsProperties)
    : ContextHandler2(rParent)
    , mrTextEffectsProperties(rTextEffectsProperties)
    , mnCurrentElement(aElementToken)
{
}

TextEffectsContext::~TextEffectsContext()
{
}

void TextEffectsContext::pushAttributeToGrabBag (sal_Int32 aAttributeId, const OUString& rElementName, const AttributeList& rAttribs)
{
    if (!rAttribs.hasAttribute(aAttributeId))
        return;
    OUString aString = rAttribs.getStringDefaulted(aAttributeId);
    mpGrabBagStack->addString(rElementName, aString);
}

void TextEffectsContext::processAttributes(const AttributeList& rAttribs)
{
    mpGrabBagStack->push(u"attributes"_ustr);
    switch(mnCurrentElement)
    {
        case OOX_TOKEN(w14, glow):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, rad),  u"rad"_ustr,  rAttribs);
        }
        break;
        case OOX_TOKEN(w14, srgbClr):
        case OOX_TOKEN(w14, schemeClr):
        case OOX_TOKEN(w14, tint):
        case OOX_TOKEN(w14, shade):
        case OOX_TOKEN(w14, alpha):
        case OOX_TOKEN(w14, hueMod):
        case OOX_TOKEN(w14, sat):
        case OOX_TOKEN(w14, satOff):
        case OOX_TOKEN(w14, satMod):
        case OOX_TOKEN(w14, lum):
        case OOX_TOKEN(w14, lumOff):
        case OOX_TOKEN(w14, lumMod):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, val),  u"val"_ustr,  rAttribs);
        }
        break;
        case OOX_TOKEN(w14, shadow):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, blurRad),  u"blurRad"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dist),     u"dist"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dir),      u"dir"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, sx),       u"sx"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, sy),       u"sy"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, kx),       u"kx"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, ky),       u"ky"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, algn),     u"algn"_ustr,  rAttribs);
        }
        break;
        case OOX_TOKEN(w14, reflection):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, blurRad),  u"blurRad"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, stA),      u"stA"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, stPos),    u"stPos"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, endA),     u"endA"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, endPos),   u"endPos"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dist),     u"dist"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dir),      u"dir"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, fadeDir),  u"fadeDir"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, sx),       u"sx"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, sy),       u"sy"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, kx),       u"kx"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, ky),       u"ky"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, algn),     u"algn"_ustr,  rAttribs);
        }
        break;
        case OOX_TOKEN(w14, textOutline):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, w),     u"w"_ustr,   rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, cap),  u"cap"_ustr,  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, cmpd), u"cmpd"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, algn), u"algn"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, prstDash):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, val),  u"val"_ustr,  rAttribs);
        }
        break;
        case OOX_TOKEN(w14, gs):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, pos),  u"pos"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, lin):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, ang),    u"ang"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, scaled), u"scaled"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, path):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, path), u"path"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, fillToRect):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, l), u"l"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, t), u"t"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, r), u"r"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, b), u"b"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, miter):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, lim), u"lim"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, camera):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, prst), u"prst"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, lightRig):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, rig), u"rig"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dir), u"dir"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, rot):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, lat), u"lat"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, lon), u"lon"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, rev), u"rev"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, props3d):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, extrusionH),   u"extrusionH"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, contourW),     u"contourW"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, prstMaterial), u"prstMaterial"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, bevelT):
        case OOX_TOKEN(w14, bevelB):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, w),    u"w"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, h),    u"h"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, prst), u"prst"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, ligatures):
        case OOX_TOKEN(w14, numForm):
        case OOX_TOKEN(w14, numSpacing):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, val), u"val"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, styleSet):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, id), u"id"_ustr, rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, val), u"val"_ustr, rAttribs);
        }
        break;
        case OOX_TOKEN(w14, cntxtAlts):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, val), u"val"_ustr, rAttribs);
        }
        break;
        default:
            break;
    }

    mpGrabBagStack->pop();
}

void TextEffectsContext::onStartElement(const oox::AttributeList& rAttribs)
{
    if (mpGrabBagStack == nullptr)
    {
        OUString aGrabBagName = lclGetGrabBagName(mnCurrentElement);
        mpGrabBagStack.reset(new GrabBagStack(aGrabBagName));
    }

    OUString aElementName = lclGetNameForElementId(mnCurrentElement);
    mpGrabBagStack->push(aElementName);
    processAttributes(rAttribs);
}

void TextEffectsContext::onEndElement()
{
    mpGrabBagStack->pop();

    if (mpGrabBagStack->isStackEmpty())
    {
        Sequence<PropertyValue> aSeq;
        PropertyValue aPropertyValue = mpGrabBagStack->getRootProperty();
        aPropertyValue.Value >>= aSeq;
        aPropertyValue.Value <<= aSeq[0];

        mrTextEffectsProperties.push_back(aPropertyValue);
    }

}

ContextHandlerRef TextEffectsContext::onCreateContext(sal_Int32 aElementToken, const AttributeList& /*rAttribs*/)
{
    mnCurrentElement = aElementToken;
    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
