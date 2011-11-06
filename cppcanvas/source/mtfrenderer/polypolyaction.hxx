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



#ifndef _CPPCANVAS_POLYPOLYACTION_HXX
#define _CPPCANVAS_POLYPOLYACTION_HXX

#include <action.hxx>
#include <cppcanvas/canvas.hxx>


namespace basegfx {
    class B2DPolyPolygon;
}
namespace com { namespace sun { namespace star { namespace rendering
{
    struct Texture;
    struct StrokeAttributes;
} } } }


/* Definition of internal::PolyPolyActionFactory class */

namespace cppcanvas
{
    namespace internal
    {
        struct OutDevState;

        /** Creates encapsulated converters between GDIMetaFile and
            XCanvas. The Canvas argument is deliberately placed at the
            constructor, to force reconstruction of this object for a
            new canvas. This considerably eases internal state
            handling, since a lot of the internal state (e.g. fonts,
            text layout) is Canvas-dependent.
         */
        class PolyPolyActionFactory
        {
        public:
            /// Create polygon, fill/stroke according to state
            static ActionSharedPtr createPolyPolyAction( const ::basegfx::B2DPolyPolygon&,
                                                         const CanvasSharedPtr&,
                                                         const OutDevState&     );

            /// Create texture-filled polygon
            static ActionSharedPtr createPolyPolyAction( const ::basegfx::B2DPolyPolygon&,
                                                         const CanvasSharedPtr&,
                                                         const OutDevState&,
                                                         const ::com::sun::star::rendering::Texture& );

            /// Create line polygon (always stroked, not filled)
            static ActionSharedPtr createLinePolyPolyAction( const ::basegfx::B2DPolyPolygon&,
                                                             const CanvasSharedPtr&,
                                                             const OutDevState& );

            /// Create stroked polygon
            static ActionSharedPtr createPolyPolyAction( const ::basegfx::B2DPolyPolygon&,
                                                         const CanvasSharedPtr&,
                                                         const OutDevState&,
                                                         const ::com::sun::star::rendering::StrokeAttributes& );

            /// For transparent painting of the given polygon (normally, we take the colors always opaque)
            static ActionSharedPtr createPolyPolyAction( const ::basegfx::B2DPolyPolygon&,
                                                         const CanvasSharedPtr&,
                                                         const OutDevState&,
                                                         int nTransparency );

        private:
            // static factory, disable big four
            PolyPolyActionFactory();
            ~PolyPolyActionFactory();
            PolyPolyActionFactory(const PolyPolyActionFactory&);
            PolyPolyActionFactory& operator=( const PolyPolyActionFactory& );
        };
    }
}

#endif /* _CPPCANVAS_POLYPOLYACTION_HXX */
