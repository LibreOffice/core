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

#ifndef _BGFX_PIXEL_BPIXEL_HXX
#define _BGFX_PIXEL_BPIXEL_HXX

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BASEGFX_DLLPUBLIC BPixel
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
            } maRGBO;

            struct
            {
                // bitfield
                unsigned                                mnValue : 32;   // all values
            } maCombinedRGBO;
        } maPixelUnion;

    public:
        BPixel()
        {
            maPixelUnion.maCombinedRGBO.mnValue = 0L;
        }

        // use explicit here to make sure everyone knows what he is doing. Values range from
        // 0..255 integer here.
        explicit BPixel(sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue, sal_uInt8 nOpacity)
        {
            maPixelUnion.maRGBO.mnR = nRed;
            maPixelUnion.maRGBO.mnG = nGreen;
            maPixelUnion.maRGBO.mnB = nBlue;
            maPixelUnion.maRGBO.mnO = nOpacity;
        }

        // constructor from BColor which uses double precision color, so change it
        // to local integer format. It will also be clamped here.
        BPixel(const BColor& rColor, sal_uInt8 nOpacity)
        {
            maPixelUnion.maRGBO.mnR = sal_uInt8((rColor.getRed() * 255.0) + 0.5);
            maPixelUnion.maRGBO.mnG = sal_uInt8((rColor.getGreen() * 255.0) + 0.5);
            maPixelUnion.maRGBO.mnB = sal_uInt8((rColor.getBlue() * 255.0) + 0.5);
            maPixelUnion.maRGBO.mnO = nOpacity;
        }

        // copy constructor
        BPixel(const BPixel& rPixel)
        {
            maPixelUnion.maCombinedRGBO.mnValue = rPixel.maPixelUnion.maCombinedRGBO.mnValue;
        }

        ~BPixel()
        {}

        // assignment operator
        BPixel& operator=( const BPixel& rPixel )
        {
            maPixelUnion.maCombinedRGBO.mnValue = rPixel.maPixelUnion.maCombinedRGBO.mnValue;
            return *this;
        }

        // data access read
        sal_uInt8 getRed() const { return maPixelUnion.maRGBO.mnR; }
        sal_uInt8 getGreen() const { return maPixelUnion.maRGBO.mnG; }
        sal_uInt8 getBlue() const { return maPixelUnion.maRGBO.mnB; }
        sal_uInt8 getOpacity() const { return maPixelUnion.maRGBO.mnO; }
        sal_uInt32 getRedGreenBlueOpacity() const { return maPixelUnion.maCombinedRGBO.mnValue; }

        // data access write
        void setRed(sal_uInt8 nNew) { maPixelUnion.maRGBO.mnR = nNew; }
        void setGreen(sal_uInt8 nNew) { maPixelUnion.maRGBO.mnG = nNew; }
        void setBlue(sal_uInt8 nNew) { maPixelUnion.maRGBO.mnB = nNew; }
        void setOpacity(sal_uInt8 nNew) { maPixelUnion.maRGBO.mnO = nNew; }
        void setRedGreenBlueOpacity(sal_uInt32 nRedGreenBlueOpacity) { maPixelUnion.maCombinedRGBO.mnValue = nRedGreenBlueOpacity; }
        void setRedGreenBlue(sal_uInt8 nR, sal_uInt8 nG, sal_uInt8 nB) { maPixelUnion.maRGBO.mnR = nR; maPixelUnion.maRGBO.mnG = nG; maPixelUnion.maRGBO.mnB = nB; }

        // comparators
        bool isInvisible() const { return (0 == maPixelUnion.maRGBO.mnO); }
        bool isVisible() const { return (0 != maPixelUnion.maRGBO.mnO); }
        bool isEmpty() const { return isInvisible(); }
        bool isUsed() const { return isVisible(); }

        bool operator==( const BPixel& rPixel ) const
        {
            return (rPixel.maPixelUnion.maCombinedRGBO.mnValue == maPixelUnion.maCombinedRGBO.mnValue);
        }

        bool operator!=( const BPixel& rPixel ) const
        {
            return (rPixel.maPixelUnion.maCombinedRGBO.mnValue != maPixelUnion.maCombinedRGBO.mnValue);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // external operators

    inline BPixel minimum(const BPixel& rTupA, const BPixel& rTupB)
    {
        return BPixel(
            std::min(rTupB.getRed(), rTupA.getRed()),
            std::min(rTupB.getGreen(), rTupA.getGreen()),
            std::min(rTupB.getBlue(), rTupA.getBlue()),
            std::min(rTupB.getOpacity(), rTupA.getOpacity()));
    }

    inline BPixel maximum(const BPixel& rTupA, const BPixel& rTupB)
    {
        return BPixel(
            std::max(rTupB.getRed(), rTupA.getRed()),
            std::max(rTupB.getGreen(), rTupA.getGreen()),
            std::max(rTupB.getBlue(), rTupA.getBlue()),
            std::max(rTupB.getOpacity(), rTupA.getOpacity()));
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
        return BPixel(
            rOld1.getRed() == rOld2.getRed() ? rOld1.getRed() : (sal_uInt8)(((sal_uInt32)rOld1.getRed() + (sal_uInt32)rOld2.getRed()) >> 1L),
            rOld1.getGreen() == rOld2.getGreen() ? rOld1.getGreen() : (sal_uInt8)(((sal_uInt32)rOld1.getGreen() + (sal_uInt32)rOld2.getGreen()) >> 1L),
            rOld1.getBlue() == rOld2.getBlue() ? rOld1.getBlue() : (sal_uInt8)(((sal_uInt32)rOld1.getBlue() + (sal_uInt32)rOld2.getBlue()) >> 1L),
            rOld1.getOpacity() == rOld2.getOpacity() ? rOld1.getOpacity() : (sal_uInt8)(((sal_uInt32)rOld1.getOpacity() + (sal_uInt32)rOld2.getOpacity()) >> 1L));
    }

    inline BPixel average(const BPixel& rOld1, const BPixel& rOld2, const BPixel& rOld3)
    {
        return BPixel(
            (rOld1.getRed() == rOld2.getRed() && rOld2.getRed() == rOld3.getRed()) ? rOld1.getRed() : (sal_uInt8)(((sal_uInt32)rOld1.getRed() + (sal_uInt32)rOld2.getRed() + (sal_uInt32)rOld3.getRed()) / 3L),
            (rOld1.getGreen() == rOld2.getGreen() && rOld2.getGreen() == rOld3.getGreen()) ? rOld1.getGreen() : (sal_uInt8)(((sal_uInt32)rOld1.getGreen() + (sal_uInt32)rOld2.getGreen() + (sal_uInt32)rOld3.getGreen()) / 3L),
            (rOld1.getBlue() == rOld2.getBlue() && rOld2.getBlue() == rOld3.getBlue()) ? rOld1.getBlue() : (sal_uInt8)(((sal_uInt32)rOld1.getBlue() + (sal_uInt32)rOld2.getBlue() + (sal_uInt32)rOld3.getBlue()) / 3L),
            (rOld1.getOpacity() == rOld2.getOpacity() && rOld2.getOpacity() == rOld3.getOpacity()) ? rOld1.getOpacity() : (sal_uInt8)(((sal_uInt32)rOld1.getOpacity() + (sal_uInt32)rOld2.getOpacity() + (sal_uInt32)rOld3.getOpacity()) / 3L));
    }
} // end of namespace basegfx

#endif /* _BGFX_PIXEL_BPIXEL_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
