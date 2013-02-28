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



#ifndef _ENHANCEDCUSTOMSHAPEGEOMETRY_HXX
#define _ENHANCEDCUSTOMSHAPEGEOMETRY_HXX

#include <sal/types.h>
#include <svx/msdffdef.hxx>

struct SvxMSDffVertPair
{
    sal_Int32   nValA;
    sal_Int32   nValB;
};
struct SvxMSDffCalculationData
{
    sal_uInt16  nFlags;
    sal_Int32   nVal[ 3 ];
};
struct SvxMSDffTextRectangles
{
    SvxMSDffVertPair    nPairA;
    SvxMSDffVertPair    nPairB;
};

#define MSDFF_HANDLE_FLAGS_MIRRORED_X                   0x0001
#define MSDFF_HANDLE_FLAGS_MIRRORED_Y                   0x0002
#define MSDFF_HANDLE_FLAGS_SWITCHED                     0x0004
#define MSDFF_HANDLE_FLAGS_POLAR                        0x0008
#define MSDFF_HANDLE_FLAGS_MAP                          0x0010
#define MSDFF_HANDLE_FLAGS_RANGE                        0x0020
#define MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL       0x0080
#define MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL       0x0100
#define MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL       0x0200
#define MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL       0x0400
#define MSDFF_HANDLE_FLAGS_CENTER_X_IS_SPECIAL          0x0800
#define MSDFF_HANDLE_FLAGS_CENTER_Y_IS_SPECIAL          0x1000
#define MSDFF_HANDLE_FLAGS_RADIUS_RANGE                 0x2000

struct SvxMSDffHandle
{
    sal_uInt32  nFlags;
    sal_Int32   nPositionX, nPositionY, nCenterX, nCenterY, nRangeXMin, nRangeXMax, nRangeYMin, nRangeYMax;
};
struct mso_CustomShape
{
    SvxMSDffVertPair*                       pVertices;
    sal_uInt32                              nVertices;
    sal_uInt16*                             pElements;
    sal_uInt32                              nElements;
    SvxMSDffCalculationData*                pCalculation;
    sal_uInt32                              nCalculation;
    sal_Int32*                              pDefData;
    SvxMSDffTextRectangles*                 pTextRect;
    sal_uInt32                              nTextRect;
    sal_Int32                               nCoordWidth;
    sal_Int32                               nCoordHeight;
    sal_Int32                               nXRef;
    sal_Int32                               nYRef;
    SvxMSDffVertPair*                       pGluePoints;
    sal_uInt32                              nGluePoints;
    SvxMSDffHandle*                         pHandles;
    sal_uInt32                              nHandles;
};

// 0x80000000 is used as magic constant all over the customshape code where
// it is squeezed into signed int32 type that cannot preserve this value. Good
// compilers notice the problem and have every right to complain or error-out.
// Using the magic constant already casted down consolidates the problem and is
// the least invasive way to fix it until the code gets its well deserved makeover.
#define Mx80000000 static_cast<sal_Int32>(0x80000000)
#define MSO_I | Mx80000000

sal_Bool            SortFilledObjectsToBackByDefault( MSO_SPT eSpType );
SVX_DLLPUBLIC sal_Bool          IsCustomShapeFilledByDefault( MSO_SPT eSpType );
SVX_DLLPUBLIC sal_Int16         GetCustomShapeConnectionTypeDefault( MSO_SPT eSpType );

// #i28269#
SVX_DLLPUBLIC sal_Bool          IsCustomShapeStrokedByDefault( MSO_SPT eSpType );

SVX_DLLPUBLIC const mso_CustomShape*    GetCustomShapeContent( MSO_SPT eSpType );

#endif
