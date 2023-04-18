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

#include <basegfx/utils/gradienttools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/awt/Gradient2.hpp>
#include <osl/endian.h>

#include <algorithm>
#include <cmath>

namespace basegfx
{
    bool ODFGradientInfo::operator==(const ODFGradientInfo& rODFGradientInfo) const
    {
        return getTextureTransform() == rODFGradientInfo.getTextureTransform()
            && getAspectRatio() == rODFGradientInfo.getAspectRatio()
            && getRequestedSteps() == rODFGradientInfo.getRequestedSteps();
    }

    const B2DHomMatrix& ODFGradientInfo::getBackTextureTransform() const
    {
        if(maBackTextureTransform.isIdentity())
        {
            const_cast< ODFGradientInfo* >(this)->maBackTextureTransform = getTextureTransform();
            const_cast< ODFGradientInfo* >(this)->maBackTextureTransform.invert();
        }

        return maBackTextureTransform;
    }

    /** Most of the setup for linear & axial gradient is the same, except
        for the border treatment. Factored out here.
    */
    static ODFGradientInfo init1DGradientInfo(
        const B2DRange& rTargetRange,
        sal_uInt32 nSteps,
        double fBorder,
        double fAngle,
        bool bAxial)
    {
        B2DHomMatrix aTextureTransform;

        fAngle = -fAngle;

        double fTargetSizeX(rTargetRange.getWidth());
        double fTargetSizeY(rTargetRange.getHeight());
        double fTargetOffsetX(rTargetRange.getMinX());
        double fTargetOffsetY(rTargetRange.getMinY());

        // add object expansion
        const bool bAngleUsed(!fTools::equalZero(fAngle));

        if(bAngleUsed)
        {
            const double fAbsCos(fabs(cos(fAngle)));
            const double fAbsSin(fabs(sin(fAngle)));
            const double fNewX(fTargetSizeX * fAbsCos + fTargetSizeY * fAbsSin);
            const double fNewY(fTargetSizeY * fAbsCos + fTargetSizeX * fAbsSin);

            fTargetOffsetX -= (fNewX - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fNewY - fTargetSizeY) / 2.0;
            fTargetSizeX = fNewX;
            fTargetSizeY = fNewY;
        }

        const double fSizeWithoutBorder(1.0 - fBorder);

        if(bAxial)
        {
            aTextureTransform.scale(1.0, fSizeWithoutBorder * 0.5);
            aTextureTransform.translate(0.0, 0.5);
        }
        else
        {
            if(!fTools::equal(fSizeWithoutBorder, 1.0))
            {
                aTextureTransform.scale(1.0, fSizeWithoutBorder);
                aTextureTransform.translate(0.0, fBorder);
            }
        }

        aTextureTransform.scale(fTargetSizeX, fTargetSizeY);

        // add texture rotate after scale to keep perpendicular angles
        if(bAngleUsed)
        {
            const B2DPoint aCenter(0.5 * fTargetSizeX, 0.5 * fTargetSizeY);

            aTextureTransform *= basegfx::utils::createRotateAroundPoint(aCenter, fAngle);
        }

        // add object translate
        aTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        const double fAspectRatio(fTools::equalZero(fTargetSizeY) ?  1.0 : fTargetSizeX / fTargetSizeY);

        return ODFGradientInfo(aTextureTransform, fAspectRatio, nSteps);
    }

    /** Most of the setup for radial & ellipsoidal gradient is the same,
        except for the border treatment. Factored out here.
    */
    static ODFGradientInfo initEllipticalGradientInfo(
        const B2DRange& rTargetRange,
        const B2DVector& rOffset,
        sal_uInt32 nSteps,
        double fBorder,
        double fAngle,
        bool bCircular)
    {
        B2DHomMatrix aTextureTransform;

        fAngle = -fAngle;

        double fTargetSizeX(rTargetRange.getWidth());
        double fTargetSizeY(rTargetRange.getHeight());
        double fTargetOffsetX(rTargetRange.getMinX());
        double fTargetOffsetY(rTargetRange.getMinY());

        // add object expansion
        if(bCircular)
        {
            const double fOriginalDiag(std::hypot(fTargetSizeX, fTargetSizeY));

            fTargetOffsetX -= (fOriginalDiag - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fOriginalDiag - fTargetSizeY) / 2.0;
            fTargetSizeX = fOriginalDiag;
            fTargetSizeY = fOriginalDiag;
        }
        else
        {
            fTargetOffsetX -= ((M_SQRT2 - 1) / 2.0 ) * fTargetSizeX;
            fTargetOffsetY -= ((M_SQRT2 - 1) / 2.0 ) * fTargetSizeY;
            fTargetSizeX = M_SQRT2 * fTargetSizeX;
            fTargetSizeY = M_SQRT2 * fTargetSizeY;
        }

        const double fHalfBorder((1.0 - fBorder) * 0.5);

        aTextureTransform.scale(fHalfBorder, fHalfBorder);
        aTextureTransform.translate(0.5, 0.5);
        aTextureTransform.scale(fTargetSizeX, fTargetSizeY);

        // add texture rotate after scale to keep perpendicular angles
        if(!bCircular && !fTools::equalZero(fAngle))
        {
            const B2DPoint aCenter(0.5 * fTargetSizeX, 0.5 * fTargetSizeY);

            aTextureTransform *= basegfx::utils::createRotateAroundPoint(aCenter, fAngle);
        }

        // add defined offsets after rotation
        if(!fTools::equal(0.5, rOffset.getX()) || !fTools::equal(0.5, rOffset.getY()))
        {
            // use original target size
            fTargetOffsetX += (rOffset.getX() - 0.5) * rTargetRange.getWidth();
            fTargetOffsetY += (rOffset.getY() - 0.5) * rTargetRange.getHeight();
        }

        // add object translate
        aTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        const double fAspectRatio(fTargetSizeY == 0.0 ? 1.0 : (fTargetSizeX / fTargetSizeY));

        return ODFGradientInfo(aTextureTransform, fAspectRatio, nSteps);
    }

    /** Setup for rect & square gradient is exactly the same. Factored out
        here.
    */
    static ODFGradientInfo initRectGradientInfo(
        const B2DRange& rTargetRange,
        const B2DVector& rOffset,
        sal_uInt32 nSteps,
        double fBorder,
        double fAngle,
        bool bSquare)
    {
        B2DHomMatrix aTextureTransform;

        fAngle = -fAngle;

        double fTargetSizeX(rTargetRange.getWidth());
        double fTargetSizeY(rTargetRange.getHeight());
        double fTargetOffsetX(rTargetRange.getMinX());
        double fTargetOffsetY(rTargetRange.getMinY());

        // add object expansion
        if(bSquare)
        {
            const double fSquareWidth(std::max(fTargetSizeX, fTargetSizeY));

            fTargetOffsetX -= (fSquareWidth - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fSquareWidth - fTargetSizeY) / 2.0;
            fTargetSizeX = fTargetSizeY = fSquareWidth;
        }

        // add object expansion
        const bool bAngleUsed(!fTools::equalZero(fAngle));

        if(bAngleUsed)
        {
            const double fAbsCos(fabs(cos(fAngle)));
            const double fAbsSin(fabs(sin(fAngle)));
            const double fNewX(fTargetSizeX * fAbsCos + fTargetSizeY * fAbsSin);
            const double fNewY(fTargetSizeY * fAbsCos + fTargetSizeX * fAbsSin);

            fTargetOffsetX -= (fNewX - fTargetSizeX) / 2.0;
            fTargetOffsetY -= (fNewY - fTargetSizeY) / 2.0;
            fTargetSizeX = fNewX;
            fTargetSizeY = fNewY;
        }

        const double fHalfBorder((1.0 - fBorder) * 0.5);

        aTextureTransform.scale(fHalfBorder, fHalfBorder);
        aTextureTransform.translate(0.5, 0.5);
        aTextureTransform.scale(fTargetSizeX, fTargetSizeY);

        // add texture rotate after scale to keep perpendicular angles
        if(bAngleUsed)
        {
            const B2DPoint aCenter(0.5 * fTargetSizeX, 0.5 * fTargetSizeY);

            aTextureTransform *= basegfx::utils::createRotateAroundPoint(aCenter, fAngle);
        }

        // add defined offsets after rotation
        if(!fTools::equal(0.5, rOffset.getX()) || !fTools::equal(0.5, rOffset.getY()))
        {
            // use original target size
            fTargetOffsetX += (rOffset.getX() - 0.5) * rTargetRange.getWidth();
            fTargetOffsetY += (rOffset.getY() - 0.5) * rTargetRange.getHeight();
        }

        // add object translate
        aTextureTransform.translate(fTargetOffsetX, fTargetOffsetY);

        // prepare aspect for texture
        const double fAspectRatio(fTargetSizeY == 0.0 ? 1.0 : (fTargetSizeX / fTargetSizeY));

        return ODFGradientInfo(aTextureTransform, fAspectRatio, nSteps);
    }

    namespace utils
    {
        /* Internal helper to convert ::Color from tools::color.hxx to BColor
           without the need to link against tools library. Be on the
           safe side by using the same union
        */
        namespace {
        struct ColorToBColorConverter
        {
            union {
                sal_uInt32 mValue;
                struct {
#ifdef OSL_BIGENDIAN
                    sal_uInt8 T;
                    sal_uInt8 R;
                    sal_uInt8 G;
                    sal_uInt8 B;
#else
                    sal_uInt8 B;
                    sal_uInt8 G;
                    sal_uInt8 R;
                    sal_uInt8 T;
#endif
                };
            };

            ColorToBColorConverter(sal_uInt32 nColor) : mValue(nColor) { T=0; }
            BColor getBColor() const
            {
                return BColor(R / 255.0, G / 255.0, B / 255.0);
            }
        };
        }

        /// Tooling method to fill awt::Gradient2 from data contained in the given Any
        bool fillGradient2FromAny(css::awt::Gradient2& rGradient, const css::uno::Any& rVal)
        {
            bool bRetval(false);

            if (rVal.has< css::awt::Gradient2 >())
            {
                // we can use awt::Gradient2 directly
                bRetval = (rVal >>= rGradient);
            }
            else if (rVal.has< css::awt::Gradient >())
            {
                // 1st get awt::Gradient
                css::awt::Gradient aTmp;

                if (rVal >>= aTmp)
                {
                    // copy all awt::Gradient data to awt::Gradient2
                    rGradient.Style = aTmp.Style;
                    rGradient.StartColor = aTmp.StartColor;
                    rGradient.EndColor = aTmp.EndColor;
                    rGradient.Angle = aTmp.Angle;
                    rGradient.Border = aTmp.Border;
                    rGradient.XOffset = aTmp.XOffset;
                    rGradient.YOffset = aTmp.YOffset;
                    rGradient.StartIntensity = aTmp.StartIntensity;
                    rGradient.EndIntensity = aTmp.EndIntensity;
                    rGradient.StepCount = aTmp.StepCount;

                    // complete data by creating ColorStops for awt::Gradient2
                    fillColorStopSequenceFromColorStops(
                        rGradient.ColorStops,
                        ColorStops {
                            ColorStop(0.0, ColorToBColorConverter(aTmp.StartColor).getBColor()),
                            ColorStop(1.0, ColorToBColorConverter(aTmp.EndColor).getBColor()) });
                    bRetval = true;
                }
            }

            return bRetval;
        }

        /* Tooling method to extract data from given awt::Gradient2
           to ColorStops, doing some corrections, partitally based
           on given SingleColor.
           This will do quite some preparations for the gradient
           as follows:
           - It will check for single color (resetting rSingleColor when
             this is the case) and return with empty ColorStops
           - It will blend ColorStops to Intensity if StartIntensity/
             EndIntensity != 100 is set in awt::Gradient2, so applying
             that value(s) to the gadient directly
           - It will adapt to Border if Border != 0 is set at the
             given awt::Gradient2, so applying that value to the gadient
             directly
        */
        void prepareColorStops(
            const com::sun::star::awt::Gradient2& rGradient,
            ColorStops& rColorStops,
            BColor& rSingleColor)
        {
            fillColorStopsFromGradient2(rColorStops, rGradient);

            if (isSingleColor(rColorStops, rSingleColor))
            {
                // when single color, preserve value in rSingleColor
                // and clear the ColorStops, done.
                rColorStops.clear();
                return;
            }

            if (rGradient.StartIntensity != 100 || rGradient.EndIntensity != 100)
            {
                // apply 'old' blend stuff, blend against black
                blendColorStopsToIntensity(
                    rColorStops,
                    rGradient.StartIntensity * 0.01,
                    rGradient.EndIntensity * 0.01,
                    basegfx::BColor()); // COL_BLACK

                // can lead to single color (e.g. both zero, so all black),
                // so check again
                if (isSingleColor(rColorStops, rSingleColor))
                {
                    rColorStops.clear();
                    return;
                }
            }

            if (rGradient.Border != 0)
            {
                // apply Border if set
                // NOTE: no new start node is added. The new ColorStop
                //       mechanism does not need entries at 0.0 and 1.0.
                //       In case this is needed, do that in the caller
                const double fFactor(rGradient.Border * 0.01);
                ColorStops aNewStops;

                for (const auto& candidate : rColorStops)
                {
                    if (css::awt::GradientStyle_AXIAL == rGradient.Style)
                    {
                        // for axial add the 'gap' at the start due to reverse used gradient
                        aNewStops.emplace_back((1.0 - fFactor) * candidate.getStopOffset(), candidate.getStopColor());
                    }
                    else
                    {
                        // css::awt::GradientStyle_LINEAR
                        // case awt::GradientStyle_RADIAL
                        // case awt::GradientStyle_ELLIPTICAL
                        // case awt::GradientStyle_RECT
                        // case awt::GradientStyle_SQUARE

                        // for all others add the 'gap' at the end
                        aNewStops.emplace_back(fFactor + (candidate.getStopOffset() * (1.0 - fFactor)), candidate.getStopColor());
                    }
                }

                rColorStops = aNewStops;
            }
        }

        /* Tooling method to synchronize the given ColorStops.
           The intention is that a color GradientStops and an
           alpha/transparence GradientStops gets synchronized
           for export.
           Fo the corrections the single values for color and
           alpha may be used, e.g. when ColorStops is given
           and not empty, but AlphaStops is empty, it will get
           sycronized so that it will have the same number and
           offsets in AlphaStops as in ColorStops, but with
           the given SingleAlpha as value.
           At return it guarantees that both have the same
           number of entries with the same StopOffsets, so
           that synchonized pair of ColorStops can e.g. be used
           to export a Gradient with defined/adapted alpha
           being 'coupled' indirectly using the
           'FillTransparenceGradient' method (at import time).
        */
        void synchronizeColorStops(
            ColorStops& rColorStops,
            ColorStops& rAlphaStops,
            const BColor& rSingleColor,
            const BColor& rSingleAlpha)
        {
            if (rColorStops.empty())
            {
                if (rAlphaStops.empty())
                {
                    // no AlphaStops and no ColorStops
                    // create two-stop fallbacks for both
                    rColorStops = ColorStops {
                        ColorStop(0.0, rSingleColor),
                        ColorStop(1.0, rSingleColor) };
                    rAlphaStops = ColorStops {
                        ColorStop(0.0, rSingleAlpha),
                        ColorStop(1.0, rSingleAlpha) };
                }
                else
                {
                    // AlphaStops but no ColorStops
                    // create fallback synched with existing AlphaStops
                    for (const auto& cand : rAlphaStops)
                    {
                        rColorStops.emplace_back(cand.getStopOffset(), rSingleColor);
                    }
                }

                // preparations complete, we are done
                return;
            }
            else if (rAlphaStops.empty())
            {
                // ColorStops but no AlphaStops
                // create fallback AlphaStops synched with existing ColorStops using SingleAlpha
                for (const auto& cand : rColorStops)
                {
                    rAlphaStops.emplace_back(cand.getStopOffset(), rSingleAlpha);
                }

                // preparations complete, we are done
                return;
            }

            // here we have ColorStops and AlphaStops not empty. Check if we need to
            // synchronize both or if they are already usable/in a synched state so
            // that they have same count and same StopOffsets
            bool bNeedToSyncronize(rColorStops.size() != rAlphaStops.size());

            if (!bNeedToSyncronize)
            {
                // check for same StopOffsets
                ColorStops::const_iterator aCurrColor(rColorStops.begin());
                ColorStops::const_iterator aCurrAlpha(rAlphaStops.begin());

                while (!bNeedToSyncronize &&
                    aCurrColor != rColorStops.end() &&
                    aCurrAlpha != rAlphaStops.end())
                {
                    if (fTools::equal(aCurrColor->getStopOffset(), aCurrAlpha->getStopOffset()))
                    {
                        aCurrColor++;
                        aCurrAlpha++;
                    }
                    else
                    {
                        bNeedToSyncronize = true;
                    }
                }
            }

            if (bNeedToSyncronize)
            {
                // synchronize sizes & StopOffsets
                ColorStops::const_iterator aCurrColor(rColorStops.begin());
                ColorStops::const_iterator aCurrAlpha(rAlphaStops.begin());
                ColorStops aNewColor;
                ColorStops aNewAlpha;
                ColorStopRange aColorStopRange;
                ColorStopRange aAlphaStopRange;
                bool bRealChange(false);

                do {
                    const bool bColor(aCurrColor != rColorStops.end());
                    const bool bAlpha(aCurrAlpha != rAlphaStops.end());

                    if (bColor && bAlpha)
                    {
                        const double fColorOff(aCurrColor->getStopOffset());
                        const double fAlphaOff(aCurrAlpha->getStopOffset());

                        if (fTools::less(fColorOff, fAlphaOff))
                        {
                            // copy color, create alpha
                            aNewColor.emplace_back(fColorOff, aCurrColor->getStopColor());
                            aNewAlpha.emplace_back(fColorOff, utils::modifyBColor(rAlphaStops, fColorOff, 0, aAlphaStopRange));
                            bRealChange = true;
                            aCurrColor++;
                        }
                        else if (fTools::more(fColorOff, fAlphaOff))
                        {
                            // copy alpha, create color
                            aNewColor.emplace_back(fAlphaOff, utils::modifyBColor(rColorStops, fAlphaOff, 0, aColorStopRange));
                            aNewAlpha.emplace_back(fAlphaOff, aCurrAlpha->getStopColor());
                            bRealChange = true;
                            aCurrAlpha++;
                        }
                        else
                        {
                            // equal: copy both, advance
                            aNewColor.emplace_back(fColorOff, aCurrColor->getStopColor());
                            aNewAlpha.emplace_back(fAlphaOff, aCurrAlpha->getStopColor());
                            aCurrColor++;
                            aCurrAlpha++;
                        }
                    }
                    else if (bColor)
                    {
                        const double fColorOff(aCurrColor->getStopOffset());
                        aNewAlpha.emplace_back(fColorOff, utils::modifyBColor(rAlphaStops, fColorOff, 0, aAlphaStopRange));
                        bRealChange = true;
                        aCurrColor++;
                    }
                    else if (bAlpha)
                    {
                        const double fAlphaOff(aCurrAlpha->getStopOffset());
                        aNewColor.emplace_back(fAlphaOff, utils::modifyBColor(rColorStops, fAlphaOff, 0, aColorStopRange));
                        bRealChange = true;
                        aCurrAlpha++;
                    }
                    else
                    {
                        // no more input, break do..while loop
                        break;
                    }
                }
                while(true);

                if (bRealChange)
                {
                    // copy on 'real' change, that means data was added.
                    // This should always be the cease and should have been
                    // detected as such above, see bNeedToSyncronize
                    rColorStops = aNewColor;
                    rAlphaStops = aNewColor;
                }
            }
        }

        /* Tooling method to linearly blend the Colors contained in
           a given ColorStop vector against a given Color using the
           given intensity values.
           The intensity values fStartIntensity, fEndIntensity are
           in the range of [0.0 .. 1.0] and describe how much the
           blend is supposed to be done at the start color position
           and the end color position resprectively, where 0.0 means
           to fully use the given BlendColor, 1.0 means to not change
           the existing color in the ColorStop.
           Every color entry in the given ColorStop is blended
           relative to it's StopPosition, interpolating the
           given intensities with the range [0.0 .. 1.0] to do so.
        */
        void blendColorStopsToIntensity(ColorStops& rColorStops, double fStartIntensity, double fEndIntensity, const basegfx::BColor& rBlendColor)
        {
            // no entries, done
            if (rColorStops.empty())
                return;

            // correct intensities (maybe assert when input was wrong)
            fStartIntensity = std::max(std::min(1.0, fStartIntensity), 0.0);
            fEndIntensity = std::max(std::min(1.0, fEndIntensity), 0.0);

            // all 100%, no real blend, done
            if (basegfx::fTools::equal(fStartIntensity, 1.0) && basegfx::fTools::equal(fEndIntensity, 1.0))
                return;

            // blend relative to StopOffset position
            for (auto& candidate : rColorStops)
            {
                const double fOffset(candidate.getStopOffset());
                const double fIntensity((fStartIntensity * (1.0 - fOffset)) + (fEndIntensity * fOffset));
                candidate = basegfx::ColorStop(
                    fOffset,
                    basegfx::interpolate(rBlendColor, candidate.getStopColor(), fIntensity));
            }
        }

        /* Tooling method to check if a ColorStop vector is defined
           by a single color. It returns true if this is the case.
           If true is returned, rSingleColor contains that single
           color for convenience.
           NOTE: If no ColorStop is defined, a fallback to BColor-default
                 (which is black) and true will be returned
        */
        bool isSingleColor(const ColorStops& rColorStops, BColor& rSingleColor)
        {
            if (rColorStops.empty())
            {
                rSingleColor = BColor();
                return true;
            }

            if (1 == rColorStops.size())
            {
                rSingleColor = rColorStops.front().getStopColor();
                return true;
            }

            rSingleColor = rColorStops.front().getStopColor();

            for (auto const& rCandidate : rColorStops)
            {
                if (rCandidate.getStopColor() != rSingleColor)
                    return false;
            }

            return true;
        }

        /* Tooling method to reverse ColorStops, including offsets.
           When also mirroring offsets a valid sort keeps valid.
        */
        void reverseColorStops(ColorStops& rColorStops)
        {
            // can use std::reverse, but also need to adapt offset(s)
            std::reverse(rColorStops.begin(), rColorStops.end());
            for (auto& candidate : rColorStops)
                candidate = ColorStop(1.0 - candidate.getStopOffset(), candidate.getStopColor());
        }

        /* Tooling method to convert UNO API data to ColorStops.
           This will try to extract ColorStop data from the given
           awt::Gradient2.
        */
        void fillColorStopsFromGradient2(ColorStops& rColorStops, const com::sun::star::awt::Gradient2& rGradient)
        {
            const sal_Int32 nLen(rGradient.ColorStops.getLength());

            if (0 == nLen)
                return;

            // we have ColorStops
            rColorStops.clear();
            rColorStops.reserve(nLen);
            const css::awt::ColorStop* pSourceColorStop(rGradient.ColorStops.getConstArray());

            for (sal_Int32 a(0); a < nLen; a++, pSourceColorStop++)
            {
                rColorStops.emplace_back(
                    pSourceColorStop->StopOffset,
                    BColor(pSourceColorStop->StopColor.Red, pSourceColorStop->StopColor.Green, pSourceColorStop->StopColor.Blue));
            }
        }

        /* Tooling method to convert UNO API data to ColorStops.
           This will try to extract ColorStop data from the given
           Any, so if it's of type awt::Gradient2 that data will be
           extracted, converted and copied into the given ColorStops.
        */
        void fillColorStopsFromAny(ColorStops& rColorStops, const css::uno::Any& rVal)
        {
            css::awt::Gradient2 aGradient2;
            if (!(rVal >>= aGradient2))
                return;

            fillColorStopsFromGradient2(rColorStops, aGradient2);
        }

        /* Tooling method to fill a awt::ColorStopSequence with
           the data from the given ColorStops. This is used in
           UNO API implementations.
        */
        void fillColorStopSequenceFromColorStops(css::awt::ColorStopSequence& rColorStopSequence, const ColorStops& rColorStops)
        {
            // fill ColorStops to extended Gradient2
            rColorStopSequence.realloc(rColorStops.size());
            css::awt::ColorStop* pTargetColorStop(rColorStopSequence.getArray());

            for (const auto& candidate : rColorStops)
            {
                pTargetColorStop->StopOffset = candidate.getStopOffset();
                pTargetColorStop->StopColor = css::rendering::RGBColor(
                    candidate.getStopColor().getRed(),
                    candidate.getStopColor().getGreen(),
                    candidate.getStopColor().getBlue());
                pTargetColorStop++;
            }
        }

        /* Tooling method that allows to replace the StartColor in a
           vector of ColorStops. A vector in 'ordered state' is expected,
           so you may use/have used sortAndCorrectColorStops, see below.
           This method is for convenience & backwards compatibility, please
           think about handling multi-colored gradients directly.
        */
        void replaceStartColor(ColorStops& rColorStops, const BColor& rStart)
        {
            ColorStops::iterator a1stNonStartColor(rColorStops.begin());

            // search for highest existing non-StartColor
            while (a1stNonStartColor != rColorStops.end() && basegfx::fTools::lessOrEqual(a1stNonStartColor->getStopOffset(), 0.0))
                a1stNonStartColor++;

            // create new ColorStops by 1st adding new one and then all
            // non-StartColor entries
            ColorStops aNewColorStops;

            aNewColorStops.reserve(rColorStops.size() + 1);
            aNewColorStops.emplace_back(0.0, rStart);
            aNewColorStops.insert(aNewColorStops.end(), a1stNonStartColor, rColorStops.end());

            // assign & done
            rColorStops = aNewColorStops;
        }

        /* Tooling method that allows to replace the EndColor in a
           vector of ColorStops. A vector in 'ordered state' is expected,
           so you may use/have used sortAndCorrectColorStops, see below.
           This method is for convenience & backwards compatibility, please
           think about handling multi-colored gradients directly.
        */
        void replaceEndColor(ColorStops& rColorStops, const BColor& rEnd)
        {
            // erase all evtl. existing EndColor(s)
            while (!rColorStops.empty() && basegfx::fTools::moreOrEqual(rColorStops.back().getStopOffset(), 1.0))
                rColorStops.pop_back();

            // add at the end of existing ColorStops
            rColorStops.emplace_back(1.0, rEnd);
        }

        // Tooling method to quickly create a ColorStop vector for a given set of Start/EndColor
        ColorStops createColorStopsFromStartEndColor(const BColor& rStart, const BColor& rEnd)
        {
            return ColorStops {
                ColorStop(0.0, rStart),
                ColorStop(1.0, rEnd) };
        }

        /* Tooling method to guarantee sort and correctness for
           the given ColorStops vector.
           A vector fulfilling these conditions is called to be
           in 'ordered state'.

           At return, the following conditions are guaranteed:
           - contains no ColorStops with offset < 0.0 (will
             be removed)
           - contains no ColorStops with offset > 1.0 (will
             be removed)
           - ColorStops with identical offsets are now allowed
           - will be sorted from lowest offset to highest

           Some more notes:
           - It can happen that the result is empty
           - It is allowed to have consecutive entries with
             the same color, this represents single-color
             regions inside the gradient
           - A entry with 0.0 is not required or forced, so
             no 'StartColor' is technically required
           - A entry with 1.0 is not required or forced, so
             no 'EndColor' is technically required

           All this is done in one run (sort + O(N)) without
           creating a copy of the data in any form
        */
        void sortAndCorrectColorStops(ColorStops& rColorStops)
        {
            // no content, we are done
            if (rColorStops.empty())
                return;

            if (1 == rColorStops.size())
            {
                // no gradient at all, but preserve given color
                // evtl. correct offset to be in valid range [0.0 .. 1.0]
                // NOTE: This does not move it to 0.0 or 1.0, it *can* still
                //       be somewhere in-between what is allowed
                rColorStops[0] = ColorStop(
                    std::max(0.0, std::min(1.0, rColorStops[0].getStopOffset())),
                    rColorStops[0].getStopColor());

                // done
                return;
            }

            // start with sorting the input data. Remember that
            // this preserves the order of equal entries, where
            // equal is defined here by offset (see use operator==)
            std::sort(rColorStops.begin(), rColorStops.end());

            // prepare status values
            size_t write(0);

            // use the paradigm of a band machine with two heads, read
            // and write with write <= read all the time. Step over the
            // data using read and check for valid entry. If valid, decide
            // how to keep it
            for (size_t read(0); read < rColorStops.size(); read++)
            {
                // get offset of entry at read position
                double fOff(rColorStops[read].getStopOffset());

                if (basegfx::fTools::less(fOff, 0.0) && read + 1 < rColorStops.size())
                {
                    // value < 0.0 and we have a next entry. check for gradient snippet
                    // containing 0.0 resp. StartColor
                    const double fOff2(rColorStops[read + 1].getStopOffset());

                    if (basegfx::fTools::more(fOff2, 0.0))
                    {
                        // read is the start of a gradient snippet containing 0.0. Correct
                        // entry to StartColor, interpolate to correct StartColor
                        rColorStops[read] = ColorStop(0.0, basegfx::interpolate(
                            rColorStops[read].getStopColor(),
                            rColorStops[read + 1].getStopColor(),
                            (0.0 - fOff) / (fOff2 - fOff)));

                        // adapt fOff
                        fOff = 0.0;
                    }
                }

                // step over < 0 values, these are outside and will be removed
                if (basegfx::fTools::less(fOff, 0.0))
                {
                    continue;
                }

                if (basegfx::fTools::less(fOff, 1.0) && read + 1 < rColorStops.size())
                {
                    // value < 1.0 and we have a next entry. check for gradient snippet
                    // containing 1.0 resp. EndColor
                    const double fOff2(rColorStops[read + 1].getStopOffset());

                    if (basegfx::fTools::more(fOff2, 1.0))
                    {
                        // read is the start of a gradient snippet containing 1.0. Correct
                        // next entry to EndColor, interpolate to correct EndColor
                        rColorStops[read + 1] = ColorStop(1.0, basegfx::interpolate(
                            rColorStops[read].getStopColor(),
                            rColorStops[read + 1].getStopColor(),
                            (1.0 - fOff) / (fOff2 - fOff)));

                        // adapt fOff
                        fOff = 1.0;
                    }
                }

                // step over > 1 values; even break, since all following
                // entries will also be bigger due to being sorted, so done
                if (basegfx::fTools::more(fOff, 1.0))
                {
                    break;
                }

                // entry is valid value at read position
                // copy if write target is empty (write at start) or when
                // write target is different to read in color or offset
                if (0 == write || !(rColorStops[read] == rColorStops[write-1]))
                {
                    if (write != read)
                    {
                        // copy read to write backwards to close gaps
                        rColorStops[write] = rColorStops[read];
                    }

                    // always forward write position
                    write++;
                }
            }

            // correct size when length is reduced. write is always at
            // last used position + 1
            if (rColorStops.size() > write)
            {
                if (0 == write)
                {
                    // no valid entries at all, but not empty. This can only happen
                    // when all entries are below 0.0 or above 1.0 (else a gradient
                    // snippet spawning over both would have been detected)
                    if (basegfx::fTools::less(rColorStops.back().getStopOffset(), 0.0))
                    {
                        // all outside too low, rescue last due to being closest to content
                        rColorStops = ColorStops { ColorStop(0.0, rColorStops.back().getStopColor()) };
                    }
                    else // if (basegfx::fTools::more(rColorStops.front().getStopOffset(), 1.0))
                    {
                        // all outside too high, rescue first due to being closest to content
                        rColorStops = ColorStops { ColorStop(1.0, rColorStops.front().getStopColor()) };
                    }
                }
                else
                {
                    rColorStops.resize(write);
                }
            }
        }

        BColor modifyBColor(
            const ColorStops& rColorStops,
            double fScaler,
            sal_uInt32 nRequestedSteps,
            ColorStopRange& rLastColorStopRange)
        {
            // no color at all, done
            if (rColorStops.empty())
                return BColor();

            // outside range -> at start
            const double fMin(rColorStops.front().getStopOffset());
            if (fScaler < fMin)
                return rColorStops.front().getStopColor();

            // outside range -> at end
            const double fMax(rColorStops.back().getStopOffset());
            if (fScaler > fMax)
                return rColorStops.back().getStopColor();

            // special case for the 'classic' case with just two colors:
            // we can optimize that and keep the speed/resources low
            // by avoiding some calculations and an O(log(N)) array access
            if (2 == rColorStops.size())
            {
                if (fTools::equal(fMin, fMax))
                    return rColorStops.front().getStopColor();

                const basegfx::BColor aCStart(rColorStops.front().getStopColor());
                const basegfx::BColor aCEnd(rColorStops.back().getStopColor());
                const sal_uInt32 nSteps(
                    calculateNumberOfSteps(
                        nRequestedSteps,
                        aCStart,
                        aCEnd));

                // we need to extend the interpolation to the local
                // range of ColorStops. Despite having two ColorStops
                // these are not necessarily at 0.0 and 1.0, so mabe
                // not the classical Start/EndColor (what is allowed)
                fScaler = (fScaler - fMin) / (fMax - fMin);
                return basegfx::interpolate(
                    aCStart,
                    aCEnd,
                    nSteps > 1 ? floor(fScaler * nSteps) / double(nSteps - 1) : fScaler);
            }

            // check if we need to newly populate the needed interpolation data
            // or if we can re-use from last time.
            // If this scope is not entered, we do not need the binary search. It's
            // only a single buffered entry, and only used when more than three
            // ColorStops exist, but makes a huge difference compared with acessing
            // the sorted ColorStop vector each time.
            // NOTE: with this simple change I get very high hit rates, e.g. rotating
            //       a donut with gradient test '1' hit rate is at 0.99909440357755486
            if (rLastColorStopRange.mfOffsetStart == rLastColorStopRange.mfOffsetEnd
                || fScaler < rLastColorStopRange.mfOffsetStart
                || fScaler > rLastColorStopRange.mfOffsetEnd)
            {
                // access needed spot in sorted array using binary search
                // NOTE: This *seems* slow(er) when developing compared to just
                //       looping/accessing, but that's just due to the extensive
                //       debug test code created by the stl. In a pro version,
                //       all is good/fast as expected
                const auto upperBound(
                    std::upper_bound(
                        rColorStops.begin(),
                        rColorStops.end(),
                        ColorStop(fScaler),
                        [](const ColorStop& x, const ColorStop& y) { return x.getStopOffset() < y.getStopOffset(); }));

                // no upper bound, done
                if (rColorStops.end() == upperBound)
                    return rColorStops.back().getStopColor();

                // lower bound is one entry back, access that
                const auto lowerBound(upperBound - 1);

                // no lower bound, done
                if (rColorStops.end() == lowerBound)
                    return rColorStops.back().getStopColor();

                // we have lower and upper bound, get colors and offsets
                rLastColorStopRange.maColorStart = lowerBound->getStopColor();
                rLastColorStopRange.maColorEnd = upperBound->getStopColor();
                rLastColorStopRange.mfOffsetStart = lowerBound->getStopOffset();
                rLastColorStopRange.mfOffsetEnd = upperBound->getStopOffset();
            }

            // when there are just two color steps this cannot happen, but when using
            // a range of colors this *may* be used inside the range to represent
            // single-colored regions inside a ColorRange. Use that color & done
            if (rLastColorStopRange.maColorStart == rLastColorStopRange.maColorEnd)
                return rLastColorStopRange.maColorStart;

            // calculate number of steps and adapted proportinal
            // range for scaler in [0.0 .. 1.0]
            const double fAdaptedScaler((fScaler - rLastColorStopRange.mfOffsetStart) /
                (rLastColorStopRange.mfOffsetEnd - rLastColorStopRange.mfOffsetStart));
            const sal_uInt32 nSteps(
                calculateNumberOfSteps(
                    nRequestedSteps,
                    rLastColorStopRange.maColorStart,
                    rLastColorStopRange.maColorEnd));

            // interpolate & evtl. apply steps
            return interpolate(
                rLastColorStopRange.maColorStart,
                rLastColorStopRange.maColorEnd,
                nSteps > 1 ? floor(fAdaptedScaler * nSteps) / double(nSteps - 1) : fAdaptedScaler);
        }

        sal_uInt32 calculateNumberOfSteps(
            sal_uInt32 nRequestedSteps,
            const BColor& rStart,
            const BColor& rEnd)
        {
            const sal_uInt32 nMaxSteps(sal_uInt32((rStart.getMaximumDistance(rEnd) * 127.5) + 0.5));

            if (0 == nRequestedSteps)
            {
                nRequestedSteps = nMaxSteps;
            }

            if(nRequestedSteps > nMaxSteps)
            {
                nRequestedSteps = nMaxSteps;
            }

            return std::max(sal_uInt32(1), nRequestedSteps);
        }

        ODFGradientInfo createLinearODFGradientInfo(
            const B2DRange& rTargetArea,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return init1DGradientInfo(
                rTargetArea,
                nSteps,
                fBorder,
                fAngle,
                false);
        }

        ODFGradientInfo createAxialODFGradientInfo(
            const B2DRange& rTargetArea,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return init1DGradientInfo(
                rTargetArea,
                nSteps,
                fBorder,
                fAngle,
                true);
        }

        ODFGradientInfo createRadialODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder)
        {
            return initEllipticalGradientInfo(
                rTargetArea,
                rOffset,
                nSteps,
                fBorder,
                0.0,
                true);
        }

        ODFGradientInfo createEllipticalODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return initEllipticalGradientInfo(
                rTargetArea,
                rOffset,
                nSteps,
                fBorder,
                fAngle,
                false);
        }

        ODFGradientInfo createSquareODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return initRectGradientInfo(
                rTargetArea,
                rOffset,
                nSteps,
                fBorder,
                fAngle,
                true);
        }

        ODFGradientInfo createRectangularODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle)
        {
            return initRectGradientInfo(
                rTargetArea,
                rOffset,
                nSteps,
                fBorder,
                fAngle,
                false);
        }

        double getLinearGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);

            // Ignore X, this is not needed at all for Y-Oriented gradients
            // if(aCoor.getX() < 0.0 || aCoor.getX() > 1.0)
            // {
            //     return 0.0;
            // }

            if(aCoor.getY() <= 0.0)
            {
                return 0.0; // start value for inside
            }

            if(aCoor.getY() >= 1.0)
            {
                return 1.0; // end value for outside
            }

            return aCoor.getY();
        }

        double getAxialGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);

            // Ignore X, this is not needed at all for Y-Oriented gradients
            //if(aCoor.getX() < 0.0 || aCoor.getX() > 1.0)
            //{
            //    return 0.0;
            //}

            const double fAbsY(fabs(aCoor.getY()));

            if(fAbsY >= 1.0)
            {
                return 1.0; // use end value when outside in Y
            }

            return fAbsY;
        }

        double getRadialGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);

            if(aCoor.getX() < -1.0 || aCoor.getX() > 1.0 || aCoor.getY() < -1.0 || aCoor.getY() > 1.0)
            {
                return 0.0;
            }

            return 1.0 - std::hypot(aCoor.getX(), aCoor.getY());
        }

        double getEllipticalGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);

            if(aCoor.getX() < -1.0 || aCoor.getX() > 1.0 || aCoor.getY() < -1.0 || aCoor.getY() > 1.0)
            {
                return 0.0;
            }

            double fAspectRatio(rGradInfo.getAspectRatio());
            double t(1.0);

            // MCGR: Similar to getRectangularGradientAlpha (please
            // see there) we need to use aspect ratio here. Due to
            // initEllipticalGradientInfo using M_SQRT2 to make this
            // gradient look 'nicer' this correciton seems not 100%
            // correct, but is close enough for now
            if(fAspectRatio > 1.0)
            {
                t = 1.0 - std::hypot(aCoor.getX() / fAspectRatio, aCoor.getY());
            }
            else if(fAspectRatio > 0.0)
            {
                t = 1.0 - std::hypot(aCoor.getX(), aCoor.getY() * fAspectRatio);
            }

            return t;
        }

        double getSquareGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);
            const double fAbsX(fabs(aCoor.getX()));

            if(fAbsX >= 1.0)
            {
                return 0.0;
            }

            const double fAbsY(fabs(aCoor.getY()));

            if(fAbsY >= 1.0)
            {
                return 0.0;
            }

            return 1.0 - std::max(fAbsX, fAbsY);
        }

        double getRectangularGradientAlpha(const B2DPoint& rUV, const ODFGradientInfo& rGradInfo)
        {
            const B2DPoint aCoor(rGradInfo.getBackTextureTransform() * rUV);
            double fAbsX(fabs(aCoor.getX()));

            if(fAbsX >= 1.0)
            {
                return 0.0;
            }

            double fAbsY(fabs(aCoor.getY()));

            if(fAbsY >= 1.0)
            {
                return 0.0;
            }

            // MCGR: Visualiations using the texturing method for
            // displaying gradients (getBackTextureTransform is
            // involved) show wrong results for GradientElliptical
            // and GradientRect, this can be best seen when using
            // less steps, e.g. just four. This thus has influence
            // on cppcanvas (slideshow) and 3D textures, so needs
            // to be corrected.
            // Missing is to use the aspect ratio of the object
            // in this [-1, -1, 1, 1] unified coordinate space
            // after getBackTextureTransform is applied. Optically
            // in the larger direction of the texturing the color
            // step distances are too big *because* we are in that
            // unit range now.
            // To correct that, a kind of 'limo stretching' needs to
            // be applied, adding space around the center
            // proportional to the aspect ratio, so the intuitive
            // idea would be to do
            //
            // fAbsX' = ((fAspectRatio - 1) + fAbsX) / fAspectRatio
            //
            // which scales from the center. This does not work, and
            // after some thoughts it's clear why: It's not the
            // position that needs to be moved (this cannot be
            // changed), but the position *before* that scale has
            // to be determined to get the correct, shifted color
            // for the already 'new' position. Thus, turn around
            // the expression as
            //
            // fAbsX' * fAspectRatio = fAspectRatio - 1 + fAbsX
            // fAbsX' * fAspectRatio - fAspectRatio + 1 = fAbsX
            // fAbsX = (fAbsX' - 1) * fAspectRatio + 1
            //
            // This works and can even be simply adapted for
            // fAspectRatio < 1.0 aka vertical is bigger.
            double fAspectRatio(rGradInfo.getAspectRatio());
            if(fAspectRatio > 1.0)
            {
                fAbsX = ((fAbsX - 1) * fAspectRatio) + 1;
            }
            else if(fAspectRatio > 0.0)
            {
                fAbsY = ((fAbsY - 1) / fAspectRatio) + 1;
            }

            return 1.0 - std::max(fAbsX, fAbsY);
        }
    } // namespace utils
} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
