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



#ifndef _BGFX_RASTER_BZPIXELRASTER_HXX
#define _BGFX_RASTER_BZPIXELRASTER_HXX

#include <basegfx/raster/bpixelraster.hxx>
#include <rtl/memory.h>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BASEGFX_DLLPUBLIC BZPixelRaster : public BPixelRaster
    {
    protected:
        // additionally, host a ZBuffer
        sal_uInt16*                 mpZBuffer;

    public:
        // reset
        void resetZ()
        {
            reset();
            rtl_zeroMemory(mpZBuffer, sizeof(sal_uInt16) * mnCount);
        }

        // constructor/destructor
        BZPixelRaster(sal_uInt32 nWidth, sal_uInt32 nHeight)
        :   BPixelRaster(nWidth, nHeight),
            mpZBuffer(new sal_uInt16[mnCount])
        {
            rtl_zeroMemory(mpZBuffer, sizeof(sal_uInt16) * mnCount);
        }

        ~BZPixelRaster()
        {
            delete [] mpZBuffer;
        }

        // data access read only
        const sal_uInt16& getZ(sal_uInt32 nIndex) const
        {
#ifdef DBG_UTIL
            if(nIndex >= mnCount)
            {
                OSL_ENSURE(false, "getZ: Access out of range (!)");
                return mpZBuffer[0L];
            }
#endif
            return mpZBuffer[nIndex];
        }

        // data access read/write
        sal_uInt16& getZ(sal_uInt32 nIndex)
        {
#ifdef DBG_UTIL
            if(nIndex >= mnCount)
            {
                OSL_ENSURE(false, "getZ: Access out of range (!)");
                return mpZBuffer[0L];
            }
#endif
            return mpZBuffer[nIndex];
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RASTER_BZPIXELRASTER_HXX */
