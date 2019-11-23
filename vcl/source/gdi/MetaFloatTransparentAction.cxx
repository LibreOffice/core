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
#include <vcl/MetaFloatTransparentAction.hxx>

#include <TypeSerializer.hxx>

MetaFloatTransparentAction::MetaFloatTransparentAction() :
    MetaAction(MetaActionType::FLOATTRANSPARENT)
{}

MetaFloatTransparentAction::~MetaFloatTransparentAction()
{}

MetaFloatTransparentAction::MetaFloatTransparentAction( const GDIMetaFile& rMtf, const Point& rPos,
                                                        const Size& rSize, const Gradient& rGradient ) :
    MetaAction      ( MetaActionType::FLOATTRANSPARENT ),
    maMtf           ( rMtf ),
    maPoint         ( rPos ),
    maSize          ( rSize ),
    maGradient      ( rGradient )
{}

void MetaFloatTransparentAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTransparent( maMtf, maPoint, maSize, maGradient );
}

rtl::Reference<MetaAction> MetaFloatTransparentAction::Clone()
{
    return new MetaFloatTransparentAction( *this );
}

void MetaFloatTransparentAction::Move( long nHorzMove, long nVertMove )
{
    maPoint.Move( nHorzMove, nVertMove );
}

void MetaFloatTransparentAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maPoint, maSize);
    aRectangle.Scale( fScaleX, fScaleY );
    maPoint = aRectangle.TopLeft();
    maSize = aRectangle.GetSize();
}

void MetaFloatTransparentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    maMtf.Write( rOStm );
    TypeSerializer aSerializer(rOStm);
    aSerializer.writePoint(maPoint);
    aSerializer.writeSize(maSize);
    aSerializer.writeGradient(maGradient);
}

void MetaFloatTransparentAction::Read(SvStream& rIStm, ImplMetaReadData* pData)
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadGDIMetaFile(rIStm, maMtf, pData);
    TypeSerializer aSerializer(rIStm);
    aSerializer.readPoint(maPoint);
    aSerializer.readSize(maSize);
    aSerializer.readGradient(maGradient);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
