/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-baosic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oox/drawingml/effectproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/shapepropertymap.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/token/tokens.hxx"

#include <basegfx/numeric/ftools.hxx>

namespace oox {
namespace drawingml {

// ============================================================================

void EffectShadowProperties::assignUsed(const EffectShadowProperties& rSourceProps)
{
    moShadowDist.assignIfUsed( rSourceProps.moShadowDist );
    moShadowDir.assignIfUsed( rSourceProps.moShadowDir );
    moShadowColor.assignIfUsed( rSourceProps.moShadowColor );
}

void EffectProperties::assignUsed( const EffectProperties& rSourceProps )
{
    maShadow.assignUsed(rSourceProps.maShadow);
}

void EffectProperties::pushToPropMap( PropertyMap& rPropMap,
        const GraphicHelper& rGraphicHelper ) const
{
    if (maShadow.moShadowDist.has())
    {
        // Negative X or Y dist indicates left or up, respectively
        double nAngle = (maShadow.moShadowDir.get(0) / PER_DEGREE) * F_PI180;
        sal_Int32 nDist = convertEmuToHmm(maShadow.moShadowDist.get(0));
        sal_Int32 nXDist = cos(nAngle) * nDist;
        sal_Int32 nYDist = sin(nAngle) * nDist;

        rPropMap.setProperty( PROP_Shadow, sal_True );
        rPropMap.setProperty( PROP_ShadowXDistance, nXDist);
        rPropMap.setProperty( PROP_ShadowYDistance, nYDist);
        rPropMap.setProperty( PROP_ShadowColor, maShadow.moShadowColor.getColor(rGraphicHelper, -1 ) );
        rPropMap.setProperty( PROP_ShadowTransparence, maShadow.moShadowColor.getTransparency());
    }
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
