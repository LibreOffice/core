/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implfont.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _CPPCANVAS_IMPLFONT_HXX
#define _CPPCANVAS_IMPLFONT_HXX

#include <com/sun/star/uno/Reference.hxx>

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP__
#include <com/sun/star/rendering/XCanvas.hpp>
#endif
#include <cppcanvas/font.hxx>

namespace rtl
{
    class OUString;
}

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvasFont;
} } } }

/* Definition of Font class */

namespace cppcanvas
{

    namespace internal
    {

        class ImplFont : public Font
        {
        public:
            ImplFont( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::rendering::XCanvas >& rCanvas,
                      const ::rtl::OUString& rFontName,
                      const double& rCellSize );

            virtual ~ImplFont();

            virtual ::rtl::OUString getName() const;
            virtual double          getCellSize() const;

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XCanvasFont > getUNOFont() const;

        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >        mxCanvas;
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont >    mxFont;
        };
    }
}

#endif /* _CPPCANVAS_IMPLFONT_HXX */
