/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salmathutils.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:34:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
