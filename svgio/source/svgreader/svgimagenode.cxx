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

#include <svgio/svgreader/svgimagenode.hxx>
#include <svgio/svgreader/svgdocument.hxx>
#include <sax/tools/converter.hxx>
#include <tools/stream.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graphicfilter.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <rtl/uri.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

namespace svgio
{
    namespace svgreader
    {
        SvgImageNode::SvgImageNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenRect, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maSvgAspectRatio(),
            mpaTransform(nullptr),
            maX(0),
            maY(0),
            maWidth(0),
            maHeight(0),
            maXLink(),
            maUrl(),
            maMimeType(),
            maData()
        {
        }

        SvgImageNode::~SvgImageNode()
        {
            delete mpaTransform;
        }

        const SvgStyleAttributes* SvgImageNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle("image", maSvgStyleAttributes);
        }

        void SvgImageNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(rTokenName, aSVGToken, aContent, false);

            // parse own
            switch(aSVGToken)
            {
                case SVGTokenStyle:
                {
                    readLocalCssStyle(aContent);
                    break;
                }
                case SVGTokenPreserveAspectRatio:
                {
                    setSvgAspectRatio(readSvgAspectRatio(aContent));
                    break;
                }
                case SVGTokenTransform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(&aMatrix);
                    }
                    break;
                }
                case SVGTokenX:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setX(aNum);
                    }
                    break;
                }
                case SVGTokenY:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setY(aNum);
                    }
                    break;
                }
                case SVGTokenWidth:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setWidth(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenHeight:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setHeight(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenXlinkHref:
                {
                    const sal_Int32 nLen(aContent.getLength());

                    if(nLen)
                    {
                        readImageLink(aContent, maXLink, maUrl, maMimeType, maData);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void extractFromGraphic(
            const Graphic& rGraphic,
            drawinglayer::primitive2d::Primitive2DSequence& rEmbedded,
            basegfx::B2DRange& rViewBox,
            BitmapEx& rBitmapEx)
        {
            if(GRAPHIC_BITMAP == rGraphic.GetType())
            {
                if(rGraphic.getSvgData().get())
                {
                    // embedded Svg
                    rEmbedded = rGraphic.getSvgData()->getPrimitive2DSequence();

                    // fill aViewBox
                    rViewBox = rGraphic.getSvgData()->getRange();
                }
                else
                {
                    // get bitmap
                    rBitmapEx = rGraphic.GetBitmapEx();
                }
            }
            else
            {
                // evtl. convert to bitmap
                rBitmapEx = rGraphic.GetBitmapEx();
            }
        }

        void SvgImageNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool /*bReferenced*/) const
        {
            // get size range and create path
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(pStyle && getWidth().isSet() && getHeight().isSet())
            {
                const double fWidth(getWidth().solve(*this, xcoordinate));
                const double fHeight(getHeight().solve(*this, ycoordinate));

                if(fWidth > 0.0 && fHeight > 0.0)
                {
                    BitmapEx aBitmapEx;
                    drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                    // prepare Target and ViewBox for evtl. AspectRatio mappings
                    const double fX(getX().isSet() ? getX().solve(*this, xcoordinate) : 0.0);
                    const double fY(getY().isSet() ? getY().solve(*this, ycoordinate) : 0.0);
                    const basegfx::B2DRange aTarget(fX, fY, fX + fWidth, fY + fHeight);
                    basegfx::B2DRange aViewBox(aTarget);

                    if(!maMimeType.isEmpty() && !maData.isEmpty())
                    {
                        // use embedded base64 encoded data
                        css::uno::Sequence< sal_Int8 > aPass;
                        ::sax::Converter::decodeBase64(aPass, maData);

                        if(aPass.hasElements())
                        {
                            SvMemoryStream aStream(aPass.getArray(), aPass.getLength(), StreamMode::READ);
                            Graphic aGraphic;

                            if(GRFILTER_OK == GraphicFilter::GetGraphicFilter().ImportGraphic(
                                aGraphic,
                                OUString(),
                                aStream))
                            {
                                extractFromGraphic(aGraphic, aNewTarget, aViewBox, aBitmapEx);
                            }
                        }
                    }
                    else if(!maUrl.isEmpty())
                    {
                        const OUString& rPath = getDocument().getAbsolutePath();
                        OUString aAbsUrl;
                        try {
                            aAbsUrl = rtl::Uri::convertRelToAbs(rPath, maUrl);
                        } catch (rtl::MalformedUriException & e) {
                            SAL_WARN(
                                "svg",
                                "caught rtl::MalformedUriException \""
                                    << e.getMessage() << "\"");
                        }

                        if (!aAbsUrl.isEmpty() && !rPath.equals(aAbsUrl))
                        {
                            SvFileStream aStream(aAbsUrl, STREAM_STD_READ);
                            Graphic aGraphic;

                            if(GRFILTER_OK == GraphicFilter::GetGraphicFilter().ImportGraphic(
                                   aGraphic,
                                   aAbsUrl,
                                   aStream))
                            {
                                extractFromGraphic(aGraphic, aNewTarget, aViewBox, aBitmapEx);
                            }
                        }
                    }
                    else if(!maXLink.isEmpty())
                    {
                        const SvgNode* mpXLink = getDocument().findSvgNodeById(maXLink);

                        if(mpXLink && Display_none != mpXLink->getDisplay())
                        {
                            mpXLink->decomposeSvgNode(aNewTarget, true);

                            if(aNewTarget.hasElements())
                            {
                                aViewBox = drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                    aNewTarget,
                                    drawinglayer::geometry::ViewInformation2D());
                            }
                        }
                    }

                    if(!aBitmapEx.IsEmpty())
                    {
                        // create content from created bitmap
                        aNewTarget.realloc(1);
                        aNewTarget[0] = new drawinglayer::primitive2d::BitmapPrimitive2D(
                            aBitmapEx,
                            basegfx::B2DHomMatrix());

                        // fill aViewBox. No size set yet, use unit size
                        aViewBox = basegfx::B2DRange(0.0, 0.0, 1.0, 1.0);
                    }

                    if(aNewTarget.hasElements())
                    {
                        if(aTarget.equal(aViewBox))
                        {
                            // just add to rTarget
                            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aNewTarget);
                        }
                        else
                        {
                            // create mapping
                            const SvgAspectRatio& rRatio = getSvgAspectRatio();

                            if(rRatio.isSet())
                            {
                                // let mapping be created from SvgAspectRatio
                                const basegfx::B2DHomMatrix aEmbeddingTransform(rRatio.createMapping(aTarget, aViewBox));

                                if(!aEmbeddingTransform.isIdentity())
                                {
                                    const drawinglayer::primitive2d::Primitive2DReference xRef(
                                        new drawinglayer::primitive2d::TransformPrimitive2D(
                                            aEmbeddingTransform,
                                            aNewTarget));

                                    aNewTarget = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
                                }

                                if(!rRatio.isMeetOrSlice())
                                {
                                    // need to embed in MaskPrimitive2D to ensure clipping
                                    const drawinglayer::primitive2d::Primitive2DReference xMask(
                                        new drawinglayer::primitive2d::MaskPrimitive2D(
                                            basegfx::B2DPolyPolygon(
                                                basegfx::tools::createPolygonFromRect(aTarget)),
                                            aNewTarget));

                                    aNewTarget = drawinglayer::primitive2d::Primitive2DSequence(&xMask, 1);
                                }
                            }
                            else
                            {
                                // choose default mapping
                                const basegfx::B2DHomMatrix aEmbeddingTransform(SvgAspectRatio::createLinearMapping(aTarget, aViewBox));

                                if(!aEmbeddingTransform.isIdentity())
                                {
                                    const drawinglayer::primitive2d::Primitive2DReference xRef(
                                        new drawinglayer::primitive2d::TransformPrimitive2D(
                                            aEmbeddingTransform,
                                            aNewTarget));

                                    aNewTarget = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
                                }
                            }

                            // embed and add to rTarget, take local extra-transform into account
                            pStyle->add_postProcess(rTarget, aNewTarget, getTransform());
                        }
                    }
                }
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
