/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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
                OSL_ENSURE(false, "PolyPolygon object without geometry detected, this should not be created (!)");
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

//////////////////////////////////////////////////////////////////////////////
// eof
