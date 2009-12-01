/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdrobjcustomshape.cxx,v $
 * $Revision: 1.7.18.1 $
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
#include <svx/sdr/contact/viewcontactofsdrobjcustomshape.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <svditer.hxx>
#include <svx/sdr/primitive2d/sdrcustomshapeprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/obj3d.hxx>

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
                const double fExtraTextRotation(GetCustomShapeObj().GetExtraTextRotation());
                const GeoStat& rGeoStat(GetCustomShapeObj().GetGeoStat());

                // only correct when rotation and/or shear is used
                if(rGeoStat.nShearWink || rGeoStat.nDrehWink || !basegfx::fTools::equalZero(fExtraTextRotation))
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

                    if(!basegfx::fTools::equalZero(fExtraTextRotation))
                    {
                        aRotMatrix.rotate((360.0 - fExtraTextRotation) * F_PI180);
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
            SdrText* pSdrText = GetCustomShapeObj().getText(0);

            if(pSdrText)
            {
                // #i98072# Get shandow and text; eventually suppress the text if it's
                // a TextPath FontworkGallery object
                drawinglayer::attribute::SdrShadowTextAttribute* pAttribute = drawinglayer::primitive2d::createNewSdrShadowTextAttribute(
                    rItemSet,
                    *pSdrText,
                    GetCustomShapeObj().IsTextPath());
                drawinglayer::primitive2d::Primitive2DSequence xGroup;
                bool bHasText(pAttribute && pAttribute->getText());

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

                            if(rGeoStat.nShearWink)
                            {
                                aTextBoxMatrix.shearX(tan((36000 - rGeoStat.nShearWink) * F_PI18000));
                            }

                            if(rGeoStat.nDrehWink)
                            {
                                aTextBoxMatrix.rotate((36000 - rGeoStat.nDrehWink) * F_PI18000);
                            }

                            if(!basegfx::fTools::equalZero(fExtraTextRotation))
                            {
                                aTextBoxMatrix.rotate((360.0 - fExtraTextRotation) * F_PI180);
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

                    // make sure a (even empty) SdrShadowTextAttribute exists for
                    // primitive creation
                    if(!pAttribute)
                    {
                        pAttribute = new drawinglayer::attribute::SdrShadowTextAttribute(0L, 0L);
                    }

                    // create primitive
                    const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::SdrCustomShapePrimitive2D(
                        *pAttribute,
                        xGroup,
                        aTextBoxMatrix,
                        bWordWrap,
                        false,          // #SJ# New parameter to force to clipped BlockText for SC
                        b3DShape));
                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }

                if(pAttribute)
                {
                    delete pAttribute;
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
