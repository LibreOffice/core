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
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/basegfxdllapi.h>

class ImplB3DPolyPolygon;

namespace basegfx
{
    class B3DPolygon;
    class B3DHomMatrix;
    class B2DHomMatrix;
}

namespace basegfx
{
    class BASEGFX_DLLPUBLIC B3DPolyPolygon
    {
    public:
        typedef o3tl::cow_wrapper< ImplB3DPolyPolygon, o3tl::ThreadSafeRefCountingPolicy > ImplType;

    private:
        ImplType                                        mpPolyPolygon;

    public:
        B3DPolyPolygon();
        B3DPolyPolygon(const B3DPolyPolygon& rPolyPolygon);
        B3DPolyPolygon(B3DPolyPolygon&& rPolygon);
        explicit B3DPolyPolygon(const B3DPolygon& rPolygon);
        ~B3DPolyPolygon();

        // assignment operator
        B3DPolyPolygon& operator=(const B3DPolyPolygon& rPolyPolygon);
        B3DPolyPolygon& operator=(B3DPolyPolygon&& rPolyPolygon);

        // compare operators
        bool operator==(const B3DPolyPolygon& rPolyPolygon) const;

        // polygon interface
        sal_uInt32 count() const;

        // B3DPolygon interface
        B3DPolygon const & getB3DPolygon(sal_uInt32 nIndex) const;
        void setB3DPolygon(sal_uInt32 nIndex, const B3DPolygon& rPolygon);

        // BColor interface
        bool areBColorsUsed() const;
        void clearBColors();

        // Normals interface
        void transformNormals(const B3DHomMatrix& rMatrix);
        bool areNormalsUsed() const;
        void clearNormals();

        // TextureCoordinate interface
        void transformTextureCoordinates(const B2DHomMatrix& rMatrix);
        bool areTextureCoordinatesUsed() const;
        void clearTextureCoordinates();

        // append single polygon
        void append(const B3DPolygon& rPolygon, sal_uInt32 nCount = 1);

        // append multiple polygons
        void append(const B3DPolyPolygon& rPolyPolygon);

        // remove
        void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

        // reset to empty state
        void clear();

        // flip polygon direction
        void flip();

        // test if tools::PolyPolygon has double points
        bool hasDoublePoints() const;

        // remove double points, at the begin/end and follow-ups, too
        void removeDoublePoints();

        // apply transformation given in matrix form to the polygon
        void transform(const basegfx::B3DHomMatrix& rMatrix);

        // polygon iterators (same iterator validity conditions as for vector)
        const B3DPolygon* begin() const;
        const B3DPolygon* end() const;
        B3DPolygon* begin();
        B3DPolygon* end();
    };
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
