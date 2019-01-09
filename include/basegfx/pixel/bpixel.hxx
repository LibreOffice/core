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

#ifndef INCLUDED_BASEGFX_PIXEL_BPIXEL_HXX
#define INCLUDED_BASEGFX_PIXEL_BPIXEL_HXX

#include <sal/types.h>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class BPixel final
    {
        union
        {
            struct
            {
                unsigned                                mnR : 8;        // red intensity
                unsigned                                mnG : 8;        // green intensity
                unsigned                                mnB : 8;        // blue intensity
                unsigned                                mnO : 8;        // opacity, 0 == full transparence
            } maRGBO;

            struct
            {
                unsigned                                mnValue : 32;   // all values
            } maCombinedRGBO;
        } maPixelUnion;

    public:
        BPixel()
        {
            maPixelUnion.maCombinedRGBO.mnValue = 0;
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

        // data access read
        sal_uInt8 getRed() const { return maPixelUnion.maRGBO.mnR; }
        sal_uInt8 getGreen() const { return maPixelUnion.maRGBO.mnG; }
        sal_uInt8 getBlue() const { return maPixelUnion.maRGBO.mnB; }
        sal_uInt8 getOpacity() const { return maPixelUnion.maRGBO.mnO; }

        // data access write
        void setRed(sal_uInt8 nNew) { maPixelUnion.maRGBO.mnR = nNew; }
        void setGreen(sal_uInt8 nNew) { maPixelUnion.maRGBO.mnG = nNew; }
        void setBlue(sal_uInt8 nNew) { maPixelUnion.maRGBO.mnB = nNew; }
        void setOpacity(sal_uInt8 nNew) { maPixelUnion.maRGBO.mnO = nNew; }

        // comparators
        bool operator==( const BPixel& rPixel ) const
        {
            return (rPixel.maPixelUnion.maCombinedRGBO.mnValue == maPixelUnion.maCombinedRGBO.mnValue);
        }

        bool operator!=( const BPixel& rPixel ) const
        {
            return (rPixel.maPixelUnion.maCombinedRGBO.mnValue != maPixelUnion.maCombinedRGBO.mnValue);
        }
    };


} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_PIXEL_BPIXEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
