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


#include <svx/sdr/contact/viewcontactofsdrcaptionobj.hxx>
#include <svx/svdocapt.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive2d/sdrcaptionprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////
// includes for special text box shadow (SC)

#include <svl/itemset.hxx>
#include <svx/xhatch.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfltrit.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrCaptionObj::ViewContactOfSdrCaptionObj(SdrCaptionObj& rCaptionObj)
        :   ViewContactOfSdrRectObj(rCaptionObj)
        {
        }

        ViewContactOfSdrCaptionObj::~ViewContactOfSdrCaptionObj()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrCaptionObj::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrCaptionObj& rCaptionObj(GetCaptionObj());
            const SfxItemSet& rItemSet = rCaptionObj.GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    rCaptionObj.getText(0)));

            // take unrotated snap rect (direct model data) for position and size
            const Rectangle& rRectangle = rCaptionObj.GetGeoRect();
            const ::basegfx::B2DRange aObjectRange(
                rRectangle.Left(), rRectangle.Top(),
                rRectangle.Right(), rRectangle.Bottom());
            const GeoStat& rGeoStat(rCaptionObj.GetGeoStat());

            // fill object matrix
            basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                aObjectRange.getWidth(), aObjectRange.getHeight(),
                rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0,
                rGeoStat.nDrehWink ? (36000 - rGeoStat.nDrehWink) * F_PI18000 : 0.0,
                aObjectRange.getMinX(), aObjectRange.getMinY()));

            // calculate corner radius
            double fCornerRadiusX;
            double fCornerRadiusY;
            drawinglayer::primitive2d::calculateRelativeCornerRadius(
                rCaptionObj.GetEckenradius(), aObjectRange, fCornerRadiusX, fCornerRadiusY);

            // create primitive. Always create one (even if invisible) to let the decomposition
            // of SdrCaptionPrimitive2D create needed invisible elements for HitTest and BoundRect
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::SdrCaptionPrimitive2D(
                    aObjectMatrix,
                    aAttribute,
                    rCaptionObj.getTailPolygon(),
                    fCornerRadiusX,
                    fCornerRadiusY));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);

            if(!aAttribute.isDefault() && rCaptionObj.GetSpecialTextBoxShadow())
            {
                // for SC, the caption object may have a specialized shadow. The usual object shadow is off
                // and a specialized shadow gets created here (see old paint)
                const SdrShadowColorItem& rShadColItem = (SdrShadowColorItem&)(rItemSet.Get(SDRATTR_SHADOWCOLOR));
                const sal_uInt16 nTransp(((SdrShadowTransparenceItem&)(rItemSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue());
                const Color aShadCol(rShadColItem.GetColorValue());
                const XFillStyle eStyle = ((XFillStyleItem&)(rItemSet.Get(XATTR_FILLSTYLE))).GetValue();

                // Create own ItemSet and modify as needed
                // Always hide lines for special calc shadow
                SfxItemSet aSet(rItemSet);
                aSet.Put(XLineStyleItem(XLINE_NONE));

                if(XFILL_HATCH == eStyle)
                {
                    // #41666# Hatch color is set hard to shadow color
                    XHatch aHatch = ((XFillHatchItem&)(rItemSet.Get(XATTR_FILLHATCH))).GetHatchValue();
                    aHatch.SetColor(aShadCol);
                    aSet.Put(XFillHatchItem(String(),aHatch));
                }
                else
                {
                    if(XFILL_NONE != eStyle && XFILL_SOLID != eStyle)
                    {
                        // force fill to solid (for Gradient and Bitmap)
                        aSet.Put(XFillStyleItem(XFILL_SOLID));
                    }

                    aSet.Put(XFillColorItem(String(),aShadCol));
                    aSet.Put(XFillTransparenceItem(nTransp));
                }

                // crete FillAttribute from modified ItemSet
                const drawinglayer::attribute::SdrFillAttribute aFill(
                    drawinglayer::primitive2d::createNewSdrFillAttribute(aSet));
                drawinglayer::primitive2d::Primitive2DReference xSpecialShadow;

                if(!aFill.isDefault() && 1.0 != aFill.getTransparence())
                {
                    // add shadow offset to object matrix
                    const sal_uInt32 nXDist(((SdrShadowXDistItem&)(rItemSet.Get(SDRATTR_SHADOWXDIST))).GetValue());
                    const sal_uInt32 nYDist(((SdrShadowYDistItem&)(rItemSet.Get(SDRATTR_SHADOWYDIST))).GetValue());
                    aObjectMatrix.translate(nXDist, nYDist);

                    // create unit outline polygon as geometry (see SdrCaptionPrimitive2D::create2DDecomposition)
                    basegfx::B2DPolygon aUnitOutline(basegfx::tools::createPolygonFromRect(
                        basegfx::B2DRange(0.0, 0.0, 1.0, 1.0), fCornerRadiusX, fCornerRadiusY));

                    // create the specialized shadow primitive
                    xSpecialShadow = drawinglayer::primitive2d::createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        aObjectMatrix,
                        aFill,
                        drawinglayer::attribute::FillGradientAttribute());
                }

                if(xSpecialShadow.is())
                {
                    // if we really got a special shadow, create a two-element retval with the shadow
                    // behind the standard object's geometry
                    xRetval.realloc(2);

                    xRetval[0] = xSpecialShadow;
                    xRetval[1] = xReference;
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
