/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _BGFX_RASTER_BPIXELRASTER_HXX
#define _BGFX_RASTER_BPIXELRASTER_HXX

#include <algorithm>
#include <sal/types.h>
#include <basegfx/pixel/bpixel.hxx>
#include <rtl/memory.h>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BASEGFX_DLLPUBLIC BPixelRaster
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
            rtl_zeroMemory(mpContent, sizeof(BPixel) * mnCount);
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
                OSL_ENSURE(false, "getBPixel: Access out of range (!)");
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
                OSL_ENSURE(false, "getBPixel: Access out of range (!)");
                return mpContent[0L];
            }
#endif
            return mpContent[nIndex];
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RASTER_BPIXELRASTER_HXX */
