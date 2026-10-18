/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/EffectItems.hxx>
#include <svx/unomid.hxx>

#include <docmodel/effect/GlowEffectJSON.hxx>
#include <docmodel/effect/ReflectionEffectJSON.hxx>
#include <docmodel/effect/SoftEdgeEffectJSON.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/uno/UnoGlowEffect.hxx>
#include <docmodel/uno/UnoReflectionEffect.hxx>
#include <docmodel/uno/UnoSoftEdgeEffect.hxx>

#include <tools/json_writer.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <cmath>
#include <sstream>

using namespace css;
using namespace ::cpo;

namespace
{
template <class Effect> OUString getEffectJSON(Effect const& rEffect)
{
    return OStringToOUString(model::effect::convertToJSON(rEffect), RTL_TEXTENCODING_UTF8);
}

/// Reads the whole effect from JSON. A value that the JSON lacks stays.
template <class Effect> bool putEffectJSON(const uno::Any& rValue, Effect& rEffect)
{
    OUString sJSON;
    if (!(rValue >>= sJSON))
        return false;
    Effect aEffect(rEffect);
    if (!model::effect::convertFromJSON(sJSON.toUtf8(), aEffect))
        return false;
    rEffect = aEffect;
    return true;
}

/// The JSON of an effect that can be off, with "Enabled" next to the values of the effect.
template <class Effect> OUString getEffectJSON(Effect const& rEffect, bool bEnabled)
{
    tools::JsonWriter aTree;
    aTree.put("Enabled", bEnabled);
    model::effect::convertToJSONTree(aTree, rEffect);
    return OUString::fromUtf8(aTree.finishAndGetAsOString());
}

/// Reads the whole effect and whether it is on from JSON. A value that the JSON lacks stays.
template <class Effect>
bool putEffectJSON(const uno::Any& rValue, Effect& rEffect, bool& rEnabled)
{
    OUString sJSON;
    if (!(rValue >>= sJSON))
        return false;
    bool bEnabled = rEnabled;
    try
    {
        std::stringstream aStream((std::string(sJSON.toUtf8())));
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        // A value that is no bool throws, and a JSON without "Enabled" keeps the state.
        if (auto oEnabled = aTree.get_child_optional("Enabled"))
            bEnabled = oEnabled->get_value<bool>();
    }
    catch (const boost::property_tree::ptree_error&)
    {
        return false;
    }
    if (!putEffectJSON(rValue, rEffect))
        return false;
    rEnabled = bEnabled;
    return true;
}

/// Reads the whole effect from its UNO object.
template <class Interface, class Effect>
bool putEffect(const uno::Any& rValue, Effect& rEffect,
               std::optional<Effect> (*pGetFromUnoEffect)(uno::Reference<Interface> const&))
{
    uno::Reference<Interface> xEffect;
    if (!(rValue >>= xEffect))
        return false;
    std::optional<Effect> oEffect = pGetFromUnoEffect(xEffect);
    if (!oEffect)
        return false;
    rEffect = *oEffect;
    return true;
}

/// The item with the JSON of the effect as its state.
template <class Effect>
boost::property_tree::ptree dumpEffectAsJSON(boost::property_tree::ptree aTree,
                                             Effect const& rEffect)
{
    boost::property_tree::ptree aState;
    std::stringstream aStream(std::string(model::effect::convertToJSON(rEffect)));
    boost::property_tree::read_json(aStream, aState);
    aTree.push_back(std::make_pair("state", aState));
    return aTree;
}
}

SdrGlowEffectItem::SdrGlowEffectItem(TypedWhichId<SdrGlowEffectItem> nWhich)
    : SfxPoolItem(nWhich)
{
    maEffect.maColor = model::ComplexColor::createRGB(COL_BLACK);
}

SdrGlowEffectItem::SdrGlowEffectItem(TypedWhichId<SdrGlowEffectItem> nWhich,
                                     model::GlowEffect const& rEffect)
    : SfxPoolItem(nWhich)
    , maEffect(rEffect)
{
}

SfxPoolItem* SdrGlowEffectItem::CreateDefault()
{
    return new SdrGlowEffectItem(TypedWhichId<SdrGlowEffectItem>(0));
}

Color SdrGlowEffectItem::getColor() const
{
    // The final color is the color with its transformations and its theme resolved. Its alpha is
    // the transparency of the glow, so the color itself is opaque.
    return maEffect.maColor.getFinalColor().GetRGBColor();
}

sal_Int16 SdrGlowEffectItem::getTransparency() const
{
    return 100 - std::lround(maEffect.maColor.getAlpha() / 100.0);
}

void SdrGlowEffectItem::setColor(Color aColor)
{
    if (aColor.GetRGBColor() == getColor().GetRGBColor())
        return;
    const sal_Int32 nAlpha = maEffect.maColor.getAlpha();
    maEffect.maColor = model::ComplexColor::createRGB(aColor.GetRGBColor());
    maEffect.maColor.setAlpha(nAlpha);
}

void SdrGlowEffectItem::setComplexColor(model::ComplexColor const& rColor)
{
    model::ComplexColor aColor(rColor);
    // The color that a theme color resolves to stays the color of the glow.
    if (aColor.getType() != model::ColorType::RGB)
        aColor.setFinalColor(getColor());
    aColor.setAlpha(maEffect.maColor.getAlpha());
    maEffect.maColor = aColor;
}

void SdrGlowEffectItem::setTransparency(sal_Int16 nTransparency)
{
    maEffect.maColor.setAlpha((100 - nTransparency) * 100);
}

bool SdrGlowEffectItem::resolveColor(model::ColorSet const& rColorSet)
{
    if (!maEffect.maColor.isValidThemeType())
        return false;
    maEffect.maColor.setFinalColor(rColorSet.resolveColor(maEffect.maColor));
    return true;
}

bool SdrGlowEffectItem::operator==(const SfxPoolItem& rItem) const
{
    if (!SfxPoolItem::operator==(rItem))
        return false;
    auto const& rOther = static_cast<const SdrGlowEffectItem&>(rItem);
    return maEffect == rOther.maEffect && getColor() == rOther.getColor();
}

SdrGlowEffectItem* SdrGlowEffectItem::Clone(SfxItemPool*) const
{
    return new SdrGlowEffectItem(*this);
}

bool SdrGlowEffectItem::QueryValue(uno::Any& rValue, sal_uInt8 nMemberId) const
{
    // The radius is a gfx::Length, so it does not depend on the unit of the pool.
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_EFFECT:
            rValue <<= model::effect::createXGlowEffect(maEffect);
            return true;
        case MID_EFFECT_JSON:
            rValue <<= getEffectJSON(maEffect);
            return true;
        case MID_GLOW_RADIUS:
            rValue <<= maEffect.maRadius.as_hmm<sal_Int32>();
            return true;
        case MID_GLOW_COLOR:
            rValue <<= getColor();
            return true;
        case MID_GLOW_COMPLEX_COLOR:
        {
            model::ComplexColor aColor(maEffect.maColor);
            aColor.setAlpha(10000);
            rValue <<= model::color::createXComplexColor(aColor);
            return true;
        }
        case MID_GLOW_TRANSPARENCY:
            rValue <<= getTransparency();
            return true;
    }
    return false;
}

bool SdrGlowEffectItem::PutValue(const uno::Any& rValue, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_EFFECT:
            return putEffect(rValue, maEffect, &model::effect::getFromXGlowEffect);
        case MID_EFFECT_JSON:
            return putEffectJSON(rValue, maEffect);
        case MID_GLOW_RADIUS:
        {
            sal_Int32 nRadius = 0;
            if (!(rValue >>= nRadius))
                return false;
            setRadius(gfx::Length::hmm(nRadius));
            return true;
        }
        case MID_GLOW_COLOR:
        {
            Color aColor;
            if (!(rValue >>= aColor))
                return false;
            setColor(aColor);
            return true;
        }
        case MID_GLOW_COMPLEX_COLOR:
        {
            uno::Reference<util::XComplexColor> xColor;
            if (!(rValue >>= xColor))
                return false;
            setComplexColor(model::color::getFromXComplexColor(xColor));
            return true;
        }
        case MID_GLOW_TRANSPARENCY:
        {
            sal_Int16 nTransparency = 0;
            if (!(rValue >>= nTransparency))
                return false;
            setTransparency(nTransparency);
            return true;
        }
    }
    return false;
}

boost::property_tree::ptree SdrGlowEffectItem::dumpAsJSON() const
{
    return dumpEffectAsJSON(SfxPoolItem::dumpAsJSON(), maEffect);
}

SdrSoftEdgeEffectItem::SdrSoftEdgeEffectItem(TypedWhichId<SdrSoftEdgeEffectItem> nWhich,
                                             model::SoftEdgeEffect const& rEffect)
    : SfxPoolItem(nWhich)
    , maEffect(rEffect)
{
}

SfxPoolItem* SdrSoftEdgeEffectItem::CreateDefault()
{
    return new SdrSoftEdgeEffectItem(TypedWhichId<SdrSoftEdgeEffectItem>(0));
}

bool SdrSoftEdgeEffectItem::operator==(const SfxPoolItem& rItem) const
{
    return SfxPoolItem::operator==(rItem)
           && maEffect == static_cast<const SdrSoftEdgeEffectItem&>(rItem).maEffect;
}

SdrSoftEdgeEffectItem* SdrSoftEdgeEffectItem::Clone(SfxItemPool*) const
{
    return new SdrSoftEdgeEffectItem(*this);
}

bool SdrSoftEdgeEffectItem::QueryValue(uno::Any& rValue, sal_uInt8 nMemberId) const
{
    // The radius is a gfx::Length, so it does not depend on the unit of the pool.
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_EFFECT:
            rValue <<= model::effect::createXSoftEdgeEffect(maEffect);
            return true;
        case MID_EFFECT_JSON:
            rValue <<= getEffectJSON(maEffect);
            return true;
        case MID_SOFTEDGE_RADIUS:
            rValue <<= maEffect.maRadius.as_hmm<sal_Int32>();
            return true;
    }
    return false;
}

bool SdrSoftEdgeEffectItem::PutValue(const uno::Any& rValue, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_EFFECT:
            return putEffect(rValue, maEffect, &model::effect::getFromXSoftEdgeEffect);
        case MID_EFFECT_JSON:
            return putEffectJSON(rValue, maEffect);
        case MID_SOFTEDGE_RADIUS:
        {
            sal_Int32 nRadius = 0;
            if (!(rValue >>= nRadius))
                return false;
            setRadius(gfx::Length::hmm(nRadius));
            return true;
        }
    }
    return false;
}

boost::property_tree::ptree SdrSoftEdgeEffectItem::dumpAsJSON() const
{
    return dumpEffectAsJSON(SfxPoolItem::dumpAsJSON(), maEffect);
}

SdrReflectionEffectItem::SdrReflectionEffectItem(TypedWhichId<SdrReflectionEffectItem> nWhich)
    : SfxPoolItem(nWhich)
{
    maEffect.mnDirection = 5400000_deg60000;
    maEffect.mnScaleY = -100000;
    maEffect.meAlignment = model::RectangleAlignment::BottomLeft;
    maEffect.mbRotateWithShape = false;
}

SfxPoolItem* SdrReflectionEffectItem::CreateDefault()
{
    return new SdrReflectionEffectItem(TypedWhichId<SdrReflectionEffectItem>(0));
}

// The alphas and the positions of the effect are in 1/1000 percent.

sal_Int16 SdrReflectionEffectItem::getStartTransparency() const
{
    return 100 - std::lround(maEffect.mnStartAlpha / 1000.0);
}

sal_Int16 SdrReflectionEffectItem::getEndTransparency() const
{
    return 100 - std::lround(maEffect.mnEndAlpha / 1000.0);
}

void SdrReflectionEffectItem::setStartTransparency(sal_Int16 nTransparency)
{
    maEffect.mnStartAlpha = (100 - nTransparency) * 1000;
}

void SdrReflectionEffectItem::setEndTransparency(sal_Int16 nTransparency)
{
    maEffect.mnEndAlpha = (100 - nTransparency) * 1000;
}

sal_Int16 SdrReflectionEffectItem::getStartPosition() const
{
    return std::lround(maEffect.mnStartPosition / 1000.0);
}

sal_Int16 SdrReflectionEffectItem::getEndPosition() const
{
    return std::lround(maEffect.mnEndPosition / 1000.0);
}

void SdrReflectionEffectItem::setStartPosition(sal_Int16 nPosition)
{
    maEffect.mnStartPosition = nPosition * 1000;
}

void SdrReflectionEffectItem::setEndPosition(sal_Int16 nPosition)
{
    maEffect.mnEndPosition = nPosition * 1000;
}

bool SdrReflectionEffectItem::operator==(const SfxPoolItem& rItem) const
{
    if (!SfxPoolItem::operator==(rItem))
        return false;
    auto const& rOther = static_cast<const SdrReflectionEffectItem&>(rItem);
    return mbEnabled == rOther.mbEnabled && maEffect == rOther.maEffect;
}

SdrReflectionEffectItem* SdrReflectionEffectItem::Clone(SfxItemPool*) const
{
    return new SdrReflectionEffectItem(*this);
}

bool SdrReflectionEffectItem::QueryValue(uno::Any& rValue, sal_uInt8 nMemberId) const
{
    // The lengths are gfx::Length, so they do not depend on the unit of the pool.
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_EFFECT:
            rValue <<= model::effect::createXReflectionEffect(maEffect);
            return true;
        case MID_EFFECT_JSON:
            rValue <<= getEffectJSON(maEffect, mbEnabled);
            return true;
        case MID_REFLECTION_ENABLED:
            rValue <<= mbEnabled;
            return true;
        case MID_REFLECTION_DISTANCE:
            rValue <<= maEffect.maDistance.as_hmm<sal_Int32>();
            return true;
        case MID_REFLECTION_BLUR_RADIUS:
            rValue <<= maEffect.maBlurRadius.as_hmm<sal_Int32>();
            return true;
        case MID_REFLECTION_START_TRANSPARENCY:
            rValue <<= getStartTransparency();
            return true;
        case MID_REFLECTION_START_POSITION:
            rValue <<= getStartPosition();
            return true;
        case MID_REFLECTION_END_TRANSPARENCY:
            rValue <<= getEndTransparency();
            return true;
        case MID_REFLECTION_END_POSITION:
            rValue <<= getEndPosition();
            return true;
    }
    return false;
}

bool SdrReflectionEffectItem::PutValue(const uno::Any& rValue, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_EFFECT:
            return putEffect(rValue, maEffect, &model::effect::getFromXReflectionEffect);
        case MID_EFFECT_JSON:
            return putEffectJSON(rValue, maEffect, mbEnabled);
        case MID_REFLECTION_ENABLED:
            return rValue >>= mbEnabled;
        case MID_REFLECTION_DISTANCE:
        case MID_REFLECTION_BLUR_RADIUS:
        {
            sal_Int32 nLength = 0;
            if (!(rValue >>= nLength))
                return false;
            if (nMemberId == MID_REFLECTION_DISTANCE)
                setDistance(gfx::Length::hmm(nLength));
            else
                setBlurRadius(gfx::Length::hmm(nLength));
            return true;
        }
        case MID_REFLECTION_START_TRANSPARENCY:
        case MID_REFLECTION_START_POSITION:
        case MID_REFLECTION_END_TRANSPARENCY:
        case MID_REFLECTION_END_POSITION:
        {
            sal_Int16 nPercent = 0;
            if (!(rValue >>= nPercent))
                return false;
            if (nMemberId == MID_REFLECTION_START_TRANSPARENCY)
                setStartTransparency(nPercent);
            else if (nMemberId == MID_REFLECTION_START_POSITION)
                setStartPosition(nPercent);
            else if (nMemberId == MID_REFLECTION_END_TRANSPARENCY)
                setEndTransparency(nPercent);
            else
                setEndPosition(nPercent);
            return true;
        }
    }
    return false;
}

boost::property_tree::ptree SdrReflectionEffectItem::dumpAsJSON() const
{
    boost::property_tree::ptree aTree = dumpEffectAsJSON(SfxPoolItem::dumpAsJSON(), maEffect);
    aTree.get_child("state").put("Enabled", mbEnabled);
    return aTree;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
