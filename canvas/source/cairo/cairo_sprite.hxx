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



#ifndef _CAIROCANVAS_SPRITE_HXX
#define _CAIROCANVAS_SPRITE_HXX

#include <canvas/base/sprite.hxx>

#include "cairo_cairo.hxx"

namespace cairocanvas
{
    /** Specialization of ::canvas::Sprite interface, to also provide
        redraw methods.
     */
    class Sprite : public ::canvas::Sprite
    {
    public:

        /** Redraw sprite at the stored position.

            @param bBufferedUpdate
            When true, the redraw does <em>not</em> happen directly on
            the front buffer, but within a VDev. Used to speed up
            drawing.
         */
        virtual void redraw( const ::cairo::CairoSharedPtr& pCairo,
                             bool                           bBufferedUpdate ) const = 0;

        /** Redraw sprite at the given position.

            @param rPos
            Output position of the sprite. Overrides the sprite's own
            output position.

            @param bBufferedUpdate
            When true, the redraw does <em>not</em> happen directly on
            the front buffer, but within a VDev. Used to speed up
            drawing.
        */
        virtual void redraw( const ::cairo::CairoSharedPtr& pCairo,
                             const ::basegfx::B2DPoint&     rOrigOutputPos,
                             bool                           bBufferedUpdate ) const = 0;
    };
}

#endif /* _CAIROCANVAS_SPRITE_HXX */
