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



#ifndef INCLUDED_CANVAS_ISURFACE_HXX
#define INCLUDED_CANVAS_ISURFACE_HXX

#include <sal/types.h>

#include <boost/shared_ptr.hpp>


namespace basegfx
{
    class B2IRange;
    class B2IPoint;
    class B2IVector;
}

namespace canvas
{
    struct IColorBuffer;

    struct ISurface
    {
        virtual ~ISurface() {}

        /** Select texture behind this interface to be the current one
            for primitive output.
         */
        virtual bool selectTexture() = 0;

        /** Tells whether the surface is valid or not
        */
        virtual bool isValid() = 0;

        /** Update surface content from given IColorBuffer

            This method updates the given subarea of the surface from
            the given color buffer bits.

            @param rDestPos
            Position in the surface, where the subset update should
            have its left, top edge

            @param rSourceRect
            Size and position of the rectangular subset update in the
            source color buffer

            @param rSource
            Source bits to use for the update

            @return true, if the update was successful
         */
        virtual bool update( const ::basegfx::B2IPoint& rDestPos,
                             const ::basegfx::B2IRange& rSourceRect,
                             IColorBuffer&              rSource ) = 0;

        virtual ::basegfx::B2IVector getSize() = 0;
    };

    typedef ::boost::shared_ptr< ISurface > ISurfaceSharedPtr;
}

#endif /* INCLUDED_CANVAS_ISURFACE_HXX */
