/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shadow3dextractor.cxx,v $
 *
 *  $Revision: 1.9 $
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

#include <drawinglayer/processor3d/shadow3dextractor.hxx>
#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedalphaprimitive2d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        // as tooling, the process() implementation takes over API handling and calls this
        // virtual render method when the primitive implementation is BufDecPrimitive3D-based.
        void Shadow3DExtractingProcessor::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate)
        {
            // it is a BufDecPrimitive3D implementation, use getPrimitive3DID() call for switch
            switch(rCandidate.getPrimitive3DID())
            {
                case PRIMITIVE3D_ID_SHADOWPRIMITIVE3D :
                {
                    // shadow3d object. Call recursive with content and start conversion
                    const primitive3d::ShadowPrimitive3D& rPrimitive = static_cast< const primitive3d::ShadowPrimitive3D& >(rCandidate);

                    // set new target
                    primitive2d::Primitive2DSequence aNewSubList;
                    primitive2d::Primitive2DSequence* pLastTargetSequence = mpPrimitive2DSequence;
                    mpPrimitive2DSequence = &aNewSubList;

                    // activate convert
                    const bool bLastConvert(mbConvert);
                    mbConvert = true;

                    // set projection flag
                    const bool bLastUseProjection(mbUseProjection);
                    mbUseProjection = rPrimitive.getShadow3D();

                    // process content
                    process(rPrimitive.getChildren());

                    // restore values
                    mbUseProjection = bLastUseProjection;
                    mbConvert = bLastConvert;
                    mpPrimitive2DSequence = pLastTargetSequence;

                    // create 2d shadow primitive with result
                    const primitive2d::Primitive2DReference xRef(new primitive2d::ShadowPrimitive2D(rPrimitive.getShadowTransform(), rPrimitive.getShadowColor(), aNewSubList));

                    if(basegfx::fTools::more(rPrimitive.getShadowTransparence(), 0.0))
                    {
                        // create simpleTransparencePrimitive, add created primitives
                        const primitive2d::Primitive2DSequence aNewTransPrimitiveVector(&xRef, 1L);
                        const primitive2d::Primitive2DReference xRef2(new primitive2d::UnifiedAlphaPrimitive2D(aNewTransPrimitiveVector, rPrimitive.getShadowTransparence()));
                        primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(*mpPrimitive2DSequence, xRef2);
                    }
                    else
                    {
                        // add directly
                        primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(*mpPrimitive2DSequence, xRef);
                    }
                    break;
                }
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    // transform group. Remember current transformations
                    const primitive3d::TransformPrimitive3D& rPrimitive = static_cast< const primitive3d::TransformPrimitive3D& >(rCandidate);
                    const geometry::ViewInformation3D aLastViewInformation3D(getViewInformation3D());

                    // create new transformation; add new object transform from right side
                    const geometry::ViewInformation3D aNewViewInformation3D(
                        aLastViewInformation3D.getObjectTransformation() * rPrimitive.getTransformation(),
                        aLastViewInformation3D.getOrientation(),
                        aLastViewInformation3D.getProjection(),
                        aLastViewInformation3D.getDeviceToView(),
                        aLastViewInformation3D.getViewTime(),
                        aLastViewInformation3D.getExtendedInformationSequence());
                    updateViewInformation(aNewViewInformation3D);

                    if(mbShadowProjectionIsValid)
                    {
                        // update buffered WorldToEye and EyeToView
                        maWorldToEye = getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation();
                        maEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();
                    }

                    // let break down
                    process(rPrimitive.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation3D);

                    if(mbShadowProjectionIsValid)
                    {
                        // update buffered WorldToEye and EyeToView
                        maWorldToEye = getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation();
                        maEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();
                    }
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    // PolygonHairlinePrimitive3D
                    if(mbConvert)
                    {
                        const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(rCandidate);
                        basegfx::B2DPolygon a2DHairline;

                        if(mbUseProjection)
                        {
                            if(mbShadowProjectionIsValid)
                            {
                                a2DHairline = impDoShadowProjection(rPrimitive.getB3DPolygon());
                            }
                        }
                        else
                        {
                            a2DHairline = basegfx::tools::createB2DPolygonFromB3DPolygon(rPrimitive.getB3DPolygon(), getViewInformation3D().getObjectToView());
                        }

                        if(a2DHairline.count())
                        {
                            a2DHairline.transform(getObjectTransformation());
                            const primitive2d::Primitive2DReference xRef(new primitive2d::PolygonHairlinePrimitive2D(a2DHairline, maPrimitiveColor));
                            primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(*mpPrimitive2DSequence, xRef);
                        }
                    }
                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    // PolyPolygonMaterialPrimitive3D
                    if(mbConvert)
                    {
                        const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rCandidate);
                        basegfx::B2DPolyPolygon a2DFill;

                        if(mbUseProjection)
                        {
                            if(mbShadowProjectionIsValid)
                            {
                                a2DFill = impDoShadowProjection(rPrimitive.getB3DPolyPolygon());
                            }
                        }
                        else
                        {
                            a2DFill = basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(rPrimitive.getB3DPolyPolygon(), getViewInformation3D().getObjectToView());
                        }

                        if(a2DFill.count())
                        {
                            a2DFill.transform(getObjectTransformation());
                            const primitive2d::Primitive2DReference xRef(new primitive2d::PolyPolygonColorPrimitive2D(a2DFill, maPrimitiveColor));
                            primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(*mpPrimitive2DSequence, xRef);
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

        Shadow3DExtractingProcessor::Shadow3DExtractingProcessor(
            const geometry::ViewInformation3D& rViewInformation,
            const basegfx::B2DHomMatrix& rObjectTransformation,
            const basegfx::B3DVector& rLightNormal,
            double fShadowSlant,
            const basegfx::B3DRange& rContained3DRange)
        :   BaseProcessor3D(rViewInformation),
            maPrimitive2DSequence(),
            mpPrimitive2DSequence(&maPrimitive2DSequence),
            maObjectTransformation(rObjectTransformation),
            maWorldToEye(),
            maEyeToView(),
            maLightNormal(rLightNormal),
            maShadowPlaneNormal(),
            maPlanePoint(),
            mfLightPlaneScalar(0.0),
            maPrimitiveColor(),
            mbShadowProjectionIsValid(false),
            mbConvert(false),
            mbUseProjection(false)
        {
            // normalize light normal, get and normalize shadow plane normal and calculate scalar from it
            maLightNormal.normalize();
            maShadowPlaneNormal = basegfx::B3DVector(0.0, sin(fShadowSlant), cos(fShadowSlant));
            maShadowPlaneNormal.normalize();
            mfLightPlaneScalar = maLightNormal.scalar(maShadowPlaneNormal);

            // use only when scalar is > 0.0, so the light is in front of the object
            if(basegfx::fTools::more(mfLightPlaneScalar, 0.0))
            {
                // prepare buffered WorldToEye and EyeToView
                maWorldToEye = getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation();
                maEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();

                // calculate range to get front edge around which to rotate the shadow's projection
                basegfx::B3DRange aContained3DRange(rContained3DRange);
                aContained3DRange.transform(getWorldToEye());
                maPlanePoint.setX(maShadowPlaneNormal.getX() < 0.0 ? aContained3DRange.getMinX() : aContained3DRange.getMaxX());
                maPlanePoint.setY(maShadowPlaneNormal.getY() > 0.0 ? aContained3DRange.getMinY() : aContained3DRange.getMaxY());
                maPlanePoint.setZ(aContained3DRange.getMinZ() - (aContained3DRange.getDepth() / 8.0));

                // set flag that shadow projection is prepared and allowed
                mbShadowProjectionIsValid = true;
            }
        }

        basegfx::B2DPolygon Shadow3DExtractingProcessor::impDoShadowProjection(const basegfx::B3DPolygon& rSource)
        {
            basegfx::B2DPolygon aRetval;

            for(sal_uInt32 a(0L); a < rSource.count(); a++)
            {
                // get point, transform to eye coordinate system
                basegfx::B3DPoint aCandidate(rSource.getB3DPoint(a));
                aCandidate *= getWorldToEye();

                // we are in eye coordinates
                // ray is (aCandidate + fCut * maLightNormal)
                // plane is (maPlanePoint, maShadowPlaneNormal)
                // maLightNormal.scalar(maShadowPlaneNormal) is already in mfLightPlaneScalar and > 0.0
                // get cut point of ray with shadow plane
                const double fCut(basegfx::B3DVector(maPlanePoint - aCandidate).scalar(maShadowPlaneNormal) / mfLightPlaneScalar);
                aCandidate += maLightNormal * fCut;

                // transform to view, use 2d coordinates
                aCandidate *= getEyeToView();
                aRetval.append(basegfx::B2DPoint(aCandidate.getX(), aCandidate.getY()));
            }

            // copy closed flag
            aRetval.setClosed(rSource.isClosed());

            return aRetval;
        }

        basegfx::B2DPolyPolygon Shadow3DExtractingProcessor::impDoShadowProjection(const basegfx::B3DPolyPolygon& rSource)
        {
            basegfx::B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rSource.count(); a++)
            {
                aRetval.append(impDoShadowProjection(rSource.getB3DPolygon(a)));
            }

            return aRetval;
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
