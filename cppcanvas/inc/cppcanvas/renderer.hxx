/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: renderer.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:14:55 $
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

#ifndef _CPPCANVAS_RENDERER_HXX
#define _CPPCANVAS_RENDERER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _COMPHELPER_OPTIONALVALUE_HXX
#include <comphelper/optionalvalue.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _CPPCANVAS_CANVASGRAPHIC_HXX
#include <cppcanvas/canvasgraphic.hxx>
#endif
#ifndef _CPPCANVAS_COLOR_HXX
#include <cppcanvas/color.hxx>
#endif

namespace basegfx
{
    class B2DRange;
}

/* Definition of Renderer interface */

namespace cppcanvas
{

    class Renderer : public virtual CanvasGraphic
    {
    public:
        /** Render subset of metafile to given canvas

            This method renders the given subset of the content to the
            associated canvas.

            @param nStartIndex
            The index of the first action to be rendered (the indices
            correspond roughly to the action indices of the
            originating GDIMetaFile. Note, although, that certain
            actions, e.g. text, accounts for more than one index: a
            text produces as many addressable indices as it has
            characters).

            @param nEndIndex
            The index of the first action _not_ painted anymore,
            i.e. the action after the last action rendered (the
            indices correspond roughly to the action indices of the
            originating GDIMetaFile. Note, although, that certain
            actions, e.g. text, accounts for more than one index: a
            text produces as many addressable indices as it has
            characters).

            @return whether the rendering finished successfully.
         */
        virtual bool drawSubset( sal_Int32  nStartIndex,
                                 sal_Int32  nEndIndex ) const = 0;

        /** Query bounding box of metafile subset

            This method queries the actual bounding box of the given
            subset, when rendered on the associated canvas.

            @param nStartIndex
            The index of the first action to be rendered (the indices
            correspond roughly to the action indices of the
            originating GDIMetaFile. Note, although, that certain
            actions, e.g. text, accounts for more than one index: a
            text produces as many addressable indices as it has
            characters).

            @param nEndIndex
            The index of the first action _not_ painted anymore,
            i.e. the action after the last action rendered (the
            indices correspond roughly to the action indices of the
            originating GDIMetaFile. Note, although, that certain
            actions, e.g. text, accounts for more than one index: a
            text produces as many addressable indices as it has
            characters).

            @return the bounding box of the specified subset
         */
        virtual ::basegfx::B2DRange getSubsetArea( sal_Int32    nStartIndex,
                                                   sal_Int32    nEndIndex ) const = 0;

        /** Parameters for the Renderer
         */
        struct Parameters
        {
            /// Optionally forces the fill color attribute for all actions
            ::comphelper::OptionalValue< Color::IntSRGBA >          maFillColor;

            /// Optionally forces the line color attribute for all actions
            ::comphelper::OptionalValue< Color::IntSRGBA >          maLineColor;

            /// Optionally forces the text color attribute for all actions
            ::comphelper::OptionalValue< Color::IntSRGBA >          maTextColor;

            /// Optionally forces the given fontname for all text actions
            ::comphelper::OptionalValue< ::rtl::OUString >          maFontName;

            /** Optionally transforms all text output actions with the
                given matrix (in addition to the overall canvas
                transformation).

                Note that the matrix given here is applied to the unit
                rect coordinate system, i.e. the metafile is assumed
                to be contained in the unit rect.
             */
            ::comphelper::OptionalValue< ::basegfx::B2DHomMatrix >  maTextTransformation;

            /// Optionally forces the given font weight for all text actions
            ::comphelper::OptionalValue< sal_Int8 >                 maFontWeight;

            /// Optionally forces the given font letter form (italics etc.) for all text actions
            ::comphelper::OptionalValue< sal_Int8 >                 maFontLetterForm;

            /// Optionally forces underlining for all text actions
            ::comphelper::OptionalValue< bool >                     maFontUnderline;
        };
    };

    typedef ::boost::shared_ptr< ::cppcanvas::Renderer > RendererSharedPtr;
}

#endif /* _CPPCANVAS_RENDERER_HXX */
