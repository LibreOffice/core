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



#ifndef _BGFX_TOOLS_CLIPSTATE_HXX
#define _BGFX_TOOLS_CLIPSTATE_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DRange;
    class B2DPolyRange;
    class B2DPolygon;
    class B2DPolyPolygon;

    namespace tools
    {
        struct ImplB2DClipState;

        /** This class provides an optimized, symbolic clip state for graphical output

            Having a current 'clip' state is a common attribute of
            almost all graphic output APIs, most of which internally
            represent it via a list of rectangular bands. In contrast,
            this implementation purely uses symbolic clips, but in a
            quite efficient manner, deferring actual evaluation until
            a clip representation is requested, and using faster code
            paths for common special cases (like all-rectangle clips)
         */
        class B2DClipState
        {
        public:
            typedef o3tl::cow_wrapper< ImplB2DClipState > ImplType;

        private:
            ImplType mpImpl;

        public:
            /// Init clip, in 'cleared' state - everything is visible
            B2DClipState();
            ~B2DClipState();
            B2DClipState( const B2DClipState& );
            explicit B2DClipState( const B2DRange& );
            explicit B2DClipState( const B2DPolygon& );
            explicit B2DClipState( const B2DPolyPolygon& );
            B2DClipState& operator=( const B2DClipState& );

            /// unshare this poly-range with all internally shared instances
            void makeUnique();

            /// Set clip to 'null' - nothing is visible
            void makeNull();
            /// returns true when clip is 'null' - nothing is visible
            bool isNull() const;

            /// Set clip 'cleared' - everything is visible
            void makeClear();
            /// returns true when clip is 'cleared' - everything is visible
            bool isCleared() const;

            bool operator==(const B2DClipState&) const;
            bool operator!=(const B2DClipState&) const;

            void unionRange(const B2DRange& );
            void unionPolygon(const B2DPolygon& );
            void unionPolyPolygon(const B2DPolyPolygon& );
            void unionClipState(const B2DClipState& );

            void intersectRange(const B2DRange& );
            void intersectPolygon(const B2DPolygon& );
            void intersectPolyPolygon(const B2DPolyPolygon& );
            void intersectClipState(const B2DClipState& );

            void subtractRange(const B2DRange& );
            void subtractPolygon(const B2DPolygon& );
            void subtractPolyPolygon(const B2DPolyPolygon& );
            void subtractClipState(const B2DClipState& );

            void xorRange(const B2DRange& );
            void xorPolygon(const B2DPolygon& );
            void xorPolyPolygon(const B2DPolyPolygon& );
            void xorClipState(const B2DClipState& );

            B2DPolyPolygon getClipPoly() const;
        };
    }
}

#endif // _BGFX_TOOLS_CLIPSTATE_HXX
