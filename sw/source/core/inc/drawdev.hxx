/*************************************************************************
 *
 *  $RCSfile: drawdev.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:20 $
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

#ifndef _DRAWDEV_HXX
#define _DRAWDEV_HXX

#include "swrect.hxx"

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

/*************************************************************************
 *                          class SwDrawDev
 *
 * Alle Draw-Methoden werden um den Offset *pPos verschoben.
 *************************************************************************/

class SwDrawDev
{
    OutputDevice  *pOut;
    const Point *pPos;

public:
    inline SwDrawDev( OutputDevice *pOut, const Point *pPos )
        :pOut(pOut), pPos(pPos) { }

    inline OutputDevice *GetOut() { return pOut; }

    // Ausgabemethoden
    inline void DrawText( const Point& rStart, const String& rTxt,
                          const USHORT nIdx = 0,
                          const USHORT nLen = STRING_LEN );
    inline void DrawStretchText( const Point& rStart, USHORT nWidth,
                          const String& rTxt,
                          const USHORT nIdx = 0,
                          const USHORT nLen = STRING_LEN );
    inline void DrawTextArray( const Point& rStart,
                          const String& rTxt,
                          long *pKernArray = 0,
                          const USHORT nIdx = 0,
                          const USHORT nLen = STRING_LEN);
    inline void DrawLine( const Point& rStart, const Point& rEnd );
    inline void DrawRect( const SwRect& rRect,
                          const USHORT nHorzRount = 0,
                          const USHORT nVertRound = 0 );

    inline const Point *GetOrigin() const {return pPos; }
};

/*************************************************************************
 *                      SwDrawDev::DrawText
 *************************************************************************/

inline void SwDrawDev::DrawText( const Point& rStart, const String& rTxt,
                                 const USHORT nIdx, const USHORT nLen )
{
    if( !pPos )
        pOut->DrawText( rStart, rTxt, nIdx, nLen );
    else
        pOut->DrawText( rStart - *pPos, rTxt, nIdx, nLen );
}

/*************************************************************************
 *                      SwDrawDev::DrawStretchText
 *************************************************************************/

inline void SwDrawDev::DrawStretchText( const Point& rStart, USHORT nWidth,
       const String& rTxt, const USHORT nIdx, const USHORT nLen )
{
    if( !pPos )
        pOut->DrawStretchText( rStart, nWidth, rTxt, nIdx, nLen );
    else
        pOut->DrawStretchText( rStart - *pPos, nWidth, rTxt, nIdx, nLen );
}

/*************************************************************************
 *                      SwDrawDev::DrawTextArray
 *************************************************************************/

inline void SwDrawDev::DrawTextArray( const Point& rStart, const String& rTxt,
            long *pKernArray, const USHORT nIdx, const USHORT nLen )
{
    if( !pPos )
        pOut->DrawTextArray( rStart, rTxt, pKernArray, nIdx, nLen );
    else
        pOut->DrawTextArray( rStart - *pPos, rTxt, pKernArray, nIdx, nLen );
}

/*************************************************************************
 *                      SwDrawDev::DrawLine
 *************************************************************************/

inline void SwDrawDev::DrawLine( const Point& rStart, const Point& rEnd )
{
    if( !pPos )
        pOut->DrawLine( rStart, rEnd );
    else
        pOut->DrawLine( rStart - *pPos, rEnd - *pPos );
}

/*************************************************************************
 *                      SwDrawDev::DrawRect
 *************************************************************************/

inline void SwDrawDev::DrawRect( const SwRect& rRect,
                      const USHORT nHorzRound, const USHORT nVertRound )
{
    SwRect aRect( rRect );
    if( pPos )
        aRect.Pos() -= *pPos;
    pOut->DrawRect( aRect.SVRect(), nHorzRound, nVertRound );
}


#endif
