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

#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <vcl/outdev.hxx>
#include <vcl/MetaTextLineAction.hxx>

#include <TypeSerializer.hxx>

MetaTextLineAction::MetaTextLineAction() :
    MetaAction  ( MetaActionType::TEXTLINE ),
    mnWidth     ( 0 ),
    meStrikeout ( STRIKEOUT_NONE ),
    meUnderline ( LINESTYLE_NONE ),
    meOverline  ( LINESTYLE_NONE )
{}

MetaTextLineAction::~MetaTextLineAction()
{}

MetaTextLineAction::MetaTextLineAction( const Point& rPos, long nWidth,
                                        FontStrikeout eStrikeout,
                                        FontLineStyle eUnderline,
                                        FontLineStyle eOverline ) :
    MetaAction  ( MetaActionType::TEXTLINE ),
    maPos       ( rPos ),
    mnWidth     ( nWidth ),
    meStrikeout ( eStrikeout ),
    meUnderline ( eUnderline ),
    meOverline  ( eOverline )
{}

void MetaTextLineAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTextLine( maPos, mnWidth, meStrikeout, meUnderline, meOverline );
}

rtl::Reference<MetaAction> MetaTextLineAction::Clone()
{
    return new MetaTextLineAction( *this );
}

void MetaTextLineAction::Move( long nHorzMove, long nVertMove )
{
    maPos.Move( nHorzMove, nVertMove );
}

void MetaTextLineAction::Scale( double fScaleX, double fScaleY )
{
    maPos.Scale( fScaleX, fScaleY );
    mnWidth = FRound( mnWidth * fabs(fScaleX) );
}

void MetaTextLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);

    TypeSerializer aSerializer(rOStm);
    aSerializer.writePoint(maPos);

    rOStm.WriteInt32( mnWidth );
    rOStm.WriteUInt32( meStrikeout );
    rOStm.WriteUInt32( meUnderline );
    // new in version 2
    rOStm.WriteUInt32( meOverline );
}

void MetaTextLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);

    sal_Int32 nTempWidth(0);
    TypeSerializer aSerializer(rIStm);
    aSerializer.readPoint(maPos);
    rIStm.ReadInt32(nTempWidth);
    mnWidth = nTempWidth;

    sal_uInt32 nTempStrikeout(0);
    rIStm.ReadUInt32( nTempStrikeout );
    meStrikeout = static_cast<FontStrikeout>(nTempStrikeout);

    sal_uInt32 nTempUnderline(0);
    rIStm.ReadUInt32( nTempUnderline );
    meUnderline = static_cast<FontLineStyle>(nTempUnderline);

    if (aCompat.GetVersion() >= 2)
    {
        sal_uInt32 nTempOverline(0);
        rIStm.ReadUInt32(nTempOverline);
        meOverline = static_cast<FontLineStyle>(nTempOverline);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
