/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
