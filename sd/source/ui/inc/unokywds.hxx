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

#include <sal/config.h>
#include <rtl/ustring.hxx>

// SdUnoPseudoStyleFamily
inline constexpr OUString sUNO_PseudoSheet_Background = u"background"_ustr;

// SdLayer
inline constexpr OUString sUNO_LayerName_background = u"background"_ustr;
inline constexpr OUString sUNO_LayerName_background_objects = u"backgroundobjects"_ustr;
inline constexpr OUString sUNO_LayerName_layout = u"layout"_ustr;
inline constexpr OUString sUNO_LayerName_controls = u"controls"_ustr;
inline constexpr OUString sUNO_LayerName_measurelines = u"measurelines"_ustr;

// services
inline constexpr OUString sUNO_Service_FillProperties = u"com.sun.star.drawing.FillProperties"_ustr;
inline constexpr OUString sUNO_Service_PageBackground = u"com.sun.star.drawing.PageBackground"_ustr;
inline constexpr OUString sUNO_Service_ImageMapRectangleObject
    = u"com.sun.star.image.ImageMapRectangleObject"_ustr;
inline constexpr OUString sUNO_Service_ImageMapCircleObject
    = u"com.sun.star.image.ImageMapCircleObject"_ustr;
inline constexpr OUString sUNO_Service_ImageMapPolygonObject
    = u"com.sun.star.image.ImageMapPolygonObject"_ustr;

// properties
inline constexpr OUString sUNO_Prop_ForbiddenCharacters = u"ForbiddenCharacters"_ustr;
inline constexpr OUString sUNO_Prop_MapUnit = u"MapUnit"_ustr;
inline constexpr OUString sUNO_Prop_VisibleArea = u"VisibleArea"_ustr;
inline constexpr OUString sUNO_Prop_TabStop = u"TabStop"_ustr;
inline constexpr OUString sUNO_Prop_CharLocale = u"CharLocale"_ustr;
inline constexpr OUString sUNO_Prop_AutomContFocus = u"AutomaticControlFocus"_ustr;
inline constexpr OUString sUNO_Prop_ApplyFrmDsgnMode = u"ApplyFormDesignMode"_ustr;
inline constexpr OUString sUNO_Prop_IsBackgroundVisible = u"IsBackgroundVisible"_ustr;
inline constexpr OUString sUNO_Prop_IsBackgroundObjectsVisible = u"IsBackgroundObjectsVisible"_ustr;
inline constexpr OUString sUNO_Prop_UserDefinedAttributes = u"UserDefinedAttributes"_ustr;
inline constexpr OUString sUNO_Prop_BookmarkURL = u"BookmarkURL"_ustr;
inline constexpr OUString sUNO_Prop_RuntimeUID = u"RuntimeUID"_ustr;
inline constexpr OUString sUNO_Prop_HasValidSignatures = u"HasValidSignatures"_ustr;
inline constexpr OUString sUNO_Prop_AllowLinkUpdate = u"AllowLinkUpdate"_ustr;
inline constexpr OUString sUNO_Prop_InteropGrabBag = u"InteropGrabBag"_ustr;
inline constexpr OUString sUNO_Prop_Theme = u"Theme"_ustr;

// view settings
inline constexpr OUString sUNO_View_ViewId = u"ViewId"_ustr;
inline constexpr OUString sUNO_View_SnapLinesDrawing = u"SnapLinesDrawing"_ustr;
inline constexpr OUString sUNO_View_SnapLinesNotes = u"SnapLinesNotes"_ustr;
inline constexpr OUString sUNO_View_SnapLinesHandout = u"SnapLinesHandout"_ustr;
inline constexpr OUString sUNO_View_RulerIsVisible = u"RulerIsVisible"_ustr;
inline constexpr OUString sUNO_View_PageKind = u"PageKind"_ustr;
inline constexpr OUString sUNO_View_SelectedPage = u"SelectedPage"_ustr;
inline constexpr OUString sUNO_View_IsLayerMode = u"IsLayerMode"_ustr;
inline constexpr OUString sUNO_View_IsDoubleClickTextEdit = u"IsDoubleClickTextEdit"_ustr;
inline constexpr OUString sUNO_View_IsClickChangeRotation = u"IsClickChangeRotation"_ustr;
inline constexpr OUString sUNO_View_SlidesPerRow = u"SlidesPerRow"_ustr;
inline constexpr OUString sUNO_View_EditMode = u"EditMode"_ustr;
inline const char sUNO_View_EditModeStandard[] = "EditModeStandard"; // To be deprecated
// inline const char sUNO_View_EditModeNotes[] = "EditModeNotes";
// inline const char sUNO_View_EditModeHandout[] = "EditModeHandout";

inline constexpr OUString sUNO_View_GridIsVisible = u"GridIsVisible"_ustr;
inline constexpr OUString sUNO_View_GridIsFront = u"GridIsFront"_ustr;
inline constexpr OUString sUNO_View_IsSnapToGrid = u"IsSnapToGrid"_ustr;
inline constexpr OUString sUNO_View_IsSnapToPageMargins = u"IsSnapToPageMargins"_ustr;
inline constexpr OUString sUNO_View_IsSnapToSnapLines = u"IsSnapToSnapLines"_ustr;
inline constexpr OUString sUNO_View_IsSnapToObjectFrame = u"IsSnapToObjectFrame"_ustr;
inline constexpr OUString sUNO_View_IsSnapToObjectPoints = u"IsSnapToObjectPoints"_ustr;
inline constexpr OUString sUNO_View_IsPlusHandlesAlwaysVisible = u"IsPlusHandlesAlwaysVisible"_ustr;
inline constexpr OUString sUNO_View_IsFrameDragSingles = u"IsFrameDragSingles"_ustr;
inline constexpr OUString sUNO_View_EliminatePolyPointLimitAngle
    = u"EliminatePolyPointLimitAngle"_ustr;
inline constexpr OUString sUNO_View_IsEliminatePolyPoints = u"IsEliminatePolyPoints"_ustr;
inline const char sUNO_View_ActiveLayer[] = "ActiveLayer";
inline constexpr OUString sUNO_View_NoAttribs = u"NoAttribs"_ustr;
inline constexpr OUString sUNO_View_NoColors = u"NoColors"_ustr;
inline constexpr OUString sUNO_View_GridCoarseWidth = u"GridCoarseWidth"_ustr;
inline constexpr OUString sUNO_View_GridCoarseHeight = u"GridCoarseHeight"_ustr;
inline constexpr OUString sUNO_View_GridFineWidth = u"GridFineWidth"_ustr;
inline constexpr OUString sUNO_View_GridFineHeight = u"GridFineHeight"_ustr;
inline constexpr OUString sUNO_View_IsAngleSnapEnabled = u"IsAngleSnapEnabled"_ustr;
inline constexpr OUString sUNO_View_SnapAngle = u"SnapAngle"_ustr;
inline constexpr OUString sUNO_View_GridSnapWidthXNumerator = u"GridSnapWidthXNumerator"_ustr;
inline constexpr OUString sUNO_View_GridSnapWidthXDenominator = u"GridSnapWidthXDenominator"_ustr;
inline constexpr OUString sUNO_View_GridSnapWidthYNumerator = u"GridSnapWidthYNumerator"_ustr;
inline constexpr OUString sUNO_View_GridSnapWidthYDenominator = u"GridSnapWidthYDenominator"_ustr;
inline constexpr OUString sUNO_View_VisibleLayers = u"VisibleLayers"_ustr;
inline constexpr OUString sUNO_View_PrintableLayers = u"PrintableLayers"_ustr;
inline constexpr OUString sUNO_View_LockedLayers = u"LockedLayers"_ustr;

inline constexpr OUString sUNO_View_VisibleAreaTop = u"VisibleAreaTop"_ustr;
inline constexpr OUString sUNO_View_VisibleAreaLeft = u"VisibleAreaLeft"_ustr;
inline constexpr OUString sUNO_View_VisibleAreaWidth = u"VisibleAreaWidth"_ustr;
inline constexpr OUString sUNO_View_VisibleAreaHeight = u"VisibleAreaHeight"_ustr;

inline constexpr OUString sUNO_View_ZoomOnPage = u"ZoomOnPage"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
