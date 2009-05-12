/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdrpathobj.cxx,v $
 *
 * $Revision: 1.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewcontactofsdrpathobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/primitive2d/sdrpathprimitive2d.hxx>

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
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SfxItemSet& rItemSet = GetPathObj().GetMergedItemSet();
            SdrText* pSdrText = GetPathObj().getText(0);

            if(pSdrText)
            {
                drawinglayer::attribute::SdrLineFillShadowTextAttribute* pAttribute = drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(rItemSet, *pSdrText);

                if(pAttribute)
                {
                    if(pAttribute->isVisible())
                    {
                        // prepare object transformation and unit polygon (direct model data)
                        ::basegfx::B2DHomMatrix aObjectMatrix;
                        ::basegfx::B2DPolyPolygon aUnitPolyPolygon(GetPathObj().GetPathPoly());
                        const bool bIsLine(
                            !aUnitPolyPolygon.areControlPointsUsed()
                            && 1L == aUnitPolyPolygon.count()
                            && 2L == aUnitPolyPolygon.getB2DPolygon(0L).count());

                        if(bIsLine)
                        {
                            // special handling for single line mode (2 points)
                            const ::basegfx::B2DPolygon aSubPolygon(aUnitPolyPolygon.getB2DPolygon(0L));
                            const ::basegfx::B2DPoint aStart(aSubPolygon.getB2DPoint(0L));
                            const ::basegfx::B2DPoint aEnd(aSubPolygon.getB2DPoint(1L));
                            const ::basegfx::B2DVector aLine(aEnd - aStart);

                            // create new polygon
                            ::basegfx::B2DPolygon aNewPolygon;
                            aNewPolygon.append(::basegfx::B2DPoint(0.0, 0.0));
                            aNewPolygon.append(::basegfx::B2DPoint(aLine.getLength(), 0.0));
                            aUnitPolyPolygon.setB2DPolygon(0L, aNewPolygon);

                            // fill objectMatrix with rotation and offset (no shear for lines, scale in polygon)
                            aObjectMatrix.rotate(atan2(aLine.getY(), aLine.getX()));
                            aObjectMatrix.translate(aStart.getX(), aStart.getY());
                        }
                        else
                        {
                            // create scaled, but unsheared, unrotated and untranslated polygon
                            // by creating the object matrix and back-transforming the polygon
                            const ::basegfx::B2DRange aObjectRange(::basegfx::tools::getRange(aUnitPolyPolygon));
                            const GeoStat& rGeoStat(GetPathObj().GetGeoStat());

                            aObjectMatrix.shearX(tan((36000 - rGeoStat.nShearWink) * F_PI18000));
                            aObjectMatrix.rotate((36000 - rGeoStat.nDrehWink) * F_PI18000);
                            aObjectMatrix.translate(aObjectRange.getMinX(), aObjectRange.getMinY());

                            // ceate scaled unit polygon from object's absolute path
                            ::basegfx::B2DHomMatrix aInverse(aObjectMatrix);
                            aInverse.invert();
                            aUnitPolyPolygon.transform(aInverse);
                        }

                        // create primitive
                        const drawinglayer::primitive2d::Primitive2DReference xReference(
                            new drawinglayer::primitive2d::SdrPathPrimitive2D(aObjectMatrix, *pAttribute, aUnitPolyPolygon));
                        xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                    }

                    delete pAttribute;
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
