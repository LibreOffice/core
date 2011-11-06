/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
