/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: font.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:14:12 $
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

#ifndef _CPPCANVAS_FONT_HXX
#define _CPPCANVAS_FONT_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

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

    class Font
    {
    public:
        virtual ~Font() {}

        virtual ::rtl::OUString getName() const = 0;
        virtual double          getCellSize() const = 0;

        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XCanvasFont > getUNOFont() const = 0;
    };

    typedef ::boost::shared_ptr< ::cppcanvas::Font > FontSharedPtr;
}

#endif /* _CPPCANVAS_FONT_HXX */
