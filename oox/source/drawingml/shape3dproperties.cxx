/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawingml/shape3dproperties.hxx>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <oox/token/tokens.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;


namespace oox::drawingml {

OUString Generic3DProperties::getCameraPrstName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_legacyObliqueTopLeft:          return "legacyObliqueTopLeft";
        case XML_legacyObliqueTop:              return "legacyObliqueTop";
        case XML_legacyObliqueTopRight:         return "legacyObliqueTopRight";
        case XML_legacyObliqueLeft:             return "legacyObliqueLeft";
        case XML_legacyObliqueFront:            return "legacyObliqueFront";
        case XML_legacyObliqueRight:            return "legacyObliqueRight";
        case XML_legacyObliqueBottomLeft:       return "legacyObliqueBottomLeft";
        case XML_legacyObliqueBottom:           return "legacyObliqueBottom";
        case XML_legacyObliqueBottomRight:      return "legacyObliqueBottomRight";
        case XML_legacyPerspectiveTopLeft:      return "legacyPerspectiveTopLeft";
        case XML_legacyPerspectiveTop:          return "legacyPerspectiveTop";
        case XML_legacyPerspectiveTopRight:     return "legacyPerspectiveTopRight";
        case XML_legacyPerspectiveLeft:         return "legacyPerspectiveLeft";
        case XML_legacyPerspectiveFront:        return "legacyPerspectiveFront";
        case XML_legacyPerspectiveRight:        return "legacyPerspectiveRight";
        case XML_legacyPerspectiveBottomLeft:   return "legacyPerspectiveBottomLeft";
        case XML_legacyPerspectiveBottom:       return "legacyPerspectiveBottom";
        case XML_legacyPerspectiveBottomRight:  return "legacyPerspectiveBottomRight";
        case XML_orthographicFront:             return "orthographicFront";
        case XML_isometricTopUp:                return "isometricTopUp";
        case XML_isometricTopDown:              return "isometricTopDown";
        case XML_isometricBottomUp:             return "isometricBottomUp";
        case XML_isometricBottomDown:           return "isometricBottomDown";
        case XML_isometricLeftUp:               return "isometricLeftUp";
        case XML_isometricLeftDown:             return "isometricLeftDown";
        case XML_isometricRightUp:              return "isometricRightUp";
        case XML_isometricRightDown:            return "isometricRightDown";
        case XML_isometricOffAxis1Left:         return "isometricOffAxis1Left";
        case XML_isometricOffAxis1Right:        return "isometricOffAxis1Right";
        case XML_isometricOffAxis1Top:          return "isometricOffAxis1Top";
        case XML_isometricOffAxis2Left:         return "isometricOffAxis2Left";
        case XML_isometricOffAxis2Right:        return "isometricOffAxis2Right";
        case XML_isometricOffAxis2Top:          return "isometricOffAxis2Top";
        case XML_isometricOffAxis3Left:         return "isometricOffAxis3Left";
        case XML_isometricOffAxis3Right:        return "isometricOffAxis3Right";
        case XML_isometricOffAxis3Bottom:       return "isometricOffAxis3Bottom";
        case XML_isometricOffAxis4Left:         return "isometricOffAxis4Left";
        case XML_isometricOffAxis4Right:        return "isometricOffAxis4Right";
        case XML_isometricOffAxis4Bottom:       return "isometricOffAxis4Bottom";
        case XML_obliqueTopLeft:                return "obliqueTopLeft";
        case XML_obliqueTop:                    return "obliqueTop";
        case XML_obliqueTopRight:               return "obliqueTopRight";
        case XML_obliqueLeft:                   return "obliqueLeft";
        case XML_obliqueRight:                  return "obliqueRight";
        case XML_obliqueBottomLeft:             return "obliqueBottomLeft";
        case XML_obliqueBottom:                 return "obliqueBottom";
        case XML_obliqueBottomRight:            return "obliqueBottomRight";
        case XML_perspectiveFront:              return "perspectiveFront";
        case XML_perspectiveLeft:               return "perspectiveLeft";
        case XML_perspectiveRight:              return "perspectiveRight";
        case XML_perspectiveAbove:              return "perspectiveAbove";
        case XML_perspectiveBelow:              return "perspectiveBelow";
        case XML_perspectiveAboveLeftFacing:        return "perspectiveAboveLeftFacing";
        case XML_perspectiveAboveRightFacing:       return "perspectiveAboveRightFacing";
        case XML_perspectiveContrastingLeftFacing:  return "perspectiveContrastingLeftFacing";
        case XML_perspectiveContrastingRightFacing: return "perspectiveContrastingRightFacing";
        case XML_perspectiveHeroicLeftFacing:       return "perspectiveHeroicLeftFacing";
        case XML_perspectiveHeroicRightFacing:      return "perspectiveHeroicRightFacing";
        case XML_perspectiveHeroicExtremeLeftFacing:    return "perspectiveHeroicExtremeLeftFacing";
        case XML_perspectiveHeroicExtremeRightFacing:   return "perspectiveHeroicExtremeRightFacing";
        case XML_perspectiveRelaxed:                    return "perspectiveRelaxed";
        case XML_perspectiveRelaxedModerately:          return "perspectiveRelaxedModerately";
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getCameraPrstName - unexpected prst type" );
    return OUString();
}

OUString Generic3DProperties::getLightRigName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_legacyFlat1:       return "legacyFlat1";
        case XML_legacyFlat2:       return "legacyFlat2";
        case XML_legacyFlat3:       return "legacyFlat3";
        case XML_legacyFlat4:       return "legacyFlat4";
        case XML_legacyNormal1:     return "legacyNormal1";
        case XML_legacyNormal2:     return "legacyNormal2";
        case XML_legacyNormal3:     return "legacyNormal3";
        case XML_legacyNormal4:     return "legacyNormal4";
        case XML_legacyHarsh1:      return "legacyHarsh1";
        case XML_legacyHarsh2:      return "legacyHarsh2";
        case XML_legacyHarsh3:      return "legacyHarsh3";
        case XML_legacyHarsh4:      return "legacyHarsh4";
        case XML_threePt:           return "threePt";
        case XML_balanced:          return "balanced";
        case XML_soft:              return "soft";
        case XML_harsh:             return "harsh";
        case XML_flood:             return "flood";
        case XML_contrasting:       return "contrasting";
        case XML_morning:           return "morning";
        case XML_sunrise:           return "sunrise";
        case XML_sunset:            return "sunset";
        case XML_chilly:            return "chilly";
        case XML_freezing:          return "freezing";
        case XML_flat:              return "flat";
        case XML_twoPt:             return "twoPt";
        case XML_glow:              return "glow";
        case XML_brightRoom:        return "brightRoom";
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getLightRigName - unexpected token" );
    return OUString();
}

OUString Generic3DProperties::getLightRigDirName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_tl:    return "tl";
        case XML_t:     return "t";
        case XML_tr:    return "tr";
        case XML_l:     return "l";
        case XML_r:     return "r";
        case XML_bl:    return "bl";
        case XML_b:     return "b";
        case XML_br:    return "br";
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getLightRigDirName - unexpected token" );
    return OUString();
}

OUString Generic3DProperties::getBevelPresetTypeString( sal_Int32 nType )
{
    switch (nType)
    {
        case XML_relaxedInset:  return "relaxedInset";
        case XML_circle:        return "circle";
        case XML_slope:         return "slope";
        case XML_cross:         return "cross";
        case XML_angle:         return "angle";
        case XML_softRound:     return "softRound";
        case XML_convex:        return "convex";
        case XML_coolSlant:     return "coolSlant";
        case XML_divot:         return "divot";
        case XML_riblet:        return "riblet";
        case XML_hardEdge:      return "hardEdge";
        case XML_artDeco:       return "artDeco";
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getBevelPresetTypeString - unexpected token" );
    return OUString();
}

OUString Generic3DProperties::getPresetMaterialTypeString( sal_Int32 nType )
{
    switch (nType)
    {
        case XML_legacyMatte:       return "legacyMatte";
        case XML_legacyPlastic:     return "legacyPlastic";
        case XML_legacyMetal:       return "legacyMetal";
        case XML_legacyWireframe:   return "legacyWireframe";
        case XML_matte:             return "matte";
        case XML_plastic:           return "plastic";
        case XML_metal:             return "metal";
        case XML_warmMatte:         return "warmMatte";
        case XML_translucentPowder: return "translucentPowder";
        case XML_powder:            return "powder";
        case XML_dkEdge:            return "dkEdge";
        case XML_softEdge:          return "softEdge";
        case XML_clear:             return "clear";
        case XML_flat:              return "flat";
        case XML_softmetal:         return "softmetal";
        case XML_none:              return "none";
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getPresetMaterialTypeString - unexpected token" );
    return OUString();
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getCameraAttributes()
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(6);
    sal_Int32 nSize = 0;
    if( mfFieldOfVision.has() )
    {
        aSeq[nSize].Name = "fov";
        aSeq[nSize].Value <<= mfFieldOfVision.use();
        nSize++;
    }
    if( mfZoom.has() )
    {
        aSeq[nSize].Name = "zoom";
        aSeq[nSize].Value <<= mfZoom.use();
        nSize++;
    }
    if( mnPreset.has() )
    {
        aSeq[nSize].Name = "prst";
        aSeq[nSize].Value <<= getCameraPrstName( mnPreset.use() );
        nSize++;
    }
    if( maCameraRotation.mnLatitude.has() )
    {
        aSeq[nSize].Name = "rotLat";
        aSeq[nSize].Value <<= maCameraRotation.mnLatitude.use();
        nSize++;
    }
    if( maCameraRotation.mnLongitude.has() )
    {
        aSeq[nSize].Name = "rotLon";
        aSeq[nSize].Value <<= maCameraRotation.mnLongitude.use();
        nSize++;
    }
    if( maCameraRotation.mnRevolution.has() )
    {
        aSeq[nSize].Name = "rotRev";
        aSeq[nSize].Value <<= maCameraRotation.mnRevolution.use();
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getLightRigAttributes()
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(5);
    sal_Int32 nSize = 0;
    if( mnLightRigDirection.has() )
    {
        aSeq[nSize].Name = "dir";
        aSeq[nSize].Value <<= getLightRigDirName( mnLightRigDirection.use() );
        nSize++;
    }
    if( mnLightRigType.has() )
    {
        aSeq[nSize].Name = "rig";
        aSeq[nSize].Value <<= getLightRigName( mnLightRigType.use() );
        nSize++;
    }
    if( maLightRigRotation.mnLatitude.has() )
    {
        aSeq[nSize].Name = "rotLat";
        aSeq[nSize].Value <<= maLightRigRotation.mnLatitude.use();
        nSize++;
    }
    if( maLightRigRotation.mnLongitude.has() )
    {
        aSeq[nSize].Name = "rotLon";
        aSeq[nSize].Value <<= maLightRigRotation.mnLongitude.use();
        nSize++;
    }
    if( maLightRigRotation.mnRevolution.has() )
    {
        aSeq[nSize].Name = "rotRev";
        aSeq[nSize].Value <<= maLightRigRotation.mnRevolution.use();
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getBevelAttributes( BevelProperties rProps )
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(3);
    sal_Int32 nSize = 0;
    if( rProps.mnPreset.has() )
    {
        aSeq[nSize].Name = "prst";
        aSeq[nSize].Value <<= getBevelPresetTypeString( rProps.mnPreset.use() );
        nSize++;
    }
    if( rProps.mnWidth.has() )
    {
        aSeq[nSize].Name = "w";
        aSeq[nSize].Value <<= rProps.mnWidth.use();
        nSize++;
    }
    if( rProps.mnHeight.has() )
    {
        aSeq[nSize].Name = "h";
        aSeq[nSize].Value <<= rProps.mnHeight.use();
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getColorAttributes(
        const Color& rColor, const GraphicHelper& rGraphicHelper, ::Color rPhClr )
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(2);
    const OUString& sColorScheme = rColor.getSchemeName();
    if( sColorScheme.isEmpty() )
    {
        // RGB color and transparency value
        aSeq[0].Name = "rgbClr";
        aSeq[0].Value <<= rColor.getColor( rGraphicHelper, rPhClr );
        aSeq[1].Name = "rgbClrTransparency";
        aSeq[1].Value <<= rColor.getTransparency();
    }
    else
    {
        // scheme color with name and transformations
        aSeq[0].Name = "schemeClr";
        aSeq[0].Value <<= sColorScheme;
        aSeq[1].Name = "schemeClrTransformations";
        aSeq[1].Value <<= rColor.getTransformations();
    }
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getShape3DAttributes(
        const GraphicHelper& rGraphicHelper, ::Color rPhClr )
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(8);
    sal_Int32 nSize = 0;
    if( mnExtrusionH.has() )
    {
        aSeq[nSize].Name = "extrusionH";
        aSeq[nSize].Value <<= mnExtrusionH.use();
        nSize++;
    }
    if( mnContourW.has() )
    {
        aSeq[nSize].Name = "contourW";
        aSeq[nSize].Value <<= mnContourW.use();
        nSize++;
    }
    if( mnShapeZ.has() )
    {
        aSeq[nSize].Name = "z";
        aSeq[nSize].Value <<= mnShapeZ.use();
        nSize++;
    }
    if( mnMaterial.has() )
    {
        aSeq[nSize].Name = "prstMaterial";
        aSeq[nSize].Value <<= getPresetMaterialTypeString( mnMaterial.use() );
        nSize++;
    }
    if( maTopBevelProperties.has() )
    {
        aSeq[nSize].Name = "bevelT";
        aSeq[nSize].Value <<= getBevelAttributes( maTopBevelProperties.use() );
        nSize++;
    }
    if( maBottomBevelProperties.has() )
    {
        aSeq[nSize].Name = "bevelB";
        aSeq[nSize].Value <<= getBevelAttributes( maBottomBevelProperties.use() );
        nSize++;
    }
    if( maExtrusionColor.isUsed() )
    {
        aSeq[nSize].Name = "extrusionClr";
        aSeq[nSize].Value <<= getColorAttributes( maExtrusionColor, rGraphicHelper, rPhClr );
        nSize++;
    }
    if( maContourColor.isUsed() )
    {
        aSeq[nSize].Name = "contourClr";
        aSeq[nSize].Value <<= getColorAttributes( maContourColor, rGraphicHelper, rPhClr );
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
