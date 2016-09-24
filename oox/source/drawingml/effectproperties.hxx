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

namespace oox {
namespace drawingml {

struct EffectShadowProperties
{
    OptValue< sal_Int64 > moShadowDist;
    OptValue< sal_Int64 > moShadowDir;
    Color moShadowColor;

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

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
