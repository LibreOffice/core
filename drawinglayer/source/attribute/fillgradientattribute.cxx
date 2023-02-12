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

#include <drawinglayer/attribute/fillgradientattribute.hxx>

namespace drawinglayer::attribute
{
        class ImpFillGradientAttribute
        {
        public:
            // data definitions
            double                                  mfBorder;
            double                                  mfOffsetX;
            double                                  mfOffsetY;
            double                                  mfAngle;
            FillGradientAttribute::ColorSteps       maColorSteps;
            GradientStyle                           meStyle;
            sal_uInt16                              mnSteps;

            ImpFillGradientAttribute(
                GradientStyle eStyle,
                double fBorder,
                double fOffsetX,
                double fOffsetY,
                double fAngle,
                const basegfx::BColor& rStartColor,
                const basegfx::BColor& rEndColor,
                const FillGradientAttribute::ColorSteps* pColorSteps,
                sal_uInt16 nSteps)
            :   mfBorder(fBorder),
                mfOffsetX(fOffsetX),
                mfOffsetY(fOffsetY),
                mfAngle(fAngle),
                maColorSteps(),
                meStyle(eStyle),
                mnSteps(nSteps)
            {
                // always add start color to guarantee a color at all. It's also just safer
                // to have one and not an empty vector, that spares many checks in the using code
                maColorSteps.emplace_back(0.0, rStartColor);

                // if we have ColorSteps, integrate these
                if(nullptr != pColorSteps)
                {
                    for(const auto& candidate : *pColorSteps)
                    {
                        // only allow ]0.0 .. 1.0[ as offset values, *excluding* 0.0 and 1.0
                        // explicitly - these are reserved for start/end color
                        if(basegfx::fTools::more(candidate.getOffset(), 0.0) && basegfx::fTools::less(candidate.getOffset(), 1.0))
                        {
                            // ignore same offsets, independent from color (so 1st one wins)
                            // having two or more same offsets is an error (may assert evtl.)
                            bool bAccept(true);

                            for(const auto& compare : maColorSteps)
                            {
                                if(basegfx::fTools::equal(compare.getOffset(), candidate.getOffset()))
                                {
                                    bAccept = false;
                                    break;
                                }
                            }

                            if(bAccept)
                            {
                                maColorSteps.push_back(candidate);
                            }
                        }
                    }

                    // sort by offset when colors were added
                    if(maColorSteps.size() > 1)
                    {
                        std::sort(maColorSteps.begin(), maColorSteps.end());
                    }
                }

                // add end color if different from last color - which is the start color
                // when no ColorSteps are given
                if(rEndColor != maColorSteps.back().getColor())
                {
                    maColorSteps.emplace_back(1.0, rEndColor);
                }
            }

            ImpFillGradientAttribute()
            :   mfBorder(0.0),
                mfOffsetX(0.0),
                mfOffsetY(0.0),
                mfAngle(0.0),
                maColorSteps(),
                meStyle(GradientStyle::Linear),
                mnSteps(0)
            {
                // always add a fallback color, see above
                maColorSteps.emplace_back(0.0, basegfx::BColor());
            }

            // data read access
            GradientStyle getStyle() const { return meStyle; }
            double getBorder() const { return mfBorder; }
            double getOffsetX() const { return mfOffsetX; }
            double getOffsetY() const { return mfOffsetY; }
            double getAngle() const { return mfAngle; }
            const FillGradientAttribute::ColorSteps& getColorSteps() const { return maColorSteps; }
            sal_uInt16 getSteps() const { return mnSteps; }

            bool hasSingleColor() const
            {
                // no entries (should not happen, see comments for startColor)
                if (0 == maColorSteps.size())
                    return true;

                // check if not all colors are the same
                const basegfx::BColor& rColor(maColorSteps[0].getColor());
                for (size_t a(1); a < maColorSteps.size(); a++)
                    if (maColorSteps[a].getColor() != rColor)
                        return false;

                // all colors are the same
                return true;
            }

            bool operator==(const ImpFillGradientAttribute& rCandidate) const
            {
                return (getStyle() == rCandidate.getStyle()
                    && getBorder() == rCandidate.getBorder()
                    && getOffsetX() == rCandidate.getOffsetX()
                    && getOffsetY() == rCandidate.getOffsetY()
                    && getAngle() == rCandidate.getAngle()
                    && getColorSteps() == rCandidate.getColorSteps()
                    && getSteps() == rCandidate.getSteps());
            }
        };

        namespace
        {
            FillGradientAttribute::ImplType& theGlobalDefault()
            {
                static FillGradientAttribute::ImplType SINGLETON;
                return SINGLETON;
            }
        }

        FillGradientAttribute::FillGradientAttribute(
            GradientStyle eStyle,
            double fBorder,
            double fOffsetX,
            double fOffsetY,
            double fAngle,
            const basegfx::BColor& rStartColor,
            const basegfx::BColor& rEndColor,
            const ColorSteps* pColorSteps,
            sal_uInt16 nSteps)
        :   mpFillGradientAttribute(ImpFillGradientAttribute(
                eStyle, fBorder, fOffsetX, fOffsetY, fAngle, rStartColor, rEndColor, pColorSteps, nSteps))
        {
        }

        FillGradientAttribute::FillGradientAttribute()
        :   mpFillGradientAttribute(theGlobalDefault())
        {
        }

        FillGradientAttribute::FillGradientAttribute(const FillGradientAttribute&) = default;

        FillGradientAttribute::FillGradientAttribute(FillGradientAttribute&&) = default;

        FillGradientAttribute::~FillGradientAttribute() = default;

        bool FillGradientAttribute::isDefault() const
        {
            return mpFillGradientAttribute.same_object(theGlobalDefault());
        }

        bool FillGradientAttribute::hasSingleColor() const
        {
            return mpFillGradientAttribute->hasSingleColor();
        }

        FillGradientAttribute& FillGradientAttribute::operator=(const FillGradientAttribute&) = default;

        FillGradientAttribute& FillGradientAttribute::operator=(FillGradientAttribute&&) = default;

        bool FillGradientAttribute::operator==(const FillGradientAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpFillGradientAttribute == mpFillGradientAttribute;
        }

        const FillGradientAttribute::ColorSteps& FillGradientAttribute::getColorSteps() const
        {
            return mpFillGradientAttribute->getColorSteps();
        }

        double FillGradientAttribute::getBorder() const
        {
            return mpFillGradientAttribute->getBorder();
        }

        double FillGradientAttribute::getOffsetX() const
        {
            return mpFillGradientAttribute->getOffsetX();
        }

        double FillGradientAttribute::getOffsetY() const
        {
            return mpFillGradientAttribute->getOffsetY();
        }

        double FillGradientAttribute::getAngle() const
        {
            return mpFillGradientAttribute->getAngle();
        }

        GradientStyle FillGradientAttribute::getStyle() const
        {
            return mpFillGradientAttribute->getStyle();
        }

        sal_uInt16 FillGradientAttribute::getSteps() const
        {
            return mpFillGradientAttribute->getSteps();
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
