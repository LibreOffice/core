/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shadow3dextractor.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-02-07 13:41:59 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_SHADOW3DEXTRACTOR_HXX
#include <drawinglayer/processor3d/shadow3dextractor.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SHADOWPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/unifiedalphaprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRLABELPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/sdrlabelprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        Shadow3DExtractingProcessor::Shadow3DExtractingProcessor(
            double fTime,
            const basegfx::B2DHomMatrix& rObjectTransformation,
            const basegfx::B3DHomMatrix& rWorldToEye,
            const basegfx::B3DHomMatrix& rEyeToView,
            const attribute::SdrLightingAttribute& rSdrLightingAttribute,
            const primitive3d::Primitive3DSequence& rPrimitiveSequence,
            double fShadowSlant)
        :   BaseProcessor3D(fTime),
            maPrimitive2DSequence(),
            mpPrimitive2DSequence(&maPrimitive2DSequence),
            maObjectTransformation(rObjectTransformation),
            maWorldToEye(rWorldToEye),
            maEyeToView(rEyeToView),
            maWorldToView(maEyeToView * maWorldToEye),
            maLightNormal(),
            maShadowPlaneNormal(),
            maPlanePoint(),
            mfLightPlaneScalar(0.0),
            maPrimitiveColor(),
            mbShadowProjectionIsValid(false),
            mbConvert(false),
            mbUseProjection(false)
        {
            // create deviceToView projection for shadow geometry
            // outcome is [-1.0 .. 1.0] in X,Y and Z. bring to [0.0 .. 1.0]. Also
            // necessary to flip Y due to screen orientation
            // Z is not needed, but will also be brought to [0.0 .. 1.0]
            basegfx::B3DHomMatrix aDeviceToView;
            aDeviceToView.scale(0.5, -0.5, 0.5);
            aDeviceToView.translate(0.5, 0.5, 0.5);

            // calculate shadow projection stuff
            if(rSdrLightingAttribute.getLightVector().size())
            {
                // get light normal, plane normal and sclalar from it
                maLightNormal = rSdrLightingAttribute.getLightVector()[0L].getDirection();
                maLightNormal.normalize();
                maShadowPlaneNormal = basegfx::B3DVector(0.0, sin(fShadowSlant), cos(fShadowSlant));
                maShadowPlaneNormal.normalize();
                mfLightPlaneScalar = maLightNormal.scalar(maShadowPlaneNormal);

                // use only when scalar is > 0.0, so the light is in front of the object
                if(basegfx::fTools::more(mfLightPlaneScalar, 0.0))
                {
                    basegfx::B3DRange aContained3DRange(primitive3d::getB3DRangeFromPrimitive3DSequence(rPrimitiveSequence, getTime()));
                    aContained3DRange.transform(getWorldToEye());
                    maPlanePoint.setX(maShadowPlaneNormal.getX() < 0.0 ? aContained3DRange.getMinX() : aContained3DRange.getMaxX());
                    maPlanePoint.setY(maShadowPlaneNormal.getY() > 0.0 ? aContained3DRange.getMinY() : aContained3DRange.getMaxY());
                    maPlanePoint.setZ(aContained3DRange.getMinZ() - (aContained3DRange.getDepth() / 8.0));

                    // set flag that shadow projection is prepared and allowed
                    mbShadowProjectionIsValid = true;
                }
            }
        }

        void Shadow3DExtractingProcessor::process(const primitive3d::Primitive3DSequence& rSource)
        {
            if(rSource.hasElements())
            {
                const sal_Int32 nCount(rSource.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    // get reference
                    const primitive3d::Primitive3DReference xReference(rSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BasePrimitive3D implementation
                        const primitive3d::BasePrimitive3D* pBasePrimitive = dynamic_cast< const primitive3d::BasePrimitive3D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            // it is a BasePrimitive3D implementation, use getPrimitiveID() call for switch
                            switch(pBasePrimitive->getPrimitiveID())
                            {
                                case PRIMITIVE3D_ID_SHADOWPRIMITIVE3D :
                                {
                                    // shadow3d object. Call recursive with content and start conversion
                                    const primitive3d::ShadowPrimitive3D& rPrimitive = static_cast< const primitive3d::ShadowPrimitive3D& >(*pBasePrimitive);

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
                                    const primitive3d::TransformPrimitive3D& rPrimitive = static_cast< const primitive3d::TransformPrimitive3D& >(*pBasePrimitive);
                                    basegfx::B3DHomMatrix aLastWorldToView(getWorldToView());
                                    basegfx::B3DHomMatrix aLastWorldToEye(getWorldToEye());

                                    // create new transformations
                                    maWorldToView = getWorldToView() * rPrimitive.getTransformation();
                                    maWorldToEye = getWorldToEye() * rPrimitive.getTransformation();

                                    // let break down
                                    process(rPrimitive.getChildren());

                                    // restore transformations
                                    maWorldToView = aLastWorldToView;
                                    maWorldToEye = aLastWorldToEye;
                                    break;
                                }
                                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                                {
                                    // PolygonHairlinePrimitive3D
                                    if(mbConvert)
                                    {
                                        const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(*pBasePrimitive);
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
                                            a2DHairline = basegfx::tools::createB2DPolygonFromB3DPolygon(rPrimitive.getB3DPolygon(), getWorldToView());
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
                                        const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(*pBasePrimitive);
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
                                            a2DFill = basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(rPrimitive.getB3DPolyPolygon(), getWorldToView());
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
                                case PRIMITIVE3D_ID_SDRLABELPRIMITIVE3D :
                                {
                                    // SdrLabelPrimitive3D
                                    // has no 3d shadow, accept and ignore
                                    break;
                                }
                                default :
                                {
                                    // process recursively
                                    process(pBasePrimitive->get3DDecomposition(getTime()));
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // unknown implementation, use UNO API call instead and process recursively
                            const uno::Sequence< beans::PropertyValue > xViewParameters(primitive3d::TimeToViewParameters(getTime()));
                            process(xReference->getDecomposition(xViewParameters));
                        }
                    }
                }
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
