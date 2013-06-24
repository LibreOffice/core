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

#include "oox/drawingml/scene3dcontext.hxx"
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"

using ::oox::core::ContextHandler;
using ::oox::core::XmlFilterBase;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

Scene3DContext::Scene3DContext( ContextHandler2Helper& rParent, Shape3DProperties& r3DProperties ) throw()
: ContextHandler2( rParent )
, mr3DProperties( r3DProperties )
{
}

ContextHandlerRef Scene3DContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_camera:
        mr3DProperties.mfFieldOfVision = rAttribs.getInteger( XML_fov, 0 ) / 36000000.0;
        mr3DProperties.mfZoom = rAttribs.getInteger( XML_zoom, 100000 ) / 100000.0;
        mr3DProperties.mnPreset = rAttribs.getToken( XML_prst, XML_none );

legacyObliqueTopLeft
legacyObliqueTop
legacyObliqueTopRight
legacyObliqueLeft
legacyObliqueFront
legacyObliqueRight
legacyObliqueBottomLeft
legacyObliqueBottom
legacyObliqueBottomRight
legacyPerspectiveTopLeft
legacyPerspectiveTop
legacyPerspectiveTopRight
legacyPerspectiveLeft
legacyPerspectiveFront
legacyPerspectiveRight
legacyPerspectiveBottomLeft
legacyPerspectiveBottom
legacyPerspectiveBottomRight
orthographicFront
isometricTopUp
isometricTopDown
isometricBottomUp
isometricBottomDown
isometricLeftUp
isometricLeftDown
isometricRightUp
isometricRightDown
isometricOffAxis1Left
isometricOffAxis1Right
isometricOffAxis1Top
isometricOffAxis2Left
isometricOffAxis2Right
isometricOffAxis2Top
isometricOffAxis3Left
isometricOffAxis3Right
isometricOffAxis3Bottom
isometricOffAxis4Left
isometricOffAxis4Right
isometricOffAxis4Bottom
obliqueTopLeft
obliqueTop
obliqueTopRight
obliqueLeft
obliqueRight
obliqueBottomLeft
obliqueBottom
obliqueBottomRight
perspectiveFront
perspectiveLeft
perspectiveRight
perspectiveAbove
perspectiveBelow
perspectiveAboveLeftFacing
perspectiveAboveRightFacing
perspectiveContrastingLeftFacing
perspectiveContrastingRightFacing
perspectiveHeroicLeftFacing
perspectiveHeroicRightFacing
perspectiveHeroicExtremeLeftFacing
perspectiveHeroicExtremeRightFacing
perspectiveRelaxed
perspectiveRelaxedModerately


        // TODO: nested element XML_rot
        break;
    case NMSP_DRAWINGML|XML_lightRig:
        mr3DProperties.mnLightRigDirection = rAttribs.getToken( XML_dir, XML_none );

XML_tl
XML_t
XML_tr
XML_l
XML_r
XML_bl
XML_b
XML_br


        mr3DProperties.mnLightRigType = rAttribs.getToken( XML_rig, XML_none );

XML_legacyFlat1
XML_legacyFlat2
XML_legacyFlat3
XML_legacyFlat4
XML_legacyNormal1
XML_legacyNormal2
XML_legacyNormal3
XML_legacyNormal4
XML_legacyHarsh1
XML_legacyHarsh2
XML_legacyHarsh3
XML_legacyHarsh4
XML_threePt
XML_balanced
XML_soft
XML_harsh
XML_flood
XML_contrasting
XML_morning
XML_sunrise
XML_sunset
XML_chilly
XML_freezing
XML_flat
XML_twoPt
XML_glow
XML_brightRoom

        // TODO: nested element XML_rot
        break;
    case NMSP_DRAWINGML|XML_backdrop:
    case NMSP_DRAWINGML|XML_extLst:
        return 0; // TODO: later (backdrop is not supported by core anyway)
    }
    return 0;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
