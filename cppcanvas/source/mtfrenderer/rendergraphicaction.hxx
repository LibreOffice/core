/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _CPPCANVAS_RENDERGRAPHICACTION_HXX
#define _CPPCANVAS_RENDERGRAPHICACTION_HXX

#include <cppcanvas/canvas.hxx>
#include <action.hxx>

namespace basegfx {
    class B2DPoint;
    class B2DVector;
}
namespace vcl {
    class RenderGraphic;
}

/* Definition of internal::RenderGraphicActionFactory class */

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
        class RenderGraphicActionFactory
        {
        public:
            /// Scaled rendergraphic action, dest point and dest size
            static ActionSharedPtr createRenderGraphicAction( const ::vcl::RenderGraphic& rRenderGraphic,
                                                       const ::basegfx::B2DPoint&  rDstPoint,
                                                       const ::basegfx::B2DVector& rDstSize,
                                                       const CanvasSharedPtr&,
                                                       const OutDevState& );

        private:
            // static factory, disable big four
            RenderGraphicActionFactory();
            ~RenderGraphicActionFactory();
            RenderGraphicActionFactory(const RenderGraphicActionFactory&);
            RenderGraphicActionFactory& operator=( const RenderGraphicActionFactory& );
        };
    }
}

#endif /*_CPPCANVAS_RENDERGRAPHICACTION_HXX */
