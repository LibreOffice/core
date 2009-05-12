/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lineaction.hxx,v $
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
