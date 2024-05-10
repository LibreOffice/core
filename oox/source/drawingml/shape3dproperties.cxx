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

#include <comphelper/propertyvalue.hxx>
#include <oox/token/tokens.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;


namespace oox::drawingml {

OUString Generic3DProperties::getCameraPrstName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_legacyObliqueTopLeft:          return u"legacyObliqueTopLeft"_ustr;
        case XML_legacyObliqueTop:              return u"legacyObliqueTop"_ustr;
        case XML_legacyObliqueTopRight:         return u"legacyObliqueTopRight"_ustr;
        case XML_legacyObliqueLeft:             return u"legacyObliqueLeft"_ustr;
        case XML_legacyObliqueFront:            return u"legacyObliqueFront"_ustr;
        case XML_legacyObliqueRight:            return u"legacyObliqueRight"_ustr;
        case XML_legacyObliqueBottomLeft:       return u"legacyObliqueBottomLeft"_ustr;
        case XML_legacyObliqueBottom:           return u"legacyObliqueBottom"_ustr;
        case XML_legacyObliqueBottomRight:      return u"legacyObliqueBottomRight"_ustr;
        case XML_legacyPerspectiveTopLeft:      return u"legacyPerspectiveTopLeft"_ustr;
        case XML_legacyPerspectiveTop:          return u"legacyPerspectiveTop"_ustr;
        case XML_legacyPerspectiveTopRight:     return u"legacyPerspectiveTopRight"_ustr;
        case XML_legacyPerspectiveLeft:         return u"legacyPerspectiveLeft"_ustr;
        case XML_legacyPerspectiveFront:        return u"legacyPerspectiveFront"_ustr;
        case XML_legacyPerspectiveRight:        return u"legacyPerspectiveRight"_ustr;
        case XML_legacyPerspectiveBottomLeft:   return u"legacyPerspectiveBottomLeft"_ustr;
        case XML_legacyPerspectiveBottom:       return u"legacyPerspectiveBottom"_ustr;
        case XML_legacyPerspectiveBottomRight:  return u"legacyPerspectiveBottomRight"_ustr;
        case XML_orthographicFront:             return u"orthographicFront"_ustr;
        case XML_isometricTopUp:                return u"isometricTopUp"_ustr;
        case XML_isometricTopDown:              return u"isometricTopDown"_ustr;
        case XML_isometricBottomUp:             return u"isometricBottomUp"_ustr;
        case XML_isometricBottomDown:           return u"isometricBottomDown"_ustr;
        case XML_isometricLeftUp:               return u"isometricLeftUp"_ustr;
        case XML_isometricLeftDown:             return u"isometricLeftDown"_ustr;
        case XML_isometricRightUp:              return u"isometricRightUp"_ustr;
        case XML_isometricRightDown:            return u"isometricRightDown"_ustr;
        case XML_isometricOffAxis1Left:         return u"isometricOffAxis1Left"_ustr;
        case XML_isometricOffAxis1Right:        return u"isometricOffAxis1Right"_ustr;
        case XML_isometricOffAxis1Top:          return u"isometricOffAxis1Top"_ustr;
        case XML_isometricOffAxis2Left:         return u"isometricOffAxis2Left"_ustr;
        case XML_isometricOffAxis2Right:        return u"isometricOffAxis2Right"_ustr;
        case XML_isometricOffAxis2Top:          return u"isometricOffAxis2Top"_ustr;
        case XML_isometricOffAxis3Left:         return u"isometricOffAxis3Left"_ustr;
        case XML_isometricOffAxis3Right:        return u"isometricOffAxis3Right"_ustr;
        case XML_isometricOffAxis3Bottom:       return u"isometricOffAxis3Bottom"_ustr;
        case XML_isometricOffAxis4Left:         return u"isometricOffAxis4Left"_ustr;
        case XML_isometricOffAxis4Right:        return u"isometricOffAxis4Right"_ustr;
        case XML_isometricOffAxis4Bottom:       return u"isometricOffAxis4Bottom"_ustr;
        case XML_obliqueTopLeft:                return u"obliqueTopLeft"_ustr;
        case XML_obliqueTop:                    return u"obliqueTop"_ustr;
        case XML_obliqueTopRight:               return u"obliqueTopRight"_ustr;
        case XML_obliqueLeft:                   return u"obliqueLeft"_ustr;
        case XML_obliqueRight:                  return u"obliqueRight"_ustr;
        case XML_obliqueBottomLeft:             return u"obliqueBottomLeft"_ustr;
        case XML_obliqueBottom:                 return u"obliqueBottom"_ustr;
        case XML_obliqueBottomRight:            return u"obliqueBottomRight"_ustr;
        case XML_perspectiveFront:              return u"perspectiveFront"_ustr;
        case XML_perspectiveLeft:               return u"perspectiveLeft"_ustr;
        case XML_perspectiveRight:              return u"perspectiveRight"_ustr;
        case XML_perspectiveAbove:              return u"perspectiveAbove"_ustr;
        case XML_perspectiveBelow:              return u"perspectiveBelow"_ustr;
        case XML_perspectiveAboveLeftFacing:        return u"perspectiveAboveLeftFacing"_ustr;
        case XML_perspectiveAboveRightFacing:       return u"perspectiveAboveRightFacing"_ustr;
        case XML_perspectiveContrastingLeftFacing:  return u"perspectiveContrastingLeftFacing"_ustr;
        case XML_perspectiveContrastingRightFacing: return u"perspectiveContrastingRightFacing"_ustr;
        case XML_perspectiveHeroicLeftFacing:       return u"perspectiveHeroicLeftFacing"_ustr;
        case XML_perspectiveHeroicRightFacing:      return u"perspectiveHeroicRightFacing"_ustr;
        case XML_perspectiveHeroicExtremeLeftFacing:    return u"perspectiveHeroicExtremeLeftFacing"_ustr;
        case XML_perspectiveHeroicExtremeRightFacing:   return u"perspectiveHeroicExtremeRightFacing"_ustr;
        case XML_perspectiveRelaxed:                    return u"perspectiveRelaxed"_ustr;
        case XML_perspectiveRelaxedModerately:          return u"perspectiveRelaxedModerately"_ustr;
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getCameraPrstName - unexpected prst type" );
    return OUString();
}

OUString Generic3DProperties::getLightRigName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_legacyFlat1:       return u"legacyFlat1"_ustr;
        case XML_legacyFlat2:       return u"legacyFlat2"_ustr;
        case XML_legacyFlat3:       return u"legacyFlat3"_ustr;
        case XML_legacyFlat4:       return u"legacyFlat4"_ustr;
        case XML_legacyNormal1:     return u"legacyNormal1"_ustr;
        case XML_legacyNormal2:     return u"legacyNormal2"_ustr;
        case XML_legacyNormal3:     return u"legacyNormal3"_ustr;
        case XML_legacyNormal4:     return u"legacyNormal4"_ustr;
        case XML_legacyHarsh1:      return u"legacyHarsh1"_ustr;
        case XML_legacyHarsh2:      return u"legacyHarsh2"_ustr;
        case XML_legacyHarsh3:      return u"legacyHarsh3"_ustr;
        case XML_legacyHarsh4:      return u"legacyHarsh4"_ustr;
        case XML_threePt:           return u"threePt"_ustr;
        case XML_balanced:          return u"balanced"_ustr;
        case XML_soft:              return u"soft"_ustr;
        case XML_harsh:             return u"harsh"_ustr;
        case XML_flood:             return u"flood"_ustr;
        case XML_contrasting:       return u"contrasting"_ustr;
        case XML_morning:           return u"morning"_ustr;
        case XML_sunrise:           return u"sunrise"_ustr;
        case XML_sunset:            return u"sunset"_ustr;
        case XML_chilly:            return u"chilly"_ustr;
        case XML_freezing:          return u"freezing"_ustr;
        case XML_flat:              return u"flat"_ustr;
        case XML_twoPt:             return u"twoPt"_ustr;
        case XML_glow:              return u"glow"_ustr;
        case XML_brightRoom:        return u"brightRoom"_ustr;
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getLightRigName - unexpected token" );
    return OUString();
}

OUString Generic3DProperties::getLightRigDirName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_tl:    return u"tl"_ustr;
        case XML_t:     return u"t"_ustr;
        case XML_tr:    return u"tr"_ustr;
        case XML_l:     return u"l"_ustr;
        case XML_r:     return u"r"_ustr;
        case XML_bl:    return u"bl"_ustr;
        case XML_b:     return u"b"_ustr;
        case XML_br:    return u"br"_ustr;
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getLightRigDirName - unexpected token" );
    return OUString();
}

OUString Generic3DProperties::getBevelPresetTypeString( sal_Int32 nType )
{
    switch (nType)
    {
        case XML_relaxedInset:  return u"relaxedInset"_ustr;
        case XML_circle:        return u"circle"_ustr;
        case XML_slope:         return u"slope"_ustr;
        case XML_cross:         return u"cross"_ustr;
        case XML_angle:         return u"angle"_ustr;
        case XML_softRound:     return u"softRound"_ustr;
        case XML_convex:        return u"convex"_ustr;
        case XML_coolSlant:     return u"coolSlant"_ustr;
        case XML_divot:         return u"divot"_ustr;
        case XML_riblet:        return u"riblet"_ustr;
        case XML_hardEdge:      return u"hardEdge"_ustr;
        case XML_artDeco:       return u"artDeco"_ustr;
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getBevelPresetTypeString - unexpected token" );
    return OUString();
}

OUString Generic3DProperties::getPresetMaterialTypeString( sal_Int32 nType )
{
    switch (nType)
    {
        case XML_legacyMatte:       return u"legacyMatte"_ustr;
        case XML_legacyPlastic:     return u"legacyPlastic"_ustr;
        case XML_legacyMetal:       return u"legacyMetal"_ustr;
        case XML_legacyWireframe:   return u"legacyWireframe"_ustr;
        case XML_matte:             return u"matte"_ustr;
        case XML_plastic:           return u"plastic"_ustr;
        case XML_metal:             return u"metal"_ustr;
        case XML_warmMatte:         return u"warmMatte"_ustr;
        case XML_translucentPowder: return u"translucentPowder"_ustr;
        case XML_powder:            return u"powder"_ustr;
        case XML_dkEdge:            return u"dkEdge"_ustr;
        case XML_softEdge:          return u"softEdge"_ustr;
        case XML_clear:             return u"clear"_ustr;
        case XML_flat:              return u"flat"_ustr;
        case XML_softmetal:         return u"softmetal"_ustr;
        case XML_none:              return u"none"_ustr;
    }
    SAL_WARN( "oox.drawingml", "Generic3DProperties::getPresetMaterialTypeString - unexpected token" );
    return OUString();
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getCameraAttributes()
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(6);
    auto pSeq = aSeq.getArray();
    sal_Int32 nSize = 0;
    if( mfFieldOfVision.has_value() )
    {
        pSeq[nSize].Name = "fov";
        pSeq[nSize].Value <<= *mfFieldOfVision;
        nSize++;
    }
    if( mfZoom.has_value() )
    {
        pSeq[nSize].Name = "zoom";
        pSeq[nSize].Value <<= *mfZoom;
        nSize++;
    }
    if( mnPreset.has_value() )
    {
        pSeq[nSize].Name = "prst";
        pSeq[nSize].Value <<= getCameraPrstName( *mnPreset );
        nSize++;
    }
    if( maCameraRotation.mnLatitude.has_value() )
    {
        pSeq[nSize].Name = "rotLat";
        pSeq[nSize].Value <<= *maCameraRotation.mnLatitude;
        nSize++;
    }
    if( maCameraRotation.mnLongitude.has_value() )
    {
        pSeq[nSize].Name = "rotLon";
        pSeq[nSize].Value <<= *maCameraRotation.mnLongitude;
        nSize++;
    }
    if( maCameraRotation.mnRevolution.has_value() )
    {
        pSeq[nSize].Name = "rotRev";
        pSeq[nSize].Value <<= *maCameraRotation.mnRevolution;
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getLightRigAttributes()
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(5);
    auto pSeq = aSeq.getArray();
    sal_Int32 nSize = 0;
    if( mnLightRigDirection.has_value() )
    {
        pSeq[nSize].Name = "dir";
        pSeq[nSize].Value <<= getLightRigDirName( *mnLightRigDirection );
        nSize++;
    }
    if( mnLightRigType.has_value() )
    {
        pSeq[nSize].Name = "rig";
        pSeq[nSize].Value <<= getLightRigName( *mnLightRigType );
        nSize++;
    }
    if( maLightRigRotation.mnLatitude.has_value() )
    {
        pSeq[nSize].Name = "rotLat";
        pSeq[nSize].Value <<= *maLightRigRotation.mnLatitude;
        nSize++;
    }
    if( maLightRigRotation.mnLongitude.has_value() )
    {
        pSeq[nSize].Name = "rotLon";
        pSeq[nSize].Value <<= *maLightRigRotation.mnLongitude;
        nSize++;
    }
    if( maLightRigRotation.mnRevolution.has_value() )
    {
        pSeq[nSize].Name = "rotRev";
        pSeq[nSize].Value <<= *maLightRigRotation.mnRevolution;
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getBevelAttributes( BevelProperties rProps )
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(3);
    auto pSeq = aSeq.getArray();
    sal_Int32 nSize = 0;
    if( rProps.mnPreset.has_value() )
    {
        pSeq[nSize].Name = "prst";
        pSeq[nSize].Value <<= getBevelPresetTypeString( *rProps.mnPreset );
        nSize++;
    }
    if( rProps.mnWidth.has_value() )
    {
        pSeq[nSize].Name = "w";
        pSeq[nSize].Value <<= *rProps.mnWidth;
        nSize++;
    }
    if( rProps.mnHeight.has_value() )
    {
        pSeq[nSize].Name = "h";
        pSeq[nSize].Value <<= *rProps.mnHeight;
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getColorAttributes(
        const Color& rColor, const GraphicHelper& rGraphicHelper, ::Color rPhClr )
{
    const OUString& sColorScheme = rColor.getSchemeColorName();
    if( sColorScheme.isEmpty() )
    {
        // RGB color and transparency value
        return { comphelper::makePropertyValue(u"rgbClr"_ustr, rColor.getColor( rGraphicHelper, rPhClr )),
                 comphelper::makePropertyValue(u"rgbClrTransparency"_ustr, rColor.getTransparency()) };
    }
    // scheme color with name and transformations
    return { comphelper::makePropertyValue(u"schemeClr"_ustr, sColorScheme),
             comphelper::makePropertyValue(u"schemeClrTransformations"_ustr,
                                           rColor.getTransformations()) };
}

css::uno::Sequence< css::beans::PropertyValue > Generic3DProperties::getShape3DAttributes(
        const GraphicHelper& rGraphicHelper, ::Color rPhClr )
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(8);
    auto pSeq = aSeq.getArray();
    sal_Int32 nSize = 0;
    if( mnExtrusionH.has_value() )
    {
        pSeq[nSize].Name = "extrusionH";
        pSeq[nSize].Value <<= *mnExtrusionH;
        nSize++;
    }
    if( mnContourW.has_value() )
    {
        pSeq[nSize].Name = "contourW";
        pSeq[nSize].Value <<= *mnContourW;
        nSize++;
    }
    if( mnShapeZ.has_value() )
    {
        pSeq[nSize].Name = "z";
        pSeq[nSize].Value <<= *mnShapeZ;
        nSize++;
    }
    if( mnMaterial.has_value() )
    {
        pSeq[nSize].Name = "prstMaterial";
        pSeq[nSize].Value <<= getPresetMaterialTypeString( *mnMaterial );
        nSize++;
    }
    if( maTopBevelProperties.has_value() )
    {
        pSeq[nSize].Name = "bevelT";
        pSeq[nSize].Value <<= getBevelAttributes( *maTopBevelProperties );
        nSize++;
    }
    if( maBottomBevelProperties.has_value() )
    {
        pSeq[nSize].Name = "bevelB";
        pSeq[nSize].Value <<= getBevelAttributes( *maBottomBevelProperties );
        nSize++;
    }
    if( maExtrusionColor.isUsed() )
    {
        pSeq[nSize].Name = "extrusionClr";
        pSeq[nSize].Value <<= getColorAttributes( maExtrusionColor, rGraphicHelper, rPhClr );
        nSize++;
    }
    if( maContourColor.isUsed() )
    {
        pSeq[nSize].Name = "contourClr";
        pSeq[nSize].Value <<= getColorAttributes( maContourColor, rGraphicHelper, rPhClr );
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
