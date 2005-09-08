/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolyaction.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:21:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CPPCANVAS_POLYPOLYACTION_HXX
#define _CPPCANVAS_POLYPOLYACTION_HXX

#include <action.hxx>
#include <cppcanvas/canvas.hxx>

class PolyPolygon;

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
            static ActionSharedPtr createPolyPolyAction( const ::PolyPolygon&,
                                                         const CanvasSharedPtr&,
                                                         const OutDevState&     );

            /// Create texture-filled polygon
            static ActionSharedPtr createPolyPolyAction( const ::PolyPolygon&,
                                                         const CanvasSharedPtr&,
                                                         const OutDevState&,
                                                         const ::com::sun::star::rendering::Texture& );

            /// Create line polygon (always stroked, not filled)
            static ActionSharedPtr createLinePolyPolyAction( const ::PolyPolygon&,
                                                             const CanvasSharedPtr&,
                                                             const OutDevState& );

            /// Create stroked polygon
            static ActionSharedPtr createPolyPolyAction( const ::PolyPolygon&,
                                                         const CanvasSharedPtr&,
                                                         const OutDevState&,
                                                         const ::com::sun::star::rendering::StrokeAttributes& );

            /// For transparent painting of the given polygon (normally, we take the colors always opaque)
            static ActionSharedPtr createPolyPolyAction( const ::PolyPolygon&,
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
