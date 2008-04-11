/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impltext.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppcanvas.hxx"

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
#include <rtl/ustring.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>


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
