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

#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFillAttribute
        {
        public:
            // refcounter
            sal_uInt32                          mnRefCount;

            // fill definitions
            double                              mfTransparence;     // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                     maColor;            // fill color
            FillGradientAttribute               maGradient;         // fill gradient (if used)
            FillHatchAttribute                  maHatch;            // fill hatch (if used)
            SdrFillBitmapAttribute              maBitmap;           // fill bitmap (if used)

        public:
            ImpSdrFillAttribute(
                double fTransparence,
                const basegfx::BColor& rColor,
                const FillGradientAttribute& rGradient,
                const FillHatchAttribute& rHatch,
                const SdrFillBitmapAttribute& rBitmap)
            :   mnRefCount(0),
                mfTransparence(fTransparence),
                maColor(rColor),
                maGradient(rGradient),
                maHatch(rHatch),
                maBitmap(rBitmap)
            {
            }

            // data read access
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
            const FillGradientAttribute& getGradient() const { return maGradient; }
            const FillHatchAttribute& getHatch() const { return maHatch; }
            const SdrFillBitmapAttribute& getBitmap() const { return maBitmap; }

            // compare operator
            bool operator==(const ImpSdrFillAttribute& rCandidate) const
            {
                return(getTransparence() == rCandidate.getTransparence()
                    && getColor() == rCandidate.getColor()
                    && getGradient() == rCandidate.getGradient()
                    && getHatch() == rCandidate.getHatch()
                    && getBitmap() == rCandidate.getBitmap());
            }

            static ImpSdrFillAttribute* get_global_default()
            {
                static ImpSdrFillAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrFillAttribute(
                        0.0,
                        basegfx::BColor(),
                        FillGradientAttribute(),
                        FillHatchAttribute(),
                        SdrFillBitmapAttribute());

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrFillAttribute::SdrFillAttribute(
            double fTransparence,
            const basegfx::BColor& rColor,
            const FillGradientAttribute& rGradient,
            const FillHatchAttribute& rHatch,
            const SdrFillBitmapAttribute& rBitmap)
        :   mpSdrFillAttribute(new ImpSdrFillAttribute(
                fTransparence, rColor, rGradient, rHatch, rBitmap))
        {
        }

        SdrFillAttribute::SdrFillAttribute()
        :   mpSdrFillAttribute(ImpSdrFillAttribute::get_global_default())
        {
            mpSdrFillAttribute->mnRefCount++;
        }

        SdrFillAttribute::SdrFillAttribute(const SdrFillAttribute& rCandidate)
        :   mpSdrFillAttribute(rCandidate.mpSdrFillAttribute)
        {
            mpSdrFillAttribute->mnRefCount++;
        }

        SdrFillAttribute::~SdrFillAttribute()
        {
            if(mpSdrFillAttribute->mnRefCount)
            {
                mpSdrFillAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrFillAttribute;
            }
        }

        bool SdrFillAttribute::isDefault() const
        {
            return mpSdrFillAttribute == ImpSdrFillAttribute::get_global_default();
        }

        SdrFillAttribute& SdrFillAttribute::operator=(const SdrFillAttribute& rCandidate)
        {
            if(rCandidate.mpSdrFillAttribute != mpSdrFillAttribute)
            {
                if(mpSdrFillAttribute->mnRefCount)
                {
                    mpSdrFillAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrFillAttribute;
                }

                mpSdrFillAttribute = rCandidate.mpSdrFillAttribute;
                mpSdrFillAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrFillAttribute::operator==(const SdrFillAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrFillAttribute == mpSdrFillAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrFillAttribute == *mpSdrFillAttribute);
        }

        double SdrFillAttribute::getTransparence() const
        {
            return mpSdrFillAttribute->getTransparence();
        }

        const basegfx::BColor& SdrFillAttribute::getColor() const
        {
            return mpSdrFillAttribute->getColor();
        }

        const FillGradientAttribute& SdrFillAttribute::getGradient() const
        {
            return mpSdrFillAttribute->getGradient();
        }

        const FillHatchAttribute& SdrFillAttribute::getHatch() const
        {
            return mpSdrFillAttribute->getHatch();
        }

        const SdrFillBitmapAttribute& SdrFillAttribute::getBitmap() const
        {
            return mpSdrFillAttribute->getBitmap();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
