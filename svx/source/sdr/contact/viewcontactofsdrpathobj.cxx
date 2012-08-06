/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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


#include <svx/sdr/contact/viewcontactofsdrpathobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/primitive2d/sdrpathprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////

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
                    GetPathObj().getText(0)));
            basegfx::B2DPolyPolygon aUnitPolyPolygon(GetPathObj().GetPathPoly());
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
