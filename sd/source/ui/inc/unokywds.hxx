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
inline const char sUNO_PseudoSheet_Background[] = "background";

// SdLayer
inline const char sUNO_LayerName_background[] = "background";
inline const char sUNO_LayerName_background_objects[] = "backgroundobjects";
inline const char sUNO_LayerName_layout[] = "layout";
inline const char sUNO_LayerName_controls[] = "controls";
inline const char sUNO_LayerName_measurelines[] = "measurelines";

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
inline const char sUNO_View_SnapLinesDrawing[] = "SnapLinesDrawing";
inline const char sUNO_View_SnapLinesNotes[] = "SnapLinesNotes";
inline const char sUNO_View_SnapLinesHandout[] = "SnapLinesHandout";
inline const char sUNO_View_RulerIsVisible[] = "RulerIsVisible";
inline const char sUNO_View_PageKind[] = "PageKind";
inline const char sUNO_View_SelectedPage[] = "SelectedPage";
inline const char sUNO_View_IsLayerMode[] = "IsLayerMode";
inline const char sUNO_View_IsDoubleClickTextEdit[] = "IsDoubleClickTextEdit";
inline const char sUNO_View_IsClickChangeRotation[] = "IsClickChangeRotation";
inline const char sUNO_View_SlidesPerRow[] = "SlidesPerRow";
inline const char sUNO_View_EditMode[] = "EditMode";
inline const char sUNO_View_EditModeStandard[] = "EditModeStandard"; // To be deprecated
// inline const char sUNO_View_EditModeNotes[] = "EditModeNotes";
// inline const char sUNO_View_EditModeHandout[] = "EditModeHandout";

inline const char sUNO_View_GridIsVisible[] = "GridIsVisible";
inline const char sUNO_View_GridIsFront[] = "GridIsFront";
inline const char sUNO_View_IsSnapToGrid[] = "IsSnapToGrid";
inline const char sUNO_View_IsSnapToPageMargins[] = "IsSnapToPageMargins";
inline const char sUNO_View_IsSnapToSnapLines[] = "IsSnapToSnapLines";
inline const char sUNO_View_IsSnapToObjectFrame[] = "IsSnapToObjectFrame";
inline const char sUNO_View_IsSnapToObjectPoints[] = "IsSnapToObjectPoints";
inline const char sUNO_View_IsPlusHandlesAlwaysVisible[] = "IsPlusHandlesAlwaysVisible";
inline const char sUNO_View_IsFrameDragSingles[] = "IsFrameDragSingles";
inline const char sUNO_View_EliminatePolyPointLimitAngle[] = "EliminatePolyPointLimitAngle";
inline const char sUNO_View_IsEliminatePolyPoints[] = "IsEliminatePolyPoints";
inline const char sUNO_View_ActiveLayer[] = "ActiveLayer";
inline const char sUNO_View_NoAttribs[] = "NoAttribs";
inline const char sUNO_View_NoColors[] = "NoColors";
inline const char sUNO_View_GridCoarseWidth[] = "GridCoarseWidth";
inline const char sUNO_View_GridCoarseHeight[] = "GridCoarseHeight";
inline const char sUNO_View_GridFineWidth[] = "GridFineWidth";
inline const char sUNO_View_GridFineHeight[] = "GridFineHeight";
inline const char sUNO_View_IsAngleSnapEnabled[] = "IsAngleSnapEnabled";
inline const char sUNO_View_SnapAngle[] = "SnapAngle";
inline const char sUNO_View_GridSnapWidthXNumerator[] = "GridSnapWidthXNumerator";
inline const char sUNO_View_GridSnapWidthXDenominator[] = "GridSnapWidthXDenominator";
inline const char sUNO_View_GridSnapWidthYNumerator[] = "GridSnapWidthYNumerator";
inline const char sUNO_View_GridSnapWidthYDenominator[] = "GridSnapWidthYDenominator";
inline const char sUNO_View_VisibleLayers[] = "VisibleLayers";
inline const char sUNO_View_PrintableLayers[] = "PrintableLayers";
inline const char sUNO_View_LockedLayers[] = "LockedLayers";

inline const char sUNO_View_VisibleAreaTop[] = "VisibleAreaTop";
inline const char sUNO_View_VisibleAreaLeft[] = "VisibleAreaLeft";
inline const char sUNO_View_VisibleAreaWidth[] = "VisibleAreaWidth";
inline const char sUNO_View_VisibleAreaHeight[] = "VisibleAreaHeight";

inline const char sUNO_View_ZoomOnPage[] = "ZoomOnPage";
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
