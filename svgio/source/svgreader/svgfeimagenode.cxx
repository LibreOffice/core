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

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/graphicfilter.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <svgfeimagenode.hxx>
#include <o3tl/string_view.hxx>
#include <svgdocument.hxx>
#include <comphelper/base64.hxx>
#include <tools/stream.hxx>
#include <rtl/uri.hxx>

namespace svgio::svgreader
{
SvgFeImageNode::SvgFeImageNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgFilterNode(SVGToken::FeImage, rDocument, pParent)
{
}

SvgFeImageNode::~SvgFeImageNode() {}

void SvgFeImageNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
{
    // parse own
    switch (aSVGToken)
    {
        case SVGToken::Style:
        {
            readLocalCssStyle(aContent);
            break;
        }
        case SVGToken::In:
        {
            maIn = aContent.trim();
            break;
        }
        case SVGToken::Result:
        {
            maResult = aContent.trim();
            break;
        }
        case SVGToken::Href:
        case SVGToken::XlinkHref:
        {
            const sal_Int32 nLen(aContent.getLength());

            if (nLen)
            {
                OUString aXLink;
                readImageLink(aContent, aXLink, maUrl, maData);
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

void SvgFeImageNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                           const SvgFilterNode* pParent) const
{
    if (const drawinglayer::primitive2d::Primitive2DContainer* rSource
        = pParent->findGraphicSource(maIn))
    {
        rTarget = *rSource;
    }

    BitmapEx aBitmapEx;

    if (!maData.isEmpty())
    {
        // use embedded base64 encoded data
        css::uno::Sequence<sal_Int8> aPass;
        ::comphelper::Base64::decode(aPass, maData);

        if (aPass.hasElements())
        {
            SvMemoryStream aStream(aPass.getArray(), aPass.getLength(), StreamMode::READ);
            Graphic aGraphic;

            if (ERRCODE_NONE
                == GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, u"", aStream))
            {
                aBitmapEx = aGraphic.GetBitmapEx();
            }
        }
    }
    else if (!maUrl.isEmpty())
    {
        const OUString& rPath = getDocument().getAbsolutePath();
        OUString aAbsUrl;
        try
        {
            aAbsUrl = rtl::Uri::convertRelToAbs(rPath, maUrl);
        }
        catch (rtl::MalformedUriException& e)
        {
            SAL_WARN("svg", "caught rtl::MalformedUriException \"" << e.getMessage() << "\"");
        }

        if (!aAbsUrl.isEmpty() && rPath != aAbsUrl)
        {
            SvFileStream aStream(aAbsUrl, StreamMode::STD_READ);
            Graphic aGraphic;

            if (ERRCODE_NONE
                == GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aAbsUrl, aStream))
            {
                aBitmapEx = aGraphic.GetBitmapEx();
            }
        }
    }

    if (!aBitmapEx.IsEmpty() && 0 != aBitmapEx.GetSizePixel().Width()
        && 0 != aBitmapEx.GetSizePixel().Height())
    {
        basegfx::B2DRange aViewBox
            = rTarget.getB2DRange(drawinglayer::geometry::ViewInformation2D());
        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::BitmapPrimitive2D(
                aBitmapEx, basegfx::utils::createScaleTranslateB2DHomMatrix(
                               aViewBox.getRange(), aViewBox.getMinimum())));

        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }

    pParent->addGraphicSourceToMapper(maResult, rTarget);
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
