/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sceneprimitive2d.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-01-30 12:25:05 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SCENEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX
#include <drawinglayer/processor3d/defaultprocessor3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_SHADOW3DEXTRACTOR_HXX
#include <drawinglayer/processor3d/shadow3dextractor.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_LABEL3DEXTRACTOR_HXX
#include <drawinglayer/processor3d/label3dextractor.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_OPTIONSDRAWINGLAYER_HXX
#include <svtools/optionsdrawinglayer.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_GEOMETRY2DEXTRACTOR_HXX
#include <drawinglayer/processor3d/geometry2dextractor.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence ScenePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence aRetval;

            // create 2D shadows from contained 3D primitives. This creates the shadow primitives on demand and tells if
            // there are some or not. Do this at start, the shadow might still be visible even when the scene is not
            if(impGetShadow3D(rViewInformation))
            {
                // test visibility
                const basegfx::B2DRange aShadow2DRange(getB2DRangeFromPrimitive2DSequence(maShadowPrimitives, rViewInformation));

                if(aShadow2DRange.overlaps(rViewInformation.getViewport()))
                {
                    // add extracted 2d shadows (before 3d scene creations itself)
                    aRetval = maShadowPrimitives;
                }
            }

            // get geometry of output range
            basegfx::B2DPolygon aLogicOutline(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
            aLogicOutline.transform(getObjectTransformation());

            // clip it against ViewRange
            const basegfx::B2DPolyPolygon aLogicClippedOutline(basegfx::tools::clipPolygonOnRange(aLogicOutline, rViewInformation.getViewport(), true, false));
            const basegfx::B2DRange aLogicVisiblePart(basegfx::tools::getRange(aLogicClippedOutline));

            if(!aLogicVisiblePart.isEmpty())
            {
                // transform back to unity
                basegfx::B2DHomMatrix aInverseTransform(getObjectTransformation());
                basegfx::B2DPolyPolygon aUnitClippedOutline(aLogicClippedOutline);
                aInverseTransform.invert();
                aUnitClippedOutline.transform(aInverseTransform);

                // get logical size by decomposing
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                getObjectTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

                // get logical sizes and generate visible part in unit coordinates
                double fLogicSizeX(aScale.getX());
                double fLogicSizeY(aScale.getY());
                const basegfx::B2DRange aUnitVisiblePart(basegfx::tools::getRange(aUnitClippedOutline));

                // test if view size (pixel) maybe too big and limit it
                const double fViewSizeX(fLogicSizeX * (rViewInformation.getViewTransformation() * basegfx::B2DVector(aUnitVisiblePart.getWidth(), 0.0)).getLength());
                const double fViewSizeY(fLogicSizeY * (rViewInformation.getViewTransformation() * basegfx::B2DVector(0.0, aUnitVisiblePart.getHeight())).getLength());
                const double fViewVisibleArea(fViewSizeX * fViewSizeY);
                const SvtOptionsDrawinglayer aDrawinglayerOpt;
                const double fMaximumVisibleArea(aDrawinglayerOpt.GetQuadratic3DRenderLimit());
                double fReduceFactor(1.0);

                if(fViewVisibleArea > fMaximumVisibleArea)
                {
                    fReduceFactor = sqrt(fMaximumVisibleArea / fViewVisibleArea);
                    fLogicSizeX *= fReduceFactor;
                    fLogicSizeY *= fReduceFactor;
                }

                // use default 3D primitive processor to create BitmapEx for aUnitVisiblePart and process
                processor3d::DefaultProcessor3D aProcessor3D(
                    rViewInformation,
                    getTransformation3D(),
                    getSdrSceneAttribute(),
                    getSdrLightingAttribute(),
                    fLogicSizeX,
                    fLogicSizeY,
                    aUnitVisiblePart);

                aProcessor3D.processNonTransparent(getChildren3D());
                aProcessor3D.processTransparent(getChildren3D());

                const BitmapEx aNewBitmap(aProcessor3D.getBitmapEx());
                const Size aBitmapSizePixel(aNewBitmap.GetSizePixel());

                if(aBitmapSizePixel.getWidth() && aBitmapSizePixel.getHeight())
                {
                    // correct reduce-factor free logic, view-unit-aligned coordinates for the created bitmap
                    // without translation
                    basegfx::B2DPoint aViewTopLeft(aUnitVisiblePart.getMinX() * aScale.getX(), aUnitVisiblePart.getMinY() * aScale.getY());
                    aViewTopLeft *= rViewInformation.getViewTransformation();
                    aViewTopLeft.setX(floor(aViewTopLeft.getX()));
                    aViewTopLeft.setY(floor(aViewTopLeft.getY()));
                    const basegfx::B2DPoint aLogicTopLeft(rViewInformation.getInverseViewTransformation() * aViewTopLeft);

                    // same for size, do not forget to correct by fReduceFactor
                    const basegfx::B2DVector aViewSize(aBitmapSizePixel.getWidth() / fReduceFactor, aBitmapSizePixel.getHeight() / fReduceFactor);
                    const basegfx::B2DVector aLogicSize(rViewInformation.getInverseViewTransformation() * aViewSize);

                    // create 2d transform from it. Target is to always have a output-pixel-aligned
                    // logic position and size wich corresponds to the bitmap size (when no reduceFactor)
                    basegfx::B2DHomMatrix aNew2DTransform;
                    aNew2DTransform.scale(aLogicSize.getX(), aLogicSize.getY());
                    aNew2DTransform.translate(aLogicTopLeft.getX(), aLogicTopLeft.getY());
                    aNew2DTransform.shearX(fShearX);
                    aNew2DTransform.rotate(fRotate);
                    aNew2DTransform.translate(aTranslate.getX(), aTranslate.getY());

                    // create bitmap primitive and add
                    BitmapPrimitive2D* pNewTextBitmap = new BitmapPrimitive2D(aNewBitmap, aNew2DTransform);
                    const Primitive2DReference xRef(pNewTextBitmap);
                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, xRef);

#ifdef DBG_UTIL
                    {
                        // check if pixel size and transformation is correct
                        if(basegfx::fTools::equal(fReduceFactor, 1.0))
                        {
                            if(basegfx::fTools::equalZero(fShearX) && basegfx::fTools::equalZero(fRotate))
                            {
                                basegfx::B2DVector aScaleB, aTranslateB;
                                double fRotateB, fShearXB;

                                pNewTextBitmap->getTransform().decompose(aScaleB, aTranslateB, fRotateB, fShearXB);
                                basegfx::B2DVector aViewSizeB(basegfx::absolute(aScaleB));
                                aViewSizeB *= rViewInformation.getViewTransformation();
                                const Size aSourceSizePixel(pNewTextBitmap->getBitmapEx().GetSizePixel());
                                const bool bXEqual(aSourceSizePixel.getWidth() == basegfx::fround(aViewSizeB.getX()));
                                const bool bYEqual(aSourceSizePixel.getHeight() == basegfx::fround(aViewSizeB.getY()));
                                OSL_ENSURE(bXEqual && bYEqual, "3D renderer produced non-pixel-aligned graphic (!)");
                            }
                        }
                    }
#endif
                }
            }

            // create 2D labels from contained 3D label primitives. This creates the label primitives on demand and tells if
            // there are some or not. Do this at end, the labels might still be visible even when the scene is not
            if(impGetLabel3D(rViewInformation))
            {
                // test visibility
                const basegfx::B2DRange aLabel2DRange(getB2DRangeFromPrimitive2DSequence(maLabelPrimitives, rViewInformation));

                if(aLabel2DRange.overlaps(rViewInformation.getViewport()))
                {
                    // add extracted 2d labels (after 3d scene creations)
                    appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, maLabelPrimitives);
                }
            }

            return aRetval;
        }

        Primitive2DSequence ScenePrimitive2D::getGeometry2D(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence aRetval;

            // create 2D shadows from contained 3D primitives
            if(impGetShadow3D(rViewInformation))
            {
                // add extracted 2d shadows (before 3d scene creations itself)
                aRetval = maShadowPrimitives;
            }

            // create 2D projected geometry from 3D geometry
            if(getChildren3D().hasElements())
            {
                // create 2D geometry extraction processor
                processor3d::Geometry2DExtractingProcessor aGeometryProcessor(
                    rViewInformation.getViewTime(),
                    getObjectTransformation(),
                    getTransformation3D().getWorldToView());

                // process local primitives
                aGeometryProcessor.process(getChildren3D());

                // fetch result and append
                Primitive2DSequence a2DExtractedPrimitives(aGeometryProcessor.getPrimitive2DSequence());
                appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, a2DExtractedPrimitives);
            }

            // create 2D labels from contained 3D label primitives
            if(impGetLabel3D(rViewInformation))
            {
                // add extracted 2d labels (after 3d scene creations)
                appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, maLabelPrimitives);
            }

            return aRetval;
        }

        ScenePrimitive2D::ScenePrimitive2D(
            const primitive3d::Primitive3DSequence& rxChildren3D,
            const attribute::SdrSceneAttribute& rSdrSceneAttribute,
            const attribute::SdrLightingAttribute& rSdrLightingAttribute,
            const basegfx::B2DHomMatrix& rObjectTransformation,
            const geometry::Transformation3D& rTransformation3D)
        :   BasePrimitive2D(),
            mxChildren3D(rxChildren3D),
            maSdrSceneAttribute(rSdrSceneAttribute),
            maSdrLightingAttribute(rSdrLightingAttribute),
            maObjectTransformation(rObjectTransformation),
            maTransformation3D(rTransformation3D),
            mbShadow3DChecked(false),
            mbLabel3DChecked(false),
            maLastViewTransformation(),
            maLastViewport()
        {
        }

        bool ScenePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const ScenePrimitive2D& rCompare = (ScenePrimitive2D&)rPrimitive;

                return (primitive3d::arePrimitive3DSequencesEqual(getChildren3D(), rCompare.getChildren3D())
                    && getSdrSceneAttribute() == rCompare.getSdrSceneAttribute()
                    && getSdrLightingAttribute() == rCompare.getSdrLightingAttribute()
                    && getObjectTransformation() == rCompare.getObjectTransformation()
                    && getTransformation3D() == rCompare.getTransformation3D());
            }

            return false;
        }

        basegfx::B2DRange ScenePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // call parent. Do not calculate using unit range since the decomposition may extend the range
            // by single pixels to level out the boundaries of the created bitmap primitives. So, use
            // the default mechanism to use the range of the sub-primitives.
            basegfx::B2DRange aRetval(BasePrimitive2D::getB2DRange(rViewInformation));

            // expand by evtl. existing shadow primitives
            if(impGetShadow3D(rViewInformation))
            {
                const basegfx::B2DRange aShadow2DRange(getB2DRangeFromPrimitive2DSequence(maShadowPrimitives, rViewInformation));

                if(!aShadow2DRange.isEmpty())
                {
                    aRetval.expand(aShadow2DRange);
                }
            }

            // expand by evtl. existing label primitives
            if(impGetLabel3D(rViewInformation))
            {
                const basegfx::B2DRange aLabel2DRange(getB2DRangeFromPrimitive2DSequence(maLabelPrimitives, rViewInformation));

                if(!aLabel2DRange.isEmpty())
                {
                    aRetval.expand(aLabel2DRange);
                }
            }

            return aRetval;
        }

        Primitive2DSequence ScenePrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // check if mpLocalDecomposition can be reused. Basic decision is based on the
            // comparison of old and new viewTransformations, if they are equal reuse is possible anyways.
            bool bNeedNewDecomposition(false);

            if(getLocalDecomposition().hasElements())
            {
                if(maLastViewport != rViewInformation.getViewport() || maLastViewTransformation != rViewInformation.getViewTransformation())
                {
                    bNeedNewDecomposition = true;
                }
            }

            if(bNeedNewDecomposition)
            {
                // here is space to find out if only the visible view area has changed (scroll)
                // and thus the object decomposition may be reused. For that, the logic visible part
                // and the view sizes need to be the same
                basegfx::B2DPolygon aWorldOutline(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
                aWorldOutline.transform(getObjectTransformation());

                // calculate last clipped visible part
                const basegfx::B2DPolyPolygon aLastLogicClippedOutline(basegfx::tools::clipPolygonOnRange(aWorldOutline, maLastViewport, true, false));
                const basegfx::B2DRange aLastLogicVisiblePart(basegfx::tools::getRange(aLastLogicClippedOutline));

                // calculate new clipped visible part
                const basegfx::B2DPolyPolygon aNewLogicClippedOutline(basegfx::tools::clipPolygonOnRange(aWorldOutline, rViewInformation.getViewport(), true, false));
                const basegfx::B2DRange aNewLogicVisiblePart(basegfx::tools::getRange(aNewLogicClippedOutline));

                if(aLastLogicVisiblePart.equal(aNewLogicVisiblePart))
                {
                    // logic visible part is the same, check for same view size to test for zooming
                    basegfx::B2DRange aLastViewVisiblePart(aLastLogicVisiblePart);
                    aLastViewVisiblePart.transform(maLastViewTransformation);
                    const basegfx::B2DVector aLastViewSize(aLastViewVisiblePart.getRange());

                    basegfx::B2DRange aNewViewVisiblePart(aNewLogicVisiblePart);
                    aNewViewVisiblePart.transform(rViewInformation.getViewTransformation());
                    const basegfx::B2DVector aNewViewSize(aNewViewVisiblePart.getRange());

                    if(aLastViewSize.equal(aNewViewSize))
                    {
                        // view size is the same, reuse possible
                        bNeedNewDecomposition = false;
                    }
                }
            }

            if(bNeedNewDecomposition)
            {
                // conditions of last local decomposition have changed, delete
                const_cast< ScenePrimitive2D* >(this)->setLocalDecomposition(Primitive2DSequence());
            }

            if(!getLocalDecomposition().hasElements())
            {
                // remember ViewRange and ViewTransformation
                const_cast< ScenePrimitive2D* >(this)->maLastViewTransformation = rViewInformation.getViewTransformation();
                const_cast< ScenePrimitive2D* >(this)->maLastViewport = rViewInformation.getViewport();
            }

            // use parent implementation
            return BasePrimitive2D::get2DDecomposition(rViewInformation);
        }

        bool ScenePrimitive2D::impGetShadow3D(const geometry::ViewInformation2D& rViewInformation) const
        {
            osl::MutexGuard aGuard( m_aMutex );

            // create on demand
            if(!mbShadow3DChecked && getChildren3D().hasElements())
            {
                // create shadow extraction processor
                const basegfx::B3DHomMatrix aWorldToEye(getTransformation3D().getOrientation() * getTransformation3D().getTransformation());
                const basegfx::B3DHomMatrix aEyeToView(getTransformation3D().getDeviceToView() * getTransformation3D().getProjection());

                processor3d::Shadow3DExtractingProcessor aShadowProcessor(
                    rViewInformation.getViewTime(),
                    getObjectTransformation(),
                    aWorldToEye,
                    aEyeToView,
                    getSdrLightingAttribute(),
                    getChildren3D(),
                    getSdrSceneAttribute().getShadowSlant());

                // process local primitives
                aShadowProcessor.process(getChildren3D());

                // fetch result and set checked flag
                const_cast< ScenePrimitive2D* >(this)->maShadowPrimitives = aShadowProcessor.getPrimitive2DSequence();
                const_cast< ScenePrimitive2D* >(this)->mbShadow3DChecked = true;
            }

            // return if there are shadow primitives
            return maShadowPrimitives.hasElements();
        }

        bool ScenePrimitive2D::impGetLabel3D(const geometry::ViewInformation2D& rViewInformation) const
        {
            osl::MutexGuard aGuard( m_aMutex );

            // create on demand
            if(!mbLabel3DChecked && getChildren3D().hasElements())
            {
                // create label extraction processor
                processor3d::Label3DExtractingProcessor aLabelProcessor(
                    rViewInformation.getViewTime(),
                    getObjectTransformation(),
                    getTransformation3D().getWorldToView());

                // process local primitives
                aLabelProcessor.process(getChildren3D());

                // fetch result and set checked flag
                const_cast< ScenePrimitive2D* >(this)->maLabelPrimitives = aLabelProcessor.getPrimitive2DSequence();
                const_cast< ScenePrimitive2D* >(this)->mbLabel3DChecked = true;
            }

            // return if there are label primitives
            return maLabelPrimitives.hasElements();
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(ScenePrimitive2D, PRIMITIVE2D_ID_SCENEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
