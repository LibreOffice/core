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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
