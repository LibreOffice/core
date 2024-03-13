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

#include <rtl/ustring.hxx>

inline constexpr OUString UNO_NAME_CHAR_COLOR = u"CharColor"_ustr;
inline constexpr OUString UNO_NAME_CHAR_COLOR_THEME = u"CharColorTheme"_ustr;
inline constexpr OUString UNO_NAME_CHAR_COLOR_TINT_OR_SHADE = u"CharColorTintOrShade"_ustr;
inline constexpr OUString UNO_NAME_CHAR_COMPLEX_COLOR = u"CharComplexColor"_ustr;
inline constexpr OUString UNO_NAME_CHAR_HEIGHT = u"CharHeight"_ustr;
inline constexpr OUString UNO_NAME_CHAR_POSTURE = u"CharPosture"_ustr;
inline constexpr OUString UNO_NAME_CHAR_SHADOWED = u"CharShadowed"_ustr;
inline constexpr OUString UNO_NAME_CHAR_UNDERLINE = u"CharUnderline"_ustr;
inline constexpr OUString UNO_NAME_CHAR_WEIGHT = u"CharWeight"_ustr;

inline constexpr OUString UNO_NAME_FILLSTYLE = u"FillStyle"_ustr;
inline constexpr OUString UNO_NAME_FILLCOLOR = u"FillColor"_ustr;
inline constexpr OUString UNO_NAME_FILLCOLOR_THEME = u"FillColorTheme"_ustr;
inline constexpr OUString UNO_NAME_FILLCOLOR_LUM_MOD = u"FillColorLumMod"_ustr;
inline constexpr OUString UNO_NAME_FILLCOLOR_LUM_OFF = u"FillColorLumOff"_ustr;
inline constexpr OUString UNO_NAME_FILL_COMPLEX_COLOR = u"FillComplexColor"_ustr;
inline constexpr OUString UNO_NAME_FILLGRADIENT = u"FillGradient"_ustr;
inline constexpr OUString UNO_NAME_FILLGRADIENTNAME = u"FillGradientName"_ustr;
inline constexpr OUString UNO_NAME_FILLHATCH = u"FillHatch"_ustr;
inline constexpr OUString UNO_NAME_FILLHATCHNAME = u"FillHatchName"_ustr;
inline constexpr OUString UNO_NAME_FILLBITMAP = u"FillBitmap"_ustr;
inline constexpr OUString UNO_NAME_FILLBITMAPURL = u"FillBitmapURL"_ustr;
inline constexpr OUString UNO_NAME_FILLBITMAPNAME = u"FillBitmapName"_ustr;
inline constexpr OUString UNO_NAME_FILLGRADIENTSTEPCOUNT = u"FillGradientStepCount"_ustr;
inline constexpr OUString UNO_NAME_FILLBACKGROUND = u"FillBackground"_ustr;
inline constexpr OUString UNO_NAME_FILLUSESLIDEBACKGROUND = u"FillUseSlideBackground"_ustr;
inline constexpr OUString UNO_NAME_FILLCOLOR_2 = u"FillColor2"_ustr;

inline constexpr OUString UNO_NAME_EDGEKIND = u"EdgeKind"_ustr;
inline constexpr OUString UNO_NAME_EDGELINE1DELTA = u"EdgeLine1Delta"_ustr;
inline constexpr OUString UNO_NAME_EDGELINE2DELTA = u"EdgeLine2Delta"_ustr;
inline constexpr OUString UNO_NAME_EDGELINE3DELTA = u"EdgeLine3Delta"_ustr;
inline constexpr OUString UNO_NAME_EDGENODE1HORZDIST = u"EdgeNode1HorzDist"_ustr;
inline constexpr OUString UNO_NAME_EDGENODE1VERTDIST = u"EdgeNode1VertDist"_ustr;
inline constexpr OUString UNO_NAME_EDGENODE2HORZDIST = u"EdgeNode2HorzDist"_ustr;
inline constexpr OUString UNO_NAME_EDGENODE2VERTDIST = u"EdgeNode2VertDist"_ustr;
inline constexpr OUString UNO_NAME_EDGEOOXMLCURVE = u"EdgeOOXMLCurve"_ustr;

inline constexpr OUString UNO_NAME_FILLBMP_OFFSET_X = u"FillBitmapOffsetX"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_OFFSET_Y = u"FillBitmapOffsetY"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_POSITION_OFFSET_X = u"FillBitmapPositionOffsetX"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_POSITION_OFFSET_Y = u"FillBitmapPositionOffsetY"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_RECTANGLE_POINT = u"FillBitmapRectanglePoint"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_TILE = u"FillBitmapTile"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_STRETCH = u"FillBitmapStretch"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_LOGICAL_SIZE = u"FillBitmapLogicalSize"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_SIZE_X = u"FillBitmapSizeX"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_SIZE_Y = u"FillBitmapSizeY"_ustr;
inline constexpr OUString UNO_NAME_FILL_TRANSPARENCE = u"FillTransparence"_ustr;
inline constexpr OUString UNO_NAME_FILLTRANSPARENCEGRADIENT = u"FillTransparenceGradient"_ustr;
inline constexpr OUString UNO_NAME_FILLTRANSPARENCEGRADIENTNAME = u"FillTransparenceGradientName"_ustr;
inline constexpr OUString UNO_NAME_FILLBMP_MODE = u"FillBitmapMode"_ustr;


inline constexpr OUString UNO_NAME_LINESTYLE = u"LineStyle"_ustr;
inline constexpr OUString UNO_NAME_LINEDASH = u"LineDash"_ustr;
inline constexpr OUString UNO_NAME_LINEWIDTH = u"LineWidth"_ustr;
inline constexpr OUString UNO_NAME_LINECOLOR = u"LineColor"_ustr;
inline constexpr OUString UNO_NAME_LINE_COMPLEX_COLOR = u"LineComplexColor"_ustr;
inline constexpr OUString UNO_NAME_LINEJOINT = u"LineJoint"_ustr;
inline constexpr OUString UNO_NAME_LINESTART = u"LineStart"_ustr;
inline constexpr OUString UNO_NAME_LINEEND = u"LineEnd"_ustr;
inline constexpr OUString UNO_NAME_LINESTARTWIDTH = u"LineStartWidth"_ustr;
inline constexpr OUString UNO_NAME_LINEENDWIDTH = u"LineEndWidth"_ustr;
inline constexpr OUString UNO_NAME_LINESTARTCENTER = u"LineStartCenter"_ustr;
inline constexpr OUString UNO_NAME_LINEENDCENTER = u"LineEndCenter"_ustr;
inline constexpr OUString UNO_NAME_LINETRANSPARENCE = u"LineTransparence"_ustr;
inline constexpr OUString UNO_NAME_LINECAP = u"LineCap"_ustr;

inline constexpr OUString UNO_NAME_SHADOW = u"Shadow"_ustr;
inline constexpr OUString UNO_NAME_SHADOWCOLOR = u"ShadowColor"_ustr;
inline constexpr OUString UNO_NAME_SHADOWXDIST = u"ShadowXDistance"_ustr;
inline constexpr OUString UNO_NAME_SHADOWYDIST = u"ShadowYDistance"_ustr;
inline constexpr OUString UNO_NAME_SHADOWSIZEX = u"ShadowSizeX"_ustr;
inline constexpr OUString UNO_NAME_SHADOWSIZEY = u"ShadowSizeY"_ustr;
inline constexpr OUString UNO_NAME_SHADOWTRANSPARENCE = u"ShadowTransparence"_ustr;
inline constexpr OUString UNO_NAME_SHADOWBLUR = u"ShadowBlur"_ustr;
inline constexpr OUString UNO_NAME_SHADOWALIGNMENT = u"ShadowAlignment"_ustr;

inline constexpr OUString UNO_NAME_EDGERADIUS = u"CornerRadius"_ustr;

inline constexpr OUString UNO_NAME_TEXT_MINFRAMEHEIGHT = u"TextMinimumFrameHeight"_ustr;
inline constexpr OUString UNO_NAME_TEXT_AUTOGROWHEIGHT = u"TextAutoGrowHeight"_ustr;
inline constexpr OUString UNO_NAME_TEXT_FITTOSIZE = u"TextFitToSize"_ustr;
inline constexpr OUString UNO_NAME_TEXT_LEFTDIST = u"TextLeftDistance"_ustr;
inline constexpr OUString UNO_NAME_TEXT_RIGHTDIST = u"TextRightDistance"_ustr;
inline constexpr OUString UNO_NAME_TEXT_UPPERDIST = u"TextUpperDistance"_ustr;
inline constexpr OUString UNO_NAME_TEXT_LOWERDIST = u"TextLowerDistance"_ustr;
inline constexpr OUString UNO_NAME_TEXT_VERTADJUST = u"TextVerticalAdjust"_ustr;
inline constexpr OUString UNO_NAME_TEXT_MAXFRAMEHEIGHT = u"TextMaximumFrameHeight"_ustr;
inline constexpr OUString UNO_NAME_TEXT_MINFRAMEWIDTH = u"TextMinimumFrameWidth"_ustr;
inline constexpr OUString UNO_NAME_TEXT_MAXFRAMEWIDTH = u"TextMaximumFrameWidth"_ustr;
inline constexpr OUString UNO_NAME_TEXT_AUTOGROWWIDTH = u"TextAutoGrowWidth"_ustr;
inline constexpr OUString UNO_NAME_TEXT_HORZADJUST = u"TextHorizontalAdjust"_ustr;
inline constexpr OUString UNO_NAME_TEXT_ANIKIND = u"TextAnimationKind"_ustr;
inline constexpr OUString UNO_NAME_TEXT_ANIDIRECTION = u"TextAnimationDirection"_ustr;
inline constexpr OUString UNO_NAME_TEXT_ANISTARTINSIDE = u"TextAnimationStartInside"_ustr;
inline constexpr OUString UNO_NAME_TEXT_ANISTOPINSIDE = u"TextAnimationStopInside"_ustr;
inline constexpr OUString UNO_NAME_TEXT_ANICOUNT = u"TextAnimationCount"_ustr;
inline constexpr OUString UNO_NAME_TEXT_ANIDELAY = u"TextAnimationDelay"_ustr;
inline constexpr OUString UNO_NAME_TEXT_ANIAMOUNT = u"TextAnimationAmount"_ustr;
inline constexpr OUString UNO_NAME_TEXT_CONTOURFRAME = u"TextContourFrame"_ustr;
inline constexpr OUString UNO_NAME_TEXT_WRITINGMODE = u"TextWritingMode"_ustr;
inline constexpr OUString UNO_NAME_TEXT_FONTINDEPENDENTLINESPACING = u"FontIndependentLineSpacing"_ustr;
inline constexpr OUString UNO_NAME_TEXT_WORDWRAP = u"TextWordWrap"_ustr;
inline constexpr OUString UNO_NAME_TEXT_CHAINNEXTNAME = u"TextChainNextName"_ustr;
inline constexpr OUString UNO_NAME_TEXT_CLIPVERTOVERFLOW = u"TextClipVerticalOverflow"_ustr;

inline constexpr OUString UNO_NAME_MEASUREKIND = u"MeasureKind"_ustr;
inline constexpr OUString UNO_NAME_MEASURETEXTHPOS = u"MeasureTextHorizontalPosition"_ustr;
inline constexpr OUString UNO_NAME_MEASURETEXTVPOS = u"MeasureTextVerticalPosition"_ustr;
inline constexpr OUString UNO_NAME_MEASURELINEDIST = u"MeasureLineDistance"_ustr;
inline constexpr OUString UNO_NAME_MEASUREHELPLINEOVERHANG = u"MeasureHelpLineOverhang"_ustr;
inline constexpr OUString UNO_NAME_MEASUREHELPLINEDIST = u"MeasureHelpLineDistance"_ustr;
inline constexpr OUString UNO_NAME_MEASUREHELPLINE1LEN = u"MeasureHelpLine1Length"_ustr;
inline constexpr OUString UNO_NAME_MEASUREHELPLINE2LEN = u"MeasureHelpLine2Length"_ustr;
inline constexpr OUString UNO_NAME_MEASUREBELOWREFEDGE = u"MeasureBelowReferenceEdge"_ustr;
inline constexpr OUString UNO_NAME_MEASURETEXTROTA90 = u"MeasureTextRotate90"_ustr;
inline constexpr OUString UNO_NAME_MEASURETEXTUPSIDEDOWN = u"MeasureTextUpsideDown"_ustr;
inline constexpr OUString UNO_NAME_MEASUREOVERHANG = u"MeasureOverhang"_ustr;
inline constexpr OUString UNO_NAME_MEASUREUNIT = u"MeasureUnit"_ustr;
inline constexpr OUString UNO_NAME_MEASURESHOWUNIT = u"MeasureShowUnit"_ustr;
inline constexpr OUString UNO_NAME_MEASUREFORMATSTRING = u"MeasureFormatString"_ustr;
inline constexpr OUString UNO_NAME_MEASURETEXTAUTOANGLE = u"MeasureTextAutoAngle"_ustr;
inline constexpr OUString UNO_NAME_MEASURETEXTAUTOANGLEVIEW = u"MeasureTextAutoAngleView"_ustr;
inline constexpr OUString UNO_NAME_MEASURETEXTISFIXEDANGLE = u"MeasureTextIsFixedAngle"_ustr;
inline constexpr OUString UNO_NAME_MEASURETEXTFIXEDANGLE = u"MeasureTextFixedAngle"_ustr;

inline constexpr OUString UNO_NAME_CIRCKIND = u"CircleKind"_ustr;
inline constexpr OUString UNO_NAME_CIRCSTARTANGLE = u"CircleStartAngle"_ustr;
inline constexpr OUString UNO_NAME_CIRCENDANGLE = u"CircleEndAngle"_ustr;

inline constexpr OUString UNO_NAME_POLYGONKIND = u"PolygonKind"_ustr;
inline constexpr OUString UNO_NAME_POLYPOLYGON = u"PolyPolygon"_ustr;
inline constexpr OUString UNO_NAME_POLYPOLYGONBEZIER = u"PolyPolygonBezier"_ustr;
inline constexpr OUString UNO_NAME_POLYGON = u"Polygon"_ustr;

inline constexpr OUString UNO_NAME_MISC_OBJ_INTEROPGRABBAG = u"InteropGrabBag"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_ZORDER = u"ZOrder"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_MOVEPROTECT = u"MoveProtect"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_SIZEPROTECT = u"SizeProtect"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_PRINTABLE = u"Printable"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_LAYERID = u"LayerID"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_LAYERNAME = u"LayerName"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_NAME = u"Name"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_ROTATEANGLE = u"RotateAngle"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_SHEARANGLE = u"ShearAngle"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_FRAMERECT = u"FrameRect"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_BOUNDRECT = u"BoundRect"_ustr;

// #i68101#
inline constexpr OUString UNO_NAME_MISC_OBJ_TITLE = u"Title"_ustr;
inline constexpr OUString UNO_NAME_MISC_OBJ_DESCRIPTION = u"Description"_ustr;

inline constexpr OUString UNO_NAME_HYPERLINK = u"Hyperlink"_ustr;

inline constexpr OUString UNO_NAME_GRAPHOBJ_FILLBITMAP = u"GraphicObjectFillBitmap"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_REPLACEMENT_GRAPHIC = u"ReplacementGraphic"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_GRAFSTREAMURL = u"GraphicStreamURL"_ustr;
#define UNO_NAME_GRAPHOBJ_URLPKGPREFIX          "vnd.sun.star.Package:"
inline constexpr OUString UNO_NAME_GRAPHOBJ_GRAPHIC = u"Graphic"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_GRAPHIC_URL = u"GraphicURL"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_IS_SIGNATURELINE = u"IsSignatureLine"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_SIGNATURELINE_ID = u"SignatureLineId"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_SIGNATURELINE_SUGGESTED_SIGNER_NAME = u"SignatureLineSuggestedSignerName"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_SIGNATURELINE_SUGGESTED_SIGNER_TITLE = u"SignatureLineSuggestedSignerTitle"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_SIGNATURELINE_SUGGESTED_SIGNER_EMAIL = u"SignatureLineSuggestedSignerEmail"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_SIGNATURELINE_SIGNING_INSTRUCTIONS = u"SignatureLineSigningInstructions"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_SIGNATURELINE_SHOW_SIGN_DATE = u"SignatureLineShowSignDate"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_SIGNATURELINE_CAN_ADD_COMMENT = u"SignatureLineCanAddComment"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_SIGNATURELINE_UNSIGNED_IMAGE = u"SignatureLineUnsignedImage"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_SIGNATURELINE_IS_SIGNED = u"SignatureLineIsSigned"_ustr;
inline constexpr OUString UNO_NAME_GRAPHOBJ_QRCODE = u"BarCodeProperties"_ustr;

inline constexpr OUString UNO_NAME_OLE2_METAFILE = u"MetaFile"_ustr;
inline constexpr OUString UNO_NAME_OLE2_PERSISTNAME = u"PersistName"_ustr;

inline constexpr OUString UNO_NAME_3D_SCENE_PERSPECTIVE = u"D3DScenePerspective"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_DISTANCE = u"D3DSceneDistance"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_FOCAL_LENGTH = u"D3DSceneFocalLength"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_TWO_SIDED_LIGHTING = u"D3DSceneTwoSidedLighting"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTCOLOR_1 = u"D3DSceneLightColor1"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTCOLOR_2 = u"D3DSceneLightColor2"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTCOLOR_3 = u"D3DSceneLightColor3"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTCOLOR_4 = u"D3DSceneLightColor4"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTCOLOR_5 = u"D3DSceneLightColor5"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTCOLOR_6 = u"D3DSceneLightColor6"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTCOLOR_7 = u"D3DSceneLightColor7"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTCOLOR_8 = u"D3DSceneLightColor8"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTON_1 = u"D3DSceneLightOn1"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTON_2 = u"D3DSceneLightOn2"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTON_3 = u"D3DSceneLightOn3"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTON_4 = u"D3DSceneLightOn4"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTON_5 = u"D3DSceneLightOn5"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTON_6 = u"D3DSceneLightOn6"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTON_7 = u"D3DSceneLightOn7"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTON_8 = u"D3DSceneLightOn8"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTDIRECTION_1 = u"D3DSceneLightDirection1"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTDIRECTION_2 = u"D3DSceneLightDirection2"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTDIRECTION_3 = u"D3DSceneLightDirection3"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTDIRECTION_4 = u"D3DSceneLightDirection4"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTDIRECTION_5 = u"D3DSceneLightDirection5"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTDIRECTION_6 = u"D3DSceneLightDirection6"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTDIRECTION_7 = u"D3DSceneLightDirection7"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_LIGHTDIRECTION_8 = u"D3DSceneLightDirection8"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_AMBIENTCOLOR = u"D3DSceneAmbientColor"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_SHADOW_SLANT = u"D3DSceneShadowSlant"_ustr;
inline constexpr OUString UNO_NAME_3D_SCENE_SHADE_MODE = u"D3DSceneShadeMode"_ustr;

inline constexpr OUString UNO_NAME_3D_DOUBLE_SIDED = u"D3DDoubleSided"_ustr;
inline constexpr OUString UNO_NAME_3D_NORMALS_KIND = u"D3DNormalsKind"_ustr;
inline constexpr OUString UNO_NAME_3D_NORMALS_INVERT = u"D3DNormalsInvert"_ustr;
inline constexpr OUString UNO_NAME_3D_TEXTURE_PROJ_X = u"D3DTextureProjectionX"_ustr;
inline constexpr OUString UNO_NAME_3D_TEXTURE_PROJ_Y = u"D3DTextureProjectionY"_ustr;
inline constexpr OUString UNO_NAME_3D_SHADOW_3D = u"D3DShadow3D"_ustr;
inline constexpr OUString UNO_NAME_3D_MAT_COLOR = u"D3DMaterialColor"_ustr;
inline constexpr OUString UNO_NAME_3D_MAT_EMISSION = u"D3DMaterialEmission"_ustr;
inline constexpr OUString UNO_NAME_3D_MAT_SPECULAR = u"D3DMaterialSpecular"_ustr;
inline constexpr OUString UNO_NAME_3D_MAT_SPECULAR_INTENSITY = u"D3DMaterialSpecularIntensity"_ustr;
inline constexpr OUString UNO_NAME_3D_TEXTURE_KIND = u"D3DTextureKind"_ustr;
inline constexpr OUString UNO_NAME_3D_TEXTURE_MODE = u"D3DTextureMode"_ustr;
inline constexpr OUString UNO_NAME_3D_TEXTURE_FILTER = u"D3DTextureFilter"_ustr;

// #i28528#
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry
inline constexpr OUString UNO_NAME_3D_REDUCED_LINE_GEOMETRY = u"D3DReducedLineGeometry"_ustr;

inline constexpr OUString UNO_NAME_3D_HORZ_SEGS = u"D3DHorizontalSegments"_ustr;
inline constexpr OUString UNO_NAME_3D_VERT_SEGS = u"D3DVerticalSegments"_ustr;

inline constexpr OUString UNO_NAME_3D_PERCENT_DIAGONAL = u"D3DPercentDiagonal"_ustr;
inline constexpr OUString UNO_NAME_3D_BACKSCALE = u"D3DBackscale"_ustr;
inline constexpr OUString UNO_NAME_3D_LATHE_END_ANGLE = u"D3DEndAngle"_ustr;

inline constexpr OUString UNO_NAME_3D_EXTRUDE_DEPTH = u"D3DDepth"_ustr;

inline constexpr OUString UNO_NAME_3D_TRANSFORM_MATRIX = u"D3DTransformMatrix"_ustr;
inline constexpr OUString UNO_NAME_3D_CAMERA_GEOMETRY = u"D3DCameraGeometry"_ustr;
inline constexpr OUString UNO_NAME_3D_POS = u"D3DPosition"_ustr;
inline constexpr OUString UNO_NAME_3D_SIZE = u"D3DSize"_ustr;
inline constexpr OUString UNO_NAME_3D_POS_IS_CENTER = u"D3DPositionIsCenter"_ustr;
inline constexpr OUString UNO_NAME_3D_POLYPOLYGON3D = u"D3DPolyPolygon3D"_ustr;
inline constexpr OUString UNO_NAME_3D_NORMALSPOLYGON3D = u"D3DNormalsPolygon3D"_ustr;
inline constexpr OUString UNO_NAME_3D_TEXTUREPOLYGON3D = u"D3DTexturePolygon3D"_ustr;
inline constexpr OUString UNO_NAME_3D_LINEONLY = u"D3DLineOnly"_ustr;

// New 3D properties which are possible for lathe and extrude 3d objects
inline constexpr OUString UNO_NAME_3D_SMOOTH_NORMALS = u"D3DSmoothNormals"_ustr;
inline constexpr OUString UNO_NAME_3D_SMOOTH_LIDS = u"D3DSmoothLids"_ustr;
inline constexpr OUString UNO_NAME_3D_CHARACTER_MODE = u"D3DCharacterMode"_ustr;
inline constexpr OUString UNO_NAME_3D_CLOSE_FRONT = u"D3DCloseFront"_ustr;
inline constexpr OUString UNO_NAME_3D_CLOSE_BACK = u"D3DCloseBack"_ustr;

inline constexpr OUString UNO_NAME_NUMBERING = u"NumberingIsNumber"_ustr;
inline constexpr OUString UNO_NAME_NUMBERING_RULES = u"NumberingRules"_ustr;
inline constexpr OUString UNO_NAME_NUMBERING_LEVEL = u"NumberingLevel"_ustr;

inline constexpr OUString UNO_NAME_NRULE_NUMBERINGTYPE = u"NumberingType"_ustr;
inline constexpr OUString UNO_NAME_NRULE_PREFIX = u"Prefix"_ustr;
inline constexpr OUString UNO_NAME_NRULE_SUFFIX = u"Suffix"_ustr;
#define UNO_NAME_NRULE_BULLETID                 "BulletId"
inline constexpr OUString UNO_NAME_NRULE_BULLET_COLOR = u"BulletColor"_ustr;
inline constexpr OUString UNO_NAME_NRULE_BULLET_RELSIZE = u"BulletRelSize"_ustr;
inline constexpr OUString UNO_NAME_NRULE_BULLET_FONT = u"BulletFont"_ustr;
inline constexpr OUString UNO_NAME_NRULE_START_WITH = u"StartWith"_ustr;
inline constexpr OUString UNO_NAME_NRULE_LEFT_MARGIN = u"LeftMargin"_ustr;
inline constexpr OUString UNO_NAME_NRULE_FIRST_LINE_OFFSET = u"FirstLineOffset"_ustr;
inline constexpr OUString UNO_NAME_NRULE_ADJUST = u"Adjust"_ustr;

inline constexpr OUString UNO_NAME_EDIT_FONT_DESCRIPTOR = u"FontDescriptor"_ustr;

inline constexpr OUString UNO_NAME_EDIT_PARA_ADJUST = u"ParaAdjust"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_BMARGIN = u"ParaBottomMargin"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_LASTLINEADJ = u"ParaLastLineAdjust"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_LMARGIN = u"ParaLeftMargin"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_LINESPACING = u"ParaLineSpacing"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_RMARGIN = u"ParaRightMargin"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_TABSTOPS = u"ParaTabStops"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_TABSTOP_DEFAULT_DISTANCE = u"ParaTabStopDefaultDistance"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_TMARGIN = u"ParaTopMargin"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_IS_HYPHEN = u"ParaIsHyphenation"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION = u"ParaIsHangingPunctuation"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_IS_CHARACTER_DISTANCE = u"ParaIsCharacterDistance"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_IS_FORBIDDEN_RULES = u"ParaIsForbiddenRules"_ustr;
inline constexpr OUString UNO_NAME_EDIT_PARA_FIRST_LINE_INDENT = u"ParaFirstLineIndent"_ustr;

inline constexpr OUString UNO_NAME_EDIT_CHAR_HEIGHT = u"CharHeight"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTNAME = u"CharFontName"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTSTYLENAME = u"CharFontStyleName"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTFAMILY = u"CharFontFamily"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTCHARSET = u"CharFontCharSet"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTPITCH = u"CharFontPitch"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_POSTURE = u"CharPosture"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_WEIGHT = u"CharWeight"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_LOCALE = u"CharLocale"_ustr;

inline constexpr OUString UNO_NAME_EDIT_CHAR_HEIGHT_ASIAN = u"CharHeightAsian"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTNAME_ASIAN = u"CharFontNameAsian"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTSTYLENAME_ASIAN = u"CharFontStyleNameAsian"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTFAMILY_ASIAN = u"CharFontFamilyAsian"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTCHARSET_ASIAN = u"CharFontCharSetAsian"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTPITCH_ASIAN = u"CharFontPitchAsian"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_POSTURE_ASIAN = u"CharPostureAsian"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_WEIGHT_ASIAN = u"CharWeightAsian"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_LOCALE_ASIAN = u"CharLocaleAsian"_ustr;

inline constexpr OUString UNO_NAME_EDIT_CHAR_HEIGHT_COMPLEX = u"CharHeightComplex"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTNAME_COMPLEX = u"CharFontNameComplex"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTSTYLENAME_COMPLEX = u"CharFontStyleNameComplex"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTFAMILY_COMPLEX = u"CharFontFamilyComplex"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTCHARSET_COMPLEX = u"CharFontCharSetComplex"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_FONTPITCH_COMPLEX = u"CharFontPitchComplex"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_POSTURE_COMPLEX = u"CharPostureComplex"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_WEIGHT_COMPLEX = u"CharWeightComplex"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_LOCALE_COMPLEX = u"CharLocaleComplex"_ustr;

inline constexpr OUString UNO_NAME_EDIT_CHAR_COLOR = u"CharColor"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_COLOR_THEME = u"CharColorTheme"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_COLOR_TINT_OR_SHADE = u"CharColorTintOrShade"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_COLOR_LUM_MOD = u"CharColorLumMod"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_COLOR_LUM_OFF = u"CharColorLumOff"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_COMPLEX_COLOR = u"CharComplexColor"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_TRANSPARENCE = u"CharTransparence"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_CROSSEDOUT = u"CharCrossedOut"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_STRIKEOUT = u"CharStrikeout"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_CASEMAP = u"CharCaseMap"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_ESCAPEMENT = u"CharEscapement"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_SHADOWED = u"CharShadowed"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_UNDERLINE = u"CharUnderline"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_OVERLINE = u"CharOverline"_ustr;

inline constexpr OUString UNO_NAME_EDIT_CHAR_BACKGROUND_COLOR = u"CharBackColor"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_BACKGROUND_COMPLEX_COLOR = u"CharBackgroundComplexColor"_ustr;
inline constexpr OUString UNO_NAME_EDIT_CHAR_BACKGROUND_TRANSPARENT = u"CharBackTransparent"_ustr;

inline constexpr OUString UNO_NAME_BITMAP = u"Bitmap"_ustr;

inline constexpr OUString UNO_NAME_LINKDISPLAYNAME = u"LinkDisplayName"_ustr;
inline constexpr OUString UNO_NAME_LINKDISPLAYBITMAP = u"LinkDisplayBitmap"_ustr;

inline constexpr OUString UNO_NAME_GRAPHIC_LUMINANCE = u"AdjustLuminance"_ustr;
inline constexpr OUString UNO_NAME_GRAPHIC_CONTRAST = u"AdjustContrast"_ustr;
inline constexpr OUString UNO_NAME_GRAPHIC_RED = u"AdjustRed"_ustr;
inline constexpr OUString UNO_NAME_GRAPHIC_GREEN = u"AdjustGreen"_ustr;
inline constexpr OUString UNO_NAME_GRAPHIC_BLUE = u"AdjustBlue"_ustr;
inline constexpr OUString UNO_NAME_GRAPHIC_GAMMA = u"Gamma"_ustr;
inline constexpr OUString UNO_NAME_GRAPHIC_TRANSPARENCY = u"Transparency"_ustr;
inline constexpr OUString UNO_NAME_GRAPHIC_COLOR_MODE = u"GraphicColorMode"_ustr;
inline constexpr OUString UNO_NAME_GRAPHIC_GRAPHICCROP = u"GraphicCrop"_ustr;

inline constexpr OUString UNO_NAME_OBJ_ISEMPTYPRESOBJ = u"IsEmptyPresentationObject"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
