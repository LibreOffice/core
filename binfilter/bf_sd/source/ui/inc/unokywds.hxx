/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SD_UNOKYWDS_HXX_
#define _SD_UNOKYWDS_HXX_


#include <tools/solar.h>
namespace binfilter {

#ifndef SD_DEFINE_KEYWORDS
#define SD_CONSTASCII_ACTION( n, s ) extern sal_Char __FAR_DATA n[sizeof(s)]
#else
#define SD_CONSTASCII_ACTION( n, s ) sal_Char __FAR_DATA n[sizeof(s)] = s
#endif

#define UNO_PREFIX "com.sun.star."

// SdXImpressDocument
SD_CONSTASCII_ACTION( sUNO_Service_DrawingDocument, UNO_PREFIX "drawing.DrawingDocument" );
SD_CONSTASCII_ACTION( sUNO_Service_PresentationDocument, UNO_PREFIX "drawing.PresentationDocument" );

// SdUnoStyleFamilies
SD_CONSTASCII_ACTION( sUNO_SdUnoStyleFamilies, "SdUnoStyleFamilies" );
SD_CONSTASCII_ACTION( sUNO_Graphic_Style_Family_Name, "graphics" );

// SdUnoPseudoStyleFamily
SD_CONSTASCII_ACTION( sUNO_SdUnoPseudoStyleFamily, "SdUnoPseudoStyleFamily" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Title, "title" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_SubTitle, "subtitle" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Background, "background" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Background_Objects, "backgroundobjects" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Notes, "notes" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline1, "outline1" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline2, "outline2" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline3, "outline3" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline4, "outline4" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline5, "outline5" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline6, "outline6" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline7, "outline7" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline8, "outline8" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline9, "outline9" );

// SdUnoPseudoStyle
SD_CONSTASCII_ACTION( sUNO_SdUnoPseudoStyle, "SdUnoPseudoStyle" );

// SdUnoGraphicStyleFamily
SD_CONSTASCII_ACTION( sUNO_SdUnoGraphicStyleFamily, "SdUnoGraphicStyleFamily" );
SD_CONSTASCII_ACTION( sUNO_StyleName_standard, "standard" );
SD_CONSTASCII_ACTION( sUNO_StyleName_objwitharrow, "objectwitharrow" );
SD_CONSTASCII_ACTION( sUNO_StyleName_objwithshadow, "objectwithshadow" );
SD_CONSTASCII_ACTION( sUNO_StyleName_objwithoutfill, "objectwithoutfill" );
SD_CONSTASCII_ACTION( sUNO_StyleName_text, "text" );
SD_CONSTASCII_ACTION( sUNO_StyleName_textbody, "textbody" );
SD_CONSTASCII_ACTION( sUNO_StyleName_textbodyjust, "textbodyjustfied" );
SD_CONSTASCII_ACTION( sUNO_StyleName_textbodyindent, "textbodyindent" );
SD_CONSTASCII_ACTION( sUNO_StyleName_title, "title" );
SD_CONSTASCII_ACTION( sUNO_StyleName_title1, "title1" );
SD_CONSTASCII_ACTION( sUNO_StyleName_title2, "title2" );
SD_CONSTASCII_ACTION( sUNO_StyleName_headline, "headline" );
SD_CONSTASCII_ACTION( sUNO_StyleName_headline1, "headline1" );
SD_CONSTASCII_ACTION( sUNO_StyleName_headline2, "headline2" );
SD_CONSTASCII_ACTION( sUNO_StyleName_measure, "measure" );

// SdUnoGraphicStyle
SD_CONSTASCII_ACTION( sUNO_SdUnoGraphicStyle, "SdUnoGraphicStyle" );

// SdUnoPageBackground
SD_CONSTASCII_ACTION( sUNO_SdUnoPageBackground, "SdUnoPageBackground" );

// SdLayerManager
SD_CONSTASCII_ACTION( sUNO_SdLayerManager, "SdUnoLayerManager" );

// SdLayer
SD_CONSTASCII_ACTION( sUNO_SdLayer, "SdUnoLayer" );
SD_CONSTASCII_ACTION( sUNO_LayerName_background,		 "background" );
SD_CONSTASCII_ACTION( sUNO_LayerName_background_objects, "backgroundobjects" );
SD_CONSTASCII_ACTION( sUNO_LayerName_layout,			 "layout" );
SD_CONSTASCII_ACTION( sUNO_LayerName_controls,			 "controls" );
SD_CONSTASCII_ACTION( sUNO_LayerName_measurelines,		 "measurelines" );

// SdXShape
SD_CONSTASCII_ACTION( sUNO_shape_style, "Style" );
SD_CONSTASCII_ACTION( sUNO_shape_layername, "LayerName" );
SD_CONSTASCII_ACTION( sUNO_shape_zorder, "ZOrder" );

// services
SD_CONSTASCII_ACTION( sUNO_Service_StyleFamily, UNO_PREFIX "style.StyleFamily" );
SD_CONSTASCII_ACTION( sUNO_Service_StyleFamilies, UNO_PREFIX "style.StyleFamilies" );
SD_CONSTASCII_ACTION( sUNO_Service_Style, UNO_PREFIX "style.Style" );
SD_CONSTASCII_ACTION( sUNO_Service_FillProperties, UNO_PREFIX "drawing.FillProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_LineProperties, UNO_PREFIX "drawing.LineProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_ParagraphProperties, UNO_PREFIX "style.ParagraphProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_CharacterProperties, UNO_PREFIX "style.CharacterProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_Text, UNO_PREFIX "drawing.Text" );
SD_CONSTASCII_ACTION( sUNO_Service_TextProperties, UNO_PREFIX "drawing.TextProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_ShadowProperties, UNO_PREFIX "drawing.ShadowProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_ConnectorProperties, UNO_PREFIX "drawing.ConnectorProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_MeasureProperties, UNO_PREFIX "drawing.MeasureProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_PageBackground, UNO_PREFIX "drawing.PageBackground" );
SD_CONSTASCII_ACTION( sUNO_Service_DrawingLayer, UNO_PREFIX "drawing.Layer" );
SD_CONSTASCII_ACTION( sUNO_Service_DrawingLayerManager, UNO_PREFIX "drawing.LayerManager" );
SD_CONSTASCII_ACTION( sUNO_Service_GraphicObjectShape, UNO_PREFIX "drawing.GraphicObjectShape" );
SD_CONSTASCII_ACTION( sUNO_Service_ImageMapRectangleObject, UNO_PREFIX "image.ImageMapRectangleObject" );
SD_CONSTASCII_ACTION( sUNO_Service_ImageMapCircleObject, UNO_PREFIX "image.ImageMapCircleObject" );
SD_CONSTASCII_ACTION( sUNO_Service_ImageMapPolygonObject, UNO_PREFIX "image.ImageMapPolygonObject" );

// properties
SD_CONSTASCII_ACTION( sUNO_Prop_Background,				"Background" );
SD_CONSTASCII_ACTION( sUNO_Prop_ForbiddenCharacters,	"ForbiddenCharacters" );
SD_CONSTASCII_ACTION( sUNO_Prop_MapUnit,				"MapUnit" );
SD_CONSTASCII_ACTION( sUNO_Prop_VisibleArea,			"VisibleArea" );
SD_CONSTASCII_ACTION( sUNO_Prop_TabStop,				"TabStop" );
SD_CONSTASCII_ACTION( sUNO_Prop_CharLocale,				"CharLocale" );
SD_CONSTASCII_ACTION( sUNO_Prop_AutomContFocus,			"AutomaticControlFocus" );
SD_CONSTASCII_ACTION( sUNO_Prop_ApplyFrmDsgnMode,		"ApplyFormDesignMode" );
SD_CONSTASCII_ACTION( sUNO_Prop_IsBackgroundVisible,	"IsBackgroundVisible" );
SD_CONSTASCII_ACTION( sUNO_Prop_IsBackgroundObjectsVisible,	"IsBackgroundObjectsVisible" );
SD_CONSTASCII_ACTION( sUNO_Prop_UserDefinedAttributes, "UserDefinedAttributes" );
SD_CONSTASCII_ACTION( sUNO_Prop_BookmarkURL,			"BookmarkURL" );

// view settings
SD_CONSTASCII_ACTION( sUNO_View_ViewId,							"ViewId" );
SD_CONSTASCII_ACTION( sUNO_View_SnapLinesDrawing,				"SnapLinesDrawing" );
SD_CONSTASCII_ACTION( sUNO_View_SnapLinesNotes,					"SnapLinesNotes" );
SD_CONSTASCII_ACTION( sUNO_View_SnapLinesHandout,				"SnapLinesHandout" );
SD_CONSTASCII_ACTION( sUNO_View_RulerIsVisible,					"RulerIsVisible" );
SD_CONSTASCII_ACTION( sUNO_View_PageKind,						"PageKind" );
SD_CONSTASCII_ACTION( sUNO_View_SelectedPage,					"SelectedPage" );
SD_CONSTASCII_ACTION( sUNO_View_IsLayerMode,					"IsLayerMode" );
SD_CONSTASCII_ACTION( sUNO_View_IsQuickEdit,					"IsQuickEdit" );
SD_CONSTASCII_ACTION( sUNO_View_IsBigHandles,					"IsBigHandles" );
SD_CONSTASCII_ACTION( sUNO_View_IsDoubleClickTextEdit,			"IsDoubleClickTextEdit"	);
SD_CONSTASCII_ACTION( sUNO_View_IsClickChangeRotation,			"IsClickChangeRotation" );
SD_CONSTASCII_ACTION( sUNO_View_IsDragWithCopy,					"IsDragWithCopy" );
SD_CONSTASCII_ACTION( sUNO_View_SlidesPerRow,					"SlidesPerRow" );
SD_CONSTASCII_ACTION( sUNO_View_DrawMode,						"DrawMode" );
SD_CONSTASCII_ACTION( sUNO_View_PreviewDrawMode,				"PreviewDrawMode" );
SD_CONSTASCII_ACTION( sUNO_View_IsShowPreviewInPageMode,		"IsShowPreviewInPageMode" );
SD_CONSTASCII_ACTION( sUNO_View_IsShowPreviewInMasterPageMode,	"IsShowPreviewInMasterPageMode" );
SD_CONSTASCII_ACTION( sUNO_View_SetShowPreviewInOutlineMode,	"SetShowPreviewInOutlineMode" );
SD_CONSTASCII_ACTION( sUNO_View_EditModeStandard,				"EditModeStandard" );
SD_CONSTASCII_ACTION( sUNO_View_EditModeNotes,					"EditModeNotes" );
SD_CONSTASCII_ACTION( sUNO_View_EditModeHandout,				"EditModeHandout" );
SD_CONSTASCII_ACTION( sUNO_View_VisArea,						"VisArea" );

SD_CONSTASCII_ACTION( sUNO_View_GridIsVisible,					"GridIsVisible" );
SD_CONSTASCII_ACTION( sUNO_View_GridIsFront,					"GridIsFront" );
SD_CONSTASCII_ACTION( sUNO_View_IsSnapToGrid,					"IsSnapToGrid" );
SD_CONSTASCII_ACTION( sUNO_View_IsSnapToPageMargins,			"IsSnapToPageMargins" );
SD_CONSTASCII_ACTION( sUNO_View_IsSnapToSnapLines,				"IsSnapToSnapLines" );
SD_CONSTASCII_ACTION( sUNO_View_IsSnapToObjectFrame,			"IsSnapToObjectFrame" );
SD_CONSTASCII_ACTION( sUNO_View_IsSnapToObjectPoints,			"IsSnapToObjectPoints" );
SD_CONSTASCII_ACTION( sUNO_View_IsSnapLinesVisible,				"IsSnapLinesVisible" );
SD_CONSTASCII_ACTION( sUNO_View_IsDragStripes,					"IsDragStripes" );
SD_CONSTASCII_ACTION( sUNO_View_IsPlusHandlesAlwaysVisible,		"IsPlusHandlesAlwaysVisible" );
SD_CONSTASCII_ACTION( sUNO_View_IsFrameDragSingles,				"IsFrameDragSingles" );
SD_CONSTASCII_ACTION( sUNO_View_IsMarkedHitMovesAlways,			"IsMarkedHitMovesAlways" );
SD_CONSTASCII_ACTION( sUNO_View_EliminatePolyPointLimitAngle,	"EliminatePolyPointLimitAngle" );
SD_CONSTASCII_ACTION( sUNO_View_IsEliminatePolyPoints,			"IsEliminatePolyPoints" );
SD_CONSTASCII_ACTION( sUNO_View_IsLineDraft,					"IsLineDraft" );
SD_CONSTASCII_ACTION( sUNO_View_IsFillDraft,					"IsFillDraft" );
SD_CONSTASCII_ACTION( sUNO_View_IsTextDraft,					"IsTextDraft" );
SD_CONSTASCII_ACTION( sUNO_View_IsGrafDraft,					"IsGrafDraft" );
SD_CONSTASCII_ACTION( sUNO_View_ActiveLayer,					"ActiveLayer" );
SD_CONSTASCII_ACTION( sUNO_View_NoAttribs,						"NoAttribs" );
SD_CONSTASCII_ACTION( sUNO_View_NoColors,						"NoColors" );
SD_CONSTASCII_ACTION( sUNO_View_GridCoarseWidth,				"GridCoarseWidth" );
SD_CONSTASCII_ACTION( sUNO_View_GridCoarseHeight,				"GridCoarseHeight" );
SD_CONSTASCII_ACTION( sUNO_View_GridFineWidth,					"GridFineWidth" );
SD_CONSTASCII_ACTION( sUNO_View_GridFineHeight,					"GridFineHeight" );
SD_CONSTASCII_ACTION( sUNO_View_GridSnapWidth,					"GridSnapWidth" );
SD_CONSTASCII_ACTION( sUNO_View_GridSnapHeight,					"GridSnapHeight" );
SD_CONSTASCII_ACTION( sUNO_View_IsAngleSnapEnabled,				"IsAngleSnapEnabled" );
SD_CONSTASCII_ACTION( sUNO_View_SnapAngle,						"SnapAngle" );
SD_CONSTASCII_ACTION( sUNO_View_GridSnapWidthXNumerator,		"GridSnapWidthXNumerator" );
SD_CONSTASCII_ACTION( sUNO_View_GridSnapWidthXDenominator,		"GridSnapWidthXDenominator" );
SD_CONSTASCII_ACTION( sUNO_View_GridSnapWidthYNumerator,		"GridSnapWidthYNumerator" );
SD_CONSTASCII_ACTION( sUNO_View_GridSnapWidthYDenominator,		"GridSnapWidthYDenominator" );
SD_CONSTASCII_ACTION( sUNO_View_VisibleLayers,					"VisibleLayers" );
SD_CONSTASCII_ACTION( sUNO_View_PrintableLayers,				"PrintableLayers" );
SD_CONSTASCII_ACTION( sUNO_View_LockedLayers,					"LockedLayers" );

SD_CONSTASCII_ACTION( sUNO_View_VisibleAreaTop,					"VisibleAreaTop" );
SD_CONSTASCII_ACTION( sUNO_View_VisibleAreaLeft,				"VisibleAreaLeft" );
SD_CONSTASCII_ACTION( sUNO_View_VisibleAreaWidth,				"VisibleAreaWidth" );
SD_CONSTASCII_ACTION( sUNO_View_VisibleAreaHeight,				"VisibleAreaHeight" );

SD_CONSTASCII_ACTION( sUNO_View_ZoomOnPage,						"ZoomOnPage" );
} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
