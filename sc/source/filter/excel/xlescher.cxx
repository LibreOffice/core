/*************************************************************************
 *
 *  $RCSfile: xlescher.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:47:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef SC_XLESCHER_HXX
#include "xlescher.hxx"
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XESTREAM_HXX
#include "xestream.hxx"
#endif


// Escher client anchor =======================================================

namespace {

/** Returns the scaling factor to calculate coordinates from twips. */
double lclGetTwipsScale( MapUnit eMapUnit )
{
    /*  #111027# We cannot use OutputDevice::LogicToLogic() or the XclTools conversion
        functions to calculate drawing layer coordinates due to Calc's strange definition
        of a point (1 inch == 72.27 points, instead of 72 points). */
    double fScale = 1.0;
    switch( eMapUnit )
    {
        case MAP_TWIP:      fScale = 72 / POINTS_PER_INCH;  break;  // Calc twips <-> real twips
        case MAP_100TH_MM:  fScale = HMM_PER_TWIPS;         break;  // Calc twips <-> 1/100mm
        default:            DBG_ERRORFILE( "lclGetTwipsScale - map unit not implemented" );
    }
    return fScale;
}

/** Calculates a drawing layer X position (in twips) from an Escher object column position. */
long lclGetXFromCol( ScDocument& rDoc, SCTAB nScTab, sal_uInt16 nXclCol, sal_uInt16 nOffset, double fScale )
{
    return static_cast< long >( fScale * (rDoc.GetColOffset( static_cast<SCCOL>(nXclCol), nScTab ) +
        ::std::min( nOffset / 1024.0, 1.0 ) * rDoc.GetColWidth( static_cast<SCCOL>(nXclCol), nScTab )) + 0.5 );
}

/** Calculates a drawing layer Y position (in twips) from an Escher object row position. */
long lclGetYFromRow( ScDocument& rDoc, SCTAB nScTab, sal_uInt16 nXclRow, sal_uInt16 nOffset, double fScale )
{
    return static_cast< long >( fScale * (rDoc.GetRowOffset( static_cast<SCROW>(nXclRow), nScTab ) +
        ::std::min( nOffset / 256.0, 1.0 ) * rDoc.GetRowHeight( static_cast<SCROW>(nXclRow), nScTab )) + 0.5 );
}

/** Calculates an Escher object column position from a drawing layer X position (in twips). */
void lclGetColFromX(
        ScDocument& rDoc, SCTAB nScTab, sal_uInt16& rnXclCol,
        sal_uInt16& rnOffset, sal_uInt16 nXclStartCol,
        long& rnStartW, long nX, double fScale )
{
    // rnStartW in conjunction with nXclStartCol is used as buffer for previously calculated width
    long nTwipsX = static_cast< long >( nX / fScale + 0.5 );
    long nColW = 0;
    for( rnXclCol = nXclStartCol; rnXclCol <= MAXCOL; ++rnXclCol )
    {
        nColW = rDoc.GetColWidth( static_cast<SCCOL>(rnXclCol), nScTab );
        if( rnStartW + nColW > nTwipsX )
            break;
        rnStartW += nColW;
    }
    rnOffset = nColW ? static_cast< sal_uInt16 >( (nTwipsX - rnStartW) * 1024.0 / nColW + 0.5 ) : 0;
}

/** Calculates an Escher object row position from a drawing layer Y position (in twips). */
void lclGetRowFromY(
        ScDocument& rDoc, SCTAB nScTab,
        sal_uInt16& rnXclRow, sal_uInt16& rnOffset, sal_uInt16 nXclStartRow,
        long& rnStartH, long nY, double fScale )
{
    // rnStartH in conjunction with nXclStartRow is used as buffer for previously calculated height
    long nTwipsY = static_cast< long >( nY / fScale + 0.5 );
    long nRowH = 0;
    for( rnXclRow = nXclStartRow; rnXclRow <= MAXROW; ++rnXclRow )
    {
        nRowH = rDoc.GetRowHeight( static_cast<SCROW>(rnXclRow), nScTab );
        if( rnStartH + nRowH > nTwipsY )
            break;
        rnStartH += nRowH;
    }
    rnOffset = nRowH ? static_cast< sal_uInt16 >( (nTwipsY - rnStartH) * 256.0 / nRowH + 0.5 ) : 0;
}

/** Mirrors a rectangle (from LTR to RTL layout or vice versa). */
void lclMirrorRectangle( Rectangle& rRect )
{
    long nLeft = rRect.Left();
    rRect.Left() = -rRect.Right();
    rRect.Right() = -nLeft;
}

} // namespace


// ----------------------------------------------------------------------------

XclEscherAnchor::XclEscherAnchor( SCTAB nScTab ) :
    mnLCol( 0 ), mnLX( 0 ),
    mnTRow( 0 ), mnTY( 0 ),
    mnRCol( 0 ), mnRX( 0 ),
    mnBRow( 0 ), mnBY( 0 ),
    mnScTab( nScTab )
{
}

Rectangle XclEscherAnchor::GetRect( ScDocument& rDoc, MapUnit eMapUnit ) const
{
    double fScale = lclGetTwipsScale( eMapUnit );
    Rectangle aRect(
        lclGetXFromCol( rDoc, mnScTab, mnLCol, mnLX, fScale ),
        lclGetYFromRow( rDoc, mnScTab, mnTRow, mnTY, fScale ),
        lclGetXFromCol( rDoc, mnScTab, mnRCol, mnRX + 1, fScale ),
        lclGetYFromRow( rDoc, mnScTab, mnBRow, mnBY, fScale ) );

    // #106948# adjust coordinates in mirrored sheets
    if( rDoc.IsLayoutRTL( mnScTab ) )
        lclMirrorRectangle( aRect );
    return aRect;
}

void XclEscherAnchor::SetRect( ScDocument& rDoc, const Rectangle& rRect, MapUnit eMapUnit )
{
    Rectangle aRect( rRect );
    // #106948# adjust coordinates in mirrored sheets
    if( rDoc.IsLayoutRTL( mnScTab ) )
        lclMirrorRectangle( aRect );

    double fScale = lclGetTwipsScale( eMapUnit );
    long nDummy = 0;
    lclGetColFromX( rDoc, mnScTab, mnLCol, mnLX, 0,      nDummy, aRect.Left(),   fScale );
    lclGetColFromX( rDoc, mnScTab, mnRCol, mnRX, mnLCol, nDummy, aRect.Right(),  fScale );
    nDummy = 0;
    lclGetRowFromY( rDoc, mnScTab, mnTRow, mnTY, 0,      nDummy, aRect.Top(),    fScale );
    lclGetRowFromY( rDoc, mnScTab, mnBRow, mnBY, mnTRow, nDummy, aRect.Bottom(), fScale );
}

SvStream& operator>>( SvStream& rStrm, XclEscherAnchor& rAnchor )
{
    return rStrm
        >> rAnchor.mnLCol >> rAnchor.mnLX
        >> rAnchor.mnTRow >> rAnchor.mnTY
        >> rAnchor.mnRCol >> rAnchor.mnRX
        >> rAnchor.mnBRow >> rAnchor.mnBY;
}

SvStream& operator<<( SvStream& rStrm, const XclEscherAnchor& rAnchor )
{
    return rStrm
        << rAnchor.mnLCol << rAnchor.mnLX
        << rAnchor.mnTRow << rAnchor.mnTY
        << rAnchor.mnRCol << rAnchor.mnRX
        << rAnchor.mnBRow << rAnchor.mnBY;
}

XclImpStream& operator>>( XclImpStream& rStrm, XclEscherAnchor& rAnchor )
{
    return rStrm
        >> rAnchor.mnLCol >> rAnchor.mnLX
        >> rAnchor.mnTRow >> rAnchor.mnTY
        >> rAnchor.mnRCol >> rAnchor.mnRX
        >> rAnchor.mnBRow >> rAnchor.mnBY;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclEscherAnchor& rAnchor )
{
    return rStrm
        << rAnchor.mnLCol << rAnchor.mnLX
        << rAnchor.mnTRow << rAnchor.mnTY
        << rAnchor.mnRCol << rAnchor.mnRX
        << rAnchor.mnBRow << rAnchor.mnBY;
}


// ============================================================================

