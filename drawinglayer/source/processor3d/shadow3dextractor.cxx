/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shadow3dextractor.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:57:48 $
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

#ifndef _DRAWINGLAYER_PROCESSOR3D_SHADOW3DEXTRACTOR_HXX
#include <drawinglayer/processor3d/shadow3dextractor.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_SHADOWPRIMITIVE_HXX
#include <drawinglayer/primitive/shadowprimitive.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _DRAWINGLAYER_GEOMETRY_TRANSFORMATION3D_HXX
#include <drawinglayer/geometry/transformation3d.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_SIMPLETRANSPARENCEPRIMITIVE_HXX
#include <drawinglayer/primitive/simpletransparenceprimitive.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_POLYGONPRIMITIVE_HXX
#include <drawinglayer/primitive/polygonprimitive.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_POLYPOLYGONPRIMITIVE_HXX
#include <drawinglayer/primitive/polypolygonprimitive.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        shadow3DExtractingProcessor::shadow3DExtractingProcessor(
            const geometry::viewInformation& rViewInformation,
            const geometry::transformation3D& rTransformation3D,
            const attribute::sdrLightingAttribute& rSdrLightingAttribute,
            const primitive3d::primitiveVector3D& rPrimitiveVector,
            double fShadowSlant)
        :   baseProcessor3D(rViewInformation, rTransformation3D),
            mpTargetVector(&maPrimitiveVector),
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

            // create complete 3d transformation set for shadow stuff
            maWorldToEye = getTransformation3D().getOrientation() * getTransformation3D().getTransformation();
            maEyeToView = getTransformation3D().getDeviceToView() * getTransformation3D().getProjection();
            maWorldToView = maEyeToView * maWorldToEye;

            // calculate shadow projection stuff
            if(rSdrLightingAttribute.getLightVector().size())
            {
                // get light normal, plane normal and sclalar from it
                maLightNormal = rSdrLightingAttribute.getLightVector()[0L].getDirection();
                maLightNormal.normalize();
                maShadowPlaneNormal = basegfx::B3DVector(0.0, sin(fShadowSlant), cos(fShadowSlant));
                maShadowPlaneNormal.normalize();
                mfLightPlaneScalar = maLightNormal.scalar(maShadowPlaneNormal);

                if(basegfx::fTools::more(mfLightPlaneScalar, 0.0))
                {
                    // use only when scalar is > 0.0, so the light is in front of the object
                    basegfx::B3DRange aContained3DRange(get3DRangeFromVector(rPrimitiveVector));
                    aContained3DRange.transform(maWorldToEye);
                    maPlanePoint.setX(maShadowPlaneNormal.getX() < 0.0 ? aContained3DRange.getMinX() : aContained3DRange.getMaxX());
                    maPlanePoint.setY(maShadowPlaneNormal.getY() > 0.0 ? aContained3DRange.getMinY() : aContained3DRange.getMaxY());
                    maPlanePoint.setZ(aContained3DRange.getMinZ() - (aContained3DRange.getDepth() / 8.0));

                    // set flag that shadow projection is prepared and allowed
                    mbShadowProjectionIsValid = true;
                }
            }
        }

        shadow3DExtractingProcessor::~shadow3DExtractingProcessor()
        {
        }

        void shadow3DExtractingProcessor::process(const primitive3d::primitiveVector3D& rSource)
        {
            for(sal_uInt32 a(0L); a < rSource.size(); a++)
            {
                // get reference
                const primitive3d::referencedPrimitive3D& rCandidate = rSource[a];

                switch(rCandidate.getID())
                {
                    case CreatePrimitiveID('S', 'H', 'D', '3'):
                    {
                        // shadow3d object. Call recursive with content and start conversion
                        const primitive3d::shadowPrimitive3D& rPrimitive = static_cast< const primitive3d::shadowPrimitive3D& >(rCandidate.getBasePrimitive());

                        // set new target
                        primitive::primitiveVector aNewSubList;
                        primitive::primitiveVector* pLastTargetVector = mpTargetVector;
                        mpTargetVector = &aNewSubList;

                        // activate convert
                        const bool bLastConvert(mbConvert);
                        mbConvert = true;

                        // set projection flag
                        const bool bLastUseProjection(mbUseProjection);
                        mbUseProjection = rPrimitive.getShadow3D();

                        // process content
                        process(rPrimitive.getPrimitives());

                        // restore values
                        mbUseProjection = bLastUseProjection;
                        mbConvert = bLastConvert;
                        mpTargetVector = pLastTargetVector;

                        // create 2d shadow primitive with result
                        primitive::shadowPrimitive* pNew = new primitive::shadowPrimitive(rPrimitive.getShadowTransform(), rPrimitive.getShadowColor(), aNewSubList);

                        if(basegfx::fTools::more(rPrimitive.getShadowTransparence(), 0.0))
                        {
                            // create simpleTransparencePrimitive, add created primitives
                            primitive::primitiveVector aNewTransPrimitiveVector;
                            aNewTransPrimitiveVector.push_back(primitive::referencedPrimitive(*pNew));
                            primitive::simpleTransparencePrimitive* pNewTrans = new primitive::simpleTransparencePrimitive(rPrimitive.getShadowTransparence(), aNewTransPrimitiveVector);
                            mpTargetVector->push_back(primitive::referencedPrimitive(*pNewTrans));
                        }
                        else
                        {
                            // add directly
                            mpTargetVector->push_back(primitive::referencedPrimitive(*pNew));
                        }

                        break;
                    }

                    case CreatePrimitiveID('T', 'R', 'N', '3'):
                    {
                        // transform group. Remember current transformations
                        const primitive3d::transformPrimitive3D& rPrimitive = static_cast< const primitive3d::transformPrimitive3D& >(rCandidate.getBasePrimitive());
                        basegfx::B3DHomMatrix aLastWorldToView(maWorldToView);
                        basegfx::B3DHomMatrix aLastWorldToEye(maWorldToEye);

                        // create new transformations
                        maWorldToView = maWorldToView * rPrimitive.getTransformation();
                        maWorldToEye = maWorldToEye * rPrimitive.getTransformation();

                        // let break down
                        process(rPrimitive.getPrimitives());

                        // restore transformations
                        maWorldToView = aLastWorldToView;
                        maWorldToEye = aLastWorldToEye;
                        break;
                    }

                    case CreatePrimitiveID('P', 'O', 'H', '3'):
                    {
                        // polygonHairlinePrimitive3D
                        if(mbConvert)
                        {
                            const primitive3d::polygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::polygonHairlinePrimitive3D& >(rCandidate.getBasePrimitive());
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
                                a2DHairline = basegfx::tools::createB2DPolygonFromB3DPolygon(rPrimitive.getB3DPolygon(), maWorldToView);
                            }

                            if(a2DHairline.count())
                            {
                                a2DHairline.transform(getTransformation3D().getObjectTransformation());
                                primitive::polygonHairlinePrimitive* pNew = new primitive::polygonHairlinePrimitive(a2DHairline, maPrimitiveColor);
                                mpTargetVector->push_back(primitive::referencedPrimitive(*pNew));
                            }
                        }

                        break;
                    }

                    case CreatePrimitiveID('P', 'O', 'M', '3'):
                    {
                        // polyPolygonMaterialPrimitive3D
                        if(mbConvert)
                        {
                            const primitive3d::polyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::polyPolygonMaterialPrimitive3D& >(rCandidate.getBasePrimitive());
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
                                a2DFill = basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(rPrimitive.getB3DPolyPolygon(), maWorldToView);
                            }

                            if(a2DFill.count())
                            {
                                a2DFill.transform(getTransformation3D().getObjectTransformation());
                                primitive::polyPolygonColorPrimitive* pNew = new primitive::polyPolygonColorPrimitive(a2DFill, maPrimitiveColor);
                                mpTargetVector->push_back(primitive::referencedPrimitive(*pNew));
                            }
                        }

                        break;
                    }

                    case CreatePrimitiveID('L', 'A', 'B', '3'):
                    {
                        // has no 3d shadow, accept and ignore
                        break;
                    }

                    default:
                    {
                        // let break down
                        process(rCandidate.getBasePrimitive().getDecomposition());
                        break;
                    }
                }
            }
        }

        basegfx::B2DPolygon shadow3DExtractingProcessor::impDoShadowProjection(const basegfx::B3DPolygon& rSource)
        {
            basegfx::B2DPolygon aRetval;

            for(sal_uInt32 a(0L); a < rSource.count(); a++)
            {
                // get point, transform to eye coordinate system
                basegfx::B3DPoint aCandidate(rSource.getB3DPoint(a));
                aCandidate *= maWorldToEye;

                // we are in eye coordinates
                // ray is (aCandidate + fCut * maLightNormal)
                // plane is (maPlanePoint, maShadowPlaneNormal)
                // maLightNormal.scalar(maShadowPlaneNormal) is already in mfLightPlaneScalar and > 0.0
                // get cut point of ray with shadow plane
                const double fCut(basegfx::B3DVector(maPlanePoint - aCandidate).scalar(maShadowPlaneNormal) / mfLightPlaneScalar);
                aCandidate += maLightNormal * fCut;

                // transform to view, use 2d coordinates
                aCandidate *= maEyeToView;
                aRetval.append(basegfx::B2DPoint(aCandidate.getX(), aCandidate.getY()));
            }

            // copy closed flag
            aRetval.setClosed(rSource.isClosed());

            return aRetval;
        }

        basegfx::B2DPolyPolygon shadow3DExtractingProcessor::impDoShadowProjection(const basegfx::B3DPolyPolygon& rSource)
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
