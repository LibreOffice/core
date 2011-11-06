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
