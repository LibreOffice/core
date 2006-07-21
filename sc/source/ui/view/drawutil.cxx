/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawutil.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:53:11 $
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <vcl/outdev.hxx>

#include "drawutil.hxx"
#include "document.hxx"
#include "global.hxx"
#include "viewdata.hxx"

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------


inline Fraction MakeFraction( long nA, long nB )
{
    return ( nA && nB ) ? Fraction(nA,nB) : Fraction(1,1);
}

void ScDrawUtil::CalcScale( ScDocument* pDoc, SCTAB nTab,
                            SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            OutputDevice* pDev,
                            const Fraction& rZoomX, const Fraction& rZoomY,
                            double nPPTX, double nPPTY,
                            Fraction& rScaleX, Fraction& rScaleY )
{
    long nPixelX = 0;
    long nTwipsX = 0;
    long nPixelY = 0;
    long nTwipsY = 0;
    for (SCCOL i=nStartCol; i<nEndCol; i++)
    {
        USHORT nWidth = pDoc->GetColWidth(i,nTab);
        nTwipsX += (long) nWidth;
        nPixelX += ScViewData::ToPixel( nWidth, nPPTX );
    }
    ScCoupledCompressedArrayIterator< SCROW, BYTE, USHORT> aIter(
            pDoc->GetRowFlagsArray( nTab), nStartRow, nEndRow-1, CR_HIDDEN, 0,
            pDoc->GetRowHeightArray( nTab));
    for ( ; aIter; ++aIter )
    {
        USHORT nHeight = *aIter;
        nTwipsY += (long) nHeight;
        nPixelY += ScViewData::ToPixel( nHeight, nPPTY );
    }

    MapMode aHMMMode( MAP_100TH_MM, Point(), rZoomX, rZoomY );
    Point aPixelLog = pDev->PixelToLogic( Point( nPixelX,nPixelY ), aHMMMode );

    //  Fraction(double) ctor can be used here (and avoid overflows of PixelLog * Zoom)
    //  because ReduceInaccurate is called later anyway.

    if ( aPixelLog.X() && nTwipsX )
        rScaleX = Fraction( ((double)aPixelLog.X()) *
                            ((double)rZoomX.GetNumerator()) /
                            ((double)nTwipsX) /
                            ((double)HMM_PER_TWIPS) /
                            ((double)rZoomX.GetDenominator()) );
    else
        rScaleX = Fraction( 1, 1 );

    if ( aPixelLog.Y() && nTwipsY )
        rScaleY = Fraction( ((double)aPixelLog.Y()) *
                            ((double)rZoomY.GetNumerator()) /
                            ((double)nTwipsY) /
                            ((double)HMM_PER_TWIPS) /
                            ((double)rZoomY.GetDenominator()) );
    else
        rScaleY = Fraction( 1, 1 );

    //  17 bits of accuracy are needed to always hit the right part of
    //  cells in the last rows
    rScaleX.ReduceInaccurate( 17 );
    rScaleY.ReduceInaccurate( 17 );
}




