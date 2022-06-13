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

inline constexpr OUStringLiteral UNO_NAME_CHAR_COLOR = u"CharColor";
inline constexpr OUStringLiteral UNO_NAME_CHAR_HEIGHT = u"CharHeight";
inline constexpr OUStringLiteral UNO_NAME_CHAR_POSTURE = u"CharPosture";
inline constexpr OUStringLiteral UNO_NAME_CHAR_SHADOWED = u"CharShadowed";
inline constexpr OUStringLiteral UNO_NAME_CHAR_UNDERLINE = u"CharUnderline";
inline constexpr OUStringLiteral UNO_NAME_CHAR_WEIGHT = u"CharWeight";

inline constexpr OUStringLiteral UNO_NAME_FILLSTYLE = u"FillStyle";
inline constexpr OUStringLiteral UNO_NAME_FILLCOLOR = u"FillColor";
inline constexpr OUStringLiteral UNO_NAME_FILLCOLOR_THEME = u"FillColorTheme";
inline constexpr OUStringLiteral UNO_NAME_FILLCOLOR_LUM_MOD = u"FillColorLumMod";
inline constexpr OUStringLiteral UNO_NAME_FILLCOLOR_LUM_OFF = u"FillColorLumOff";
inline constexpr OUStringLiteral UNO_NAME_FILLGRADIENT = u"FillGradient";
inline constexpr OUStringLiteral UNO_NAME_FILLGRADIENTNAME = u"FillGradientName";
inline constexpr OUStringLiteral UNO_NAME_FILLHATCH = u"FillHatch";
inline constexpr OUStringLiteral UNO_NAME_FILLHATCHNAME = u"FillHatchName";
inline constexpr OUStringLiteral UNO_NAME_FILLBITMAP = u"FillBitmap";
inline constexpr OUStringLiteral UNO_NAME_FILLBITMAPURL = u"FillBitmapURL";
inline constexpr OUStringLiteral UNO_NAME_FILLBITMAPNAME = u"FillBitmapName";
inline constexpr OUStringLiteral UNO_NAME_FILLGRADIENTSTEPCOUNT = u"FillGradientStepCount";
inline constexpr OUStringLiteral UNO_NAME_FILLBACKGROUND = u"FillBackground";
inline constexpr OUStringLiteral UNO_NAME_FILLUSESLIDEBACKGROUND = u"FillUseSlideBackground";
inline constexpr OUStringLiteral UNO_NAME_FILLCOLOR_2 = u"FillColor2";

inline constexpr OUStringLiteral UNO_NAME_EDGEKIND = u"EdgeKind";
inline constexpr OUStringLiteral UNO_NAME_EDGELINE1DELTA = u"EdgeLine1Delta";
inline constexpr OUStringLiteral UNO_NAME_EDGELINE2DELTA = u"EdgeLine2Delta";
inline constexpr OUStringLiteral UNO_NAME_EDGELINE3DELTA = u"EdgeLine3Delta";
inline constexpr OUStringLiteral UNO_NAME_EDGENODE1HORZDIST = u"EdgeNode1HorzDist";
inline constexpr OUStringLiteral UNO_NAME_EDGENODE1VERTDIST = u"EdgeNode1VertDist";
inline constexpr OUStringLiteral UNO_NAME_EDGENODE2HORZDIST = u"EdgeNode2HorzDist";
inline constexpr OUStringLiteral UNO_NAME_EDGENODE2VERTDIST = u"EdgeNode2VertDist";

inline constexpr OUStringLiteral UNO_NAME_FILLBMP_OFFSET_X = u"FillBitmapOffsetX";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_OFFSET_Y = u"FillBitmapOffsetY";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_POSITION_OFFSET_X = u"FillBitmapPositionOffsetX";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_POSITION_OFFSET_Y = u"FillBitmapPositionOffsetY";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_RECTANGLE_POINT = u"FillBitmapRectanglePoint";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_TILE = u"FillBitmapTile";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_STRETCH = u"FillBitmapStretch";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_LOGICAL_SIZE = u"FillBitmapLogicalSize";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_SIZE_X = u"FillBitmapSizeX";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_SIZE_Y = u"FillBitmapSizeY";
inline constexpr OUStringLiteral UNO_NAME_FILL_TRANSPARENCE = u"FillTransparence";
inline constexpr OUStringLiteral UNO_NAME_FILLTRANSPARENCEGRADIENT = u"FillTransparenceGradient";
inline constexpr OUStringLiteral UNO_NAME_FILLTRANSPARENCEGRADIENTNAME = u"FillTransparenceGradientName";
inline constexpr OUStringLiteral UNO_NAME_FILLBMP_MODE = u"FillBitmapMode";


inline constexpr OUStringLiteral UNO_NAME_LINESTYLE = u"LineStyle";
inline constexpr OUStringLiteral UNO_NAME_LINEDASH = u"LineDash";
inline constexpr OUStringLiteral UNO_NAME_LINEWIDTH = u"LineWidth";
inline constexpr OUStringLiteral UNO_NAME_LINECOLOR = u"LineColor";
inline constexpr OUStringLiteral UNO_NAME_LINEJOINT = u"LineJoint";
inline constexpr OUStringLiteral UNO_NAME_LINESTART = u"LineStart";
inline constexpr OUStringLiteral UNO_NAME_LINEEND = u"LineEnd";
inline constexpr OUStringLiteral UNO_NAME_LINESTARTWIDTH = u"LineStartWidth";
inline constexpr OUStringLiteral UNO_NAME_LINEENDWIDTH = u"LineEndWidth";
inline constexpr OUStringLiteral UNO_NAME_LINESTARTCENTER = u"LineStartCenter";
inline constexpr OUStringLiteral UNO_NAME_LINEENDCENTER = u"LineEndCenter";
inline constexpr OUStringLiteral UNO_NAME_LINETRANSPARENCE = u"LineTransparence";
inline constexpr OUStringLiteral UNO_NAME_LINECAP = u"LineCap";

inline constexpr OUStringLiteral UNO_NAME_SHADOW = u"Shadow";
inline constexpr OUStringLiteral UNO_NAME_SHADOWCOLOR = u"ShadowColor";
inline constexpr OUStringLiteral UNO_NAME_SHADOWXDIST = u"ShadowXDistance";
inline constexpr OUStringLiteral UNO_NAME_SHADOWYDIST = u"ShadowYDistance";
inline constexpr OUStringLiteral UNO_NAME_SHADOWSIZEX = u"ShadowSizeX";
inline constexpr OUStringLiteral UNO_NAME_SHADOWSIZEY = u"ShadowSizeY";
inline constexpr OUStringLiteral UNO_NAME_SHADOWTRANSPARENCE = u"ShadowTransparence";
inline constexpr OUStringLiteral UNO_NAME_SHADOWBLUR = u"ShadowBlur";

inline constexpr OUStringLiteral UNO_NAME_EDGERADIUS = u"CornerRadius";

inline constexpr OUStringLiteral UNO_NAME_TEXT_MINFRAMEHEIGHT = u"TextMinimumFrameHeight";
inline constexpr OUStringLiteral UNO_NAME_TEXT_AUTOGROWHEIGHT = u"TextAutoGrowHeight";
inline constexpr OUStringLiteral UNO_NAME_TEXT_FITTOSIZE = u"TextFitToSize";
inline constexpr OUStringLiteral UNO_NAME_TEXT_LEFTDIST = u"TextLeftDistance";
inline constexpr OUStringLiteral UNO_NAME_TEXT_RIGHTDIST = u"TextRightDistance";
inline constexpr OUStringLiteral UNO_NAME_TEXT_UPPERDIST = u"TextUpperDistance";
inline constexpr OUStringLiteral UNO_NAME_TEXT_LOWERDIST = u"TextLowerDistance";
inline constexpr OUStringLiteral UNO_NAME_TEXT_VERTADJUST = u"TextVerticalAdjust";
inline constexpr OUStringLiteral UNO_NAME_TEXT_MAXFRAMEHEIGHT = u"TextMaximumFrameHeight";
inline constexpr OUStringLiteral UNO_NAME_TEXT_MINFRAMEWIDTH = u"TextMinimumFrameWidth";
inline constexpr OUStringLiteral UNO_NAME_TEXT_MAXFRAMEWIDTH = u"TextMaximumFrameWidth";
inline constexpr OUStringLiteral UNO_NAME_TEXT_AUTOGROWWIDTH = u"TextAutoGrowWidth";
inline constexpr OUStringLiteral UNO_NAME_TEXT_HORZADJUST = u"TextHorizontalAdjust";
inline constexpr OUStringLiteral UNO_NAME_TEXT_ANIKIND = u"TextAnimationKind";
inline constexpr OUStringLiteral UNO_NAME_TEXT_ANIDIRECTION = u"TextAnimationDirection";
inline constexpr OUStringLiteral UNO_NAME_TEXT_ANISTARTINSIDE = u"TextAnimationStartInside";
inline constexpr OUStringLiteral UNO_NAME_TEXT_ANISTOPINSIDE = u"TextAnimationStopInside";
inline constexpr OUStringLiteral UNO_NAME_TEXT_ANICOUNT = u"TextAnimationCount";
inline constexpr OUStringLiteral UNO_NAME_TEXT_ANIDELAY = u"TextAnimationDelay";
inline constexpr OUStringLiteral UNO_NAME_TEXT_ANIAMOUNT = u"TextAnimationAmount";
inline constexpr OUStringLiteral UNO_NAME_TEXT_CONTOURFRAME = u"TextContourFrame";
inline constexpr OUStringLiteral UNO_NAME_TEXT_WRITINGMODE = u"TextWritingMode";
inline constexpr OUStringLiteral UNO_NAME_TEXT_FONTINDEPENDENTLINESPACING = u"FontIndependentLineSpacing";
inline constexpr OUStringLiteral UNO_NAME_TEXT_WORDWRAP = u"TextWordWrap";
inline constexpr OUStringLiteral UNO_NAME_TEXT_CHAINNEXTNAME = u"TextChainNextName";

inline constexpr OUStringLiteral UNO_NAME_MEASUREKIND = u"MeasureKind";
inline constexpr OUStringLiteral UNO_NAME_MEASURETEXTHPOS = u"MeasureTextHorizontalPosition";
inline constexpr OUStringLiteral UNO_NAME_MEASURETEXTVPOS = u"MeasureTextVerticalPosition";
inline constexpr OUStringLiteral UNO_NAME_MEASURELINEDIST = u"MeasureLineDistance";
inline constexpr OUStringLiteral UNO_NAME_MEASUREHELPLINEOVERHANG = u"MeasureHelpLineOverhang";
inline constexpr OUStringLiteral UNO_NAME_MEASUREHELPLINEDIST = u"MeasureHelpLineDistance";
inline constexpr OUStringLiteral UNO_NAME_MEASUREHELPLINE1LEN = u"MeasureHelpLine1Length";
inline constexpr OUStringLiteral UNO_NAME_MEASUREHELPLINE2LEN = u"MeasureHelpLine2Length";
inline constexpr OUStringLiteral UNO_NAME_MEASUREBELOWREFEDGE = u"MeasureBelowReferenceEdge";
inline constexpr OUStringLiteral UNO_NAME_MEASURETEXTROTA90 = u"MeasureTextRotate90";
inline constexpr OUStringLiteral UNO_NAME_MEASURETEXTUPSIDEDOWN = u"MeasureTextUpsideDown";
inline constexpr OUStringLiteral UNO_NAME_MEASUREOVERHANG = u"MeasureOverhang";
inline constexpr OUStringLiteral UNO_NAME_MEASUREUNIT = u"MeasureUnit";
inline constexpr OUStringLiteral UNO_NAME_MEASURESHOWUNIT = u"MeasureShowUnit";
inline constexpr OUStringLiteral UNO_NAME_MEASUREFORMATSTRING = u"MeasureFormatString";
inline constexpr OUStringLiteral UNO_NAME_MEASURETEXTAUTOANGLE = u"MeasureTextAutoAngle";
inline constexpr OUStringLiteral UNO_NAME_MEASURETEXTAUTOANGLEVIEW = u"MeasureTextAutoAngleView";
inline constexpr OUStringLiteral UNO_NAME_MEASURETEXTISFIXEDANGLE = u"MeasureTextIsFixedAngle";
inline constexpr OUStringLiteral UNO_NAME_MEASURETEXTFIXEDANGLE = u"MeasureTextFixedAngle";

inline constexpr OUStringLiteral UNO_NAME_CIRCKIND = u"CircleKind";
inline constexpr OUStringLiteral UNO_NAME_CIRCSTARTANGLE = u"CircleStartAngle";
inline constexpr OUStringLiteral UNO_NAME_CIRCENDANGLE = u"CircleEndAngle";

inline constexpr OUStringLiteral UNO_NAME_POLYGONKIND = u"PolygonKind";
inline constexpr OUStringLiteral UNO_NAME_POLYPOLYGON = u"PolyPolygon";
inline constexpr OUStringLiteral UNO_NAME_POLYPOLYGONBEZIER = u"PolyPolygonBezier";
inline constexpr OUStringLiteral UNO_NAME_POLYGON = u"Polygon";

inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_INTEROPGRABBAG = u"InteropGrabBag";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_ZORDER = u"ZOrder";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_MOVEPROTECT = u"MoveProtect";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_SIZEPROTECT = u"SizeProtect";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_PRINTABLE = u"Printable";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_LAYERID = u"LayerID";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_LAYERNAME = u"LayerName";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_NAME = u"Name";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_ROTATEANGLE = u"RotateAngle";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_SHEARANGLE = u"ShearAngle";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_FRAMERECT = u"FrameRect";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_BOUNDRECT = u"BoundRect";

// #i68101#
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_TITLE = u"Title";
inline constexpr OUStringLiteral UNO_NAME_MISC_OBJ_DESCRIPTION = u"Description";

inline constexpr OUStringLiteral UNO_NAME_HYPERLINK = u"Hyperlink";

inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_FILLBITMAP = u"GraphicObjectFillBitmap";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_REPLACEMENT_GRAPHIC = u"ReplacementGraphic";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_GRAFSTREAMURL = u"GraphicStreamURL";
#define UNO_NAME_GRAPHOBJ_URLPKGPREFIX          "vnd.sun.star.Package:"
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_GRAPHIC = u"Graphic";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_GRAPHIC_URL = u"GraphicURL";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_IS_SIGNATURELINE = u"IsSignatureLine";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_ID = u"SignatureLineId";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SUGGESTED_SIGNER_NAME = u"SignatureLineSuggestedSignerName";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SUGGESTED_SIGNER_TITLE = u"SignatureLineSuggestedSignerTitle";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SUGGESTED_SIGNER_EMAIL = u"SignatureLineSuggestedSignerEmail";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SIGNING_INSTRUCTIONS = u"SignatureLineSigningInstructions";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SHOW_SIGN_DATE = u"SignatureLineShowSignDate";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_CAN_ADD_COMMENT = u"SignatureLineCanAddComment";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_UNSIGNED_IMAGE = u"SignatureLineUnsignedImage";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_IS_SIGNED = u"SignatureLineIsSigned";
inline constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_QRCODE = u"BarCodeProperties";

inline constexpr OUStringLiteral UNO_NAME_OLE2_METAFILE = u"MetaFile";
inline constexpr OUStringLiteral UNO_NAME_OLE2_PERSISTNAME = u"PersistName";

inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_PERSPECTIVE = u"D3DScenePerspective";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_DISTANCE = u"D3DSceneDistance";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_FOCAL_LENGTH = u"D3DSceneFocalLength";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_TWO_SIDED_LIGHTING = u"D3DSceneTwoSidedLighting";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_1 = u"D3DSceneLightColor1";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_2 = u"D3DSceneLightColor2";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_3 = u"D3DSceneLightColor3";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_4 = u"D3DSceneLightColor4";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_5 = u"D3DSceneLightColor5";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_6 = u"D3DSceneLightColor6";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_7 = u"D3DSceneLightColor7";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_8 = u"D3DSceneLightColor8";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_1 = u"D3DSceneLightOn1";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_2 = u"D3DSceneLightOn2";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_3 = u"D3DSceneLightOn3";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_4 = u"D3DSceneLightOn4";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_5 = u"D3DSceneLightOn5";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_6 = u"D3DSceneLightOn6";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_7 = u"D3DSceneLightOn7";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_8 = u"D3DSceneLightOn8";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_1 = u"D3DSceneLightDirection1";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_2 = u"D3DSceneLightDirection2";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_3 = u"D3DSceneLightDirection3";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_4 = u"D3DSceneLightDirection4";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_5 = u"D3DSceneLightDirection5";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_6 = u"D3DSceneLightDirection6";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_7 = u"D3DSceneLightDirection7";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_8 = u"D3DSceneLightDirection8";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_AMBIENTCOLOR = u"D3DSceneAmbientColor";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_SHADOW_SLANT = u"D3DSceneShadowSlant";
inline constexpr OUStringLiteral UNO_NAME_3D_SCENE_SHADE_MODE = u"D3DSceneShadeMode";

inline constexpr OUStringLiteral UNO_NAME_3D_DOUBLE_SIDED = u"D3DDoubleSided";
inline constexpr OUStringLiteral UNO_NAME_3D_NORMALS_KIND = u"D3DNormalsKind";
inline constexpr OUStringLiteral UNO_NAME_3D_NORMALS_INVERT = u"D3DNormalsInvert";
inline constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_PROJ_X = u"D3DTextureProjectionX";
inline constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_PROJ_Y = u"D3DTextureProjectionY";
inline constexpr OUStringLiteral UNO_NAME_3D_SHADOW_3D = u"D3DShadow3D";
inline constexpr OUStringLiteral UNO_NAME_3D_MAT_COLOR = u"D3DMaterialColor";
inline constexpr OUStringLiteral UNO_NAME_3D_MAT_EMISSION = u"D3DMaterialEmission";
inline constexpr OUStringLiteral UNO_NAME_3D_MAT_SPECULAR = u"D3DMaterialSpecular";
inline constexpr OUStringLiteral UNO_NAME_3D_MAT_SPECULAR_INTENSITY = u"D3DMaterialSpecularIntensity";
inline constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_KIND = u"D3DTextureKind";
inline constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_MODE = u"D3DTextureMode";
inline constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_FILTER = u"D3DTextureFilter";

// #i28528#
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry
inline constexpr OUStringLiteral UNO_NAME_3D_REDUCED_LINE_GEOMETRY = u"D3DReducedLineGeometry";

inline constexpr OUStringLiteral UNO_NAME_3D_HORZ_SEGS = u"D3DHorizontalSegments";
inline constexpr OUStringLiteral UNO_NAME_3D_VERT_SEGS = u"D3DVerticalSegments";

inline constexpr OUStringLiteral UNO_NAME_3D_PERCENT_DIAGONAL = u"D3DPercentDiagonal";
inline constexpr OUStringLiteral UNO_NAME_3D_BACKSCALE = u"D3DBackscale";
inline constexpr OUStringLiteral UNO_NAME_3D_LATHE_END_ANGLE = u"D3DEndAngle";

inline constexpr OUStringLiteral UNO_NAME_3D_EXTRUDE_DEPTH = u"D3DDepth";

inline constexpr OUStringLiteral UNO_NAME_3D_TRANSFORM_MATRIX = u"D3DTransformMatrix";
inline constexpr OUStringLiteral UNO_NAME_3D_CAMERA_GEOMETRY = u"D3DCameraGeometry";
inline constexpr OUStringLiteral UNO_NAME_3D_POS = u"D3DPosition";
inline constexpr OUStringLiteral UNO_NAME_3D_SIZE = u"D3DSize";
inline constexpr OUStringLiteral UNO_NAME_3D_POS_IS_CENTER = u"D3DPositionIsCenter";
inline constexpr OUStringLiteral UNO_NAME_3D_POLYPOLYGON3D = u"D3DPolyPolygon3D";
inline constexpr OUStringLiteral UNO_NAME_3D_NORMALSPOLYGON3D = u"D3DNormalsPolygon3D";
inline constexpr OUStringLiteral UNO_NAME_3D_TEXTUREPOLYGON3D = u"D3DTexturePolygon3D";
inline constexpr OUStringLiteral UNO_NAME_3D_LINEONLY = u"D3DLineOnly";

// New 3D properties which are possible for lathe and extrude 3d objects
inline constexpr OUStringLiteral UNO_NAME_3D_SMOOTH_NORMALS = u"D3DSmoothNormals";
inline constexpr OUStringLiteral UNO_NAME_3D_SMOOTH_LIDS = u"D3DSmoothLids";
inline constexpr OUStringLiteral UNO_NAME_3D_CHARACTER_MODE = u"D3DCharacterMode";
inline constexpr OUStringLiteral UNO_NAME_3D_CLOSE_FRONT = u"D3DCloseFront";
inline constexpr OUStringLiteral UNO_NAME_3D_CLOSE_BACK = u"D3DCloseBack";

inline constexpr OUStringLiteral UNO_NAME_NUMBERING = u"NumberingIsNumber";
inline constexpr OUStringLiteral UNO_NAME_NUMBERING_RULES = u"NumberingRules";
inline constexpr OUStringLiteral UNO_NAME_NUMBERING_LEVEL = u"NumberingLevel";

inline constexpr OUStringLiteral UNO_NAME_NRULE_NUMBERINGTYPE = u"NumberingType";
inline constexpr OUStringLiteral UNO_NAME_NRULE_PREFIX = u"Prefix";
inline constexpr OUStringLiteral UNO_NAME_NRULE_SUFFIX = u"Suffix";
#define UNO_NAME_NRULE_BULLETID                 "BulletId"
inline constexpr OUStringLiteral UNO_NAME_NRULE_BULLET_COLOR = u"BulletColor";
inline constexpr OUStringLiteral UNO_NAME_NRULE_BULLET_RELSIZE = u"BulletRelSize";
inline constexpr OUStringLiteral UNO_NAME_NRULE_BULLET_FONT = u"BulletFont";
inline constexpr OUStringLiteral UNO_NAME_NRULE_START_WITH = u"StartWith";
inline constexpr OUStringLiteral UNO_NAME_NRULE_LEFT_MARGIN = u"LeftMargin";
inline constexpr OUStringLiteral UNO_NAME_NRULE_FIRST_LINE_OFFSET = u"FirstLineOffset";
inline constexpr OUStringLiteral UNO_NAME_NRULE_ADJUST = u"Adjust";

inline constexpr OUStringLiteral UNO_NAME_EDIT_FONT_DESCRIPTOR = u"FontDescriptor";

inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_ADJUST = u"ParaAdjust";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_BMARGIN = u"ParaBottomMargin";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_LASTLINEADJ = u"ParaLastLineAdjust";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_LMARGIN = u"ParaLeftMargin";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_LINESPACING = u"ParaLineSpacing";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_RMARGIN = u"ParaRightMargin";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_TAPSTOPS = u"ParaTabStops";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_TMARGIN = u"ParaTopMargin";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_IS_HYPHEN = u"ParaIsHyphenation";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION = u"ParaIsHangingPunctuation";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_IS_CHARACTER_DISTANCE = u"ParaIsCharacterDistance";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_IS_FORBIDDEN_RULES = u"ParaIsForbiddenRules";
inline constexpr OUStringLiteral UNO_NAME_EDIT_PARA_FIRST_LINE_INDENT = u"ParaFirstLineIndent";

inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_HEIGHT = u"CharHeight";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTNAME = u"CharFontName";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTSTYLENAME = u"CharFontStyleName";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTFAMILY = u"CharFontFamily";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTCHARSET = u"CharFontCharSet";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTPITCH = u"CharFontPitch";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_POSTURE = u"CharPosture";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_WEIGHT = u"CharWeight";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_LOCALE = u"CharLocale";

inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_HEIGHT_ASIAN = u"CharHeightAsian";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTNAME_ASIAN = u"CharFontNameAsian";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTSTYLENAME_ASIAN = u"CharFontStyleNameAsian";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTFAMILY_ASIAN = u"CharFontFamilyAsian";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTCHARSET_ASIAN = u"CharFontCharSetAsian";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTPITCH_ASIAN = u"CharFontPitchAsian";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_POSTURE_ASIAN = u"CharPostureAsian";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_WEIGHT_ASIAN = u"CharWeightAsian";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_LOCALE_ASIAN = u"CharLocaleAsian";

inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_HEIGHT_COMPLEX = u"CharHeightComplex";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTNAME_COMPLEX = u"CharFontNameComplex";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTSTYLENAME_COMPLEX = u"CharFontStyleNameComplex";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTFAMILY_COMPLEX = u"CharFontFamilyComplex";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTCHARSET_COMPLEX = u"CharFontCharSetComplex";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTPITCH_COMPLEX = u"CharFontPitchComplex";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_POSTURE_COMPLEX = u"CharPostureComplex";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_WEIGHT_COMPLEX = u"CharWeightComplex";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_LOCALE_COMPLEX = u"CharLocaleComplex";

inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_COLOR = u"CharColor";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_COLOR_THEME = u"CharColorTheme";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_COLOR_TINT_OR_SHADE = u"CharColorTintOrShade";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_COLOR_LUM_MOD = u"CharColorLumMod";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_COLOR_LUM_OFF = u"CharColorLumOff";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_TRANSPARENCE = u"CharTransparence";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_CROSSEDOUT = u"CharCrossedOut";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_STRIKEOUT = u"CharStrikeout";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_CASEMAP = u"CharCaseMap";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_ESCAPEMENT = u"CharEscapement";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_SHADOWED = u"CharShadowed";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_UNDERLINE = u"CharUnderline";
inline constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_OVERLINE = u"CharOverline";

inline constexpr OUStringLiteral UNO_NAME_BITMAP = u"Bitmap";

inline constexpr OUStringLiteral UNO_NAME_LINKDISPLAYNAME = u"LinkDisplayName";
inline constexpr OUStringLiteral UNO_NAME_LINKDISPLAYBITMAP = u"LinkDisplayBitmap";

inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_LUMINANCE = u"AdjustLuminance";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_CONTRAST = u"AdjustContrast";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_RED = u"AdjustRed";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_GREEN = u"AdjustGreen";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_BLUE = u"AdjustBlue";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_GAMMA = u"Gamma";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_TRANSPARENCY = u"Transparency";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_COLOR_MODE = u"GraphicColorMode";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_GRAPHICCROP = u"GraphicCrop";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
