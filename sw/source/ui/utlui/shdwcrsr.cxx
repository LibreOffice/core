/*************************************************************************
 *
 *  $RCSfile: shdwcrsr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:50 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#include "swtypes.hxx"
#include "shdwcrsr.hxx"


SwShadowCursor::~SwShadowCursor()
{
    if( USHRT_MAX != nOldMode )
        DrawCrsr( aOldPt, nOldHeight, nOldMode );
}

void SwShadowCursor::SetPos( const Point& rPt, long nHeight, USHORT nMode )
{
    Point aPt( pWin->LogicToPixel( rPt ));
    nHeight = pWin->LogicToPixel( Size( 0, nHeight )).Height();
    if( aOldPt != aPt || nOldHeight != nHeight || nOldMode != nMode )
    {
        if( USHRT_MAX != nOldMode )
            DrawCrsr( aOldPt, nOldHeight, nOldMode );

        DrawCrsr( aPt, nHeight, nMode );
        nOldMode = nMode;
        nOldHeight = nHeight;
        aOldPt = aPt;
    }
}

void SwShadowCursor::DrawTri( const Point& rPt, long nHeight, BOOL bLeft )
{
    USHORT nLineDiff = ( nHeight / 2 );
    USHORT nLineDiffHalf = nLineDiff / 2;

    // Punkt oben
    Point aPt1( (bLeft ? rPt.X() - 3 : rPt.X() + 3),
                rPt.Y() + nLineDiffHalf );
    // Punkt unten
    Point aPt2( aPt1.X(), aPt1.Y() + nHeight - nLineDiff - 1 );
    short nDiff = bLeft ? -1 : 1;
    while( aPt1.Y() <= aPt2.Y() )
    {
        pWin->DrawLine( aPt1, aPt2 );
        aPt1.Y()++, aPt2.Y()--;
        aPt2.X() = aPt1.X() += nDiff;
    }
}

void SwShadowCursor::DrawCrsr( const Point& rPt, long nHeight, USHORT nMode )
{
    nHeight = (((nHeight / 4)+1) * 4) + 1;

    pWin->Push();

    pWin->SetMapMode( MAP_PIXEL );
    pWin->SetRasterOp( ROP_XOR );

    pWin->SetLineColor( Color( aCol.GetColor() ^ COL_WHITE ) );

    // 1. der Strich:
    pWin->DrawLine( Point( rPt.X(), rPt.Y() + 1),
              Point( rPt.X(), rPt.Y() - 2 + nHeight ));

    // 2. das Dreieck
    if( HORI_LEFT == nMode || HORI_CENTER == nMode )    // Pfeil nach rechts
        DrawTri( rPt, nHeight, FALSE );
    if( HORI_RIGHT == nMode || HORI_CENTER == nMode )   // Pfeil nach links
        DrawTri( rPt, nHeight, TRUE );

    pWin->Pop();
}

void SwShadowCursor::Paint()
{
    if( USHRT_MAX != nOldMode )
        DrawCrsr( aOldPt, nOldHeight, nOldMode );
}

Rectangle SwShadowCursor::GetRect() const
{
    long nH = nOldHeight;
    Point aPt( aOldPt );

    nH = (((nH / 4)+1) * 4) + 1;
    USHORT nWidth = nH / 4 + 3 + 1;

    Size aSz( nWidth, nH );

    if( HORI_RIGHT == nOldMode )
        aPt.X() -= aSz.Width();
    else if( HORI_CENTER == nOldMode )
    {
        aPt.X() -= aSz.Width();
        aSz.Width() *= 2;
    }

    return pWin->PixelToLogic( Rectangle( aPt, aSz ) );
}

/*************************************************************************

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.5  2000/09/18 16:06:19  willem.vandorp
      OpenOffice header added.

      Revision 1.4  2000/02/11 15:00:59  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.3  1999/09/20 10:45:53  os
      Color changes

      Revision 1.2  1997/11/03 15:10:26  JP
      neu: Optionen/-Page/Basic-Schnittst. fuer ShadowCursor


      Rev 1.1   03 Nov 1997 16:10:26   JP
   neu: Optionen/-Page/Basic-Schnittst. fuer ShadowCursor

      Rev 1.0   24 Oct 1997 18:31:58   JP
   Cursor fuers Einfuegen an beliebiger Position


*************************************************************************/



