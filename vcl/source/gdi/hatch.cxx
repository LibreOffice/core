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
#include <tools/GenericTypeSerializer.hxx>
#include <vcl/hatch.hxx>

ImplHatch::ImplHatch() :
    maColor     ( COL_BLACK ),
    meStyle     ( HatchStyle::Single ),
    mnDistance  ( 1 ),
    mnAngle     ( 0 )
{
}

bool ImplHatch::operator==( const ImplHatch& rImplHatch ) const
{
    return maColor == rImplHatch.maColor &&
        meStyle == rImplHatch.meStyle &&
        mnDistance == rImplHatch.mnDistance &&
        mnAngle == rImplHatch.mnAngle;
}

Hatch::Hatch() = default;

Hatch::Hatch( const Hatch& ) = default;

Hatch::Hatch( HatchStyle eStyle, const Color& rColor,
              tools::Long nDistance, Degree10 nAngle10 ) : mpImplHatch()
{
    mpImplHatch->maColor = rColor;
    mpImplHatch->meStyle = eStyle;
    mpImplHatch->mnDistance = nDistance;
    mpImplHatch->mnAngle = nAngle10;
}

Hatch::~Hatch() = default;

Hatch& Hatch::operator=( const Hatch& ) = default;

bool Hatch::operator==( const Hatch& rHatch ) const
{
    return mpImplHatch == rHatch.mpImplHatch;
}


void Hatch::SetColor( const Color& rColor )
{
    mpImplHatch->maColor = rColor;
}

void Hatch::SetDistance( tools::Long nDistance )
{
    mpImplHatch->mnDistance = nDistance;
}

void Hatch::SetAngle( Degree10 nAngle10 )
{
    mpImplHatch->mnAngle = nAngle10;
}

SvStream& ReadHatch( SvStream& rIStm, Hatch& rHatch )
{
    VersionCompatRead aCompat(rIStm);
    sal_uInt16 nTmp16;
    sal_Int32 nTmp32(0);

    rIStm.ReadUInt16(nTmp16);
    rHatch.mpImplHatch->meStyle = static_cast<HatchStyle>(nTmp16);

    tools::GenericTypeSerializer aSerializer(rIStm);
    aSerializer.readColor(rHatch.mpImplHatch->maColor);
    rIStm.ReadInt32(nTmp32);
    rHatch.mpImplHatch->mnDistance = nTmp32;
    rIStm.ReadUInt16(nTmp16);
    rHatch.mpImplHatch->mnAngle = Degree10(nTmp16);

    return rIStm;
}

SvStream& WriteHatch( SvStream& rOStm, const Hatch& rHatch )
{
    VersionCompatWrite aCompat(rOStm, 1);

    rOStm.WriteUInt16( static_cast<sal_uInt16>(rHatch.mpImplHatch->meStyle) );

    tools::GenericTypeSerializer aSerializer(rOStm);
    aSerializer.writeColor(rHatch.mpImplHatch->maColor);
    rOStm.WriteInt32( rHatch.mpImplHatch->mnDistance ).WriteUInt16( rHatch.mpImplHatch->mnAngle.get() );

    return rOStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
