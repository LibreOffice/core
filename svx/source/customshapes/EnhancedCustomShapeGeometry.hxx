/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnhancedCustomShapeGeometry.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 16:44:12 $
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

#ifndef _ENHANCEDCUSTOMSHAPEGEOMETRY_HXX
#define _ENHANCEDCUSTOMSHAPEGEOMETRY_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _MSDFFDEF_HXX
#include <svx/msdffdef.hxx>
#endif

struct SvxMSDffVertPair
{
    sal_Int32   nValA;
    sal_Int32   nValB;
};
struct SvxMSDffCalculationData
{
    sal_uInt16  nFlags;
    sal_Int16   nVal[ 3 ];
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

#define MSO_I | (sal_Int32)0x80000000

const sal_Bool          SortFilledObjectsToBackByDefault( MSO_SPT eSpType );
const sal_Bool          IsCustomShapeFilledByDefault( MSO_SPT eSpType );
const sal_Int16         GetCustomShapeConnectionTypeDefault( MSO_SPT eSpType );

// #i28269#
const sal_Bool          IsCustomShapeStrokedByDefault( MSO_SPT eSpType );

const mso_CustomShape*  GetCustomShapeContent( MSO_SPT eSpType );

#endif
