/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/attribute/sdrattribute.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineAttribute::SdrLineAttribute(
            basegfx::B2DLineJoin eJoin, double fWidth, double fTransparence, const basegfx::BColor& rColor,
            const ::std::vector< double >& rDotDashArray, double fFullDotDashLen)
        :   meJoin(eJoin),
            mfWidth(fWidth),
            mfTransparence(fTransparence),
            maColor(rColor),
            maDotDashArray(rDotDashArray),
            mfFullDotDashLen(fFullDotDashLen)
        {
        }

        SdrLineAttribute::SdrLineAttribute(const basegfx::BColor& rColor)
        :   meJoin(basegfx::B2DLINEJOIN_NONE),
            mfWidth(0.0),
            mfTransparence(0.0),
            maColor(rColor),
            maDotDashArray(),
            mfFullDotDashLen(0.0)
        {
        }

        SdrLineAttribute::~SdrLineAttribute()
        {
        }

        bool SdrLineAttribute::operator==(const SdrLineAttribute& rCandidate) const
        {
            return (meJoin == rCandidate.meJoin
                && mfWidth == rCandidate.mfWidth
                && mfTransparence == rCandidate.mfTransparence
                && maColor == rCandidate.maColor
                && maDotDashArray == rCandidate.maDotDashArray);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineStartEndAttribute::SdrLineStartEndAttribute(
            const basegfx::B2DPolyPolygon& rStartPolyPolygon, const basegfx::B2DPolyPolygon& rEndPolyPolygon,
            double fStartWidth, double fEndWidth, bool bStartActive, bool bEndActive, bool bStartCentered, bool bEndCentered)
        :   maStartPolyPolygon(rStartPolyPolygon),
            maEndPolyPolygon(rEndPolyPolygon),
            mfStartWidth(fStartWidth),
            mfEndWidth(fEndWidth),
            mbStartActive(bStartActive),
            mbEndActive(bEndActive),
            mbStartCentered(bStartCentered),
            mbEndCentered(bEndCentered)
        {
        }

        SdrLineStartEndAttribute::~SdrLineStartEndAttribute()
        {
        }

        bool SdrLineStartEndAttribute::operator==(const SdrLineStartEndAttribute& rCandidate) const
        {
            return (mbStartActive == rCandidate.mbStartActive
                && mbEndActive == rCandidate.mbEndActive
                && mbStartCentered == rCandidate.mbStartCentered
                && mbEndCentered == rCandidate.mbEndCentered
                && mfStartWidth == rCandidate.mfStartWidth
                && mfEndWidth == rCandidate.mfEndWidth
                && maStartPolyPolygon == rCandidate.maStartPolyPolygon
                && maEndPolyPolygon == rCandidate.maEndPolyPolygon);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrShadowAttribute::SdrShadowAttribute(const basegfx::B2DVector& rOffset, double fTransparence, const basegfx::BColor& rColor)
        :   maOffset(rOffset),
            mfTransparence(fTransparence),
            maColor(rColor)
        {
        }

        SdrShadowAttribute::~SdrShadowAttribute()
        {
        }

        bool SdrShadowAttribute::operator==(const SdrShadowAttribute& rCandidate) const
        {
            return (mfTransparence == rCandidate.mfTransparence
                && maColor == rCandidate.maColor
                && maOffset == rCandidate.maOffset);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrFillAttribute::SdrFillAttribute(
            double fTransparence,
            const basegfx::BColor& rColor,
            FillGradientAttribute* pGradient,
            FillHatchAttribute* pHatch,
            SdrFillBitmapAttribute* pBitmap)
        :   mfTransparence(fTransparence),
            maColor(rColor),
            mpGradient(pGradient),
            mpHatch(pHatch),
            mpBitmap(pBitmap)
        {
        }

        SdrFillAttribute::SdrFillAttribute(const SdrFillAttribute& rCandidate)
        :   mfTransparence(1.0),
            mpGradient(0L),
            mpHatch(0L),
            mpBitmap(0L)
        {
            if(!(*this == rCandidate))
            {
                *this = rCandidate;
            }
        }

        SdrFillAttribute::~SdrFillAttribute()
        {
            delete mpGradient;
            delete mpHatch;
            delete mpBitmap;
        }

        SdrFillAttribute& SdrFillAttribute::operator=(const SdrFillAttribute& rCandidate)
        {
            // copy data
            mfTransparence = rCandidate.mfTransparence;
            maColor = rCandidate.maColor;

            // handle mpGradient
            {
                // delete local mpGradient if necessary
                if(mpGradient && ((!rCandidate.mpGradient) || (!(*mpGradient == *rCandidate.mpGradient))))
                {
                    delete mpGradient;
                    mpGradient = 0L;
                }

                // copy mpGradient if necessary
                if(!mpGradient && rCandidate.mpGradient)
                {
                    mpGradient = new FillGradientAttribute(*rCandidate.mpGradient);
                }
            }

            // handle mpHatch
            {
                // delete local mpHatch if necessary
                if(mpHatch && ((!rCandidate.mpHatch) || (!(*mpHatch == *rCandidate.mpHatch))))
                {
                    delete mpHatch;
                    mpHatch = 0L;
                }

                // copy mpHatch if necessary
                if(!mpHatch && rCandidate.mpHatch)
                {
                    mpHatch = new FillHatchAttribute(*rCandidate.mpHatch);
                }
            }

            // handle mpBitmap
            {
                // delete local mpBitmap if necessary
                if(mpBitmap && ((!rCandidate.mpBitmap) || (!(*mpBitmap == *rCandidate.mpBitmap))))
                {
                    delete mpBitmap;
                    mpBitmap = 0L;
                }

                // copy mpBitmap if necessary
                if(!mpBitmap && rCandidate.mpBitmap)
                {
                    mpBitmap = new SdrFillBitmapAttribute(*rCandidate.mpBitmap);
                }
            }

            return *this;
        }

        bool SdrFillAttribute::operator==(const SdrFillAttribute& rCandidate) const
        {
            if(mfTransparence != rCandidate.mfTransparence)
                return false;

            if(mpGradient)
            {
                if(!rCandidate.mpGradient)
                    return false;

                if(!(*mpGradient == *rCandidate.mpGradient))
                    return false;
            }
            else if(mpHatch)
            {
                if(!rCandidate.mpHatch)
                    return false;

                if(!(*mpHatch == *rCandidate.mpHatch))
                    return false;

                if(mpHatch->isFillBackground() && !(maColor == rCandidate.maColor))
                    return false;
            }
            else if(mpBitmap)
            {
                if(!rCandidate.mpBitmap)
                    return false;

                if(!(*mpBitmap == *rCandidate.mpBitmap))
                    return false;
            }
            else
            {
                if(!rCandidate.isColor())
                    return false;

                if(!(maColor == rCandidate.maColor))
                    return false;
            }

            return true;
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
