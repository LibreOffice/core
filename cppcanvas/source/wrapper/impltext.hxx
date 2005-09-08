/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impltext.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:31:21 $
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

#ifndef _CANVAS_IMPLTEXT_HXX
#define _CANVAS_IMPLTEXT_HXX

#ifndef _COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <com/sun/star/rendering/RenderState.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_STRINGCONTEXT_HPP__
#include <com/sun/star/rendering/StringContext.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP__
#include <com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCANVASFONT_HPP__
#include <com/sun/star/rendering/XCanvasFont.hpp>
#endif

#include <cppcanvas/text.hxx>
#include <canvasgraphichelper.hxx>


namespace cppcanvas
{
    namespace internal
    {

        class ImplText : public virtual ::cppcanvas::Text, protected CanvasGraphicHelper
        {
        public:

            ImplText( const CanvasSharedPtr&    rParentCanvas,
                      const ::rtl::OUString&    rText );

            virtual ~ImplText();

            virtual bool draw() const;

            virtual void setFont( const FontSharedPtr& );
            virtual FontSharedPtr getFont();

        private:
            // default: disabled copy/assignment
            ImplText(const ImplText&);
            ImplText& operator= ( const ImplText& );

            FontSharedPtr       mpFont;
            ::rtl::OUString     maText;
        };
    }
}

#endif /* _CANVAS_IMPLTEXT_HXX */
