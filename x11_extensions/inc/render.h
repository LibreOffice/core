/*
 * $XFree86: xc/include/extensions/render.h,v 1.3 2000/11/20 07:13:09 keithp Exp $
 *
 * Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#ifndef _RENDER_H_
#define _RENDER_H_

typedef unsigned long   Glyph;
typedef unsigned long   GlyphSet;
typedef unsigned long   Picture;
typedef unsigned long   PictFormat;

#define RENDER_NAME "RENDER"
#define RENDER_MAJOR    0
#define RENDER_MINOR    1

#define X_RenderQueryVersion            0
#define X_RenderQueryPictFormats        1
#define X_RenderQueryPictIndexValues        2
#define X_RenderQueryDithers            3
#define X_RenderCreatePicture           4
#define X_RenderChangePicture           5
#define X_RenderSetPictureClipRectangles    6
#define X_RenderFreePicture         7
#define X_RenderComposite           8
#define X_RenderScale               9
#define X_RenderTrapezoids          10
#define X_RenderTriangles           11
#define X_RenderTriStrip            12
#define X_RenderTriFan              13
#define X_RenderColorTrapezoids         14
#define X_RenderColorTriangles          15
#define X_RenderTransform           16
#define X_RenderCreateGlyphSet          17
#define X_RenderReferenceGlyphSet       18
#define X_RenderFreeGlyphSet            19
#define X_RenderAddGlyphs           20
#define X_RenderAddGlyphsFromPicture        21
#define X_RenderFreeGlyphs          22
#define X_RenderCompositeGlyphs8        23
#define X_RenderCompositeGlyphs16       24
#define X_RenderCompositeGlyphs32       25
#define X_RenderFillRectangles          26

#define BadPictFormat               0
#define BadPicture              1
#define BadPictOp               2
#define BadGlyphSet             3
#define BadGlyph                4
#define RenderNumberErrors          (BadGlyph+1)

#define PictTypeIndexed             0
#define PictTypeDirect              1

#define PictOpClear             0
#define PictOpSrc               1
#define PictOpDst               2
#define PictOpOver              3
#define PictOpOverReverse           4
#define PictOpIn                5
#define PictOpInReverse             6
#define PictOpOut               7
#define PictOpOutReverse            8
#define PictOpAtop              9
#define PictOpAtopReverse           10
#define PictOpXor               11
#define PictOpAdd               12
#define PictOpSaturate              13
#define PictOpMaximum               13

#define PolyEdgeSharp               0
#define PolyEdgeSmooth              1

#define PolyModePrecise             0
#define PolyModeImprecise           1

#define CPRepeat                (1 << 0)
#define CPAlphaMap              (1 << 1)
#define CPAlphaXOrigin              (1 << 2)
#define CPAlphaYOrigin              (1 << 3)
#define CPClipXOrigin               (1 << 4)
#define CPClipYOrigin               (1 << 5)
#define CPClipMask              (1 << 6)
#define CPGraphicsExposure          (1 << 7)
#define CPSubwindowMode             (1 << 8)
#define CPPolyEdge              (1 << 9)
#define CPPolyMode              (1 << 10)
#define CPDither                (1 << 11)
#define CPComponentAlpha            (1 << 12)
#define CPLastBit               11

#endif  /* _RENDER_H_ */
