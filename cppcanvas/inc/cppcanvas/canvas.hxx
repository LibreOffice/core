/*************************************************************************
 *
 *  $RCSfile: canvas.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:50:53 $
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

#ifndef _CPPCANVAS_CANVAS_HXX
#define _CPPCANVAS_CANVAS_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _CPPCANVAS_FONT_HXX
#include <cppcanvas/font.hxx>
#endif
#ifndef _CPPCANVAS_COLOR_HXX
#include <cppcanvas/color.hxx>
#endif

namespace rtl
{
    class OUString;
}

namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
}

namespace drafts { namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvas;
    struct ViewState;
} } } } }


/* Definition of BitmapCanvas */

namespace cppcanvas
{
    class PolyPolygon;
    class Canvas;

    // forward declaration, since PolyPolygon also references Canvas
    typedef ::boost::shared_ptr< PolyPolygon > PolyPolygonSharedPtr;

    // forward declaration, since cloneCanvas() also references Canvas
    typedef ::boost::shared_ptr< Canvas > CanvasSharedPtr;

    /** Canvas interface
     */
    class Canvas
    {
    public:
        enum
        {
            /** Extra pixel used when canvas anti-aliases.

                Enlarge the bounding box of drawing primitives by this
                amount in both dimensions, and on both sides of the
                bounds, to account for extra pixel touched outside the
                actual primitive bounding box, when the canvas
                performs anti-aliasing.
             */
            ANTIALIASING_EXTRA_SIZE=2
        };

        virtual ~Canvas() {}

        virtual void                        setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) = 0;
        virtual ::basegfx::B2DHomMatrix     getTransformation() const = 0;

        virtual void                        setClip( const PolyPolygonSharedPtr& rClipPoly ) = 0;
        virtual PolyPolygonSharedPtr        getClip() const = 0;

        virtual FontSharedPtr               createFont( const ::rtl::OUString& rFontName, const double& rCellSize ) const = 0;

        virtual ColorSharedPtr              createColor() const = 0;

        virtual CanvasSharedPtr             clone() const = 0;

        // this should be considered private. if RTTI gets enabled
        // someday, remove that to a separate interface
        virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::rendering::XCanvas >          getUNOCanvas() const = 0;
        virtual ::drafts::com::sun::star::rendering::ViewState      getViewState() const = 0;
    };

}

#endif /* _CPPCANVAS_CANVAS_HXX */
