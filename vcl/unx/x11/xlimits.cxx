/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
    //
    // see, e.g. moz#424333
    if (width > SAL_MAX_INT16 || height > SAL_MAX_INT16)
    {
        SAL_WARN("vcl", "overlarge pixmap: " << width << " x " << height);
        return None;
    }
    return XCreatePixmap(display, d, width, height, depth);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
