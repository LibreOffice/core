/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2013 LibreOffice contributors.
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
#include <basegfx/range/b2ibox.hxx>
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

typedef enum {
    MLOSelectionNone,
    MLOSelectionText,
    MLOSelectionGraphic
} MLOSelectionKind;

#ifdef IOS
typedef CGRect MLORect;
#else
typedef basegfx::B2IBox MLORect;
#endif

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

// 2) Those implmented in the lower layers to be called by the upper
// layer, in cases where we don't want to include a bunch of the
// "normal" LibreOffice C++ headers in an otherwise purely Objective-C
// CocoaTouch-based source file. Of course it depends on the case
// where that is wanted, and this all is work in progress. Prefixed by
// touch_lo_.

void touch_lo_keyboard_did_hide();

void touch_lo_runMain();
void touch_lo_set_view_size(int width, int height);
void touch_lo_render_windows(void *context, int minX, int minY, int width, int height);
void touch_lo_tap(int x, int y);
void touch_lo_pan(int deltaX, int deltaY);
void touch_lo_zoom(int x, int y, float scale);
void touch_lo_keyboard_input(int c);

/** Draw part of the document.

tilePosX, tilePosY, tileWidth, tileHeight address the part of the document to be drawn.
context, contextHeight, contextWidth specify where to draw.
*/
void touch_lo_draw_tile(void *context, int contextWidth, int contextHeight, int tilePosX, int tilePosY, int tileWidth, int tileHeight);

typedef enum { DOWN, MOVE, UP} MLOMouseButtonState;

void touch_lo_mouse_drag(int x, int y, MLOMouseButtonState state);

void touch_lo_selection_attempt_resize(const void *documentHandle,
                                       MLORect *selectedRectangles,
                                       int numberOfRectangles);

#ifdef __cplusplus
}
#endif

#endif // HAVE_FEATURE_DESKTOP

#endif // INCLUDED_TOUCH_TOUCH_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
