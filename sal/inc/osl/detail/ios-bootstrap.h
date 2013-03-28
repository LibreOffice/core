/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IOS_BOOSTRAP_H
#define IOS_BOOSTRAP_H

#if defined(IOS)

#include <CoreGraphics/CoreGraphics.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <osl/detail/component-mapping.h>

/* These functions are the interface between the upper GUI layers of a
 * LibreOffice-based iOS app and the lower "core" layers, used in cases
 * where the core parts need to call functions in the upper parts or
 * vice versa.
 *
 * Thus there are two classes of functions here:
 *
 * 1) Those to be implemented in the upper layer and called by the
 * lower layer
 *
 * 2) Those implmented in the lower layers to be called by the upper
 * layer, in cases where we don't want to include a bunch of the
 * "normal" LibreOffice C++ headers in an otherwise purely Objective-C
 * CocoaTouch-based source file. Of course it depends on the case
 * where that is wanted, and this all is work in progress.
 */

/* 1) */

void lo_damaged(CGRect rect);

/* 2) */

void lo_runMain();
void lo_set_view_size(int width, int height);
void lo_render_windows(CGContextRef context, CGRect rect);
void lo_tap(int x, int y);

#ifdef __cplusplus
}
#endif

#endif // IOS
#endif // IOS_BOOTSTRAP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
