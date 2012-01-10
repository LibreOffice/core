#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssrendering
PACKAGE=com$/sun$/star$/rendering

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AnimationAttributes.idl \
    AnimationRepeat.idl \
    BlendMode.idl \
    CanvasFactory.idl \
    Caret.idl \
    ColorComponentTag.idl \
    ColorProfile.idl \
    ColorSpaceType.idl \
    CompositeOperation.idl \
    EmphasisMark.idl \
    FillRule.idl \
    FloatingPointBitmapFormat.idl \
    FloatingPointBitmapLayout.idl \
    FontInfo.idl \
    FontMetrics.idl \
    FontRequest.idl \
    IntegerBitmapLayout.idl \
    InterpolationMode.idl \
    Panose.idl \
    PanoseArmStyle.idl \
    PanoseContrast.idl \
    PanoseFamilyTypes.idl \
    PanoseLetterForm.idl \
    PanoseMidline.idl \
    PanoseProportion.idl \
    PanoseSerifStyle.idl \
    PanoseStrokeVariation.idl \
    PanoseWeight.idl \
    PanoseXHeight.idl \
    PathCapType.idl \
    PathJoinType.idl \
    RenderState.idl \
    RenderingIntent.idl \
    RepaintResult.idl \
    StringContext.idl \
    StrokeAttributes.idl \
    TextDirection.idl \
    TextHit.idl \
    Texture.idl \
    TexturingMode.idl \
    ViewState.idl \
    VolatileContentDestroyedException.idl \
    XAnimatedSprite.idl \
    XAnimation.idl \
    XBezierPolyPolygon2D.idl \
    XBitmap.idl \
    XBitmapCanvas.idl \
    XBitmapPalette.idl \
    XBufferController.idl \
    XCachedPrimitive.idl \
    XCanvas.idl \
    XCanvasFont.idl \
    XColorSpace.idl \
    XCustomSprite.idl \
    XGraphicDevice.idl \
    XHalfFloatBitmap.idl \
    XHalfFloatReadOnlyBitmap.idl \
    XIeeeDoubleBitmap.idl \
    XIeeeDoubleReadOnlyBitmap.idl \
    XIeeeFloatBitmap.idl \
    XIeeeFloatReadOnlyBitmap.idl \
    XIntegerBitmap.idl \
    XIntegerBitmapColorSpace.idl \
    XIntegerReadOnlyBitmap.idl \
    XLinePolyPolygon2D.idl \
    XParametricPolyPolygon2D.idl \
    XPolyPolygon2D.idl \
    XSimpleCanvas.idl \
    XSprite.idl \
    XSpriteCanvas.idl \
    XTextLayout.idl \
    XVolatileBitmap.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
