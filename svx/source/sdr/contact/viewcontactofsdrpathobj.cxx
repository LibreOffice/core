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
#include <svtools/optionsdrawinglayer.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <sdr/primitive2d/sdrattributecreator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <sdr/primitive2d/sdrpathprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <osl/diagnose.h>
#include <unotools/configmgr.hxx>
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

        static sal_uInt32 ensureGeometry(basegfx::B2DPolyPolygon& rUnitPolyPolygon)
        {
            sal_uInt32 nPolyCount(rUnitPolyPolygon.count());
            sal_uInt32 nPointCount(0);

            for(auto const& rPolygon : std::as_const(rUnitPolyPolygon))
            {
                nPointCount += rPolygon.count();
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

            return nPolyCount;
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
            sal_uInt32 nPolyCount(ensureGeometry(aUnitPolyPolygon));

            // prepare object transformation and unit polygon (direct model data)
            basegfx::B2DHomMatrix aObjectMatrix;
            basegfx::B2DPolyPolygon aUnitDefinitionPolyPolygon;
            bool bIsLine(
                !aUnitPolyPolygon.areControlPointsUsed()
                && 1 == nPolyCount
                && 2 == aUnitPolyPolygon.getB2DPolygon(0).count());

            if(bIsLine)
            {
                //tdf#63955 if we have an extremely long line then clip it to a
                //very generous range of -1 page width/height vs +1 page
                //width/height to avoid oom and massive churn generating a huge
                //polygon chain to cover the length in applyLineDashing if this
                //line is dashed
                const SdrPage* pPage(GetPathObj().getSdrPageFromSdrObject());
                sal_Int32 nPageWidth = pPage ? pPage->GetWidth() : 0;
                sal_Int32 nPageHeight = pPage ? pPage->GetHeight() : 0;

                //But, see tdf#101187, only do this if our generous clip region
                //would not over flow into a tiny clip region
                if (nPageWidth < SAL_MAX_INT32/2 && nPageHeight < SAL_MAX_INT32/2 && !utl::ConfigManager::IsFuzzing())
                {
                    //But, see tdf#97276, tdf#126184 and tdf#98366. Don't clip too much if the
                    //underlying page dimension is unknown or a paste document
                    //where the page sizes use the odd default of 10x10
                    const sal_Int32 nMaxPaperWidth = SvtOptionsDrawinglayer::GetMaximumPaperWidth() * 1000;
                    const sal_Int32 nMaxPaperHeight = SvtOptionsDrawinglayer::GetMaximumPaperHeight() * 1000;
                    nPageWidth = std::max<sal_Int32>(nPageWidth, nMaxPaperWidth);
                    nPageHeight = std::max<sal_Int32>(nPageHeight, nMaxPaperHeight);
                    basegfx::B2DRange aClipRange(-nPageWidth, -nPageHeight,
                                                 nPageWidth*2, nPageHeight*2);

                    aUnitPolyPolygon = basegfx::utils::clipPolyPolygonOnRange(aUnitPolyPolygon,
                                                                       aClipRange, true, true);
                    nPolyCount = ensureGeometry(aUnitPolyPolygon);

                    // re-check that we have't been clipped out to oblivion
                    bIsLine =
                        !aUnitPolyPolygon.areControlPointsUsed()
                        && 1 == nPolyCount
                        && 2 == aUnitPolyPolygon.getB2DPolygon(0).count();
                }
            }

            if(bIsLine)
            {

                // special handling for single line mode (2 points)
                const basegfx::B2DPolygon aSubPolygon(aUnitPolyPolygon.getB2DPolygon(0));
                const basegfx::B2DPoint aStart(aSubPolygon.getB2DPoint(0));
                const basegfx::B2DPoint aEnd(aSubPolygon.getB2DPoint(1));
                const basegfx::B2DVector aLine(aEnd - aStart);

                // #i102548# create new unit polygon for line (horizontal)
                basegfx::B2DPolygon aNewPolygon;
                aNewPolygon.append(basegfx::B2DPoint(0.0, 0.0));
                aNewPolygon.append(basegfx::B2DPoint(1.0, 0.0));
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
                    rGeoStat.nRotationAngle ? toRadians(36000_deg100 - rGeoStat.nRotationAngle) : 0.0,
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
                    aUnitPolyPolygon,
                    aUnitDefinitionPolyPolygon));

            rVisitor.visit(xReference);
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
