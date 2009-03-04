/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdrrectobj.cxx,v $
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

#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>
#include <svx/svdorect.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <svx/sdr/primitive2d/sdrrectangleprimitive2d.hxx>
#include <svtools/itemset.hxx>
#include <svx/sdr/primitive2d/sdrprimitivetools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrRectObj::ViewContactOfSdrRectObj(SdrRectObj& rRectObj)
        :   ViewContactOfTextObj(rRectObj)
        {
        }

        ViewContactOfSdrRectObj::~ViewContactOfSdrRectObj()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrRectObj::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SfxItemSet& rItemSet = GetRectObj().GetMergedItemSet();
            SdrText* pSdrText = GetRectObj().getText(0);

            if(pSdrText)
            {
                drawinglayer::attribute::SdrLineFillShadowTextAttribute* pAttribute = drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(rItemSet, *pSdrText);

                if(pAttribute)
                {
                    if(pAttribute->isVisible())
                    {
                        // take unrotated snap rect (direct model data) for position and size
                        const Rectangle& rRectangle = GetRectObj().GetGeoRect();
                        const ::basegfx::B2DRange aObjectRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());
                        const GeoStat& rGeoStat(GetRectObj().GetGeoStat());
                        ::basegfx::B2DHomMatrix aObjectMatrix;

                        // fill object matrix
                        if(!::basegfx::fTools::equalZero(aObjectRange.getWidth()))
                        {
                            aObjectMatrix.set(0, 0, aObjectRange.getWidth());
                        }

                        if(!::basegfx::fTools::equalZero(aObjectRange.getHeight()))
                        {
                            aObjectMatrix.set(1, 1, aObjectRange.getHeight());
                        }

                        if(rGeoStat.nShearWink)
                        {
                            aObjectMatrix.shearX(tan((36000 - rGeoStat.nShearWink) * F_PI18000));
                        }

                        if(rGeoStat.nDrehWink)
                        {
                            aObjectMatrix.rotate((36000 - rGeoStat.nDrehWink) * F_PI18000);
                        }

                        aObjectMatrix.translate(aObjectRange.getMinX(), aObjectRange.getMinY());

                        // calculate corner radius
                        sal_uInt32 nCornerRadius(((SdrEckenradiusItem&)(rItemSet.Get(SDRATTR_ECKENRADIUS))).GetValue());
                        double fCornerRadiusX;
                        double fCornerRadiusY;
                        drawinglayer::primitive2d::calculateRelativeCornerRadius(nCornerRadius, aObjectRange, fCornerRadiusX, fCornerRadiusY);

                        // create primitive
                        const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::SdrRectanglePrimitive2D(
                            aObjectMatrix, *pAttribute, fCornerRadiusX, fCornerRadiusY));
                        xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                    }

                    delete pAttribute;
                }

                if(!xRetval.hasElements())
                {
                    // #i99123#
                    // Object is invisible. Create a fallback primitive for HitTest
                    basegfx::B2DHomMatrix aObjectMatrix;
                    basegfx::B2DPolyPolygon aObjectPolyPolygon;
                    GetRectObj().TRGetBaseGeometry(aObjectMatrix, aObjectPolyPolygon);
                    const drawinglayer::primitive2d::Primitive2DReference xReference(
                        drawinglayer::primitive2d::createFallbackHitTestPrimitive(aObjectMatrix));
                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
