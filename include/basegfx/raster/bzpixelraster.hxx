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

#include <basegfx/raster/bpixelraster.hxx>
#include <basegfx/basegfxdllapi.h>
#include <osl/diagnose.h>

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

#endif // INCLUDED_BASEGFX_RASTER_BZPIXELRASTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
