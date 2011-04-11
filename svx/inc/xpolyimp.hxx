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

#ifndef _XPOLYIMP_HXX
#define _XPOLYIMP_HXX

#include <tools/gen.hxx>
#include <svx/xpoly.hxx>
#include <vector>

class ImpXPolygon
{
public:
    Point*          pPointAry;
    sal_uInt8*           pFlagAry;
    Point*          pOldPointAry;
    sal_Bool            bDeleteOldPoints;
    sal_uInt16          nSize;
    sal_uInt16          nResize;
    sal_uInt16          nPoints;
    sal_uInt16          nRefCount;

    ImpXPolygon( sal_uInt16 nInitSize = 16, sal_uInt16 nResize=16 );
    ImpXPolygon( const ImpXPolygon& rImpXPoly );
    ~ImpXPolygon();

    bool operator==(const ImpXPolygon& rImpXPoly) const;
    bool operator!=(const ImpXPolygon& rImpXPoly) const { return !operator==(rImpXPoly); }

    void CheckPointDelete()
    {
        if ( bDeleteOldPoints )
        {
            delete[] (char*)pOldPointAry;
            bDeleteOldPoints = sal_False;
        }
    }

    void Resize( sal_uInt16 nNewSize, sal_Bool bDeletePoints = sal_True );
    void InsertSpace( sal_uInt16 nPos, sal_uInt16 nCount );
    void Remove( sal_uInt16 nPos, sal_uInt16 nCount );
};

typedef ::std::vector< XPolygon* > XPolygonList;

class ImpXPolyPolygon
{
public:
    XPolygonList aXPolyList;
    sal_uInt16       nRefCount;

                ImpXPolyPolygon() { nRefCount = 1; }
                ImpXPolyPolygon( const ImpXPolyPolygon& rImpXPolyPoly );
                ~ImpXPolyPolygon();

    bool operator==(const ImpXPolyPolygon& rImpXPolyPoly) const;
    bool operator!=(const ImpXPolyPolygon& rImpXPolyPoly) const { return !operator==(rImpXPolyPoly); }
};



#endif      // _XPOLYIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
