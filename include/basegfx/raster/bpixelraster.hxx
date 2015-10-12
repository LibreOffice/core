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

#ifndef INCLUDED_BASEGFX_RASTER_BPIXELRASTER_HXX
#define INCLUDED_BASEGFX_RASTER_BPIXELRASTER_HXX

#include <algorithm>
#include <string.h>
#include <sal/types.h>
#include <basegfx/pixel/bpixel.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class BPixelRaster
    {
    private:
        BPixelRaster(const BPixelRaster&) = delete;
        BPixelRaster& operator=(const BPixelRaster&) = delete;

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
            assert(nIndex < mnCount && "Access out of range");
            return mpContent[nIndex];
        }

        // data access read/write
        BPixel& getBPixel(sal_uInt32 nIndex)
        {
            assert(nIndex < mnCount && "Access out of range");
            return mpContent[nIndex];
        }
    };
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_RASTER_BPIXELRASTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
