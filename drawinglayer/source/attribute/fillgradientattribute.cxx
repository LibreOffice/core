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
#include <basegfx/color/bcolor.hxx>
#include <rtl/instance.hxx>



namespace drawinglayer
{
    namespace attribute
    {
        class ImpFillGradientAttribute
        {
        public:
            // data definitions
            GradientStyle                           meStyle;
            double                                  mfBorder;
            double                                  mfOffsetX;
            double                                  mfOffsetY;
            double                                  mfAngle;
            basegfx::BColor                         maStartColor;
            basegfx::BColor                         maEndColor;
            sal_uInt16                              mnSteps;

            ImpFillGradientAttribute(
                GradientStyle eStyle,
                double fBorder,
                double fOffsetX,
                double fOffsetY,
                double fAngle,
                const basegfx::BColor& rStartColor,
                const basegfx::BColor& rEndColor,
                sal_uInt16 nSteps)
            :   meStyle(eStyle),
                mfBorder(fBorder),
                mfOffsetX(fOffsetX),
                mfOffsetY(fOffsetY),
                mfAngle(fAngle),
                maStartColor(rStartColor),
                maEndColor(rEndColor),
                mnSteps(nSteps)
            {
            }

            ImpFillGradientAttribute()
            :   meStyle(GRADIENTSTYLE_LINEAR),
                mfBorder(0.0),
                mfOffsetX(0.0),
                mfOffsetY(0.0),
                mfAngle(0.0),
                maStartColor(basegfx::BColor()),
                maEndColor(basegfx::BColor()),
                mnSteps(0)
            {
            }

            // data read access
            GradientStyle getStyle() const { return meStyle; }
            double getBorder() const { return mfBorder; }
            double getOffsetX() const { return mfOffsetX; }
            double getOffsetY() const { return mfOffsetY; }
            double getAngle() const { return mfAngle; }
            const basegfx::BColor& getStartColor() const { return maStartColor; }
            const basegfx::BColor& getEndColor() const { return maEndColor; }
            sal_uInt16 getSteps() const { return mnSteps; }

            bool operator==(const ImpFillGradientAttribute& rCandidate) const
            {
                return (getStyle() == rCandidate.getStyle()
                    && getBorder() == rCandidate.getBorder()
                    && getOffsetX() == rCandidate.getOffsetX()
                    && getOffsetY() == rCandidate.getOffsetY()
                    && getAngle() == rCandidate.getAngle()
                    && getStartColor() == rCandidate.getStartColor()
                    && getEndColor() == rCandidate.getEndColor()
                    && getSteps() == rCandidate.getSteps());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< FillGradientAttribute::ImplType, theGlobalDefault > {};
        }

        FillGradientAttribute::FillGradientAttribute(
            GradientStyle eStyle,
            double fBorder,
            double fOffsetX,
            double fOffsetY,
            double fAngle,
            const basegfx::BColor& rStartColor,
            const basegfx::BColor& rEndColor,
            sal_uInt16 nSteps)
        :   mpFillGradientAttribute(ImpFillGradientAttribute(
                eStyle, fBorder, fOffsetX, fOffsetY, fAngle, rStartColor, rEndColor, nSteps))
        {
        }

        FillGradientAttribute::FillGradientAttribute()
        :   mpFillGradientAttribute(theGlobalDefault::get())
        {
        }

        FillGradientAttribute::FillGradientAttribute(const FillGradientAttribute& rCandidate)
        :   mpFillGradientAttribute(rCandidate.mpFillGradientAttribute)
        {
        }

        FillGradientAttribute::~FillGradientAttribute()
        {
        }

        bool FillGradientAttribute::isDefault() const
        {
            return mpFillGradientAttribute.same_object(theGlobalDefault::get());
        }

        FillGradientAttribute& FillGradientAttribute::operator=(const FillGradientAttribute& rCandidate)
        {
            mpFillGradientAttribute = rCandidate.mpFillGradientAttribute;
            return *this;
        }

        bool FillGradientAttribute::operator==(const FillGradientAttribute& rCandidate) const
        {
            return rCandidate.mpFillGradientAttribute == mpFillGradientAttribute;
        }

        const basegfx::BColor& FillGradientAttribute::getStartColor() const
        {
            return mpFillGradientAttribute->getStartColor();
        }

        const basegfx::BColor& FillGradientAttribute::getEndColor() const
        {
            return mpFillGradientAttribute->getEndColor();
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

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
