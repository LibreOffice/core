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

#ifndef INCLUDED_BASEGFX_POLYGON_B3DPOLYGON_HXX
#define INCLUDED_BASEGFX_POLYGON_B3DPOLYGON_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/basegfxdllapi.h>


// predeclarations
class ImplB3DPolygon;

namespace basegfx
{
    class B3DPolygon;
    class B3DPoint;
    class B3DHomMatrix;
    class B3DVector;
    class B2DPoint;
    class B2DHomMatrix;
    class BColor;
} // end of namespace basegfx



namespace basegfx
{
    class BASEGFX_DLLPUBLIC B3DPolygon
    {
    public:
        typedef o3tl::cow_wrapper< ImplB3DPolygon > ImplType;

    private:
        // internal data.
        ImplType                                    mpPolygon;

    public:
        B3DPolygon();
        B3DPolygon(const B3DPolygon& rPolygon);
        ~B3DPolygon();

        // assignment operator
        B3DPolygon& operator=(const B3DPolygon& rPolygon);

        // compare operators
        bool operator==(const B3DPolygon& rPolygon) const;
        bool operator!=(const B3DPolygon& rPolygon) const;

        // member count
        sal_uInt32 count() const;

        // Coordinate interface
        B3DPoint getB3DPoint(sal_uInt32 nIndex) const;
        void setB3DPoint(sal_uInt32 nIndex, const B3DPoint& rValue);

        // Coordinate append
        void append(const B3DPoint& rPoint, sal_uInt32 nCount = 1);

        // BColor interface
        BColor getBColor(sal_uInt32 nIndex) const;
        void setBColor(sal_uInt32 nIndex, const BColor& rValue);
        bool areBColorsUsed() const;
        void clearBColors();

        // Normals interface
        B3DVector getNormal() const; // plane normal
        B3DVector getNormal(sal_uInt32 nIndex) const; // normal in each point
        void setNormal(sal_uInt32 nIndex, const B3DVector& rValue);
        void transformNormals(const B3DHomMatrix& rMatrix);
        bool areNormalsUsed() const;
        void clearNormals();

        // TextureCoordinate interface
        B2DPoint getTextureCoordinate(sal_uInt32 nIndex) const;
        void setTextureCoordinate(sal_uInt32 nIndex, const B2DPoint& rValue);
        void transformTextureCoordiantes(const B2DHomMatrix& rMatrix);
        bool areTextureCoordinatesUsed() const;
        void clearTextureCoordinates();

        // append other 2D polygons
        void append(const B3DPolygon& rPoly, sal_uInt32 nIndex = 0, sal_uInt32 nCount = 0);

        // remove
        void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

        // clear all points
        void clear();

        // closed state
        bool isClosed() const;
        void setClosed(bool bNew);

        // flip polygon direction
        void flip();

        // test if Polygon has double points
        bool hasDoublePoints() const;

        // remove double points, at the begin/end and follow-ups, too
        void removeDoublePoints();

        // apply transformation given in matrix form to the polygon
        void transform(const B3DHomMatrix& rMatrix);
    };
} // end of namespace basegfx




#endif // INCLUDED_BASEGFX_POLYGON_B3DPOLYGON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
