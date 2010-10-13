/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_drawinglayer.hxx"

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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
