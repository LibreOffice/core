/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <sal/log.hxx>
#include <unx/x11/xlimits.hxx>

Pixmap limitXCreatePixmap(Display *display, Drawable d, unsigned int width, unsigned int height, unsigned int depth)
{
    
    
    
    
    
    //
    
    if (width > SAL_MAX_INT16 || height > SAL_MAX_INT16)
    {
        SAL_WARN("vcl", "overlarge pixmap: " << width << " x " << height);
        return None;
    }
    return XCreatePixmap(display, d, width, height, depth);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
