/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "drawingml/texteffectscontext.hxx"

#include <map>

namespace oox { namespace drawingml {

namespace
{

OUString lclGetNameForElementId(sal_uInt32 aId)
{
    static std::map<sal_uInt32, OUString> aIdMap;
    if(aIdMap.empty())
    {
        aIdMap[OOX_TOKEN(w14, srgbClr)]      = "srgbClr";
        aIdMap[OOX_TOKEN(w14, schemeClr)]    = "schemeClr";
        aIdMap[OOX_TOKEN(w14, tint)]         = "tint";
        aIdMap[OOX_TOKEN(w14, shade)]        = "shade";
        aIdMap[OOX_TOKEN(w14, alpha)]        = "alpha";
        aIdMap[OOX_TOKEN(w14, hueMod)]       = "hueMod";
        aIdMap[OOX_TOKEN(w14, sat)]          = "sat";
        aIdMap[OOX_TOKEN(w14, satOff)]       = "satOff";
        aIdMap[OOX_TOKEN(w14, satMod)]       = "satMod";
        aIdMap[OOX_TOKEN(w14, lum)]          = "lum";
        aIdMap[OOX_TOKEN(w14, lumOff)]       = "lumOff";
        aIdMap[OOX_TOKEN(w14, lumMod)]       = "lumMod";
        aIdMap[OOX_TOKEN(w14, noFill)]       = "noFill";
        aIdMap[OOX_TOKEN(w14, solidFill)]    = "solidFill";
        aIdMap[OOX_TOKEN(w14, gradFill)]     = "gradFill";
        aIdMap[OOX_TOKEN(w14, gsLst)]        = "gsLst";
        aIdMap[OOX_TOKEN(w14, gs)]           = "gs";
        aIdMap[OOX_TOKEN(w14, pos)]          = "pos";
        aIdMap[OOX_TOKEN(w14, lin)]          = "lin";
        aIdMap[OOX_TOKEN(w14, path)]         = "path";
        aIdMap[OOX_TOKEN(w14, fillToRect)]   = "fillToRect";
        aIdMap[OOX_TOKEN(w14, prstDash)]     = "prstDash";
        aIdMap[OOX_TOKEN(w14, round)]        = "round";
        aIdMap[OOX_TOKEN(w14, bevel)]        = "bevel";
        aIdMap[OOX_TOKEN(w14, miter)]        = "miter";
        aIdMap[OOX_TOKEN(w14, camera)]       = "camera";
        aIdMap[OOX_TOKEN(w14, lightRig)]     = "lightRig";
        aIdMap[OOX_TOKEN(w14, rot)]          = "rot";
        aIdMap[OOX_TOKEN(w14, bevelT)]       = "bevelT";
        aIdMap[OOX_TOKEN(w14, bevelB)]       = "bevelB";
        aIdMap[OOX_TOKEN(w14, extrusionClr)] = "extrusionClr";
        aIdMap[OOX_TOKEN(w14, contourClr)]   = "contourClr";
        aIdMap[OOX_TOKEN(w14, styleSet)]     = "styleSet";

        aIdMap[OOX_TOKEN(w14, glow)]         = "glow";
        aIdMap[OOX_TOKEN(w14, shadow)]       = "shadow";
        aIdMap[OOX_TOKEN(w14, reflection)]   = "reflection";
        aIdMap[OOX_TOKEN(w14, textOutline)]  = "textOutline";
        aIdMap[OOX_TOKEN(w14, textFill)]     = "textFill";
        aIdMap[OOX_TOKEN(w14, scene3d)]      = "scene3d";
        aIdMap[OOX_TOKEN(w14, props3d)]      = "props3d";
        aIdMap[OOX_TOKEN(w14, ligatures)]    = "ligatures";
        aIdMap[OOX_TOKEN(w14, numForm)]      = "numForm";
        aIdMap[OOX_TOKEN(w14, numSpacing)]   = "numSpacing";
        aIdMap[OOX_TOKEN(w14, stylisticSets)]= "stylisticSets";
        aIdMap[OOX_TOKEN(w14, cntxtAlts)]    = "cntxtAlts";
    }

    return aIdMap[aId];
}

OUString lclGetGrabBagName(sal_uInt32 aId)
{
    static std::map<sal_uInt32, OUString> aGrabBagNameMap;
    if(aGrabBagNameMap.empty())
    {
        aGrabBagNameMap[OOX_TOKEN(w14, glow)]         = "CharGlowTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, shadow)]       = "CharShadowTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, reflection)]   = "CharReflectionTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, textOutline)]  = "CharTextOutlineTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, textFill)]     = "CharTextFillTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, scene3d)]      = "CharScene3DTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, props3d)]      = "CharProps3DTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, ligatures)]    = "CharLigaturesTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, numForm)]      = "CharNumFormTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, numSpacing)]   = "CharNumSpacingTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, stylisticSets)]= "CharStylisticSetsTextEffect";
        aGrabBagNameMap[OOX_TOKEN(w14, cntxtAlts)]    = "CharCntxtAltsTextEffect";
    }

    return aGrabBagNameMap[aId];
}

const char constAttributesSequenceName[] = "attributes";

}

using namespace oox::core;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

TextEffectsContext::TextEffectsContext(
                        ContextHandler2Helper& rParent,
                        sal_Int32 aElementToken,
                        std::vector<PropertyValue>& rTextEffectsProperties)
    : ContextHandler2(rParent)
    , mrTextEffectsProperties(rTextEffectsProperties)
    , mpGrabBagStack(nullptr)
    , mnCurrentElement(aElementToken)
{
}

TextEffectsContext::~TextEffectsContext()
{
}

void TextEffectsContext::pushAttributeToGrabBag (const sal_Int32& aAttributeId, const OUString& rElementName, const AttributeList& rAttribs)
{
    if (!rAttribs.hasAttribute(aAttributeId))
        return;
    OUString aString = rAttribs.getString(aAttributeId).get();
    mpGrabBagStack->addString(rElementName, aString);
}

void TextEffectsContext::processAttributes(const AttributeList& rAttribs)
{
    mpGrabBagStack->push(constAttributesSequenceName);
    switch(mnCurrentElement)
    {
        case OOX_TOKEN(w14, glow):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, rad),  "rad",  rAttribs);
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
            pushAttributeToGrabBag(OOX_TOKEN(w14, val),  "val",  rAttribs);
        }
        break;
        case OOX_TOKEN(w14, shadow):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, blurRad),  "blurRad",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dist),     "dist",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dir),      "dir",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, sx),       "sx",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, sy),       "sy",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, kx),       "kx",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, ky),       "ky",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, algn),     "algn",  rAttribs);
        }
        break;
        case OOX_TOKEN(w14, reflection):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, blurRad),  "blurRad",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, stA),      "stA",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, stPos),    "stPos",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, endA),     "endA",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, endPos),   "endPos",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dist),     "dist",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dir),      "dir",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, fadeDir),  "fadeDir",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, sx),       "sx",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, sy),       "sy",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, kx),       "kx",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, ky),       "ky",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, algn),     "algn",  rAttribs);
        }
        break;
        case OOX_TOKEN(w14, textOutline):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, w),     "w",   rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, cap),  "cap",  rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, cmpd), "cmpd", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, algn), "algn", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, prstDash):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, val),  "val",  rAttribs);
        }
        break;
        case OOX_TOKEN(w14, gs):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, pos),  "pos", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, lin):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, ang),    "ang", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, scaled), "scaled", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, path):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, path), "path", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, fillToRect):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, l), "l", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, t), "t", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, r), "r", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, b), "b", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, miter):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, lim), "lim", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, camera):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, prst), "prst", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, lightRig):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, rig), "rig", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, dir), "dir", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, rot):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, lat), "lat", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, lon), "lon", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, rev), "rev", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, props3d):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, extrusionH),   "extrusionH", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, contourW),     "contourW", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, prstMaterial), "prstMaterial", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, bevelT):
        case OOX_TOKEN(w14, bevelB):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, w),    "w", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, h),    "h", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, prst), "prst", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, ligatures):
        case OOX_TOKEN(w14, numForm):
        case OOX_TOKEN(w14, numSpacing):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, val), "val", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, styleSet):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, id), "id", rAttribs);
            pushAttributeToGrabBag(OOX_TOKEN(w14, val), "val", rAttribs);
        }
        break;
        case OOX_TOKEN(w14, cntxtAlts):
        {
            pushAttributeToGrabBag(OOX_TOKEN(w14, val), "val", rAttribs);
        }
        break;
        default:
            break;
    }

    mpGrabBagStack->pop();
}

void TextEffectsContext::onStartElement(const oox::AttributeList& rAttribs)
{
    if(mpGrabBagStack.get() == nullptr)
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
        aPropertyValue.Value = makeAny(aSeq[0]);

        mrTextEffectsProperties.push_back(aPropertyValue);
    }

}

ContextHandlerRef TextEffectsContext::onCreateContext(sal_Int32 aElementToken, const AttributeList& /*rAttribs*/)
{
    mnCurrentElement = aElementToken;
    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
