/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdrcircobj.cxx,v $
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

#include <svx/sdr/contact/viewcontactofsdrcircobj.hxx>
#include <svx/svdocirc.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <svx/sdr/primitive2d/sdrellipseprimitive2d.hxx>
#include <svtools/itemset.hxx>
#include <svx/sxciaitm.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrCircObj::ViewContactOfSdrCircObj(SdrCircObj& rCircObj)
        :   ViewContactOfSdrRectObj(rCircObj)
        {
        }

        ViewContactOfSdrCircObj::~ViewContactOfSdrCircObj()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrCircObj::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SfxItemSet& rItemSet = GetCircObj().GetMergedItemSet();
            SdrText* pSdrText = GetCircObj().getText(0);

            if(pSdrText)
            {
                drawinglayer::attribute::SdrLineFillShadowTextAttribute* pAttribute = drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(rItemSet, *pSdrText);

                if(pAttribute)
                {
                    if(pAttribute->isVisible())
                    {
                        // take unrotated snap rect (direct model data) for position and size
                        const Rectangle& rRectangle = GetCircObj().GetGeoRect();
                        const ::basegfx::B2DRange aObjectRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());
                        const GeoStat& rGeoStat(GetCircObj().GetGeoStat());

                        // fill object matrix
                        const basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            aObjectRange.getWidth(), aObjectRange.getHeight(),
                            rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0,
                            rGeoStat.nDrehWink ? (36000 - rGeoStat.nDrehWink) * F_PI18000 : 0.0,
                            aObjectRange.getMinX(), aObjectRange.getMinY()));

                        // create primitive data
                        const sal_uInt16 nIdentifier(GetCircObj().GetObjIdentifier());

                        if(OBJ_CIRC == nIdentifier)
                        {
                            const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::SdrEllipsePrimitive2D(aObjectMatrix, *pAttribute));
                            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                        }
                        else
                        {
                            const sal_Int32 nNewStart(((SdrCircStartAngleItem&)rItemSet.Get(SDRATTR_CIRCSTARTANGLE)).GetValue());
                            const sal_Int32 nNewEnd(((SdrCircEndAngleItem&)rItemSet.Get(SDRATTR_CIRCENDANGLE)).GetValue());
                            const double fStart(((36000 - nNewEnd) % 36000) * F_PI18000);
                            const double fEnd(((36000 - nNewStart) % 36000) * F_PI18000);
                            const bool bCloseSegment(OBJ_CARC != nIdentifier);
                            const bool bCloseUsingCenter(OBJ_SECT == nIdentifier);
                            const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::SdrEllipseSegmentPrimitive2D(aObjectMatrix, *pAttribute, fStart, fEnd, bCloseSegment, bCloseUsingCenter));
                            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                        }
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
