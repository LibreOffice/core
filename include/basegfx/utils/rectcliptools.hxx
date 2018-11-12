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

#ifndef INCLUDED_BASEGFX_UTILS_RECTCLIPTOOLS_HXX
#define INCLUDED_BASEGFX_UTILS_RECTCLIPTOOLS_HXX

#include <sal/types.h>
#include <basegfx/range/b2ibox.hxx>


namespace basegfx
{
    namespace utils
    {
        namespace RectClipFlags
        {
            static const sal_uInt32 LEFT   = sal_Int32(0x01);
            static const sal_uInt32 RIGHT  = sal_Int32(0x02);
            static const sal_uInt32 TOP    = sal_Int32(0x04);
            static const sal_uInt32 BOTTOM = sal_Int32(0x08);
        }

        /** Calc clip mask for Cohen-Sutherland rectangle clip

            This function returns a clip mask used for the
            Cohen-Sutherland rectangle clip method, where one or more
            of the lower four bits are set, if the given point is
            outside one or more of the four half planes defining the
            rectangle (see RectClipFlags for possible values)
         */
        template< class Point, class Rect > inline
           sal_uInt32 getCohenSutherlandClipFlags( const Point& rP,
                                                   const Rect&  rR )
        {
            // maxY | minY | maxX | minX
            sal_uInt32 clip;
            clip = (rP.getX() < rR.getMinX()) << 0;
            clip |= (rP.getX() > rR.getMaxX()) << 1;
            clip |= (rP.getY() < rR.getMinY()) << 2;
            clip |= (rP.getY() > rR.getMaxY()) << 3;
            return clip;
        }

        /// Cohen-Sutherland mask calculation - overload for boxes.
        template< class Point > inline
           sal_uInt32 getCohenSutherlandClipFlags( const Point&  rP,
                                                   const B2IBox& rB )
        {
            // maxY | minY | maxX | minX
            sal_uInt32 clip;
            clip = (rP.getX() <  rB.getMinX()) << 0;
            clip |= (rP.getX() >= rB.getMaxX()) << 1;
            clip |= (rP.getY() <  rB.getMinY()) << 2;
            clip |= (rP.getY() >= rB.getMaxY()) << 3;
            return clip;
        }

    }
}

#endif // INCLUDED_BASEGFX_UTILS_RECTCLIPTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
