/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grfattr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:53:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"

#include <tools/vcompat.hxx>
#include "grfmgr.hxx"

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
    mbInvert        ( FALSE ),
    mcTransparency  ( 0 ),
    meDrawMode      ( GRAPHICDRAWMODE_STANDARD )
{
}

// ------------------------------------------------------------------------

GraphicAttr::~GraphicAttr()
{
}

// ------------------------------------------------------------------------

BOOL GraphicAttr::operator==( const GraphicAttr& rAttr ) const
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
    UINT16          nTmp16;

    rIStm >> nTmp32 >> nTmp32 >> rAttr.mfGamma >> rAttr.mnMirrFlags >> rAttr.mnRotate10;
    rIStm >> rAttr.mnContPercent >> rAttr.mnLumPercent >> rAttr.mnRPercent >> rAttr.mnGPercent >> rAttr.mnBPercent;
    rIStm >> rAttr.mbInvert >> rAttr.mcTransparency >> nTmp16;
    rAttr.meDrawMode = (GraphicDrawMode) nTmp16;

    if( aCompat.GetVersion() >= 2 )
    {
        rIStm >> rAttr.mnLeftCrop >> rAttr.mnTopCrop >> rAttr.mnRightCrop >> rAttr.mnBottomCrop;
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
    rOStm << rAttr.mbInvert << rAttr.mcTransparency << (UINT16) rAttr.meDrawMode;
    rOStm << rAttr.mnLeftCrop << rAttr.mnTopCrop << rAttr.mnRightCrop << rAttr.mnBottomCrop;

    return rOStm;
}
