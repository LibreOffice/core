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


#include <sdr/contact/viewcontactofsdrpathobj.hxx>
#include <svx/svdopath.hxx>
#include <sdr/primitive2d/sdrattributecreator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <sdr/primitive2d/sdrpathprimitive2d.hxx>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <osl/diagnose.h>
#include <vcl/canvastools.hxx>

namespace sdr::contact
{
        ViewContactOfSdrPathObj::ViewContactOfSdrPathObj(SdrPathObj& rPathObj)
        :   ViewContactOfTextObj(rPathObj)
        {
        }

        ViewContactOfSdrPathObj::~ViewContactOfSdrPathObj()
        {
        }

        /// return true if polycount == 1
        static bool ensureGeometry(basegfx::B2DPolyPolygon& rUnitPolyPolygon)
        {
            sal_uInt32 nPolyCount(rUnitPolyPolygon.count());
            sal_uInt32 nPointCount(0);

            for(auto const& rPolygon : std::as_const(rUnitPolyPolygon))
            {
                nPointCount += rPolygon.count();
                // return early if we definitely have geometry
                if (nPointCount > 1)
                    return nPolyCount == 1;
            }

            if(!nPointCount)
            {
                OSL_FAIL("PolyPolygon object without geometry detected, this should not be created (!)");
                basegfx::B2DPolygon aFallbackLine;
                aFallbackLine.append(basegfx::B2DPoint(0.0, 0.0));
                aFallbackLine.append(basegfx::B2DPoint(1000.0, 1000.0));
                rUnitPolyPolygon = basegfx::B2DPolyPolygon(aFallbackLine);

                nPolyCount = 1;
            }

            return nPolyCount == 1;
        }

        void ViewContactOfSdrPathObj::createViewIndependentPrimitive2DSequence(drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
        {
            const SfxItemSet& rItemSet = GetPathObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillEffectsTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillEffectsTextAttribute(
                    rItemSet,
                    GetPathObj().getText(0),
                    false));
            basegfx::B2DPolyPolygon aUnitPolyPolygon(GetPathObj().GetPathPoly());
            bool bPolyCountIsOne(ensureGeometry(aUnitPolyPolygon));

            // prepare object transformation and unit polygon (direct model data)
            basegfx::B2DHomMatrix aObjectMatrix;
            basegfx::B2DPolyPolygon aUnitDefinitionPolyPolygon;
            const bool bIsLine(
                !aUnitPolyPolygon.areControlPointsUsed()
                && bPolyCountIsOne
                && 2 == aUnitPolyPolygon.getB2DPolygon(0).count());

            if(bIsLine)
            {
                // special handling for single line mode (2 points)
                const basegfx::B2DPolygon & rSubPolygon(aUnitPolyPolygon.getB2DPolygon(0));
                const basegfx::B2DPoint aStart(rSubPolygon.getB2DPoint(0));
                const basegfx::B2DPoint aEnd(rSubPolygon.getB2DPoint(1));
                const basegfx::B2DVector aLine(aEnd - aStart);

                // #i102548# create new unit polygon for line (horizontal)
                static const basegfx::B2DPolygon aNewPolygon{basegfx::B2DPoint(0.0, 0.0), basegfx::B2DPoint(1.0, 0.0)};
                aUnitPolyPolygon.setB2DPolygon(0, aNewPolygon);

                // #i102548# fill objectMatrix with rotation and offset (no shear for lines)
                aObjectMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                    aLine.getLength(), 1.0,
                    0.0,
                    atan2(aLine.getY(), aLine.getX()),
                    aStart.getX(), aStart.getY());
            }
            else
            {
                // #i102548# create unscaled, unsheared, unrotated and untranslated polygon
                // (unit polygon) by creating the object matrix and back-transforming the polygon
                const basegfx::B2DRange aObjectRange(basegfx::utils::getRange(aUnitPolyPolygon));
                const GeoStat& rGeoStat(GetPathObj().GetGeoStat());
                const double fWidth(aObjectRange.getWidth());
                const double fHeight(aObjectRange.getHeight());
                const double fScaleX(basegfx::fTools::equalZero(fWidth) ? 1.0 : fWidth);
                const double fScaleY(basegfx::fTools::equalZero(fHeight) ? 1.0 : fHeight);

                aObjectMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                    fScaleX, fScaleY,
                    -rGeoStat.mfTanShearAngle,
                    rGeoStat.m_nRotationAngle ? toRadians(36000_deg100 - rGeoStat.m_nRotationAngle) : 0.0,
                    aObjectRange.getMinX(), aObjectRange.getMinY());

                // create unit polygon from object's absolute path
                basegfx::B2DHomMatrix aInverse(aObjectMatrix);
                aInverse.invert();
                aUnitPolyPolygon.transform(aInverse);

                // OperationSmiley: Check if a FillGeometryDefiningShape is set
                const SdrObject* pFillGeometryDefiningShape(GetPathObj().getFillGeometryDefiningShape());

                if(nullptr != pFillGeometryDefiningShape)
                {
                    // If yes, get it's BoundRange and use as defining Geometry for the FillStyle.
                    // If no, aUnitDefinitionPolyPolygon will just be empty and thus be interpreted
                    // as unused.
                    // Using SnapRect will make the FillDefinition to always be extended e.g.
                    // for rotated/sheared objects.
                    const tools::Rectangle& rSnapRect(pFillGeometryDefiningShape->GetSnapRect());

                    aUnitDefinitionPolyPolygon.append(
                        basegfx::utils::createPolygonFromRect(
                                vcl::unotools::b2DRectangleFromRectangle(rSnapRect)));

                    // use same coordinate system as the shape geometry -> this
                    // makes it relative to shape's unit geometry and thus freely
                    // transformable with the shape
                    aUnitDefinitionPolyPolygon.transform(aInverse);
                }
            }

            // create primitive. Always create primitives to allow the decomposition of
            // SdrPathPrimitive2D to create needed invisible elements for HitTest and/or BoundRect
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::SdrPathPrimitive2D(
                    aObjectMatrix,
                    aAttribute,
                    std::move(aUnitPolyPolygon),
                    std::move(aUnitDefinitionPolyPolygon)));

#ifdef DBG_UTIL
            // helper to create something that uses InvertPrimitive2D to be able
            // to check/debug implementations in SDPRs. There is not much left
            // doing InvertPrimitive2D nowadays (luckily). To use, create a polygon
            // using one of the polygon tools.
            static bool bTestInvert(false);
            if (bTestInvert)
            {
                drawinglayer::primitive2d::Primitive2DContainer aContainer;
                aContainer.push_back(xReference);
                const drawinglayer::primitive2d::Primitive2DReference xReference2(
                    new drawinglayer::primitive2d::InvertPrimitive2D(std::move(aContainer)));
                rVisitor.visit(xReference2);
                return;
            }
#endif
            rVisitor.visit(xReference);
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
