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



namespace drawinglayer
{
    namespace attribute
    {
        class ImpLineAttribute
        {
        public:
            // data definitions
            basegfx::BColor                         maColor;                // color
            double                                  mfWidth;                // absolute line width
            basegfx::B2DLineJoin                    meLineJoin;             // type of LineJoin
            css::drawing::LineCap                   meLineCap;              // BUTT, ROUND, or SQUARE

            ImpLineAttribute(
                const basegfx::BColor& rColor,
                double fWidth,
                basegfx::B2DLineJoin aB2DLineJoin,
                css::drawing::LineCap aLineCap)
            :   maColor(rColor),
                mfWidth(fWidth),
                meLineJoin(aB2DLineJoin),
                meLineCap(aLineCap)
            {
            }

            ImpLineAttribute()
            :   maColor(basegfx::BColor()),
                mfWidth(0.0),
                meLineJoin(basegfx::B2DLineJoin::Round),
                meLineCap(css::drawing::LineCap_BUTT)
            {
            }

            // data read access
            const basegfx::BColor& getColor() const { return maColor; }
            double getWidth() const { return mfWidth; }
            basegfx::B2DLineJoin getLineJoin() const { return meLineJoin; }
            css::drawing::LineCap getLineCap() const { return meLineCap; }

            bool operator==(const ImpLineAttribute& rCandidate) const
            {
                return (getColor() == rCandidate.getColor()
                    && getWidth() == rCandidate.getWidth()
                    && getLineJoin() == rCandidate.getLineJoin()
                    && getLineCap() == rCandidate.getLineCap());
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
            css::drawing::LineCap aLineCap)
        :   mpLineAttribute(
                ImpLineAttribute(
                    rColor,
                    fWidth,
                    aB2DLineJoin,
                    aLineCap))
        {
        }

        LineAttribute::LineAttribute()
        :   mpLineAttribute(theGlobalDefault::get())
        {
        }

        LineAttribute::LineAttribute(const LineAttribute& rCandidate)
        :   mpLineAttribute(rCandidate.mpLineAttribute)
        {
        }

        LineAttribute::~LineAttribute()
        {
        }

        bool LineAttribute::isDefault() const
        {
            return mpLineAttribute.same_object(theGlobalDefault::get());
        }

        LineAttribute& LineAttribute::operator=(const LineAttribute& rCandidate)
        {
            mpLineAttribute = rCandidate.mpLineAttribute;
            return *this;
        }

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

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
