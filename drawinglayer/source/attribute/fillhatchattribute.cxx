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

#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <basegfx/color/bcolor.hxx>
#include <rtl/instance.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpFillHatchAttribute
        {
        public:
            // data definitions
            HatchStyle                              meStyle;
            double                                  mfDistance;
            double                                  mfAngle;
            basegfx::BColor                         maColor;

            // bitfield
            unsigned                                mbFillBackground : 1;

            ImpFillHatchAttribute(
                HatchStyle eStyle,
                double fDistance,
                double fAngle,
                const basegfx::BColor& rColor,
                bool bFillBackground)
            :   meStyle(eStyle),
                mfDistance(fDistance),
                mfAngle(fAngle),
                maColor(rColor),
                mbFillBackground(bFillBackground)
            {
            }

            ImpFillHatchAttribute()
            :   meStyle(HATCHSTYLE_SINGLE),
                mfDistance(0.0),
                mfAngle(0.0),
                maColor(basegfx::BColor()),
                mbFillBackground(false)
            {
            }

            // data read access
            HatchStyle getStyle() const { return meStyle; }
            double getDistance() const { return mfDistance; }
            double getAngle() const { return mfAngle; }
            const basegfx::BColor& getColor() const { return maColor; }
            bool isFillBackground() const { return mbFillBackground; }

            bool operator==(const ImpFillHatchAttribute& rCandidate) const
            {
                return (getStyle() == rCandidate.getStyle()
                    && getDistance() == rCandidate.getDistance()
                    && getAngle() == rCandidate.getAngle()
                    && getColor() == rCandidate.getColor()
                    && isFillBackground()  == rCandidate.isFillBackground());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< FillHatchAttribute::ImplType, theGlobalDefault > {};
        }

        FillHatchAttribute::FillHatchAttribute(
            HatchStyle eStyle,
            double fDistance,
            double fAngle,
            const basegfx::BColor& rColor,
            bool bFillBackground)
        :   mpFillHatchAttribute(ImpFillHatchAttribute(
                eStyle, fDistance, fAngle, rColor, bFillBackground))
        {
        }

        FillHatchAttribute::FillHatchAttribute()
        :   mpFillHatchAttribute(theGlobalDefault::get())
        {
        }

        FillHatchAttribute::FillHatchAttribute(const FillHatchAttribute& rCandidate)
        :   mpFillHatchAttribute(rCandidate.mpFillHatchAttribute)
        {
        }

        FillHatchAttribute::~FillHatchAttribute()
        {
        }

        bool FillHatchAttribute::isDefault() const
        {
            return mpFillHatchAttribute.same_object(theGlobalDefault::get());
        }

        FillHatchAttribute& FillHatchAttribute::operator=(const FillHatchAttribute& rCandidate)
        {
            mpFillHatchAttribute = rCandidate.mpFillHatchAttribute;
            return *this;
        }

        bool FillHatchAttribute::operator==(const FillHatchAttribute& rCandidate) const
        {
            return rCandidate.mpFillHatchAttribute == mpFillHatchAttribute;
        }

        // data read access
        HatchStyle FillHatchAttribute::getStyle() const
        {
            return mpFillHatchAttribute->getStyle();
        }

        double FillHatchAttribute::getDistance() const
        {
            return mpFillHatchAttribute->getDistance();
        }

        double FillHatchAttribute::getAngle() const
        {
            return mpFillHatchAttribute->getAngle();
        }

        const basegfx::BColor& FillHatchAttribute::getColor() const
        {
            return mpFillHatchAttribute->getColor();
        }

        bool FillHatchAttribute::isFillBackground() const
        {
            return mpFillHatchAttribute->isFillBackground();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
