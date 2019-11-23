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

#include <vcl/gdimtf.hxx>
#include <vcl/outdev.hxx>
#include <vcl/MetaGradientExAction.hxx>

#include <TypeSerializer.hxx>

MetaGradientExAction::MetaGradientExAction() :
    MetaAction  ( MetaActionType::GRADIENTEX )
{}

MetaGradientExAction::MetaGradientExAction( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient ) :
    MetaAction  ( MetaActionType::GRADIENTEX ),
    maPolyPoly  ( rPolyPoly ),
    maGradient  ( rGradient )
{}

MetaGradientExAction::~MetaGradientExAction()
{}

void MetaGradientExAction::Execute( OutputDevice* pOut )
{
    if( pOut->GetConnectMetaFile() )
    {
        pOut->GetConnectMetaFile()->AddAction( this );
    }
}

rtl::Reference<MetaAction> MetaGradientExAction::Clone()
{
    return new MetaGradientExAction( *this );
}

void MetaGradientExAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaGradientExAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

void MetaGradientExAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    // #i105373# see comment at MetaTransparentAction::Write
    tools::PolyPolygon aNoCurvePolyPolygon;
    maPolyPoly.AdaptiveSubdivide(aNoCurvePolyPolygon);

    WritePolyPolygon( rOStm, aNoCurvePolyPolygon );
    TypeSerializer aSerializer(rOStm);
    aSerializer.writeGradient(maGradient);
}

void MetaGradientExAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadPolyPolygon( rIStm, maPolyPoly );
    TypeSerializer aSerializer(rIStm);
    aSerializer.readGradient(maGradient);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
