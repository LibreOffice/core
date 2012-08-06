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

#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PagePreviewPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence xRetval;
            Primitive2DSequence aContent(getPageContent());

            if(aContent.hasElements()
                && basegfx::fTools::more(getContentWidth(), 0.0)
                && basegfx::fTools::more(getContentHeight(), 0.0))
            {
                // the decomposed matrix will be needed
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                if(basegfx::fTools::more(aScale.getX(), 0.0) && basegfx::fTools::more(aScale.getY(), 0.0))
                {
                    // check if content overlaps with tageted size and needs to be embedded with a
                    // clipping primitive
                    const basegfx::B2DRange aRealContentRange(getB2DRangeFromPrimitive2DSequence(aContent, rViewInformation));
                    const basegfx::B2DRange aAllowedContentRange(0.0, 0.0, getContentWidth(), getContentHeight());

                    if(!aAllowedContentRange.isInside(aRealContentRange))
                    {
                        const Primitive2DReference xReferenceA(
                            new MaskPrimitive2D(
                                basegfx::B2DPolyPolygon(
                                    basegfx::tools::createPolygonFromRect(aAllowedContentRange)), aContent));
                        aContent = Primitive2DSequence(&xReferenceA, 1);
                    }

                    // create a mapping from content to object.
                    basegfx::B2DHomMatrix aPageTrans;

                    if(getKeepAspectRatio())
                    {
                        // #i101075# when keeping the aspect ratio is wanted, it is necessary to calculate
                        // an equidistant scaling in X and Y and a corresponding translation to
                        // center the output. Calculate needed scale factors
                        const double fScaleX(aScale.getX() / getContentWidth());
                        const double fScaleY(aScale.getY() / getContentHeight());

                        // to keep the aspect, use the smaller scale and adapt missing size by translation
                        if(fScaleX < fScaleY)
                        {
                            // height needs to be adapted
                            const double fNeededHeight(aScale.getY() / fScaleX);
                            const double fSpaceToAdd(fNeededHeight - getContentHeight());

                            aPageTrans.translate(0.0, fSpaceToAdd * 0.5);
                            aPageTrans.scale(fScaleX, aScale.getY() / fNeededHeight);
                        }
                        else
                        {
                            // width needs to be adapted
                            const double fNeededWidth(aScale.getX() / fScaleY);
                            const double fSpaceToAdd(fNeededWidth - getContentWidth());

                            aPageTrans.translate(fSpaceToAdd * 0.5, 0.0);
                            aPageTrans.scale(aScale.getX() / fNeededWidth, fScaleY);
                        }

                        // add the missing object transformation aspects
                        const basegfx::B2DHomMatrix aCombined(basegfx::tools::createShearXRotateTranslateB2DHomMatrix(
                            fShearX, fRotate, aTranslate.getX(), aTranslate.getY()));
                        aPageTrans = aCombined * aPageTrans;
                    }
                    else
                    {
                        // completely scale to PageObject size. Scale to unit size.
                        aPageTrans.scale(1.0/ getContentWidth(), 1.0 / getContentHeight());

                        // apply object matrix
                        aPageTrans *= getTransform();
                    }

                    // embed in necessary transformation to map from SdrPage to SdrPageObject
                    const Primitive2DReference xReferenceB(new TransformPrimitive2D(aPageTrans, aContent));
                    xRetval = Primitive2DSequence(&xReferenceB, 1);
                }
            }

            return xRetval;
        }

        PagePreviewPrimitive2D::PagePreviewPrimitive2D(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
            const basegfx::B2DHomMatrix& rTransform,
            double fContentWidth,
            double fContentHeight,
            const Primitive2DSequence& rPageContent,
            bool bKeepAspectRatio)
        :   BufferedDecompositionPrimitive2D(),
            mxDrawPage(rxDrawPage),
            maPageContent(rPageContent),
            maTransform(rTransform),
            mfContentWidth(fContentWidth),
            mfContentHeight(fContentHeight),
            mbKeepAspectRatio(bKeepAspectRatio)
        {
        }

        bool PagePreviewPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PagePreviewPrimitive2D& rCompare = static_cast< const PagePreviewPrimitive2D& >(rPrimitive);

                return (getXDrawPage() == rCompare.getXDrawPage()
                    && getPageContent() == rCompare.getPageContent()
                    && getTransform() == rCompare.getTransform()
                    && getContentWidth() == rCompare.getContentWidth()
                    && getContentHeight() == rCompare.getContentHeight()
                    && getKeepAspectRatio() == rCompare.getKeepAspectRatio());
            }

            return false;
        }

        basegfx::B2DRange PagePreviewPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation`*/) const
        {
            // nothing is allowed to stick out of a PagePreviewPrimitive, thus we
            // can quickly deliver our range here
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());
            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PagePreviewPrimitive2D, PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
