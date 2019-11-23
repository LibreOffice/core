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
#include <vcl/MetaTextRectAction.hxx>

#include <TypeSerializer.hxx>

MetaTextRectAction::MetaTextRectAction() :
    MetaAction  ( MetaActionType::TEXTRECT ),
    mnStyle     ( DrawTextFlags::NONE )
{}

MetaTextRectAction::~MetaTextRectAction()
{}

MetaTextRectAction::MetaTextRectAction( const tools::Rectangle& rRect,
                                        const OUString& rStr, DrawTextFlags nStyle ) :
    MetaAction  ( MetaActionType::TEXTRECT ),
    maRect      ( rRect ),
    maStr       ( rStr ),
    mnStyle     ( nStyle )
{}

void MetaTextRectAction::Execute( OutputDevice* pOut )
{
    pOut->DrawText( maRect, maStr, mnStyle );
}

rtl::Reference<MetaAction> MetaTextRectAction::Clone()
{
    return new MetaTextRectAction( *this );
}

void MetaTextRectAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaTextRectAction::Scale( double fScaleX, double fScaleY )
{
    maRect.Scale( fScaleX, fScaleY );
}

void MetaTextRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);
    TypeSerializer aSerializer(rOStm);
    aSerializer.writeRectangle(maRect);
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm.WriteUInt16( static_cast<sal_uInt16>(mnStyle) );

    write_uInt16_lenPrefixed_uInt16s_FromOUString(rOStm, maStr); // version 2
}

void MetaTextRectAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    TypeSerializer aSerializer(rIStm);
    aSerializer.readRectangle(maRect);
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    sal_uInt16 nTmp;
    rIStm  .ReadUInt16( nTmp );
    mnStyle = static_cast<DrawTextFlags>(nTmp);

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
        maStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(rIStm);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
