#*************************************************************************
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.2 $
#
#  last change: $Author: vg $ $Date: 2005-03-10 12:36:18 $
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright (c) 2003 by Sun Microsystems, Inc.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of Sun Microsystems, Inc. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#     
#**************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=rendering

TARGET=cssrendering
PACKAGE=com$/sun$/star$/rendering

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    PanoseFamilyTypes.idl \
    PanoseSerifStyle.idl \
    PanoseWeight.idl \
    PanoseProportion.idl \
    PanoseContrast.idl \
    PanoseStrokeVariation.idl \
    PanoseArmStyle.idl \
    PanoseLetterForm.idl \
    PanoseMidline.idl \
    PanoseXHeight.idl \
    Panose.idl \
    AnimationAttributes.idl \
    AnimationRepeat.idl \
    PathCapType.idl \
    PathJoinType.idl \
    Caret.idl \
    RenderState.idl \
    RenderingIntent.idl \
    ColorProfile.idl \
    ColorSpaceType.idl \
    XColorSpace.idl \
    RepaintResult.idl \
    CompositeOperation.idl \
    StringContext.idl \
    EmphasisMark.idl \
    StrokeAttributes.idl \
    Endianness.idl \
    TextDirection.idl \
    FillRule.idl \
    TextHit.idl \
    FloatingPointBitmapFormat.idl \
    Texture.idl \
    FloatingPointBitmapLayout.idl \
    TexturingMode.idl \
    XLinePolyPolygon2D.idl \
    FontInfo.idl \
    ViewState.idl \
    XParametricPolyPolygon2D.idl \
    VolatileContentDestroyedException.idl \
    XParametricPolyPolygon2DFactory.idl \
    FontMetrics.idl \
    XAnimatedSprite.idl \
    XPolyPolygon2D.idl \
    FontRequest.idl \
    XAnimation.idl \
    XSprite.idl \
    IntegerBitmapFormat.idl \
    XBezierPolyPolygon2D.idl \
    XSpriteCanvas.idl \
    IntegerBitmapLayout.idl \
    XBitmap.idl \
    XTextLayout.idl \
    XBitmapCanvas.idl \
    XVolatileBitmap.idl \
    InterpolationMode.idl \
    XBitmapPalette.idl \
    XBufferController.idl \
    XCachedPrimitive.idl \
    XGraphicDevice.idl \
    XCanvas.idl \
    XCustomSprite.idl \
    XCanvasFont.idl \
    XHalfFloatBitmap.idl \
    XIeeeDoubleBitmap.idl \
    XIeeeFloatBitmap.idl \
    XIntegerBitmap.idl \
    CanvasFactory.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
