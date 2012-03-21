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



#ifndef _WINGDI_
#define _WINGDI_

typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT, *PPOINT, *LPPOINT;


typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef struct
{
    WORD    fract;
    SHORT   value;
} W32FIXED;

typedef struct tagPOINTFX
{
    W32FIXED x;
    W32FIXED y;
} POINTFX, *LPPOINTFX;

typedef struct tagTTPOLYCURVE
{
    WORD wType;
    WORD cpfx;
    POINTFX apfx[1];
} TTPOLYCURVE, *LPTTPOLYCURVE;

typedef struct tagTTPOLYGONHEADER
{
    DWORD cb;
    DWORD dwType;
    POINTFX pfxStart;
} TTPOLYGONHEADER, *LPTTPOLYGONHEADER;

typedef struct
{
    UINT    gmBlackBoxX;
    UINT    gmBlackBoxY;
    POINT   gmptGlyphOrigin;
    SHORT   gmCellIncX;
    SHORT   gmCellIncY;
} GLYPHMETRICS, *LPGLYPHMETRICS;

#define GGO_METRICS         0
#define GGO_BITMAP          1
#define GGO_NATIVE          2
#define GGO_BEZIER          3
#define GGO_GRAY2_BITMAP    4
#define GGO_GRAY4_BITMAP    5
#define GGO_GRAY8_BITMAP    6
#define GGO_GLYPH_INDEX     0x80

#define TT_PRIM_LINE    1
#define TT_PRIM_QSPLINE 2
#define TT_PRIM_CSPLINE 3
#define TT_POLYGON_TYPE 24

typedef struct
{
     W32FIXED  eM11;
     W32FIXED  eM12;
     W32FIXED  eM21;
     W32FIXED  eM22;
} MAT2, *LPMAT2;

#endif // _WINGDI_
