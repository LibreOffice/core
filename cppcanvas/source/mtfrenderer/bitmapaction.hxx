/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bitmapaction.hxx,v $
 * $Revision: 1.8 $
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
