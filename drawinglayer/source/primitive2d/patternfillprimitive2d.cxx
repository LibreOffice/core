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

#include <drawinglayer/primitive2d/patternfillprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <texture/texture.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <drawinglayer/converters.hxx>

using namespace com::sun::star;

#define MAXIMUM_SQUARE_LENGTH (186.0)
#define MINIMUM_SQUARE_LENGTH (16.0)
#define MINIMUM_TILES_LENGTH (3)

namespace drawinglayer::primitive2d
{
        void PatternFillPrimitive2D::calculateNeededDiscreteBufferSize(
            sal_uInt32& rWidth,
            sal_uInt32& rHeight,
            const geometry::ViewInformation2D& rViewInformation) const
        {
            // reset parameters
            rWidth = rHeight = 0;

            // check if resolution is in the range which may be buffered
            const basegfx::B2DPolyPolygon& rMaskPolygon = getMask();
            const basegfx::B2DRange aMaskRange(rMaskPolygon.getB2DRange());

            // get discrete rounded up square size of a single tile
            const basegfx::B2DHomMatrix aMaskRangeTransformation(
                basegfx::utils::createScaleTranslateB2DHomMatrix(
                    aMaskRange.getRange(),
                    aMaskRange.getMinimum()));
            const basegfx::B2DHomMatrix aTransform(
                rViewInformation.getObjectToViewTransformation() * aMaskRangeTransformation);
            const basegfx::B2DPoint aTopLeft(aTransform * getReferenceRange().getMinimum());
            const basegfx::B2DPoint aX(aTransform * basegfx::B2DPoint(getReferenceRange().getMaxX(), getReferenceRange().getMinY()));
            const basegfx::B2DPoint aY(aTransform * basegfx::B2DPoint(getReferenceRange().getMinX(), getReferenceRange().getMaxY()));
            const double fW(basegfx::B2DVector(aX - aTopLeft).getLength());
            const double fH(basegfx::B2DVector(aY - aTopLeft).getLength());
            const double fSquare(fW * fH);

            if(fSquare <= 0.0)
                return;

            // check if less than a maximum square pixels is used
            static const sal_uInt32 fMaximumSquare(MAXIMUM_SQUARE_LENGTH * MAXIMUM_SQUARE_LENGTH);

            if(fSquare >= fMaximumSquare)
                return;

            // calculate needed number of tiles and check if used more than a minimum count
            const texture::GeoTexSvxTiled aTiling(getReferenceRange());
            const sal_uInt32 nTiles(aTiling.getNumberOfTiles());
            static const sal_uInt32 nMinimumTiles(MINIMUM_TILES_LENGTH * MINIMUM_TILES_LENGTH);

            if(nTiles < nMinimumTiles)
                return;

            rWidth = basegfx::fround(ceil(fW));
            rHeight = basegfx::fround(ceil(fH));
            static const sal_uInt32 fMinimumSquare(MINIMUM_SQUARE_LENGTH * MINIMUM_SQUARE_LENGTH);

            if(fSquare < fMinimumSquare)
            {
                const double fRel(fW/fH);
                rWidth = basegfx::fround(sqrt(fMinimumSquare * fRel));
                rHeight = basegfx::fround(sqrt(fMinimumSquare / fRel));
            }
        }

        void PatternFillPrimitive2D::getTileSize(
            sal_uInt32& rWidth,
            sal_uInt32& rHeight,
            const geometry::ViewInformation2D& rViewInformation) const
        {
            const basegfx::B2DRange aMaskRange(getMask().getB2DRange());

            // get discrete rounded up square size of a single tile
            const basegfx::B2DHomMatrix aMaskRangeTransformation(
                basegfx::utils::createScaleTranslateB2DHomMatrix(
                    aMaskRange.getRange(),
                    aMaskRange.getMinimum()));
            const basegfx::B2DHomMatrix aTransform(
                rViewInformation.getObjectToViewTransformation() * aMaskRangeTransformation);
            const basegfx::B2DPoint aTopLeft(aTransform * getReferenceRange().getMinimum());
            const basegfx::B2DPoint aX(aTransform * basegfx::B2DPoint(getReferenceRange().getMaxX(), getReferenceRange().getMinY()));
            const basegfx::B2DPoint aY(aTransform * basegfx::B2DPoint(getReferenceRange().getMinX(), getReferenceRange().getMaxY()));
            const double fW(basegfx::B2DVector(aX - aTopLeft).getLength());
            const double fH(basegfx::B2DVector(aY - aTopLeft).getLength());

            rWidth = basegfx::fround(ceil(fW));
            rHeight = basegfx::fround(ceil(fH));
        }

        Primitive2DContainer PatternFillPrimitive2D::createContent(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DContainer aContent;

            // see if buffering is wanted. If so, create buffered content in given resolution
            if(0 != mnDiscreteWidth && 0 != mnDiscreteHeight)
            {
                const geometry::ViewInformation2D aViewInformation2D;
                primitive2d::Primitive2DReference xEmbedRef(
                    new primitive2d::TransformPrimitive2D(
                        basegfx::utils::createScaleB2DHomMatrix(mnDiscreteWidth, mnDiscreteHeight),
                        Primitive2DContainer(getChildren())));
                primitive2d::Primitive2DContainer xEmbedSeq { xEmbedRef };

                const BitmapEx aBitmapEx(
                    convertToBitmapEx(
                        std::move(xEmbedSeq),
                        aViewInformation2D,
                        mnDiscreteWidth,
                        mnDiscreteHeight,
                        mnDiscreteWidth * mnDiscreteHeight));

                if(!aBitmapEx.IsEmpty())
                {
                    const Size& rBmpPix = aBitmapEx.GetSizePixel();

                    if(rBmpPix.Width() > 0 && rBmpPix.Height() > 0)
                    {
                        const primitive2d::Primitive2DReference xEmbedRefBitmap(
                            new primitive2d::BitmapPrimitive2D(
                                VCLUnoHelper::CreateVCLXBitmap(aBitmapEx),
                                basegfx::B2DHomMatrix()));
                        aContent = primitive2d::Primitive2DContainer { xEmbedRefBitmap };
                    }
                }
            }

            if(aContent.empty())
            {
                // buffering was not tried or did fail - reset remembered buffered size
                // in any case
                PatternFillPrimitive2D* pThat = const_cast< PatternFillPrimitive2D* >(this);
                pThat->mnDiscreteWidth = pThat->mnDiscreteHeight = 0;

                // use children as default context
                aContent = getChildren();

                // check if content needs to be clipped
                const basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);
                const basegfx::B2DRange aContentRange(aContent.getB2DRange(rViewInformation));

                if(!aUnitRange.isInside(aContentRange))
                {
                    const Primitive2DReference xRef(
                        new MaskPrimitive2D(
                            basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(aUnitRange)),
                            std::move(aContent)));

                    aContent = Primitive2DContainer { xRef };
                }
            }

            return aContent;
        }

        //  create buffered content in given resolution
        BitmapEx PatternFillPrimitive2D::createTileImage(sal_uInt32 nWidth, sal_uInt32 nHeight) const
        {
            const geometry::ViewInformation2D aViewInformation2D;
            Primitive2DContainer aContent(createContent(aViewInformation2D));
            const primitive2d::Primitive2DReference xEmbedRef(
                    new primitive2d::TransformPrimitive2D(
                        basegfx::utils::createScaleB2DHomMatrix(nWidth, nHeight),
                        std::move(aContent)));
            primitive2d::Primitive2DContainer xEmbedSeq { xEmbedRef };

            return convertToBitmapEx(
                        std::move(xEmbedSeq),
                        aViewInformation2D,
                        nWidth,
                        nHeight,
                        nWidth * nHeight);
        }

        void PatternFillPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DContainer aRetval;

            if(getChildren().empty())
                return;

            if(!(!getReferenceRange().isEmpty() && getReferenceRange().getWidth() > 0.0 && getReferenceRange().getHeight() > 0.0))
                return;

            const basegfx::B2DRange aMaskRange(getMask().getB2DRange());

            if(!(!aMaskRange.isEmpty() && aMaskRange.getWidth() > 0.0 && aMaskRange.getHeight() > 0.0))
                return;

            // create tiling matrices
            std::vector< basegfx::B2DHomMatrix > aMatrices;
            texture::GeoTexSvxTiled aTiling(getReferenceRange());

            aTiling.appendTransformations(aMatrices);

            // create content
            Primitive2DContainer aContent(createContent(rViewInformation));

            // resize result
            aRetval.resize(aMatrices.size());

            // create one primitive for each matrix
            for(size_t a(0); a < aMatrices.size(); a++)
            {
                aRetval[a] = new TransformPrimitive2D(
                    aMatrices[a],
                    Primitive2DContainer(aContent));
            }

            // transform result which is in unit coordinates to mask's object coordinates
            {
                const basegfx::B2DHomMatrix aMaskTransform(
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        aMaskRange.getRange(),
                        aMaskRange.getMinimum()));

                Primitive2DReference xRef(
                    new TransformPrimitive2D(
                        aMaskTransform,
                        std::move(aRetval)));

                aRetval = Primitive2DContainer { xRef };
            }

            // embed result in mask
            {
                rContainer.push_back(
                    new MaskPrimitive2D(
                        getMask(),
                        std::move(aRetval)));
            }
        }

        PatternFillPrimitive2D::PatternFillPrimitive2D(
            const basegfx::B2DPolyPolygon& rMask,
            Primitive2DContainer&& rChildren,
            const basegfx::B2DRange& rReferenceRange)
        :   maMask(rMask),
            maChildren(std::move(rChildren)),
            maReferenceRange(rReferenceRange),
            mnDiscreteWidth(0),
            mnDiscreteHeight(0)
        {
        }

        bool PatternFillPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const PatternFillPrimitive2D& rCompare = static_cast< const PatternFillPrimitive2D& >(rPrimitive);

                return (getMask() == rCompare.getMask()
                    && getChildren() == rCompare.getChildren()
                    && getReferenceRange() == rCompare.getReferenceRange());
            }

            return false;
        }

        basegfx::B2DRange PatternFillPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /* rViewInformation */ ) const
        {
            return getMask().getB2DRange();
        }

        void PatternFillPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            // The existing buffered decomposition uses a buffer in the remembered
            // size or none if sizes are zero. Get new needed sizes which depend on
            // the given ViewInformation
            bool bResetBuffering = false;
            sal_uInt32 nW(0);
            sal_uInt32 nH(0);
            calculateNeededDiscreteBufferSize(nW, nH, rViewInformation);
            const bool bBufferingCurrentlyUsed(0 != mnDiscreteWidth && 0 != mnDiscreteHeight);
            const bool bBufferingNextUsed(0 != nW && 0 != nH);

            if(bBufferingNextUsed)
            {
                // buffering is now possible
                if(bBufferingCurrentlyUsed)
                {
                    if(nW > mnDiscreteWidth || nH > mnDiscreteHeight)
                    {
                        // Higher resolution is needed than used in the existing buffered
                        // decomposition - create new one
                        bResetBuffering = true;
                    }
                    else if(double(nW * nH) / double(mnDiscreteWidth * mnDiscreteHeight) <= 0.5)
                    {
                        // Size has shrunk for 50% or more - it's worth to refresh the buffering
                        // to spare some resources
                        bResetBuffering = true;
                    }
                }
                else
                {
                    // currently no buffering used - reset evtl. unbuffered
                    // decomposition to start buffering
                    bResetBuffering = true;
                }
            }
            else
            {
                // buffering is no longer possible
                if(bBufferingCurrentlyUsed)
                {
                    // reset decomposition to allow creation of unbuffered one
                    bResetBuffering = true;
                }
            }

            if(bResetBuffering)
            {
                PatternFillPrimitive2D* pThat = const_cast< PatternFillPrimitive2D* >(this);
                pThat->mnDiscreteWidth = nW;
                pThat->mnDiscreteHeight = nH;
                pThat->setBuffered2DDecomposition(Primitive2DContainer());
            }

            // call parent
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        sal_Int64 SAL_CALL PatternFillPrimitive2D::estimateUsage()
        {
            size_t nRet(0);
            for (auto& it : getChildren())
                nRet += it->estimateUsage();
            return nRet;
        }

        // provide unique ID
        sal_uInt32 PatternFillPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_PATTERNFILLPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
