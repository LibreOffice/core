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
#include <docmodel/effect/SoftEdgeEffectJSON.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/uno/UnoGlowEffect.hxx>
#include <docmodel/uno/UnoSoftEdgeEffect.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
