/*************************************************************************
 *
 *  $RCSfile: drawutil.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:59:44 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
    for (SCROW j=nStartRow; j<nEndRow; j++)
    {
        USHORT nHeight = pDoc->GetRowHeight(j,nTab);
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




