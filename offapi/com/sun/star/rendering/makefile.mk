#*************************************************************************
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.3 $
#
#  last change: $Author: obo $ $Date: 2005-06-15 10:27:20 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

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
