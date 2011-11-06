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



#ifndef _CPPCANVAS_TRANSPARENCYGROUPACTION_HXX
#define _CPPCANVAS_TRANSPARENCYGROUPACTION_HXX

#include <cppcanvas/canvas.hxx>
#include <cppcanvas/renderer.hxx>
#include <action.hxx>

#include <memory> // auto_ptr

namespace basegfx {
    class B2DPoint;
    class B2DVector;
}

class GDIMetaFile;
class Gradient;


/* Definition of internal::TransparencyGroupActionFactory class */

namespace cppcanvas
{
    namespace internal
    {
        struct OutDevState;

        typedef ::std::auto_ptr< GDIMetaFile >  MtfAutoPtr;
        typedef ::std::auto_ptr< Gradient >     GradientAutoPtr;

        /** Transparency group action.

            This action groups a bunch of other actions, to be
            rendered with the given transparency setting against the
            background.

            Creates encapsulated converters between GDIMetaFile and
            XCanvas. The Canvas argument is deliberately placed at the
            constructor, to force reconstruction of this object for a
            new canvas. This considerably eases internal state
            handling, since a lot of the internal state (e.g. fonts,
            text layout) is Canvas-dependent.
         */
        class TransparencyGroupActionFactory
        {
        public:
            /** Create new transparency group action.

                @param rGroupMtf
                Metafile that groups all actions to be rendered
                transparent

                @param rParms
                Render parameters

                @param rDstPoint
                Left, top edge of destination, in current state
                coordinate system

                @param rDstSize
                Size of the transparency group object, in current
                state coordinate system.

                @param nAlpha
                Alpha value, must be in the range [0,1]
             */
            static ActionSharedPtr createTransparencyGroupAction( MtfAutoPtr&                   rGroupMtf,
                                                                  const Renderer::Parameters&   rParms,
                                                                  const ::basegfx::B2DPoint&    rDstPoint,
                                                                  const ::basegfx::B2DVector&   rDstSize,
                                                                  double                        nAlpha,
                                                                  const CanvasSharedPtr&        rCanvas,
                                                                  const OutDevState&            rState );

            /** Create new transparency group action.

                @param rGroupMtf
                Metafile that groups all actions to be rendered
                transparent.

                @param rAlphaGradient
                VCL gradient, to be rendered into the action's alpha
                channel.

                @param rParms
                Render parameters

                @param rDstPoint
                Left, top edge of destination, in current state
                coordinate system

                @param rDstSize
                Size of the transparency group object, in current
                state coordinate system.
             */
            static ActionSharedPtr createTransparencyGroupAction( MtfAutoPtr&                   rGroupMtf,
                                                                  GradientAutoPtr&              rAlphaGradient,
                                                                  const Renderer::Parameters&   rParms,
                                                                  const ::basegfx::B2DPoint&    rDstPoint,
                                                                  const ::basegfx::B2DVector&   rDstSize,
                                                                  const CanvasSharedPtr&        rCanvas,
                                                                  const OutDevState&            rState );

        private:
            // static factory, disable big four
            TransparencyGroupActionFactory();
            ~TransparencyGroupActionFactory();
            TransparencyGroupActionFactory(const TransparencyGroupActionFactory&);
            TransparencyGroupActionFactory& operator=( const TransparencyGroupActionFactory& );
        };
    }
}

#endif /*_CPPCANVAS_TRANSPARENCYGROUPACTION_HXX */
