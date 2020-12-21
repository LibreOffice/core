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

#include <drawinglayer/attribute/lineattribute.hxx>
#include <basegfx/color/bcolor.hxx>
#include <rtl/instance.hxx>


namespace drawinglayer::attribute
{
        class ImpLineAttribute
        {
        public:
            // data definitions
            basegfx::BColor                         maColor;                // color
            double                                  mfWidth;                // absolute line width
            basegfx::B2DLineJoin                    meLineJoin;             // type of LineJoin
            css::drawing::LineCap                   meLineCap;              // BUTT, ROUND, or SQUARE
            double                                  mfMiterMinimumAngle;     // as needed for createAreaGeometry
            css::drawing::LineDash                  meLineDash;             // DotLen DotCount DashLen DashCount Distance

            ImpLineAttribute(
                const basegfx::BColor& rColor,
                double fWidth,
                basegfx::B2DLineJoin aB2DLineJoin,
                css::drawing::LineCap aLineCap,
                double fMiterMinimumAngle,
                css::drawing::LineDash aLineDash)
            :   maColor(rColor),
                mfWidth(fWidth),
                meLineJoin(aB2DLineJoin),
                meLineCap(aLineCap),
                mfMiterMinimumAngle(fMiterMinimumAngle),
                meLineDash(aLineDash)
            {
            }

            ImpLineAttribute()
            :   maColor(basegfx::BColor()),
                mfWidth(0.0),
                meLineJoin(basegfx::B2DLineJoin::Round),
                meLineCap(css::drawing::LineCap_BUTT),
                mfMiterMinimumAngle(basegfx::deg2rad(15.0)),
                meLineDash()
            {
            }

            // data read access
            const basegfx::BColor& getColor() const { return maColor; }
            double getWidth() const { return mfWidth; }
            basegfx::B2DLineJoin getLineJoin() const { return meLineJoin; }
            css::drawing::LineCap getLineCap() const { return meLineCap; }
            css::drawing::LineDash getLineDash() const { return meLineDash; }
            double getMiterMinimumAngle() const { return mfMiterMinimumAngle; }

            bool operator==(const ImpLineAttribute& rCandidate) const
            {
                return (getColor() == rCandidate.getColor()
                    && getWidth() == rCandidate.getWidth()
                    && getLineJoin() == rCandidate.getLineJoin()
                    && getLineCap() == rCandidate.getLineCap()
                    && getLineDash() == rCandidate.getLineDash()
                    && getMiterMinimumAngle() == rCandidate.getMiterMinimumAngle());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< LineAttribute::ImplType, theGlobalDefault > {};
        }

        LineAttribute::LineAttribute(
            const basegfx::BColor& rColor,
            double fWidth,
            basegfx::B2DLineJoin aB2DLineJoin,
            css::drawing::LineCap aLineCap,
            double fMiterMinimumAngle,
            css::drawing::LineDash aLineDash)
        :   mpLineAttribute(
                ImpLineAttribute(
                    rColor,
                    fWidth,
                    aB2DLineJoin,
                    aLineCap,
                    fMiterMinimumAngle,
                    aLineDash))
        {
        }

        LineAttribute::LineAttribute()
        :   mpLineAttribute(theGlobalDefault::get())
        {
        }

        LineAttribute::LineAttribute(const LineAttribute&) = default;

        LineAttribute::~LineAttribute() = default;

        bool LineAttribute::isDefault() const
        {
            return mpLineAttribute.same_object(theGlobalDefault::get());
        }

        LineAttribute& LineAttribute::operator=(const LineAttribute&) = default;

        bool LineAttribute::operator==(const LineAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpLineAttribute == mpLineAttribute;
        }

        const basegfx::BColor& LineAttribute::getColor() const
        {
            return mpLineAttribute->getColor();
        }

        double LineAttribute::getWidth() const
        {
            return mpLineAttribute->getWidth();
        }

        basegfx::B2DLineJoin LineAttribute::getLineJoin() const
        {
            return mpLineAttribute->getLineJoin();
        }

        css::drawing::LineCap LineAttribute::getLineCap() const
        {
            return mpLineAttribute->getLineCap();
        }

        css::drawing::LineDash LineAttribute::getLineDash() const
        {
            return mpLineAttribute->getLineDash();
        }

        double LineAttribute::getMiterMinimumAngle() const
        {
            return mpLineAttribute->getMiterMinimumAngle();
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
