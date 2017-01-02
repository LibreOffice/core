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
#ifndef INCLUDED_TOOLS_INC_POLY_H
#define INCLUDED_TOOLS_INC_POLY_H

#include <sal/types.h>

class Point;

class SAL_WARN_UNUSED ImplPolygonData
{
public:
    Point*          mpPointAry;
    sal_uInt8*           mpFlagAry;
    sal_uInt16          mnPoints;
    sal_uInt32      mnRefCount;
};

class SAL_WARN_UNUSED ImplPolygon  : public ImplPolygonData
{
public:
                    ImplPolygon( sal_uInt16 nInitSize, bool bFlags = false );
                    ImplPolygon( sal_uInt16 nPoints, const Point* pPtAry, const sal_uInt8* pInitFlags = NULL );
                    ImplPolygon( const ImplPolygon& rImplPoly );
                    ~ImplPolygon();

    void            ImplSetSize( sal_uInt16 nSize, bool bResize = true );
    void            ImplCreateFlagArray();
    bool            ImplSplit( sal_uInt16 nPos, sal_uInt16 nSpace, ImplPolygon* pInitPoly = NULL );
};

#define MAX_POLYGONS        ((sal_uInt16)0x3FF0)

namespace tools {
class Polygon;
}

class SAL_WARN_UNUSED ImplPolyPolygon
{
public:
    tools::Polygon** mpPolyAry;
    sal_uInt32          mnRefCount;
    sal_uInt16          mnCount;
    sal_uInt16          mnSize;
    sal_uInt16          mnResize;

                    ImplPolyPolygon( sal_uInt16 nInitSize, sal_uInt16 nResize )
                        { mpPolyAry = NULL; mnCount = 0; mnRefCount = 1;
                          mnSize = nInitSize; mnResize = nResize; }
                    ImplPolyPolygon( sal_uInt16 nInitSize );
                    ImplPolyPolygon( const ImplPolyPolygon& rImplPolyPoly );
                    ~ImplPolyPolygon();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
