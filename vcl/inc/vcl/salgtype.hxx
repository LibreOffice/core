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



#ifndef _SV_SALGTYPE_HXX
#define _SV_SALGTYPE_HXX

#include <vcl/sv.h>

// ------------
// - SalColor -
// ------------

typedef sal_uInt32 SalColor;
#define MAKE_SALCOLOR( r, g, b )    ((SalColor)(((sal_uInt32)((sal_uInt8)(b))))|(((sal_uInt32)((sal_uInt8)(g)))<<8)|(((sal_uInt32)((sal_uInt8)(r)))<<16))
#define SALCOLOR_RED( n )           ((sal_uInt8)((n)>>16))
#define SALCOLOR_GREEN( n )         ((sal_uInt8)(((sal_uInt16)(n)) >> 8))
#define SALCOLOR_BLUE( n )          ((sal_uInt8)(n))
#define SALCOLOR_NONE           (~(SalColor)0)
// ------------
// - SalPoint -
// ------------

// must equal to class Point
struct SalPoint
{
    long        mnX;
    long        mnY;
};

typedef const SalPoint*   PCONSTSALPOINT;

// --------------
// - SalTwoRect -
// --------------

struct SalTwoRect
{
    long        mnSrcX;
    long        mnSrcY;
    long        mnSrcWidth;
    long        mnSrcHeight;
    long        mnDestX;
    long        mnDestY;
    long        mnDestWidth;
    long        mnDestHeight;
};

// ---------------
// - SalROPColor -
// ---------------

typedef sal_uInt16 SalROPColor;
#define SAL_ROP_0                   ((SalROPColor)0)
#define SAL_ROP_1                   ((SalROPColor)1)
#define SAL_ROP_INVERT              ((SalROPColor)2)

// -------------
// - SalInvert -
// -------------

typedef sal_uInt16 SalInvert;
#define SAL_INVERT_HIGHLIGHT        ((SalInvert)0x0001)
#define SAL_INVERT_50               ((SalInvert)0x0002)
#define SAL_INVERT_TRACKFRAME       ((SalInvert)0x0004)

#endif // _SV_SALGTYPE_HXX
