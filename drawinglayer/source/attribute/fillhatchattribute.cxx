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

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpFillHatchAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

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
            :   mnRefCount(0),
                meStyle(eStyle),
                mfDistance(fDistance),
                mfAngle(fAngle),
                maColor(rColor),
                mbFillBackground(bFillBackground)
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

            static ImpFillHatchAttribute* get_global_default()
            {
                static ImpFillHatchAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpFillHatchAttribute(
                        HATCHSTYLE_SINGLE,
                        0.0, 0.0,
                        basegfx::BColor(),
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        FillHatchAttribute::FillHatchAttribute(
            HatchStyle eStyle,
            double fDistance,
            double fAngle,
            const basegfx::BColor& rColor,
            bool bFillBackground)
        :   mpFillHatchAttribute(new ImpFillHatchAttribute(
                eStyle, fDistance, fAngle, rColor, bFillBackground))
        {
        }

        FillHatchAttribute::FillHatchAttribute()
        :   mpFillHatchAttribute(ImpFillHatchAttribute::get_global_default())
        {
            mpFillHatchAttribute->mnRefCount++;
        }

        FillHatchAttribute::FillHatchAttribute(const FillHatchAttribute& rCandidate)
        :   mpFillHatchAttribute(rCandidate.mpFillHatchAttribute)
        {
            mpFillHatchAttribute->mnRefCount++;
        }

        FillHatchAttribute::~FillHatchAttribute()
        {
            if(mpFillHatchAttribute->mnRefCount)
            {
                mpFillHatchAttribute->mnRefCount--;
            }
            else
            {
                delete mpFillHatchAttribute;
            }
        }

        bool FillHatchAttribute::isDefault() const
        {
            return mpFillHatchAttribute == ImpFillHatchAttribute::get_global_default();
        }

        FillHatchAttribute& FillHatchAttribute::operator=(const FillHatchAttribute& rCandidate)
        {
            if(rCandidate.mpFillHatchAttribute != mpFillHatchAttribute)
            {
                if(mpFillHatchAttribute->mnRefCount)
                {
                    mpFillHatchAttribute->mnRefCount--;
                }
                else
                {
                    delete mpFillHatchAttribute;
                }

                mpFillHatchAttribute = rCandidate.mpFillHatchAttribute;
                mpFillHatchAttribute->mnRefCount++;
            }

            return *this;
        }

        bool FillHatchAttribute::operator==(const FillHatchAttribute& rCandidate) const
        {
            if(rCandidate.mpFillHatchAttribute == mpFillHatchAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpFillHatchAttribute == *mpFillHatchAttribute);
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
