/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_DRAWINGML_EFFECTPROPERTIES_HXX
#define INCLUDED_OOX_DRAWINGML_EFFECTPROPERTIES_HXX

#include <oox/drawingml/color.hxx>
#include <oox/helper/propertymap.hxx>

#include <memory>
#include <vector>
#include <map>

namespace oox::drawingml {

struct EffectGlowProperties
{
    OptValue< sal_Int64 > moGlowRad; // size of glow effect
    Color moGlowColor;
    // TODO saturation and luminance missing

    void assignUsed( const EffectGlowProperties& rSourceProps );
};

struct EffectSoftEdgeProperties
{
    OptValue<sal_Int64> moRad; // size of effect

    void assignUsed(const EffectSoftEdgeProperties& rSourceProps);
};

struct EffectShadowProperties
{
    OptValue< sal_Int64 > moShadowDist;
    OptValue< sal_Int64 > moShadowDir;
    OptValue< sal_Int64 > moShadowSx;
    OptValue< sal_Int64 > moShadowSy;
    Color moShadowColor;
    OptValue< sal_Int64 > moShadowBlur; // size of blur effect

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const EffectShadowProperties& rSourceProps );
};

struct Effect
{
    OUString msName;
    std::map< OUString, css::uno::Any > maAttribs;
    Color moColor;

    css::beans::PropertyValue getEffect();
};

struct EffectProperties
{
    EffectShadowProperties maShadow;
    EffectGlowProperties maGlow;
    EffectSoftEdgeProperties maSoftEdge;

    /** Stores all effect properties, including those not supported by core yet */
    std::vector<std::unique_ptr<Effect>> m_Effects;

    EffectProperties() {}
    EffectProperties(EffectProperties const& rOther)
    {
        assignUsed(rOther);
    }

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const EffectProperties& rSourceProps );

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const GraphicHelper& rGraphicHelper ) const;
};

} // namespace oox::drawingml

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
