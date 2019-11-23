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

#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <vcl/outdev.hxx>
#include <vcl/MetaPolyLineAction.hxx>

MetaPolyLineAction::MetaPolyLineAction() :
    MetaAction(MetaActionType::POLYLINE)
{}

MetaPolyLineAction::~MetaPolyLineAction()
{}

MetaPolyLineAction::MetaPolyLineAction( const tools::Polygon& rPoly ) :
    MetaAction  ( MetaActionType::POLYLINE ),
    maPoly      ( rPoly )
{}

MetaPolyLineAction::MetaPolyLineAction( const tools::Polygon& rPoly, const LineInfo& rLineInfo ) :
    MetaAction  ( MetaActionType::POLYLINE ),
    maLineInfo  ( rLineInfo ),
    maPoly      ( rPoly )
{}

void MetaPolyLineAction::Execute( OutputDevice* pOut )
{
    if( maLineInfo.IsDefault() )
        pOut->DrawPolyLine( maPoly );
    else
        pOut->DrawPolyLine( maPoly, maLineInfo );
}

rtl::Reference<MetaAction> MetaPolyLineAction::Clone()
{
    return new MetaPolyLineAction( *this );
}

void MetaPolyLineAction::Move( long nHorzMove, long nVertMove )
{
    maPoly.Move( nHorzMove, nVertMove );
}

void MetaPolyLineAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoly( maPoly, fScaleX, fScaleY );
    maLineInfo.Scale( fScaleX, fScaleY );
}

void MetaPolyLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 3);

    tools::Polygon aSimplePoly;
    maPoly.AdaptiveSubdivide( aSimplePoly );

    WritePolygon( rOStm, aSimplePoly );                               // Version 1
    WriteLineInfo( rOStm, maLineInfo );                                // Version 2

    bool bHasPolyFlags = maPoly.HasFlags();        // Version 3
    rOStm.WriteBool( bHasPolyFlags );
    if ( bHasPolyFlags )
        maPoly.Write( rOStm );
}

void MetaPolyLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);

    // Version 1
    ReadPolygon( rIStm, maPoly );

    // Version 2
    if( aCompat.GetVersion() >= 2 )
        ReadLineInfo( rIStm, maLineInfo );
    if ( aCompat.GetVersion() >= 3 )
    {
        sal_uInt8 bHasPolyFlags(0);
        rIStm.ReadUChar( bHasPolyFlags );
        if ( bHasPolyFlags )
            maPoly.Read( rIStm );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
