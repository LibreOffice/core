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

#include <svx/autoformathelper.hxx>
#include <tools/stream.hxx>
#include <svl/legacyitem.hxx>
#include <editeng/legacyitem.hxx>
#include <svx/legacyitem.hxx>

AutoFormatVersions::AutoFormatVersions()
:   nFontVersion(0),
    nFontHeightVersion(0),
    nWeightVersion(0),
    nPostureVersion(0),
    nUnderlineVersion(0),
    nOverlineVersion(0),
    nCrossedOutVersion(0),
    nContourVersion(0),
    nShadowedVersion(0),
    nColorVersion(0),
    nBoxVersion(0),
    nLineVersion(0),
    nBrushVersion(0),
    nAdjustVersion(0),
    nHorJustifyVersion(0),
    nVerJustifyVersion(0),
    nOrientationVersion(0),
    nMarginVersion(0),
    nBoolVersion(0),
    nInt32Version(0),
    nRotateModeVersion(0),
    nNumFormatVersion(0)
{
}

const sal_uInt16 AUTOFORMAT_ID_300OVRLN      = 10031;
const sal_uInt16 AUTOFORMAT_ID_680DR14      = 10011;
const sal_uInt16 AUTOFORMAT_ID_504      = 9801;

void AutoFormatVersions::LoadBlockA( SvStream& rStream, sal_uInt16 nVer )
{
    rStream.ReadUInt16( nFontVersion );
    rStream.ReadUInt16( nFontHeightVersion );
    rStream.ReadUInt16( nWeightVersion );
    rStream.ReadUInt16( nPostureVersion );
    rStream.ReadUInt16( nUnderlineVersion );
    if ( nVer >= AUTOFORMAT_ID_300OVRLN )
        rStream.ReadUInt16( nOverlineVersion );
    rStream.ReadUInt16( nCrossedOutVersion );
    rStream.ReadUInt16( nContourVersion );
    rStream.ReadUInt16( nShadowedVersion );
    rStream.ReadUInt16( nColorVersion );
    rStream.ReadUInt16( nBoxVersion );
    if ( nVer >= AUTOFORMAT_ID_680DR14 )
        rStream.ReadUInt16( nLineVersion );
    rStream.ReadUInt16( nBrushVersion );
    rStream.ReadUInt16( nAdjustVersion );
}

void AutoFormatVersions::LoadBlockB( SvStream& rStream, sal_uInt16 nVer )
{
    rStream.ReadUInt16( nHorJustifyVersion );
    rStream.ReadUInt16( nVerJustifyVersion );
    rStream.ReadUInt16( nOrientationVersion );
    rStream.ReadUInt16( nMarginVersion );
    rStream.ReadUInt16( nBoolVersion );
    if ( nVer >= AUTOFORMAT_ID_504 )
    {
        rStream.ReadUInt16( nInt32Version );
        rStream.ReadUInt16( nRotateModeVersion );
    }
    rStream.ReadUInt16( nNumFormatVersion );
}

void AutoFormatVersions::WriteBlockA(SvStream& rStream, sal_uInt16 fileVersion)
{
    rStream.WriteUInt16(legacy::SvxFont::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxFontHeight::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxWeight::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxPosture::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxTextLine::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxTextLine::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxCrossedOut::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SfxBool::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SfxBool::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxColor::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxBox::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxLine::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxBrush::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxAdjust::GetVersion(fileVersion));
}

void AutoFormatVersions::WriteBlockB(SvStream& rStream, sal_uInt16 fileVersion)
{
    rStream.WriteUInt16(legacy::SvxHorJustify::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxVerJustify::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxOrientation::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxMargin::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SfxBool::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::CntInt32::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxRotateMode::GetVersion(fileVersion));
    rStream.WriteUInt16( 0 );       // NumberFormat
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
