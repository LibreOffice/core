/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _CPPCANVAS_IMPLFONT_HXX
#define _CPPCANVAS_IMPLFONT_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <boost/shared_ptr.hpp>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <cppcanvas/font.hxx>


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
                      const OUString& rFontName,
                      const double& rCellSize );

            virtual ~ImplFont();

            virtual OUString getName() const SAL_OVERRIDE;
            virtual double          getCellSize() const SAL_OVERRIDE;

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XCanvasFont > getUNOFont() const SAL_OVERRIDE;

        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >        mxCanvas;
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont >    mxFont;
        };
    }
}

#endif /* _CPPCANVAS_IMPLFONT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
