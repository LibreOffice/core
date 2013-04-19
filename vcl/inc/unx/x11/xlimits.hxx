/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_X11LIMITS
#define VCL_X11LIMITS

#include <X11/Xlib.h>
#include <vclpluginapi.h>

VCLPLUG_GEN_PUBLIC Pixmap limitXCreatePixmap(Display *display, Drawable d, unsigned int width, unsigned int height, unsigned int depth);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
