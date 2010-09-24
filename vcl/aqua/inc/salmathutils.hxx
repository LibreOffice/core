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

#ifndef _SV_SALMATHUTILS_HXX
#define _SV_SALMATHUTILS_HXX

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------------------
//
//                            Structures
//
// ------------------------------------------------------------------

// LRectCoor is an abreviation for rectangular coordinates
// represented as long integers

struct LRectCoor
{
    long  x;
    long  y;
    long  z;
};

// ------------------------------------------------------------------
//
//                       Type Definitions
//
// ------------------------------------------------------------------

// LRectCoorVec is an abreviation for vectors in rectangular
// coordinates represented as long integers

typedef struct LRectCoor   LRectCoor;
typedef LRectCoor         *LRectCoorVector;
typedef LRectCoorVector   *LRectCoorTensor;

// ------------------------------------------------------------------
//
//                      Function Headers
//
// ------------------------------------------------------------------

void CSwap  ( char            &rX, char            &rY );
void UCSwap ( unsigned char   &rX, unsigned char   &rY );
void SSwap  ( short           &rX, short           &rY );
void USSwap ( unsigned short  &rX, unsigned short  &rY );
void LSwap  ( long            &rX, long            &rY );
void ULSwap ( unsigned long   &rX, unsigned long   &rY );

// ------------------------------------------------------------------

unsigned long  Euclidian2Norm ( const LRectCoorVector  pVec );

// ------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif  // _SV_SALMATHUTILS_HXX
