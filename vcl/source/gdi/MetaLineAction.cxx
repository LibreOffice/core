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

#include <tools/vcompat.hxx>

#include <vcl/outdev.hxx>
#include <vcl/MetaLineAction.hxx>

#include <TypeSerializer.hxx>

MetaLineAction::MetaLineAction() :
    MetaAction(MetaActionType::LINE)
{}

MetaLineAction::~MetaLineAction()
{}

MetaLineAction::MetaLineAction( const Point& rStart, const Point& rEnd ) :
    MetaAction  ( MetaActionType::LINE ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{}

MetaLineAction::MetaLineAction( const Point& rStart, const Point& rEnd,
                                const LineInfo& rLineInfo ) :
    MetaAction  ( MetaActionType::LINE ),
    maLineInfo  ( rLineInfo ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{}

void MetaLineAction::Execute( OutputDevice* pOut )
{
    if( maLineInfo.IsDefault() )
        pOut->DrawLine( maStartPt, maEndPt );
    else
        pOut->DrawLine( maStartPt, maEndPt, maLineInfo );
}

rtl::Reference<MetaAction> MetaLineAction::Clone()
{
    return new MetaLineAction( *this );
}

void MetaLineAction::Move( long nHorzMove, long nVertMove )
{
    maStartPt.Move( nHorzMove, nVertMove );
    maEndPt.Move( nHorzMove, nVertMove );
}

void MetaLineAction::Scale( double fScaleX, double fScaleY )
{
    maStartPt.Scale( fScaleX, fScaleY );
    maEndPt.Scale( fScaleX, fScaleY );
    maLineInfo.Scale( fScaleX, fScaleY );
}

void MetaLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);

    // Version 1
    TypeSerializer aSerializer(rOStm);
    aSerializer.writePoint(maStartPt);
    aSerializer.writePoint(maEndPt);
    // Version 2
    WriteLineInfo( rOStm, maLineInfo );
}

void MetaLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);

    // Version 1
    TypeSerializer aSerializer(rIStm);
    aSerializer.readPoint(maStartPt);
    aSerializer.readPoint(maEndPt);

    // Version 2
    if( aCompat.GetVersion() >= 2 )
    {
        ReadLineInfo( rIStm, maLineInfo );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
