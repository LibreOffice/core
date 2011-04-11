/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _CANVAS_IMPLTEXT_HXX
#define _CANVAS_IMPLTEXT_HXX

#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
