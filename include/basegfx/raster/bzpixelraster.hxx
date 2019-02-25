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

#ifndef INCLUDED_BASEGFX_RASTER_BZPIXELRASTER_HXX
#define INCLUDED_BASEGFX_RASTER_BZPIXELRASTER_HXX

#include <basegfx/pixel/bpixel.hxx>
#include <sal/types.h>
#include <memory>
#include <string.h>

namespace basegfx
{
    class BZPixelRaster
    {
    private:
        BZPixelRaster(const BZPixelRaster&) = delete;
        BZPixelRaster& operator=(const BZPixelRaster&) = delete;

        sal_uInt32                  mnWidth;
        sal_uInt32                  mnHeight;
        sal_uInt32                  mnCount;
        std::unique_ptr<BPixel[]>   mpContent;
        std::unique_ptr<sal_uInt16[]>  mpZBuffer;

    public:
        // constructor/destructor
        BZPixelRaster(sal_uInt32 nWidth, sal_uInt32 nHeight)
        :   mnWidth(nWidth),
            mnHeight(nHeight),
            mnCount(nWidth * nHeight),
            mpContent(new BPixel[mnCount]),
            mpZBuffer(new sal_uInt16[mnCount])
        {
            memset(mpZBuffer.get(), 0, sizeof(sal_uInt16) * mnCount);
        }

         // coordinate calcs between X/Y and span
        sal_uInt32 getIndexFromXY(sal_uInt32 nX, sal_uInt32 nY) const { return (nX + (nY * mnWidth)); }

        // data access read
        sal_uInt32 getWidth() const { return mnWidth; }
        sal_uInt32 getHeight() const { return mnHeight; }

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

        // data access read only
        const sal_uInt16& getZ(sal_uInt32 nIndex) const
        {
            assert(nIndex < mnCount && "Access out of range");
            return mpZBuffer[nIndex];
        }

        // data access read/write
        sal_uInt16& getZ(sal_uInt32 nIndex)
        {
            assert(nIndex < mnCount && "Access out of range");
            return mpZBuffer[nIndex];
        }
    };
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_RASTER_BZPIXELRASTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
