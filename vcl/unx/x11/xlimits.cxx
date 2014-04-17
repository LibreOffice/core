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
    // of 16 bit to the size. And in practice some drivers
    // fall over with values close to the max.

    // see, e.g. moz#424333, fdo#48961, rhbz#1086714
    // we've a duplicate of this in canvas :-(
    if (width > SAL_MAX_INT16-10 || height > SAL_MAX_INT16-10)
    {
        SAL_WARN("vcl", "overlarge pixmap: " << width << " x " << height);
        return None;
    }
    return XCreatePixmap(display, d, width, height, depth);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
