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

#include "drawingml/shape3dproperties.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/tokens.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {

OUString Shape3DProperties::getCameraPrstName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_legacyObliqueTopLeft:          return OUString( "legacyObliqueTopLeft" );
        case XML_legacyObliqueTop:              return OUString( "legacyObliqueTop" );
        case XML_legacyObliqueTopRight:         return OUString( "legacyObliqueTopRight" );
        case XML_legacyObliqueLeft:             return OUString( "legacyObliqueLeft" );
        case XML_legacyObliqueFront:            return OUString( "legacyObliqueFront" );
        case XML_legacyObliqueRight:            return OUString( "legacyObliqueRight" );
        case XML_legacyObliqueBottomLeft:       return OUString( "legacyObliqueBottomLeft" );
        case XML_legacyObliqueBottom:           return OUString( "legacyObliqueBottom" );
        case XML_legacyObliqueBottomRight:      return OUString( "legacyObliqueBottomRight" );
        case XML_legacyPerspectiveTopLeft:      return OUString( "legacyPerspectiveTopLeft" );
        case XML_legacyPerspectiveTop:          return OUString( "legacyPerspectiveTop" );
        case XML_legacyPerspectiveTopRight:     return OUString( "legacyPerspectiveTopRight" );
        case XML_legacyPerspectiveLeft:         return OUString( "legacyPerspectiveLeft" );
        case XML_legacyPerspectiveFront:        return OUString( "legacyPerspectiveFront" );
        case XML_legacyPerspectiveRight:        return OUString( "legacyPerspectiveRight" );
        case XML_legacyPerspectiveBottomLeft:   return OUString( "legacyPerspectiveBottomLeft" );
        case XML_legacyPerspectiveBottom:       return OUString( "legacyPerspectiveBottom" );
        case XML_legacyPerspectiveBottomRight:  return OUString( "legacyPerspectiveBottomRight" );
        case XML_orthographicFront:             return OUString( "orthographicFront" );
        case XML_isometricTopUp:                return OUString( "isometricTopUp" );
        case XML_isometricTopDown:              return OUString( "isometricTopDown" );
        case XML_isometricBottomUp:             return OUString( "isometricBottomUp" );
        case XML_isometricBottomDown:           return OUString( "isometricBottomDown" );
        case XML_isometricLeftUp:               return OUString( "isometricLeftUp" );
        case XML_isometricLeftDown:             return OUString( "isometricLeftDown" );
        case XML_isometricRightUp:              return OUString( "isometricRightUp" );
        case XML_isometricRightDown:            return OUString( "isometricRightDown" );
        case XML_isometricOffAxis1Left:         return OUString( "isometricOffAxis1Left" );
        case XML_isometricOffAxis1Right:        return OUString( "isometricOffAxis1Right" );
        case XML_isometricOffAxis1Top:          return OUString( "isometricOffAxis1Top" );
        case XML_isometricOffAxis2Left:         return OUString( "isometricOffAxis2Left" );
        case XML_isometricOffAxis2Right:        return OUString( "isometricOffAxis2Right" );
        case XML_isometricOffAxis2Top:          return OUString( "isometricOffAxis2Top" );
        case XML_isometricOffAxis3Left:         return OUString( "isometricOffAxis3Left" );
        case XML_isometricOffAxis3Right:        return OUString( "isometricOffAxis3Right" );
        case XML_isometricOffAxis3Bottom:       return OUString( "isometricOffAxis3Bottom" );
        case XML_isometricOffAxis4Left:         return OUString( "isometricOffAxis4Left" );
        case XML_isometricOffAxis4Right:        return OUString( "isometricOffAxis4Right" );
        case XML_isometricOffAxis4Bottom:       return OUString( "isometricOffAxis4Bottom" );
        case XML_obliqueTopLeft:                return OUString( "obliqueTopLeft" );
        case XML_obliqueTop:                    return OUString( "obliqueTop" );
        case XML_obliqueTopRight:               return OUString( "obliqueTopRight" );
        case XML_obliqueLeft:                   return OUString( "obliqueLeft" );
        case XML_obliqueRight:                  return OUString( "obliqueRight" );
        case XML_obliqueBottomLeft:             return OUString( "obliqueBottomLeft" );
        case XML_obliqueBottom:                 return OUString( "obliqueBottom" );
        case XML_obliqueBottomRight:            return OUString( "obliqueBottomRight" );
        case XML_perspectiveFront:              return OUString( "perspectiveFront" );
        case XML_perspectiveLeft:               return OUString( "perspectiveLeft" );
        case XML_perspectiveRight:              return OUString( "perspectiveRight" );
        case XML_perspectiveAbove:              return OUString( "perspectiveAbove" );
        case XML_perspectiveBelow:              return OUString( "perspectiveBelow" );
        case XML_perspectiveAboveLeftFacing:        return OUString( "perspectiveAboveLeftFacing" );
        case XML_perspectiveAboveRightFacing:       return OUString( "perspectiveAboveRightFacing" );
        case XML_perspectiveContrastingLeftFacing:  return OUString( "perspectiveContrastingLeftFacing" );
        case XML_perspectiveContrastingRightFacing: return OUString( "perspectiveContrastingRightFacing" );
        case XML_perspectiveHeroicLeftFacing:       return OUString( "perspectiveHeroicLeftFacing" );
        case XML_perspectiveHeroicRightFacing:      return OUString( "perspectiveHeroicRightFacing" );
        case XML_perspectiveHeroicExtremeLeftFacing:    return OUString( "perspectiveHeroicExtremeLeftFacing" );
        case XML_perspectiveHeroicExtremeRightFacing:   return OUString( "perspectiveHeroicExtremeRightFacing" );
        case XML_perspectiveRelaxed:                    return OUString( "perspectiveRelaxed" );
        case XML_perspectiveRelaxedModerately:          return OUString( "perspectiveRelaxedModerately" );
    }
    SAL_WARN( "oox.drawingml", "Shape3DProperties::getCameraPrstName - unexpected prst type" );
    return OUString();
}

OUString Shape3DProperties::getLightRigName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_legacyFlat1:       return OUString( "legacyFlat1" );
        case XML_legacyFlat2:       return OUString( "legacyFlat2" );
        case XML_legacyFlat3:       return OUString( "legacyFlat3" );
        case XML_legacyFlat4:       return OUString( "legacyFlat4" );
        case XML_legacyNormal1:     return OUString( "legacyNormal1" );
        case XML_legacyNormal2:     return OUString( "legacyNormal2" );
        case XML_legacyNormal3:     return OUString( "legacyNormal3" );
        case XML_legacyNormal4:     return OUString( "legacyNormal4" );
        case XML_legacyHarsh1:      return OUString( "legacyHarsh1" );
        case XML_legacyHarsh2:      return OUString( "legacyHarsh2" );
        case XML_legacyHarsh3:      return OUString( "legacyHarsh3" );
        case XML_legacyHarsh4:      return OUString( "legacyHarsh4" );
        case XML_threePt:           return OUString( "threePt" );
        case XML_balanced:          return OUString( "balanced" );
        case XML_soft:              return OUString( "soft" );
        case XML_harsh:             return OUString( "harsh" );
        case XML_flood:             return OUString( "flood" );
        case XML_contrasting:       return OUString( "contrasting" );
        case XML_morning:           return OUString( "morning" );
        case XML_sunrise:           return OUString( "sunrise" );
        case XML_sunset:            return OUString( "sunset" );
        case XML_chilly:            return OUString( "chilly" );
        case XML_freezing:          return OUString( "freezing" );
        case XML_flat:              return OUString( "flat" );
        case XML_twoPt:             return OUString( "twoPt" );
        case XML_glow:              return OUString( "glow" );
        case XML_brightRoom:        return OUString( "brightRoom" );
    }
    SAL_WARN( "oox.drawingml", "Shape3DProperties::getLightRigName - unexpected token" );
    return OUString();
}

OUString Shape3DProperties::getLightRigDirName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_tl:    return OUString( "tl" );
        case XML_t:     return OUString( "t" );
        case XML_tr:    return OUString( "tr" );
        case XML_l:     return OUString( "l" );
        case XML_r:     return OUString( "r" );
        case XML_bl:    return OUString( "bl" );
        case XML_b:     return OUString( "b" );
        case XML_br:    return OUString( "br" );
    }
    SAL_WARN( "oox.drawingml", "Shape3DProperties::getLightRigDirName - unexpected token" );
    return OUString();
}

OUString Shape3DProperties::getBevelPresetTypeString( sal_Int32 nType )
{
    switch (nType)
    {
        case XML_relaxedInset:  return OUString("relaxedInset");
        case XML_circle:        return OUString("circle");
        case XML_slope:         return OUString("slope");
        case XML_cross:         return OUString("cross");
        case XML_angle:         return OUString("angle");
        case XML_softRound:     return OUString("softRound");
        case XML_convex:        return OUString("convex");
        case XML_coolSlant:     return OUString("coolSlant");
        case XML_divot:         return OUString("divot");
        case XML_riblet:        return OUString("riblet");
        case XML_hardEdge:      return OUString("hardEdge");
        case XML_artDeco:       return OUString("artDeco");
    }
    SAL_WARN( "oox.drawingml", "Shape3DProperties::getBevelPresetTypeString - unexpected token" );
    return OUString();
}

OUString Shape3DProperties::getPresetMaterialTypeString( sal_Int32 nType )
{
    switch (nType)
    {
        case XML_legacyMatte:       return OUString("legacyMatte");
        case XML_legacyPlastic:     return OUString("legacyPlastic");
        case XML_legacyMetal:       return OUString("legacyMetal");
        case XML_legacyWireframe:   return OUString("legacyWireframe");
        case XML_matte:             return OUString("matte");
        case XML_plastic:           return OUString("plastic");
        case XML_metal:             return OUString("metal");
        case XML_warmMatte:         return OUString("warmMatte");
        case XML_translucentPowder: return OUString("translucentPowder");
        case XML_powder:            return OUString("powder");
        case XML_dkEdge:            return OUString("dkEdge");
        case XML_softEdge:          return OUString("softEdge");
        case XML_clear:             return OUString("clear");
        case XML_flat:              return OUString("flat");
        case XML_softmetal:         return OUString("softmetal");
        case XML_none:              return OUString("none");
    }
    SAL_WARN( "oox.drawingml", "Shape3DProperties::getPresetMaterialTypeString - unexpected token" );
    return OUString();
}

css::uno::Sequence< css::beans::PropertyValue > Shape3DProperties::getCameraAttributes()
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(6);
    sal_Int32 nSize = 0;
    if( mfFieldOfVision.has() )
    {
        aSeq[nSize].Name = "fov";
        aSeq[nSize].Value = css::uno::Any( mfFieldOfVision.use() );
        nSize++;
    }
    if( mfZoom.has() )
    {
        aSeq[nSize].Name = "zoom";
        aSeq[nSize].Value = css::uno::Any( mfZoom.use() );
        nSize++;
    }
    if( mnPreset.has() )
    {
        aSeq[nSize].Name = "prst";
        aSeq[nSize].Value = css::uno::Any( getCameraPrstName( mnPreset.use() ) );
        nSize++;
    }
    if( maCameraRotation.mnLatitude.has() )
    {
        aSeq[nSize].Name = "rotLat";
        aSeq[nSize].Value = css::uno::Any( maCameraRotation.mnLatitude.use() );
        nSize++;
    }
    if( maCameraRotation.mnLongitude.has() )
    {
        aSeq[nSize].Name = "rotLon";
        aSeq[nSize].Value = css::uno::Any( maCameraRotation.mnLongitude.use() );
        nSize++;
    }
    if( maCameraRotation.mnRevolution.has() )
    {
        aSeq[nSize].Name = "rotRev";
        aSeq[nSize].Value = css::uno::Any( maCameraRotation.mnRevolution.use() );
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Shape3DProperties::getLightRigAttributes()
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(5);
    sal_Int32 nSize = 0;
    if( mnLightRigDirection.has() )
    {
        aSeq[nSize].Name = "dir";
        aSeq[nSize].Value = css::uno::Any( getLightRigDirName( mnLightRigDirection.use() ) );
        nSize++;
    }
    if( mnLightRigType.has() )
    {
        aSeq[nSize].Name = "rig";
        aSeq[nSize].Value = css::uno::Any( getLightRigName( mnLightRigType.use() ) );
        nSize++;
    }
    if( maLightRigRotation.mnLatitude.has() )
    {
        aSeq[nSize].Name = "rotLat";
        aSeq[nSize].Value = css::uno::Any( maLightRigRotation.mnLatitude.use() );
        nSize++;
    }
    if( maLightRigRotation.mnLongitude.has() )
    {
        aSeq[nSize].Name = "rotLon";
        aSeq[nSize].Value = css::uno::Any( maLightRigRotation.mnLongitude.use() );
        nSize++;
    }
    if( maLightRigRotation.mnRevolution.has() )
    {
        aSeq[nSize].Name = "rotRev";
        aSeq[nSize].Value = css::uno::Any( maLightRigRotation.mnRevolution.use() );
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Shape3DProperties::getBevelAttributes( BevelProperties rProps )
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(3);
    sal_Int32 nSize = 0;
    if( rProps.mnPreset.has() )
    {
        aSeq[nSize].Name = "prst";
        aSeq[nSize].Value = css::uno::Any( getBevelPresetTypeString( rProps.mnPreset.use() ) );
        nSize++;
    }
    if( rProps.mnWidth.has() )
    {
        aSeq[nSize].Name = "w";
        aSeq[nSize].Value = css::uno::Any( rProps.mnWidth.use() );
        nSize++;
    }
    if( rProps.mnHeight.has() )
    {
        aSeq[nSize].Name = "h";
        aSeq[nSize].Value = css::uno::Any( rProps.mnHeight.use() );
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Shape3DProperties::getColorAttributes(
        const Color& rColor, const GraphicHelper& rGraphicHelper, sal_Int32 rPhClr )
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(2);
    OUString sColorScheme = rColor.getSchemeName();
    if( sColorScheme.isEmpty() )
    {
        // RGB color and transparency value
        aSeq[0].Name = "rgbClr";
        aSeq[0].Value = css::uno::Any( rColor.getColor( rGraphicHelper, rPhClr ) );
        aSeq[1].Name = "rgbClrTransparency";
        aSeq[1].Value = css::uno::Any( rColor.getTransparency() );
    }
    else
    {
        // scheme color with name and transformations
        aSeq[0].Name = "schemeClr";
        aSeq[0].Value = css::uno::Any( sColorScheme );
        aSeq[1].Name = "schemeClrTransformations";
        aSeq[1].Value = css::uno::Any( rColor.getTransformations() );
    }
    return aSeq;
}

css::uno::Sequence< css::beans::PropertyValue > Shape3DProperties::getShape3DAttributes(
        const GraphicHelper& rGraphicHelper, sal_Int32 rPhClr )
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(8);
    sal_Int32 nSize = 0;
    if( mnExtrusionH.has() )
    {
        aSeq[nSize].Name = "extrusionH";
        aSeq[nSize].Value = css::uno::Any( mnExtrusionH.use() );
        nSize++;
    }
    if( mnContourW.has() )
    {
        aSeq[nSize].Name = "contourW";
        aSeq[nSize].Value = css::uno::Any( mnContourW.use() );
        nSize++;
    }
    if( mnShapeZ.has() )
    {
        aSeq[nSize].Name = "z";
        aSeq[nSize].Value = css::uno::Any( mnShapeZ.use() );
        nSize++;
    }
    if( mnMaterial.has() )
    {
        aSeq[nSize].Name = "prstMaterial";
        aSeq[nSize].Value = css::uno::Any( getPresetMaterialTypeString( mnMaterial.use() ) );
        nSize++;
    }
    if( maTopBevelProperties.has() )
    {
        aSeq[nSize].Name = "bevelT";
        aSeq[nSize].Value = css::uno::Any( getBevelAttributes( maTopBevelProperties.use() ) );
        nSize++;
    }
    if( maBottomBevelProperties.has() )
    {
        aSeq[nSize].Name = "bevelB";
        aSeq[nSize].Value = css::uno::Any( getBevelAttributes( maBottomBevelProperties.use() ) );
        nSize++;
    }
    if( maExtrusionColor.isUsed() )
    {
        aSeq[nSize].Name = "extrusionClr";
        aSeq[nSize].Value = css::uno::Any( getColorAttributes( maExtrusionColor, rGraphicHelper, rPhClr ) );
        nSize++;
    }
    if( maContourColor.isUsed() )
    {
        aSeq[nSize].Name = "contourClr";
        aSeq[nSize].Value = css::uno::Any( getColorAttributes( maContourColor, rGraphicHelper, rPhClr ) );
        nSize++;
    }
    aSeq.realloc( nSize );
    return aSeq;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
