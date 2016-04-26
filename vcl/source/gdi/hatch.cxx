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
    maColor     ( COL_BLACK ),
    meStyle     ( HATCH_SINGLE ),
    mnDistance  ( 1 ),
    mnAngle     ( 0 )
{
}

ImplHatch::ImplHatch( const ImplHatch& rImplHatch ) :
    maColor     ( rImplHatch.maColor ),
    meStyle     ( rImplHatch.meStyle ),
    mnDistance  ( rImplHatch.mnDistance ),
    mnAngle     ( rImplHatch.mnAngle )
{
}

bool ImplHatch::operator==( const ImplHatch& rImplHatch ) const
{
    if( maColor == rImplHatch.maColor &&
        meStyle == rImplHatch.meStyle &&
        mnDistance == rImplHatch.mnDistance &&
        mnAngle == rImplHatch.mnAngle)
        return true;
    return false;
}

Hatch::Hatch() : mpImplHatch()
{
}

Hatch::Hatch( const Hatch& rHatch ) : mpImplHatch( rHatch.mpImplHatch )
{
}

Hatch::Hatch( HatchStyle eStyle, const Color& rColor,
              long nDistance, sal_uInt16 nAngle10 ) : mpImplHatch()
{
    mpImplHatch->maColor = rColor;
    mpImplHatch->meStyle = eStyle;
    mpImplHatch->mnDistance = nDistance;
    mpImplHatch->mnAngle = nAngle10;
}

Hatch::~Hatch()
{
}

Hatch& Hatch::operator=( const Hatch& rHatch )
{
    mpImplHatch = rHatch.mpImplHatch;
    return *this;
}

bool Hatch::operator==( const Hatch& rHatch ) const
{
    return mpImplHatch == rHatch.mpImplHatch;
}


void Hatch::SetColor( const Color& rColor )
{
    mpImplHatch->maColor = rColor;
}

void Hatch::SetDistance( long nDistance )
{
    mpImplHatch->mnDistance = nDistance;
}

void Hatch::SetAngle( sal_uInt16 nAngle10 )
{
    mpImplHatch->mnAngle = nAngle10;
}

SvStream& ReadHatch( SvStream& rIStm, Hatch& rHatch )
{
    VersionCompat   aCompat( rIStm, StreamMode::READ );
    sal_uInt16          nTmp16;
    sal_Int32       nTmp32(0);

    rIStm.ReadUInt16( nTmp16 ); rHatch.mpImplHatch->meStyle = (HatchStyle) nTmp16;
    ReadColor( rIStm, rHatch.mpImplHatch->maColor ).ReadInt32( nTmp32 ).ReadUInt16(
      rHatch.mpImplHatch->mnAngle );
    rHatch.mpImplHatch->mnDistance = nTmp32;

    return rIStm;
}

SvStream& WriteHatch( SvStream& rOStm, const Hatch& rHatch )
{
    VersionCompat aCompat( rOStm, StreamMode::WRITE, 1 );

    rOStm.WriteUInt16( rHatch.mpImplHatch->meStyle );
    WriteColor( rOStm, rHatch.mpImplHatch->maColor );
    rOStm.WriteInt32( rHatch.mpImplHatch->mnDistance ).WriteUInt16( rHatch.mpImplHatch->mnAngle );

    return rOStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
