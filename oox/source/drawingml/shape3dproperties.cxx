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

#include "oox/drawingml/shape3dproperties.hxx"
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

css::uno::Sequence< css::beans::PropertyValue > Shape3DProperties::getCameraAttributes()
{
    css::uno::Sequence<css::beans::PropertyValue> aSeq(3);
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
    aSeq.realloc( nSize );
    return aSeq;
}



} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
