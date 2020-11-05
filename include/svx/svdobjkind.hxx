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

#pragma once

#include <com/sun/star/form/FormComponentType.hpp>

enum SdrObjKind
{
    OBJ_NONE = 0, /// abstract object (SdrObject)
    OBJ_GRUP = 1, /// object group
    OBJ_LINE = 2, /// line
    OBJ_RECT = 3, /// rectangle (round corners optional)
    OBJ_CIRC = 4, /// circle, ellipse
    OBJ_SECT = 5, /// circle section
    OBJ_CARC = 6, /// circle arc
    OBJ_CCUT = 7, /// circle cut
    OBJ_POLY = 8, /// polygon, PolyPolygon
    OBJ_PLIN = 9, /// PolyLine
    OBJ_PATHLINE = 10, /// open Bezier-curve
    OBJ_PATHFILL = 11, /// closed Bezier-curve
    OBJ_FREELINE = 12, /// open free-hand line
    OBJ_FREEFILL = 13, /// closed free-hand line
    OBJ_SPLNLINE = 14, /// natural cubic Spline                  (ni)
    OBJ_SPLNFILL = 15, /// periodic cubic Spline                 (ni)
    OBJ_TEXT = 16, /// text object
    OBJ_TITLETEXT = 20, /// TitleText, special text object for StarDraw
    OBJ_OUTLINETEXT = 21, /// OutlineText, special text object for StarDraw
    OBJ_GRAF = 22, /// foreign graphic (StarView Graphic)
    OBJ_OLE2 = 23, /// OLE object
    OBJ_EDGE = 24, /// connector object
    OBJ_CAPTION = 25, /// caption object
    OBJ_PATHPOLY = 26, /// Polygon/PolyPolygon represented by SdrPathObj
    OBJ_PATHPLIN = 27, /// Polyline represented by SdrPathObj
    OBJ_PAGE = 28, /// object that represents a SdrPage
    OBJ_MEASURE = 29, /// measurement object
    OBJ_FRAME = 31, /// continuously activated OLE (PlugIn-Frame or similar)
    OBJ_UNO = 32, /// Universal Network Object packed into SvDraw object
    OBJ_CUSTOMSHAPE = 33, /// custom shape
    OBJ_MEDIA = 34, /// media shape
    OBJ_TABLE = 35, /// table

    OBJ_OLE2_APPLET = 100,
    OBJ_OLE2_PLUGIN = 101,

    // engine3d, arbitrarily place at 200
    E3D_SCENE_ID = 202,
    // E3D_OBJECT_ID should not be used, it's only a helper class for E3DScene and E3DCompoundObject
    E3D_OBJECT_ID = 203,
    E3D_CUBEOBJ_ID = 204,
    E3D_SPHEREOBJ_ID = 205,
    E3D_EXTRUDEOBJ_ID = 206,
    E3D_LATHEOBJ_ID = 207,
    E3D_COMPOUNDOBJ_ID = 208,
    E3D_POLYGONOBJ_ID = 209,

    // for form components, arbitrarily place at 300
    OBJ_FM_CONTROL = 300 + css::form::FormComponentType::CONTROL,
    OBJ_FM_EDIT = 300 + css::form::FormComponentType::TEXTFIELD,
    OBJ_FM_BUTTON = 300 + css::form::FormComponentType::COMMANDBUTTON,
    OBJ_FM_FIXEDTEXT = 300 + css::form::FormComponentType::FIXEDTEXT,
    OBJ_FM_LISTBOX = 300 + css::form::FormComponentType::LISTBOX,
    OBJ_FM_CHECKBOX = 300 + css::form::FormComponentType::CHECKBOX,
    OBJ_FM_COMBOBOX = 300 + css::form::FormComponentType::COMBOBOX,
    OBJ_FM_RADIOBUTTON = 300 + css::form::FormComponentType::RADIOBUTTON,
    OBJ_FM_GROUPBOX = 300 + css::form::FormComponentType::GROUPBOX,
    OBJ_FM_GRID = 300 + css::form::FormComponentType::GRIDCONTROL,
    OBJ_FM_IMAGEBUTTON = 300 + css::form::FormComponentType::IMAGEBUTTON,
    OBJ_FM_FILECONTROL = 300 + css::form::FormComponentType::FILECONTROL,
    OBJ_FM_DATEFIELD = 300 + css::form::FormComponentType::DATEFIELD,
    OBJ_FM_TIMEFIELD = 300 + css::form::FormComponentType::TIMEFIELD,
    OBJ_FM_NUMERICFIELD = 300 + css::form::FormComponentType::NUMERICFIELD,
    OBJ_FM_CURRENCYFIELD = 300 + css::form::FormComponentType::CURRENCYFIELD,
    OBJ_FM_PATTERNFIELD = 300 + css::form::FormComponentType::PATTERNFIELD,
    OBJ_FM_HIDDEN = 300 + css::form::FormComponentType::HIDDENCONTROL,
    OBJ_FM_IMAGECONTROL = 300 + css::form::FormComponentType::IMAGECONTROL,
    OBJ_FM_FORMATTEDFIELD = 300 + css::form::FormComponentType::PATTERNFIELD + 1,
    OBJ_FM_SCROLLBAR = 300 + css::form::FormComponentType::PATTERNFIELD + 2,
    OBJ_FM_SPINBUTTON = 300 + css::form::FormComponentType::PATTERNFIELD + 3,
    OBJ_FM_NAVIGATIONBAR = 300 + css::form::FormComponentType::PATTERNFIELD + 4,

    // basctl, arbitrarily place at 400
    OBJ_DLG_CONTROL = 401,
    OBJ_DLG_DIALOG = 402,
    OBJ_DLG_PUSHBUTTON = 403,
    OBJ_DLG_RADIOBUTTON = 404,
    OBJ_DLG_CHECKBOX = 405,
    OBJ_DLG_LISTBOX = 406,
    OBJ_DLG_COMBOBOX = 407,
    OBJ_DLG_GROUPBOX = 408,
    OBJ_DLG_EDIT = 409,
    OBJ_DLG_FIXEDTEXT = 410,
    OBJ_DLG_IMAGECONTROL = 411,
    OBJ_DLG_PROGRESSBAR = 412,
    OBJ_DLG_HSCROLLBAR = 413,
    OBJ_DLG_VSCROLLBAR = 414,
    OBJ_DLG_HFIXEDLINE = 415,
    OBJ_DLG_VFIXEDLINE = 416,
    OBJ_DLG_DATEFIELD = 417,
    OBJ_DLG_TIMEFIELD = 418,
    OBJ_DLG_NUMERICFIELD = 419,
    OBJ_DLG_CURRENCYFIELD = 420,
    OBJ_DLG_FORMATTEDFIELD = 421,
    OBJ_DLG_PATTERNFIELD = 422,
    OBJ_DLG_FILECONTROL = 423,
    OBJ_DLG_TREECONTROL = 424,
    OBJ_DLG_SPINBUTTON = 425,
    OBJ_DLG_GRIDCONTROL = 426,
    OBJ_DLG_HYPERLINKCONTROL = 427,

    OBJ_DLG_FORMRADIO = 428,
    OBJ_DLG_FORMCHECK = 429,
    OBJ_DLG_FORMLIST = 430,
    OBJ_DLG_FORMCOMBO = 431,
    OBJ_DLG_FORMSPIN = 432,
    OBJ_DLG_FORMVSCROLL = 433,
    OBJ_DLG_FORMHSCROLL = 434,

    // reportdesign, arbitrarily place at 500
    OBJ_RD_FIXEDTEXT = 501,
    OBJ_RD_IMAGECONTROL = 502,
    OBJ_RD_FORMATTEDFIELD = 503,
    OBJ_RD_HFIXEDLINE = 504,
    OBJ_RD_VFIXEDLINE = 505,
    OBJ_RD_SUBREPORT = 506,

    // writer, arbitrarily place at 600
    SwFlyDrawObjIdentifier = 601
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
