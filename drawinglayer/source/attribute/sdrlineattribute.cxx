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

#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <basegfx/color/bcolor.hxx>


namespace drawinglayer::attribute
{
        class ImpSdrLineAttribute
        {
        public:
            // line definitions
            double                                  mfWidth;            // 1/100th mm, 0.0==hair
            double                                  mfTransparence;     // [0.0 .. 1.0], 0.0==no transp.
            double                                  mfFullDotDashLen;   // sum of maDotDashArray (for convenience)
            basegfx::BColor                         maColor;            // color of line
            std::vector< double >                   maDotDashArray;     // array of double which defines the dot-dash pattern
            basegfx::B2DLineJoin                    meJoin;             // B2DLINEJOIN_* defines
            css::drawing::LineCap                   meCap;              // BUTT, ROUND, or SQUARE

            ImpSdrLineAttribute(
                basegfx::B2DLineJoin eJoin,
                double fWidth,
                double fTransparence,
                const basegfx::BColor& rColor,
                css::drawing::LineCap eCap,
                std::vector< double >&& rDotDashArray,
                double fFullDotDashLen)
            :   mfWidth(fWidth),
                mfTransparence(fTransparence),
                mfFullDotDashLen(fFullDotDashLen),
                maColor(rColor),
                maDotDashArray(std::move(rDotDashArray)),
                meJoin(eJoin),
                meCap(eCap)
            {
            }

            ImpSdrLineAttribute()
            :   mfWidth(0.0),
                mfTransparence(0.0),
                mfFullDotDashLen(0.0),
                meJoin(basegfx::B2DLineJoin::Round),
                meCap(css::drawing::LineCap_BUTT)
            {
            }

            // data read access
            basegfx::B2DLineJoin getJoin() const { return meJoin; }
            double getWidth() const { return mfWidth; }
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
            css::drawing::LineCap getCap() const { return meCap; }
            const std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const { return mfFullDotDashLen; }

            bool operator==(const ImpSdrLineAttribute& rCandidate) const
            {
                return (getJoin() == rCandidate.getJoin()
                    && getWidth() == rCandidate.getWidth()
                    && getTransparence() == rCandidate.getTransparence()
                    && getColor() == rCandidate.getColor()
                    && getCap() == rCandidate.getCap()
                    && getDotDashArray() == rCandidate.getDotDashArray());
            }
        };

        namespace
        {
            SdrLineAttribute::ImplType& theGlobalDefault()
            {
                static SdrLineAttribute::ImplType SINGLETON;
                return SINGLETON;
            }
        }

        SdrLineAttribute::SdrLineAttribute(
            basegfx::B2DLineJoin eJoin,
            double fWidth,
            double fTransparence,
            const basegfx::BColor& rColor,
            css::drawing::LineCap eCap,
            std::vector< double >&& rDotDashArray,
            double fFullDotDashLen)
        :   mpSdrLineAttribute(
                ImpSdrLineAttribute(
                    eJoin,
                    fWidth,
                    fTransparence,
                    rColor,
                    eCap,
                    std::move(rDotDashArray),
                    fFullDotDashLen))

        {
        }

        SdrLineAttribute::SdrLineAttribute()
        :   mpSdrLineAttribute(theGlobalDefault())
        {
        }

        SdrLineAttribute::SdrLineAttribute(const SdrLineAttribute&) = default;

        SdrLineAttribute::SdrLineAttribute(SdrLineAttribute&&) = default;

        SdrLineAttribute::~SdrLineAttribute() = default;

        bool SdrLineAttribute::isDefault() const
        {
            return mpSdrLineAttribute.same_object(theGlobalDefault());
        }

        SdrLineAttribute& SdrLineAttribute::operator=(const SdrLineAttribute&) = default;

        SdrLineAttribute& SdrLineAttribute::operator=(SdrLineAttribute&&) = default;

        bool SdrLineAttribute::operator==(const SdrLineAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpSdrLineAttribute == mpSdrLineAttribute;
        }

        basegfx::B2DLineJoin SdrLineAttribute::getJoin() const
        {
            return mpSdrLineAttribute->getJoin();
        }

        double SdrLineAttribute::getWidth() const
        {
            return mpSdrLineAttribute->getWidth();
        }

        double SdrLineAttribute::getTransparence() const
        {
            return mpSdrLineAttribute->getTransparence();
        }

        const basegfx::BColor& SdrLineAttribute::getColor() const
        {
            return mpSdrLineAttribute->getColor();
        }

        const std::vector< double >& SdrLineAttribute::getDotDashArray() const
        {
            return mpSdrLineAttribute->getDotDashArray();
        }

        double SdrLineAttribute::getFullDotDashLen() const
        {
            return mpSdrLineAttribute->getFullDotDashLen();
        }

        css::drawing::LineCap SdrLineAttribute::getCap() const
        {
            return mpSdrLineAttribute->getCap();
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
