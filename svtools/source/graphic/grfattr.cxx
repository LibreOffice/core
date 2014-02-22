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
#include <svtools/grfmgr.hxx>


// - GraphicAttr -


GraphicAttr::GraphicAttr() :
    mfGamma         ( 1.0 ),
    mnMirrFlags     ( 0 ),
    mnLeftCrop      ( 0 ),
    mnTopCrop       ( 0 ),
    mnRightCrop     ( 0 ),
    mnBottomCrop    ( 0 ),
    mnRotate10      ( 0 ),
    mnContPercent   ( 0 ),
    mnLumPercent    ( 0 ),
    mnRPercent      ( 0 ),
    mnGPercent      ( 0 ),
    mnBPercent      ( 0 ),
    mbInvert        ( sal_False ),
    mcTransparency  ( 0 ),
    meDrawMode      ( GRAPHICDRAWMODE_STANDARD )
{
}



GraphicAttr::~GraphicAttr()
{
}



sal_Bool GraphicAttr::operator==( const GraphicAttr& rAttr ) const
{
    return( ( mfGamma == rAttr.mfGamma ) &&
            ( mnMirrFlags == rAttr.mnMirrFlags ) &&
            ( mnLeftCrop == rAttr.mnLeftCrop ) &&
            ( mnTopCrop == rAttr.mnTopCrop ) &&
            ( mnRightCrop == rAttr.mnRightCrop ) &&
            ( mnBottomCrop == rAttr.mnBottomCrop ) &&
            ( mnRotate10 == rAttr.mnRotate10 ) &&
            ( mnContPercent == rAttr.mnContPercent ) &&
            ( mnLumPercent == rAttr.mnLumPercent ) &&
            ( mnRPercent == rAttr.mnRPercent ) &&
            ( mnGPercent == rAttr.mnGPercent ) &&
            ( mnBPercent == rAttr.mnBPercent ) &&
            ( mbInvert == rAttr.mbInvert ) &&
            ( mcTransparency == rAttr.mcTransparency ) &&
            ( meDrawMode == rAttr.meDrawMode ) );
}



SvStream& ReadGraphicAttr( SvStream& rIStm, GraphicAttr& rAttr )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    sal_uInt32      nTmp32;
    sal_uInt16          nTmp16;

    rIStm.ReadUInt32( nTmp32 ).ReadUInt32( nTmp32 ).ReadDouble( rAttr.mfGamma ).ReadUInt32( rAttr.mnMirrFlags ).ReadUInt16( rAttr.mnRotate10 );
    rIStm.ReadInt16( rAttr.mnContPercent ).ReadInt16( rAttr.mnLumPercent ).ReadInt16( rAttr.mnRPercent ).ReadInt16( rAttr.mnGPercent ).ReadInt16( rAttr.mnBPercent );
    rIStm.ReadUChar( rAttr.mbInvert ).ReadUChar( rAttr.mcTransparency ).ReadUInt16( nTmp16 );
    rAttr.meDrawMode = (GraphicDrawMode) nTmp16;

    if( aCompat.GetVersion() >= 2 )
    {
        //#fdo39428 SvStream no longer supports operator>>(long&)
        sal_Int32 nTmpL(0), nTmpT(0), nTmpR(0), nTmpB(0);
        rIStm.ReadInt32( nTmpL ).ReadInt32( nTmpT ).ReadInt32( nTmpR ).ReadInt32( nTmpB );
        rAttr.mnLeftCrop = nTmpL;
        rAttr.mnTopCrop = nTmpT;
        rAttr.mnRightCrop = nTmpR;
        rAttr.mnBottomCrop = nTmpB;
    }

    return rIStm;
}



SvStream& WriteGraphicAttr( SvStream& rOStm, const GraphicAttr& rAttr )
{
    VersionCompat       aCompat( rOStm, STREAM_WRITE, 2 );
    const sal_uInt32    nTmp32 = 0;

    rOStm.WriteUInt32( nTmp32 ).WriteUInt32( nTmp32 );
    rOStm.WriteDouble( rAttr.mfGamma );
    rOStm.WriteUInt32( rAttr.mnMirrFlags ).WriteUInt16( rAttr.mnRotate10 );
    rOStm.WriteInt16( rAttr.mnContPercent ).WriteInt16( rAttr.mnLumPercent ).WriteInt16( rAttr.mnRPercent ).WriteInt16( rAttr.mnGPercent ).WriteInt16( rAttr.mnBPercent );
    rOStm.WriteUChar( rAttr.mbInvert ).WriteUChar( rAttr.mcTransparency ).WriteUInt16( (sal_uInt16) rAttr.meDrawMode );
    //#fdo39428 SvStream no longer supports operator<<(long)
    rOStm.WriteInt32( sal::static_int_cast<sal_Int32>(rAttr.mnLeftCrop) )
         .WriteInt32( sal::static_int_cast<sal_Int32>(rAttr.mnTopCrop) )
         .WriteInt32( sal::static_int_cast<sal_Int32>(rAttr.mnRightCrop) )
         .WriteInt32( sal::static_int_cast<sal_Int32>(rAttr.mnBottomCrop) );

    return rOStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
