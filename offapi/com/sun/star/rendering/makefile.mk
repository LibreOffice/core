#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.9 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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
