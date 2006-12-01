#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 17:11:37 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
