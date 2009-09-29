/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zbufferprocessor3d.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:31:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/processor3d/cutfindprocessor3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#include <drawinglayer/primitive3d/hittestprimitive3d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        CutFindProcessor::CutFindProcessor(const geometry::ViewInformation3D& rViewInformation,
            const basegfx::B3DPoint& rFront,
            const basegfx::B3DPoint& rBack,
            bool bAnyHit)
        :   BaseProcessor3D(rViewInformation),
            maFront(rFront),
            maBack(rBack),
            maResult(),
            maCombinedTransform(),
            mbAnyHit(bAnyHit)
        {
        }

        void CutFindProcessor::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate)
        {
            if(getAnyHit() && maResult.size())
            {
                // stop processing as soon as a hit was recognized
                return;
            }

            // it is a BasePrimitive3D implementation, use getPrimitive3DID() call for switch
            switch(rCandidate.getPrimitive3DID())
            {
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    // transform group.
                    const primitive3d::TransformPrimitive3D& rPrimitive = static_cast< const primitive3d::TransformPrimitive3D& >(rCandidate);

                    // remember old and transform front, back to object coordinates
                    const basegfx::B3DPoint aLastFront(maFront);
                    const basegfx::B3DPoint aLastBack(maBack);
                    basegfx::B3DHomMatrix aInverseTrans(rPrimitive.getTransformation());
                    aInverseTrans.invert();
                    maFront *= aInverseTrans;
                    maBack *= aInverseTrans;

                    // remember current and create new transformation; add new object transform from right side
                    const geometry::ViewInformation3D aLastViewInformation3D(getViewInformation3D());
                    const geometry::ViewInformation3D aNewViewInformation3D(
                        aLastViewInformation3D.getObjectTransformation() * rPrimitive.getTransformation(),
                        aLastViewInformation3D.getOrientation(),
                        aLastViewInformation3D.getProjection(),
                        aLastViewInformation3D.getDeviceToView(),
                        aLastViewInformation3D.getViewTime(),
                        aLastViewInformation3D.getExtendedInformationSequence());
                    updateViewInformation(aNewViewInformation3D);

                    // #i102956# remember needed back-transform for found cuts (combine from right side)
                    const basegfx::B3DHomMatrix aLastCombinedTransform(maCombinedTransform);
                    maCombinedTransform = maCombinedTransform * rPrimitive.getTransformation();

                    // let break down
                    process(rPrimitive.getChildren());

                    // restore transformations and front, back
                    maCombinedTransform = aLastCombinedTransform;
                    updateViewInformation(aLastViewInformation3D);
                    maFront = aLastFront;
                    maBack = aLastBack;
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    // PolygonHairlinePrimitive3D, not used for hit test with planes, ignore. This
                    // means that also thick line expansion will not be hit-tested as
                    // PolyPolygonMaterialPrimitive3D
                    break;
                }
                case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D :
                {
                    // #i97321#
                    // For HatchTexturePrimitive3D, do not use the decomposition since it will produce
                    // clipped hatch lines in 3D. It can be used when the hatch also has a filling, but for
                    // simplicity, just use the children which are the PolyPolygonMaterialPrimitive3D
                    // which define the hatched areas anyways; for HitTest this is more than adequate
                    const primitive3d::HatchTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::HatchTexturePrimitive3D& >(rCandidate);
                    process(rPrimitive.getChildren());
                    break;
                }
                case PRIMITIVE3D_ID_HITTESTPRIMITIVE3D :
                {
                    // HitTestPrimitive3D, force usage due to we are doing a hit test and this
                    // primitive only gets generated on 3d objects without fill, exactly for this
                    // purpose
                    const primitive3d::HitTestPrimitive3D& rPrimitive = static_cast< const primitive3d::HitTestPrimitive3D& >(rCandidate);
                    process(rPrimitive.getChildren());
                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    // PolyPolygonMaterialPrimitive3D
                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rCandidate);

                    if(!maFront.equal(maBack))
                    {
                           const basegfx::B3DPolyPolygon& rPolyPolygon = rPrimitive.getB3DPolyPolygon();
                        const sal_uInt32 nPolyCount(rPolyPolygon.count());

                        if(nPolyCount)
                        {
                               const basegfx::B3DPolygon aPolygon(rPolyPolygon.getB3DPolygon(0));
                            const sal_uInt32 nPointCount(aPolygon.count());

                            if(nPointCount > 2)
                            {
                                const basegfx::B3DVector aPlaneNormal(aPolygon.getNormal());

                                if(!aPlaneNormal.equalZero())
                                {
                                    const basegfx::B3DPoint aPointOnPlane(aPolygon.getB3DPoint(0));
                                    double fCut(0.0);

                                    if(basegfx::tools::getCutBetweenLineAndPlane(aPlaneNormal, aPointOnPlane, maFront, maBack, fCut))
                                    {
                                        const basegfx::B3DPoint aCutPoint(basegfx::interpolate(maFront, maBack, fCut));

                                        if(basegfx::tools::isInside(rPolyPolygon, aCutPoint, false))
                                        {
                                            // #i102956# add result. Do not forget to do this in the coordinate
                                            // system the processor get started with, so use the collected
                                            // combined transformation from processed TransformPrimitive3D's
                                            maResult.push_back(maCombinedTransform * aCutPoint);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    break;
                }
                default :
                {
                    // process recursively
                    process(rCandidate.get3DDecomposition(getViewInformation3D()));
                    break;
                }
            }
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
