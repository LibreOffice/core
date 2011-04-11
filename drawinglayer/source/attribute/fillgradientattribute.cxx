/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpFillGradientAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

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
            :   mnRefCount(0),
                meStyle(eStyle),
                mfBorder(fBorder),
                mfOffsetX(fOffsetX),
                mfOffsetY(fOffsetY),
                mfAngle(fAngle),
                maStartColor(rStartColor),
                maEndColor(rEndColor),
                mnSteps(nSteps)
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

            static ImpFillGradientAttribute* get_global_default()
            {
                static ImpFillGradientAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpFillGradientAttribute(
                        GRADIENTSTYLE_LINEAR,
                        0.0, 0.0, 0.0, 0.0,
                        basegfx::BColor(),
                        basegfx::BColor(),
                        0);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        FillGradientAttribute::FillGradientAttribute(
            GradientStyle eStyle,
            double fBorder,
            double fOffsetX,
            double fOffsetY,
            double fAngle,
            const basegfx::BColor& rStartColor,
            const basegfx::BColor& rEndColor,
            sal_uInt16 nSteps)
        :   mpFillGradientAttribute(new ImpFillGradientAttribute(
                eStyle, fBorder, fOffsetX, fOffsetY, fAngle, rStartColor, rEndColor, nSteps))
        {
        }

        FillGradientAttribute::FillGradientAttribute()
        :   mpFillGradientAttribute(ImpFillGradientAttribute::get_global_default())
        {
            mpFillGradientAttribute->mnRefCount++;
        }

        FillGradientAttribute::FillGradientAttribute(const FillGradientAttribute& rCandidate)
        :   mpFillGradientAttribute(rCandidate.mpFillGradientAttribute)
        {
            mpFillGradientAttribute->mnRefCount++;
        }

        FillGradientAttribute::~FillGradientAttribute()
        {
            if(mpFillGradientAttribute->mnRefCount)
            {
                mpFillGradientAttribute->mnRefCount--;
            }
            else
            {
                delete mpFillGradientAttribute;
            }
        }

        bool FillGradientAttribute::isDefault() const
        {
            return mpFillGradientAttribute == ImpFillGradientAttribute::get_global_default();
        }

        FillGradientAttribute& FillGradientAttribute::operator=(const FillGradientAttribute& rCandidate)
        {
            if(rCandidate.mpFillGradientAttribute != mpFillGradientAttribute)
            {
                if(mpFillGradientAttribute->mnRefCount)
                {
                    mpFillGradientAttribute->mnRefCount--;
                }
                else
                {
                    delete mpFillGradientAttribute;
                }

                mpFillGradientAttribute = rCandidate.mpFillGradientAttribute;
                mpFillGradientAttribute->mnRefCount++;
            }

            return *this;
        }

        bool FillGradientAttribute::operator==(const FillGradientAttribute& rCandidate) const
        {
            if(rCandidate.mpFillGradientAttribute == mpFillGradientAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpFillGradientAttribute == *mpFillGradientAttribute);
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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
