/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
