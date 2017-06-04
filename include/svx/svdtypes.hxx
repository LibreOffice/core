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

#ifndef INCLUDED_SVX_SVDTYPES_HXX
#define INCLUDED_SVX_SVDTYPES_HXX

#include <sal/types.h>
#include <o3tl/strong_int.hxx>

// commands for EndCreate()
enum class SdrCreateCmd
{
    NextPoint,     // next traverse station, segment of a circle: next coordinate
    NextObject,    // next polygon in PolyPolygon
    ForceEnd       // forced end
};

enum class SdrDragMode
{
    Move,
    Resize,
    Rotate,
    Mirror,
    Shear,
    Crook,

    // new modes for interactive transparence and gradient tools
    Transparence,
    Gradient,

    // new mode for interactive cropping of graphics
    Crop
};



/*
 * Layer
 */
// You can use this value in the methods of SdrLayerSet, but false is returned
// every time or the method does nothing.
// type declaration for Layer-IDs
struct SdrLayerIDTag {};
typedef o3tl::strong_int<sal_uInt8,SdrLayerIDTag> SdrLayerID;

// If there is no layer when it should be identified, then
// SdrLayerAdmin::GetLayerID(const String&) returns a value.
constexpr SdrLayerID SDRLAYER_NOTFOUND(0xff);

/*
 * Page + ObjList
 */
enum class SdrObjListKind {
    Unknown    = 0x00,
    // pure object lists:
    GroupObj   = 0x01,
    // Here is space for 12 more types of lists
    // Pages:
    DrawPage   = 0x10,
    MasterPage = 0x11
    // Here is space for 8 more types of pages
};   // reserved for Surrogate

/*
 * Repeat
 */
enum class SdrRepeatFunc {
    NONE,
    // with parameter
    Move,         // distance
    Resize,       // factor, reference(%) related to MarkRect
    Rotate,       // rotation angle, reference(%) related to MarkRect
    // Without parameter
    Delete,
    CombinePolyPoly,
    CombineOnePoly,
    DismantlePolys,
    DismantleLines,
    ConvertToPoly,
    ConvertToPath,
    Group,
    Ungroup,
    PutToTop,
    PutToBottom,
    MoveToTop,
    MoveToBottom,
    ReverseOrder,
    ImportMtf,
};

#endif // INCLUDED_SVX_SVDTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
