/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
        { OOX_TOKEN(w14, srgbClr)      , u"srgbClr"_ustr },
        { OOX_TOKEN(w14, schemeClr)    , u"schemeClr"_ustr },
        { OOX_TOKEN(w14, tint)         , u"tint"_ustr },
        { OOX_TOKEN(w14, shade)        , u"shade"_ustr },
        { OOX_TOKEN(w14, alpha)        , u"alpha"_ustr },
        { OOX_TOKEN(w14, hueMod)       , u"hueMod"_ustr },
        { OOX_TOKEN(w14, sat)          , u"sat"_ustr },
        { OOX_TOKEN(w14, satOff)       , u"satOff"_ustr },
        { OOX_TOKEN(w14, satMod)       , u"satMod"_ustr },
        { OOX_TOKEN(w14, lum)          , u"lum"_ustr },
        { OOX_TOKEN(w14, lumOff)       , u"lumOff"_ustr },
        { OOX_TOKEN(w14, lumMod)       , u"lumMod"_ustr },
        { OOX_TOKEN(w14, noFill)       , u"noFill"_ustr },
        { OOX_TOKEN(w14, solidFill)    , u"solidFill"_ustr },
        { OOX_TOKEN(w14, gradFill)     , u"gradFill"_ustr },
        { OOX_TOKEN(w14, gsLst)        , u"gsLst"_ustr },
        { OOX_TOKEN(w14, gs)           , u"gs"_ustr },
        { OOX_TOKEN(w14, pos)          , u"pos"_ustr },
        { OOX_TOKEN(w14, lin)          , u"lin"_ustr },
        { OOX_TOKEN(w14, path)         , u"path"_ustr },
        { OOX_TOKEN(w14, fillToRect)   , u"fillToRect"_ustr },
        { OOX_TOKEN(w14, prstDash)     , u"prstDash"_ustr },
        { OOX_TOKEN(w14, round)        , u"round"_ustr },
        { OOX_TOKEN(w14, bevel)        , u"bevel"_ustr },
        { OOX_TOKEN(w14, miter)        , u"miter"_ustr },
        { OOX_TOKEN(w14, camera)       , u"camera"_ustr },
        { OOX_TOKEN(w14, lightRig)     , u"lightRig"_ustr },
        { OOX_TOKEN(w14, rot)          , u"rot"_ustr },
        { OOX_TOKEN(w14, bevelT)       , u"bevelT"_ustr },
        { OOX_TOKEN(w14, bevelB)       , u"bevelB"_ustr },
        { OOX_TOKEN(w14, extrusionClr) , u"extrusionClr"_ustr },
        { OOX_TOKEN(w14, contourClr)   , u"contourClr"_ustr} ,
        { OOX_TOKEN(w14, styleSet)     , u"styleSet"_ustr },

        { OOX_TOKEN(w14, glow)         , u"glow"_ustr },
        { OOX_TOKEN(w14, shadow)       , u"shadow"_ustr },
        { OOX_TOKEN(w14, reflection)   , u"reflection"_ustr },
        { OOX_TOKEN(w14, textOutline)  , u"textOutline"_ustr },
        { OOX_TOKEN(w14, textFill)     , u"textFill"_ustr },
        { OOX_TOKEN(w14, scene3d)      , u"scene3d"_ustr },
        { OOX_TOKEN(w14, props3d)      , u"props3d"_ustr },
        { OOX_TOKEN(w14, ligatures)    , u"ligatures"_ustr },
        { OOX_TOKEN(w14, numForm)      , u"numForm"_ustr },
        { OOX_TOKEN(w14, numSpacing)   , u"numSpacing"_ustr },
        { OOX_TOKEN(w14, stylisticSets), u"stylisticSets"_ustr },
        { OOX_TOKEN(w14, cntxtAlts)    , u"cntxtAlts"_ustr },
    };
    const auto iter = aIdMap.find(aId);
    assert(iter != aIdMap.end());
    return iter->second;
}

OUString const & lclGetGrabBagName(sal_uInt32 aId)
{
    static std::map<sal_uInt32, OUString> const aGrabBagNameMap
    {
        { OOX_TOKEN(w14, glow)         , u"CharGlowTextEffect"_ustr },
        { OOX_TOKEN(w14, shadow)       , u"CharShadowTextEffect"_ustr },
        { OOX_TOKEN(w14, reflection)   , u"CharReflectionTextEffect"_ustr },
        { OOX_TOKEN(w14, textOutline)  , u"CharTextOutlineTextEffect"_ustr },
        { OOX_TOKEN(w14, textFill)     , u"CharTextFillTextEffect"_ustr },
        { OOX_TOKEN(w14, scene3d)      , u"CharScene3DTextEffect"_ustr },
        { OOX_TOKEN(w14, props3d)      , u"CharProps3DTextEffect"_ustr },
        { OOX_TOKEN(w14, ligatures)    , u"CharLigaturesTextEffect"_ustr },
        { OOX_TOKEN(w14, numForm)      , u"CharNumFormTextEffect"_ustr },
        { OOX_TOKEN(w14, numSpacing)   , u"CharNumSpacingTextEffect"_ustr },
        { OOX_TOKEN(w14, stylisticSets), u"CharStylisticSetsTextEffect"_ustr },
        { OOX_TOKEN(w14, cntxtAlts)    , u"CharCntxtAltsTextEffect"_ustr },
    };
    const auto iter = aGrabBagNameMap.find(aId);
    assert(iter != aGrabBagNameMap.end());
    return iter->second;
}

}

using namespace oox::core;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace ::cpo::uno;

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
