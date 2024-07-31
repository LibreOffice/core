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

#include <overlayobject.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <drawinglayer/primitive2d/PolyPolygonMarkerPrimitive2D.hxx>
#include <officecfg/Office/Common.hxx>
#include <vcl/settings.hxx>

using sdr::overlay::OverlayObject;
using sdr::overlay::OverlayManager;

#define DASH_UPDATE_INTERVAL 500    // in msec

ScOverlayDashedBorder::ScOverlayDashedBorder(const ::basegfx::B2DRange& rRange, const Color& rColor) :
    OverlayObject(rColor),
    mbToggle(true)
{
    // tdf#115688: Let the user choose in the accessibility option page ("Tools" --> "Options" --> "Accessibility --> "Allow other animations") if the "marching ants" animation is allowed.
    // tdf#161765: Don't override LO's animation settings with OS's all-or-nothing animation setting,
    // but do respect OS's animation setting if the user has selected the option "System".
    // New options: "System"/"No"/"Yes"
    mbAllowsAnimation = MiscSettings::IsAnimatedOthersAllowed();
    maRange = rRange;
}

ScOverlayDashedBorder::~ScOverlayDashedBorder()
{
}

void ScOverlayDashedBorder::Trigger(sal_uInt32 nTime)
{
    OverlayManager* pMgr = getOverlayManager();
    if (pMgr)
    {
        SetTime(nTime + DASH_UPDATE_INTERVAL);
        mbToggle = !mbToggle;
        pMgr->InsertEvent(*this);
        objectChange();
    }
}

void ScOverlayDashedBorder::stripeDefinitionHasChanged()
{
    objectChange();
}

drawinglayer::primitive2d::Primitive2DContainer ScOverlayDashedBorder::createOverlayObjectPrimitive2DSequence()
{
    using ::basegfx::B2DPolygon;
    using ::basegfx::B2DPolyPolygon;

    OverlayManager* pMgr = getOverlayManager();
    if (!pMgr)
        return drawinglayer::primitive2d::Primitive2DContainer();

    basegfx::BColor aColorA = pMgr->getStripeColorA().getBColor();
    basegfx::BColor aColorB = pMgr->getStripeColorB().getBColor();
    if (!mbToggle)
        ::std::swap(aColorA, aColorB);

    const basegfx::B2DPolygon aPoly = basegfx::utils::createPolygonFromRect(maRange);
    B2DPolyPolygon aPolygon(aPoly);
    const drawinglayer::primitive2d::Primitive2DReference aReference(
        new drawinglayer::primitive2d::PolyPolygonMarkerPrimitive2D(
            std::move(aPolygon), aColorA, aColorB, pMgr->getStripeLengthPixel()));

    return drawinglayer::primitive2d::Primitive2DContainer { aReference };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
