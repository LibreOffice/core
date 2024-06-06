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

enum class SdrObjKind : sal_uInt16
{
    NONE = 0, /// abstract object (SdrObject)
    Group = 1, /// object group
    Line = 2, /// line
    Rectangle = 3, /// rectangle (round corners optional)
    CircleOrEllipse = 4, /// circle, ellipse
    CircleSection = 5, /// circle section
    CircleArc = 6, /// circle arc
    CircleCut = 7, /// circle cut
    Polygon = 8, /// polygon, PolyPolygon
    PolyLine = 9, /// PolyLine
    PathLine = 10, /// open Bezier-curve
    PathFill = 11, /// closed Bezier-curve
    FreehandLine = 12, /// open free-hand line
    FreehandFill = 13, /// closed free-hand line
    Text = 16, /// text object
    TitleText = 20, /// TitleText, special text object for StarDraw
    OutlineText = 21, /// OutlineText, special text object for StarDraw
    Graphic = 22, /// foreign graphic (StarView Graphic)
    OLE2 = 23, /// OLE object
    Edge = 24, /// connector object
    Caption = 25, /// caption object
    PathPoly = 26, /// Polygon/PolyPolygon represented by SdrPathObj
    PathPolyLine = 27, /// Polyline represented by SdrPathObj
    Page = 28, /// object that represents a SdrPage
    Measure = 29, /// measurement object
    OLEPluginFrame = 31, /// continuously activated OLE (PlugIn-Frame or similar)
    UNO = 32, /// Universal Network Object packed into SvDraw object
    CustomShape = 33, /// custom shape
    Media = 34, /// media shape
    Table = 35, /// table
    Annotation = 36, /// annotation object

    OLE2Applet = 100,
    OLE2Plugin = 101,

    // engine3d, arbitrarily place at 200
    E3D_Scene = 202,
    // E3D_OBJECT_ID should not be used, it's only a helper class for E3DScene and E3DCompoundObject
    E3D_Object = 203,
    E3D_Cube = 204,
    E3D_Sphere = 205,
    E3D_Extrusion = 206,
    E3D_Lathe = 207,
    E3D_CompoundObject = 208,
    E3D_Polygon = 209,
    E3D_INVENTOR_FIRST = E3D_Scene,
    E3D_INVENTOR_LAST = E3D_Polygon,

    // for form components, arbitrarily place at 300
    FormControl = 300 + css::form::FormComponentType::CONTROL,
    FormEdit = 300 + css::form::FormComponentType::TEXTFIELD,
    FormButton = 300 + css::form::FormComponentType::COMMANDBUTTON,
    FormFixedText = 300 + css::form::FormComponentType::FIXEDTEXT,
    FormListbox = 300 + css::form::FormComponentType::LISTBOX,
    FormCheckbox = 300 + css::form::FormComponentType::CHECKBOX,
    FormCombobox = 300 + css::form::FormComponentType::COMBOBOX,
    FormRadioButton = 300 + css::form::FormComponentType::RADIOBUTTON,
    FormGroupBox = 300 + css::form::FormComponentType::GROUPBOX,
    FormGrid = 300 + css::form::FormComponentType::GRIDCONTROL,
    FormImageButton = 300 + css::form::FormComponentType::IMAGEBUTTON,
    FormFileControl = 300 + css::form::FormComponentType::FILECONTROL,
    FormDateField = 300 + css::form::FormComponentType::DATEFIELD,
    FormTimeField = 300 + css::form::FormComponentType::TIMEFIELD,
    FormNumericField = 300 + css::form::FormComponentType::NUMERICFIELD,
    FormCurrencyField = 300 + css::form::FormComponentType::CURRENCYFIELD,
    FormPatternField = 300 + css::form::FormComponentType::PATTERNFIELD,
    FormHidden = 300 + css::form::FormComponentType::HIDDENCONTROL,
    FormImageControl = 300 + css::form::FormComponentType::IMAGECONTROL,
    FormFormattedField = 300 + css::form::FormComponentType::PATTERNFIELD + 1,
    FormScrollbar = 300 + css::form::FormComponentType::PATTERNFIELD + 2,
    FormSpinButton = 300 + css::form::FormComponentType::PATTERNFIELD + 3,
    FormNavigationBar = 300 + css::form::FormComponentType::PATTERNFIELD + 4,

    // basctl, arbitrarily place at 400
    BasicDialogControl = 401,
    BasicDialogDialog = 402,
    BasicDialogPushButton = 403,
    BasicDialogRadioButton = 404,
    BasicDialogCheckbox = 405,
    BasicDialogListbox = 406,
    BasicDialogCombobox = 407,
    BasicDialogGroupBox = 408,
    BasicDialogEdit = 409,
    BasicDialogFixedText = 410,
    BasicDialogImageControl = 411,
    BasicDialogProgressbar = 412,
    BasicDialogHorizontalScrollbar = 413,
    BasicDialogVerticalScrollbar = 414,
    BasicDialogHorizontalFixedLine = 415,
    BasicDialogVerticalFixedLine = 416,
    BasicDialogDateField = 417,
    BasicDialogTimeField = 418,
    BasicDialogNumericField = 419,
    BasicDialogCurencyField = 420,
    BasicDialogFormattedField = 421,
    BasicDialogPatternField = 422,
    BasicDialogFileControl = 423,
    BasicDialogTreeControl = 424,
    BasicDialogSpinButton = 425,
    BasicDialogGridControl = 426,
    BasicDialogHyperlinkControl = 427,

    BasicDialogFormRadio = 428,
    BasicDialogFormCheck = 429,
    BasicDialogFormList = 430,
    BasicDialogFormCombo = 431,
    BasicDialogFormSpin = 432,
    BasicDialogFormVerticalScroll = 433,
    BasicDialogFormHorizontalScroll = 434,

    // reportdesign, arbitrarily place at 500
    ReportDesignFixedText = 501,
    ReportDesignImageControl = 502,
    ReportDesignFormattedField = 503,
    ReportDesignHorizontalFixedLine = 504,
    ReportDesignVerticalFixedLine = 505,
    ReportDesignSubReport = 506,

    // writer, arbitrarily place at 600
    SwFlyDrawObjIdentifier = 601,
    NewFrame = 602
};

inline constexpr bool IsInventorE3D(SdrObjKind e)
{
    return e >= SdrObjKind::E3D_INVENTOR_FIRST && e <= SdrObjKind::E3D_INVENTOR_LAST;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
