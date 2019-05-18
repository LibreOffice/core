/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_ENHANCEDCUSTOMSHAPEGEOMETRY_HXX
#define INCLUDED_SVX_ENHANCEDCUSTOMSHAPEGEOMETRY_HXX

#include <sal/types.h>
#include <svx/msdffdef.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>

struct SvxMSDffVertPair
{
    sal_Int32 const   nValA;
    sal_Int32 const   nValB;
};
struct SvxMSDffCalculationData
{
    sal_uInt16 const  nFlags;
    sal_Int32 const   nVal[ 3 ];
};
struct SvxMSDffTextRectangles
{
    SvxMSDffVertPair const    nPairA;
    SvxMSDffVertPair const    nPairB;
};

enum class SvxMSDffHandleFlags
{
    NONE                         = 0x0000,
    MIRRORED_X                   = 0x0001,
    MIRRORED_Y                   = 0x0002,
    SWITCHED                     = 0x0004,
    POLAR                        = 0x0008,
    MAP                          = 0x0010,
    RANGE                        = 0x0020,
    RANGE_X_MIN_IS_SPECIAL       = 0x0080,
    RANGE_X_MAX_IS_SPECIAL       = 0x0100,
    RANGE_Y_MIN_IS_SPECIAL       = 0x0200,
    RANGE_Y_MAX_IS_SPECIAL       = 0x0400,
    CENTER_X_IS_SPECIAL          = 0x0800,
    CENTER_Y_IS_SPECIAL          = 0x1000,
    RADIUS_RANGE                 = 0x2000,
};
namespace o3tl
{
    template<> struct typed_flags<SvxMSDffHandleFlags> : is_typed_flags<SvxMSDffHandleFlags, 0x3fff> {};
}

struct SvxMSDffHandle
{
    SvxMSDffHandleFlags const nFlags;
    sal_Int32   nPositionX, nPositionY, nCenterX, nCenterY, nRangeXMin, nRangeXMax, nRangeYMin, nRangeYMax;
};
struct mso_CustomShape
{
    SvxMSDffVertPair* const                       pVertices;
    sal_uInt32 const                              nVertices;
    sal_uInt16* const                             pElements;
    sal_uInt32 const                              nElements;
    SvxMSDffCalculationData* const                pCalculation;
    sal_uInt32 const                              nCalculation;
    sal_Int32* const                              pDefData;
    SvxMSDffTextRectangles* const                 pTextRect;
    sal_uInt32 const                              nTextRect;
    sal_Int32 const                               nCoordWidth;
    sal_Int32 const                               nCoordHeight;
    sal_Int32 const                               nXRef;
    sal_Int32 const                               nYRef;
    SvxMSDffVertPair* const                       pGluePoints;
    sal_uInt32 const                              nGluePoints;
    SvxMSDffHandle* const                         pHandles;
    sal_uInt32 const                              nHandles;
};

#define MSO_I | sal_Int32(0x80000000)

bool                        SortFilledObjectsToBackByDefault( MSO_SPT eSpType );
SVX_DLLPUBLIC bool          IsCustomShapeFilledByDefault( MSO_SPT eSpType );
SVX_DLLPUBLIC sal_Int16     GetCustomShapeConnectionTypeDefault( MSO_SPT eSpType );

// #i28269#
SVX_DLLPUBLIC bool          IsCustomShapeStrokedByDefault( MSO_SPT eSpType );

SVX_DLLPUBLIC const mso_CustomShape*    GetCustomShapeContent( MSO_SPT eSpType );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
