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

#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrLineAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // line definitions
            basegfx::B2DLineJoin                    meJoin;             // B2DLINEJOIN_* defines
            double                                  mfWidth;            // 1/100th mm, 0.0==hair
            double                                  mfTransparence;     // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                         maColor;            // color of line
            ::std::vector< double >                 maDotDashArray;     // array of double which defines the dot-dash pattern
            double                                  mfFullDotDashLen;   // sum of maDotDashArray (for convenience)

            ImpSdrLineAttribute(
                basegfx::B2DLineJoin eJoin,
                double fWidth,
                double fTransparence,
                const basegfx::BColor& rColor,
                const ::std::vector< double >& rDotDashArray,
                double fFullDotDashLen)
            :   mnRefCount(0),
                meJoin(eJoin),
                mfWidth(fWidth),
                mfTransparence(fTransparence),
                maColor(rColor),
                maDotDashArray(rDotDashArray),
                mfFullDotDashLen(fFullDotDashLen)
            {
            }

            explicit ImpSdrLineAttribute(const basegfx::BColor& rColor)
            :   mnRefCount(0),
                meJoin(basegfx::B2DLINEJOIN_NONE),
                mfWidth(0.0),
                mfTransparence(0.0),
                maColor(rColor),
                maDotDashArray(),
                mfFullDotDashLen(0.0)
            {
            }

            // data read access
            basegfx::B2DLineJoin getJoin() const { return meJoin; }
            double getWidth() const { return mfWidth; }
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
            const ::std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const { return mfFullDotDashLen; }

            bool operator==(const ImpSdrLineAttribute& rCandidate) const
            {
                return (getJoin() == rCandidate.getJoin()
                    && getWidth() == rCandidate.getWidth()
                    && getTransparence() == rCandidate.getTransparence()
                    && getColor() == rCandidate.getColor()
                    && getDotDashArray() == rCandidate.getDotDashArray());
            }

            static ImpSdrLineAttribute* get_global_default()
            {
                static ImpSdrLineAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrLineAttribute(
                        basegfx::B2DLINEJOIN_ROUND,
                        0.0,
                        0.0,
                        basegfx::BColor(),
                        std::vector< double >(),
                        0.0);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrLineAttribute::SdrLineAttribute(
            basegfx::B2DLineJoin eJoin,
            double fWidth,
            double fTransparence,
            const basegfx::BColor& rColor,
            const ::std::vector< double >& rDotDashArray,
            double fFullDotDashLen)
        :   mpSdrLineAttribute(new ImpSdrLineAttribute(
                eJoin, fWidth, fTransparence, rColor, rDotDashArray, fFullDotDashLen))
        {
        }

        SdrLineAttribute::SdrLineAttribute()
        :   mpSdrLineAttribute(ImpSdrLineAttribute::get_global_default())
        {
            mpSdrLineAttribute->mnRefCount++;
        }

        SdrLineAttribute::SdrLineAttribute(const SdrLineAttribute& rCandidate)
        :   mpSdrLineAttribute(rCandidate.mpSdrLineAttribute)
        {
            mpSdrLineAttribute->mnRefCount++;
        }

        SdrLineAttribute::~SdrLineAttribute()
        {
            if(mpSdrLineAttribute->mnRefCount)
            {
                mpSdrLineAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrLineAttribute;
            }
        }

        bool SdrLineAttribute::isDefault() const
        {
            return mpSdrLineAttribute == ImpSdrLineAttribute::get_global_default();
        }

        SdrLineAttribute& SdrLineAttribute::operator=(const SdrLineAttribute& rCandidate)
        {
            if(rCandidate.mpSdrLineAttribute != mpSdrLineAttribute)
            {
                if(mpSdrLineAttribute->mnRefCount)
                {
                    mpSdrLineAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrLineAttribute;
                }

                mpSdrLineAttribute = rCandidate.mpSdrLineAttribute;
                mpSdrLineAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrLineAttribute::operator==(const SdrLineAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrLineAttribute == mpSdrLineAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrLineAttribute == *mpSdrLineAttribute);
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

        const ::std::vector< double >& SdrLineAttribute::getDotDashArray() const
        {
            return mpSdrLineAttribute->getDotDashArray();
        }

        double SdrLineAttribute::getFullDotDashLen() const
        {
            return mpSdrLineAttribute->getFullDotDashLen();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
