/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#include <PlaceholderDecoration.hxx>

#include <numeric>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <officecfg/Office/Impress.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <vcl/font.hxx>

#include <sdpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>

namespace sd
{
drawinglayer::primitive2d::Primitive2DContainer
createPlaceholderDecoration(SdrObject& rObject, bool bSubContentProcessing)
{
    drawinglayer::primitive2d::Primitive2DContainer aRetval;

    SdrPage* pSdrPage = rObject.getSdrPageFromSdrObject();
    if (!pSdrPage)
        return aRetval;

    const SdPage* pObjectsSdPage = dynamic_cast<SdPage*>(pSdrPage);
    PresObjKind eKind(PresObjKind::NONE);

    bool bCreateOutline(false);

    if (rObject.IsEmptyPresObj() && DynCastSdrTextObj(&rObject) != nullptr)
    {
        if (!bSubContentProcessing || !rObject.IsNotVisibleAsMaster())
        {
            eKind = pObjectsSdPage ? pObjectsSdPage->GetPresObjKind(&rObject) : PresObjKind::NONE;
            bCreateOutline = true;
        }
    }
    else if ((rObject.GetObjInventor() == SdrInventor::Default)
             && (rObject.GetObjIdentifier() == SdrObjKind::Text))
    {
        if (pObjectsSdPage)
        {
            eKind = pObjectsSdPage->GetPresObjKind(&rObject);

            if ((eKind == PresObjKind::Footer) || (eKind == PresObjKind::Header)
                || (eKind == PresObjKind::DateTime) || (eKind == PresObjKind::SlideNumber))
            {
                if (!bSubContentProcessing)
                {
                    // only draw a boundary for header&footer objects on the masterpage itself
                    bCreateOutline = true;
                }
            }
        }
    }
    else if ((rObject.GetObjInventor() == SdrInventor::Default)
             && (rObject.GetObjIdentifier() == SdrObjKind::Page))
    {
        // only for handout page, else this frame will be created for each
        // page preview object in SlideSorter and PagePane
        if (pObjectsSdPage && PageKind::Handout == pObjectsSdPage->GetPageKind())
        {
            bCreateOutline = true;
        }
    }
    if (bCreateOutline)
        bCreateOutline = officecfg::Office::Impress::Misc::TextObject::ShowBoundary::get();

    if (bCreateOutline)
    {
        // empty presentation objects get a gray frame
        const svtools::ColorConfig aColorConfig;
        const svtools::ColorConfigValue aColor(aColorConfig.GetColorValue(svtools::DOCBOUNDARIES));

        // get basic object transformation
        const basegfx::BColor aRGBColor(aColor.nColor.getBColor());
        basegfx::B2DHomMatrix aObjectMatrix;
        basegfx::B2DPolyPolygon aObjectPolyPolygon;
        rObject.TRGetBaseGeometry(aObjectMatrix, aObjectPolyPolygon);

        // create dashed border
        {
            // create object polygon
            basegfx::B2DPolygon aPolygon(basegfx::utils::createUnitPolygon());
            aPolygon.transform(aObjectMatrix);

            // create line and stroke attribute
            ::std::vector<double> aDotDashArray{ 160.0, 80.0 };

            const double fFullDotDashLen(
                ::std::accumulate(aDotDashArray.begin(), aDotDashArray.end(), 0.0));
            const drawinglayer::attribute::LineAttribute aLine(aRGBColor);
            drawinglayer::attribute::StrokeAttribute aStroke(std::move(aDotDashArray),
                                                             fFullDotDashLen);

            // create primitive and add
            const drawinglayer::primitive2d::Primitive2DReference xRef(
                new drawinglayer::primitive2d::PolygonStrokePrimitive2D(std::move(aPolygon), aLine,
                                                                        std::move(aStroke)));
            aRetval.push_back(xRef);
        }

        // now paint the placeholder description, but only when masterpage
        // is displayed as page directly (MasterPage view)
        if (!bSubContentProcessing && pSdrPage->IsMasterPage())
        {
            OUString aObjectString;

            switch (eKind)
            {
                case PresObjKind::Title:
                {
                    if (pObjectsSdPage && pObjectsSdPage->GetPageKind() == PageKind::Standard)
                    {
                        static OUString aTitleAreaStr(SdResId(STR_PLACEHOLDER_DESCRIPTION_TITLE));
                        aObjectString = aTitleAreaStr;
                    }

                    break;
                }
                case PresObjKind::Outline:
                {
                    static OUString aOutlineAreaStr(SdResId(STR_PLACEHOLDER_DESCRIPTION_OUTLINE));
                    aObjectString = aOutlineAreaStr;
                    break;
                }
                case PresObjKind::Footer:
                {
                    static OUString aFooterAreaStr(SdResId(STR_PLACEHOLDER_DESCRIPTION_FOOTER));
                    aObjectString = aFooterAreaStr;
                    break;
                }
                case PresObjKind::Header:
                {
                    static OUString aHeaderAreaStr(SdResId(STR_PLACEHOLDER_DESCRIPTION_HEADER));
                    aObjectString = aHeaderAreaStr;
                    break;
                }
                case PresObjKind::DateTime:
                {
                    static OUString aDateTimeStr(SdResId(STR_PLACEHOLDER_DESCRIPTION_DATETIME));
                    aObjectString = aDateTimeStr;
                    break;
                }
                case PresObjKind::Notes:
                {
                    static OUString aDateTimeStr(SdResId(STR_PLACEHOLDER_DESCRIPTION_NOTES));
                    aObjectString = aDateTimeStr;
                    break;
                }
                case PresObjKind::SlideNumber:
                {
                    if (pObjectsSdPage && pObjectsSdPage->GetPageKind() == PageKind::Standard)
                    {
                        static OUString aSlideAreaStr(SdResId(STR_PLACEHOLDER_DESCRIPTION_SLIDE));
                        aObjectString = aSlideAreaStr;
                    }
                    else
                    {
                        static OUString aNumberAreaStr(SdResId(STR_PLACEHOLDER_DESCRIPTION_NUMBER));
                        aObjectString = aNumberAreaStr;
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }

            if (!aObjectString.isEmpty())
            {
                // decompose object matrix to be able to place text correctly
                basegfx::B2DTuple aScale;
                basegfx::B2DTuple aTranslate;
                double fRotate, fShearX;
                aObjectMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

                // create font
                SdrTextObj* pTextObj = DynCastSdrTextObj(&rObject);
                const SdrTextVertAdjust eTVA(pTextObj ? pTextObj->GetTextVerticalAdjust()
                                                      : SDRTEXTVERTADJUST_CENTER);
                vcl::Font aScaledVclFont;

                // use a text size factor to get more reliable text sizes from the text layouter
                // (and from vcl), tipp from HDU
                static const sal_uInt32 nTextSizeFactor(100);

                // use a factor to get more linear text size calculations
                aScaledVclFont.SetFontHeight(500 * nTextSizeFactor);

                // get basic geometry and get text size
                drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;
                aTextLayouter.setFont(aScaledVclFont);
                const sal_Int32 nTextLength(aObjectString.getLength());

                // do not forget to use the factor again to get the width for the 500
                const double fTextWidth(aTextLayouter.getTextWidth(aObjectString, 0, nTextLength)
                                        * (1.0 / nTextSizeFactor));
                const double fTextHeight(aTextLayouter.getTextHeight() * (1.0 / nTextSizeFactor));

                // calculate text primitive position. If text is at bottom, use top for
                // the extra text and vice versa
                const double fHorDist(125);
                const double fVerDist(125);
                const double fPosX((aTranslate.getX() + aScale.getX()) - fTextWidth - fHorDist);
                const double fPosY((SDRTEXTVERTADJUST_BOTTOM == eTVA)
                                       ? aTranslate.getY() - fVerDist + fTextHeight
                                       : (aTranslate.getY() + aScale.getY()) - fVerDist);

                // get font attributes; use normally scaled font
                vcl::Font aVclFont;
                basegfx::B2DVector aTextSizeAttribute;

                aVclFont.SetFontHeight(500);

                drawinglayer::attribute::FontAttribute aFontAttribute(
                    drawinglayer::primitive2d::getFontAttributeFromVclFont(aTextSizeAttribute,
                                                                           aVclFont, false, false));

                // fill text matrix
                const basegfx::B2DHomMatrix aTextMatrix(
                    basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                        aTextSizeAttribute.getX(), aTextSizeAttribute.getY(), fShearX, fRotate,
                        fPosX, fPosY));

                // create DXTextArray (can be empty one)
                ::std::vector<double> aDXArray{};

                // create locale; this may need some more information in the future
                css::lang::Locale aLocale;

                // create primitive and add
                const drawinglayer::primitive2d::Primitive2DReference xRef(
                    new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                        aTextMatrix, aObjectString, 0, nTextLength, std::move(aDXArray), {},
                        std::move(aFontAttribute), std::move(aLocale), aRGBColor));
                aRetval.push_back(xRef);
            }
        }
    }
    return aRetval;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
