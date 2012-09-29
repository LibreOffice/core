/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _BGFX_RASTER_BPIXELRASTER_HXX
#define _BGFX_RASTER_BPIXELRASTER_HXX

#include <algorithm>
#include <sal/types.h>
#include <basegfx/pixel/bpixel.hxx>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BPixelRaster
    {
    private:
        // do not allow copy constructor and assignment operator
        BPixelRaster(const BPixelRaster&);
        BPixelRaster& operator=(const BPixelRaster&);

    protected:
        sal_uInt32                  mnWidth;
        sal_uInt32                  mnHeight;
        sal_uInt32                  mnCount;
        BPixel*                     mpContent;

    public:
        // reset
        void reset()
        {
            memset(mpContent, 0, sizeof(BPixel) * mnCount);
        }

        // constructor/destructor
        BPixelRaster(sal_uInt32 nWidth, sal_uInt32 nHeight)
        :   mnWidth(nWidth),
            mnHeight(nHeight),
            mnCount(nWidth * nHeight),
            mpContent(new BPixel[mnCount])
        {
            reset();
        }

        ~BPixelRaster()
        {
            delete [] mpContent;
        }

        // coordinate calcs between X/Y and span
        sal_uInt32 getIndexFromXY(sal_uInt32 nX, sal_uInt32 nY) const { return (nX + (nY * mnWidth)); }
        sal_uInt32 getXFromIndex(sal_uInt32 nIndex) const { return (nIndex % mnWidth); }
        sal_uInt32 getYFromIndex(sal_uInt32 nIndex) const { return (nIndex / mnWidth); }

        // data access read
        sal_uInt32 getWidth() const { return mnWidth; }
        sal_uInt32 getHeight() const { return mnHeight; }
        sal_uInt32 getCount() const { return mnCount; }

        // data access read only
        const BPixel& getBPixel(sal_uInt32 nIndex) const
        {
#ifdef DBG_UTIL
            if(nIndex >= mnCount)
            {
                OSL_FAIL("getBPixel: Access out of range (!)");
                return BPixel::getEmptyBPixel();
            }
#endif
            return mpContent[nIndex];
        }

        // data access read/write
        BPixel& getBPixel(sal_uInt32 nIndex)
        {
#ifdef DBG_UTIL
            if(nIndex >= mnCount)
            {
                OSL_FAIL("getBPixel: Access out of range (!)");
                return mpContent[0L];
            }
#endif
            return mpContent[nIndex];
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RASTER_BPIXELRASTER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
