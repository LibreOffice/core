/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapaction.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:18:02 $
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

#ifndef _CPPCANVAS_BITMAPACTION_HXX
#define _CPPCANVAS_BITMAPACTION_HXX

#include <cppcanvas/canvas.hxx>
#include <action.hxx>

class Point;
class Size;
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
                                                       const ::Point&   rDstPoint,
                                                       const CanvasSharedPtr&,
                                                       const OutDevState& );

            /// Scaled bitmap action, dest point and dest size
            static ActionSharedPtr createBitmapAction( const ::BitmapEx&,
                                                       const ::Point&   rDstPoint,
                                                       const ::Size&    rDstSize,
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
