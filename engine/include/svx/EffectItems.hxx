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
#include <docmodel/effect/BlurEffect.hxx>
#include <docmodel/effect/GlowEffect.hxx>
#include <docmodel/effect/InnerShadowEffect.hxx>
#include <docmodel/effect/ReflectionEffect.hxx>
#include <docmodel/effect/SoftEdgeEffect.hxx>
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

/** The soft edge of a shape. A soft edge with a radius of 0 is no soft edge.

    The members of the item for UNO are MID_EFFECT for the XSoftEdgeEffect, MID_EFFECT_JSON for
    the JSON of model::effect::convertToJSON, and MID_SOFTEDGE_RADIUS (1/100 mm).
 */
class SVXCORE_DLLPUBLIC SdrSoftEdgeEffectItem final : public SfxPoolItem
{
    model::SoftEdgeEffect maEffect;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrSoftEdgeEffectItem)
    explicit SdrSoftEdgeEffectItem(TypedWhichId<SdrSoftEdgeEffectItem> nWhich,
                                   model::SoftEdgeEffect const& rEffect = {});
    static SfxPoolItem* CreateDefault();

    model::SoftEdgeEffect const& getEffect() const { return maEffect; }
    void setRadius(gfx::Length aRadius) { maEffect.maRadius = aRadius; }

    bool operator==(const SfxPoolItem& rItem) const override;
    SdrSoftEdgeEffectItem* Clone(SfxItemPool* pPool = nullptr) const override;
    bool QueryValue(cpo::uno::Any& rValue, sal_uInt8 nMemberId = 0) const override;
    bool PutValue(const cpo::uno::Any& rValue, sal_uInt8 nMemberId) override;
    boost::property_tree::ptree dumpAsJSON() const override;
};

/** The reflection of a shape, and whether the shape has it. A reflection that is off keeps its
    values, so turning it on again brings them back.

    The default is a plain mirror image below the shape, which is how the reflection is drawn and
    what the reflection presets use, so a reflection made in the document matches what is drawn.

    The members of the item for UNO are MID_EFFECT for the XReflectionEffect, MID_EFFECT_JSON for
    the JSON of model::effect::convertToJSON, MID_REFLECTION_ENABLED, MID_REFLECTION_DISTANCE and
    MID_REFLECTION_BLUR_RADIUS (1/100 mm), and the transparencies and positions of the fade in
    percent.
 */
class SVXCORE_DLLPUBLIC SdrReflectionEffectItem final : public SfxPoolItem
{
    bool mbEnabled = false;
    model::ReflectionEffect maEffect;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrReflectionEffectItem)
    explicit SdrReflectionEffectItem(TypedWhichId<SdrReflectionEffectItem> nWhich);
    static SfxPoolItem* CreateDefault();

    bool isEnabled() const { return mbEnabled; }
    void setEnabled(bool bEnabled) { mbEnabled = bEnabled; }
    model::ReflectionEffect const& getEffect() const { return maEffect; }

    void setDistance(gfx::Length aDistance) { maEffect.maDistance = aDistance; }
    void setBlurRadius(gfx::Length aBlurRadius) { maEffect.maBlurRadius = aBlurRadius; }

    /// The transparency where the fade starts and ends, in percent.
    sal_Int16 getStartTransparency() const;
    sal_Int16 getEndTransparency() const;
    void setStartTransparency(sal_Int16 nTransparency);
    void setEndTransparency(sal_Int16 nTransparency);

    /// The position where the fade starts and ends, in percent of the height of the reflection.
    sal_Int16 getStartPosition() const;
    sal_Int16 getEndPosition() const;
    void setStartPosition(sal_Int16 nPosition);
    void setEndPosition(sal_Int16 nPosition);

    bool operator==(const SfxPoolItem& rItem) const override;
    SdrReflectionEffectItem* Clone(SfxItemPool* pPool = nullptr) const override;
    bool QueryValue(cpo::uno::Any& rValue, sal_uInt8 nMemberId = 0) const override;
    bool PutValue(const cpo::uno::Any& rValue, sal_uInt8 nMemberId) override;
    boost::property_tree::ptree dumpAsJSON() const override;
};

/** The blur of a shape, and whether the shape has it. A blur with a radius of 0 is a blur all the
    same, so the item says whether there is one. Nothing draws the blur yet, so the item keeps it
    for the round trip of the document.

    The members of the item for UNO are MID_EFFECT for the XBlurEffect, MID_EFFECT_JSON for the JSON
    of model::effect::convertToJSON, and MID_BLUR_ENABLED.
 */
class SVXCORE_DLLPUBLIC SdrBlurEffectItem final : public SfxPoolItem
{
    bool mbEnabled = false;
    model::BlurEffect maEffect;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrBlurEffectItem)
    explicit SdrBlurEffectItem(TypedWhichId<SdrBlurEffectItem> nWhich);
    static SfxPoolItem* CreateDefault();

    bool isEnabled() const { return mbEnabled; }
    void setEnabled(bool bEnabled) { mbEnabled = bEnabled; }
    model::BlurEffect const& getEffect() const { return maEffect; }
    void setEffect(model::BlurEffect const& rEffect) { maEffect = rEffect; }

    bool operator==(const SfxPoolItem& rItem) const override;
    SdrBlurEffectItem* Clone(SfxItemPool* pPool = nullptr) const override;
    bool QueryValue(cpo::uno::Any& rValue, sal_uInt8 nMemberId = 0) const override;
    bool PutValue(const cpo::uno::Any& rValue, sal_uInt8 nMemberId) override;
    boost::property_tree::ptree dumpAsJSON() const override;
};

/** The inner shadow of a shape, and whether the shape has it. Nothing draws the inner shadow yet,
    so the item keeps it for the round trip of the document.

    The members of the item for UNO are MID_EFFECT for the XInnerShadowEffect, MID_EFFECT_JSON for
    the JSON of model::effect::convertToJSON, and MID_INNER_SHADOW_ENABLED.
 */
class SVXCORE_DLLPUBLIC SdrInnerShadowEffectItem final : public SfxPoolItem
{
    bool mbEnabled = false;
    model::InnerShadowEffect maEffect;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrInnerShadowEffectItem)
    explicit SdrInnerShadowEffectItem(TypedWhichId<SdrInnerShadowEffectItem> nWhich);
    static SfxPoolItem* CreateDefault();

    bool isEnabled() const { return mbEnabled; }
    void setEnabled(bool bEnabled) { mbEnabled = bEnabled; }
    model::InnerShadowEffect const& getEffect() const { return maEffect; }
    void setEffect(model::InnerShadowEffect const& rEffect) { maEffect = rEffect; }

    bool operator==(const SfxPoolItem& rItem) const override;
    SdrInnerShadowEffectItem* Clone(SfxItemPool* pPool = nullptr) const override;
    bool QueryValue(cpo::uno::Any& rValue, sal_uInt8 nMemberId = 0) const override;
    bool PutValue(const cpo::uno::Any& rValue, sal_uInt8 nMemberId) override;
    boost::property_tree::ptree dumpAsJSON() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
