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

#ifndef INCLUDED_CPPCANVAS_COLOR_HXX
#define INCLUDED_CPPCANVAS_COLOR_HXX

#include <sal/types.h>

/* Definition of Color class */

namespace cppcanvas
{
    /** Color in the sRGB color space, plus alpha channel

        The four bytes of the sal_uInt32 are allocated as follows
        to the color channels and alpha: 0xRRGGBBAA.
     */
    typedef sal_uInt32 IntSRGBA;

    inline sal_uInt8 getRed( IntSRGBA nCol )
    {
        return static_cast<sal_uInt8>( (nCol&0xFF000000U) >> 24U );
    }

    inline sal_uInt8 getGreen( IntSRGBA nCol )
    {
        return static_cast<sal_uInt8>( (nCol&0x00FF0000U) >> 16U );
    }

    inline sal_uInt8 getBlue( IntSRGBA nCol )
    {
        return static_cast<sal_uInt8>( (nCol&0x0000FF00U) >> 8U );
    }

    inline sal_uInt8 getAlpha( IntSRGBA nCol )
    {
        return static_cast<sal_uInt8>( nCol&0x000000FFU );
    }

    inline IntSRGBA makeColor( sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue, sal_uInt8 nAlpha )
    {
        return (nRed << 24U)|(nGreen << 16U)|(nBlue << 8U)|nAlpha;
    }

    inline sal_Int32 makeColorARGB( sal_uInt8 nAlpha, sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue)
    {
        return (nAlpha << 24U)|(nRed << 16U)|(nGreen << 8U)|nBlue;
    }

}

#endif // INCLUDED_CPPCANVAS_COLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
