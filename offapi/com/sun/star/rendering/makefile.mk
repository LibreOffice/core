#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: kz $ $Date: 2008-04-04 16:13:20 $
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
    AnimationAttributes.idl \
    AnimationRepeat.idl \
    BlendMode.idl \
    CanvasFactory.idl \
    Caret.idl \
    ColorProfile.idl \
    ColorSpaceType.idl \
    CompositeOperation.idl \
    EmphasisMark.idl \
    Endianness.idl \
    FillRule.idl \
    FloatingPointBitmapFormat.idl \
    FloatingPointBitmapLayout.idl \
    FontInfo.idl \
    FontMetrics.idl \
    FontRequest.idl \
    IntegerBitmapFormat.idl \
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
    XIeeeDoubleBitmap.idl \
    XIeeeFloatBitmap.idl \
    XIntegerBitmap.idl \
    XLinePolyPolygon2D.idl \
    XParametricPolyPolygon2D.idl \
    XParametricPolyPolygon2DFactory.idl \
    XPolyPolygon2D.idl \
    XSimpleCanvas.idl \
    XSprite.idl \
    XSpriteCanvas.idl \
    XTextLayout.idl \
    XVolatileBitmap.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
