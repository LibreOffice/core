/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CANVAS_RENDERING_ICOLORBUFFER_HXX
#define INCLUDED_CANVAS_RENDERING_ICOLORBUFFER_HXX

#include <sal/types.h>

namespace canvas
{
    /** Interface for a raw memory pixel container

        Use this interface to represent a surface of raw pixel (e.g. a
        bitmap) to the canvas rendering framework.
     */
    struct IColorBuffer
    {
        /// The underlying pixel format for this buffer
        enum class Format
        {
            // 32-bit ARGB pixel format with alpha, 8 bits per channel.
            A8R8G8B8,

            // 32-bit RGB pixel format, 8 bits per channel.
            X8R8G8B8
        };

        virtual ~IColorBuffer() {}

        /** Get a pointer to the raw memory bits of the pixel
         */
        virtual sal_uInt8* lock() const = 0;

        /** unlock previous locked buffer
        */
        virtual void unlock() const = 0;

        /** Get width in pixel
         */
        virtual sal_uInt32 getWidth() const = 0;

        /** Get height in pixel
         */
        virtual sal_uInt32 getHeight() const = 0;

        /** Offset, in bytes, between consecutive scan lines of the bitmap.
            If the stride is positive, the bitmap is top-down.
            If the stride is negative, the bitmap is bottom-up.
            The returned value is only valid while the buffer is locked.
         */
        virtual sal_uInt32 getStride() const = 0;

        /** Get format of the color buffer
         */
        virtual Format getFormat() const = 0;
    };
}

#endif // INCLUDED_CANVAS_RENDERING_ICOLORBUFFER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
