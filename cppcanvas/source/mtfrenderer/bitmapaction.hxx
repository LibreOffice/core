/*************************************************************************
 *
 *  $RCSfile: bitmapaction.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:54:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CPPCANVAS_BITMAPACTION_HXX
#define _CPPCANVAS_BITMAPACTION_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <drafts/com/sun/star/rendering/RenderState.hpp>
#endif

#include <cppcanvas/canvas.hxx>
#include <action.hxx>

class Point;
class Size;
class BitmapEx;
class Color;

namespace drafts { namespace com { namespace sun { namespace star { namespace rendering
{
    class   XBitmap;
} } } } }

/* Definition of internal::BitmapAction class */

namespace cppcanvas
{
    namespace internal
    {
        struct OutDevState;

        /** Encapsulated converter between GDIMetaFile and
            XCanvas. The Canvas argument is deliberately placed at the
            constructor, to force reconstruction of this object for a
            new canvas. This considerably eases internal state
            handling, since a lot of the internal state
            (e.g. deviceColor) is Canvas-dependent.
         */
        class BitmapAction : public Action
        {
        public:
            BitmapAction( const ::BitmapEx&,
                          const ::Point&    rDstPoint,
                          const CanvasSharedPtr&,
                          const OutDevState& );
            BitmapAction( const ::BitmapEx&,
                          const ::Point&    rDstPoint,
                          const ::Size&     rDstSize,
                          const CanvasSharedPtr&,
                          const OutDevState& );
            BitmapAction( const ::BitmapEx&,
                          const ::Point&    rSrcPoint,
                          const ::Size&     rSrcSize,
                          const ::Point&    rDstPoint,
                          const ::Size&     rDstSize,
                          const CanvasSharedPtr&,
                          const OutDevState& );
            virtual ~BitmapAction();

            virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;

        private:
            // default: disabled copy/assignment
            BitmapAction(const BitmapAction&);
            BitmapAction& operator = ( const BitmapAction& );

            ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::rendering::XBitmap >  mxBitmap;
            CanvasSharedPtr                                     mpCanvas;
            ::drafts::com::sun::star::rendering::RenderState    maState;
        };
    }
}

#endif /*_CPPCANVAS_BITMAPACTION_HXX */
