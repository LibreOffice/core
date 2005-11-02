/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impltext.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:44:48 $
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

#include <impltext.hxx>
#include <canvas/canvastools.hxx>

#ifndef _COM_SUN_STAR_RENDERING_TEXTDIRECTION_HPP__
#include <com/sun/star/rendering/TextDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP__
#include <com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_STRINGCONTEXT_HPP__
#include <com/sun/star/rendering/StringContext.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {

        ImplText::ImplText( const CanvasSharedPtr&  rParentCanvas,
                            const ::rtl::OUString&  rText ) :
            CanvasGraphicHelper( rParentCanvas ),
            mpFont(),
            maText(rText)
        {
        }

        ImplText::~ImplText()
        {
        }

        bool ImplText::draw() const
        {
            CanvasSharedPtr pCanvas( getCanvas() );

            OSL_ENSURE( pCanvas.get() != NULL &&
                        pCanvas->getUNOCanvas().is(),
                        "ImplBitmap::draw: invalid canvas" );

            rendering::StringContext aText;
            aText.Text = maText;
            aText.StartPosition = 0;
            aText.Length = maText.getLength();

            // TODO(P1): implement caching
            // TODO(F2): where to get current BiDi status?
            sal_Int8 nBidiOption = rendering::TextDirection::WEAK_LEFT_TO_RIGHT;
            pCanvas->getUNOCanvas()->drawText( aText,
                                               mpFont->getUNOFont(),
                                               pCanvas->getViewState(),
                                               getRenderState(),
                                               nBidiOption );

            return true;
        }

        void ImplText::setFont( const FontSharedPtr& rFont )
        {
            mpFont = rFont;
        }

        FontSharedPtr ImplText::getFont()
        {
            return mpFont;
        }
    }
}
