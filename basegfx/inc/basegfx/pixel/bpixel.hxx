/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bpixel.hxx,v $
 *
 * $Revision: 1.2 $
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

#ifndef _BGFX_PIXEL_BPIXEL_HXX
#define _BGFX_PIXEL_BPIXEL_HXX

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BPixel
    {
    protected:
        union
        {
            struct
            {
                // bitfield
                unsigned                                mnR : 8;        // red intensity
                unsigned                                mnG : 8;        // green intensity
                unsigned                                mnB : 8;        // blue intensity
                unsigned                                mnO : 8;        // opacity, 0 == full transparence
            }                                           maRGBO;

            struct
            {
                // bitfield
                unsigned                                mnValue : 32;   // all values
            }                                           maCombinedRGBO;
        };

    public:
        BPixel()
        {
            maCombinedRGBO.mnValue = 0L;
        }

        // use explicit here to make sure everyone knows what he is doing. Values range from
        // 0..255 integer here.
        explicit BPixel(sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue, sal_uInt8 nOpacity)
        {
            maRGBO.mnR = nRed;
            maRGBO.mnG = nGreen;
            maRGBO.mnB = nBlue;
            maRGBO.mnO = nOpacity;
        }

        // constructor from BColor which uses double precision color, so change it
        // to local integer format. It will also be clamped here.
        BPixel(const BColor& rColor, sal_uInt8 nOpacity)
        {
            maRGBO.mnR = sal_uInt8((rColor.getRed() * 255.0) + 0.5);
            maRGBO.mnG = sal_uInt8((rColor.getGreen() * 255.0) + 0.5);
            maRGBO.mnB = sal_uInt8((rColor.getBlue() * 255.0) + 0.5);
            maRGBO.mnO = nOpacity;
        }

        // copy constructor
        BPixel(const BPixel& rPixel)
        {
            maCombinedRGBO.mnValue = rPixel.maCombinedRGBO.mnValue;
        }

        ~BPixel()
        {}

        // assignment operator
        BPixel& operator=( const BPixel& rPixel )
        {
            maCombinedRGBO.mnValue = rPixel.maCombinedRGBO.mnValue;
            return *this;
        }

        // data access read
        sal_uInt8 getRed() const { return maRGBO.mnR; }
        sal_uInt8 getGreen() const { return maRGBO.mnG; }
        sal_uInt8 getBlue() const { return maRGBO.mnB; }
        sal_uInt8 getOpacity() const { return maRGBO.mnO; }
        sal_uInt32 getRedGreenBlueOpacity() const { return maCombinedRGBO.mnValue; }

        // data access write
        void setRed(sal_uInt8 nNew) { maRGBO.mnR = nNew; }
        void setGreen(sal_uInt8 nNew) { maRGBO.mnG = nNew; }
        void setBlue(sal_uInt8 nNew) { maRGBO.mnB = nNew; }
        void setOpacity(sal_uInt8 nNew) { maRGBO.mnO = nNew; }
        void setRedGreenBlueOpacity(sal_uInt32 nRedGreenBlueOpacity) { maCombinedRGBO.mnValue = nRedGreenBlueOpacity; }
        void setRedGreenBlue(sal_uInt8 nR, sal_uInt8 nG, sal_uInt8 nB) { maRGBO.mnR = nR; maRGBO.mnG = nG; maRGBO.mnB = nB; }

        // comparators
        bool isInvisible() const { return (0 == maRGBO.mnO); }
        bool isVisible() const { return (0 != maRGBO.mnO); }
        bool isEmpty() const { return isInvisible(); }
        bool isUsed() const { return isVisible(); }

        bool operator==( const BPixel& rPixel ) const
        {
            return (rPixel.maCombinedRGBO.mnValue == maCombinedRGBO.mnValue);
        }

        bool operator!=( const BPixel& rPixel ) const
        {
            return (rPixel.maCombinedRGBO.mnValue != maCombinedRGBO.mnValue);
        }

        // empty element
        static const BPixel& getEmptyBPixel();
    };

    //////////////////////////////////////////////////////////////////////////
    // external operators

    inline BPixel minimum(const BPixel& rTupA, const BPixel& rTupB)
    {
        BPixel aMin(
            (rTupB.getRed() < rTupA.getRed()) ? rTupB.getRed() : rTupA.getRed(),
            (rTupB.getGreen() < rTupA.getGreen()) ? rTupB.getGreen() : rTupA.getGreen(),
            (rTupB.getBlue() < rTupA.getBlue()) ? rTupB.getBlue() : rTupA.getBlue(),
            (rTupB.getOpacity() < rTupA.getOpacity()) ? rTupB.getOpacity() : rTupA.getOpacity());
        return aMin;
    }

    inline BPixel maximum(const BPixel& rTupA, const BPixel& rTupB)
    {
        BPixel aMax(
            (rTupB.getRed() > rTupA.getRed()) ? rTupB.getRed() : rTupA.getRed(),
            (rTupB.getGreen() > rTupA.getGreen()) ? rTupB.getGreen() : rTupA.getGreen(),
            (rTupB.getBlue() > rTupA.getBlue()) ? rTupB.getBlue() : rTupA.getBlue(),
            (rTupB.getOpacity() > rTupA.getOpacity()) ? rTupB.getOpacity() : rTupA.getOpacity());
        return aMax;
    }

    inline BPixel interpolate(const BPixel& rOld1, const BPixel& rOld2, double t)
    {
        if(rOld1 == rOld2)
        {
            return rOld1;
        }
        else if(0.0 >= t)
        {
            return rOld1;
        }
        else if(1.0 <= t)
        {
            return rOld2;
        }
        else
        {
            const sal_uInt32 nFactor(fround(256.0 * t));
            const sal_uInt32 nNegFac(256L - nFactor);
            return BPixel(
                (sal_uInt8)(((sal_uInt32)rOld1.getRed() * nNegFac + (sal_uInt32)rOld2.getRed() * nFactor) >> 8L),
                (sal_uInt8)(((sal_uInt32)rOld1.getGreen() * nNegFac + (sal_uInt32)rOld2.getGreen() * nFactor) >> 8L),
                (sal_uInt8)(((sal_uInt32)rOld1.getBlue() * nNegFac + (sal_uInt32)rOld2.getBlue() * nFactor) >> 8L),
                (sal_uInt8)(((sal_uInt32)rOld1.getOpacity() * nNegFac + (sal_uInt32)rOld2.getOpacity() * nFactor) >> 8L));
        }
    }

    inline BPixel average(const BPixel& rOld1, const BPixel& rOld2)
    {
        if(rOld1 == rOld2)
        {
            return rOld1;
        }
        else
        {
            return BPixel(
                (sal_uInt8)(((sal_uInt32)rOld1.getRed() + (sal_uInt32)rOld2.getRed()) >> 1L),
                (sal_uInt8)(((sal_uInt32)rOld1.getGreen() + (sal_uInt32)rOld2.getGreen()) >> 1L),
                (sal_uInt8)(((sal_uInt32)rOld1.getBlue() + (sal_uInt32)rOld2.getBlue()) >> 1L),
                (sal_uInt8)(((sal_uInt32)rOld1.getOpacity() + (sal_uInt32)rOld2.getOpacity()) >> 1L));
        }
    }

    inline BPixel average(const BPixel& rOld1, const BPixel& rOld2, const BPixel& rOld3)
    {
        if(rOld1 == rOld2 && rOld2 == rOld3)
        {
            return rOld1;
        }
        else
        {
            return BPixel(
                (sal_uInt8)(((sal_uInt32)rOld1.getRed() + (sal_uInt32)rOld2.getRed() + (sal_uInt32)rOld3.getRed()) / 3L),
                (sal_uInt8)(((sal_uInt32)rOld1.getGreen() + (sal_uInt32)rOld2.getGreen() + (sal_uInt32)rOld3.getGreen()) / 3L),
                (sal_uInt8)(((sal_uInt32)rOld1.getBlue() + (sal_uInt32)rOld2.getBlue() + (sal_uInt32)rOld3.getBlue()) / 3L),
                (sal_uInt8)(((sal_uInt32)rOld1.getOpacity() + (sal_uInt32)rOld2.getOpacity() + (sal_uInt32)rOld3.getOpacity()) / 3L));
        }
    }
} // end of namespace basegfx

#endif /* _BGFX_PIXEL_BPIXEL_HXX */
