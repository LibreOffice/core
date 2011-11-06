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


#ifndef _SHDWCRSR_HXX
#define _SHDWCRSR_HXX


#include <tools/gen.hxx>
#include <tools/color.hxx>
#include  <limits.h>

class Window;

class SwShadowCursor
{
    Window* pWin;
    Color aCol;
    Point aOldPt;
    long nOldHeight;
    sal_uInt16 nOldMode;

    void DrawTri( const Point& rPt, long nHeight, sal_Bool bLeft );
    void DrawCrsr( const Point& rPt, long nHeight, sal_uInt16 nMode );

public:
    SwShadowCursor( Window& rWin, const Color& rCol )
        : pWin( &rWin ), aCol( rCol ), nOldHeight(0), nOldMode( USHRT_MAX ) {}
    ~SwShadowCursor();

    void SetPos( const Point& rPt, long nHeight, sal_uInt16 nMode );

    void Paint();

    const Point& GetPoint() const   { return aOldPt; }
    long GetHeight() const          { return nOldHeight; }
    sal_uInt16 GetMode() const          { return nOldMode; }

    Rectangle GetRect() const;
};



#endif

