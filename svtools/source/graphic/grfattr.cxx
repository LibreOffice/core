/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <tools/vcompat.hxx>
#include <svtools/grfmgr.hxx>

// ---------------
// - GraphicAttr -
// ---------------

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

// ------------------------------------------------------------------------

GraphicAttr::~GraphicAttr()
{
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, GraphicAttr& rAttr )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    sal_uInt32      nTmp32;
    sal_uInt16          nTmp16;

    rIStm >> nTmp32 >> nTmp32 >> rAttr.mfGamma >> rAttr.mnMirrFlags >> rAttr.mnRotate10;
    rIStm >> rAttr.mnContPercent >> rAttr.mnLumPercent >> rAttr.mnRPercent >> rAttr.mnGPercent >> rAttr.mnBPercent;
    rIStm >> rAttr.mbInvert >> rAttr.mcTransparency >> nTmp16;
    rAttr.meDrawMode = (GraphicDrawMode) nTmp16;

    if( aCompat.GetVersion() >= 2 )
    {
        //#fdo39428 SvStream no longer supports operator>>(long&)
        sal_Int32 nTmpL(0), nTmpT(0), nTmpR(0), nTmpB(0);
        rIStm >> nTmpL >> nTmpT >> nTmpR >> nTmpB;
        rAttr.mnLeftCrop = nTmpL;
        rAttr.mnTopCrop = nTmpT;
        rAttr.mnRightCrop = nTmpR;
        rAttr.mnBottomCrop = nTmpB;
    }

    return rIStm;
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const GraphicAttr& rAttr )
{
    VersionCompat       aCompat( rOStm, STREAM_WRITE, 2 );
    const sal_uInt32    nTmp32 = 0;

    rOStm << nTmp32 << nTmp32 << rAttr.mfGamma << rAttr.mnMirrFlags << rAttr.mnRotate10;
    rOStm << rAttr.mnContPercent << rAttr.mnLumPercent << rAttr.mnRPercent << rAttr.mnGPercent << rAttr.mnBPercent;
    rOStm << rAttr.mbInvert << rAttr.mcTransparency << (sal_uInt16) rAttr.meDrawMode;
    //#fdo39428 SvStream no longer supports operator<<(long)
    rOStm << sal::static_int_cast<sal_Int32>(rAttr.mnLeftCrop)
          << sal::static_int_cast<sal_Int32>(rAttr.mnTopCrop)
          << sal::static_int_cast<sal_Int32>(rAttr.mnRightCrop)
          << sal::static_int_cast<sal_Int32>(rAttr.mnBottomCrop);

    return rOStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
