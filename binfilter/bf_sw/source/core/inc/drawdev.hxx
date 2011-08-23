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

#ifndef _DRAWDEV_HXX
#define _DRAWDEV_HXX

#include "swrect.hxx"

#include <vcl/outdev.hxx>
namespace binfilter {

/*************************************************************************
 *							class SwDrawDev
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
 *						SwDrawDev::DrawText
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
 *						SwDrawDev::DrawStretchText
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
 *						SwDrawDev::DrawTextArray
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
 *						SwDrawDev::DrawLine
 *************************************************************************/

inline void SwDrawDev::DrawLine( const Point& rStart, const Point& rEnd )
{
    if( !pPos )
        pOut->DrawLine( rStart, rEnd );
    else
        pOut->DrawLine( rStart - *pPos, rEnd - *pPos );
}

/*************************************************************************
 *						SwDrawDev::DrawRect
 *************************************************************************/

inline void SwDrawDev::DrawRect( const SwRect& rRect,
                      const USHORT nHorzRound, const USHORT nVertRound )
{
    SwRect aRect( rRect );
    if( pPos )
        aRect.Pos() -= *pPos;
    pOut->DrawRect( aRect.SVRect(), nHorzRound, nVertRound );
}


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
