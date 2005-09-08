/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pointaction.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:21:14 $
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

#ifndef _CPPCANVAS_POINTACTION_HXX
#define _CPPCANVAS_POINTACTION_HXX

#include <action.hxx>
#include <cppcanvas/canvas.hxx>

class Point;
class Color;


/* Definition of internal::PointActionFactory class */

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
        class PointActionFactory
        {
        public:
            /// Point in current color
            static ActionSharedPtr createPointAction( const ::Point&,
                                                      const CanvasSharedPtr&,
                                                      const OutDevState& );

            /// Point in given color
            static ActionSharedPtr createPointAction( const ::Point&,
                                                      const CanvasSharedPtr&,
                                                      const OutDevState&,
                                                      const ::Color&        );

        private:
            // static factory, disable big four
            PointActionFactory();
            ~PointActionFactory();
            PointActionFactory(const PointActionFactory&);
            PointActionFactory& operator=( const PointActionFactory& );
        };
    }
}

#endif /* _CPPCANVAS_POINTACTION_HXX */
