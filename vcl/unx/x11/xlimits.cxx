/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/log.hxx>
#include <unx/x11/xlimits.hxx>

Pixmap limitXCreatePixmap(Display *display, Drawable d, unsigned int width, unsigned int height, unsigned int depth)
{
    // The X protocol request CreatePixmap puts an upper bound
    // of 16 bit to the size. Beyond that there may be implementation
    // limits of the Xserver; which we should catch by a failed XCreatePixmap
    // call. However extra large values should be caught here since we'd run into
    // 16 bit truncation here without noticing.

    // see, e.g. moz#424333
    if (width > SAL_MAX_INT16 || height > SAL_MAX_INT16)
    {
        SAL_WARN("vcl", "overlarge pixmap: " << width << " x " << height);
        return None;
    }
    return XCreatePixmap(display, d, width, height, depth);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
