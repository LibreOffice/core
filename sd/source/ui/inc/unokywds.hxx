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

#ifndef INCLUDED_SD_SOURCE_UI_INC_UNOKYWDS_HXX
#define INCLUDED_SD_SOURCE_UI_INC_UNOKYWDS_HXX

#include <sal/config.h>

// SdUnoPseudoStyleFamily
constexpr OUStringLiteral sUNO_PseudoSheet_Background = u"background";

// SdLayer
constexpr OUStringLiteral sUNO_LayerName_background = u"background";
constexpr OUStringLiteral sUNO_LayerName_background_objects = u"backgroundobjects";
constexpr OUStringLiteral sUNO_LayerName_layout = u"layout";
constexpr OUStringLiteral sUNO_LayerName_controls = u"controls";
constexpr OUStringLiteral sUNO_LayerName_measurelines = u"measurelines";

// services
constexpr OUStringLiteral sUNO_Service_FillProperties = u"com.sun.star.drawing.FillProperties";
constexpr OUStringLiteral sUNO_Service_PageBackground = u"com.sun.star.drawing.PageBackground";
inline const char sUNO_Service_ImageMapRectangleObject[]
    = "com.sun.star.image.ImageMapRectangleObject";
inline const char sUNO_Service_ImageMapCircleObject[] = "com.sun.star.image.ImageMapCircleObject";
inline const char sUNO_Service_ImageMapPolygonObject[] = "com.sun.star.image.ImageMapPolygonObject";

// properties
inline const char16_t sUNO_Prop_ForbiddenCharacters[] = u"ForbiddenCharacters";
inline const char16_t sUNO_Prop_MapUnit[] = u"MapUnit";
inline const char16_t sUNO_Prop_VisibleArea[] = u"VisibleArea";
inline const char16_t sUNO_Prop_TabStop[] = u"TabStop";
inline const char16_t sUNO_Prop_CharLocale[] = u"CharLocale";
inline const char16_t sUNO_Prop_AutomContFocus[] = u"AutomaticControlFocus";
inline const char16_t sUNO_Prop_ApplyFrmDsgnMode[] = u"ApplyFormDesignMode";
inline const char16_t sUNO_Prop_IsBackgroundVisible[] = u"IsBackgroundVisible";
inline const char16_t sUNO_Prop_IsBackgroundObjectsVisible[] = u"IsBackgroundObjectsVisible";
inline const char16_t sUNO_Prop_UserDefinedAttributes[] = u"UserDefinedAttributes";
inline const char16_t sUNO_Prop_BookmarkURL[] = u"BookmarkURL";
inline const char16_t sUNO_Prop_RuntimeUID[] = u"RuntimeUID";
inline const char16_t sUNO_Prop_HasValidSignatures[] = u"HasValidSignatures";
inline const char16_t sUNO_Prop_InteropGrabBag[] = u"InteropGrabBag";

// view settings
inline const char sUNO_View_ViewId[] = "ViewId";
constexpr OUStringLiteral sUNO_View_SnapLinesDrawing = u"SnapLinesDrawing";
constexpr OUStringLiteral sUNO_View_SnapLinesNotes = u"SnapLinesNotes";
constexpr OUStringLiteral sUNO_View_SnapLinesHandout = u"SnapLinesHandout";
constexpr OUStringLiteral sUNO_View_RulerIsVisible = u"RulerIsVisible";
constexpr OUStringLiteral sUNO_View_PageKind = u"PageKind";
constexpr OUStringLiteral sUNO_View_SelectedPage = u"SelectedPage";
constexpr OUStringLiteral sUNO_View_IsLayerMode = u"IsLayerMode";
constexpr OUStringLiteral sUNO_View_IsDoubleClickTextEdit = u"IsDoubleClickTextEdit";
constexpr OUStringLiteral sUNO_View_IsClickChangeRotation = u"IsClickChangeRotation";
constexpr OUStringLiteral sUNO_View_SlidesPerRow = u"SlidesPerRow";
constexpr OUStringLiteral sUNO_View_EditMode = u"EditMode";
inline const char sUNO_View_EditModeStandard[] = "EditModeStandard"; // To be deprecated
// inline const char sUNO_View_EditModeNotes[] = "EditModeNotes";
// inline const char sUNO_View_EditModeHandout[] = "EditModeHandout";

constexpr OUStringLiteral sUNO_View_GridIsVisible = u"GridIsVisible";
constexpr OUStringLiteral sUNO_View_GridIsFront = u"GridIsFront";
constexpr OUStringLiteral sUNO_View_IsSnapToGrid = u"IsSnapToGrid";
constexpr OUStringLiteral sUNO_View_IsSnapToPageMargins = u"IsSnapToPageMargins";
constexpr OUStringLiteral sUNO_View_IsSnapToSnapLines = u"IsSnapToSnapLines";
constexpr OUStringLiteral sUNO_View_IsSnapToObjectFrame = u"IsSnapToObjectFrame";
constexpr OUStringLiteral sUNO_View_IsSnapToObjectPoints = u"IsSnapToObjectPoints";
constexpr OUStringLiteral sUNO_View_IsPlusHandlesAlwaysVisible = u"IsPlusHandlesAlwaysVisible";
constexpr OUStringLiteral sUNO_View_IsFrameDragSingles = u"IsFrameDragSingles";
constexpr OUStringLiteral sUNO_View_EliminatePolyPointLimitAngle = u"EliminatePolyPointLimitAngle";
constexpr OUStringLiteral sUNO_View_IsEliminatePolyPoints = u"IsEliminatePolyPoints";
inline const char sUNO_View_ActiveLayer[] = "ActiveLayer";
constexpr OUStringLiteral sUNO_View_NoAttribs = u"NoAttribs";
constexpr OUStringLiteral sUNO_View_NoColors = u"NoColors";
constexpr OUStringLiteral sUNO_View_GridCoarseWidth = u"GridCoarseWidth";
constexpr OUStringLiteral sUNO_View_GridCoarseHeight = u"GridCoarseHeight";
constexpr OUStringLiteral sUNO_View_GridFineWidth = u"GridFineWidth";
constexpr OUStringLiteral sUNO_View_GridFineHeight = u"GridFineHeight";
constexpr OUStringLiteral sUNO_View_IsAngleSnapEnabled = u"IsAngleSnapEnabled";
constexpr OUStringLiteral sUNO_View_SnapAngle = u"SnapAngle";
constexpr OUStringLiteral sUNO_View_GridSnapWidthXNumerator = u"GridSnapWidthXNumerator";
constexpr OUStringLiteral sUNO_View_GridSnapWidthXDenominator = u"GridSnapWidthXDenominator";
constexpr OUStringLiteral sUNO_View_GridSnapWidthYNumerator = u"GridSnapWidthYNumerator";
constexpr OUStringLiteral sUNO_View_GridSnapWidthYDenominator = u"GridSnapWidthYDenominator";
constexpr OUStringLiteral sUNO_View_VisibleLayers = u"VisibleLayers";
constexpr OUStringLiteral sUNO_View_PrintableLayers = u"PrintableLayers";
constexpr OUStringLiteral sUNO_View_LockedLayers = u"LockedLayers";

constexpr OUStringLiteral sUNO_View_VisibleAreaTop = u"VisibleAreaTop";
constexpr OUStringLiteral sUNO_View_VisibleAreaLeft = u"VisibleAreaLeft";
constexpr OUStringLiteral sUNO_View_VisibleAreaWidth = u"VisibleAreaWidth";
constexpr OUStringLiteral sUNO_View_VisibleAreaHeight = u"VisibleAreaHeight";

inline const char sUNO_View_ZoomOnPage[] = "ZoomOnPage";
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
