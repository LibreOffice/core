/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OOX_DRAWINGML_EFFECTPROPERTIES_HXX
#define OOX_DRAWINGML_EFFECTPROPERTIES_HXX

#include "oox/drawingml/fillproperties.hxx"

namespace oox {
namespace drawingml {

// ============================================================================

struct EffectShadowProperties
{
    OptValue< sal_Int64 > moShadowDist;
    OptValue< sal_Int64 > moShadowDir;
    OptValue< sal_Int64 > moShadowAlpha;
    Color moShadowColor;

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const EffectShadowProperties& rSourceProps );
};

// ============================================================================

struct OOX_DLLPUBLIC EffectProperties
{
    EffectShadowProperties maShadow;

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const EffectProperties& rSourceProps );

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const GraphicHelper& rGraphicHelper ) const;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
