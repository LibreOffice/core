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
#include <basegfx/utils/gradienttools.hxx>

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
            basegfx::ColorSteps                     maColorSteps;
            GradientStyle                           meStyle;
            sal_uInt16                              mnSteps;

            ImpFillGradientAttribute(
                GradientStyle eStyle,
                double fBorder,
                double fOffsetX,
                double fOffsetY,
                double fAngle,
                const basegfx::ColorSteps& rColorSteps,
                sal_uInt16 nSteps)
            :   mfBorder(fBorder),
                mfOffsetX(fOffsetX),
                mfOffsetY(fOffsetY),
                mfAngle(fAngle),
                maColorSteps(rColorSteps), // copy ColorSteps
                meStyle(eStyle),
                mnSteps(nSteps)
            {
                // Correct the local ColorSteps. That will guarantee that the
                // content does contain no offsets < 0.0, > 1.0 or double
                // ones, also secures sorted arrangement and checks for
                // double colors, too (see there for more information).
                // This is what the usages of this in primitives need.
                // Since FillGradientAttribute is read-only doing this
                // once here in the constructor is sufficient
                basegfx::utils::sortAndCorrectColorSteps(maColorSteps);

                // sortAndCorrectColorSteps is rigid and can return
                // an empty result. To keep things simple, add a single
                // fallback value
                if (maColorSteps.empty())
                {
                    maColorSteps.emplace_back(0.0, basegfx::BColor());
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
            const basegfx::ColorSteps& getColorSteps() const { return maColorSteps; }
            sal_uInt16 getSteps() const { return mnSteps; }

            bool hasSingleColor() const
            {
                // No entry (should not happen, see comments for startColor above)
                // or single entry -> no gradient.
                // No need to check for all-the-same color since this is checked/done
                // in the constructor already, see there
                return maColorSteps.size() < 2;
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
            const basegfx::ColorSteps& rColorSteps,
            sal_uInt16 nSteps)
        :   mpFillGradientAttribute(ImpFillGradientAttribute(
                eStyle, fBorder, fOffsetX, fOffsetY, fAngle, rColorSteps, nSteps))
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

        const basegfx::ColorSteps& FillGradientAttribute::getColorSteps() const
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
