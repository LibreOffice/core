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


#include <sdr/contact/viewcontactofsdrcaptionobj.hxx>
#include <svx/svdocapt.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <sdr/primitive2d/sdrcaptionprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>


// includes for special text box shadow (SC)

#include <svl/itemset.hxx>
#include <svx/xhatch.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfltrit.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>

using namespace com::sun::star;

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

        drawinglayer::primitive2d::Primitive2DContainer ViewContactOfSdrCaptionObj::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DContainer xRetval;
            const SdrCaptionObj& rCaptionObj(GetCaptionObj());
            const SfxItemSet& rItemSet = rCaptionObj.GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    rCaptionObj.getText(0),
                    false));

            // take unrotated snap rect (direct model data) for position and size
            Rectangle rRectangle = rCaptionObj.GetGeoRect();
            // Hack for calc, transform position of object according
            // to current zoom so as objects relative position to grid
            // appears stable
            Point aGridOff = rCaptionObj.GetGridOffset();
            rRectangle += aGridOff;

            const ::basegfx::B2DRange aObjectRange(
                rRectangle.Left(), rRectangle.Top(),
                rRectangle.Right(), rRectangle.Bottom());
            const GeoStat& rGeoStat(rCaptionObj.GetGeoStat());

            // fill object matrix
            basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                aObjectRange.getWidth(), aObjectRange.getHeight(),
                rGeoStat.nShearAngle ? tan((36000 - rGeoStat.nShearAngle) * F_PI18000) : 0.0,
                rGeoStat.nRotationAngle ? (36000 - rGeoStat.nRotationAngle) * F_PI18000 : 0.0,
                aObjectRange.getMinX(), aObjectRange.getMinY()));

            // calculate corner radius
            double fCornerRadiusX;
            double fCornerRadiusY;
            drawinglayer::primitive2d::calculateRelativeCornerRadius(
                rCaptionObj.GetEckenradius(), aObjectRange, fCornerRadiusX, fCornerRadiusY);
            ::basegfx::B2DPolygon aTail = rCaptionObj.getTailPolygon();
            // Hack for calc, transform position of tail according
            // to current zoom so as objects relative position to grid
            // appears stable
            aTail.transform( basegfx::tools::createTranslateB2DHomMatrix( aGridOff.X(), aGridOff.Y() ) );
            // create primitive. Always create one (even if invisible) to let the decomposition
            // of SdrCaptionPrimitive2D create needed invisible elements for HitTest and BoundRect
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::SdrCaptionPrimitive2D(
                    aObjectMatrix,
                    aAttribute,
                    aTail,
                    fCornerRadiusX,
                    fCornerRadiusY));

            xRetval = drawinglayer::primitive2d::Primitive2DContainer { xReference };

            if(!aAttribute.isDefault() && rCaptionObj.GetSpecialTextBoxShadow())
            {
                // for SC, the caption object may have a specialized shadow. The usual object shadow is off
                // and a specialized shadow gets created here (see old paint)
                const XColorItem& rShadColItem = static_cast<const XColorItem&>(rItemSet.Get(SDRATTR_SHADOWCOLOR));
                const sal_uInt16 nShadowTransparence((static_cast<const SdrPercentItem&>(rItemSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue());
                const Color aShadowColor(rShadColItem.GetColorValue());
                const drawing::FillStyle eShadowStyle = (static_cast<const XFillStyleItem&>(rItemSet.Get(XATTR_FILLSTYLE))).GetValue();

                // Create own ItemSet and modify as needed
                // Always hide lines for special calc shadow
                SfxItemSet aSet(rItemSet);
                aSet.Put(XLineStyleItem(drawing::LineStyle_NONE));

                if(drawing::FillStyle_HATCH == eShadowStyle)
                {
                    // #41666# Hatch color is set hard to shadow color
                    XHatch aHatch = (static_cast<const XFillHatchItem&>(rItemSet.Get(XATTR_FILLHATCH))).GetHatchValue();
                    aHatch.SetColor(aShadowColor);
                    aSet.Put(XFillHatchItem(OUString(),aHatch));
                }
                else
                {
                    if(drawing::FillStyle_SOLID != eShadowStyle)
                    {
                        // force fill to solid (for Gradient, Bitmap and *no* fill (#119750# not filled comments *have* shadow))
                        aSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
                    }

                    aSet.Put(XFillColorItem(OUString(),aShadowColor));
                    aSet.Put(XFillTransparenceItem(nShadowTransparence));
                }

                // create FillAttribute from modified ItemSet
                const drawinglayer::attribute::SdrFillAttribute aFill(
                    drawinglayer::primitive2d::createNewSdrFillAttribute(aSet));
                drawinglayer::primitive2d::Primitive2DReference xSpecialShadow;

                if(!aFill.isDefault() && 1.0 != aFill.getTransparence())
                {
                    // add shadow offset to object matrix
                    const sal_uInt32 nXDist((static_cast<const SdrMetricItem&>(rItemSet.Get(SDRATTR_SHADOWXDIST))).GetValue());
                    const sal_uInt32 nYDist((static_cast<const SdrMetricItem&>(rItemSet.Get(SDRATTR_SHADOWYDIST))).GetValue());

                    if(nXDist || nYDist)
                    {
                        // #119750# create object and shadow outline, clip shadow outline
                        // on object outline. If there is a rest, create shadow. Do this to
                        // emulate that shadow is *not* visible behind the object for
                        // transparent object fill for comments in excel
                        basegfx::B2DPolygon aObjectOutline(
                            basegfx::tools::createPolygonFromRect(
                                basegfx::B2DRange(0.0, 0.0, 1.0, 1.0),
                                fCornerRadiusX,
                                fCornerRadiusY));
                        aObjectOutline.transform(aObjectMatrix);

                        // create shadow outline
                        basegfx::B2DPolygon aShadowOutline(aObjectOutline);
                        aShadowOutline.transform(
                            basegfx::tools::createTranslateB2DHomMatrix(nXDist, nYDist));

                        // clip shadow outline against object outline
                        const basegfx::B2DPolyPolygon aClippedShadow(
                            basegfx::tools::clipPolygonOnPolyPolygon(
                                aShadowOutline,
                                basegfx::B2DPolyPolygon(aObjectOutline),
                                false, // take the outside
                                false));

                        if(aClippedShadow.count())
                        {
                            // if there is shadow, create the specialized shadow primitive
                            xSpecialShadow = drawinglayer::primitive2d::createPolyPolygonFillPrimitive(
                                aClippedShadow,
                                aFill,
                                drawinglayer::attribute::FillGradientAttribute());
                        }
                    }
                }

                if(xSpecialShadow.is())
                {
                    // if we really got a special shadow, create a two-element retval with the shadow
                    // behind the standard object's geometry
                    xRetval.resize(2);

                    xRetval[0] = xSpecialShadow;
                    xRetval[1] = xReference;
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
