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
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/svditer.hxx>
#include <sdr/primitive2d/sdrcustomshapeprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/obj3d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>



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
            Rectangle aObjectBound(GetCustomShapeObj().GetGeoRect());
            aObjectBound += GetCustomShapeObj().GetGridOffset();
            Rectangle aTextBound(aObjectBound);
            GetCustomShapeObj().GetTextBounds(aTextBound);
            aTextBound += GetCustomShapeObj().GetGridOffset();
            basegfx::B2DRange aTextRange(aTextBound.Left(), aTextBound.Top(), aTextBound.Right(), aTextBound.Bottom());
            const basegfx::B2DRange aObjectRange(aObjectBound.Left(), aObjectBound.Top(), aObjectBound.Right(), aObjectBound.Bottom());

            // no need to correct if no extra text range
            if(aTextRange != aObjectRange)
            {
                const GeoStat& rGeoStat(GetCustomShapeObj().GetGeoStat());

                // only correct when rotation and/or shear is used
                if(rGeoStat.nShearAngle || rGeoStat.nRotationAngle )
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

                    if(rGeoStat.nShearAngle)
                    {
                        aRotMatrix.shearX(tan((36000 - rGeoStat.nShearAngle) * F_PI18000));
                    }

                    if(rGeoStat.nRotationAngle)
                    {
                        aRotMatrix.rotate((36000 - rGeoStat.nRotationAngle) * F_PI18000);
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

            // #i98072# Get shadow and text; eventually suppress the text if it's
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

            Point aGridOff = GetCustomShapeObj().GetGridOffset();

            if(pSdrObjRepresentation)
            {
                // Hack for calc, transform position of object according
                // to current zoom so as objects relative position to grid
                // appears stable
                const_cast< SdrObject* >( pSdrObjRepresentation )->SetGridOffset( aGridOff );
                SdrObjListIter aIterator(*pSdrObjRepresentation);

                while(aIterator.IsMore())
                {
                    SdrObject& rCandidate = *aIterator.Next();
                    // apply offset to each part
                    rCandidate.SetGridOffset( aGridOff );
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
                    Rectangle aObjectBound(GetCustomShapeObj().GetGeoRect());
                    // hack for calc grid sync
                    aObjectBound += GetCustomShapeObj().GetGridOffset();
                    const basegfx::B2DRange aObjectRange(aObjectBound.Left(), aObjectBound.Top(), aObjectBound.Right(), aObjectBound.Bottom());

                    // #i101684# get the text range unrotated and absolute to the object range
                    const basegfx::B2DRange aTextRange(getCorrectedTextBoundRect());

                    // Rotation before scaling
                    if(!basegfx::fTools::equalZero(GetCustomShapeObj().GetExtraTextRotation(true)))
                    {
                        basegfx::B2DVector aTranslation(0.5, 0.5);
                        aTextBoxMatrix.translate( -aTranslation.getX(), -aTranslation.getY() );
                        aTextBoxMatrix.rotate((360.0 - GetCustomShapeObj().GetExtraTextRotation(true)) * F_PI180);
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
                            aTextBoxMatrix.rotate((360.0 - fExtraTextRotation) * F_PI180);
                            aTextBoxMatrix.translate( aTranslation.getX(), aTranslation.getY() );
                        }

                        if(rGeoStat.nShearAngle)
                        {
                            aTextBoxMatrix.shearX(tan((36000 - rGeoStat.nShearAngle) * F_PI18000));
                        }

                        if(rGeoStat.nRotationAngle)
                        {
                            aTextBoxMatrix.rotate((36000 - rGeoStat.nRotationAngle) * F_PI18000);
                        }

                        // give text it's target position
                        aTextBoxMatrix.translate(aObjectRange.getMinimum().getX(), aObjectRange.getMinimum().getY());
                    }
                    else
                    {
                        aTextBoxMatrix.translate(aTextRange.getMinX(), aTextRange.getMinY());
                    }

                    // check if SdrTextWordWrapItem is set
                    bWordWrap = (static_cast<const SdrOnOffItem&>(GetCustomShapeObj().GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue();
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

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
