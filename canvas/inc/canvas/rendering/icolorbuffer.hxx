/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: icolorbuffer.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_CANVAS_ICOLORBUFFER_HXX
#define INCLUDED_CANVAS_ICOLORBUFFER_HXX

#include <sal/types.h>

#include <boost/shared_ptr.hpp>


namespace canvas
{
    /** Interface for a raw memory pixel container

        Use this interface to represent a surface of raw pixel (e.g. a
        bitmap) to the canvas rendering framework.
     */
    struct IColorBuffer
    {
        /// The underlying pixel format for this buffer
        enum Format
        {
            // 24-bit RGB pixel format, 8 bits per channel.
            FMT_R8G8B8,

            // 32-bit ARGB pixel format with alpha, 8 bits per channel.
            FMT_A8R8G8B8,

            // 32-bit RGB pixel format, 8 bits per channel.
            FMT_X8R8G8B8,

            // for enum to 32bit
            FMT_UNKNOWN = static_cast<sal_uInt32>(-1)
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

    typedef ::boost::shared_ptr< IColorBuffer > IColorBufferSharedPtr;
}

#endif /* INCLUDED_CANVAS_ICOLORBUFFER_HXX */
