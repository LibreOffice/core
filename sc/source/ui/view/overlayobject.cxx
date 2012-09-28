/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "overlayobject.hxx"
#include "vcl/outdev.hxx"
#include "vcl/lineinfo.hxx"
#include "tools/fract.hxx"
#include "basegfx/range/b2drange.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"
#include "svx/sdr/overlay/overlaymanager.hxx"
#include "drawinglayer/primitive2d/baseprimitive2d.hxx"
#include "drawinglayer/primitive2d/polypolygonprimitive2d.hxx"

using ::sdr::overlay::OverlayObject;
using ::sdr::overlay::OverlayManager;
using ::drawinglayer::primitive2d::Primitive2DSequence;

#define DASH_UPDATE_INTERVAL 180    // in msec

ScOverlayDashedBorder::ScOverlayDashedBorder(const ::basegfx::B2DRange& rRange, const Color& rColor) :
    OverlayObject(rColor),
    mbToggle(true)
{
    mbAllowsAnimation = true;
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
        pMgr->InsertEvent(this);
        objectChange();
    }
}

void ScOverlayDashedBorder::stripeDefinitionHasChanged()
{
    objectChange();
}

Primitive2DSequence ScOverlayDashedBorder::createOverlayObjectPrimitive2DSequence()
{
    using ::basegfx::B2DPolygon;
    using ::basegfx::B2DPolyPolygon;

    OverlayManager* pMgr = getOverlayManager();
    if (!pMgr)
        return Primitive2DSequence();

    basegfx::BColor aColorA = pMgr->getStripeColorA().getBColor();
    basegfx::BColor aColorB = pMgr->getStripeColorB().getBColor();
    if (!mbToggle)
        ::std::swap(aColorA, aColorB);

    const basegfx::B2DPolygon aPoly = basegfx::tools::createPolygonFromRect(maRange);
    B2DPolyPolygon aPolygon(aPoly);
    const drawinglayer::primitive2d::Primitive2DReference aReference(
        new drawinglayer::primitive2d::PolyPolygonMarkerPrimitive2D(
            aPolygon, aColorA, aColorB, pMgr->getStripeLengthPixel()));

    return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
