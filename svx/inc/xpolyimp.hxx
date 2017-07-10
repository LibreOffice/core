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

#ifndef INCLUDED_SVX_INC_XPOLYIMP_HXX
#define INCLUDED_SVX_INC_XPOLYIMP_HXX

#include <svx/xpoly.hxx>
#include <memory>
#include <vector>

class Point;

class ImpXPolygon
{
public:
    Point*              pPointAry;
    std::unique_ptr<PolyFlags[]>
                        pFlagAry;
    Point*              pOldPointAry;
    bool                bDeleteOldPoints;
    sal_uInt16          nSize;
    sal_uInt16          nResize;
    sal_uInt16          nPoints;

    ImpXPolygon( sal_uInt16 nInitSize, sal_uInt16 nResize=16 );
    ImpXPolygon( const ImpXPolygon& rImpXPoly );
    ~ImpXPolygon();

    bool operator==(const ImpXPolygon& rImpXPoly) const;

    void CheckPointDelete() const;

    void Resize( sal_uInt16 nNewSize, bool bDeletePoints = true );
    void InsertSpace( sal_uInt16 nPos, sal_uInt16 nCount );
    void Remove( sal_uInt16 nPos, sal_uInt16 nCount );
};

typedef ::std::vector< XPolygon* > XPolygonList;

class ImpXPolyPolygon
{
public:
    XPolygonList aXPolyList;

                ImpXPolyPolygon() {}
                ImpXPolyPolygon( const ImpXPolyPolygon& rImpXPolyPoly );
                ~ImpXPolyPolygon();
};


#endif // INCLUDED_SVX_INC_XPOLYIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
