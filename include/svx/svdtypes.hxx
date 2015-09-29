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

/*
 * SdrObject
 */
const sal_uInt32 SdrInventor=sal_uInt32('S')*0x00000001+
                         sal_uInt32('V')*0x00000100+
                         sal_uInt32('D')*0x00010000+
                         sal_uInt32('r')*0x01000000;

// commands for EndCreate()
enum SdrCreateCmd {SDRCREATE_NEXTPOINT,     // next traverse station, segment of a circle: next coordinate
                   SDRCREATE_NEXTOBJECT,    // next polygon in PolyPolygon
                   SDRCREATE_FORCEEND};     // forced end

enum SdrDragMode
{
    SDRDRAG_MOVE,
    SDRDRAG_RESIZE,
    SDRDRAG_ROTATE,
    SDRDRAG_MIRROR,
    SDRDRAG_SHEAR,
    SDRDRAG_CROOK,
    SDRDRAG_DISTORT,    // Free deform (transform Rect in arbitrary quad)

    // new modes for interactive transparence and gradient tools
    SDRDRAG_TRANSPARENCE,
    SDRDRAG_GRADIENT,

    // new mode for interactive cropping of graphics
    SDRDRAG_CROP
};


// fuer SdrObject::ConvertToPoly()
enum SdrConvertType {SDRCONVERT_POLY,   // create pure polygon
                     SDRCONVERT_BEZIER, // Beziersegment
                     SDRCONVERT_MIXED}; // mixed object (optimal)

/*
 * Layer
 */
// If there is no layer when it should be identified, then
// drLayerAdmin::GetLayerID(const String&) returns a value.
#define SDRLAYER_NOTFOUND 0xFF
// You can use this value in the methods of SdrLayerSet, but sal_False is returned
// every time or the methode does nothing.
// type deklaration for Layer-IDs
typedef sal_uInt8 SdrLayerID;

/*
 * Page + ObjList
 */
enum SdrObjListKind {SDROBJLIST_UNKNOWN    =0x00,
                     // reine Objektlisten:
                     SDROBJLIST_GROUPOBJ   =0x01,
                     SDROBJLIST_VIRTOBJECTS=0x02,
                     SDROBJLIST_SYMBOLTABLE=0x03,
                     // Here is space for 12 more types of lists
                     // Pages:
                     SDROBJLIST_DRAWPAGE   =0x10,
                     SDROBJLIST_MASTERPAGE =0x11
                     // Here is space for 8 more types of pages
};   // reserved for Surrogate

/*
 * Repeat
 */
enum SdrRepeatFunc {SDRREPFUNC_OBJ_NONE,
                    // with parameter
                    SDRREPFUNC_OBJ_MOVE,         // distance
                    SDRREPFUNC_OBJ_RESIZE,       // factor, reference(%) related to MarkRect
                    SDRREPFUNC_OBJ_SETSNAPRECT,  // rectangle
                    SDRREPFUNC_OBJ_SETLOGICRECT, // rectangle
                    SDRREPFUNC_OBJ_ROTATE,       // rotation angle, reference(%) related to MarkRect
                    SDRREPFUNC_OBJ_SHEAR,        // angle, factor, reference(%) related to MarkRect
                    SDRREPFUNC_OBJ_CROOKROTATE,
                    SDRREPFUNC_OBJ_CROOKSLANT,
                    SDRREPFUNC_OBJ_CROOKSTRETCH,
                    SDRREPFUNC_OBJ_ALIGN,        // Hor/Vert/bBound
                    SDRREPFUNC_OBJ_SETATTRIBUTES,// ItemSet
                    SDRREPFUNC_OBJ_SETSTYLESHEET,// StyleSheet*
                    // Without parameter
                    SDRREPFUNC_OBJ_DELETE,
                    SDRREPFUNC_OBJ_COMBINE_POLYPOLY,
                    SDRREPFUNC_OBJ_COMBINE_ONEPOLY,
                    SDRREPFUNC_OBJ_DISMANTLE_POLYS,
                    SDRREPFUNC_OBJ_DISMANTLE_LINES,
                    SDRREPFUNC_OBJ_CONVERTTOPOLY,
                    SDRREPFUNC_OBJ_CONVERTTOPATH,
                    SDRREPFUNC_OBJ_GROUP,
                    SDRREPFUNC_OBJ_UNGROUP,
                    SDRREPFUNC_OBJ_PUTTOTOP,
                    SDRREPFUNC_OBJ_PUTTOBTM,
                    SDRREPFUNC_OBJ_MOVTOTOP,
                    SDRREPFUNC_OBJ_MOVTOBTM,
                    SDRREPFUNC_OBJ_REVORDER,
                    SDRREPFUNC_OBJ_IMPORTMTF,
                    SDRREPFUNC_PAGE_DELETE,
                    SDRREPFUNC_LAST};

#endif // INCLUDED_SVX_SVDTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
