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

#include <sal/log.hxx>
#include <tools/vcompat.hxx>
#include <tools/stream.hxx>

#include <vcl/outdev.hxx>
#include <vcl/MetaPolyPolygonAction.hxx>

MetaPolyPolygonAction::MetaPolyPolygonAction() :
    MetaAction(MetaActionType::POLYPOLYGON)
{}

MetaPolyPolygonAction::~MetaPolyPolygonAction()
{}

MetaPolyPolygonAction::MetaPolyPolygonAction( const tools::PolyPolygon& rPolyPoly ) :
    MetaAction  ( MetaActionType::POLYPOLYGON ),
    maPolyPoly  ( rPolyPoly )
{}

void MetaPolyPolygonAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPolyPolygon( maPolyPoly );
}

rtl::Reference<MetaAction> MetaPolyPolygonAction::Clone()
{
    return new MetaPolyPolygonAction( *this );
}

void MetaPolyPolygonAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaPolyPolygonAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

void MetaPolyPolygonAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);

    sal_uInt16 nNumberOfComplexPolygons = 0;
    sal_uInt16 i, nPolyCount = maPolyPoly.Count();

    tools::Polygon aSimplePoly;                                // Version 1
    rOStm.WriteUInt16( nPolyCount );
    for ( i = 0; i < nPolyCount; i++ )
    {
        const tools::Polygon& rPoly = maPolyPoly.GetObject( i );
        if ( rPoly.HasFlags() )
            nNumberOfComplexPolygons++;
        rPoly.AdaptiveSubdivide( aSimplePoly );
        WritePolygon( rOStm, aSimplePoly );
    }

    rOStm.WriteUInt16( nNumberOfComplexPolygons );                  // Version 2
    for ( i = 0; nNumberOfComplexPolygons && ( i < nPolyCount ); i++ )
    {
        const tools::Polygon& rPoly = maPolyPoly.GetObject( i );
        if ( rPoly.HasFlags() )
        {
            rOStm.WriteUInt16( i );
            rPoly.Write( rOStm );

            nNumberOfComplexPolygons--;
        }
    }
}

void MetaPolyPolygonAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadPolyPolygon( rIStm, maPolyPoly );                // Version 1

    if ( aCompat.GetVersion() >= 2 )    // Version 2
    {
        sal_uInt16 nNumberOfComplexPolygons(0);
        rIStm.ReadUInt16( nNumberOfComplexPolygons );
        const size_t nMinRecordSize = sizeof(sal_uInt16);
        const size_t nMaxRecords = rIStm.remainingSize() / nMinRecordSize;
        if (nNumberOfComplexPolygons > nMaxRecords)
        {
            SAL_WARN("vcl.gdi", "Parsing error: " << nMaxRecords <<
                     " max possible entries, but " << nNumberOfComplexPolygons << " claimed, truncating");
            nNumberOfComplexPolygons = nMaxRecords;
        }
        for (sal_uInt16 i = 0; i < nNumberOfComplexPolygons; ++i)
        {
            sal_uInt16 nIndex(0);
            rIStm.ReadUInt16( nIndex );
            tools::Polygon aPoly;
            aPoly.Read( rIStm );
            if (nIndex >= maPolyPoly.Count())
            {
                SAL_WARN("vcl.gdi", "svm contains polygon index " << nIndex
                         << " outside possible range " << maPolyPoly.Count());
                continue;
            }
            maPolyPoly.Replace( aPoly, nIndex );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
