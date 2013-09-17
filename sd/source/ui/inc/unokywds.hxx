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

#ifndef _SD_UNOKYWDS_HXX_
#define _SD_UNOKYWDS_HXX_

#define UNO_PREFIX "com.sun.star."

// SdXImpressDocument
const char sUNO_Service_DrawingDocument[] = UNO_PREFIX "drawing.DrawingDocument";
const char sUNO_Service_PresentationDocument[] = UNO_PREFIX "drawing.PresentationDocument";

// SdUnoStyleFamilies
const char sUNO_SdUnoStyleFamilies[] = "SdUnoStyleFamilies";
const char sUNO_Graphic_Style_Family_Name[] = "graphics";

// SdUnoPseudoStyleFamily
const char sUNO_SdUnoPseudoStyleFamily[] = "SdUnoPseudoStyleFamily";
const char sUNO_PseudoSheet_Title[] = "title";
const char sUNO_PseudoSheet_SubTitle[] = "subtitle";
const char sUNO_PseudoSheet_Background[] = "background";
const char sUNO_PseudoSheet_Background_Objects[] = "backgroundobjects";
const char sUNO_PseudoSheet_Notes[] = "notes";
const char sUNO_PseudoSheet_Outline1[] = "outline1";
const char sUNO_PseudoSheet_Outline2[] = "outline2";
const char sUNO_PseudoSheet_Outline3[] = "outline3";
const char sUNO_PseudoSheet_Outline4[] = "outline4";
const char sUNO_PseudoSheet_Outline5[] = "outline5";
const char sUNO_PseudoSheet_Outline6[] = "outline6";
const char sUNO_PseudoSheet_Outline7[] = "outline7";
const char sUNO_PseudoSheet_Outline8[] = "outline8";
const char sUNO_PseudoSheet_Outline9[] = "outline9";

// SdUnoPseudoStyle
const char sUNO_SdUnoPseudoStyle[] = "SdUnoPseudoStyle";

// SdUnoGraphicStyleFamily
const char sUNO_SdUnoGraphicStyleFamily[] = "SdUnoGraphicStyleFamily";
const char sUNO_StyleName_standard[] = "standard";
const char sUNO_StyleName_objwitharrow[] = "objectwitharrow";
const char sUNO_StyleName_objwithshadow[] = "objectwithshadow";
const char sUNO_StyleName_objwithoutfill[] = "objectwithoutfill";
const char sUNO_StyleName_text[] = "text";
const char sUNO_StyleName_textbody[] = "textbody";
const char sUNO_StyleName_textbodyjust[] = "textbodyjustfied";
const char sUNO_StyleName_textbodyindent[] = "textbodyindent";
const char sUNO_StyleName_title[] = "title";
const char sUNO_StyleName_title1[] = "title1";
const char sUNO_StyleName_title2[] = "title2";
const char sUNO_StyleName_headline[] = "headline";
const char sUNO_StyleName_headline1[] = "headline1";
const char sUNO_StyleName_headline2[] = "headline2";
const char sUNO_StyleName_measure[] = "measure";

// SdUnoGraphicStyle
const char sUNO_SdUnoGraphicStyle[] = "SdUnoGraphicStyle";

// SdUnoPageBackground
const char sUNO_SdUnoPageBackground[] = "SdUnoPageBackground";

// SdLayerManager
const char sUNO_SdLayerManager[] = "SdUnoLayerManager";

// SdLayer
const char sUNO_SdLayer[] = "SdUnoLayer";
const char sUNO_LayerName_background[] = "background";
const char sUNO_LayerName_background_objects[] = "backgroundobjects";
const char sUNO_LayerName_layout[] = "layout";
const char sUNO_LayerName_controls[] = "controls";
const char sUNO_LayerName_measurelines[] = "measurelines";

// SdXShape
const char sUNO_shape_style[] = "Style";
const char sUNO_shape_layername[] = "LayerName";
const char sUNO_shape_zorder[] = "ZOrder";

// services
const char sUNO_Service_StyleFamily[] = UNO_PREFIX "style.StyleFamily";
const char sUNO_Service_StyleFamilies[] = UNO_PREFIX "style.StyleFamilies";
const char sUNO_Service_Style[] = UNO_PREFIX "style.Style";
const char sUNO_Service_FillProperties[] = UNO_PREFIX "drawing.FillProperties";
const char sUNO_Service_LineProperties[] = UNO_PREFIX "drawing.LineProperties";
const char sUNO_Service_ParagraphProperties[] = UNO_PREFIX "style.ParagraphProperties";
const char sUNO_Service_CharacterProperties[] = UNO_PREFIX "style.CharacterProperties";
const char sUNO_Service_Text[] = UNO_PREFIX "drawing.Text";
const char sUNO_Service_TextProperties[] = UNO_PREFIX "drawing.TextProperties";
const char sUNO_Service_ShadowProperties[] = UNO_PREFIX "drawing.ShadowProperties";
const char sUNO_Service_ConnectorProperties[] = UNO_PREFIX "drawing.ConnectorProperties";
const char sUNO_Service_MeasureProperties[] = UNO_PREFIX "drawing.MeasureProperties";
const char sUNO_Service_PageBackground[] = UNO_PREFIX "drawing.PageBackground";
const char sUNO_Service_DrawingLayer[] = UNO_PREFIX "drawing.Layer";
const char sUNO_Service_DrawingLayerManager[] = UNO_PREFIX "drawing.LayerManager";
const char sUNO_Service_GraphicObjectShape[] = UNO_PREFIX "drawing.GraphicObjectShape";
const char sUNO_Service_ImageMapRectangleObject[] = UNO_PREFIX "image.ImageMapRectangleObject";
const char sUNO_Service_ImageMapCircleObject[] = UNO_PREFIX "image.ImageMapCircleObject";
const char sUNO_Service_ImageMapPolygonObject[] = UNO_PREFIX "image.ImageMapPolygonObject";

// properties
const char sUNO_Prop_Background[] = "Background";
const char sUNO_Prop_ForbiddenCharacters[] = "ForbiddenCharacters";
const char sUNO_Prop_MapUnit[] = "MapUnit";
const char sUNO_Prop_VisibleArea[] = "VisibleArea";
const char sUNO_Prop_Aspect[] = "Aspect";
const char sUNO_Prop_TabStop[] = "TabStop";
const char sUNO_Prop_CharLocale[] = "CharLocale";
const char sUNO_Prop_AutomContFocus[] = "AutomaticControlFocus";
const char sUNO_Prop_ApplyFrmDsgnMode[] = "ApplyFormDesignMode";
const char sUNO_Prop_IsBackgroundVisible[] = "IsBackgroundVisible";
const char sUNO_Prop_IsBackgroundObjectsVisible[] = "IsBackgroundObjectsVisible";
const char sUNO_Prop_UserDefinedAttributes[] = "UserDefinedAttributes";
const char sUNO_Prop_BookmarkURL[] = "BookmarkURL";
const char sUNO_Prop_RuntimeUID[] = "RuntimeUID";
const char sUNO_Prop_HasValidSignatures[] = "HasValidSignatures";
const char sUNO_Prop_InteropGrabBag[] = "InteropGrabBag";

// view settings
const char sUNO_View_ViewId[] = "ViewId";
const char sUNO_View_SnapLinesDrawing[] = "SnapLinesDrawing";
const char sUNO_View_SnapLinesNotes[] = "SnapLinesNotes";
const char sUNO_View_SnapLinesHandout[] = "SnapLinesHandout";
const char sUNO_View_RulerIsVisible[] = "RulerIsVisible";
const char sUNO_View_PageKind[] = "PageKind";
const char sUNO_View_SelectedPage[] = "SelectedPage";
const char sUNO_View_IsLayerMode[] = "IsLayerMode";
const char sUNO_View_IsQuickEdit[] = "IsQuickEdit";
const char sUNO_View_IsDoubleClickTextEdit[] = "IsDoubleClickTextEdit";
const char sUNO_View_IsClickChangeRotation[] = "IsClickChangeRotation";
const char sUNO_View_IsDragWithCopy[] = "IsDragWithCopy";
const char sUNO_View_SlidesPerRow[] = "SlidesPerRow";
const char sUNO_View_DrawMode[] = "DrawMode";
const char sUNO_View_PreviewDrawMode[] = "PreviewDrawMode";
const char sUNO_View_IsShowPreviewInPageMode[] = "IsShowPreviewInPageMode";
const char sUNO_View_IsShowPreviewInMasterPageMode[] = "IsShowPreviewInMasterPageMode";
const char sUNO_View_SetShowPreviewInOutlineMode[] = "SetShowPreviewInOutlineMode";
const char sUNO_View_EditModeStandard[] = "EditModeStandard";
const char sUNO_View_EditModeNotes[] = "EditModeNotes";
const char sUNO_View_EditModeHandout[] = "EditModeHandout";
const char sUNO_View_VisArea[] = "VisArea";

const char sUNO_View_GridIsVisible[] = "GridIsVisible";
const char sUNO_View_GridIsFront[] = "GridIsFront";
const char sUNO_View_IsSnapToGrid[] = "IsSnapToGrid";
const char sUNO_View_IsSnapToPageMargins[] = "IsSnapToPageMargins";
const char sUNO_View_IsSnapToSnapLines[] = "IsSnapToSnapLines";
const char sUNO_View_IsSnapToObjectFrame[] = "IsSnapToObjectFrame";
const char sUNO_View_IsSnapToObjectPoints[] = "IsSnapToObjectPoints";
const char sUNO_View_IsSnapLinesVisible[] = "IsSnapLinesVisible";
const char sUNO_View_IsDragStripes[] = "IsDragStripes";
const char sUNO_View_IsPlusHandlesAlwaysVisible[] = "IsPlusHandlesAlwaysVisible";
const char sUNO_View_IsFrameDragSingles[] = "IsFrameDragSingles";
const char sUNO_View_IsMarkedHitMovesAlways[] = "IsMarkedHitMovesAlways";
const char sUNO_View_EliminatePolyPointLimitAngle[] = "EliminatePolyPointLimitAngle";
const char sUNO_View_IsEliminatePolyPoints[] = "IsEliminatePolyPoints";
const char sUNO_View_IsLineDraft[] = "IsLineDraft";
const char sUNO_View_IsFillDraft[] = "IsFillDraft";
const char sUNO_View_IsTextDraft[] = "IsTextDraft";
const char sUNO_View_IsGrafDraft[] = "IsGrafDraft";
const char sUNO_View_ActiveLayer[] = "ActiveLayer";
const char sUNO_View_NoAttribs[] = "NoAttribs";
const char sUNO_View_NoColors[] = "NoColors";
const char sUNO_View_GridCoarseWidth[] = "GridCoarseWidth";
const char sUNO_View_GridCoarseHeight[] = "GridCoarseHeight";
const char sUNO_View_GridFineWidth[] = "GridFineWidth";
const char sUNO_View_GridFineHeight[] = "GridFineHeight";
const char sUNO_View_IsAngleSnapEnabled[] = "IsAngleSnapEnabled";
const char sUNO_View_SnapAngle[] = "SnapAngle";
const char sUNO_View_GridSnapWidthXNumerator[] = "GridSnapWidthXNumerator";
const char sUNO_View_GridSnapWidthXDenominator[] = "GridSnapWidthXDenominator";
const char sUNO_View_GridSnapWidthYNumerator[] = "GridSnapWidthYNumerator";
const char sUNO_View_GridSnapWidthYDenominator[] = "GridSnapWidthYDenominator";
const char sUNO_View_VisibleLayers[] = "VisibleLayers";
const char sUNO_View_PrintableLayers[] = "PrintableLayers";
const char sUNO_View_LockedLayers[] = "LockedLayers";

const char sUNO_View_VisibleAreaTop[] = "VisibleAreaTop";
const char sUNO_View_VisibleAreaLeft[] = "VisibleAreaLeft";
const char sUNO_View_VisibleAreaWidth[] = "VisibleAreaWidth";
const char sUNO_View_VisibleAreaHeight[] = "VisibleAreaHeight";

const char sUNO_View_ZoomOnPage[] = "ZoomOnPage";
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
