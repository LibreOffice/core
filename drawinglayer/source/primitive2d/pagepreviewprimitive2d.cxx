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
#include <utility>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        Primitive2DReference PagePreviewPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DContainer aContent(getPageContent());

            if(!(!aContent.empty()
                && getContentWidth() > 0.0)
                && getContentHeight() > 0.0)
                return nullptr;

            // the decomposed matrix will be needed
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

            if(!(aScale.getX() > 0.0 && aScale.getY() > 0.0))
                return nullptr;

            // check if content overlaps with target size and needs to be embedded with a
            // clipping primitive
            const basegfx::B2DRange aRealContentRange(aContent.getB2DRange(rViewInformation));
            const basegfx::B2DRange aAllowedContentRange(0.0, 0.0, getContentWidth(), getContentHeight());

            if(!aAllowedContentRange.isInside(aRealContentRange))
            {
                const Primitive2DReference xReferenceA(
                    new MaskPrimitive2D(
                        basegfx::B2DPolyPolygon(
                            basegfx::utils::createPolygonFromRect(aAllowedContentRange)), std::move(aContent)));
                aContent = Primitive2DContainer { xReferenceA };
            }

            // create a mapping from content to object.
            basegfx::B2DHomMatrix aPageTrans;

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
            const basegfx::B2DHomMatrix aCombined(basegfx::utils::createShearXRotateTranslateB2DHomMatrix(
                    fShearX, fRotate, aTranslate.getX(), aTranslate.getY()));
            aPageTrans = aCombined * aPageTrans;

            // embed in necessary transformation to map from SdrPage to SdrPageObject
            return new TransformPrimitive2D(aPageTrans, std::move(aContent));
        }

        PagePreviewPrimitive2D::PagePreviewPrimitive2D(
            css::uno::Reference< css::drawing::XDrawPage > xDrawPage,
            basegfx::B2DHomMatrix aTransform,
            double fContentWidth,
            double fContentHeight,
            Primitive2DContainer&& rPageContent)
        :   mxDrawPage(std::move(xDrawPage)),
            maPageContent(std::move(rPageContent)),
            maTransform(std::move(aTransform)),
            mfContentWidth(fContentWidth),
            mfContentHeight(fContentHeight)
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
                    && getContentHeight() == rCompare.getContentHeight());
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
        sal_uInt32 PagePreviewPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
