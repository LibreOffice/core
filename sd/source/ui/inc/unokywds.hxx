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

// SdUnoPseudoStyleFamily
inline constexpr OUStringLiteral sUNO_PseudoSheet_Background = u"background";

// SdLayer
inline constexpr OUStringLiteral sUNO_LayerName_background = u"background";
inline constexpr OUStringLiteral sUNO_LayerName_background_objects = u"backgroundobjects";
inline constexpr OUStringLiteral sUNO_LayerName_layout = u"layout";
inline constexpr OUStringLiteral sUNO_LayerName_controls = u"controls";
inline constexpr OUStringLiteral sUNO_LayerName_measurelines = u"measurelines";

// services
inline constexpr OUStringLiteral sUNO_Service_FillProperties
    = u"com.sun.star.drawing.FillProperties";
inline constexpr OUStringLiteral sUNO_Service_PageBackground
    = u"com.sun.star.drawing.PageBackground";
inline constexpr OUStringLiteral sUNO_Service_ImageMapRectangleObject
    = u"com.sun.star.image.ImageMapRectangleObject";
inline constexpr OUStringLiteral sUNO_Service_ImageMapCircleObject
    = u"com.sun.star.image.ImageMapCircleObject";
inline constexpr OUStringLiteral sUNO_Service_ImageMapPolygonObject
    = u"com.sun.star.image.ImageMapPolygonObject";

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
inline constexpr OUStringLiteral sUNO_View_ViewId = u"ViewId";
inline constexpr OUStringLiteral sUNO_View_SnapLinesDrawing = u"SnapLinesDrawing";
inline constexpr OUStringLiteral sUNO_View_SnapLinesNotes = u"SnapLinesNotes";
inline constexpr OUStringLiteral sUNO_View_SnapLinesHandout = u"SnapLinesHandout";
inline constexpr OUStringLiteral sUNO_View_RulerIsVisible = u"RulerIsVisible";
inline constexpr OUStringLiteral sUNO_View_PageKind = u"PageKind";
inline constexpr OUStringLiteral sUNO_View_SelectedPage = u"SelectedPage";
inline constexpr OUStringLiteral sUNO_View_IsLayerMode = u"IsLayerMode";
inline constexpr OUStringLiteral sUNO_View_IsDoubleClickTextEdit = u"IsDoubleClickTextEdit";
inline constexpr OUStringLiteral sUNO_View_IsClickChangeRotation = u"IsClickChangeRotation";
inline constexpr OUStringLiteral sUNO_View_SlidesPerRow = u"SlidesPerRow";
inline constexpr OUStringLiteral sUNO_View_EditMode = u"EditMode";
inline const char sUNO_View_EditModeStandard[] = "EditModeStandard"; // To be deprecated
// inline const char sUNO_View_EditModeNotes[] = "EditModeNotes";
// inline const char sUNO_View_EditModeHandout[] = "EditModeHandout";

inline constexpr OUStringLiteral sUNO_View_GridIsVisible = u"GridIsVisible";
inline constexpr OUStringLiteral sUNO_View_GridIsFront = u"GridIsFront";
inline constexpr OUStringLiteral sUNO_View_IsSnapToGrid = u"IsSnapToGrid";
inline constexpr OUStringLiteral sUNO_View_IsSnapToPageMargins = u"IsSnapToPageMargins";
inline constexpr OUStringLiteral sUNO_View_IsSnapToSnapLines = u"IsSnapToSnapLines";
inline constexpr OUStringLiteral sUNO_View_IsSnapToObjectFrame = u"IsSnapToObjectFrame";
inline constexpr OUStringLiteral sUNO_View_IsSnapToObjectPoints = u"IsSnapToObjectPoints";
inline constexpr OUStringLiteral sUNO_View_IsPlusHandlesAlwaysVisible
    = u"IsPlusHandlesAlwaysVisible";
inline constexpr OUStringLiteral sUNO_View_IsFrameDragSingles = u"IsFrameDragSingles";
inline constexpr OUStringLiteral sUNO_View_EliminatePolyPointLimitAngle
    = u"EliminatePolyPointLimitAngle";
inline constexpr OUStringLiteral sUNO_View_IsEliminatePolyPoints = u"IsEliminatePolyPoints";
inline const char sUNO_View_ActiveLayer[] = "ActiveLayer";
inline constexpr OUStringLiteral sUNO_View_NoAttribs = u"NoAttribs";
inline constexpr OUStringLiteral sUNO_View_NoColors = u"NoColors";
inline constexpr OUStringLiteral sUNO_View_GridCoarseWidth = u"GridCoarseWidth";
inline constexpr OUStringLiteral sUNO_View_GridCoarseHeight = u"GridCoarseHeight";
inline constexpr OUStringLiteral sUNO_View_GridFineWidth = u"GridFineWidth";
inline constexpr OUStringLiteral sUNO_View_GridFineHeight = u"GridFineHeight";
inline constexpr OUStringLiteral sUNO_View_IsAngleSnapEnabled = u"IsAngleSnapEnabled";
inline constexpr OUStringLiteral sUNO_View_SnapAngle = u"SnapAngle";
inline constexpr OUStringLiteral sUNO_View_GridSnapWidthXNumerator = u"GridSnapWidthXNumerator";
inline constexpr OUStringLiteral sUNO_View_GridSnapWidthXDenominator = u"GridSnapWidthXDenominator";
inline constexpr OUStringLiteral sUNO_View_GridSnapWidthYNumerator = u"GridSnapWidthYNumerator";
inline constexpr OUStringLiteral sUNO_View_GridSnapWidthYDenominator = u"GridSnapWidthYDenominator";
inline constexpr OUStringLiteral sUNO_View_VisibleLayers = u"VisibleLayers";
inline constexpr OUStringLiteral sUNO_View_PrintableLayers = u"PrintableLayers";
inline constexpr OUStringLiteral sUNO_View_LockedLayers = u"LockedLayers";

inline constexpr OUStringLiteral sUNO_View_VisibleAreaTop = u"VisibleAreaTop";
inline constexpr OUStringLiteral sUNO_View_VisibleAreaLeft = u"VisibleAreaLeft";
inline constexpr OUStringLiteral sUNO_View_VisibleAreaWidth = u"VisibleAreaWidth";
inline constexpr OUStringLiteral sUNO_View_VisibleAreaHeight = u"VisibleAreaHeight";

inline constexpr OUStringLiteral sUNO_View_ZoomOnPage = u"ZoomOnPage";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
