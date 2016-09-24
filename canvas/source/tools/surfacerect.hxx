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

#ifndef INCLUDED_CANVAS_SOURCE_TOOLS_SURFACERECT_HXX
#define INCLUDED_CANVAS_SOURCE_TOOLS_SURFACERECT_HXX

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/vector/b2isize.hxx>

namespace canvas
{
    struct SurfaceRect
    {
        ::basegfx::B2IPoint maPos;
        ::basegfx::B2ISize  maSize;

        explicit SurfaceRect( const ::basegfx::B2ISize &rSize ) :
            maPos(),
            maSize(rSize)
        {
        }

        // coordinates contained in this rectangle are
        // constrained to the following rules:
        // 1) p.x >= pos.x
        // 2) p.x <= pos.x+size.x
        // 3) p.y >= pos.y
        // 4) p.y <= pos.y+size.y
        // in other words, 'size' means the number of pixels
        // this rectangle encloses plus one. for example with pos[0,0]
        // and size[512,512], p[512,512] would return inside.
        // a size of [0,0] therefore denotes a one-by-one rectangle.
        bool pointInside( sal_Int32 px, sal_Int32 py ) const
        {
            const sal_Int32 x1(maPos.getX());
            const sal_Int32 y1(maPos.getY());
            const sal_Int32 x2(maPos.getX()+maSize.getX());
            const sal_Int32 y2(maPos.getY()+maSize.getY());
            if(px  < x1) return false;
            if(px >= x2) return false;
            if(py  < y1) return false;
            if(py >= y2) return false;
            return true;
        }

        /// returns true if the horizontal line intersects the rect.
        bool hLineIntersect( sal_Int32 lx1, sal_Int32 lx2, sal_Int32 ly ) const
        {
            const sal_Int32 x1(maPos.getX());
            const sal_Int32 y1(maPos.getY());
            const sal_Int32 x2(maPos.getX()+maSize.getX());
            const sal_Int32 y2(maPos.getY()+maSize.getY());
            if(ly < y1) return false;
            if(ly >= y2) return false;
            if((lx1 < x1) && (lx2 < x1)) return false;
            if((lx1 >= x2) && (lx2 >= x2)) return false;
            return true;
        }

        /// returns true if the vertical line intersects the rect.
        bool vLineIntersect( sal_Int32 lx, sal_Int32 ly1, sal_Int32 ly2 ) const
        {
            const sal_Int32 x1(maPos.getX());
            const sal_Int32 y1(maPos.getY());
            const sal_Int32 x2(maPos.getX()+maSize.getX());
            const sal_Int32 y2(maPos.getY()+maSize.getY());
            if(lx < x1) return false;
            if(lx >= x2) return false;
            if((ly1 < y1) && (ly2 < y1)) return false;
            if((ly1 >= y2) && (ly2 >= y2)) return false;
            return true;
        }

        /// returns true if the passed rect intersects this one.
        bool intersection( const SurfaceRect& r ) const
        {
            const sal_Int32 x1(maPos.getX());
            const sal_Int32 y1(maPos.getY());
            const sal_Int32 x2(maPos.getX()+maSize.getX());
            const sal_Int32 y2(maPos.getY()+maSize.getY());
            if(r.hLineIntersect(x1,x2,y1)) return true;
            if(r.hLineIntersect(x1,x2,y2)) return true;
            if(r.vLineIntersect(x1,y1,y2)) return true;
            if(r.vLineIntersect(x2,y1,y2)) return true;
            return false;
        }

        bool inside( const SurfaceRect& r ) const
        {
            const sal_Int32 x1(maPos.getX());
            const sal_Int32 y1(maPos.getY());
            const sal_Int32 x2(maPos.getX()+maSize.getX());
            const sal_Int32 y2(maPos.getY()+maSize.getY());
            if(!(r.pointInside(x1,y1))) return false;
            if(!(r.pointInside(x2,y1))) return false;
            if(!(r.pointInside(x2,y2))) return false;
            if(!(r.pointInside(x1,y2))) return false;
            return true;
        }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
