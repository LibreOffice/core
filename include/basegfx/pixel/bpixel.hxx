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

#pragma once

#include <sal/types.h>
#include <basegfx/color/bcolor.hxx>

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
                unsigned                                mnA : 8;        // opacity, 0 == full transparence
            } maRGBA;

            struct
            {
                unsigned                                mnValue : 32;   // all values
            } maCombinedRGBA;
        } maPixelUnion;

    public:
        BPixel()
        {
            maPixelUnion.maCombinedRGBA.mnValue = 0;
        }

        // use explicit here to make sure everyone knows what he is doing. Values range from
        // 0..255 integer here.
        explicit BPixel(sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue, sal_uInt8 nAlpha)
        {
            maPixelUnion.maRGBA.mnR = nRed;
            maPixelUnion.maRGBA.mnG = nGreen;
            maPixelUnion.maRGBA.mnB = nBlue;
            maPixelUnion.maRGBA.mnA = nAlpha;
        }

        // constructor from BColor which uses double precision color, so change it
        // to local integer format. It will also be clamped here.
        BPixel(const BColor& rColor, sal_uInt8 nAlpha)
        {
            maPixelUnion.maRGBA.mnR = sal_uInt8((rColor.getRed() * 255.0) + 0.5);
            maPixelUnion.maRGBA.mnG = sal_uInt8((rColor.getGreen() * 255.0) + 0.5);
            maPixelUnion.maRGBA.mnB = sal_uInt8((rColor.getBlue() * 255.0) + 0.5);
            maPixelUnion.maRGBA.mnA = nAlpha;
        }

        // data access read
        sal_uInt8 getRed() const { return maPixelUnion.maRGBA.mnR; }
        sal_uInt8 getGreen() const { return maPixelUnion.maRGBA.mnG; }
        sal_uInt8 getBlue() const { return maPixelUnion.maRGBA.mnB; }
        sal_uInt8 getAlpha() const { return maPixelUnion.maRGBA.mnA; }

        // data access write
        void setRed(sal_uInt8 nNew) { maPixelUnion.maRGBA.mnR = nNew; }
        void setGreen(sal_uInt8 nNew) { maPixelUnion.maRGBA.mnG = nNew; }
        void setBlue(sal_uInt8 nNew) { maPixelUnion.maRGBA.mnB = nNew; }
        void setAlpha(sal_uInt8 nNew) { maPixelUnion.maRGBA.mnA = nNew; }

        // comparators
        bool operator==( const BPixel& rPixel ) const
        {
            return (rPixel.maPixelUnion.maCombinedRGBA.mnValue == maPixelUnion.maCombinedRGBA.mnValue);
        }

        bool operator!=( const BPixel& rPixel ) const
        {
            return (rPixel.maPixelUnion.maCombinedRGBA.mnValue != maPixelUnion.maCombinedRGBA.mnValue);
        }
    };


} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
