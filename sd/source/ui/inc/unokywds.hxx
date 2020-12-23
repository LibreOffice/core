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
inline const OUStringLiteral sUNO_PseudoSheet_Background = u"background";

// SdLayer
inline const OUStringLiteral sUNO_LayerName_background = u"background";
inline const OUStringLiteral sUNO_LayerName_background_objects = u"backgroundobjects";
inline const OUStringLiteral sUNO_LayerName_layout = u"layout";
inline const OUStringLiteral sUNO_LayerName_controls = u"controls";
inline const OUStringLiteral sUNO_LayerName_measurelines = u"measurelines";

// services
inline const char sUNO_Service_FillProperties[] = "com.sun.star.drawing.FillProperties";
inline const char sUNO_Service_PageBackground[] = "com.sun.star.drawing.PageBackground";
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
inline const OUStringLiteral sUNO_View_SnapLinesDrawing = u"SnapLinesDrawing";
inline const OUStringLiteral sUNO_View_SnapLinesNotes = u"SnapLinesNotes";
inline const OUStringLiteral sUNO_View_SnapLinesHandout = u"SnapLinesHandout";
inline const OUStringLiteral sUNO_View_RulerIsVisible = u"RulerIsVisible";
inline const OUStringLiteral sUNO_View_PageKind = u"PageKind";
inline const OUStringLiteral sUNO_View_SelectedPage = u"SelectedPage";
inline const OUStringLiteral sUNO_View_IsLayerMode = u"IsLayerMode";
inline const OUStringLiteral sUNO_View_IsDoubleClickTextEdit = u"IsDoubleClickTextEdit";
inline const OUStringLiteral sUNO_View_IsClickChangeRotation = u"IsClickChangeRotation";
inline const OUStringLiteral sUNO_View_SlidesPerRow = u"SlidesPerRow";
inline const OUStringLiteral sUNO_View_EditMode = u"EditMode";
inline const char sUNO_View_EditModeStandard[] = "EditModeStandard"; // To be deprecated
// inline const char sUNO_View_EditModeNotes[] = "EditModeNotes";
// inline const char sUNO_View_EditModeHandout[] = "EditModeHandout";

inline const OUStringLiteral sUNO_View_GridIsVisible = u"GridIsVisible";
inline const OUStringLiteral sUNO_View_GridIsFront = u"GridIsFront";
inline const OUStringLiteral sUNO_View_IsSnapToGrid = u"IsSnapToGrid";
inline const OUStringLiteral sUNO_View_IsSnapToPageMargins = u"IsSnapToPageMargins";
inline const OUStringLiteral sUNO_View_IsSnapToSnapLines = u"IsSnapToSnapLines";
inline const OUStringLiteral sUNO_View_IsSnapToObjectFrame = u"IsSnapToObjectFrame";
inline const OUStringLiteral sUNO_View_IsSnapToObjectPoints = u"IsSnapToObjectPoints";
inline const OUStringLiteral sUNO_View_IsPlusHandlesAlwaysVisible = u"IsPlusHandlesAlwaysVisible";
inline const OUStringLiteral sUNO_View_IsFrameDragSingles = u"IsFrameDragSingles";
inline const OUStringLiteral sUNO_View_EliminatePolyPointLimitAngle
    = u"EliminatePolyPointLimitAngle";
inline const OUStringLiteral sUNO_View_IsEliminatePolyPoints = u"IsEliminatePolyPoints";
inline const char sUNO_View_ActiveLayer[] = "ActiveLayer";
inline const OUStringLiteral sUNO_View_NoAttribs = u"NoAttribs";
inline const OUStringLiteral sUNO_View_NoColors = u"NoColors";
inline const OUStringLiteral sUNO_View_GridCoarseWidth = u"GridCoarseWidth";
inline const OUStringLiteral sUNO_View_GridCoarseHeight = u"GridCoarseHeight";
inline const OUStringLiteral sUNO_View_GridFineWidth = u"GridFineWidth";
inline const OUStringLiteral sUNO_View_GridFineHeight = u"GridFineHeight";
inline const OUStringLiteral sUNO_View_IsAngleSnapEnabled = u"IsAngleSnapEnabled";
inline const OUStringLiteral sUNO_View_SnapAngle = u"SnapAngle";
inline const OUStringLiteral sUNO_View_GridSnapWidthXNumerator = u"GridSnapWidthXNumerator";
inline const OUStringLiteral sUNO_View_GridSnapWidthXDenominator = u"GridSnapWidthXDenominator";
inline const OUStringLiteral sUNO_View_GridSnapWidthYNumerator = u"GridSnapWidthYNumerator";
inline const OUStringLiteral sUNO_View_GridSnapWidthYDenominator = u"GridSnapWidthYDenominator";
inline const OUStringLiteral sUNO_View_VisibleLayers = u"VisibleLayers";
inline const OUStringLiteral sUNO_View_PrintableLayers = u"PrintableLayers";
inline const OUStringLiteral sUNO_View_LockedLayers = u"LockedLayers";

inline const OUStringLiteral sUNO_View_VisibleAreaTop = u"VisibleAreaTop";
inline const OUStringLiteral sUNO_View_VisibleAreaLeft = u"VisibleAreaLeft";
inline const OUStringLiteral sUNO_View_VisibleAreaWidth = u"VisibleAreaWidth";
inline const OUStringLiteral sUNO_View_VisibleAreaHeight = u"VisibleAreaHeight";

inline const char sUNO_View_ZoomOnPage[] = "ZoomOnPage";
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
