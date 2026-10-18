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

#pragma once

#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>
#include <docmodel/effect/GlowEffect.hxx>
#include <tools/color.hxx>

namespace model
{
class ColorSet;
}

/** The glow of a shape. A glow with a radius of 0 is no glow.

    The members of the item for UNO are MID_EFFECT for the XGlowEffect, MID_EFFECT_JSON for the
    JSON of model::effect::convertToJSON, and MID_GLOW_RADIUS (1/100 mm), MID_GLOW_COLOR,
    MID_GLOW_COMPLEX_COLOR and MID_GLOW_TRANSPARENCY (percent) for the single values.
 */
class SVXCORE_DLLPUBLIC SdrGlowEffectItem final : public SfxPoolItem
{
    model::GlowEffect maEffect;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrGlowEffectItem)
    /// No glow: a radius of 0, and black as the color that a glow gets.
    explicit SdrGlowEffectItem(TypedWhichId<SdrGlowEffectItem> nWhich);
    SdrGlowEffectItem(TypedWhichId<SdrGlowEffectItem> nWhich, model::GlowEffect const& rEffect);
    static SfxPoolItem* CreateDefault();

    model::GlowEffect const& getEffect() const { return maEffect; }

    /// The color of the glow without its transparency.
    Color getColor() const;
    /// The transparency of the glow in percent.
    sal_Int16 getTransparency() const;

    void setRadius(gfx::Length aRadius) { maEffect.maRadius = aRadius; }
    /// Sets a plain color. The color that the glow already has keeps its theme color.
    void setColor(Color aColor);
    /** Sets the color, without changing the transparency. A theme color keeps the color that
        the glow has until resolveColor sets the color of the theme. */
    void setComplexColor(model::ComplexColor const& rColor);
    void setTransparency(sal_Int16 nTransparency);
    /// Sets the color that a theme color has in the color set. False when it is no theme color.
    bool resolveColor(model::ColorSet const& rColorSet);

    bool operator==(const SfxPoolItem& rItem) const override;
    SdrGlowEffectItem* Clone(SfxItemPool* pPool = nullptr) const override;
    bool QueryValue(cpo::uno::Any& rValue, sal_uInt8 nMemberId = 0) const override;
    bool PutValue(const cpo::uno::Any& rValue, sal_uInt8 nMemberId) override;
    boost::property_tree::ptree dumpAsJSON() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
