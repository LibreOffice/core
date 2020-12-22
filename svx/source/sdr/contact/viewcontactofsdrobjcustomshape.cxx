/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sdr/contact/viewcontactofsdrobjcustomshape.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sdooitm.hxx>
#include <sdr/primitive2d/sdrattributecreator.hxx>
#include <sdr/primitive2d/sdrcustomshapeprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/obj3d.hxx>
#include <vcl/canvastools.hxx>


namespace sdr::contact
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
            const tools::Rectangle aObjectBound(GetCustomShapeObj().GetGeoRect());
            tools::Rectangle aTextBound(aObjectBound);
            GetCustomShapeObj().GetTextBounds(aTextBound);
            basegfx::B2DRange aTextRange = vcl::unotools::b2DRectangleFromRectangle(aTextBound);
            const basegfx::B2DRange aObjectRange = vcl::unotools::b2DRectangleFromRectangle(aObjectBound);

            // no need to correct if no extra text range
            if(aTextRange != aObjectRange)
            {
                const GeoStat& rGeoStat(GetCustomShapeObj().GetGeoStat());

                // only correct when rotation and/or shear is used
                if(rGeoStat.nShearAngle || rGeoStat.nRotationAngle )
                {
                    // text range needs to be corrected by
                    // aObjectRange.getCenter() - aRotObjectRange.getCenter() since it's
                    // defined differently by using rotation around object center. Start
                    // with positive part
                    basegfx::B2DVector aTranslation(aObjectRange.getCenter());

                    // get rotated and sheared object's range
                    basegfx::B2DRange aRotObjectRange(aObjectRange);
                    basegfx::B2DHomMatrix aRotMatrix;

                    aRotMatrix.translate(-aObjectRange.getMinimum().getX(), -aObjectRange.getMinimum().getY());

                    if(rGeoStat.nShearAngle)
                    {
                        aRotMatrix.shearX(-rGeoStat.mfTanShearAngle);
                    }

                    if(rGeoStat.nRotationAngle)
                    {
                        aRotMatrix.rotate((36000 - rGeoStat.nRotationAngle.get()) * F_PI18000);
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

                // NbcMirror() of SdrTextObj (from which SdrObjCustomShape is derived), adds a
                // 180deg rotation around the shape center to GeoStat.nRotationAngle. So remove here the
                // 180° rotation, which was added by GetTextBounds().
                if(GetCustomShapeObj().IsMirroredY())
                {
                    basegfx::B2DHomMatrix aRotMatrix(basegfx::utils::createRotateAroundPoint(
                        aObjectRange.getCenterX(), aObjectRange.getCenterY(), F_PI));
                    aTextRange.transform(aRotMatrix);
                }
            }

            return aTextRange;
        }

        drawinglayer::primitive2d::Primitive2DContainer ViewContactOfSdrObjCustomShape::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DContainer xRetval;
            const SfxItemSet& rItemSet = GetCustomShapeObj().GetMergedItemSet();

            // #i98072# Get shadow and text; eventually suppress the text if it's
            // a TextPath FontworkGallery object
            const drawinglayer::attribute::SdrEffectsTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrEffectsTextAttribute(
                    rItemSet,
                    GetCustomShapeObj().getText(0),
                    GetCustomShapeObj().IsTextPath()));
            drawinglayer::primitive2d::Primitive2DContainer xGroup;
            bool bHasText(!aAttribute.getText().isDefault());

            // create Primitive2DContainer from sub-geometry
            const SdrObject* pSdrObjRepresentation = GetCustomShapeObj().GetSdrObjectFromCustomShape();
            bool b3DShape(false);

            if(pSdrObjRepresentation)
            {
                // tdf#118498 The processing of SdrObjListIter for SdrIterMode::DeepNoGroups
                // did change for 3D-Objects, it now correctly enters and iterates the
                // SdrObjects in the E3dScene (same as for SdrObjGroup). This is more correct
                // as the old version which just checked for dynamic_cast<const SdrObjGroup*>
                // and *only* entered these, ignoring E3dScene as grouping-object.
                // But how to fix that? Taking back the SdrObjListIter change would be easy, but
                // not correct. After checking ViewContactOfE3dScene and ViewContactOfGroup
                // I see that both traverse their children by themselves (on VC-Level,
                // see createViewIndependentPrimitive2DSequence implementations and usage of
                // GetObjectCount()). Thus in principle iterating here (esp. 'deep') seems to
                // be wrong anyways, it might have even created wrong and double geometries
                // (only with complex CustomShapes with multiple representation SdrObjects and
                // only visible when transparency involved, but runtime-expensive).
                // Thus: Just do not iterate, will check behaviour deeply.
                b3DShape = (nullptr != dynamic_cast< const E3dObject* >(pSdrObjRepresentation));
                const drawinglayer::primitive2d::Primitive2DContainer& xNew(
                    pSdrObjRepresentation->GetViewContact().getViewIndependentPrimitive2DContainer());
                xGroup.insert(xGroup.end(), xNew.begin(), xNew.end());
            }

            if(bHasText || !xGroup.empty())
            {
                // prepare text box geometry
                basegfx::B2DHomMatrix aTextBoxMatrix;
                bool bWordWrap(false);

                // take unrotated snap rect as default, then get the
                // unrotated text box. Rotation needs to be done centered
                const tools::Rectangle aObjectBound(GetCustomShapeObj().GetGeoRect());
                const basegfx::B2DRange aObjectRange = vcl::unotools::b2DRectangleFromRectangle(aObjectBound);

                if(bHasText)
                {
                    // #i101684# get the text range unrotated and absolute to the object range
                    const basegfx::B2DRange aTextRange(getCorrectedTextBoundRect());

                    // Rotation before scaling
                    if(!basegfx::fTools::equalZero(GetCustomShapeObj().GetExtraTextRotation(true)))
                    {
                        basegfx::B2DVector aTranslation(0.5, 0.5);
                        aTextBoxMatrix.translate( -aTranslation.getX(), -aTranslation.getY() );
                        aTextBoxMatrix.rotate(basegfx::deg2rad(
                            360.0 - GetCustomShapeObj().GetExtraTextRotation(true)));
                        aTextBoxMatrix.translate( aTranslation.getX(), aTranslation.getY() );
                    }
                    // give text object a size
                    aTextBoxMatrix.scale(aTextRange.getWidth(), aTextRange.getHeight());

                    // check if we have a rotation/shear at all to take care of
                    const double fExtraTextRotation(GetCustomShapeObj().GetExtraTextRotation());
                    const GeoStat& rGeoStat(GetCustomShapeObj().GetGeoStat());

                    if(rGeoStat.nShearAngle || rGeoStat.nRotationAngle || !basegfx::fTools::equalZero(fExtraTextRotation))
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
                            aTextBoxMatrix.rotate(basegfx::deg2rad(360.0 - fExtraTextRotation));
                            aTextBoxMatrix.translate( aTranslation.getX(), aTranslation.getY() );
                        }

                        if(rGeoStat.nShearAngle)
                        {
                            aTextBoxMatrix.shearX(-rGeoStat.mfTanShearAngle);
                        }

                        if(rGeoStat.nRotationAngle)
                        {
                            aTextBoxMatrix.rotate((36000 - rGeoStat.nRotationAngle.get()) * F_PI18000);
                        }

                        // give text it's target position
                        aTextBoxMatrix.translate(aObjectRange.getMinimum().getX(), aObjectRange.getMinimum().getY());
                    }
                    else
                    {
                        aTextBoxMatrix.translate(aTextRange.getMinX(), aTextRange.getMinY());
                    }

                    // check if SdrTextWordWrapItem is set
                    bWordWrap = GetCustomShapeObj().GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue();
                }

                // fill object matrix
                const basegfx::B2DHomMatrix aObjectMatrix(basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                    aObjectRange.getWidth(), aObjectRange.getHeight(),
                    /*fShearX=*/0, /*fRotate=*/0,
                    aObjectRange.getMinX(), aObjectRange.getMinY()));

                // create primitive
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::SdrCustomShapePrimitive2D(
                        aAttribute,
                        xGroup,
                        aTextBoxMatrix,
                        bWordWrap,
                        b3DShape,
                        aObjectMatrix));
                xRetval = drawinglayer::primitive2d::Primitive2DContainer { xReference };
            }

            return xRetval;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
