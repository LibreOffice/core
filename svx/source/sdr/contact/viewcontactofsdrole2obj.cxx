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

#include <drawinglayer/primitive2d/Tools.hxx>
#include <sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/svdoole2.hxx>
#include <sdr/contact/viewobjectcontactofsdrole2obj.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sdr/primitive2d/sdrole2primitive2d.hxx>
#include <sdr/primitive2d/sdrattributecreator.hxx>
#include <vcl/canvastools.hxx>
#include <tools/debug.hxx>
#include <sdr/primitive2d/sdrolecontentprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <svx/charthelper.hxx>
#include <svtools/embedhlp.hxx>

namespace sdr::contact {

// Create an Object-Specific ViewObjectContact, set ViewContact and
// ObjectContact. Always needs to return something.
ViewObjectContact& ViewContactOfSdrOle2Obj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
{
    ViewObjectContact* pRetval = new ViewObjectContactOfSdrOle2Obj(rObjectContact, *this);
    DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

    return *pRetval;
}

ViewContactOfSdrOle2Obj::ViewContactOfSdrOle2Obj(SdrOle2Obj& rOle2Obj)
:   ViewContactOfSdrRectObj(rOle2Obj)
{
}

ViewContactOfSdrOle2Obj::~ViewContactOfSdrOle2Obj()
{
}

basegfx::B2DHomMatrix ViewContactOfSdrOle2Obj::createObjectTransform() const
{
    // take unrotated snap rect (direct model data) for position and size
    const tools::Rectangle aRectangle(GetOle2Obj().GetGeoRect());
    const basegfx::B2DRange aObjectRange = vcl::unotools::b2DRectangleFromRectangle(aRectangle);

    // create object matrix
    const GeoStat& rGeoStat(GetOle2Obj().GetGeoStat());
    const double fShearX(rGeoStat.nShearAngle ? tan((36000 - rGeoStat.nShearAngle) * F_PI18000) : 0.0);
    const double fRotate(rGeoStat.nRotationAngle ? (36000 - rGeoStat.nRotationAngle) * F_PI18000 : 0.0);

    return basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
        aObjectRange.getWidth(), aObjectRange.getHeight(),
        fShearX,
        fRotate,
        aObjectRange.getMinX(), aObjectRange.getMinY());
}

drawinglayer::primitive2d::Primitive2DContainer ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters() const
{
    // get object transformation
    const basegfx::B2DHomMatrix aObjectMatrix(createObjectTransform());

    // Prepare attribute settings, will be used soon anyways
    const SfxItemSet& rItemSet = GetOle2Obj().GetMergedItemSet();

    // this may be refined more granular; if no content, attributes may get simpler
    const drawinglayer::attribute::SdrLineFillEffectsTextAttribute aAttribute(
        drawinglayer::primitive2d::createNewSdrLineFillEffectsTextAttribute(
            rItemSet,
            GetOle2Obj().getText(0),
            true));
    drawinglayer::primitive2d::Primitive2DReference xContent;

    if(GetOle2Obj().IsChart())
    {
        // try to get chart primitives and chart range directly from xChartModel
        basegfx::B2DRange aChartContentRange;
        const drawinglayer::primitive2d::Primitive2DContainer aChartSequence(
            ChartHelper::tryToGetChartContentAsPrimitive2DSequence(
                GetOle2Obj().getXModel(),
                aChartContentRange));
        const double fWidth(aChartContentRange.getWidth());
        const double fHeight(aChartContentRange.getHeight());

        if(!aChartSequence.empty()
            && basegfx::fTools::more(fWidth, 0.0)
            && basegfx::fTools::more(fHeight, 0.0))
        {
            // create embedding transformation
            basegfx::B2DHomMatrix aEmbed(
                basegfx::utils::createTranslateB2DHomMatrix(
                    -aChartContentRange.getMinX(),
                    -aChartContentRange.getMinY()));

            aEmbed.scale(1.0 / fWidth, 1.0 / fHeight);
            aEmbed = aObjectMatrix * aEmbed;
            xContent = new drawinglayer::primitive2d::TransformPrimitive2D(
                aEmbed,
                aChartSequence);
        }
    }

    if(!xContent.is())
    {
        // #i102063# embed OLE content in an own primitive; this will be able to decompose accessing
        // the weak SdrOle2 reference and will also implement getB2DRange() for fast BoundRect
        // calculations without OLE Graphic access (which may trigger e.g. chart recalculation).
        // It will also take care of HighContrast and ScaleContent
        xContent = new drawinglayer::primitive2d::SdrOleContentPrimitive2D(
            GetOle2Obj(),
            aObjectMatrix,

            // #i104867# add GraphicVersion number to be able to check for
            // content change in the primitive later
            GetOle2Obj().getEmbeddedObjectRef().getGraphicVersion() );
    }

    // create primitive. Use Ole2 primitive here. Prepare attribute settings, will
    // be used soon anyways. Always create primitives to allow the decomposition of
    // SdrOle2Primitive2D to create needed invisible elements for HitTest and/or BoundRect
    const drawinglayer::primitive2d::Primitive2DReference xReference(
        new drawinglayer::primitive2d::SdrOle2Primitive2D(
            drawinglayer::primitive2d::Primitive2DContainer { xContent },
            aObjectMatrix,
            aAttribute));

    return drawinglayer::primitive2d::Primitive2DContainer { xReference };
}

basegfx::B2DRange ViewContactOfSdrOle2Obj::getRange( const drawinglayer::geometry::ViewInformation2D& rViewInfo2D ) const
{
    // this may be refined more granular; if no content, attributes may get simpler
    const drawinglayer::attribute::SdrLineFillEffectsTextAttribute aAttribute =
        drawinglayer::primitive2d::createNewSdrLineFillEffectsTextAttribute(
            GetOle2Obj().GetMergedItemSet(),
            GetOle2Obj().getText(0),
            true);

    basegfx::B2DHomMatrix aObjectMatrix = createObjectTransform();

    drawinglayer::primitive2d::Primitive2DReference xContent =
        new drawinglayer::primitive2d::SdrOleContentPrimitive2D(
            GetOle2Obj(),
            aObjectMatrix,
            GetOle2Obj().getEmbeddedObjectRef().getGraphicVersion());

    const drawinglayer::primitive2d::Primitive2DReference xReference(
        new drawinglayer::primitive2d::SdrOle2Primitive2D(
            drawinglayer::primitive2d::Primitive2DContainer { xContent },
            aObjectMatrix,
            aAttribute));

    return drawinglayer::primitive2d::getB2DRangeFromPrimitive2DReference(xReference, rViewInfo2D);
}

drawinglayer::primitive2d::Primitive2DContainer ViewContactOfSdrOle2Obj::createViewIndependentPrimitive2DSequence() const
{
    return createPrimitive2DSequenceWithParameters();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
