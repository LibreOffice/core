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


#include <svx/sdr/contact/viewcontactofsdrpathobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/primitive2d/sdrpathprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>



namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrPathObj::ViewContactOfSdrPathObj(SdrPathObj& rPathObj)
        :   ViewContactOfTextObj(rPathObj)
        {
        }

        ViewContactOfSdrPathObj::~ViewContactOfSdrPathObj()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrPathObj::createViewIndependentPrimitive2DSequence() const
        {
            const SfxItemSet& rItemSet = GetPathObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    GetPathObj().getText(0),
                    false));
            basegfx::B2DPolyPolygon aUnitPolyPolygon(GetPathObj().GetPathPoly());
            Point aGridOff = GetPathObj().GetGridOffset();
            // Hack for calc, transform position of object according
            // to current zoom so as objects relative position to grid
            // appears stable
            aUnitPolyPolygon.transform( basegfx::tools::createTranslateB2DHomMatrix( aGridOff.X(), aGridOff.Y() ) );
            sal_uInt32 nPolyCount(aUnitPolyPolygon.count());
            sal_uInt32 nPointCount(0);

            for(sal_uInt32 a(0); a < nPolyCount; a++)
            {
                nPointCount += aUnitPolyPolygon.getB2DPolygon(a).count();
            }

            if(!nPointCount)
            {
                OSL_FAIL("PolyPolygon object without geometry detected, this should not be created (!)");
                basegfx::B2DPolygon aFallbackLine;
                aFallbackLine.append(basegfx::B2DPoint(0.0, 0.0));
                aFallbackLine.append(basegfx::B2DPoint(1000.0, 1000.0));
                aUnitPolyPolygon = basegfx::B2DPolyPolygon(aFallbackLine);

                nPolyCount = 1;
            }

            // prepare object transformation and unit polygon (direct model data)
            basegfx::B2DHomMatrix aObjectMatrix;
            const bool bIsLine(
                !aUnitPolyPolygon.areControlPointsUsed()
                && 1 == nPolyCount
                && 2 == aUnitPolyPolygon.getB2DPolygon(0).count());

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
                aObjectMatrix = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                    aLine.getLength(), 1.0,
                    0.0,
                    atan2(aLine.getY(), aLine.getX()),
                    aStart.getX(), aStart.getY());
            }
            else
            {
                // #i102548# create unscaled, unsheared, unrotated and untranslated polygon
                // (unit polygon) by creating the object matrix and back-transforming the polygon
                const basegfx::B2DRange aObjectRange(basegfx::tools::getRange(aUnitPolyPolygon));
                const GeoStat& rGeoStat(GetPathObj().GetGeoStat());
                const double fWidth(aObjectRange.getWidth());
                const double fHeight(aObjectRange.getHeight());
                const double fScaleX(basegfx::fTools::equalZero(fWidth) ? 1.0 : fWidth);
                const double fScaleY(basegfx::fTools::equalZero(fHeight) ? 1.0 : fHeight);

                aObjectMatrix = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                    fScaleX, fScaleY,
                    rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0,
                    rGeoStat.nDrehWink ? (36000 - rGeoStat.nDrehWink) * F_PI18000 : 0.0,
                    aObjectRange.getMinX(), aObjectRange.getMinY());

                // ceate unit polygon from object's absolute path
                basegfx::B2DHomMatrix aInverse(aObjectMatrix);
                aInverse.invert();
                aUnitPolyPolygon.transform(aInverse);
            }

            // create primitive. Always create primitives to allow the decomposition of
            // SdrPathPrimitive2D to create needed invisible elements for HitTest and/or BoundRect
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::SdrPathPrimitive2D(
                    aObjectMatrix,
                    aAttribute,
                    aUnitPolyPolygon));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
