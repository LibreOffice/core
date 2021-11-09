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

#ifndef INCLUDED_EDITENG_UNOPRNMS_HXX
#define INCLUDED_EDITENG_UNOPRNMS_HXX


constexpr OUStringLiteral UNO_NAME_CHAR_COLOR = u"CharColor";
constexpr OUStringLiteral UNO_NAME_CHAR_HEIGHT = u"CharHeight";
constexpr OUStringLiteral UNO_NAME_CHAR_POSTURE = u"CharPosture";
constexpr OUStringLiteral UNO_NAME_CHAR_SHADOWED = u"CharShadowed";
constexpr OUStringLiteral UNO_NAME_CHAR_UNDERLINE = u"CharUnderline";
constexpr OUStringLiteral UNO_NAME_CHAR_WEIGHT = u"CharWeight";

constexpr OUStringLiteral UNO_NAME_FILLSTYLE = u"FillStyle";
constexpr OUStringLiteral UNO_NAME_FILLCOLOR = u"FillColor";
constexpr OUStringLiteral UNO_NAME_FILLGRADIENT = u"FillGradient";
constexpr OUStringLiteral UNO_NAME_FILLGRADIENTNAME = u"FillGradientName";
constexpr OUStringLiteral UNO_NAME_FILLHATCH = u"FillHatch";
constexpr OUStringLiteral UNO_NAME_FILLHATCHNAME = u"FillHatchName";
constexpr OUStringLiteral UNO_NAME_FILLBITMAP = u"FillBitmap";
constexpr OUStringLiteral UNO_NAME_FILLBITMAPURL = u"FillBitmapURL";
constexpr OUStringLiteral UNO_NAME_FILLBITMAPNAME = u"FillBitmapName";
constexpr OUStringLiteral UNO_NAME_FILLGRADIENTSTEPCOUNT = u"FillGradientStepCount";
constexpr OUStringLiteral UNO_NAME_FILLBACKGROUND = u"FillBackground";
constexpr OUStringLiteral UNO_NAME_FILLCOLOR_2 = u"FillColor2";

constexpr OUStringLiteral UNO_NAME_EDGEKIND = u"EdgeKind";
constexpr OUStringLiteral UNO_NAME_EDGELINE1DELTA = u"EdgeLine1Delta";
constexpr OUStringLiteral UNO_NAME_EDGELINE2DELTA = u"EdgeLine2Delta";
constexpr OUStringLiteral UNO_NAME_EDGELINE3DELTA = u"EdgeLine3Delta";
constexpr OUStringLiteral UNO_NAME_EDGENODE1HORZDIST = u"EdgeNode1HorzDist";
constexpr OUStringLiteral UNO_NAME_EDGENODE1VERTDIST = u"EdgeNode1VertDist";
constexpr OUStringLiteral UNO_NAME_EDGENODE2HORZDIST = u"EdgeNode2HorzDist";
constexpr OUStringLiteral UNO_NAME_EDGENODE2VERTDIST = u"EdgeNode2VertDist";

constexpr OUStringLiteral UNO_NAME_FILLBMP_OFFSET_X = u"FillBitmapOffsetX";
constexpr OUStringLiteral UNO_NAME_FILLBMP_OFFSET_Y = u"FillBitmapOffsetY";
constexpr OUStringLiteral UNO_NAME_FILLBMP_POSITION_OFFSET_X = u"FillBitmapPositionOffsetX";
constexpr OUStringLiteral UNO_NAME_FILLBMP_POSITION_OFFSET_Y = u"FillBitmapPositionOffsetY";
constexpr OUStringLiteral UNO_NAME_FILLBMP_RECTANGLE_POINT = u"FillBitmapRectanglePoint";
constexpr OUStringLiteral UNO_NAME_FILLBMP_TILE = u"FillBitmapTile";
constexpr OUStringLiteral UNO_NAME_FILLBMP_STRETCH = u"FillBitmapStretch";
constexpr OUStringLiteral UNO_NAME_FILLBMP_LOGICAL_SIZE = u"FillBitmapLogicalSize";
constexpr OUStringLiteral UNO_NAME_FILLBMP_SIZE_X = u"FillBitmapSizeX";
constexpr OUStringLiteral UNO_NAME_FILLBMP_SIZE_Y = u"FillBitmapSizeY";
constexpr OUStringLiteral UNO_NAME_FILL_TRANSPARENCE = u"FillTransparence";
constexpr OUStringLiteral UNO_NAME_FILLTRANSPARENCEGRADIENT = u"FillTransparenceGradient";
constexpr OUStringLiteral UNO_NAME_FILLTRANSPARENCEGRADIENTNAME = u"FillTransparenceGradientName";
constexpr OUStringLiteral UNO_NAME_FILLBMP_MODE = u"FillBitmapMode";


constexpr OUStringLiteral UNO_NAME_LINESTYLE = u"LineStyle";
constexpr OUStringLiteral UNO_NAME_LINEDASH = u"LineDash";
constexpr OUStringLiteral UNO_NAME_LINEWIDTH = u"LineWidth";
constexpr OUStringLiteral UNO_NAME_LINECOLOR = u"LineColor";
constexpr OUStringLiteral UNO_NAME_LINEJOINT = u"LineJoint";
constexpr OUStringLiteral UNO_NAME_LINESTART = u"LineStart";
constexpr OUStringLiteral UNO_NAME_LINEEND = u"LineEnd";
constexpr OUStringLiteral UNO_NAME_LINESTARTWIDTH = u"LineStartWidth";
constexpr OUStringLiteral UNO_NAME_LINEENDWIDTH = u"LineEndWidth";
constexpr OUStringLiteral UNO_NAME_LINESTARTCENTER = u"LineStartCenter";
constexpr OUStringLiteral UNO_NAME_LINEENDCENTER = u"LineEndCenter";
constexpr OUStringLiteral UNO_NAME_LINETRANSPARENCE = u"LineTransparence";
constexpr OUStringLiteral UNO_NAME_LINECAP = u"LineCap";

constexpr OUStringLiteral UNO_NAME_SHADOW = u"Shadow";
constexpr OUStringLiteral UNO_NAME_SHADOWCOLOR = u"ShadowColor";
constexpr OUStringLiteral UNO_NAME_SHADOWXDIST = u"ShadowXDistance";
constexpr OUStringLiteral UNO_NAME_SHADOWYDIST = u"ShadowYDistance";
constexpr OUStringLiteral UNO_NAME_SHADOWSIZEX = u"ShadowSizeX";
constexpr OUStringLiteral UNO_NAME_SHADOWSIZEY = u"ShadowSizeY";
constexpr OUStringLiteral UNO_NAME_SHADOWTRANSPARENCE = u"ShadowTransparence";
constexpr OUStringLiteral UNO_NAME_SHADOWBLUR = u"ShadowBlur";

constexpr OUStringLiteral UNO_NAME_EDGERADIUS = u"CornerRadius";

constexpr OUStringLiteral UNO_NAME_TEXT_MINFRAMEHEIGHT = u"TextMinimumFrameHeight";
constexpr OUStringLiteral UNO_NAME_TEXT_AUTOGROWHEIGHT = u"TextAutoGrowHeight";
constexpr OUStringLiteral UNO_NAME_TEXT_FITTOSIZE = u"TextFitToSize";
constexpr OUStringLiteral UNO_NAME_TEXT_LEFTDIST = u"TextLeftDistance";
constexpr OUStringLiteral UNO_NAME_TEXT_RIGHTDIST = u"TextRightDistance";
constexpr OUStringLiteral UNO_NAME_TEXT_UPPERDIST = u"TextUpperDistance";
constexpr OUStringLiteral UNO_NAME_TEXT_LOWERDIST = u"TextLowerDistance";
constexpr OUStringLiteral UNO_NAME_TEXT_VERTADJUST = u"TextVerticalAdjust";
constexpr OUStringLiteral UNO_NAME_TEXT_MAXFRAMEHEIGHT = u"TextMaximumFrameHeight";
constexpr OUStringLiteral UNO_NAME_TEXT_MINFRAMEWIDTH = u"TextMinimumFrameWidth";
constexpr OUStringLiteral UNO_NAME_TEXT_MAXFRAMEWIDTH = u"TextMaximumFrameWidth";
constexpr OUStringLiteral UNO_NAME_TEXT_AUTOGROWWIDTH = u"TextAutoGrowWidth";
constexpr OUStringLiteral UNO_NAME_TEXT_HORZADJUST = u"TextHorizontalAdjust";
constexpr OUStringLiteral UNO_NAME_TEXT_ANIKIND = u"TextAnimationKind";
constexpr OUStringLiteral UNO_NAME_TEXT_ANIDIRECTION = u"TextAnimationDirection";
constexpr OUStringLiteral UNO_NAME_TEXT_ANISTARTINSIDE = u"TextAnimationStartInside";
constexpr OUStringLiteral UNO_NAME_TEXT_ANISTOPINSIDE = u"TextAnimationStopInside";
constexpr OUStringLiteral UNO_NAME_TEXT_ANICOUNT = u"TextAnimationCount";
constexpr OUStringLiteral UNO_NAME_TEXT_ANIDELAY = u"TextAnimationDelay";
constexpr OUStringLiteral UNO_NAME_TEXT_ANIAMOUNT = u"TextAnimationAmount";
constexpr OUStringLiteral UNO_NAME_TEXT_CONTOURFRAME = u"TextContourFrame";
constexpr OUStringLiteral UNO_NAME_TEXT_WRITINGMODE = u"TextWritingMode";
constexpr OUStringLiteral UNO_NAME_TEXT_FONTINDEPENDENTLINESPACING = u"FontIndependentLineSpacing";
constexpr OUStringLiteral UNO_NAME_TEXT_WORDWRAP = u"TextWordWrap";
constexpr OUStringLiteral UNO_NAME_TEXT_CHAINNEXTNAME = u"TextChainNextName";

constexpr OUStringLiteral UNO_NAME_MEASUREKIND = u"MeasureKind";
constexpr OUStringLiteral UNO_NAME_MEASURETEXTHPOS = u"MeasureTextHorizontalPosition";
constexpr OUStringLiteral UNO_NAME_MEASURETEXTVPOS = u"MeasureTextVerticalPosition";
constexpr OUStringLiteral UNO_NAME_MEASURELINEDIST = u"MeasureLineDistance";
constexpr OUStringLiteral UNO_NAME_MEASUREHELPLINEOVERHANG = u"MeasureHelpLineOverhang";
constexpr OUStringLiteral UNO_NAME_MEASUREHELPLINEDIST = u"MeasureHelpLineDistance";
constexpr OUStringLiteral UNO_NAME_MEASUREHELPLINE1LEN = u"MeasureHelpLine1Length";
constexpr OUStringLiteral UNO_NAME_MEASUREHELPLINE2LEN = u"MeasureHelpLine2Length";
constexpr OUStringLiteral UNO_NAME_MEASUREBELOWREFEDGE = u"MeasureBelowReferenceEdge";
constexpr OUStringLiteral UNO_NAME_MEASURETEXTROTA90 = u"MeasureTextRotate90";
constexpr OUStringLiteral UNO_NAME_MEASURETEXTUPSIDEDOWN = u"MeasureTextUpsideDown";
constexpr OUStringLiteral UNO_NAME_MEASUREOVERHANG = u"MeasureOverhang";
constexpr OUStringLiteral UNO_NAME_MEASUREUNIT = u"MeasureUnit";
constexpr OUStringLiteral UNO_NAME_MEASURESHOWUNIT = u"MeasureShowUnit";
constexpr OUStringLiteral UNO_NAME_MEASUREFORMATSTRING = u"MeasureFormatString";
constexpr OUStringLiteral UNO_NAME_MEASURETEXTAUTOANGLE = u"MeasureTextAutoAngle";
constexpr OUStringLiteral UNO_NAME_MEASURETEXTAUTOANGLEVIEW = u"MeasureTextAutoAngleView";
constexpr OUStringLiteral UNO_NAME_MEASURETEXTISFIXEDANGLE = u"MeasureTextIsFixedAngle";
constexpr OUStringLiteral UNO_NAME_MEASURETEXTFIXEDANGLE = u"MeasureTextFixedAngle";

constexpr OUStringLiteral UNO_NAME_CIRCKIND = u"CircleKind";
constexpr OUStringLiteral UNO_NAME_CIRCSTARTANGLE = u"CircleStartAngle";
constexpr OUStringLiteral UNO_NAME_CIRCENDANGLE = u"CircleEndAngle";

constexpr OUStringLiteral UNO_NAME_POLYGONKIND = u"PolygonKind";
constexpr OUStringLiteral UNO_NAME_POLYPOLYGON = u"PolyPolygon";
constexpr OUStringLiteral UNO_NAME_POLYPOLYGONBEZIER = u"PolyPolygonBezier";
constexpr OUStringLiteral UNO_NAME_POLYGON = u"Polygon";

constexpr OUStringLiteral UNO_NAME_MISC_OBJ_INTEROPGRABBAG = u"InteropGrabBag";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_ZORDER = u"ZOrder";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_MOVEPROTECT = u"MoveProtect";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_SIZEPROTECT = u"SizeProtect";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_PRINTABLE = u"Printable";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_LAYERID = u"LayerID";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_LAYERNAME = u"LayerName";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_NAME = u"Name";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_ROTATEANGLE = u"RotateAngle";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_SHEARANGLE = u"ShearAngle";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_FRAMERECT = u"FrameRect";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_BOUNDRECT = u"BoundRect";

// #i68101#
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_TITLE = u"Title";
constexpr OUStringLiteral UNO_NAME_MISC_OBJ_DESCRIPTION = u"Description";

constexpr OUStringLiteral UNO_NAME_HYPERLINK = u"Hyperlink";

constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_FILLBITMAP = u"GraphicObjectFillBitmap";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_REPLACEMENT_GRAPHIC = u"ReplacementGraphic";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_GRAFSTREAMURL = u"GraphicStreamURL";
#define UNO_NAME_GRAPHOBJ_URLPKGPREFIX          "vnd.sun.star.Package:"
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_GRAPHIC = u"Graphic";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_GRAPHIC_URL = u"GraphicURL";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_IS_SIGNATURELINE = u"IsSignatureLine";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_ID = u"SignatureLineId";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SUGGESTED_SIGNER_NAME = u"SignatureLineSuggestedSignerName";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SUGGESTED_SIGNER_TITLE = u"SignatureLineSuggestedSignerTitle";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SUGGESTED_SIGNER_EMAIL = u"SignatureLineSuggestedSignerEmail";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SIGNING_INSTRUCTIONS = u"SignatureLineSigningInstructions";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_SHOW_SIGN_DATE = u"SignatureLineShowSignDate";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_CAN_ADD_COMMENT = u"SignatureLineCanAddComment";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_UNSIGNED_IMAGE = u"SignatureLineUnsignedImage";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_SIGNATURELINE_IS_SIGNED = u"SignatureLineIsSigned";
constexpr OUStringLiteral UNO_NAME_GRAPHOBJ_QRCODE = u"BarCodeProperties";

constexpr OUStringLiteral UNO_NAME_OLE2_METAFILE = u"MetaFile";
constexpr OUStringLiteral UNO_NAME_OLE2_PERSISTNAME = u"PersistName";

constexpr OUStringLiteral UNO_NAME_3D_SCENE_PERSPECTIVE = u"D3DScenePerspective";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_DISTANCE = u"D3DSceneDistance";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_FOCAL_LENGTH = u"D3DSceneFocalLength";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_TWO_SIDED_LIGHTING = u"D3DSceneTwoSidedLighting";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_1 = u"D3DSceneLightColor1";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_2 = u"D3DSceneLightColor2";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_3 = u"D3DSceneLightColor3";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_4 = u"D3DSceneLightColor4";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_5 = u"D3DSceneLightColor5";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_6 = u"D3DSceneLightColor6";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_7 = u"D3DSceneLightColor7";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTCOLOR_8 = u"D3DSceneLightColor8";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_1 = u"D3DSceneLightOn1";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_2 = u"D3DSceneLightOn2";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_3 = u"D3DSceneLightOn3";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_4 = u"D3DSceneLightOn4";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_5 = u"D3DSceneLightOn5";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_6 = u"D3DSceneLightOn6";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_7 = u"D3DSceneLightOn7";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTON_8 = u"D3DSceneLightOn8";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_1 = u"D3DSceneLightDirection1";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_2 = u"D3DSceneLightDirection2";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_3 = u"D3DSceneLightDirection3";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_4 = u"D3DSceneLightDirection4";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_5 = u"D3DSceneLightDirection5";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_6 = u"D3DSceneLightDirection6";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_7 = u"D3DSceneLightDirection7";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_LIGHTDIRECTION_8 = u"D3DSceneLightDirection8";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_AMBIENTCOLOR = u"D3DSceneAmbientColor";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_SHADOW_SLANT = u"D3DSceneShadowSlant";
constexpr OUStringLiteral UNO_NAME_3D_SCENE_SHADE_MODE = u"D3DSceneShadeMode";

constexpr OUStringLiteral UNO_NAME_3D_DOUBLE_SIDED = u"D3DDoubleSided";
constexpr OUStringLiteral UNO_NAME_3D_NORMALS_KIND = u"D3DNormalsKind";
constexpr OUStringLiteral UNO_NAME_3D_NORMALS_INVERT = u"D3DNormalsInvert";
constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_PROJ_X = u"D3DTextureProjectionX";
constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_PROJ_Y = u"D3DTextureProjectionY";
constexpr OUStringLiteral UNO_NAME_3D_SHADOW_3D = u"D3DShadow3D";
constexpr OUStringLiteral UNO_NAME_3D_MAT_COLOR = u"D3DMaterialColor";
constexpr OUStringLiteral UNO_NAME_3D_MAT_EMISSION = u"D3DMaterialEmission";
constexpr OUStringLiteral UNO_NAME_3D_MAT_SPECULAR = u"D3DMaterialSpecular";
constexpr OUStringLiteral UNO_NAME_3D_MAT_SPECULAR_INTENSITY = u"D3DMaterialSpecularIntensity";
constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_KIND = u"D3DTextureKind";
constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_MODE = u"D3DTextureMode";
constexpr OUStringLiteral UNO_NAME_3D_TEXTURE_FILTER = u"D3DTextureFilter";

// #i28528#
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry
constexpr OUStringLiteral UNO_NAME_3D_REDUCED_LINE_GEOMETRY = u"D3DReducedLineGeometry";

constexpr OUStringLiteral UNO_NAME_3D_HORZ_SEGS = u"D3DHorizontalSegments";
constexpr OUStringLiteral UNO_NAME_3D_VERT_SEGS = u"D3DVerticalSegments";

constexpr OUStringLiteral UNO_NAME_3D_PERCENT_DIAGONAL = u"D3DPercentDiagonal";
constexpr OUStringLiteral UNO_NAME_3D_BACKSCALE = u"D3DBackscale";
constexpr OUStringLiteral UNO_NAME_3D_LATHE_END_ANGLE = u"D3DEndAngle";

constexpr OUStringLiteral UNO_NAME_3D_EXTRUDE_DEPTH = u"D3DDepth";

constexpr OUStringLiteral UNO_NAME_3D_TRANSFORM_MATRIX = u"D3DTransformMatrix";
constexpr OUStringLiteral UNO_NAME_3D_CAMERA_GEOMETRY = u"D3DCameraGeometry";
constexpr OUStringLiteral UNO_NAME_3D_POS = u"D3DPosition";
constexpr OUStringLiteral UNO_NAME_3D_SIZE = u"D3DSize";
constexpr OUStringLiteral UNO_NAME_3D_POS_IS_CENTER = u"D3DPositionIsCenter";
constexpr OUStringLiteral UNO_NAME_3D_POLYPOLYGON3D = u"D3DPolyPolygon3D";
constexpr OUStringLiteral UNO_NAME_3D_NORMALSPOLYGON3D = u"D3DNormalsPolygon3D";
constexpr OUStringLiteral UNO_NAME_3D_TEXTUREPOLYGON3D = u"D3DTexturePolygon3D";
constexpr OUStringLiteral UNO_NAME_3D_LINEONLY = u"D3DLineOnly";

// New 3D properties which are possible for lathe and extrude 3d objects
constexpr OUStringLiteral UNO_NAME_3D_SMOOTH_NORMALS = u"D3DSmoothNormals";
constexpr OUStringLiteral UNO_NAME_3D_SMOOTH_LIDS = u"D3DSmoothLids";
constexpr OUStringLiteral UNO_NAME_3D_CHARACTER_MODE = u"D3DCharacterMode";
constexpr OUStringLiteral UNO_NAME_3D_CLOSE_FRONT = u"D3DCloseFront";
constexpr OUStringLiteral UNO_NAME_3D_CLOSE_BACK = u"D3DCloseBack";

constexpr OUStringLiteral UNO_NAME_NUMBERING = u"NumberingIsNumber";
constexpr OUStringLiteral UNO_NAME_NUMBERING_RULES = u"NumberingRules";
constexpr OUStringLiteral UNO_NAME_NUMBERING_LEVEL = u"NumberingLevel";

constexpr OUStringLiteral UNO_NAME_NRULE_NUMBERINGTYPE = u"NumberingType";
constexpr OUStringLiteral UNO_NAME_NRULE_PREFIX = u"Prefix";
constexpr OUStringLiteral UNO_NAME_NRULE_SUFFIX = u"Suffix";
#define UNO_NAME_NRULE_BULLETID                 "BulletId"
constexpr OUStringLiteral UNO_NAME_NRULE_BULLET_COLOR = u"BulletColor";
constexpr OUStringLiteral UNO_NAME_NRULE_BULLET_RELSIZE = u"BulletRelSize";
constexpr OUStringLiteral UNO_NAME_NRULE_BULLET_FONT = u"BulletFont";
constexpr OUStringLiteral UNO_NAME_NRULE_START_WITH = u"StartWith";
constexpr OUStringLiteral UNO_NAME_NRULE_LEFT_MARGIN = u"LeftMargin";
constexpr OUStringLiteral UNO_NAME_NRULE_FIRST_LINE_OFFSET = u"FirstLineOffset";
constexpr OUStringLiteral UNO_NAME_NRULE_ADJUST = u"Adjust";

constexpr OUStringLiteral UNO_NAME_EDIT_FONT_DESCRIPTOR = u"FontDescriptor";

constexpr OUStringLiteral UNO_NAME_EDIT_PARA_ADJUST = u"ParaAdjust";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_BMARGIN = u"ParaBottomMargin";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_LASTLINEADJ = u"ParaLastLineAdjust";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_LMARGIN = u"ParaLeftMargin";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_LINESPACING = u"ParaLineSpacing";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_RMARGIN = u"ParaRightMargin";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_TAPSTOPS = u"ParaTabStops";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_TMARGIN = u"ParaTopMargin";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_IS_HYPHEN = u"ParaIsHyphenation";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION = u"ParaIsHangingPunctuation";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_IS_CHARACTER_DISTANCE = u"ParaIsCharacterDistance";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_IS_FORBIDDEN_RULES = u"ParaIsForbiddenRules";
constexpr OUStringLiteral UNO_NAME_EDIT_PARA_FIRST_LINE_INDENT = u"ParaFirstLineIndent";

constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_HEIGHT = u"CharHeight";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTNAME = u"CharFontName";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTSTYLENAME = u"CharFontStyleName";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTFAMILY = u"CharFontFamily";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTCHARSET = u"CharFontCharSet";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTPITCH = u"CharFontPitch";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_POSTURE = u"CharPosture";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_WEIGHT = u"CharWeight";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_LOCALE = u"CharLocale";

constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_HEIGHT_ASIAN = u"CharHeightAsian";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTNAME_ASIAN = u"CharFontNameAsian";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTSTYLENAME_ASIAN = u"CharFontStyleNameAsian";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTFAMILY_ASIAN = u"CharFontFamilyAsian";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTCHARSET_ASIAN = u"CharFontCharSetAsian";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTPITCH_ASIAN = u"CharFontPitchAsian";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_POSTURE_ASIAN = u"CharPostureAsian";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_WEIGHT_ASIAN = u"CharWeightAsian";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_LOCALE_ASIAN = u"CharLocaleAsian";

constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_HEIGHT_COMPLEX = u"CharHeightComplex";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTNAME_COMPLEX = u"CharFontNameComplex";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTSTYLENAME_COMPLEX = u"CharFontStyleNameComplex";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTFAMILY_COMPLEX = u"CharFontFamilyComplex";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTCHARSET_COMPLEX = u"CharFontCharSetComplex";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_FONTPITCH_COMPLEX = u"CharFontPitchComplex";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_POSTURE_COMPLEX = u"CharPostureComplex";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_WEIGHT_COMPLEX = u"CharWeightComplex";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_LOCALE_COMPLEX = u"CharLocaleComplex";

constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_COLOR = u"CharColor";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_TRANSPARENCE = u"CharTransparence";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_CROSSEDOUT = u"CharCrossedOut";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_STRIKEOUT = u"CharStrikeout";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_CASEMAP = u"CharCaseMap";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_ESCAPEMENT = u"CharEscapement";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_SHADOWED = u"CharShadowed";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_UNDERLINE = u"CharUnderline";
constexpr OUStringLiteral UNO_NAME_EDIT_CHAR_OVERLINE = u"CharOverline";

constexpr OUStringLiteral UNO_NAME_BITMAP = u"Bitmap";

constexpr OUStringLiteral UNO_NAME_LINKDISPLAYNAME = u"LinkDisplayName";
constexpr OUStringLiteral UNO_NAME_LINKDISPLAYBITMAP = u"LinkDisplayBitmap";

constexpr OUStringLiteral UNO_NAME_GRAPHIC_LUMINANCE = u"AdjustLuminance";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_CONTRAST = u"AdjustContrast";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_RED = u"AdjustRed";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_GREEN = u"AdjustGreen";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_BLUE = u"AdjustBlue";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_GAMMA = u"Gamma";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_TRANSPARENCY = u"Transparency";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_COLOR_MODE = u"GraphicColorMode";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_GRAPHICCROP = u"GraphicCrop";

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
