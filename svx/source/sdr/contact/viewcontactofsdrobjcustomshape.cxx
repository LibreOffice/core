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
#include <svx/sdr/contact/viewcontactofsdrobjcustomshape.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/svditer.hxx>
#include <svx/sdr/primitive2d/sdrcustomshapeprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/obj3d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrObjCustomShape::ViewContactOfSdrObjCustomShape(SdrObjCustomShape& rCustomShape)
        :   ViewContactOfTextObj(rCustomShape)
        {
        }

        ViewContactOfSdrObjCustomShape::~ViewContactOfSdrObjCustomShape()
        {
        }

        basegfx::B2DRange ViewContactOfSdrObjCustomShape::getCorrectedTextBoundRect() const
        {
            const Rectangle aObjectBound(GetCustomShapeObj().GetGeoRect());
            Rectangle aTextBound(aObjectBound);
            GetCustomShapeObj().GetTextBounds(aTextBound);
            basegfx::B2DRange aTextRange(aTextBound.Left(), aTextBound.Top(), aTextBound.Right(), aTextBound.Bottom());
            const basegfx::B2DRange aObjectRange(aObjectBound.Left(), aObjectBound.Top(), aObjectBound.Right(), aObjectBound.Bottom());

            // no need to correct if no extra text range
            if(aTextRange != aObjectRange)
            {
                const GeoStat& rGeoStat(GetCustomShapeObj().GetGeoStat());

                // only correct when rotation and/or shear is used
                if(rGeoStat.nShearWink || rGeoStat.nDrehWink )
                {
                    // text range needs to be corrected by
                    // aObjectRange.getCenter() - aRotObjectRange.getCenter() since it's
                    // defined differenly by using rotation around object center. Start
                    // with positive part
                    basegfx::B2DVector aTranslation(aObjectRange.getCenter());

                    // get rotated and sheared object's range
                    basegfx::B2DRange aRotObjectRange(aObjectRange);
                    basegfx::B2DHomMatrix aRotMatrix;

                    aRotMatrix.translate(-aObjectRange.getMinimum().getX(), -aObjectRange.getMinimum().getY());

                    if(rGeoStat.nShearWink)
                    {
                        aRotMatrix.shearX(tan((36000 - rGeoStat.nShearWink) * F_PI18000));
                    }

                    if(rGeoStat.nDrehWink)
                    {
                        aRotMatrix.rotate((36000 - rGeoStat.nDrehWink) * F_PI18000);
                    }

                    aRotMatrix.translate(aObjectRange.getMinimum().getX(), aObjectRange.getMinimum().getY());
                    aRotObjectRange.transform(aRotMatrix);

                    // add negative translation part
                    aTranslation -= aRotObjectRange.getCenter();

                    // create new range
                    aTextRange = basegfx::B2DRange(
                        aTextRange.getMinX() + aTranslation.getX(), aTextRange.getMinY() + aTranslation.getY(),
                        aTextRange.getMaxX() + aTranslation.getX(), aTextRange.getMaxY() + aTranslation.getY());
                }
            }

            return aTextRange;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrObjCustomShape::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SfxItemSet& rItemSet = GetCustomShapeObj().GetMergedItemSet();

            // #i98072# Get shandow and text; eventually suppress the text if it's
            // a TextPath FontworkGallery object
            const drawinglayer::attribute::SdrShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrShadowTextAttribute(
                    rItemSet,
                    GetCustomShapeObj().getText(0),
                    GetCustomShapeObj().IsTextPath()));
            drawinglayer::primitive2d::Primitive2DSequence xGroup;
            bool bHasText(!aAttribute.getText().isDefault());

            // create Primitive2DSequence from sub-geometry
            const SdrObject* pSdrObjRepresentation = GetCustomShapeObj().GetSdrObjectFromCustomShape();
            bool b3DShape(false);

            if(pSdrObjRepresentation)
            {
                SdrObjListIter aIterator(*pSdrObjRepresentation);

                while(aIterator.IsMore())
                {
                    SdrObject& rCandidate = *aIterator.Next();

                    if(!b3DShape && dynamic_cast< E3dObject* >(&rCandidate))
                    {
                        b3DShape = true;
                    }

                    const drawinglayer::primitive2d::Primitive2DSequence xNew(rCandidate.GetViewContact().getViewIndependentPrimitive2DSequence());
                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xGroup, xNew);
                }
            }

            if(bHasText || xGroup.hasElements())
            {
                // prepare text box geometry
                basegfx::B2DHomMatrix aTextBoxMatrix;
                bool bWordWrap(false);

                if(bHasText)
                {
                    // take unrotated snap rect as default, then get the
                    // unrotated text box. Rotation needs to be done centered
                    const Rectangle aObjectBound(GetCustomShapeObj().GetGeoRect());
                    const basegfx::B2DRange aObjectRange(aObjectBound.Left(), aObjectBound.Top(), aObjectBound.Right(), aObjectBound.Bottom());

                    // #i101684# get the text range unrotated and absolute to the object range
                    const basegfx::B2DRange aTextRange(getCorrectedTextBoundRect());

                    // give text object a size
                    aTextBoxMatrix.scale(aTextRange.getWidth(), aTextRange.getHeight());

                    // check if we have a rotation/shear at all to take care of
                    const double fExtraTextRotation(GetCustomShapeObj().GetExtraTextRotation());
                    const GeoStat& rGeoStat(GetCustomShapeObj().GetGeoStat());

                    if(rGeoStat.nShearWink || rGeoStat.nDrehWink || !basegfx::fTools::equalZero(fExtraTextRotation))
                    {
                        if(aObjectRange != aTextRange)
                        {
                            // move relative to unrotated object range
                            aTextBoxMatrix.translate(
                                aTextRange.getMinX() - aObjectRange.getMinimum().getX(),
                                aTextRange.getMinY() - aObjectRange.getMinimum().getY());
                        }

                        if(!basegfx::fTools::equalZero(fExtraTextRotation))
                        {
                            basegfx::B2DVector aTranslation(
                                ( aTextRange.getWidth() / 2 ) + ( aTextRange.getMinX() - aObjectRange.getMinimum().getX() ),
                                ( aTextRange.getHeight() / 2 ) + ( aTextRange.getMinY() - aObjectRange.getMinimum().getY() ) );
                            aTextBoxMatrix.translate( -aTranslation.getX(), -aTranslation.getY() );
                            aTextBoxMatrix.rotate((360.0 - fExtraTextRotation) * F_PI180);
                            aTextBoxMatrix.translate( aTranslation.getX(), aTranslation.getY() );
                        }

                        if(rGeoStat.nShearWink)
                        {
                            aTextBoxMatrix.shearX(tan((36000 - rGeoStat.nShearWink) * F_PI18000));
                        }

                        if(rGeoStat.nDrehWink)
                        {
                            aTextBoxMatrix.rotate((36000 - rGeoStat.nDrehWink) * F_PI18000);
                        }

                        // give text it's target position
                        aTextBoxMatrix.translate(aObjectRange.getMinimum().getX(), aObjectRange.getMinimum().getY());
                    }
                    else
                    {
                        aTextBoxMatrix.translate(aTextRange.getMinX(), aTextRange.getMinY());
                    }

                    // check if SdrTextWordWrapItem is set
                    bWordWrap = ((SdrTextWordWrapItem&)(GetCustomShapeObj().GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue();
                }

                // create primitive
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::SdrCustomShapePrimitive2D(
                        aAttribute,
                        xGroup,
                        aTextBoxMatrix,
                        bWordWrap,
                        b3DShape,
                        false));        // #SJ# New parameter to force to clipped BlockText for SC
                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }

            // #119863# always append an invisible outline for the cases where no visible content exists
            if(true)
            {
                const Rectangle aObjectBound(GetCustomShapeObj().GetGeoRect());
                const basegfx::B2DRange aObjectRange(
                    aObjectBound.Left(), aObjectBound.Top(),
                    aObjectBound.Right(), aObjectBound.Bottom());

                // create object matrix
                const GeoStat& rGeoStat(GetCustomShapeObj().GetGeoStat());
                const double fShearX(rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0);
                const double fRotate(rGeoStat.nDrehWink ? (36000 - rGeoStat.nDrehWink) * F_PI18000 : 0.0);
                const basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                    aObjectRange.getWidth(), aObjectRange.getHeight(), fShearX, fRotate,
                    aObjectRange.getMinX(), aObjectRange.getMinY()));

                drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval,
                    drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                        false,
                        aObjectMatrix));
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
