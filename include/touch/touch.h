/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TOUCH_TOUCH_H
#define INCLUDED_TOUCH_TOUCH_H

#include <config_features.h>

#define MOBILE_MAX_ZOOM_IN 600
#define MOBILE_MAX_ZOOM_OUT 80
#define MOBILE_ZOOM_SCALE_MULTIPLIER 10000

#if !HAVE_FEATURE_DESKTOP

// Let's try this way: Use Quartz 2D types for iOS, and LO's basegfx
// types for others, when/if this API is used for others. But of
// course, it is quite likely that some degree of redesign is needed
// at such a stage anyway...

#ifdef IOS
#include <premac.h>
#include <CoreGraphics/CoreGraphics.h>
#include <postmac.h>
#else
#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#endif

#ifdef __cplusplus
extern "C" {
#if 0
} // To avoid an editor indenting all inside the extern "C"
#endif
#endif

// These functions are the interface between the upper GUI layers of a
// LibreOffice-based app on a touch platform app and the lower "core"
// layers, used in cases where the core parts need to call
// functionality in the upper parts or vice versa.
//
// Thus there are two classes of functions here:
//
// 1) Those to be implemented in the upper layer and called by the
// lower layer. Prefixed by touch_ui_. The same API is used on each
// such platform. There are called from low level LibreOffice
// code. Note that these are just declared here in a header for a
// "touch" module, the per-platform implementations are elsewhere.

void touch_ui_damaged(int minX, int minY, int width, int height);

void touch_ui_show_keyboard();
void touch_ui_hide_keyboard();
bool touch_ui_keyboard_visible();

// Dialogs, work in progress, no clear plan yet what to do

typedef enum {
    MLODialogMessage,
    MLODialogInformation,
    MLODialogWarning,
    MLODialogError,
    MLODialogQuery
} MLODialogKind;

typedef enum {
    MLODialogOK,
    MLODialogCancel,
    MLODialogNo,
    MLODialogYes,
    MLODialogRetry,
    MLODialogIgnore,
} MLODialogResult;

MLODialogResult touch_ui_dialog_modal(MLODialogKind kind, const char *message);

typedef enum {
    MLOSelectionNone,
    MLOSelectionText,
    MLOSelectionGraphic
} MLOSelectionKind;

#ifdef IOS
typedef CGRect MLORect;
typedef CGFloat MLODpx;
typedef CGPoint MLODpxPoint;
typedef CGSize MLODpxSize;

#else

// Very much work in progress, just something to make this compile
typedef basegfx::B2DRange MLORect;
typedef float MLODpx;
typedef basegfx::B2DTuple MLODpxPoint;
typedef basegfx::B1DRange MLODpxSize;

#endif

// MLORip - tens of TWIPs, of questionable usefuless

typedef long long MLORip;

static const MLORip LO_TWIPS_TO_MLO_RIP_RATIO = 10L;

struct MLORipSize
{
    MLORip width;
    MLORip height;
};
typedef struct MLORipSize MLORipSize;

struct MLORipPoint
{
    MLORip x;
    MLORip y;
};
typedef struct MLORipPoint MLORipPoint;

static inline MLODpx
MLODpxByRip(MLORip rip)
{
    return (MLODpx) (rip / LO_TWIPS_TO_MLO_RIP_RATIO);
}

static inline MLORip
MLORipByDpx(MLODpx dpx)
{
    return (MLORip) (dpx * LO_TWIPS_TO_MLO_RIP_RATIO);
}

static inline MLODpxPoint
MLODpxPointByDpxes(MLODpx x, MLODpx y)
{
#ifdef IOS
    return CGPointMake(x, y);
#else
    return basegfx::B2DTuple(x, y);
#endif
}

#ifdef IOS

static inline MLODpxSize
MLODpxSizeByDpxes(MLODpx width, MLODpx height)
{
    return CGSizeMake(width, height);
}

static inline MLORipSize
MLORipSizeByRips(MLORip width, MLORip height)
{
    MLORipSize ripSize; ripSize.width = width; ripSize.height = height; return ripSize;
}

static inline MLORipPoint
MLORipPointByRips(MLORip x, MLORip y)
{
    MLORipPoint point; point.x = x; point.y = y; return point;
}

static inline MLORipSize
MLORipSizeByDpxSize(MLODpxSize dpxSize)
{
    MLORipSize ripSize;
    ripSize.width = MLORipByDpx(dpxSize.width);
    ripSize.height = MLORipByDpx(dpxSize.height);
    return ripSize;
}

static inline MLODpxSize
MLODpxSizeByRips(MLORip width, MLORip height)
{
    CGFloat fWidth = MLODpxByRip(width);
    CGFloat fHeight = MLODpxByRip(height);
    return CGSizeMake(fWidth, fHeight);
}

static inline MLODpxSize
MLODpxSizeByRipSize(MLORipSize ripSize)
{
    return MLODpxSizeByRips(ripSize.width, ripSize.height);
}

MLORipPoint MLORipPointByDpxPoint(MLODpxPoint mloDpxPoint);

MLODpxPoint MLODpxPointByMLORipPoint(MLORipPoint mloRipPoint);

#endif

// selection

void touch_ui_selection_start(MLOSelectionKind kind,
                              const void *documentHandle,
                              MLORect *rectangles,
                              int rectangleCount,
                              void *preview);

void touch_ui_selection_resize_done(bool success,
                                    const void *documentHandle,
                                    MLORect *rectangles,
                                    int rectangleCount);

void touch_ui_selection_none();

// 2) Those implemented in the lower layers to be called by the upper
// layer, in cases where we don't want to include a bunch of the
// "normal" LibreOffice C++ headers in an otherwise purely Objective-C
// CocoaTouch-based source file. Of course it depends on the case
// where that is wanted, and this all is work in progress. Prefixed by
// touch_lo_. All these are called on the UI thread and except for
// those so marked schedule work to be done asynchronously on the LO
// thread.

typedef enum { DOWN, MOVE, UP} MLOMouseButtonState;
typedef enum { NONE, SHIFT, META } MLOModifiers;
typedef int MLOModifierMask;

void touch_lo_keyboard_did_hide();

void touch_lo_set_view_size(int width, int height);
void touch_lo_tap(int x, int y);
void touch_lo_mouse(int x, int y, MLOMouseButtonState state, MLOModifierMask modifiers);
void touch_lo_pan(int deltaX, int deltaY);
void touch_lo_zoom(int x, int y, float scale);
void touch_lo_keyboard_input(int c);

/** Draw part of the document.

tilePosX, tilePosY, tileWidth, tileHeight address the part of the document to be drawn.
context, contextHeight, contextWidth specify where to draw.
*/
void touch_lo_draw_tile(void *context, int contextWidth, int contextHeight, MLODpxPoint tilePosition, MLODpxSize tileSize);
void touch_lo_copy_buffer(const void * source, size_t sourceWidth, size_t sourceHeight, size_t sourceBytesPerRow, void * target, size_t targetWidth, size_t targetHeight);
MLODpxSize touch_lo_get_content_size();
void touch_lo_mouse_drag(int x, int y, MLOMouseButtonState state);

// Move the start of the selection to (x,y)
void touch_lo_selection_start_move(const void *documentHandle,
                                   int x,
                                   int y);

// Move the end of the selection to (x,y)
void touch_lo_selection_end_move(const void *documentHandle,
                                 int x,
                                 int y);

void touch_lo_selection_attempt_resize(const void *documentHandle,
                                       MLORect *selectedRectangles,
                                       int numberOfRectangles);

// Special case: synchronous: waits for the rendering to complete
void touch_lo_render_windows(void *context, int minX, int minY, int width, int height);

// Special case: This is the function that is called in the newly
// created LO thread to run the LO code.
void touch_lo_runMain();


#ifdef __cplusplus
}
#endif

#endif // HAVE_FEATURE_DESKTOP

#endif // INCLUDED_TOUCH_TOUCH_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
