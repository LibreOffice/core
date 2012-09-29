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

#ifndef _BGFX_RASTER_BZPIXELRASTER_HXX
#define _BGFX_RASTER_BZPIXELRASTER_HXX

#include <basegfx/raster/bpixelraster.hxx>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BZPixelRaster : public BPixelRaster
    {
    protected:
        // additionally, host a ZBuffer
        sal_uInt16*                 mpZBuffer;

    public:
        // reset
        void resetZ()
        {
            reset();
            memset(mpZBuffer, 0, sizeof(sal_uInt16) * mnCount);
        }

        // constructor/destructor
        BZPixelRaster(sal_uInt32 nWidth, sal_uInt32 nHeight)
        :   BPixelRaster(nWidth, nHeight),
            mpZBuffer(new sal_uInt16[mnCount])
        {
            memset(mpZBuffer, 0, sizeof(sal_uInt16) * mnCount);
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
                OSL_FAIL("getZ: Access out of range (!)");
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
                OSL_FAIL("getZ: Access out of range (!)");
                return mpZBuffer[0L];
            }
#endif
            return mpZBuffer[nIndex];
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RASTER_BZPIXELRASTER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
