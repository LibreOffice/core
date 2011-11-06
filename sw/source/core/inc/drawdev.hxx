/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    inline SwDrawDev( OutputDevice *pOutDev, const Point *pPosition )
        :pOut(pOutDev), pPos(pPosition) { }

    inline OutputDevice *GetOut() { return pOut; }

    // Ausgabemethoden
    inline void DrawText( const Point& rStart, const String& rTxt,
                          const sal_uInt16 nIdx = 0,
                          const sal_uInt16 nLen = STRING_LEN );
    inline void DrawStretchText( const Point& rStart, sal_uInt16 nWidth,
                          const String& rTxt,
                          const sal_uInt16 nIdx = 0,
                          const sal_uInt16 nLen = STRING_LEN );
    inline void DrawTextArray( const Point& rStart,
                          const String& rTxt,
                          long *pKernArray = 0,
                          const sal_uInt16 nIdx = 0,
                          const sal_uInt16 nLen = STRING_LEN);
    inline void DrawLine( const Point& rStart, const Point& rEnd );
    inline void DrawRect( const SwRect& rRect,
                          const sal_uInt16 nHorzRount = 0,
                          const sal_uInt16 nVertRound = 0 );

    inline const Point *GetOrigin() const {return pPos; }
};

/*************************************************************************
 *                      SwDrawDev::DrawText
 *************************************************************************/

inline void SwDrawDev::DrawText( const Point& rStart, const String& rTxt,
                                 const sal_uInt16 nIdx, const sal_uInt16 nLen )
{
    if( !pPos )
        pOut->DrawText( rStart, rTxt, nIdx, nLen );
    else
        pOut->DrawText( rStart - *pPos, rTxt, nIdx, nLen );
}

/*************************************************************************
 *                      SwDrawDev::DrawStretchText
 *************************************************************************/

inline void SwDrawDev::DrawStretchText( const Point& rStart, sal_uInt16 nWidth,
       const String& rTxt, const sal_uInt16 nIdx, const sal_uInt16 nLen )
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
            long *pKernArray, const sal_uInt16 nIdx, const sal_uInt16 nLen )
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
                      const sal_uInt16 nHorzRound, const sal_uInt16 nVertRound )
{
    SwRect aRect( rRect );
    if( pPos )
        aRect.Pos() -= *pPos;
    pOut->DrawRect( aRect.SVRect(), nHorzRound, nVertRound );
}


#endif
