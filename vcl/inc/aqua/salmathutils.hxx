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

// LRectCoor is an abbreviation for rectangular coordinates
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

// LRectCoorVec is an abbreviation for vectors in rectangular
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
