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

#include <tools/debug.hxx>
#include <tools/line.hxx>
#include <tools/poly.hxx>

#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/hatch.hxx>
#include <vcl/virdev.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>

#include "salframe.hxx"
#include "salgdi.hxx"
#include "svdata.hxx"
#include "outdata.hxx"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

void OutputDevice::ImplDrawPolygon( const Polygon& rPoly, const PolyPolygon* pClipPolyPoly )
{
    if( pClipPolyPoly )
        ImplDrawPolyPolygon( rPoly, pClipPolyPoly );
    else
    {
        sal_uInt16 nPoints = rPoly.GetSize();

        if ( nPoints < 2 )
            return;

        const SalPoint* pPtAry = (const SalPoint*)rPoly.GetConstPointAry();
        mpGraphics->DrawPolygon( nPoints, pPtAry, this );
    }
}

void OutputDevice::ImplDrawPolyPolygon( const PolyPolygon& rPolyPoly, const PolyPolygon* pClipPolyPoly )
{
    PolyPolygon* pPolyPoly;

    if( pClipPolyPoly )
    {
        pPolyPoly = new PolyPolygon;
        rPolyPoly.GetIntersection( *pClipPolyPoly, *pPolyPoly );
    }
    else
        pPolyPoly = (PolyPolygon*) &rPolyPoly;

    if( pPolyPoly->Count() == 1 )
    {
        const Polygon   rPoly = pPolyPoly->GetObject( 0 );
        sal_uInt16          nSize = rPoly.GetSize();

        if( nSize >= 2 )
        {
            const SalPoint* pPtAry = (const SalPoint*)rPoly.GetConstPointAry();
            mpGraphics->DrawPolygon( nSize, pPtAry, this );
        }
    }
    else if( pPolyPoly->Count() )
    {
        sal_uInt16              nCount = pPolyPoly->Count();
        boost::scoped_array<sal_uInt32> pPointAry(new sal_uInt32[nCount]);
        boost::scoped_array<PCONSTSALPOINT> pPointAryAry(new PCONSTSALPOINT[nCount]);
        sal_uInt16              i = 0;
        do
        {
            const Polygon&  rPoly = pPolyPoly->GetObject( i );
            sal_uInt16          nSize = rPoly.GetSize();
            if ( nSize )
            {
                pPointAry[i]    = nSize;
                pPointAryAry[i] = (PCONSTSALPOINT)rPoly.GetConstPointAry();
                i++;
            }
            else
                nCount--;
        }
        while( i < nCount );

        if( nCount == 1 )
            mpGraphics->DrawPolygon( pPointAry[0], pPointAryAry[0], this );
        else
            mpGraphics->DrawPolyPolygon( nCount, pPointAry.get(), pPointAryAry.get(), this );
    }

    if( pClipPolyPoly )
        delete pPolyPoly;
}
