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



#ifndef _CPPCANVAS_BITMAPACTION_HXX
#define _CPPCANVAS_BITMAPACTION_HXX

#include <cppcanvas/canvas.hxx>
#include <action.hxx>

namespace basegfx {
    class B2DPoint;
    class B2DVector;
}
class BitmapEx;

/* Definition of internal::BitmapActionFactory class */

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
        class BitmapActionFactory
        {
        public:
            /// Unscaled bitmap action, only references destination point
            static ActionSharedPtr createBitmapAction( const ::BitmapEx&,
                                                       const ::basegfx::B2DPoint& rDstPoint,
                                                       const CanvasSharedPtr&,
                                                       const OutDevState& );

            /// Scaled bitmap action, dest point and dest size
            static ActionSharedPtr createBitmapAction( const ::BitmapEx&,
                                                       const ::basegfx::B2DPoint&  rDstPoint,
                                                       const ::basegfx::B2DVector& rDstSize,
                                                       const CanvasSharedPtr&,
                                                       const OutDevState& );

        private:
            // static factory, disable big four
            BitmapActionFactory();
            ~BitmapActionFactory();
            BitmapActionFactory(const BitmapActionFactory&);
            BitmapActionFactory& operator=( const BitmapActionFactory& );
        };
    }
}

#endif /*_CPPCANVAS_BITMAPACTION_HXX */
