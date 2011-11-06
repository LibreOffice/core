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



#ifndef _CPPCANVAS_LINEACTION_HXX
#define _CPPCANVAS_LINEACTION_HXX

#include <action.hxx>
#include <cppcanvas/canvas.hxx>

namespace basegfx {
    class B2DPoint;
}


/* Definition of internal::LineActionFactory class */

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
        class LineActionFactory
        {
        public:
            /// Plain hair line from point 1 to point 2
            static ActionSharedPtr createLineAction( const ::basegfx::B2DPoint&,
                                                     const ::basegfx::B2DPoint&,
                                                     const CanvasSharedPtr&,
                                                     const OutDevState& );

        private:
            // static factory, disable big four
            LineActionFactory();
            ~LineActionFactory();
            LineActionFactory(const LineActionFactory&);
            LineActionFactory& operator=( const LineActionFactory& );
        };
    }
}

#endif /* _CPPCANVAS_LINEACTION_HXX */
