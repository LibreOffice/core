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

#include <sal/types.h>

// SdUnoPseudoStyleFamily
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Title[] = "title";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_SubTitle[] = "subtitle";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Background[] = "background";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Background_Objects[] = "backgroundobjects";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Notes[] = "notes";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Outline1[] = "outline1";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Outline2[] = "outline2";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Outline3[] = "outline3";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Outline4[] = "outline4";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Outline5[] = "outline5";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Outline6[] = "outline6";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Outline7[] = "outline7";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Outline8[] = "outline8";
SAL_INLINE_VARIABLE const char sUNO_PseudoSheet_Outline9[] = "outline9";

// SdLayer
SAL_INLINE_VARIABLE const char sUNO_LayerName_background[] = "background";
SAL_INLINE_VARIABLE const char sUNO_LayerName_background_objects[] = "backgroundobjects";
SAL_INLINE_VARIABLE const char sUNO_LayerName_layout[] = "layout";
SAL_INLINE_VARIABLE const char sUNO_LayerName_controls[] = "controls";
SAL_INLINE_VARIABLE const char sUNO_LayerName_measurelines[] = "measurelines";

// SdXShape
SAL_INLINE_VARIABLE const char sUNO_shape_style[] = "Style";
SAL_INLINE_VARIABLE const char sUNO_shape_layername[] = "LayerName";
SAL_INLINE_VARIABLE const char sUNO_shape_zorder[] = "ZOrder";

// services
SAL_INLINE_VARIABLE const char sUNO_Service_StyleFamily[] = "com.sun.star.style.StyleFamily";
SAL_INLINE_VARIABLE const char sUNO_Service_StyleFamilies[] = "com.sun.star.style.StyleFamilies";
SAL_INLINE_VARIABLE const char sUNO_Service_Style[] = "com.sun.star.style.Style";
SAL_INLINE_VARIABLE const char sUNO_Service_FillProperties[] = "com.sun.star.drawing.FillProperties";
SAL_INLINE_VARIABLE const char sUNO_Service_LineProperties[] = "com.sun.star.drawing.LineProperties";
SAL_INLINE_VARIABLE const char sUNO_Service_ParagraphProperties[] = "com.sun.star.style.ParagraphProperties";
SAL_INLINE_VARIABLE const char sUNO_Service_CharacterProperties[] = "com.sun.star.style.CharacterProperties";
SAL_INLINE_VARIABLE const char sUNO_Service_Text[] = "com.sun.star.drawing.Text";
SAL_INLINE_VARIABLE const char sUNO_Service_TextProperties[] = "com.sun.star.drawing.TextProperties";
SAL_INLINE_VARIABLE const char sUNO_Service_ShadowProperties[] = "com.sun.star.drawing.ShadowProperties";
SAL_INLINE_VARIABLE const char sUNO_Service_ConnectorProperties[] = "com.sun.star.drawing.ConnectorProperties";
SAL_INLINE_VARIABLE const char sUNO_Service_MeasureProperties[] = "com.sun.star.drawing.MeasureProperties";
SAL_INLINE_VARIABLE const char sUNO_Service_PageBackground[] = "com.sun.star.drawing.PageBackground";
SAL_INLINE_VARIABLE const char sUNO_Service_GraphicObjectShape[] = "com.sun.star.drawing.GraphicObjectShape";
SAL_INLINE_VARIABLE const char sUNO_Service_ImageMapRectangleObject[] = "com.sun.star.image.ImageMapRectangleObject";
SAL_INLINE_VARIABLE const char sUNO_Service_ImageMapCircleObject[] = "com.sun.star.image.ImageMapCircleObject";
SAL_INLINE_VARIABLE const char sUNO_Service_ImageMapPolygonObject[] = "com.sun.star.image.ImageMapPolygonObject";

// properties
SAL_INLINE_VARIABLE const char sUNO_Prop_Background[] = "Background";
SAL_INLINE_VARIABLE const char sUNO_Prop_ForbiddenCharacters[] = "ForbiddenCharacters";
SAL_INLINE_VARIABLE const char sUNO_Prop_MapUnit[] = "MapUnit";
SAL_INLINE_VARIABLE const char sUNO_Prop_VisibleArea[] = "VisibleArea";
SAL_INLINE_VARIABLE const char sUNO_Prop_Aspect[] = "Aspect";
SAL_INLINE_VARIABLE const char sUNO_Prop_TabStop[] = "TabStop";
SAL_INLINE_VARIABLE const char sUNO_Prop_CharLocale[] = "CharLocale";
SAL_INLINE_VARIABLE const char sUNO_Prop_AutomContFocus[] = "AutomaticControlFocus";
SAL_INLINE_VARIABLE const char sUNO_Prop_ApplyFrmDsgnMode[] = "ApplyFormDesignMode";
SAL_INLINE_VARIABLE const char sUNO_Prop_IsBackgroundVisible[] = "IsBackgroundVisible";
SAL_INLINE_VARIABLE const char sUNO_Prop_IsBackgroundObjectsVisible[] = "IsBackgroundObjectsVisible";
SAL_INLINE_VARIABLE const char sUNO_Prop_UserDefinedAttributes[] = "UserDefinedAttributes";
SAL_INLINE_VARIABLE const char sUNO_Prop_BookmarkURL[] = "BookmarkURL";
SAL_INLINE_VARIABLE const char sUNO_Prop_RuntimeUID[] = "RuntimeUID";
SAL_INLINE_VARIABLE const char sUNO_Prop_HasValidSignatures[] = "HasValidSignatures";
SAL_INLINE_VARIABLE const char sUNO_Prop_InteropGrabBag[] = "InteropGrabBag";

// view settings
SAL_INLINE_VARIABLE const char sUNO_View_ViewId[] = "ViewId";
SAL_INLINE_VARIABLE const char sUNO_View_SnapLinesDrawing[] = "SnapLinesDrawing";
SAL_INLINE_VARIABLE const char sUNO_View_SnapLinesNotes[] = "SnapLinesNotes";
SAL_INLINE_VARIABLE const char sUNO_View_SnapLinesHandout[] = "SnapLinesHandout";
SAL_INLINE_VARIABLE const char sUNO_View_RulerIsVisible[] = "RulerIsVisible";
SAL_INLINE_VARIABLE const char sUNO_View_PageKind[] = "PageKind";
SAL_INLINE_VARIABLE const char sUNO_View_SelectedPage[] = "SelectedPage";
SAL_INLINE_VARIABLE const char sUNO_View_IsLayerMode[] = "IsLayerMode";
SAL_INLINE_VARIABLE const char sUNO_View_IsQuickEdit[] = "IsQuickEdit";
SAL_INLINE_VARIABLE const char sUNO_View_IsDoubleClickTextEdit[] = "IsDoubleClickTextEdit";
SAL_INLINE_VARIABLE const char sUNO_View_IsClickChangeRotation[] = "IsClickChangeRotation";
SAL_INLINE_VARIABLE const char sUNO_View_IsDragWithCopy[] = "IsDragWithCopy";
SAL_INLINE_VARIABLE const char sUNO_View_SlidesPerRow[] = "SlidesPerRow";
SAL_INLINE_VARIABLE const char sUNO_View_DrawMode[] = "DrawMode";
SAL_INLINE_VARIABLE const char sUNO_View_PreviewDrawMode[] = "PreviewDrawMode";
SAL_INLINE_VARIABLE const char sUNO_View_IsShowPreviewInPageMode[] = "IsShowPreviewInPageMode";
SAL_INLINE_VARIABLE const char sUNO_View_IsShowPreviewInMasterPageMode[] = "IsShowPreviewInMasterPageMode";
SAL_INLINE_VARIABLE const char sUNO_View_SetShowPreviewInOutlineMode[] = "SetShowPreviewInOutlineMode";
SAL_INLINE_VARIABLE const char sUNO_View_EditMode[] = "EditMode";
SAL_INLINE_VARIABLE const char sUNO_View_EditModeStandard[] = "EditModeStandard";   // To be deprecated
// SAL_INLINE_VARIABLE const char sUNO_View_EditModeNotes[] = "EditModeNotes";
// SAL_INLINE_VARIABLE const char sUNO_View_EditModeHandout[] = "EditModeHandout";
SAL_INLINE_VARIABLE const char sUNO_View_VisArea[] = "VisArea";

SAL_INLINE_VARIABLE const char sUNO_View_GridIsVisible[] = "GridIsVisible";
SAL_INLINE_VARIABLE const char sUNO_View_GridIsFront[] = "GridIsFront";
SAL_INLINE_VARIABLE const char sUNO_View_IsSnapToGrid[] = "IsSnapToGrid";
SAL_INLINE_VARIABLE const char sUNO_View_IsSnapToPageMargins[] = "IsSnapToPageMargins";
SAL_INLINE_VARIABLE const char sUNO_View_IsSnapToSnapLines[] = "IsSnapToSnapLines";
SAL_INLINE_VARIABLE const char sUNO_View_IsSnapToObjectFrame[] = "IsSnapToObjectFrame";
SAL_INLINE_VARIABLE const char sUNO_View_IsSnapToObjectPoints[] = "IsSnapToObjectPoints";
SAL_INLINE_VARIABLE const char sUNO_View_IsSnapLinesVisible[] = "IsSnapLinesVisible";
SAL_INLINE_VARIABLE const char sUNO_View_IsDragStripes[] = "IsDragStripes";
SAL_INLINE_VARIABLE const char sUNO_View_IsPlusHandlesAlwaysVisible[] = "IsPlusHandlesAlwaysVisible";
SAL_INLINE_VARIABLE const char sUNO_View_IsFrameDragSingles[] = "IsFrameDragSingles";
SAL_INLINE_VARIABLE const char sUNO_View_IsMarkedHitMovesAlways[] = "IsMarkedHitMovesAlways";
SAL_INLINE_VARIABLE const char sUNO_View_EliminatePolyPointLimitAngle[] = "EliminatePolyPointLimitAngle";
SAL_INLINE_VARIABLE const char sUNO_View_IsEliminatePolyPoints[] = "IsEliminatePolyPoints";
SAL_INLINE_VARIABLE const char sUNO_View_IsLineDraft[] = "IsLineDraft";
SAL_INLINE_VARIABLE const char sUNO_View_IsFillDraft[] = "IsFillDraft";
SAL_INLINE_VARIABLE const char sUNO_View_IsTextDraft[] = "IsTextDraft";
SAL_INLINE_VARIABLE const char sUNO_View_IsGrafDraft[] = "IsGrafDraft";
SAL_INLINE_VARIABLE const char sUNO_View_ActiveLayer[] = "ActiveLayer";
SAL_INLINE_VARIABLE const char sUNO_View_NoAttribs[] = "NoAttribs";
SAL_INLINE_VARIABLE const char sUNO_View_NoColors[] = "NoColors";
SAL_INLINE_VARIABLE const char sUNO_View_GridCoarseWidth[] = "GridCoarseWidth";
SAL_INLINE_VARIABLE const char sUNO_View_GridCoarseHeight[] = "GridCoarseHeight";
SAL_INLINE_VARIABLE const char sUNO_View_GridFineWidth[] = "GridFineWidth";
SAL_INLINE_VARIABLE const char sUNO_View_GridFineHeight[] = "GridFineHeight";
SAL_INLINE_VARIABLE const char sUNO_View_IsAngleSnapEnabled[] = "IsAngleSnapEnabled";
SAL_INLINE_VARIABLE const char sUNO_View_SnapAngle[] = "SnapAngle";
SAL_INLINE_VARIABLE const char sUNO_View_GridSnapWidthXNumerator[] = "GridSnapWidthXNumerator";
SAL_INLINE_VARIABLE const char sUNO_View_GridSnapWidthXDenominator[] = "GridSnapWidthXDenominator";
SAL_INLINE_VARIABLE const char sUNO_View_GridSnapWidthYNumerator[] = "GridSnapWidthYNumerator";
SAL_INLINE_VARIABLE const char sUNO_View_GridSnapWidthYDenominator[] = "GridSnapWidthYDenominator";
SAL_INLINE_VARIABLE const char sUNO_View_VisibleLayers[] = "VisibleLayers";
SAL_INLINE_VARIABLE const char sUNO_View_PrintableLayers[] = "PrintableLayers";
SAL_INLINE_VARIABLE const char sUNO_View_LockedLayers[] = "LockedLayers";

SAL_INLINE_VARIABLE const char sUNO_View_VisibleAreaTop[] = "VisibleAreaTop";
SAL_INLINE_VARIABLE const char sUNO_View_VisibleAreaLeft[] = "VisibleAreaLeft";
SAL_INLINE_VARIABLE const char sUNO_View_VisibleAreaWidth[] = "VisibleAreaWidth";
SAL_INLINE_VARIABLE const char sUNO_View_VisibleAreaHeight[] = "VisibleAreaHeight";

SAL_INLINE_VARIABLE const char sUNO_View_ZoomOnPage[] = "ZoomOnPage";
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
