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

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrObjCustomShape::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SfxItemSet& rItemSet = GetCustomShapeObj().GetMergedItemSet();
            SdrText* pSdrText = GetCustomShapeObj().getText(0);

            if(pSdrText)
            {
                drawinglayer::attribute::SdrShadowTextAttribute* pAttribute = drawinglayer::primitive2d::createNewSdrShadowTextAttribute(rItemSet, *pSdrText);
                drawinglayer::primitive2d::Primitive2DSequence xGroup;
                bool bHasText(pAttribute && pAttribute->getText());

                // create Primitive2DSequence from sub-geometry
                const SdrObject* pSdrObjRepresentation = GetCustomShapeObj().GetSdrObjectFromCustomShape();

                if(pSdrObjRepresentation)
                {
                    SdrObjListIter aIterator(*pSdrObjRepresentation);

                    while(aIterator.IsMore())
                    {
                        SdrObject& rCandidate = *aIterator.Next();
                        const drawinglayer::primitive2d::Primitive2DSequence xNew(rCandidate.GetViewContact().getViewIndependentPrimitive2DSequence());
                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xGroup, xNew);
                    }
                }

                if(bHasText || xGroup.hasElements())
                {
                    // prepare text box geometry
                    ::basegfx::B2DHomMatrix aTextBoxMatrix;
                    bool bWordWrap(false);

                    if(bHasText)
                    {
                        // take unrotated snap rect as default, then get the
                        // unrotated text box. Rotation needs to be done centered
                        Rectangle aTextBound(GetCustomShapeObj().GetGeoRect());
                        GetCustomShapeObj().GetTextBounds(aTextBound);
                        const ::basegfx::B2DRange aTextBoxRange(aTextBound.Left(), aTextBound.Top(), aTextBound.Right(), aTextBound.Bottom());

                        // fill object matrix
                        if(!::basegfx::fTools::equalZero(aTextBoxRange.getWidth()))
                        {
                            aTextBoxMatrix.set(0, 0, aTextBoxRange.getWidth());
                        }

                        if(!::basegfx::fTools::equalZero(aTextBoxRange.getHeight()))
                        {
                            aTextBoxMatrix.set(1, 1, aTextBoxRange.getHeight());
                        }

                        const double fExtraTextRotation(GetCustomShapeObj().GetExtraTextRotation());
                        const GeoStat& rGeoStat(GetCustomShapeObj().GetGeoStat());

                        if(rGeoStat.nShearWink || rGeoStat.nDrehWink || !::basegfx::fTools::equalZero(fExtraTextRotation))
                        {
                            const double fHalfWidth(aTextBoxRange.getWidth() * 0.5);
                            const double fHalfHeight(aTextBoxRange.getHeight() * 0.5);

                            // move to it's own center to rotate around it
                            aTextBoxMatrix.translate(-fHalfWidth, -fHalfHeight);

                            if(rGeoStat.nShearWink)
                            {
                                aTextBoxMatrix.shearX(tan((36000 - rGeoStat.nShearWink) * F_PI18000));
                            }

                            if(rGeoStat.nDrehWink)
                            {
                                aTextBoxMatrix.rotate((36000 - rGeoStat.nDrehWink) * F_PI18000);
                            }

                            if(!::basegfx::fTools::equalZero(fExtraTextRotation))
                            {
                                aTextBoxMatrix.rotate((360.0 - fExtraTextRotation) * F_PI180);
                            }

                            // move back
                            aTextBoxMatrix.translate(fHalfWidth, fHalfHeight);
                        }

                        aTextBoxMatrix.translate(aTextBoxRange.getMinX(), aTextBoxRange.getMinY());

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
                        *pAttribute, xGroup, aTextBoxMatrix, bWordWrap));
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
