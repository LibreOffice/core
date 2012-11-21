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

#ifndef _SVDTYPES_HXX
#define _SVDTYPES_HXX

#include <tools/solar.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//  @@@@  @@@@@  @@@@@    @@@@  @@@@@  @@@@@@ @@@@@  @@@@  @@@@@@
// @@  @@ @@  @@ @@  @@  @@  @@ @@  @@     @@ @@    @@  @@   @@
// @@     @@  @@ @@  @@  @@  @@ @@  @@     @@ @@    @@       @@
//  @@@@  @@  @@ @@@@@   @@  @@ @@@@@      @@ @@@@  @@       @@
//     @@ @@  @@ @@  @@  @@  @@ @@  @@     @@ @@    @@       @@
// @@  @@ @@  @@ @@  @@  @@  @@ @@  @@ @@  @@ @@    @@  @@   @@
//  @@@@  @@@@@  @@  @@   @@@@  @@@@@   @@@@  @@@@@  @@@@    @@
////////////////////////////////////////////////////////////////////////////////////////////////////

const sal_uInt32 SdrInventor=sal_uInt32('S')*0x00000001+
                         sal_uInt32('V')*0x00000100+
                         sal_uInt32('D')*0x00010000+
                         sal_uInt32('r')*0x01000000;

// Kommandos fuer EndCreate()
enum SdrCreateCmd {SDRCREATE_NEXTPOINT,     // Naechster Polygonpunkt, Kreissegment: Naechste Koordinate
                   SDRCREATE_NEXTOBJECT,    // Naechstes Polygon im PolyPolygon
                   SDRCREATE_FORCEEND};     // Ende erzwungen

enum SdrDragMode
{
    SDRDRAG_MOVE,       // Verschieben
    SDRDRAG_RESIZE,     // Groesse aendern
    SDRDRAG_ROTATE,     // Drehen
    SDRDRAG_MIRROR,     // Spiegeln
    SDRDRAG_SHEAR,      // Schraegstellen
    SDRDRAG_CROOK,      // Kreisfoermig verbiegen
    SDRDRAG_DISTORT,    // Freies verzerren (Rect in beliebiges Viereck transformieren)

    // new modes for interactive transparence and gradient tools
    SDRDRAG_TRANSPARENCE,
    SDRDRAG_GRADIENT,

    // new mode for interactive cropping of graphics
    SDRDRAG_CROP
};


// fuer SdrObject::ConvertToPoly()
enum SdrConvertType {SDRCONVERT_POLY,   // reines Polygon erzeugen
                     SDRCONVERT_BEZIER, // alles Beziersegmente
                     SDRCONVERT_MIXED}; // Gemischtes Objekt (optimal)

////////////////////////////////////////////////////////////////////////////////////////////////////
//  @@     @@@@  @@  @@ @@@@@ @@@@@
//  @@    @@  @@ @@  @@ @@    @@  @@
//  @@    @@  @@ @@  @@ @@    @@  @@
//  @@    @@@@@@  @@@@  @@@@  @@@@@
//  @@    @@  @@   @@   @@    @@  @@
//  @@    @@  @@   @@   @@    @@  @@
//  @@@@@ @@  @@   @@   @@@@@ @@  @@
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Bei der Identifikation eines Layer kann es vorkommen, das dieser nicht
// vorhanden ist. SdrLayerAdmin::GetLayerID(const String&) liefert
// dann diesen Wert:
#define SDRLAYER_NOTFOUND 0xFF
// Man kann diesen Wert jodoch ohne Bedenken den Methoden des SdrLayerSet
// zuwerfen, bekommt dann jedoch immer sal_False, bzw. tut die Methode nix.
////////////////////////////////////////////////////////////////////////////////////////////////////
// Typdeklaration fuer Layer-IDs
typedef sal_uInt8 SdrLayerID;
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// @@@@@   @@@@   @@@@  @@@@@         @@@@  @@@@@  @@@@@@ @@    @@  @@@@  @@@@@@
// @@  @@ @@  @@ @@  @@ @@           @@  @@ @@  @@     @@ @@    @@ @@  @@   @@
// @@  @@ @@  @@ @@     @@      @@   @@  @@ @@  @@     @@ @@    @@ @@       @@
// @@@@@  @@@@@@ @@ @@@ @@@@   @@@@  @@  @@ @@@@@      @@ @@    @@  @@@@    @@
// @@     @@  @@ @@  @@ @@      @@   @@  @@ @@  @@     @@ @@    @@     @@   @@
// @@     @@  @@ @@  @@ @@           @@  @@ @@  @@ @@  @@ @@    @@ @@  @@   @@
// @@     @@  @@  @@@@@ @@@@@         @@@@  @@@@@   @@@@  @@@@@ @@  @@@@    @@
////////////////////////////////////////////////////////////////////////////////////////////////////
enum SdrObjListKind {SDROBJLIST_UNKNOWN    =0x00,    // Unbekannt
                     // reine Objektlisten:
                     SDROBJLIST_GROUPOBJ   =0x01,    // Objektliste eines Gruppenobjekts
                     SDROBJLIST_VIRTOBJECTS=0x02,    // Liste ist die Liste der virtuellen Objekte
                     SDROBJLIST_SYMBOLTABLE=0x03,    // Liste ist die Symboltabelle
                     // Hier haben zur Not noch 12 weitere Listentypen Platz
                     // Pages:
                     SDROBJLIST_DRAWPAGE   =0x10,    // Liste ist eine Zeichenseite
                     SDROBJLIST_MASTERPAGE =0x11    // Liste ist eine Masterpage
                     // Hier haben zur Not noch 8 weitere Pagetypen Platz
};   // fuer die Surrogate reserviert

inline bool SdrIsPageKind(SdrObjListKind eK) { return eK>=SDROBJLIST_DRAWPAGE && eK<=0x1A; }
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//  @@@@@  @@@@@ @@@@@  @@@@@  @@@@  @@@@@@
//  @@  @@ @@    @@  @@ @@    @@  @@   @@
//  @@  @@ @@    @@  @@ @@    @@  @@   @@
//  @@@@@  @@@@  @@@@@  @@@@  @@@@@@   @@
//  @@  @@ @@    @@     @@    @@  @@   @@
//  @@  @@ @@    @@     @@    @@  @@   @@
//  @@  @@ @@@@@ @@     @@@@@ @@  @@   @@
////////////////////////////////////////////////////////////////////////////////////////////////////

enum SdrRepeatFunc {SDRREPFUNC_OBJ_NONE,
                    // Mit Parameter
                    SDRREPFUNC_OBJ_MOVE,         // Distanz
                    SDRREPFUNC_OBJ_RESIZE,       // Faktor, Referenz(%) bezogen auf MarkRect
                    SDRREPFUNC_OBJ_SETSNAPRECT,  // Rectangle
                    SDRREPFUNC_OBJ_SETLOGICRECT, // Rectangle
                    SDRREPFUNC_OBJ_ROTATE,       // Drehwinkel, Referenz(%) bezogen auf MarkRect
                    SDRREPFUNC_OBJ_SHEAR,        // Winkel,Faktor, Referenz(%) bezogen auf MarkRect
                    SDRREPFUNC_OBJ_CROOKROTATE,
                    SDRREPFUNC_OBJ_CROOKSLANT,
                    SDRREPFUNC_OBJ_CROOKSTRETCH,
                    SDRREPFUNC_OBJ_ALIGN,        // Hor/Vert/bBound
                    SDRREPFUNC_OBJ_SETATTRIBUTES,// ItemSet
                    SDRREPFUNC_OBJ_SETSTYLESHEET,// StyleSheet*
                    // Ohne Parameter
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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
