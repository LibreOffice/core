/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrattribute.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:47:34 $
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

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        sdrLineAttribute::sdrLineAttribute(
            basegfx::tools::B2DLineJoin eJoin, double fWidth, double fTransparence, const basegfx::BColor& rColor,
            const ::std::vector< double >& rDotDashArray, double fFullDotDashLen)
        :   meJoin(eJoin),
            mfWidth(fWidth),
            mfTransparence(fTransparence),
            maColor(rColor),
            maDotDashArray(rDotDashArray),
            mfFullDotDashLen(fFullDotDashLen)
        {
        }

        sdrLineAttribute::~sdrLineAttribute()
        {
        }

        bool sdrLineAttribute::operator==(const sdrLineAttribute& rCandidate) const
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
        sdrLineStartEndAttribute::sdrLineStartEndAttribute(
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

        sdrLineStartEndAttribute::~sdrLineStartEndAttribute()
        {
        }

        bool sdrLineStartEndAttribute::operator==(const sdrLineStartEndAttribute& rCandidate) const
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
        sdrShadowAttribute::sdrShadowAttribute(const basegfx::B2DVector& rOffset, double fTransparence, const basegfx::BColor& rColor)
        :   maOffset(rOffset),
            mfTransparence(fTransparence),
            maColor(rColor)
        {
        }

        sdrShadowAttribute::~sdrShadowAttribute()
        {
        }

        bool sdrShadowAttribute::operator==(const sdrShadowAttribute& rCandidate) const
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
        sdrFillAttribute::sdrFillAttribute(
            double fTransparence, const basegfx::BColor& rColor, fillGradientAttribute* pGradient,
            fillHatchAttribute* pHatch, sdrFillBitmapAttribute* pBitmap)
        :   mfTransparence(fTransparence),
            maColor(rColor),
            mpGradient(pGradient),
            mpHatch(pHatch),
            mpBitmap(pBitmap)
        {
        }

        sdrFillAttribute::sdrFillAttribute(const sdrFillAttribute& rCandidate)
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

        sdrFillAttribute::~sdrFillAttribute()
        {
            delete mpGradient;
            delete mpHatch;
            delete mpBitmap;
        }

        sdrFillAttribute& sdrFillAttribute::operator=(const sdrFillAttribute& rCandidate)
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
                    mpGradient = new fillGradientAttribute(*rCandidate.mpGradient);
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
                    mpHatch = new fillHatchAttribute(*rCandidate.mpHatch);
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
                    mpBitmap = new sdrFillBitmapAttribute(*rCandidate.mpBitmap);
                }
            }

            return *this;
        }

        bool sdrFillAttribute::operator==(const sdrFillAttribute& rCandidate) const
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
