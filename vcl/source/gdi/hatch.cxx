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
#include <tools/debug.hxx>
#include <vcl/hatch.hxx>


ImplHatch::ImplHatch() :
    mnRefCount  ( 1 ),
    maColor     ( COL_BLACK ),
    meStyle     ( HATCH_SINGLE ),
    mnDistance  ( 1 ),
    mnAngle     ( 0 )
{
}

ImplHatch::ImplHatch( const ImplHatch& rImplHatch ) :
    mnRefCount  ( 1 ),
    maColor     ( rImplHatch.maColor ),
    meStyle     ( rImplHatch.meStyle ),
    mnDistance  ( rImplHatch.mnDistance ),
    mnAngle     ( rImplHatch.mnAngle )
{
}

Hatch::Hatch()
{
    mpImplHatch = new ImplHatch;
}

Hatch::Hatch( const Hatch& rHatch )
{
    mpImplHatch = rHatch.mpImplHatch;
    mpImplHatch->mnRefCount++;
}

Hatch::Hatch( HatchStyle eStyle, const Color& rColor,
              long nDistance, sal_uInt16 nAngle10 )
{
    mpImplHatch = new ImplHatch;
    mpImplHatch->maColor = rColor;
    mpImplHatch->meStyle = eStyle;
    mpImplHatch->mnDistance = nDistance;
    mpImplHatch->mnAngle = nAngle10;
}

Hatch::~Hatch()
{
    if( !( --mpImplHatch->mnRefCount ) )
        delete mpImplHatch;
}

Hatch& Hatch::operator=( const Hatch& rHatch )
{

    rHatch.mpImplHatch->mnRefCount++;

    if( !( --mpImplHatch->mnRefCount ) )
        delete mpImplHatch;

    mpImplHatch = rHatch.mpImplHatch;
    return *this;
}

bool Hatch::operator==( const Hatch& rHatch ) const
{

    return( mpImplHatch == rHatch.mpImplHatch ||
            ( mpImplHatch->maColor == rHatch.mpImplHatch->maColor &&
              mpImplHatch->meStyle == rHatch.mpImplHatch->meStyle &&
              mpImplHatch->mnDistance == rHatch.mpImplHatch->mnDistance &&
              mpImplHatch->mnAngle == rHatch.mpImplHatch->mnAngle ) );
}

void Hatch::ImplMakeUnique()
{
    if( mpImplHatch->mnRefCount != 1 )
    {
        if( mpImplHatch->mnRefCount )
            mpImplHatch->mnRefCount--;

        mpImplHatch = new ImplHatch( *mpImplHatch );
    }
}

void Hatch::SetColor( const Color& rColor )
{
    ImplMakeUnique();
    mpImplHatch->maColor = rColor;
}

void Hatch::SetDistance( long nDistance )
{
    ImplMakeUnique();
    mpImplHatch->mnDistance = nDistance;
}

void Hatch::SetAngle( sal_uInt16 nAngle10 )
{
    ImplMakeUnique();
    mpImplHatch->mnAngle = nAngle10;
}

SvStream& ReadImplHatch( SvStream& rIStm, ImplHatch& rImplHatch )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    sal_uInt16          nTmp16;
    sal_Int32       nTmp32(0);

    rIStm.ReadUInt16( nTmp16 ); rImplHatch.meStyle = (HatchStyle) nTmp16;
    //#fdo39428 SvStream no longer supports operator>>(long&)
    ReadColor( rIStm, rImplHatch.maColor ).ReadInt32( nTmp32 ).ReadUInt16( rImplHatch.mnAngle );
    rImplHatch.mnDistance = nTmp32;

    return rIStm;
}

SvStream& WriteImplHatch( SvStream& rOStm, const ImplHatch& rImplHatch )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rOStm.WriteUInt16( (sal_uInt16) rImplHatch.meStyle );
    WriteColor( rOStm, rImplHatch.maColor );
    //#fdo39428 SvStream no longer supports operator<<(long)
    rOStm.WriteInt32( sal::static_int_cast<sal_Int32>(rImplHatch.mnDistance) ).WriteUInt16( rImplHatch.mnAngle );

    return rOStm;
}

SvStream& ReadHatch( SvStream& rIStm, Hatch& rHatch )
{
    rHatch.ImplMakeUnique();
    return ReadImplHatch( rIStm, *rHatch.mpImplHatch );
}

SvStream& WriteHatch( SvStream& rOStm, const Hatch& rHatch )
{
    return WriteImplHatch( rOStm, *rHatch.mpImplHatch );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
